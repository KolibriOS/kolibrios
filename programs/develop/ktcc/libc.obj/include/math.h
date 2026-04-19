/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef _MATH_H_
#define _MATH_H_

#include <stddef.h>

DLLAPI double acos(double _x);
DLLAPI double asin(double _x);
DLLAPI double atan(double _x);
DLLAPI double atan2(double _y, double _x);
DLLAPI double ceil(double _x);
DLLAPI double cos(double _x);
DLLAPI double cosh(double _x);
DLLAPI double exp(double _x);
DLLAPI double fabs(double _x);
DLLAPI double floor(double _x);
DLLAPI double fmod(double _x, double _y);
DLLAPI double frexp(double _x, int* _pexp);
DLLAPI double ldexp(double _x, int _exp);
DLLAPI double log(double _y);
DLLAPI double log2(double _x);
DLLAPI double log10(double _x);
DLLAPI double modf(double _x, double* _pint);
DLLAPI double pow(double _x, double _y);
DLLAPI double round(double _x);
DLLAPI double sin(double _x);
DLLAPI double sinh(double _x);
DLLAPI double sqrt(double _x);
DLLAPI double tan(double _x);
DLLAPI double tanh(double _x);
DLLAPI double acosh(double);
DLLAPI double asinh(double);
DLLAPI double atanh(double);
DLLAPI double hypot(double, double);
DLLAPI long double modfl(long double _x, long double* _pint);
DLLAPI double pow10(double _x);
DLLAPI double pow2(double _x);

#define M_E        2.7182818284590452354
#define M_LOG2E    1.4426950408889634074
#define M_LOG10E   0.43429448190325182765
#define M_LN2      0.69314718055994530942
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.78539816339744830962
#define M_1_PI     0.31830988618379067154
#define M_2_PI     0.63661977236758134308
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2    1.41421356237309504880
#define M_SQRT1_2  0.70710678118654752440
#define PI         M_PI
#define PI2        M_PI_2

struct exception {
    int type;
    const char* name;
    double arg1;
    double arg2;
    double retval;
    int err;
};

#endif /* _MATH_H_ */
