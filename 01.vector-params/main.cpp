#include <vector>

// int sum(std::vector<int> &v)
// initial value of reference to non-const must be an lvalue
int sum(const std::vector<int> &v)
{
    int s = 0;
    for (int i : v)
        s += i;
    return s;
}

int main()
{
    sum({1, 2, 3});
    sum({1, 2, 3, 4});
    return 0;
}