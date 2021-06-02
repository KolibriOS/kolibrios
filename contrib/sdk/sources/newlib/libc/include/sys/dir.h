/* BSD predecessor of POSIX.1 <dirent.h> and struct dirent */

#ifndef _SYS_DIR_H_
#define _SYS_DIR_H_

#include <dirent.h>

#define direct dirent

extern int chdir(char* dir);
extern int rmdir(const char* dir);
extern int mkdir(const char* dir, unsigned fake_mode);

#endif /*_SYS_DIR_H_*/
