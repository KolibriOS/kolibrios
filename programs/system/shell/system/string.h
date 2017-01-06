
#ifndef NULL
#define NULL ((void*)0)
#endif

void*  memset(void *mem, int c, unsigned size);
void*  memcpy(void *dst, const void *src, unsigned size);
int    memcmp(const void* buf1, const void* buf2, int count);

void   strcat(char strDest[], char strSource[]);
int    strcmp(const char* string1, const char* string2);
void   strcpy(char strDest[], const char strSource[]);
char* strncpy(char *strDest, const char *strSource, unsigned n);
int    strlen(const char* string);
char*  strchr(const char* string, int c);
char* strrchr(const char* string, int c);
void    _itoa(int i, char *s);
void  reverse(char *s);
void     itoa(int i, char *s);
int     _atoi( char *s );
int    strnicmp(const char* string1, const char* string2, unsigned count);
