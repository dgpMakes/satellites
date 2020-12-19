#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <queue>
#include "main.h"
#include <bitset>

#define OBSERVE_UP 2
#define OBSERVE_DOWN 3
#define RECHARGE 1
#define NOTHING 0
#define DOWNLINK 4
#define TURN 5

#define PROBLEM_HEIGHT 4
#define PROBLEM_WIDTH 12


// What every seach problem shall implement for the search algorithm to use
class search_problem {
    public:
        std::vector<search_problem>* get_successors();
        bool is_goal_state();
        ~search_problem();
};

// The only thing that has a cost is passing time
enum action{observe_up=OBSERVE_UP, observe_down=OBSERVE_DOWN, recharge=RECHARGE, turn=TURN, downlink=DOWNLINK, nothing=NOTHING};

struct node {
    action sat0_action;
    action sat1_action;
    int accumulated_cost;
    satellite_state* state;
    node* parent;
};

struct observation {
    int time;
    int band;
};

auto comp = [](node* lhs, node* rhs)
    {
        return lhs->accumulated_cost < rhs->accumulated_cost;
    };

/*
In this problem we are assuming there are only 12 hours on a day
There are only 4 bands and 3 possitions for the two considered satellites.
*/

class satellite_state {
    /* Dynamic variables */
    // Observations done

    public:
        int time;
        std::bitset<PROBLEM_WIDTH*PROBLEM_HEIGHT> obs_to_do;
        std::vector<int> sat_band;
        std::vector<bool> downlinked;
        std::vector<int> sat_remaining_battery;
        
    satellite_state(int time, std::vector<int> sat_band, std::vector<bool> downlinked,
     std::bitset<PROBLEM_WIDTH*PROBLEM_HEIGHT> obs_to_do, std::vector<int> sat_remaining_battery) 
    {
        this->obs_to_do = obs_to_do;
        this->time = time;
        this->sat_band = sat_band;
        this->downlinked = downlinked;
        this->sat_remaining_battery = sat_remaining_battery;
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


    std::vector<node*> get_successors() {
        std::vector<node*> v;
        /* Operations definition */
        // Definir un array de booleanos, indicando si un satélite puede hacer una acción.
        std::vector<bool> sat0(6, false); // Observe up, Observe down, turn, recharge, downlink, do_nothing
        std::vector<bool> sat1(6, false); // Observe up, Observe down, turn, recharge, downlink, do_nothing
        
        // Check if satellites can observe up
        sat0[OBSERVE_UP] = obs_to_do[this->sat_band[0]*PROBLEM_WIDTH + time] && (sat_remaining_battery[0] >= sat_observe_cost[0]) == 1;
        sat1[OBSERVE_UP] = obs_to_do[this->sat_band[1]*PROBLEM_WIDTH  + time] && (sat_remaining_battery[1] >= sat_observe_cost[1]) == 1;

        // Check observe down
        sat0[OBSERVE_DOWN] = obs_to_do[(this->sat_band[0]+1)*PROBLEM_WIDTH + time] && (sat_remaining_battery[0] >= sat_observe_cost[0]) == 1;
        sat1[OBSERVE_DOWN] = obs_to_do[(this->sat_band[1]+1)*PROBLEM_WIDTH + time] && (sat_remaining_battery[1] >= sat_observe_cost[1]) == 1;

        //Check if satellites can turn
        sat0[TURN] = sat_remaining_battery[0] >= sat_turn_cost[0];
        sat1[TURN] = sat_remaining_battery[1] >= sat_turn_cost[1];
          
        // Check if satellites can recharge        
        sat0[RECHARGE] = sat_remaining_battery[0] < sat_max_battery[0];
        sat1[RECHARGE] = sat_remaining_battery[1] < sat_max_battery[1];
        
        // Check if satellites can downlink
        bool found_obs_to_do = obs_to_do.count() != 0;

        if(!found_obs_to_do) {
            sat0[DOWNLINK] = this->sat_remaining_battery[0] >= sat_downlink_cost[0];
            sat1[DOWNLINK] = this->sat_remaining_battery[1] >= sat_downlink_cost[1];
        }      


        // Satellites can always do nothing
        sat0[NOTHING] = true;
        sat1[NOTHING] = true;
        
        // Calculate all the possible operations
        for(int i = 0; i < 6; i++) {
            for(int j = 0; j < 6; j++) {
                if(sat0[i] && sat1[j]) {

                    // Do not allow dual observation
                    if((i == 0 || i == 1) && (j == 0 || j == 1)) continue;

                    node* n = new node();


                    // Add the operation
                    int s_time;
                    if(time < 11) {
                        s_time = this->time + 1;
                        n->accumulated_cost = 1;

                    } else {
                        s_time = 0;
                        n->accumulated_cost = 12;
                    }
                    std::vector<int> s_sat_band = sat_band;
                    std::vector<bool> s_downlinked = downlinked;
                    std::vector<int> s_sat_remaining_battery = sat_remaining_battery;
                    std::bitset<PROBLEM_WIDTH*PROBLEM_HEIGHT> s_obs_to_do = obs_to_do;
                    action s0;
                    action s1;
                    

                    // Check i combination

                    switch (i)
                    {
                    case OBSERVE_UP:
                        s_obs_to_do[s_sat_band[0]*PROBLEM_WIDTH + time] = 0;
                        s_sat_remaining_battery[0] -= satellite_state::sat_observe_cost[0];
                        s0 = observe_up;
                        break;
                    case OBSERVE_DOWN:
                        s_obs_to_do[(s_sat_band[0] + 1)*PROBLEM_WIDTH + time] = 0;
                        s_sat_remaining_battery[0] -= satellite_state::sat_observe_cost[0];
                        s0 = observe_down;
                        break;
                    case TURN:
                        s_sat_band[0] = (s_sat_band[0] == 0 ? 1:0);
                        s_sat_remaining_battery[0] -= satellite_state::sat_turn_cost[0];
                        s0 = turn;
                        break;
                    case RECHARGE:
                        s_sat_remaining_battery[0] += satellite_state::sat_recharge_battery[0];
                        s0 = recharge;
                        break;
                    case DOWNLINK:
                        s_downlinked[0] = true;
                        s_sat_remaining_battery[0] -= satellite_state::sat_downlink_cost[0];
                        s0 = downlink;
                        break;
                    case NOTHING:
                        s0 = nothing;
                        break;
                    }


                    switch (j)
                    {
                    case OBSERVE_UP:
                        s_obs_to_do[sat_band[1] * PROBLEM_WIDTH + time] = 0;
                        s_sat_remaining_battery[1] -= satellite_state::sat_observe_cost[1];
                        s1 = observe_up;
                        break;
                    case OBSERVE_DOWN:
                        s_obs_to_do[(sat_band[1]+1) * PROBLEM_WIDTH + time] = 0;
                        s_sat_remaining_battery[1] -= satellite_state::sat_observe_cost[1];
                        s1 = observe_down;
                        break;
                    case TURN:
                        s_sat_band[1] = (sat_band[1] == 2 ? 1:2);
                        s_sat_remaining_battery[1] -= satellite_state::sat_turn_cost[1];
                        s1 = turn;
                        break;
                    case RECHARGE:
                        s_sat_remaining_battery[1] += satellite_state::sat_recharge_battery[1];
                        s1 = recharge;
                        break;
                    case DOWNLINK:
                        s_downlinked[1] = true;
                        s_sat_remaining_battery[1] -= satellite_state::sat_downlink_cost[1];
                        s1 = downlink;
                        break;
                    case NOTHING:
                        s1 = nothing;
                        break;
                    }

                    satellite_state* s_state = new satellite_state(s_time, s_sat_band, s_downlinked, s_obs_to_do, s_sat_remaining_battery);

                    n->sat0_action = s0;
                    n->sat1_action = s1;
                    n->state = s_state;

                    v.push_back(n);

                }
            }
        }

        return v;
        
    };

    bool is_goal_state() {
        return downlinked[0] && downlinked[1];
    };

    bool operator==(const satellite_state& other) const {
        bool time_eq = this->time == other.time;
        bool band_eq = this->sat_band == other.sat_band;
        bool obs_eq = this->obs_to_do == other.obs_to_do;
        bool downlinked_eq = this->downlinked == other.downlinked;
        bool remaining_battery_eq = this->sat_remaining_battery == other.sat_remaining_battery;

        return time_eq && band_eq && obs_eq && downlinked_eq && remaining_battery_eq;
    };

};

class satellite_hasher { 
public: 
    size_t operator()(const satellite_state* p) const
    { 
        std::stringstream s;
        s << p;
        std::string to_hash = s.str();
        return std::hash<std::string>()(to_hash); 
    } 
};

std::ostream &operator<<(std::ostream &os, const satellite_state &m) { 
          os << "\nSatellite 0 downlinked -> " << m.downlinked[0] 
          << "\nSatellite 1 downlinked -> " << m.downlinked[1] 
          << "\nSatellite 0 information: "
          << "\n     band:    " << m.sat_band[0]
          << "\n     battery: " << m.sat_remaining_battery[0]
          << "\nSatellite 1 information: "
          << "\n     band:    " << m.sat_band[1]
          << "\n     battery: " << m.sat_remaining_battery[1]
          << "\n Time -> " << m.time << "\n"
          ;


          return os;
    }

    /* Static parts */
    // Cost of making an observation
    std::vector<int> satellite_state::sat_observe_cost{0,0};

    // Cost of downlinking
    std::vector<int> satellite_state::sat_downlink_cost{0,0};

    // Cost of visibility band turn
    std::vector<int> satellite_state::sat_turn_cost{0,0};

    // How much does battery increase per charge hour
    std::vector<int> satellite_state::sat_recharge_battery{0,0};

    // Max battery capacity
    std::vector<int> satellite_state::sat_max_battery{0,0};


template<typename lambda>
class a_star {

    private:
    std::unordered_set<satellite_state*, satellite_hasher> visited;

    public:
        
        void search(satellite_state root, lambda heuristic){

            std::priority_queue<node* , std::vector<node*>, lambda > queue(heuristic);

            node* root_node = new node();
            root_node->accumulated_cost = 0;
            root_node->parent = nullptr;
            root_node->state = &root;
            root_node->sat0_action = nothing;
            root_node->sat1_action = nothing;

            std::vector<node*> successors = root_node->state->get_successors();
            for(node* root_successor : successors){
                root_successor->parent = root_node;
                queue.push(root_successor);
                visited.insert(root_successor->state);
            }



            // Infinite loop till a goal state is found
            int m = 1;
            while(!queue.empty()){
                m++;
                if(queue.top()->state->is_goal_state()) break;

                // Extract element from queue
                node* to_expand = queue.top();
                queue.pop();

                // Get all sucessors and append them to the fifo queue
                std::vector<node*> sucessors = to_expand->state->get_successors();
                for(node* sucessor : sucessors){
                    // Check if the node has already been visited
                    if(visited.find(sucessor->state) == visited.end()){
                        // The accumulated cost already has the cost of expanding the node.
                        // We sum the previous cost to get the new cost
                        sucessor->accumulated_cost += to_expand->accumulated_cost;
                        sucessor->parent = to_expand;
                        queue.push(sucessor);
                        visited.insert(sucessor->state);
                    }
                }
            }

            if(queue.empty()) {
                std::cout << "No solution found!!!\n";
                return;
            }

            // std::cout << *queue.top()->state;

            std::vector<std::string> action_to_string(6);
            action_to_string[OBSERVE_UP] = "Observe up  ";
            action_to_string[OBSERVE_DOWN] = "Observe down";
            action_to_string[RECHARGE] = "Recharge    ";
            action_to_string[NOTHING] = "Nothing     ";
            action_to_string[TURN] = "Turn        ";
            action_to_string[DOWNLINK] = "Downlink    ";

            // Backtrace it
            node* s = queue.top();

            std::vector<std::string> results;
            do {
                std::string sat1_act = action_to_string[s->sat0_action];
                std::string sat2_act = action_to_string[s->sat1_action];
                results.push_back("SAT1: " + sat1_act 
                + ", SAT2: " + sat2_act + "\n");
                s = s->parent;
            } while (s->parent != nullptr);

            for(int i = results.size(); i > 0; i--){
                std::cout << results.size()-i+1 << ". " << results[i-1];
            }

            std::cout << "Opened nodes: " << visited.size() << std::endl;
        }
};



int main(int argc, char **argv) {

    if(argc != 3){
        std::cerr << "The number of arguments is incorrect: " << argc;
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

    // Analyze the scheduled observations in the file
    std::bitset<PROBLEM_WIDTH*PROBLEM_HEIGHT> obs_to_do;
    for(std::string obs : observations_list) {

        // Get rid of the separators
        replace(obs, "(", "");
        replace(obs, ")", "");

        // Two strings, each one a number with a coordinate
        std::vector<std::string> coordinates = split(obs, ",");

        // Create an observation and fill information
        int band = std::stoi(coordinates[0]);
        int time = std::stoi(coordinates[1]);
        obs_to_do[PROBLEM_WIDTH*band + time] = 1;
    }

    // Extract satellite one static parameters
    replace(lines[1], " ", "");
    replace(lines[1], "SAT1:", "");
    std::vector<std::string> sat0_data = split(lines[1], ";");

    replace(lines[2], " ", "");
    replace(lines[2], "SAT2:", "");
    std::vector<std::string> sat1_data = split(lines[2], ";");


    // Define static parts of the problem and pass struct with all the data

    /* Static parts */
    // Cost of making an observation
    satellite_state::sat_observe_cost[0] = std::stoi(sat0_data[0]);
    satellite_state::sat_observe_cost[1] = std::stoi(sat1_data[0]);

    // Cost of downlinking
    satellite_state::sat_downlink_cost[0] = std::stoi(sat0_data[1]);
    satellite_state::sat_downlink_cost[1] = std::stoi(sat1_data[1]);

    // Cost of visibility band turn
    satellite_state::sat_turn_cost[0] = std::stoi(sat0_data[2]);
    satellite_state::sat_turn_cost[1] = std::stoi(sat1_data[2]);

    // How much does battery increase per charge hour
    satellite_state::sat_recharge_battery[0] = std::stoi(sat0_data[3]);
    satellite_state::sat_recharge_battery[1] = std::stoi(sat1_data[3]);

    // Max battery capacity
    satellite_state::sat_max_battery[0] = std::stoi(sat0_data[4]);
    satellite_state::sat_max_battery[1] = std::stoi(sat1_data[4]);

    std::vector<int> initial_sat_bands{0,2};
    std::vector<bool> downlinked{false, false};
    std::vector<int> sat_remaining_battery{satellite_state::sat_max_battery[0],
                                            satellite_state::sat_max_battery[1]};

    satellite_state root(0, initial_sat_bands, downlinked,
     obs_to_do, sat_remaining_battery);

    auto dj = [](node* a, node* b) { return a->accumulated_cost > b->accumulated_cost; };

    auto h1 = []( node* a, node* b ) 
    {
        float a_count = a->state->obs_to_do.count();
        float b_count = b->state->obs_to_do.count();

        return a->accumulated_cost + a_count > b->accumulated_cost + b_count;
    
    };

    std::string heuristic_argument = argv[2];

    
    if(heuristic_argument == "dj"){
        a_star<decltype(dj)> a;
        a.search(root, dj);

    } else if (heuristic_argument == "h1"){
        a_star<decltype(h1)> a;
        a.search(root, h1);
    } else {
        a_star<decltype(h1)> a;
        a.search(root, h1);
    }




    
    return 0;

}