#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <queue>
#include "main.h"
#include <bitset>
#include <chrono>
#include <algorithm>

#define OBSERVE_UP 0
#define OBSERVE_DOWN 1
#define RECHARGE 2
#define NOTHING 3
#define DOWNLINK 4
#define TURN 5

#define STATUS_NOT_MEASURED 0
#define STATUS_SAT_0_MEASURED 1
#define STATUS_SAT_1_MEASURED 2
#define STATUS_DOWNLINKED 3


#define PROBLEM_HEIGHT 4
#define PROBLEM_WIDTH 12

// What every seach problem shall implement for the search algorithm to use
class search_problem
{
public:
    std::vector<search_problem> *get_successors();
    bool is_goal_state();
    ~search_problem();
};

// The only thing that has a cost is passing time
enum act
{
    observe_up = OBSERVE_UP,
    observe_down = OBSERVE_DOWN,
    recharge = RECHARGE,
    turn = TURN,
    downlink = DOWNLINK,
    nothing = NOTHING
};

struct action {

    act executed_action;
    int action_data;

    // Set default values
    action()
    {
    executed_action=nothing;
    action_data=-1;
    }
};


struct node
{
    int accumulated_cost;
    satellite_state *state;
    node *parent;
};

struct observation
{
    int time;
    int band;
};

struct results {
    bool solution_found;
    std::vector<std::string> results;
    int steps;
    int expanded_nodes;
    int overall_cost;
};

auto comp = [](node *lhs, node *rhs) {
    return lhs->accumulated_cost < rhs->accumulated_cost;
};

/*
In this problem we are assuming there are only 12 hours on a day
There are only 4 bands and 3 possitions for the two considered satellites.
*/

class satellite_state
{
    /* Dynamic variables */
    // Observations done

public:
    int time;
    int associated_cost;
    action sat_1_action;
    action sat_0_action;
    std::vector<std::bitset<2>> measurement_status;
    std::vector<int> measurement_coordinates;

    std::vector<int> sat_band;
    std::vector<int> sat_remaining_battery;

    satellite_state(int time, std::vector<int> sat_band,
                    std::vector<int> measurement_coordinates, std::vector<std::bitset<2>> measurement_status, 
                    std::vector<int> sat_remaining_battery, action sat_0_action, 
                    action sat_1_action, int associated_cost)
    {        
        this->measurement_status = measurement_status;
        this->measurement_coordinates = measurement_coordinates;

        this->time = time;
        this->sat_band = sat_band;
        this->sat_remaining_battery = sat_remaining_battery;
        this->associated_cost = associated_cost;
        this->sat_0_action = sat_0_action;
        this->sat_1_action = sat_1_action;
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

    std::vector<satellite_state *> get_successors()
    {
        std::vector<satellite_state *> v;
        /* Operations definition */
        // Definir un array de booleanos, indicando si un satélite puede hacer una acción.
        std::vector<bool> sat0(6, false); // Observe up, Observe down, turn, recharge, downlink, do_nothing
        std::vector<bool> sat1(6, false); // Observe up, Observe down, turn, recharge, downlink, do_nothing

        // Check if satellites can observe up
        sat0[OBSERVE_UP] = get_index(measurement_coordinates, this->sat_band[0] * PROBLEM_WIDTH + (time%12)) != -1 && (sat_remaining_battery[0] >= sat_observe_cost[0]) == 1;
        sat1[OBSERVE_UP] = get_index(measurement_coordinates, this->sat_band[1] * PROBLEM_WIDTH + (time%12)) != -1 && (sat_remaining_battery[1] >= sat_observe_cost[1]) == 1;

        // Check observe down
        sat0[OBSERVE_DOWN] = get_index(measurement_coordinates, (this->sat_band[0] + 1) * PROBLEM_WIDTH + (time%12)) != -1 && (sat_remaining_battery[0] >= sat_observe_cost[0]) == 1;
        sat1[OBSERVE_DOWN] = get_index(measurement_coordinates, (this->sat_band[1] + 1) * PROBLEM_WIDTH + (time%12)) != -1 && (sat_remaining_battery[1] >= sat_observe_cost[1]) == 1;

        //Check if satellites can turn
        sat0[TURN] = sat_remaining_battery[0] >= sat_turn_cost[0];
        sat1[TURN] = sat_remaining_battery[1] >= sat_turn_cost[1];

        // Check if satellites can recharge
        sat0[RECHARGE] = sat_remaining_battery[0] < sat_max_battery[0];
        sat1[RECHARGE] = sat_remaining_battery[1] < sat_max_battery[1];

        // Check if each satellite can downlink
        bool can_sat_0_downlink = false;
        bool can_sat_1_downlink = false;

        // Check for each satellite if there is any measurement done and ready to downlink
        for(int i = 0; i < measurement_status.size(); i++) {
            can_sat_0_downlink = can_sat_0_downlink | (measurement_status[i] == STATUS_SAT_0_MEASURED);
            can_sat_1_downlink = can_sat_1_downlink | (measurement_status[i] == STATUS_SAT_1_MEASURED);
        }

        sat0[DOWNLINK] = can_sat_0_downlink && this->sat_remaining_battery[0] >= sat_downlink_cost[0];
        sat1[DOWNLINK] = can_sat_1_downlink && this->sat_remaining_battery[1] >= sat_downlink_cost[1];
        
        // Satellites can always do nothing
        sat0[NOTHING] = true;
        sat1[NOTHING] = true;

        // Calculate all the possible operations
        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                if (sat0[i] && sat1[j])
                {

                    // Do not allow satellites make the same observation
                    // If these conditions meet, there could be a conflict.
                    if ((i == OBSERVE_DOWN || i == OBSERVE_UP) && (j == OBSERVE_DOWN || j == OBSERVE_UP)){
                        int sat_0_band_to_observe = sat_band[0] + (i == OBSERVE_UP ? 0:1);
                        int sat_1_band_to_observe = sat_band[1] + (j == OBSERVE_UP ? 0:1);

                        // If they intend to make the same observation, it is illegal.
                        if (sat_0_band_to_observe == sat_1_band_to_observe){
                            continue;
                        }
                    }
                        
                    // Add the operation
                    int child_time;
                    child_time = this->time + 1;

                    action child_sat_0_action;
                    action child_sat_1_action;
                    std::vector<int> child_sat_band = sat_band;
                    std::vector<int> child_sat_remaining_battery = sat_remaining_battery;

                    std::vector<std::bitset<2>> child_measurement_status = measurement_status;
                    std::vector<int> child_measurement_coordinates = measurement_coordinates;

                    int child_associated_cost = 0;


                    // Check satellite zero combinations
                    int index = -1;
                    switch (i)
                    {
                    case OBSERVE_UP:
                        index = get_index(child_measurement_coordinates, child_sat_band[0] * PROBLEM_WIDTH + (time%12));
                        child_measurement_status[index] = STATUS_SAT_0_MEASURED;
                        child_sat_remaining_battery[0] -= satellite_state::sat_observe_cost[0];
                        child_sat_0_action.executed_action = observe_up;
                        child_sat_0_action.action_data = index;
                        child_associated_cost += satellite_state::sat_observe_cost[0];
                        break;
                    case OBSERVE_DOWN:
                        index = get_index(child_measurement_coordinates, (child_sat_band[0] + 1) * PROBLEM_WIDTH + (time%12));
                        child_measurement_status[index] = STATUS_SAT_0_MEASURED;
                        child_sat_remaining_battery[0] -= satellite_state::sat_observe_cost[0];
                        child_sat_0_action.executed_action = observe_down;
                        child_sat_0_action.action_data = index;
                        child_associated_cost += satellite_state::sat_observe_cost[0];
                        break;
                    case TURN:
                        child_sat_band[0] = (child_sat_band[0] == 0 ? 1 : 0);
                        child_sat_remaining_battery[0] -= satellite_state::sat_turn_cost[0];
                        child_sat_0_action.executed_action = turn;
                        child_associated_cost += satellite_state::sat_turn_cost[0];
                        break;
                    case RECHARGE:
                        child_sat_remaining_battery[0] += satellite_state::sat_recharge_battery[0];
                        child_sat_0_action.executed_action = recharge;
                        break;
                    case DOWNLINK:
                        for(int d = 0; d < child_measurement_status.size(); d++){
                            if (child_measurement_status[d] == STATUS_SAT_0_MEASURED){
                                child_measurement_status[d] = STATUS_DOWNLINKED;
                                break;
                            }
                        }                        
                        child_sat_remaining_battery[0] -= satellite_state::sat_downlink_cost[0];
                        child_associated_cost += satellite_state::sat_downlink_cost[0];
                        child_sat_0_action.executed_action = downlink;
                        child_sat_0_action.action_data = index;
                        break;
                    case NOTHING:
                        child_sat_0_action.executed_action = nothing;
                        break;
                    }

                    switch (j)
                    {
                    case OBSERVE_UP:
                        index = get_index(child_measurement_coordinates, sat_band[1] * PROBLEM_WIDTH + (time%12));
                        child_measurement_status[index] = STATUS_SAT_1_MEASURED;
                        child_sat_remaining_battery[1] -= satellite_state::sat_observe_cost[1];
                        child_associated_cost += satellite_state::sat_observe_cost[1];
                        child_sat_1_action.executed_action = observe_up;
                        break;
                    case OBSERVE_DOWN:
                        index = get_index(child_measurement_coordinates, (sat_band[1] + 1) * PROBLEM_WIDTH + (time%12));
                        child_measurement_status[index] = STATUS_SAT_1_MEASURED;
                        child_sat_remaining_battery[1] -= satellite_state::sat_observe_cost[1];
                        child_associated_cost += satellite_state::sat_observe_cost[1];
                        child_sat_1_action.executed_action = observe_down;
                        break;
                    case TURN:
                        child_sat_band[1] = (sat_band[1] == 2 ? 1 : 2);
                        child_sat_remaining_battery[1] -= satellite_state::sat_turn_cost[1];
                        child_associated_cost += satellite_state::sat_turn_cost[1];
                        child_sat_1_action.executed_action = turn;
                        break;
                    case RECHARGE:
                        child_sat_remaining_battery[1] += satellite_state::sat_recharge_battery[1];
                        child_sat_1_action.executed_action = recharge;
                        break;
                    case DOWNLINK:
                        for(int d = 0; d < child_measurement_status.size(); d++){
                            if (child_measurement_status[d] == STATUS_SAT_1_MEASURED){
                                child_measurement_status[d] = STATUS_DOWNLINKED;
                                break;
                            }
                        }  
                        child_sat_remaining_battery[1] -= satellite_state::sat_downlink_cost[1];
                        child_associated_cost += satellite_state::sat_downlink_cost[1];
                        child_sat_1_action.executed_action = downlink;
                        break;
                    case NOTHING:
                        child_sat_1_action.executed_action = nothing;
                        break;
                    }


                    satellite_state *child_state = new satellite_state(child_time, child_sat_band, 
                    child_measurement_coordinates, child_measurement_status, child_sat_remaining_battery, child_sat_0_action, 
                    child_sat_1_action, child_associated_cost);

                    v.push_back(child_state);
                }
            }
        }

        return v;
    };

    bool is_goal_state()
    {
        // Check that all have been downlinked
        return std::count(measurement_status.begin(), measurement_status.end(), 3) == measurement_status.size();
    };

    bool operator==(const satellite_state &other) const
    {
        bool time_eq = this->time == other.time;
        bool band_eq = this->sat_band == other.sat_band;
        bool status_eq = this->measurement_status == other.measurement_status;
        bool coord_eq = this->measurement_coordinates == other.measurement_coordinates;
        bool cost_eq = this->associated_cost == other.associated_cost;
        bool a0_eq = this->sat_0_action.action_data == other.sat_0_action.action_data && 
        this->sat_0_action.executed_action == other.sat_0_action.executed_action;
        bool a1_eq = this->sat_1_action.action_data == other.sat_1_action.action_data && 
        this->sat_1_action.executed_action == other.sat_1_action.executed_action;
        bool remaining_battery_eq = this->sat_remaining_battery == other.sat_remaining_battery;

        return time_eq && band_eq && status_eq && coord_eq && cost_eq && a0_eq && a1_eq && remaining_battery_eq;
    };
};

class satellite_hasher
{
public:
    size_t operator()(const satellite_state *p) const
    {
        std::stringstream s;
        s << p;
        std::string to_hash = s.str();
        return std::hash<std::string>()(to_hash);
    }
};


/* Static parts */
// Cost of making an observation
std::vector<int> satellite_state::sat_observe_cost{0, 0};

// Cost of downlinking
std::vector<int> satellite_state::sat_downlink_cost{0, 0};

// Cost of visibility band turn
std::vector<int> satellite_state::sat_turn_cost{0, 0};

// How much does battery increase per charge hour
std::vector<int> satellite_state::sat_recharge_battery{0, 0};

// Max battery capacity
std::vector<int> satellite_state::sat_max_battery{0, 0};

template <typename lambda>
class a_star
{

private:
    std::unordered_set<satellite_state *, satellite_hasher> visited;

public:
    results search(satellite_state root, lambda heuristic)
    {

        std::priority_queue<node *, std::vector<node *>, lambda> queue(heuristic);

        node *root_node = new node();
        root_node->accumulated_cost = 0;
        root_node->parent = nullptr;
        root_node->state = &root;

        std::vector<satellite_state *> root_successors = root_node->state->get_successors();
        float avg_exp = root_successors.size();

        for (satellite_state* root_successor : root_successors)
        {
            node *root_successors_node = new node();
            root_successors_node->parent = root_node;
            root_successors_node->state = root_successor;
            root_successors_node->accumulated_cost = root_successor->associated_cost;
            queue.push(root_successors_node);
            visited.insert(root_successor);
        }

        // Infinite loop till a goal state is found
        int steps = 1;
        while (!queue.empty())
        {
            steps++;
            if (queue.top()->state->is_goal_state())
                break;

            // Extract element from queue
            node *parent_node = queue.top();
            queue.pop();

            // Get all sucessors and append them to the fifo queue
            std::vector<satellite_state *> successors = parent_node->state->get_successors(); // ERROR
            avg_exp = (avg_exp + successors.size()) / 2;
            for (satellite_state *successor : successors)
            {
                // Check if the node has already been visited
                if (visited.find(successor) == visited.end())
                {
                    node* n = new node();
                    // The accumulated cost already has the cost of expanding the node.
                    // We sum the previous cost to get the new cost
                    n->accumulated_cost = parent_node->accumulated_cost + successor->associated_cost;
                    n->parent = parent_node;
                    n->state = successor;
                    queue.push(n);
                    visited.insert(successor);
                }
            }
        }

        if (queue.empty())
        {
            struct results nothing;
            nothing.solution_found = false;
            return nothing;
        }

        // std::cout << *queue.top()->state;

        std::vector<std::string> action_to_string(6);
        action_to_string[OBSERVE_UP] = "Measure";
        action_to_string[OBSERVE_DOWN] = "Measure";
        action_to_string[RECHARGE] = "Charge";
        action_to_string[NOTHING] = "IDLE";
        action_to_string[TURN] = "Turn";
        action_to_string[DOWNLINK] = "Downlink";

        // Backtrace it
        node *s = queue.top();

        std::vector<std::string> results;
        do
        {
            action a_0;
            a_0.executed_action = s->state->sat_0_action.executed_action;
            action a_1;
            a_1.executed_action = s->state->sat_0_action.executed_action;

            std::string sat_0_data = "";
            std::string sat_1_data = "";

            if(a_0.executed_action == OBSERVE_DOWN || a_0.executed_action == OBSERVE_UP || a_0.executed_action == DOWNLINK)
                sat_0_data = "O" + std::to_string(a_0.action_data);

            if(a_1.executed_action == OBSERVE_DOWN || a_1.executed_action == OBSERVE_UP || a_1.executed_action == DOWNLINK)
                sat_1_data = "O" + std::to_string(a_1.action_data);

            std::string sat_0_act = action_to_string[s->state->sat_0_action.executed_action];
            std::string sat_1_act = action_to_string[s->state->sat_1_action.executed_action];
            
            results.push_back("SAT1: " + sat_0_act + " " + sat_0_data + ", SAT2: " + sat_1_act + " " + sat_1_data + "\n");
            s = s->parent;
        } while (s->parent != nullptr);

        struct results r;
        r.solution_found = true;
        r.steps = steps;
        r.expanded_nodes = visited.size();
        r.results = results;
        r.overall_cost = queue.top()->accumulated_cost;

        return r;
    }
};

int main(int argc, char **argv)
{

    if(argc == 2 && ((std::string)argv[1] == "--help" || (std::string)argv[1] == "-h")){
        std::cout << "Use " << argv[0] << " problem.prob heuristic\n"
        << "Valid heuristics: dj, h1, h2.\n";
        return 0;
    }

    if (argc != 3)
    {
        std::cerr << "The number of arguments is incorrect: " << argc;
        return -1;
    }

    std::string problem_file_content;

    std::ifstream file(argv[1], std::ifstream::in);

    // Parsing the problem's data
    std::stringstream str_stream;
    str_stream << file.rdbuf();
    problem_file_content = str_stream.str();

    // Get each line
    std::vector<std::string> lines = split(problem_file_content, "\n");

    // Get observations list
    replace(lines[0], " ", "");
    replace(lines[0], "OBS:", "");
    std::vector<std::string> observations_list = split(lines[0], ";");

    // Analyze the scheduled observations in the file
    std::vector<int> measurement_coordinates;
    for (std::string obs : observations_list)
    {

        // Get rid of the separators
        replace(obs, "(", "");
        replace(obs, ")", "");

        // Two strings, each one a number with a coordinate
        std::vector<std::string> coordinates = split(obs, ",");

        // Create an observation and fill information
        int band = std::stoi(coordinates[0]);
        int time = std::stoi(coordinates[1]);
        measurement_coordinates.push_back(PROBLEM_WIDTH * band + time);
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

    std::vector<int> initial_sat_bands{0, 2};
    std::vector<int> sat_remaining_battery{satellite_state::sat_max_battery[0],
                                           satellite_state::sat_max_battery[1]};


    int obs_count = measurement_coordinates.size();
    std::vector<std::bitset<2>> measurement_status(obs_count, 0);

    action initial_sat_0;
    action initial_sat_1;

    satellite_state root(0, initial_sat_bands, measurement_coordinates, 
                         measurement_status, sat_remaining_battery, initial_sat_0, initial_sat_1, 0);

    auto dj = [](node *a, node *b) {return a->accumulated_cost > b->accumulated_cost;};

    auto h1 = [](node *a, node *b) {

        int left_observations_a = std::count(a->state->measurement_status.begin(), a->state->measurement_status.end(), 0);
        int left_downlinks_a = a->state->measurement_status.size()
         - std::count(a->state->measurement_status.begin(), a->state->measurement_status.end(), 3);


        int left_observations_b = std::count(b->state->measurement_status.begin(), b->state->measurement_status.end(), 0);
        int left_downlinks_b = b->state->measurement_status.size()
         - std::count(b->state->measurement_status.begin(), b->state->measurement_status.end(), 3);

        int obs_cost = std::min(satellite_state::sat_observe_cost[0], satellite_state::sat_observe_cost[1]);
        int dl_cost = std::min(satellite_state::sat_downlink_cost[0], satellite_state::sat_downlink_cost[1]);

        int heuristic_a = left_observations_a * obs_cost + left_downlinks_a * dl_cost;
        int heuristic_b = left_observations_b * obs_cost + left_downlinks_b * dl_cost;
        return a->accumulated_cost + heuristic_a > b->accumulated_cost + heuristic_b;
    };

    auto h2 = [](node *a, node *b) {

        int left_observations_a = std::count(a->state->measurement_status.begin(), a->state->measurement_status.end(), 0);
        int left_downlinks_a = a->state->measurement_status.size()
         - std::count(a->state->measurement_status.begin(), a->state->measurement_status.end(), 3);


        int left_observations_b = std::count(b->state->measurement_status.begin(), b->state->measurement_status.end(), 0);
        int left_downlinks_b = b->state->measurement_status.size()
         - std::count(b->state->measurement_status.begin(), b->state->measurement_status.end(), 3);

        int obs_cost = std::min(satellite_state::sat_observe_cost[0], satellite_state::sat_observe_cost[1]);
        int dl_cost = std::min(satellite_state::sat_downlink_cost[0], satellite_state::sat_downlink_cost[1]);

        // int total_difference_height = 0;

        // for(int i = 0; i<obs_count;i++){
        //     total_difference_height += (abs(obs[i].band - sat_band[0])+abs(obs[i].band - sat_band[1]));
        // }

        int heuristic_a = left_observations_a * obs_cost + left_downlinks_a * dl_cost;
        int heuristic_b = left_observations_b * obs_cost + left_downlinks_b * dl_cost;
        return a->accumulated_cost + heuristic_a > b->accumulated_cost + heuristic_b;
    };

    std::string heuristic_argument = argv[2];

    results r;

    auto start_time = std::chrono::high_resolution_clock::now();

    if (heuristic_argument == "dj")
    {
        a_star<decltype(dj)> a;
        r = a.search(root, dj);
    }
    else if (heuristic_argument == "h1")
    {
        a_star<decltype(h1)> a;
        r = a.search(root, h1);
    }
    else if (heuristic_argument == "h2")
    {
        a_star<decltype(h2)> a;
        r = a.search(root, h2);
    }
    else 
    {
        a_star<decltype(h1)> a;
        r = a.search(root, h1);  
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    std::stringstream sol;

    for (int i = r.results.size(); i > 0; i--)
    {
        sol << r.results.size() - i + 1 << ". " << r.results[i - 1];
    }

    std::stringstream stats;

    float time = (float)std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count()/1000000000;

    stats << "Overall time: " << time << std::endl;
    stats << "Overall cost: " << r.overall_cost << std::endl;   
    stats << "# Steps: " << r.steps << std::endl;
    stats << "# Expansions: " << r.expanded_nodes << std::endl;

    std::cout << sol.str();
    std::cout << stats.str();

    // Open the statistics file and write results to i
    std::ofstream out_file((std::string)argv[1] + ".output", std::ofstream::out);

    out_file << sol.str();
    out_file << stats.str();
    out_file.close();

    return 0;
}