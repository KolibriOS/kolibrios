
#ifndef AUTOBUILD
#define NULL ((void*)0)

void*  memset(void *mem, int c, unsigned size);
void* memcpy(void *dst, const void *src, unsigned size);
int strlen(const char* string);
#endif

void strcat(char strDest[], char strSource[]);
int strcmp(const char* string1, const char* string2);
//void strcpy(char strDest[], const char strSource[]);
char* strncpy(char *strDest, const char *strSource, unsigned n);
char *strchr(const char* string, int c);
