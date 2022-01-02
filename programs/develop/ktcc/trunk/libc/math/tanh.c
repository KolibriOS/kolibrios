#include <math.h>

double tanh (double x)
{
   	double ex = exp(x), exm = exp(-x);

	return (ex - exm) / (ex + exm);
}