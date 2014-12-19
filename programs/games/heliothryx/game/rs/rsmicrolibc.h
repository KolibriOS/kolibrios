#ifndef RS_MICROLIBC_FOR_KOLIBRI_H
#define RS_MICROLIBC_FOR_KOLIBRI_H

// some math and string functions

#ifndef NULL
	#define NULL ((void*)0)
#endif

#ifndef uint32_t
	#define uint32_t unsigned int
#endif

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif



double sqrt( double val );
float sqrtf( float f );

double sin(double x);
float sinf(float x);

double cos(double x);
float cosf(float x);

double pow(double x, double p);
float powf(float x, float p);

double exp(double x);
float expf(float x);

double log(double x);
float logf(float x);

int abs(int x);
double fabs(double x);

double floor(double x);

double round(double x);
float roundf(float x);


void* malloc(unsigned size);
void  free(void* pointer);

void*  memset(void *mem, int c, unsigned size);
void*  memcpy(void *dst, const void *src, unsigned size);

char*  strchr(char* s, int c);
unsigned int strlen ( char * str );

#endif
