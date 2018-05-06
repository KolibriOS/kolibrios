//IO library
#ifndef INCLUDE_MATH_H
#define INCLUDE_MATH_H
#print "[include <math.h>]\n"

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

:struct MATH
{
	float pi();
	float cos(float x);
	float sin(float x);
	float sqrt(float x);
	float tan(float x);
	float abs(float x);
	float floor(float x);
	signed round(float x);
	signed ceil(float x);
	signed min(signed i1, i2);
	signed max(signed i1, i2);
	signed in(signed base, min1, max1);
}math;

:signed MATH::round(float x)
{
	x+=0.6;
	return x;
}
:signed MATH::ceil(float x)
{
	dword z;
	float tmp;
	z = x;
	tmp = z;
	IF(tmp<x)z++;
	return z;
}
:float MATH::floor(float x)
{
	signed long z;
	z = x;
	IF(z==x)return x;
	IF(z<0) return z-1;
	return z;
}
:float MATH::abs(float x)
{
	IF(x<0)return -x;
	return x;
}
	
:float MATH::cos(float x)
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
:float MATH::sin(float x)
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
:float MATH::sqrt(float x)
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
:float MATH::tan(float x)
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
:signed MATH::min(signed i1, i2)
{
	if (i1 < i2) 
		return i1;
	else 
		return i2;
}
:signed MATH::max(signed i1, i2)
{
	if (i1 > i2) 
		return i1;
	else 
		return i2;
}
:signed MATH::in(signed base, min1, max1)
{
	if (base > max1) return max1;
	if (base < min1) return min1;
	return base;
}
#endif