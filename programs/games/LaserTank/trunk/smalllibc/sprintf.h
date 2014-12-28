#include "kosSyst.h"

//#define PREC 2
//#define HALF 0.499
#define PREC 6
#define HALF 0.4999999

static double double_tab[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30 };

static Dword dectab[] = { 1000000000, 100000000, 10000000, 1000000, 100000,
10000, 1000, 100, 10, 0 };

void sprintf(char *Str, char* Format, ...);
char *ftoa(double d);