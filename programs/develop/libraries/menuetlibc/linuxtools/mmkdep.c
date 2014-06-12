#include<stdio.h>
#include<stdlib.h>
#ifndef WEXITSTATUS
#include <sys/wait.h>
#endif

char buf[32768];

#if (HAS_DEVENV == 0)
char * __dev_env;
#endif

static void __env(void)
{
 char * p=getenv("MENUETDEV");
 if(!p)
 {
  printf("MENUETDEV system variable not set !!!\n");
  exit(-1);
 }
#if (HAS_DEVENV == 0)
 __dev_env=p;
#endif
}

int main(int argc,char * argv[])
{
 int u;
 __env();
 if(argc<2)
 {
  fprintf(stderr,"Usage: %s file1.c file2.s ...\n",argv[0]);
  return 1;
 }
#if (HAS_DEVENV == 0)
 sprintf(buf,"gcc -nostdinc -I%s/include -D__DEV_CONFIG_H=\"<%s/config.h>\" -M ",__dev_env,__dev_env);
#else
 sprintf(buf,"gcc -nostdinc -I/dev/env/MENUETDEV/include -D__DEV_CONFIG_H='\"/dev/env/MENUETDEV/config.h\"' -M ");
#endif
 for(u=1;u<argc;u++)
 {
  strcat(buf,argv[u]);
  strcat(buf," ");
 }
 return WEXITSTATUS(system(buf));
}
