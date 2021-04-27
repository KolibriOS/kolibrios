/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef _MATH_H_
#define _MATH_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern double	_FUNC(acos)(double _x);
extern double	_FUNC(asin)(double _x);
extern double	_FUNC(atan)(double _x);
extern double	_FUNC(atan2)(double _y, double _x);
extern double	_FUNC(ceil)(double _x);
extern double	_FUNC(cos)(double _x);
extern double	_FUNC(cosh)(double _x);
extern double	_FUNC(exp)(double _x);
extern double	_FUNC(fabs)(double _x);
extern double	_FUNC(floor)(double _x);
extern double	_FUNC(fmod)(double _x, double _y);
extern double	_FUNC(frexp)(double _x, int *_pexp);
extern double	_FUNC(ldexp)(double _x, int _exp);
extern double	_FUNC(log)(double _y);
extern double	_FUNC(log10)(double _x);
extern double	_FUNC(modf)(double _x, double *_pint);
extern double	_FUNC(pow)(double _x, double _y);
extern double	_FUNC(sin)(double _x);
extern double	_FUNC(sinh)(double _x);
extern double	_FUNC(sqrt)(double _x);
extern double	_FUNC(tan)(double _x);
extern double	_FUNC(tanh)(double _x);

#define M_E		    2.7182818284590452354
#define M_LOG2E		1.4426950408889634074
#define M_LOG10E	0.43429448190325182765
#define M_LN2		0.69314718055994530942
#define M_LN10		2.30258509299404568402
#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923
#define M_PI_4		0.78539816339744830962
#define M_1_PI		0.31830988618379067154
#define M_2_PI		0.63661977236758134308
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT2		1.41421356237309504880
#define M_SQRT1_2	0.70710678118654752440
#define PI		M_PI
#define PI2		M_PI_2

extern double	_FUNC(acosh)(double);
extern double	_FUNC(asinh)(double);
extern double	_FUNC(atanh)(double);
extern double	_FUNC(cbrt)(double);
extern double	_FUNC(exp10)(double _x);
extern double	_FUNC(exp2)(double _x);
extern double	_FUNC(expm1)(double);
extern double	_FUNC(hypot)(double, double);
extern double	_FUNC(log1p)(double);
extern double	_FUNC(log2)(double _x);
extern long double _FUNC(modfl)(long double _x, long double *_pint);
extern double	_FUNC(pow10)(double _x);
extern double	_FUNC(pow2)(double _x);
extern double	_FUNC(powi)(double, int);
extern void	_FUNC(sincos)(double, double *, double *);

/* These are in libm.a (Cygnus).  You must link -lm to get these */
/* See libm/math.h for comments */

#ifndef __cplusplus
struct exception {
	int type;
	const char *name;
	double arg1;
	double arg2;
	double retval;
	int err;
};
#endif

extern double _FUNC(erf)(double);
extern double _FUNC(erfc)(double);
extern double _FUNC(gamma)(double);
extern int    _FUNC(isinf)(double);
extern int    _FUNC(isnan)(double);
extern int    _FUNC(finite)(double);
extern double _FUNC(j0)(double);
extern double _FUNC(j1)(double);
extern double _FUNC(jn)(int, double);
extern double _FUNC(lgamma)(double);
extern double _FUNC(nan)(const char*);
extern double _FUNC(y0)(double);
extern double _FUNC(y1)(double);
extern double _FUNC(yn)(int, double);
extern double _FUNC(logb)(double);
extern double _FUNC(nextafter)(double, double);
extern double _FUNC(remainder)(double, double);
extern double _FUNC(scalb)(double, double);
#ifndef __cplusplus
extern int _FUNC(matherr)(struct exception *);
#endif
extern double _FUNC(significand)(double);
extern double _FUNC(copysign)(double, double);
extern int    _FUNC(ilogb)(double);
extern double _FUNC(rint)(double);
extern double _FUNC(scalbn)(double, int);
extern double _FUNC(drem)(double, double);
extern double _FUNC(gamma_r)(double, int *);
extern double _FUNC(lgamma_r)(double, int *);
extern float  _FUNC(acosf)(float);
extern float  _FUNC(asinf)(float);
extern float  _FUNC(atanf)(float);
extern float  _FUNC(atan2f)(float, float);
extern float  _FUNC(cosf)(float);
extern float  _FUNC(sinf)(float);
extern float  _FUNC(tanf)(float);
extern float  _FUNC(coshf)(float);
extern float  _FUNC(sinhf)(float);
extern float  _FUNC(tanhf)(float);
extern float  _FUNC(expf)(float);
extern float  _FUNC(frexpf)(float, int *);
extern float  _FUNC(ldexpf)(float, int);
extern float  _FUNC(logf)(float);
extern float  _FUNC(log10f)(float);
extern float  _FUNC(modff)(float, float *);
extern float  _FUNC(powf)(float, float);
extern float  _FUNC(sqrtf)(float);
extern float  _FUNC(ceilf)(float);
extern float  _FUNC(fabsf)(float);
extern float  _FUNC(floorf)(float);
extern float  _FUNC(fmodf)(float, float);
extern float  _FUNC(erff)(float);
extern float  _FUNC(erfcf)(float);
extern float  _FUNC(gammaf)(float);
extern float  _FUNC(hypotf)(float, float);
extern int    _FUNC(isinff)(float);
extern int    _FUNC(isnanf)(float);
extern int    _FUNC(finitef)(float);
extern float  _FUNC(j0f)(float);
extern float  _FUNC(j1f)(float);
extern float  _FUNC(jnf)(int, float);
extern float  _FUNC(lgammaf)(float);
extern float  _FUNC(nanf)(const char*);
extern float  _FUNC(y0f)(float);
extern float  _FUNC(y1f)(float);
extern float  _FUNC(ynf)(int, float);
extern float  _FUNC(acoshf)(float);
extern float  _FUNC(asinhf)(float);
extern float  _FUNC(atanhf)(float);
extern float  _FUNC(cbrtf)(float);
extern float  _FUNC(logbf)(float);
extern float  _FUNC(nextafterf)(float, float);
extern float  _FUNC(remainderf)(float, float);
extern float  _FUNC(scalbf)(float, float);
extern float  _FUNC(significandf)(float);
extern float  _FUNC(copysignf)(float, float);
extern int    _FUNC(ilogbf)(float);
extern float  _FUNC(rintf)(float);
extern float  _FUNC(scalbnf)(float, int);
extern float  _FUNC(dremf)(float, float);
extern float  _FUNC(expm1f)(float);
extern float  _FUNC(log1pf)(float);
extern float  _FUNC(gammaf_r)(float, int *);
extern float  _FUNC(lgammaf_r)(float, int *);

#ifdef __cplusplus
}
#endif

#endif /* _MATH_H_ */