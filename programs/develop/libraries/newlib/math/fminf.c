#include <math.h>

float
fminf (float _x, float _y)
{
  return ((islessequal(_x, _y) || isnan (_y)) ? _x : _y );
}
