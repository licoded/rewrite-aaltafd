#include <iostream>
#include <cassert>
#include <map>

int main()
{
    std::map<int, double> m;
    m.insert({1, 1});
    assert(m.at(1) == 1);
    m.insert({1, 2});
    assert(m.at(1) == 2);
    /**
     * Insert will not take effect if the key already exists
     * While `m[key] = value` will take effect.
    */

    return 0;
}