#ifndef _DIR_H_
#define _DIR_H_

#include <stddef.h>

extern char* _FUNC(getcwd)(char* buf, unsigned size);
extern void _FUNC(setcwd)(const char* cwd);
extern int _FUNC(rmdir)(const char* dir);
extern int _FUNC(mkdir)(const char* dir);

#endif