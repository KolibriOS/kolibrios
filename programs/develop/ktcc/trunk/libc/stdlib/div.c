#include <stdlib.h>

div_t div (int numer, int denom)
{
    div_t res;
    res.quot = numer / denom;
    res.rem = numer % denom;

    return res;
}
