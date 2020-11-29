#ifndef _DIR_H
#define _DIR_H 

#include <stdbool.h>

#define PATH_MAX 4096

// Get the path to the working directory(if buf is NULL, then memory will be allocated automatically)
char *getcwd(char *buf, unsigned size);

// Set path to working directory
void setcwd(const char* cwd);

// Remove directory (returns "true" if successful)
bool rmdir(const char* dir);

// Create directory (returns "true" if successful)
bool mkdir(const char* dir);

#endif
