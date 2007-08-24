#include <mesys.h>
//#define isspace(c) ((c)==' ')
#define abs(i) (((i)<0)?(-(i)):(i))

extern int atoib(char *s,int b);
extern int atoi(char *s);
extern char tolower(char c);
extern char toupper(char c);
extern void itoab(int n,char* s,int  b);
extern void itoa(int n,char* s);

extern void* malloc(dword size);
extern void  free(void *pointer);
extern void* realloc(void* pointer,dword size);