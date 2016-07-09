#include <math.h>

double remainder(double numer, double denom)
{
	double res;
	asm("fldl %2;"
		"fldl %1;"
		"fprem1;"
		"fstpl %0;"
		"fstp %%st;"
	: "=m"(res)
	: "m"(numer), "m"(denom)
	);
	return res;
}
//remainder of 5.3 / 2 is -0.700000
//remainder of 18.5 / 4.2 is 1.700000



double fmod(double numer, double denom)
{
	double res;
	asm("fldl %2;"
		"fldl %1;"
		"fprem;"
		"fstpl %0;"
		"fstp %%st;"
	: "=m"(res)
	: "m"(numer), "m"(denom)
	);
	return res;
}
// fmod of 5.3 / 2 is 1.300000
// fmod of 18.5 / 4.2 is 1.700000


double modf(double x, double *intpart)
{
	double res, intp;
	asm("fldl %2;"
		"fldl %2;"
		"frndint;"
		"fstl %1;"
		"fxch;"
		"fsubp %%st, %%st(1);"
		"fstpl %0"
	: "=m"(res), "=m"(intp)
	: "m"(x) 
	);
	*intpart = intp;
	return res;
}

double ldexp (double x, int expon)
// = x * 2^expot
{
	double res;
	asm("fildl %2;"
		"fldl %1;"
		"fscale;"
		"fstpl %0;"
		"fstp %%st;"
	: "=m"(res)
	: "m"(x), "m"(expon)
	);

	return res;
}

double frexp (double x, int* expon)
{
	double res;
	asm("fldl %2;"
		"fxtract;"
		"fstpl %0;"
		"fistpl %1;"
		"fstp %%st;"
	: "=m"(res), "=m"(*expon)
	: "m"(x)
	);

//	*expon = (int)ex;
	return res;
}
// 8.000000 = 0.500000 * 2^ 4
