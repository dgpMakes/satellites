#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <queue>
#include "main.h"


class satellite_hasher { 
public: 
    size_t operator()(const satellite_state& p) const
    { 
        std::stringstream s;
        s << &p;
        std::string to_hash = s.str();
        return std::hash<std::string>()(to_hash); 
    } 
};


// // What every seach problem shall implement for the search algorithm to use
class search_problem {
    public:
        std::vector<search_problem>* get_successors();
        bool is_goal_state();
        ~search_problem();
};

// The only thing that has a cost is passing time
enum action{observe_up, observe_down, recharge, turn, downlink, nothing};

struct node {
    action sat0_action;
    action sat0_action;
    int accumulated_cost;
    satellite_state* state;
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
        std::vector<std::vector<bool>> obs_to_do;
        int time;
        std::vector<int> sat_band;
        std::vector<bool> downlinked;
        std::vector<int> sat_remaining_battery;
        
    satellite_state(int time, std::vector<int> sat_band, std::vector<bool> downlinked,
     std::vector<std::vector<bool>> obs_to_do, std::vector<int> sat_remaining_battery) 
    {
        this->obs_to_do = obs_to_do;
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

        
    

    // static satellite_state* get_final_state(){
    //     return satellite_state(0, 1, 3, new vector<bool> (obs_to_do.size(), true), true);
    // };


    std::vector<node> get_successors() {
        std::vector<node> v;
        /* Operations definition */
        // Definir un array de booleanos, indicando si un satélite puede hacer una acción.
        std::vector<bool> sat0(6, false); // Observe up, Observe down, turn, recharge, downlink, do_nothing
        std::vector<bool> sat1(6, false); // Observe up, Observe down, turn, recharge, downlink, do_nothing
        
        // Check if satellites can observe up or down
        sat0[0] = obs_to_do[this->sat_band[0]][time];
        sat0[1] = obs_to_do[this->sat_band[0] + 1][time];
        sat1[0] = obs_to_do[this->sat_band[1]][time];
        sat1[1] = obs_to_do[this->sat_band[1] + 1][time];

        //Check if satellites can turn
        sat0[2] = this->sat_remaining_battery[0] > sat_turn_cost[0];
        sat0[2] = this->sat_remaining_battery[1] > sat_turn_cost[1];
          
        // Check if satellites can recharge        
        sat0[3] = this->sat_remaining_battery[0] < sat_max_battery[0];
        sat1[3] = this->sat_remaining_battery[1] < sat_max_battery[1];

        // Check if satellites can downlink       
        sat0[4] = this->sat_remaining_battery[0] > sat_downlink_cost[0];
        sat1[4] = this->sat_remaining_battery[1] > sat_downlink_cost[0];

        //Satellites can always do nothing
        sat0[5] = true;
        sat1[5] = true;
        
        for(int i = 0; i < 5; i++) {
            for(int j = 0; j < 5; j++) {
                if(sat0[i] && sat1[j]) {
                    // Add the operation
                    int s_time = this->time + 1;
                    std::vector<int> s_sat_band = sat_band;
                    std::vector<bool> s_downlinked = downlinked;
                    std::vector<int> s_sat_remaining_battery = sat_remaining_battery;
                    std::vector<std::vector<bool>> s_obs_to_do = obs_to_do;
                    action s1 = nothing;
                    action s2 = nothing;
                    
                    // Observe up
                    if(i == 0) {
                        obs_to_do[sat_band[1]][time] = false;
                        s_sat_remaining_battery[0] -= satellite_state::sat_turn_cost[0];
                    }

                    if(j == 0) {
                        obs_to_do[sat_band[1]][time] = false;
                        s_sat_remaining_battery[1] -= satellite_state::sat_turn_cost[1];
                    }

                    // Observe down
                    if(i == 1) {
                        obs_to_do[sat_band[1] + 1][time] = false;
                        s_sat_remaining_battery[0] -= satellite_state::sat_turn_cost[0];
                    }

                    if(j == 1) {
                        obs_to_do[sat_band[1] + 1][time] = false;
                        s_sat_remaining_battery[1] -= satellite_state::sat_turn_cost[1];
                    }

                    // Change band
                    if(i == 2){
                        s_sat_band[0] = (sat_band[0] == 0 ? 1:0);
                        s_sat_remaining_battery[0] -= satellite_state::sat_turn_cost[0];
                        s1 = turn;
                    }
    
                    if(j == 2){
                        s_sat_band[1] = (sat_band[1] == 2 ? 1:2);
                        s_sat_remaining_battery[1] -= satellite_state::sat_turn_cost[1];
                        s2 = turn;
                    }
                    
                    // Recharge action
                    if(i == 3){
                        s_sat_remaining_battery[0] += satellite_state::sat_recharge_battery[0];
                        s1 = recharge;
                    }
                    if(j == 3) {
                        s_sat_remaining_battery[1] += satellite_state::sat_recharge_battery[1];
                        s2 = recharge;
                    }
                    
                    // Downlink action
                    if(i == 4){
                        s_downlinked[0] = true;
                        s_sat_remaining_battery[0] -= satellite_state::sat_downlink_cost[0];
                        s1 = downlink;
                    }
                    if(j == 4){
                        s_downlinked[1] = true;
                        s_sat_remaining_battery[1] -= satellite_state::sat_downlink_cost[1];
                        s2 = downlink;
                    } 

                    //If nothing is done, time passes. 

                    satellite_state s_state(s_time, s_sat_band, s_downlinked, s_obs_to_do, s_sat_remaining_battery);

                    node n;
                    n.sat0_action = s1;
                    n.sat0_action = s2;
                    n.parent = this;
                    n.state = &s_state;

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

std::ostream &operator<<(std::ostream &os, const satellite_state &m) { 
          os << (m.downlinked[0] ? "yes":"no") << m.time << 
          " sat0band:" << std::to_string(m.sat_band[0]) << " sat1band:" << 
          std::to_string(m.sat_band[1]); 

          return os;
    }


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









// class search_algorithm {



// };
class a_star {

    public:
        
        void search(satellite_state root){
            node n;
            n.accumulated_cost = 0;
            n.parent = nullptr;
            n.state = &root;        
            queue.push(n);


            // Infinite loop till a goal state is found
            while(!queue.front().state->is_goal_state() && !queue.empty()){
            
                // Extract element from queue
                node to_expand = queue.front();
                queue.pop();

                // Get all sucessors and append them to the fifo queue
                std::vector<node> sucessors = to_expand.state->get_successors();
                for(node sucessor : sucessors){
                    // Check if the node has already been visited
                    if(visited.find(*sucessor.state) != visited.end()){
                        sucessor.accumulated_cost = to_expand.accumulated_cost + 1;
                        queue.push(sucessor);
                        visited.insert(*sucessor.state);
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
        std::unordered_set<satellite_state, satellite_hasher> visited;






};