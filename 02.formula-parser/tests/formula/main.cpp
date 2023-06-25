#include "formula/aalta_formula.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAXN 100000
char in[MAXN];

using namespace aalta;

int main()
{
    aalta_formula* x = aalta_formula (e_and, aalta_formula::TRUE(), aalta_formula::TRUE()).unique ();
	aalta_formula* y = aalta_formula (e_and, aalta_formula::TRUE(), aalta_formula::TRUE()).unique ();
	printf("%d\n", x == y);

    return 0;
}