//IO library
#ifndef INCLUDE_MATH_C
#define INCLUDE_MATH_C

static inline signed math_round(float x)
{
	x+=0.6;
	return x;
}
static inline signed math_ceil(float x)
{
	long z = (long)x;
	if(z<x) return ++z;
	return z;
}
static inline float math_floor(float x)
{
	signed long z = x;
	if(z==x)return x;
	if(z<0) return --z;
	return z;
}
static inline float math_abs(float x)
{
	if(x<0)return -x;
	return x;
}

/*
static inline float math_cos(float x)
{
	float r;
	asm 
	{
		fld x
		fcos
		fstp r
	}
	return r;
}
static inline float math_sin(float x)
{
	float r;
	asm 
	{
		fld x
		fsin
		fstp r
	}
	return r;
}
static inline float math_sqrt(float x)
{
	float r;
	asm 
	{
		fld x
		fsqrt
		fstp r
	}
	return r;
}
static inline float math_tan(float x)
{
	float r;
	asm 
	{
		fld x
		fld1
		fpatan
		fstp r
	}
	return r;
}
*/
#endif