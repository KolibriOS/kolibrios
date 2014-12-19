#include "rsmicrolibc.h"

// some math and string functions

double sqrt( double val ) {
    double result ;
    asm volatile ( "fld %1;"
                            "fsqrt;"
                            "fstp %0;" : "=g" (result) : "g" (val)
    ) ;
    return result;
};

float sqrtf( float f ) {
	return (float) sqrtf(f);
};


double sin(double val)
{
    double result ;
    asm volatile ( "fld %1;"
                            "fsin;"
                            "fstp %0;" : "=g" (result) : "g" (val)
    ) ;
    return result;
}

double cos(double val)
{
    double result ;
    asm volatile ( "fld %1;"
                            "fcos;"
                            "fstp %0;" : "=g" (result) : "g" (val)
    ) ;
    return result;
}


double exp (double x)
{
   double result;
   asm ("fldl2e; "
   		 "fmulp; "
   		 "fld %%st; "
   		 "frndint; "
   		 "fsub %%st,%%st(1); "
   		 "fxch;" 
   		 "fchs; "
   		 "f2xm1; "
   		 "fld1; "
   		 "faddp; "
   		 "fxch; "
   		 "fld1; "
   		 "fscale; "
   		 "fstp %%st(1); "
   		 "fmulp" :  "=t"(result) : "0"(x));
   return result;
}

float expf(float x) {
	return (float)(exp(x));
};




double log(double Power)
{
    
	// From here:  http://www.codeproject.com/Tips/311714/Natural-Logarithms-and-Exponent
    
	double N, P, L, R, A, E;
	E = 2.71828182845905;
	P = Power;
	N = 0.0;

            // This speeds up the convergence by calculating the integral
	while(P >= E)  
	{
		P /= E;
		N++;
	}
            N += (P / E);
	P = Power;
	
	do
	{
		A = N;
		L = (P / (exp(N - 1.0)));
		R = ((N - 1.0) * E);
		N = ((L + R) / E);
	} while (!( fabs(N-A)<0.01 ));
	
	return N;
}
    

float logf(float x) {
	return (float)(log(x));
};

double pow(double x, double p) {
    
    if (x < 0.001) {
        return 0.0; 
    };
    
	return exp(p * log(x));
};
float powf(float x, float p) {
	return expf(p * logf(x));
};




int abs(int x) {
	return (x>0) ? x : -x;
};
double fabs(double x) {
	return (x>0) ? x : -x;
};

double floor(double x) {
	return (double)((int)x); // <--------- only positive! 
};

double round(double x) {
	return floor(x+0.5);
};
float roundf(float x) {
	return (float)round(x);
};




void* malloc(unsigned s)
{
	asm ("int $0x40"::"a"(68), "b"(12), "c"(s) );
}


void free(void *p)
{
	asm ("int $0x40"::"a"(68), "b"(13), "c"(p) );
}

void*  memset(void *mem, int c, unsigned size)
{
	unsigned i;

	for ( i = 0; i < size; i++ )
		 *((char *)mem+i) = (char) c;

	return NULL;	
}


void* memcpy(void *dst, const void *src, unsigned size)
{

	unsigned i;

	for ( i = 0; i < size; i++)
		*(char *)(dst+i) = *(char *)(src+i);

	return NULL;
}


char*  strchr(char* s, int c) {

	while (*s) {
		if (*s == (char) c) {
			return s;
		};
		s++;
	};
	return NULL;

};

unsigned int strlen ( char * str ) {
	unsigned int len = 0;
	while ( *str ) {
		len++;
		str++;
	};
	return len;
};

