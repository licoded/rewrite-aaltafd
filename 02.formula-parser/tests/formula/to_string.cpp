#include "formula/aalta_formula.h"
#include <cassert>
#include <iostream>


#define MAXN 100000
char in[MAXN];

using namespace aalta;

int main()
{
    std::vector<const char *> str = {
        "a",
        "!a",
    };
    auto t = aalta_formula::TAIL();
    for(const auto it : str)
    {
        auto temp = aalta_formula(it).unique();
        std::cout << temp->to_string() << std::endl;
    }
    return 0;
}