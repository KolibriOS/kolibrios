/* String handling <string.h>

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* String function conventions */

/*
   In any of the following functions taking a size_t n to specify the length of
   an array or size of a memory region, n may be 0, but the pointer arguments to
   the call shall still be valid unless otherwise stated.
*/

/* Copying functions */

extern void* _FUNC(memccpy)(void *restrict dest, const void *restrict src, int c, size_t n);

/* Copy a number of n characters from the memory area pointed to by s2 to the
   area pointed to by s1. If the two areas overlap, behaviour is undefined.
   Returns the value of s1.
*/

#ifdef __TINYC__
extern void* memcpy(void* s1, const void* s2, size_t n);
extern void* memset(void* s, int c, size_t n);
extern void* memmove(void* s1, const void* s2, size_t n);
#else
extern void* _FUNC(memcpy)(void* s1, const void* s2, size_t n);
extern void* _FUNC(memset)(void* s, int c, size_t n);
extern void* _FUNC(memmove)(void* s1, const void* s2, size_t n);
#endif

/* Copy the character array s2 (including terminating '\0' byte) into the
   character array s1.
   Returns the value of s1.
*/
extern char* _FUNC(strcpy)(char*  s1, const char* s2);

/* Copy a maximum of n characters from the character array s2 into the character
   array s1. If s2 is shorter than n characters, '\0' bytes will be appended to
   the copy in s1 until n characters have been written. If s2 is longer than n
   characters, NO terminating '\0' will be written to s1. If the arrays overlap,
   behaviour is undefined.
   Returns the value of s1.
*/
extern char* _FUNC(strncpy)(char* s1, const char* s2, size_t n);

/* Concatenation functions */

/* Append the contents of the character array s2 (including terminating '\0') to
   the character array s1 (first character of s2 overwriting the '\0' of s1). If
   the arrays overlap, behaviour is undefined.
   Returns the value of s1.
*/
extern char* _FUNC(strcat)(char* s1, const char* s2);

/* Append a maximum of n characters from the character array s2 to the character
   array s1 (first character of s2 overwriting the '\0' of s1). A terminating
   '\0' is ALWAYS appended, even if the full n characters have already been
   written. If the arrays overlap, behaviour is undefined.
   Returns the value of s1.
*/
extern char* _FUNC(strncat)(char* s1, const char* s2, size_t n);

/* Comparison functions */

/* Compare the first n characters of the memory areas pointed to by s1 and s2.
   Returns 0 if s1 == s2, a negative number if s1 < s2, and a positive number if
   s1 > s2.
*/
extern int _FUNC(memcmp)(const void * s1, const void* s2, size_t n);

/* Compare the character arrays s1 and s2.
   Returns 0 if s1 == s2, a negative number if s1 < s2, and a positive number if
   s1 > s2.
*/
extern int _FUNC(strcmp)(const char * s1, const char* s2);

/* Compare the character arrays s1 and s2, interpreted as specified by the
   LC_COLLATE category of the current locale.
   Returns 0 if s1 == s2, a negative number if s1 < s2, and a positive number if
   s1 > s2.
   TODO: Currently a dummy wrapper for strcmp() as PDCLib does not yet support
   locales.
*/
extern int _FUNC(strcoll)(const char* s1, const char* s2);

/* Compare no more than the first n characters of the character arrays s1 and
   s2.
   Returns 0 if s1 == s2, a negative number if s1 < s2, and a positive number if
   s1 > s2.
*/
extern int _FUNC(strncmp)(const char* s1, const char* s2, size_t n);

/* Transform the character array s2 as appropriate for the LC_COLLATE setting of
   the current locale. If length of resulting string is less than n, store it in
   the character array pointed to by s1. Return the length of the resulting
   string.
*/
extern size_t _FUNC(strxfrm)(char* s1, const char* s2, size_t n);

/* Search functions */

/* Search the first n characters in the memory area pointed to by s for the
   character c (interpreted as unsigned char).
   Returns a pointer to the first instance found, or NULL.
*/
extern void* _FUNC(memchr)(const void* s, int c, size_t n);

/* Search the character array s (including terminating '\0') for the character c
   (interpreted as char).
   Returns a pointer to the first instance found, or NULL.
*/
extern char* _FUNC(strchr)(const char* s, int c);

/* Determine the length of the initial substring of character array s1 which
   consists only of characters not from the character array s2.
   Returns the length of that substring.
*/
extern size_t _FUNC(strcspn)(const char* s1, const char* s2);

/* Search the character array s1 for any character from the character array s2.
   Returns a pointer to the first occurrence, or NULL.
*/
extern char* _FUNC(strpbrk)(const char* s1, const char* s2);

/* Search the character array s (including terminating '\0') for the character c
   (interpreted as char).
   Returns a pointer to the last instance found, or NULL.
*/
extern char* _FUNC(strrchr)(const char * s, int c );

/* Determine the length of the initial substring of character array s1 which
   consists only of characters from the character array s2.
   Returns the length of that substring.
*/
extern size_t _FUNC(strspn)(const char * s1, const char * s2);

/* Search the character array s1 for the substring in character array s2.
   Returns a pointer to that sbstring, or NULL. If s2 is of length zero,
   returns s1.
*/
extern char* _FUNC(strstr)(const char * s1, const char * s2);

/* In a series of subsequent calls, parse a C string into tokens.
   On the first call to strtok(), the first argument is a pointer to the to-be-
   parsed C string. On subsequent calls, the first argument is NULL unless you
   want to start parsing a new string. s2 holds an array of separator characters
   which can differ from call to call. Leading separators are skipped, the first
   trailing separator overwritten with '\0'.
   Returns a pointer to the next token.
   WARNING: This function uses static storage, and as such is not reentrant.
*/
extern char* _FUNC(strtok)(char* s1, const char* s2);

/* Map an error number to a (locale-specific) error message string. Error
   numbers are typically errno values, but any number is mapped to a message.
   TODO: PDCLib does not yet support locales.
*/
extern char*  _FUNC(strerror)(int errnum);

/* Returns the length of the string s (excluding terminating '\0').*/
extern size_t _FUNC(strlen)(const char * s);

/* The function reverses the sequence of characters in the string pointed to by str. */
extern char* _FUNC(strrev)(char *str);

/* The strdup function executes the function pointed to by the str argument. */
extern char* _FUNC(strdup)(const char *str);

#endif
