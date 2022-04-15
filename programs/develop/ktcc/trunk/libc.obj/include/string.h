#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>

extern void* _FUNC(memccpy)(void* restrict dest, const void* restrict src, int c, size_t n);

#ifdef __TINYC__
extern void* memcpy(void* s1, const void* s2, size_t n);
extern void* memset(void* s, int c, size_t n);
extern void* memmove(void* s1, const void* s2, size_t n);
#else
extern void* _FUNC(memcpy)(void* s1, const void* s2, size_t n);
extern void* _FUNC(memset)(void* s, int c, size_t n);
extern void* _FUNC(memmove)(void* s1, const void* s2, size_t n);
#endif

extern char* _FUNC(strcpy)(char* s1, const char* s2);
extern char* _FUNC(strncpy)(char* s1, const char* s2, size_t n);
extern char* _FUNC(strcat)(char* s1, const char* s2);
extern char* _FUNC(strncat)(char* s1, const char* s2, size_t n);
extern int _FUNC(memcmp)(const void* s1, const void* s2, size_t n);
extern int _FUNC(strcmp)(const char* s1, const char* s2);
extern int _FUNC(strcoll)(const char* s1, const char* s2);
extern int _FUNC(strncmp)(const char* s1, const char* s2, size_t n);
extern size_t _FUNC(strxfrm)(char* s1, const char* s2, size_t n);
extern void* _FUNC(memchr)(const void* s, int c, size_t n);
extern char* _FUNC(strchr)(const char* s, int c);
extern size_t _FUNC(strcspn)(const char* s1, const char* s2);
extern char* _FUNC(strpbrk)(const char* s1, const char* s2);
extern char* _FUNC(strrchr)(const char* s, int c);
extern size_t _FUNC(strspn)(const char* s1, const char* s2);
extern char* _FUNC(strstr)(const char* s1, const char* s2);
extern char* _FUNC(strtok)(char* s1, const char* s2);
extern char* _FUNC(strerror)(int errnum);
extern size_t _FUNC(strlen)(const char* s);
extern char* _FUNC(strrev)(char* str);
extern char* _FUNC(strdup)(const char* str);

#endif
