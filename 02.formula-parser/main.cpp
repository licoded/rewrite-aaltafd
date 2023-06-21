#include "formula/aalta_formula.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAXN 100000
char in[MAXN];

using namespace aalta;

int main()
{
    puts ("please input the formula:");
    if (fgets (in, MAXN, stdin) == NULL)
    {
        printf ("Error: read input!\n");
        exit (0);
    }

    aalta_formula* af;

    return 0;
}