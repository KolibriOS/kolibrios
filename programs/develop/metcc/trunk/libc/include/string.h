#ifndef string_h
#define string_h
extern void* memchr(const void*,int,int);
extern int memcmp(const void*,const void*,int);
extern void* memcpy(void*,const void*,int);
extern void* memmove(void*,const void*,int);
extern void* memset(void*,int,int);
extern char* strcat(char*,const char*);
extern char* strchr(const char*,int);
extern int strcmp(const char*,const char*);
extern int strcoll(const char*,const char*);
extern char* strcpy(char*,const char*);
extern int strcspn(const char*,const char*);
extern int strlen(const char*);
extern char* strncat(char*,const char*,int);
extern int strncmp(const char*,const char*,int);
extern char* strncpy(char*,const char*,int);
extern char* strpbrk(const char*,const char*);
extern char* strrchr(const char*,int);
extern int strspn(const char*,const char*);
extern char* strstr(const char*,const char*);
extern char* strtok(char*,const char*);
extern int strxfrm(char*,const char*,int);
extern char* strdup(const char*);
#endif