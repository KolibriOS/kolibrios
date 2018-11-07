#ifndef string_h
#define string_h
typedef unsigned int size_t;


extern void* memchr(const void*,int,size_t);
extern int memcmp(const void*,const void*,size_t);
extern void* memcpy(void*,const void*,size_t);
extern void* memmove(void*,const void*,size_t);
extern void* memset(void*,int,size_t);
extern char* strcat(char*,const char*);
extern char* strchr(const char*,int);
extern int strcmp(const char*,const char*);
extern int strcoll(const char*,const char*);
extern char* strcpy(char*,const char*);
extern size_t strcspn(const char*,const char*);
extern int strlen(const char*);
extern char* strncat(char*,const char*,size_t);
extern int strncmp(const char*,const char*,size_t);
extern char* strncpy(char*,const char*,size_t);
extern char* strpbrk(const char*,const char*);
extern char* strrchr(const char*,int);
extern size_t strspn(const char*,const char*);
extern char* strstr(const char*,const char*);
extern char* strtok(char*,const char*);
extern int strxfrm(char*,const char*,int);
extern char* strdup(const char*);
extern char* strrev(char *p);
char * strerror ( int errnum );

#ifndef NULL
# define NULL ((void*)0)
#endif

#endif
