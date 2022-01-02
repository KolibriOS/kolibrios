#include <math.h>

double asin(double x)
{
	return atan(sqrt(x * x / (1.0 - x * x)));
}