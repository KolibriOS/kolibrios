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
}math;
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
#endif