#include <math.h>

double round(double x)
{
    double whole;

    if (x < 0.0)
        return -round(-x);
    whole = floor(x);
    if (x != whole && x - whole >= 0.5)
        whole += 1.0;
    return whole;
}
