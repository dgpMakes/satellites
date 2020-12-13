#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

vector<string> split(string s, string delimiter);
void replace(string & data, string search, string replace);

int main(int argc, char **argv) {

    if(argc != 3){
        cerr << "The number of arguments is incorrect";
        return 0;
    }

    string problem_file_content;

    ifstream file(argv[1], ifstream::in);
    
    //parsing the problem's data
    stringstream strStream;
    strStream << file.rdbuf();
    problem_file_content = strStream.str();

    vector<string> lines = split(problem_file_content, "\n");
    replace(lines[0], " ", "");
    replace(lines[0], "OBS:", "");
    vector<string> observations = split(lines[0], ";");

    replace(lines[1], " ", "");
    replace(lines[1], "SAT1:", "");
    vector<string> sat1_data = split(lines[1], ";");

    replace(lines[2], " ", "");
    replace(lines[2], "SAT2:", "");
    vector<string> sat2_data = split(lines[2], ";");
    
    return 0;

}

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