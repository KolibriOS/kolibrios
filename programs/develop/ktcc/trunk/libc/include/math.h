/* Copyright (C) 1999 DJ Delorie, see http://www.delorie.com/copyright.html for details */
/* Copyright (C) 1998 DJ Delorie, see http://www.delorie.com/copyright.html for details */
/* Copyright (C) 1995 DJ Delorie, see http://www.delorie.com/copyright.html for details */

#ifndef _MATH_H
#define _MATH_H

extern double   acos(double _x);
extern double   asin(double _x);
extern double   atan(double _x);
extern double   atan2(double _y, double _x);
extern float    ceilf(float);
extern double   ceil(double _x);
extern double   cos(double _x);
extern double   cosh(double _x);
extern double   exp(double _x);
extern double   exp2(double _x);
extern double   fabs(double _x);
extern float    fabsf(float);
extern double   floor(double _x);
extern float    floorf(float);
extern double   fmod(double _x, double _y);
extern double   frexp(double _x, int *_pexp);
extern double   ldexp(double _x, int _exp);
extern double   remainder(double, double);
extern double   log(double _y);
extern double   log10(double _x);
extern double   modf(double _x, double *_pint);
extern double   pow(double _x, double _y);
extern double   sin(double _x);
extern double   sinh(double _x);
extern double   sqrt(double _x);
extern float    sqrtf(float);
extern double   tan(double _x);
extern double   tanh(double _x);
extern double   round(double x);
extern long double roundl  (long double x);

#define M_E             2.7182818284590452354
#define M_LOG2E         1.4426950408889634074
#define M_LOG10E        0.43429448190325182765
#define M_LN2           0.69314718055994530942
#define M_LN10          2.30258509299404568402
#define M_PI            3.14159265358979323846
#define M_PI_2          1.57079632679489661923
#define M_PI_4          0.78539816339744830962
#define M_1_PI          0.31830988618379067154
#define M_2_PI          0.63661977236758134308
#define M_2_SQRTPI      1.12837916709551257390
#define M_SQRT2         1.41421356237309504880
#define M_SQRT1_2       0.70710678118654752440
#define PI              M_PI
#define PI2             M_PI_2

#endif
