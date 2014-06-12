#include<sys/types.h>

extern "C" void * malloc(unsigned int);

void * operator new(unsigned int n)
{
 return malloc(n);
}

void * operator new[](unsigned int n)
{
 return malloc(n);
}
