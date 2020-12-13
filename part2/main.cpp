#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main(int argc, char **argv) {

    if(argc != 3){
        cerr << "The number of arguments is incorrect";
        return 0;
    }

    string problem_file_content;

    ifstream file(argv[1], ifstream::in);
    
    stringstream strStream;
    strStream << file.rdbuf();
    problem_file_content = strStream.str();

    size_t pos = 0;
    string lines;
    string delimiter = "\n";
    while ((pos = problem_file_content.find(delimiter)) != string::npos) {
        lines = problem_file_content.substr(0, pos);
        cout << problem_file_content << std::endl;
        problem_file_content.erase(0, pos + delimiter.length());
    }

    cout << problem_file_content << std::endl;




}

