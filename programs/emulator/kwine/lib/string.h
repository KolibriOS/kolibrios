#ifndef _STRING_H
#define _STRING_H

void* memset(void *mem, int c, unsigned size);
void* memcpy(void *dst, const void *src, unsigned size);
int memcmp(const void* buf1, const void* buf2, int count);

char *strcat(char strDest[], char strSource[]);
int strcmp(const char* s1, const char* s2);
char *strcpy(char strDest[], const char strSource[]);
char* strncpy(char *strDest, const char *strSource, unsigned n);
int strlen(const char* string);
char* strchr(const char* string, int c);
char* strrchr(const char* string, int c);

void _itoa(int i, char *s);
void reverse(char s[]);
void itoa(int n, char s[]);
int atoi ( char *s );



#endif