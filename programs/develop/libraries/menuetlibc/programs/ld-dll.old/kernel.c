#include"loader.h"

#define EX(x) \
    { "_"#x , (unsigned long)&x }

extern void * sbrk(int);
extern void _exit(int);

static struct {
 char * name;
 unsigned long ptr;
} kextable[]={
 EX(sbrk),
 EX(errno),
 EX(malloc),
 EX(free),
 EX(realloc),
 EX(atexit),
 EX(exit),
 EX(getenv),
 EX(_exit),
};

#define NR_KEX	(sizeof(kextable)/sizeof(kextable[0]))

unsigned long kexport_lookup(char * name)
{
 int i,j;
 j=strlen(name);
 for(i=0;i<NR_KEX;i++)
 {
  if(strlen(kextable[i].name)==j && 
     !strncmp(kextable[i].name,name,j)) return kextable[i].ptr;
 }
 return 0;
}
