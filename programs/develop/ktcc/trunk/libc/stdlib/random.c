
#include "stdlib.h"

unsigned int seed_o=1;

void srand (unsigned int seed)
{
	seed_o=seed;
}

int rand (void)
{
	seed_o=(seed_o*25173+13849) & (65535);
	return(seed_o);
}
