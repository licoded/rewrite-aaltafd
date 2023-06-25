#include "formula/aalta_formula.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <iostream>


#define MAXN 100000
char in[MAXN];

using namespace aalta;

int main()
{
    aalta_formula* o = aalta_formula::TRUE();
    assert(o->is_literal());

    // === TESTs for unique() func
    aalta_formula* x = aalta_formula (e_and, aalta_formula::TRUE(), aalta_formula::TRUE()).unique ();
	aalta_formula* y = aalta_formula (e_and, aalta_formula::TRUE(), aalta_formula::TRUE()).unique ();
    assert(x == y);

    // === TESTs for to_string() func
    std::cout << o->to_string() << std::endl;
    std::cout << aalta_formula::TRUE()->to_string() << std::endl;

    std::cout << x->to_string() << std::endl;
    // std::cout << y.to_string() << std::endl;
    // The above line code will cause a ERROR:
    // expression must have class type but it has type "aalta::aalta_formula *"
    std::cout << y->to_string() << std::endl;

    // 不加 const 前缀会报警告如下:
    // warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
    std::vector<const char *> str = {
        "a",
        "!a",
        "X(a)",
        "X(!a)",
        "!X(a)",
        "X(a|b) & c",
        "X(a|b) & X(c)",
        "X(a|b) & G(X(c))",
    };

    // === TESTs for constructor/build func
    /**
     * ERROR: a fully qualified constructor call is not allowed
     * CODE: std::cout << aalta_formula::aalta_formula("hhh").to_string() << std::endl;
    */
    std::cout << aalta_formula("hhh").unique()->to_string() << std::endl;
    std::cout << aalta_formula("a").unique()->to_string() << std::endl;
    for(const auto it : str)
    {
        std::cout << aalta_formula(it).unique()->to_string() << std::endl;
    }

    // === TESTs for add_tail() func
    
    // std::cout << str.size();
    for(const auto it : str)
    {
        std::cout << aalta_formula(it).add_tail()->to_string() << std::endl;
    }

    return 0;
}