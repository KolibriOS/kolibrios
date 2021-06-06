#ifndef _DLFCN_H
#define _DLFCN_H

#define RTLD_LAZY	0x00001
#define RTLD_NOW	0x00002
#define RTLD_GLOBAL	0x00100
#define RTLD_LOCAL	0

int    dlclose(void *handle);
char  *dlerror(void);
void  *dlopen(const char *name, int mode);
void  *dlsym(void *restrict handle, const char *restrict name);

#endif