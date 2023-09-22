#ifndef __L_LIBC_H_INCLUDED_
#define __L_LIBC_H_INCLUDED_
//
// libc.obj
//

//
// libc - import table
//
#define import_libc sprintf
//int (* scanf)(char* __buffer, const char* __format, ...) = (int (*)(char*, const char*, ...))&"scanf"; //run console aplication ???
int (* sprintf)(char* __buffer, const char* __format, ...) = (int (*)(char*, const char*, ...))&"sprintf";
#ifndef __KOS_LIB_H_INCLUDED_
	char* (* strchr)(const char* string, int c) = (char* (*)(const char*, int))&"strchr";
#endif
char* (* strcat)(char* str1, const char* str2) = (char* (*)(char*, const char*))&"strcat";
#ifndef __KOS_LIB_H_INCLUDED_
	int (* strcmp)(const char* str1, const char* str2) = (int (*)(const char*, const char*))&"strcmp";
#endif
int (* strcoll)(const char* str1, const char* str2) = (int (*)(const char*, const char*))&"strcoll";
#ifndef __KOS_LIB_H_INCLUDED_
	char* (* strcpy)(char* str1, const char* str2) = (char* (*)(char*, const char*))&"strcpy";
#endif
int (* strcspn)(const char* string, const char* strCharSet) = (int (*)(const char*, const char*))&"strcspn";
char* (* strdup)(const char* str) = (char* (*)(const char*))&"strdup";
char* (* strerror)(int err) = (char* (*)(int))&"strerror";
#ifndef __KOS_LIB_H_INCLUDED_
	int (* strlen)(const char* string) = (int (*)(const char*))&"strlen";
#endif
char* (* strncat)(char* strDest, const char* strSource, int count) = (char* (*)(char*, const char*, int))&"strncat";
int (* strncmp)(const char* str1, const char* str2, int count) = (int (*)(const char*, const char*, int))&"strncmp";
#ifndef __KOS_LIB_H_INCLUDED_
	char* (* strncpy)(char* strDest, const char* strSource, int count) = (char* (*)(char*, const char*, int))&"strncpy";
#endif
char* (* strrchr)(const char* s, int c) = (char* (*)(const char*, int))&"strrchr";
char* (* strrev)(char* p) = (char* (*)(char*))&"strrev";
int (* strspn)(const char* string, const char* strCharSet) = (int (*)(const char*, const char*))&"strspn";
#ifndef __KOS_LIB_H_INCLUDED_
	char* (* strstr)(const char* s, const char* find) = (char* (*)(const char*, const char*))&"strstr";
#endif
char* (* strtok)(char* s, const char* delim) = (char* (*)(char*, const char*))&"strtok";
int (* strxfrm)(char* strDest, const char* strSource, int count) = (int (*)(char*, const char*, int))&"strxfrm";

double (* exp)(double x) = (double (*)(double))&"exp";
double (* fabs)(double x) = (double (*)(double))&"fabs";

asm{
	dd 0,0
}

#endif