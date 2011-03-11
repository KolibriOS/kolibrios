#include <math.h>
#include <errno.h>

double ldexp(double x, int expn)
{
  double res;
  if (!isfinite (x) || x == 0.0L)
    return x;

  __asm__ ("fscale"
  	    : "=t" (res)
        : "0" (x), "u" ((double) expn));

 // if (!isfinite (res) || res == 0.0L)
 //   errno = ERANGE;

  return res;
}

