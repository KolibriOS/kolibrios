/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef _MATH_H_
#define _MATH_H_

#include <stddef.h>

extern double _FUNC(acos)(double _x);
extern double _FUNC(asin)(double _x);
extern double _FUNC(atan)(double _x);
extern double _FUNC(atan2)(double _y, double _x);
extern double _FUNC(ceil)(double _x);
extern double _FUNC(cos)(double _x);
extern double _FUNC(cosh)(double _x);
extern double _FUNC(exp)(double _x);
extern double _FUNC(fabs)(double _x);
extern double _FUNC(floor)(double _x);
extern double _FUNC(fmod)(double _x, double _y);
extern double _FUNC(frexp)(double _x, int* _pexp);
extern double _FUNC(ldexp)(double _x, int _exp);
extern double _FUNC(log)(double _y);
extern double _FUNC(log10)(double _x);
extern double _FUNC(modf)(double _x, double* _pint);
extern double _FUNC(pow)(double _x, double _y);
extern double _FUNC(sin)(double _x);
extern double _FUNC(sinh)(double _x);
extern double _FUNC(sqrt)(double _x);
extern double _FUNC(tan)(double _x);
extern double _FUNC(tanh)(double _x);
extern double _FUNC(acosh)(double);
extern double _FUNC(asinh)(double);
extern double _FUNC(atanh)(double);
extern double _FUNC(hypot)(double, double);
extern long double _FUNC(modfl)(long double _x, long double* _pint);
extern double _FUNC(pow10)(double _x);
extern double _FUNC(pow2)(double _x);

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