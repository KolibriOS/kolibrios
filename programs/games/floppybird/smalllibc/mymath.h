/* Rocket Forces
 * Filename: mymath.h
 * Version 0.1
 * Copyright (c) Serial 2007
 */


extern "C" int _fltused = 0;

#define M_PI 3.14159265358979323846

inline double sin(double x)
{
	__asm	fld	x
	__asm	fsin
}

inline double cos(double x)
{
	__asm	fld	x
	__asm	fcos
}

inline double sqrt(double x)
{
	__asm	fld	x
	__asm	fsqrt
}

inline double acos(double x)
{
	__asm	fld x
	__asm	fld st(0)
	__asm	fmul st,st(1)
	__asm	fld1 
	__asm	fsubrp st(1),st(0)
	__asm	fsqrt 
	__asm	fxch st(1) 
	__asm	fpatan
}

inline double atan(double x)
{
	double res = acos(1 / sqrt(1 + x * x));
	if (x < 0)
	{
		res *= -1;
	}
	return res;
}

inline int round_int(double x)
{
	int i;
	static const float round_to_nearest = 0.5f;
	__asm
	{
		fld      x
		fadd     st, st(0)
		fadd     round_to_nearest
		fistp    i
		sar      i, 1
	}
	return i;
}

inline int floor_int(double x)
{
	int i;
	static const float round_toward_m_i = -0.5f;
	__asm
	{
		fld      x
		fadd     st, st (0)
		fadd     round_toward_m_i
		fistp    i
		sar      i, 1
	}
	return i;
}

inline int ceil_int(double x)
{
	int i;
	static const float round_toward_p_i = -0.5f;
	__asm
	{
		fld      x
		fadd     st, st (0)
		fsubr    round_toward_p_i
		fistp    i
		sar      i, 1
	}
	return (-i);
}
