#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <queue>

std::vector<std::string> split(std::string s, std::string delimiter);
void replace(std::string & data, std::string search, std::string replace);




// // What every seach problem shall implement for the search algorithm to use
class search_problem {
    public:
        std::vector<search_problem>* get_successors();
        bool is_goal_state();
        ~search_problem();
};

// The only thing that has a cost is passing time
enum action{observation, recharge, turn, downlink, nothing};

struct node {
    action sat1_action;
    action sat2_action;
    int accumulated_cost;
    satellite_state state;
    satellite_state* parent;
};

struct observation {
    int time;
    int band;
};

/*
In this problem we are assuming there are only 12 hours on a day
There are only 4 bands and 3 possitions for the two considered satellites.
*/

class satellite_state {
    /* Dynamic variables */
    // Observations done
        

    public:
        std::vector<bool> is_obs_done;
        int time;
        std::vector<int> sat_band;
        std::vector<bool> downlinked;
        std::vector<int> sat_remaining_battery;
        
    satellite_state(int time, std::vector<int> sat_band, std::vector<bool> downlinked,
     std::vector<bool> is_obs_done, std::vector<int> sat_remaining_battery) 
    {
        this->is_obs_done = is_obs_done;
        this->time = time;
        this->sat_band = sat_band;
        this->downlinked = downlinked;
        this->sat_remaining_battery = sat_max_battery;
    }

    satellite_state() = default;

    
    /* Static parts */
    // Max battery capacity
    static std::vector<int> sat_max_battery;

    // How much does battery increase per charge hour
    static std::vector<int> sat_recharge_battery;

    // Cost of making an observation
    static std::vector<int> sat_observe_cost;

    // Cost of downlinking
    static std::vector<int> sat_downlink_cost;

    // Cost of visibility band turn
    static std::vector<int> sat_turn_cost;

    static std::vector<struct observation> obs_to_do;

        
    

    // static satellite_state* get_final_state(){
    //     return satellite_state(0, 1, 3, new vector<bool> (obs_to_do.size(), true), true);
    // };


    std::vector<node> get_successors() {
        std::vector<node> v;
        /* Operations definition */

        // Definir un array de booleanos, indicando si un satélite puede hacer una acción.
        std::vector<bool> sat1(5, false); // Observe, turn, recharge, downlink, do_nothing
        std::vector<bool> sat2(5, false);

        for(int i = 0; i < 5; i++) {
            for(int j = 0; j < 5; j++) {
                if(sat1[i] && sat2[j]) {
                    // Add the operation
                    int s_time = this->time + 1;
                    std::vector<int> s_sat_band = sat_band;
                    std::vector<bool> s_downlinked = downlinked;
                    std::vector<int> s_sat_remaining_battery = sat_remaining_battery;
                    std::vector<bool> s_is_obs_done = is_obs_done;
                    action s1 = nothing;
                    action s2 = nothing;

                    // Changing band
                    if(i == 1){
                        s_sat_band[0] = (sat_band[0] == 0 ? 1:0);
                        s_sat_remaining_battery[0] -= satellite_state::sat_turn_cost[0];
                        s1 = turn;
                    }
    
                    if(j == 1){
                        s_sat_band[1] = (sat_band[1] == 2 ? 1:2);
                        s_sat_remaining_battery[1] -= satellite_state::sat_turn_cost[1];
                        s2 = turn;
                    }
                    
                    
                    // Recharge action
                    if(i == 2){
                        s_sat_remaining_battery[0] += satellite_state::sat_recharge_battery[0];
                        s1 = recharge;
                    }
                    if(j == 2) {
                        s_sat_remaining_battery[1] += satellite_state::sat_recharge_battery[1];
                        s2 = recharge;
                    }
                    
                    // Downlink action
                    if(i == 3){
                        s_downlinked[0] = true;
                        s_sat_remaining_battery[0] -= satellite_state::sat_downlink_cost[0];
                        s1 = downlink;
                    }
                    if(j == 3){
                        s_downlinked[1] = true;
                        s_sat_remaining_battery[1] -= satellite_state::sat_downlink_cost[1];
                        s2 = downlink;
                    } 

                    //If do nothing, nothing happens and time passes. 

                    satellite_state s_state(s_time, s_sat_band, s_downlinked, s_is_obs_done, s_sat_remaining_battery);

                    node n;
                    n.sat1_action = s1;
                    n.sat2_action = s2;
                    n.parent = this;
                    n.state = s_state;

                    v.push_back(n);

                }
            }
        }

  
    


        // for (int satellite=0; satellite<2; satellite++){
            
        //     int to_downlink[2];
            
        //     // Make an observation
        //     //Check if there is enough battery
        //     if(sat_remaining_battery[satellite]>sat_observe_cost[satellite]){
        
        //         //Check if there is something to observe
        //         int i = 0;
        //         int j = 0;
        //         bool is_observed = false;

        //         //Check if there is something to observe and if it has been observed
        //         while(i<3 && j < obs_to_do.size()){
        //             if(sat_band[satellite] == i && time == j && !is_obs_done[i][j]){
        //                 // is_obs_done[i][satellitej] = true;
        //                 sat_remaining_battery[satellite] -= sat_observe_cost[satellite];
        //                 i = 5;
        //                 //ok, se puede
        //             }
        //             i++;
        //         }
        //     }
          
        // }

        return v;
        
    };

    bool is_goal_state() {
        return downlinked[0] && downlinked[1];
    };

    bool operator==(const satellite_state& other) {
        bool time_eq = this->time == other.time;
        bool sat1_band_eq = this->sat_band[0] == other.sat_band[0];
        bool sat2_band_eq = this->sat_band[1] == other.sat_band[1];
        bool sat1_downlinked_eq = this->downlinked[0] == other.downlinked[0];
        bool sat2_downlinked_eq = this->downlinked[1] == other.downlinked[1];

        bool done_obs_eq = this->is_obs_done == other.is_obs_done;
        return time_eq && sat1_band_eq && sat2_band_eq && done_obs_eq;
    };

    friend std::ostream & operator<<(std::ostream & os, const satellite_state & foo){ 
          os << (foo.downlinked[0] ? "yes":"no") << foo.time << 
          " sat1band:" << std::to_string(foo.sat_band[0]) << " sat2band:" << 
          std::to_string(foo.sat_band[1]); 

          return os;
    }
};


    /* Static parts */
    // Cost of making an observation
    std::vector<int> satellite_state::sat_observe_cost{2, 0};

    // Cost of downlinking
    std::vector<int> satellite_state::sat_downlink_cost{0,0};

    // Cost of visibility band turn
    std::vector<int> satellite_state::sat_turn_cost{0,0};

    // How much does battery increase per charge hour
    std::vector<int> satellite_state::sat_recharge_battery{0,0};

    // Max battery capacity
    std::vector<int> satellite_state::sat_max_battery{0,0};
    // Observations to do
    std::vector<struct observation> satellite_state::obs_to_do;









// class search_algorithm {



// };
class a_star {

    public:
        
        void search(satellite_state root){
            node n;
            n.accumulated_cost = 0;
            n.parent = nullptr;
            n.state = root;            
            queue.push(n);

            // Infinite loop till a goal state is found
            while(!queue.front().state.is_goal_state() && !queue.empty()){
            
                // Extract element from queue
                node to_expand = queue.front();
                queue.pop();

                // Get all sucessors and append them to the fifo queue
                std::vector<node> sucessors = to_expand.state.get_successors();
                for(node sucessor : sucessors){
                    // Check if the node has already been visited
                    if(visited.find(sucessor.state) != visited.end()){
                        queue.push(sucessor);
                        visited.insert(sucessor.state);
                    }
                }
            }

            if(queue.empty()) {
                std::cout << "No solution found!!!\n";
                return;
            }

            std::cout << queue.front().state << "\n Found a goal state!!!! \n";
            std::cout << queue.front().state;
        }

    private:
        std::queue<node> queue;
        std::unordered_set<satellite_state> visited;





};

int main(int argc, char **argv) {

    if(argc != 3){
        std::cerr << "The number of arguments is incorrect";
        return 0;
    }

    std::string problem_file_content;

    std::ifstream file(argv[1], std::ifstream::in);
    
    // Parsing the problem's data
    std::stringstream strStream;
    strStream << file.rdbuf();
    problem_file_content = strStream.str();

    // Get each line
    std::vector<std::string> lines = split(problem_file_content, "\n");


    // Get observations list
    replace(lines[0], " ", "");
    replace(lines[0], "OBS:", "");
    std::vector<std::string> observations_list = split(lines[0], ";");

    // Transform each observation into a observation struct
    std::vector<struct observation> observations;
    for(std::string obs : observations_list) {

        // Get rid of the separators
        replace(obs, "(", "");
        replace(obs, ")", "");

        // Two strings, each one a number with a coordinate
        std::vector<std::string> coordinates = split(obs, ",");

        // Create an observation and fill information
        struct observation o;
        o.time = std::stoi(coordinates[0]);
        o.band = std::stoi(coordinates[1]);
        observations.push_back(o);
    }

    // Extract satellite one static parameters
    replace(lines[1], " ", "");
    replace(lines[1], "SAT1:", "");
    std::vector<std::string> sat1_data = split(lines[1], ";");

    

    replace(lines[2], " ", "");
    replace(lines[2], "SAT2:", "");
    std::vector<std::string> sat2_data = split(lines[2], ";");


    // Define static parts of the problem and pass struct with all the data

    /* Static parts */
    // Cost of making an observation
    satellite_state::sat_observe_cost[0] = std::stoi(sat1_data[0]);
    satellite_state::sat_observe_cost[1] = std::stoi(sat2_data[0]);

    // Cost of downlinking
    satellite_state::sat_downlink_cost[0] = std::stoi(sat1_data[1]);
    satellite_state::sat_downlink_cost[1] = std::stoi(sat2_data[1]);

    // Cost of visibility band turn
    satellite_state::sat_turn_cost[0] = std::stoi(sat1_data[2]);
    satellite_state::sat_turn_cost[1] = std::stoi(sat2_data[2]);

    // How much does battery increase per charge hour
    satellite_state::sat_recharge_battery[0] = std::stoi(sat1_data[3]);
    satellite_state::sat_recharge_battery[1] = std::stoi(sat2_data[3]);

    // Max battery capacity
    satellite_state::sat_max_battery[0] = std::stoi(sat1_data[4]);
    satellite_state::sat_max_battery[2] = std::stoi(sat2_data[4]);

    /* Static parts */
    // Observations to do
    satellite_state::obs_to_do = observations;
    std::vector<bool> v (satellite_state::obs_to_do.size(), false);
    // satellite_state root = satellite_state(0, 1, 3, v, false);
    // a_star a(root);
    // a.search();


    
    return 0;

}



















// Helper functions

std::vector<std::string> split(std::string s, std::string delimiter) {
    std::size_t pos = 0;
    std::string sub;
    std::vector<std::string> extracted_lines;

    while ((pos = s.find(delimiter)) != std::string::npos) {
        sub = s.substr(0, pos);
        extracted_lines.push_back(sub);
        s.erase(0, pos + delimiter.length());
    }
    // Add the last part
    extracted_lines.push_back(s);

    return extracted_lines;
}

void replace(std::string & data, std::string search, std::string replace)
{
    // Get the first occurrence
    size_t pos = data.find(search);
    // Repeat till end is reached
    while(pos != std::string::npos)
    {
        // Replace this occurrence of Sub String
        data.replace(pos, search.size(), replace);
        // Get the next occurrence from the current position
        pos = data.find(search, pos + replace.size());
    }
}