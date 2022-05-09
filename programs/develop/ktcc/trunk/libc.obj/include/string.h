#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>

DLLAPI void* memccpy(void* restrict dest, const void* restrict src, int c, size_t n);

#ifdef __TINYC__
extern void* memcpy(void* s1, const void* s2, size_t n);
extern void* memset(void* s, int c, size_t n);
extern void* memmove(void* s1, const void* s2, size_t n);
#else
DLLAPI void* memcpy(void* s1, const void* s2, size_t n);
DLLAPI void* memset(void* s, int c, size_t n);
DLLAPI void* memmove(void* s1, const void* s2, size_t n);
#endif

DLLAPI char* strcpy(char* s1, const char* s2);
DLLAPI char* strncpy(char* s1, const char* s2, size_t n);
DLLAPI char* strcat(char* s1, const char* s2);
DLLAPI char* strncat(char* s1, const char* s2, size_t n);
DLLAPI int memcmp(const void* s1, const void* s2, size_t n);
DLLAPI int strcmp(const char* s1, const char* s2);
DLLAPI int strcoll(const char* s1, const char* s2);
DLLAPI int strncmp(const char* s1, const char* s2, size_t n);
DLLAPI size_t strxfrm(char* s1, const char* s2, size_t n);
DLLAPI void* memchr(const void* s, int c, size_t n);
DLLAPI char* strchr(const char* s, int c);
DLLAPI size_t strcspn(const char* s1, const char* s2);
DLLAPI char* strpbrk(const char* s1, const char* s2);
DLLAPI char* strrchr(const char* s, int c);
DLLAPI size_t strspn(const char* s1, const char* s2);
DLLAPI char* strstr(const char* s1, const char* s2);
DLLAPI char* strtok(char* s1, const char* s2);
DLLAPI char* strerror(int errnum);
DLLAPI size_t strlen(const char* s);
DLLAPI char* strrev(char* str);
DLLAPI char* strdup(const char* str);

#endif
