#include <iostream>
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;

int main()
{
    json ex3 = {
        {"happy", true},
        {"pi", 3.141},
    };
    std::cout << "Hello" << std::endl;
    std::cout << ex3 << std::endl;

    std::ifstream f("example.json");
    json data = json::parse(f);
    std::cout << data << std::endl;

    // write prettified JSON to another file
    std::ofstream o("pretty.json");
    o << std::setw(4) << ex3 << std::endl;

    return 0;
}