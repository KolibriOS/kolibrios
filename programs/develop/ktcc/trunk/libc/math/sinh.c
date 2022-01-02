#include <math.h>

double sinh (double x)
{
	return (exp(x) - exp(-x)) / 2;
}