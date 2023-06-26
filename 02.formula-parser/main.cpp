#include "formula/aalta_formula.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>


#define MAXN 100000
char in[MAXN];

using namespace aalta;

int main()
{
    aalta_formula::TAIL (); //set tail id to be 1

    puts ("please input the formula:");
    if (fgets (in, MAXN, stdin) == NULL)
    {
        printf ("Error: read input!\n");
        exit (0);
    }
    aalta_formula* af = aalta_formula(in).unique();
    std::cout << af->to_string () << std::endl;

    // af = af->nnf();              // has been done in `build()` func
    af = af->add_tail();
    // af = af->remove_wnext();     // has been done in `build()` func
    // af = af->simplify();         // just skip this now
    af = af->split_next();

    return 0;
}