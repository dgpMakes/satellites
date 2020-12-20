class satellite_state;
struct node;

std::vector<std::string> split(std::string s, std::string delimiter);
void replace(std::string & data, std::string search, std::string replace);
int get_index(std::vector<int> v, int i);
void remove_vector_element(std::vector<int> *v, int k);