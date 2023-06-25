#include <string>
#include <iostream>
#include <cassert>

class Test {
private:
    static int sa;
    int a_;
public:
    Test() : a_(0) {}
    Test(int a) : a_(a) {}
    int getA() const { 
        // a_++;
        sa++;
        return a_;
    }
    static int getSA() {
        return sa;
    }
};

int Test::sa = 0;

int main() {
    Test t;
    std::cout << Test::getSA() << std::endl;
    std::cout << t.getA() << std::endl;
    std::cout << Test::getSA() << std::endl;
    return 0;
}