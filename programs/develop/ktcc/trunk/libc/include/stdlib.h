#ifndef stdlib_h
#define stdlib_h
#include "kolibrisys.h"

//#define isspace(c) ((c)==' ')
#define abs(i) (((i)<0)?(-(i)):(i))

extern int atoib(char *s,int b);
extern int atoi(char *s);
extern unsigned char tolower(unsigned char c);
extern unsigned char toupper(unsigned char c);
extern void itoab(int n,char* s,int  b);
extern void itoa(int n,char* s);

extern void* stdcall malloc(dword size);
extern void  stdcall free(void *pointer);
extern void* stdcall realloc(void* pointer,dword size);
#endif