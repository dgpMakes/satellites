#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <queue>

std::vector<std::string> split(std::string s, std::string delimiter);
void replace(std::string & data, std::string search, std::string replace);

struct observation {
    int time;
    int band;
};


// // What every seach problem shall implement for the search algorithm to use
class search_problem {
    public:
        static search_problem* get_initial_state();
        static search_problem* get_final_state();
        std::vector<search_problem>* get_sucessors();
        bool is_goal_state();
        ~search_problem();
};

/*
In this problem we are assuming there are only 12 hours on a day
There are only 4 bands and three possitions for the two considered satellites.
*/

class satellite_state {
    /* Dynamic variables */
    // Observations done
    private:
    std::vector<bool> is_obs_done;
    int time;
    int sat1_band;
    int sat2_band;
    bool downlinked;
    int sat1_remaining_battery;
    int sat2_remaining_battery;

    public:

    satellite_state(int time, int sat1_band, int sat2_band,
     std::vector<bool> is_obs_done, bool downlinked) {
        this->is_obs_done = is_obs_done;
        
        if(time >= 0 && time < 13){
            this->time = time;
        }
        
        this->sat1_band = sat1_band;
        this->sat2_band = sat2_band;
        this->downlinked = downlinked;

        this->sat1_remaining_battery = sat1_max_battery;
        this->sat2_remaining_battery = sat2_max_battery;
    }

    satellite_state() = default;

    
    /* Static parts */
    // Max battery capacity
    static int sat1_max_battery;
    static int sat2_max_battery;

    // How much does battery increase per charge hour
    static int sat1_battery_recharge;
    static int sat2_battery_recharge;

    // Cost of making an observation
    static int sat1_observe_cost;
    static int sat2_observe_cost;

    // Cost of downlinking
    static int sat1_downlink_cost;
    static int sat2_downlink_cost;

    // Cost of visibility band turn
    static int sat1_turn_cost;
    static int sat2_turn_cost;

    static std::vector<observation> obs_to_do;

    static satellite_state* get_initial_state(){
        std::vector<bool> v (obs_to_do.size(), false);
        return new satellite_state(0, 1, 3, v, false);
    };

    // static satellite_state* get_final_state(){
    //     return satellite_state(0, 1, 3, new vector<bool> (obs_to_do.size(), true), true);
    // };


    std::vector<satellite_state> get_sucessors() {
        std::vector<satellite_state> v;
        /* Operations definition */

        // Advance time
        if(this->time < 12){
            satellite_state s1(this->time + 1, this->sat1_band, this->sat2_band, this->is_obs_done, this->downlinked);
            v.push_back(s1);
        } else {
            satellite_state s1(0, this->sat1_band, this->sat2_band, this->is_obs_done, this->downlinked);
            v.push_back(s1);
        }

        //satellite1
        
        bool observation(){
        // Make an observation
            //Check if there is enough battery
            if(sat1_remaining_battery>sat1_observe_cost){
           
                //Check if there is something to observe
                int i = 0;
                int j = 0;

                //Check if there is something to observe and if it has been observed
                while(i<=3 && j < obs_to_do[i].size()){
                    if(sat1_band == i && time == j && !is_obs_done[i][j]){
                        is_obs_done[i][j] = true;
                        sat1_remaining_battery -= sat1_observe_cost;
                        i = 5;
                    }
                    i++;
                }
            }
            //Check if it has been already observed
        }
        // Recharge
            //Check if battery is at its maximum

        // Turn to a different visibility band
            //Check if there is enough battery


        // Downlink information
            //Check if there is enough battery




        return v;
    };

    bool is_goal_state() {
        return downlinked;
    };

    bool operator==(const satellite_state& other) {
        bool time_eq = this->time == other.time;
        bool sat1_band_eq = this->sat1_band == other.sat1_band;
        bool sat2_band_eq = this->sat2_band == other.sat2_band;
        bool downlinked_eq = this->downlinked == other.downlinked;
        bool done_obs_eq = this->is_obs_done == other.is_obs_done;
        return time_eq && sat1_band_eq && sat2_band_eq && downlinked_eq && done_obs_eq;
    };

    friend std::ostream & operator<<(std::ostream & os, const satellite_state & foo){ 
          os << (foo.downlinked ? "yes":"no") << foo.time << 
          " sat1band:" << std::to_string(foo.sat1_band) << " sat2band:" << 
          std::to_string(foo.sat2_band); 

          return os;
    }
};









    /* Static parts */
    // Cost of making an observation
    int satellite_state::sat1_observe_cost = 0;
    int satellite_state::sat2_observe_cost = 0;

    // Cost of downlinking
    int satellite_state::sat1_downlink_cost = 0;
    int satellite_state::sat2_downlink_cost = 0;

    // Cost of visibility band turn
    int satellite_state::sat1_turn_cost = 0;
    int satellite_state::sat2_turn_cost = 0;

    // How much does battery increase per charge hour
    int satellite_state::sat1_battery_recharge = 0;
    int satellite_state::sat2_battery_recharge = 0;

    // Max battery capacity
    int satellite_state::sat1_max_battery = 0;
    int satellite_state::sat2_max_battery = 0;

    /* Static parts */
    // Observations to do
    std::vector<observation> satellite_state::obs_to_do;









// class search_algorithm {



// };
class a_star {

    public:
        
        a_star(satellite_state root){
            this->root = root;
        }

        void search(){
            std::vector<satellite_state> root_sucessors = root.get_sucessors();
            for (satellite_state sp : root_sucessors){
                queue.push(sp);
            }

            // Infinite loop till a goal state is found
            while(!queue.front().is_goal_state() && !queue.empty()){
            
                // Extract element from queue
                satellite_state to_expand = queue.front();
                queue.pop();

                // Get all sucessors and append them to the fifo queue
                std::vector<satellite_state> sucessors = to_expand.get_sucessors();
                for(satellite_state sucessor : sucessors){
                    // Check if the node has already been visited
                    if(visited.find(sucessor) != visited.end()){
                        queue.push(sucessor);
                        visited.insert(sucessor);
                    }
                }
            }

            if(queue.empty()) {
                std::cout << "No solution found!!!\n";
                return;
            }

            std::cout << queue.front() << "\n Found a goal state!!!! \n";
            std::cout << queue.front();
        }

    private:
        std::queue<satellite_state> queue;
        std::unordered_set<satellite_state> visited;
        satellite_state root;





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
    std::vector<observation> observations;
    for(std::string obs : observations_list) {

        // Get rid of the separators
        replace(obs, "(", "");
        replace(obs, ")", "");

        // Two strings, each one a number with a coordinate
        std::vector<std::string> coordinates = split(obs, ",");

        // Create an observation and fill information
        observation o;
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
    satellite_state::sat1_observe_cost = std::stoi(sat1_data[0]);
    satellite_state::sat2_observe_cost = std::stoi(sat2_data[0]);

    // Cost of downlinking
    satellite_state::sat1_downlink_cost = std::stoi(sat1_data[1]);
    satellite_state::sat2_downlink_cost = std::stoi(sat2_data[1]);

    // Cost of visibility band turn
    satellite_state::sat1_turn_cost = std::stoi(sat1_data[2]);
    satellite_state::sat2_turn_cost = std::stoi(sat2_data[2]);

    // How much does battery increase per charge hour
    satellite_state::sat1_battery_recharge = std::stoi(sat1_data[3]);
    satellite_state::sat2_battery_recharge = std::stoi(sat2_data[3]);

    // Max battery capacity
    satellite_state::sat1_max_battery = std::stoi(sat1_data[4]);
    satellite_state::sat2_max_battery = std::stoi(sat2_data[4]);

    /* Static parts */
    // Observations to do
    satellite_state::obs_to_do = observations;
    satellite_state root = *satellite_state::get_initial_state();
    a_star a(root);
    a.search();


    
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