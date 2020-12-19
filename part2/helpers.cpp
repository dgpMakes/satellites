#include <vector>
#include <iostream>

std::vector<std::string> split(std::string s, std::string delimiter)
{
    std::size_t pos = 0;
    std::string sub;
    std::vector<std::string> extracted_lines;

    while ((pos = s.find(delimiter)) != std::string::npos)
    {
        sub = s.substr(0, pos);
        extracted_lines.push_back(sub);
        s.erase(0, pos + delimiter.length());
    }
    // Add the last part
    extracted_lines.push_back(s);

    return extracted_lines;
}

int get_index(std::vector<int> v, int i)
{
    auto item = std::find(v.begin(), v.end(), i);
    return (item != v.end() ? item-v.begin():-1);
}

void replace(std::string &data, std::string search, std::string replace)
{
    // Get the first occurrence
    size_t pos = data.find(search);
    // Repeat till end is reached
    while (pos != std::string::npos)
    {
        // Replace this occurrence of Sub String
        data.replace(pos, search.size(), replace);
        // Get the next occurrence from the current position
        pos = data.find(search, pos + replace.size());
    }
}