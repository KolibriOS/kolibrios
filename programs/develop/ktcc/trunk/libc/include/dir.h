#ifndef _DIR_H
#define _DIR_H 

#include <stdbool.h>

#define rmfile(obj) rmdir(obj)
#define PATH_MAX 4096
#define T_FOLDER 16
#define T_FILE 0 
#define FS_ERROR -1

typedef struct {
    unsigned type;
    char *name;
} short_file_info;

//Writes information about files in the "dir" folder to an struct  array"list". Returns the number of files.
int lsdir(const char* dir, short_file_info **list);

// Get the path to the working directory(if buf is NULL, then memory will be allocated automatically)
char *getcwd(char *buf, unsigned size);

// Set path to working directory
void setcwd(const char* cwd);

// Delete empty folder (returns "true" if successful)
bool rmdir(const char* dir);

// Delete a file (returns "true" if successful)
bool rmfile(const char* name);

// Create a foldery (returns "true" if successful)
bool mkdir(const char* dir);

#endif
