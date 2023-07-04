#include "formula/aalta_formula.h"
#include "ltlfchecker.h"
#include "carchecker.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#define MAXN 100000
char in[MAXN];

using namespace aalta;

int main(int argc, char** argv)
{
    bool BLSC = false;

    for (int i = argc; i > 1; i --)
	{
		if (strcmp (argv[i-1], "-blsc") == 0)
			BLSC = true;
    }

    aalta_formula::TAIL(); // set tail id to be 1

    puts("please input the formula:");
    if (fgets(in, MAXN, stdin) == NULL)
    {
        printf("Error: read input!\n");
        exit(0);
    }
    aalta_formula *af = aalta_formula(in).unique();
    std::cout << af->to_string() << std::endl;

    // af = af->nnf();              // has been done in `build()` func
    // af = af->remove_wnext();     // has been done in `build()` func
    af = af->split_next();
    af = af->add_tail();
    // af = af->simplify();         // just skip this now

    std::cout << "=== after all transfer" << std::endl;
    std::cout << af->to_string() << std::endl;

    bool res;
    if (BLSC)
    {
        LTLfChecker checker(af);
        res = checker.check();
    }
    else
    {
        CARChecker checker(af);
        res = checker.check();
    }
    printf("%s\n", res ? "sat" : "unsat");

    return 0;
}