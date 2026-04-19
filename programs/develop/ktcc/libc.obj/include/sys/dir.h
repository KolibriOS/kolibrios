#ifndef _DIR_H_
#define _DIR_H_

#include <stddef.h>

DLLAPI char* getcwd(char* buf, unsigned size);
DLLAPI void setcwd(const char* cwd);
DLLAPI int rmdir(const char* dir);
DLLAPI int mkdir(const char* dir);

#endif