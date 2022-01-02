#include <math.h>

long double roundl  (long double x)
{
	if (x > 0)
		return floor(x + 0.5);
	else
		return ceil(x - 0.5);
}
