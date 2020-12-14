#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

vector<string> split(string s, string delimiter);
void replace(string & data, string search, string replace);
struct observation {
    int time;
    int band;
};


// What every seach problem shall implement for the search algorithm to use
class search_problem {
    public:
        static search_problem* get_initial_state();
        static search_problem* get_final_state();
        virtual vector<search_problem> get_sucessors() = 0;
        virtual bool is_goal_state() = 0;
        virtual ~search_problem() = 0;
};




/*
In this problem we are assuming there are only 12 hours on a day
There are only 4 bands and three possitions for the two considered satellites.
*/

class satellite_problem : public search_problem {
    
    satellite_problem(int time, int sat1_band, int sat2_band, vector<bool> is_obs_done, bool downlinked) {
        this->is_obs_done = is_obs_done;
        
        if(time >= 0 && time < 13){
            this->time = time;
        }
        
        this->sat1_band = sat1_band;
        this->sat2_band = sat2_band;
        this->downlinked = downlinked;
    }

    public:
    
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

    static vector<observation> obs_to_do;

    static satellite_problem* get_initial_state(){
        vector<bool> v (obs_to_do.size(), false);
        return new satellite_problem(0, 1, 3, v, false);
    };

    // static satellite_problem* get_final_state(){
    //     return satellite_problem(0, 1, 3, new vector<bool> (obs_to_do.size(), true), true);
    // };


    vector<search_problem> get_sucessors() {
        vector<search_problem> v;
        return v;
    };

    bool is_goal_state() {
        return downlinked;
    };

    bool operator==(const satellite_problem& other) {
        bool time_eq = this->time == other.time;
        bool sat1_band_eq = this->sat1_band == other.sat1_band;
        bool sat2_band_eq = this->sat2_band == other.sat2_band;
        bool downlinked_eq = this->downlinked == other.downlinked;
        bool done_obs_eq = this->is_obs_done == other.is_obs_done;
        return time_eq && sat1_band_eq && sat2_band_eq && downlinked_eq && done_obs_eq;
    };



    /* Dynamic variables */
    // Observations done
    private:
    vector<bool> is_obs_done;
    int time;
    int sat1_band;
    int sat2_band;
    bool downlinked;

};



class search_algorithm {



};

class a_star : public search_algorithm {

};













// Helper functions

vector<string> split(string s, string delimiter) {
    size_t pos = 0;
    string sub;
    vector<string> extracted_lines;

    while ((pos = s.find(delimiter)) != string::npos) {
        sub = s.substr(0, pos);
        extracted_lines.push_back(sub);
        s.erase(0, pos + delimiter.length());
    }
    // Add the last part
    extracted_lines.push_back(s);

    return extracted_lines;
}

void replace(string & data, string search, string replace)
{
    // Get the first occurrence
    size_t pos = data.find(search);
    // Repeat till end is reached
    while(pos != string::npos)
    {
        // Replace this occurrence of Sub String
        data.replace(pos, search.size(), replace);
        // Get the next occurrence from the current position
        pos = data.find(search, pos + replace.size());
    }
}



int main(int argc, char **argv) {

    if(argc != 3){
        cerr << "The number of arguments is incorrect";
        return 0;
    }

    string problem_file_content;

    ifstream file(argv[1], ifstream::in);
    
    // Parsing the problem's data
    stringstream strStream;
    strStream << file.rdbuf();
    problem_file_content = strStream.str();

    // Get each line
    vector<string> lines = split(problem_file_content, "\n");


    // Get observations list
    replace(lines[0], " ", "");
    replace(lines[0], "OBS:", "");
    vector<string> observations_list = split(lines[0], ";");

    // Transform each observation into a observation struct
    vector<observation> observations;
    for(string obs : observations_list) {

        // Get rid of the separators
        replace(obs, "(", "");
        replace(obs, ")", "");

        // Two strings, each one a number with a coordinate
        vector<string> coordinates = split(obs, ",");

        // Create an observation and fill information
        observation o;
        o.time = stoi(coordinates[0]);
        o.band = stoi(coordinates[1]);
        observations.push_back(o);
    }

    // Extract satellite one static parameters
    replace(lines[1], " ", "");
    replace(lines[1], "SAT1:", "");
    vector<string> sat1_data = split(lines[1], ";");

    

    replace(lines[2], " ", "");
    replace(lines[2], "SAT2:", "");
    vector<string> sat2_data = split(lines[2], ";");


    // Define static parts of the problem and pass struct with all the data

    /* Static parts */
    // Cost of making an observation
    satellite_problem::sat1_observe_cost = stoi(sat1_data[0]);
    satellite_problem::sat2_observe_cost = stoi(sat2_data[0]);

    // Cost of downlinking
    satellite_problem::sat1_downlink_cost = stoi(sat1_data[1]);
    satellite_problem::sat2_downlink_cost = stoi(sat2_data[1]);

    // Cost of visibility band turn
    satellite_problem::sat1_turn_cost = stoi(sat1_data[2]);
    satellite_problem::sat2_turn_cost = stoi(sat2_data[2]);

    // How much does battery increase per charge hour
    satellite_problem::sat1_battery_recharge = stoi(sat1_data[3]);
    satellite_problem::sat2_battery_recharge = stoi(sat2_data[3]);

    // Max battery capacity
    satellite_problem::sat1_max_battery = stoi(sat1_data[4]);
    satellite_problem::sat2_max_battery = stoi(sat2_data[4]);

    /* Static parts */
    // Observations to do
    satellite_problem::obs_to_do = observations;
    
    return 0;

}