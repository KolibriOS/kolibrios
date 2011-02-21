#include<stdio.h>
#include<stdlib.h>
#ifndef WEXITSTATUS
#include <sys/wait.h>
#endif

char buf[32768];

#if (HAS_DEVENV==0)
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
#if (HAS_DEVENV==0)
 __dev_env=p;
#endif
}

int main(int argc,char * argv[])
{
 int u;
 __env();
 if(argc<3)
 {
  fprintf(stderr,"Usage: %s infile.cpp outfile.o\n",argv[0]);
  return 1;
 }
#if (HAS_DEVENV==1)
 sprintf(buf,"%s -c %s -o %s -nostdinc -fno-builtin -I/dev/env/MENUETDEV/include -fno-common "
	     "-I/dev/env/MENUETDEV/include/STL "
             "-O1 -fno-rtti -fno-exceptions -fomit-frame-pointer -D__MENUETOS__ ",TOOLNAME,argv[1],argv[2]);
#else
 sprintf(buf,"%s -c %s -o %s -nostdinc -fno-builtin -I%s/include -fno-common "
	     "-I%s/include/STL -D__MENUETOS__ "
             "-O1 -fno-rtti -fno-exceptions -fomit-frame-pointer ",TOOLNAME,argv[1],argv[2],
	     __dev_env,__dev_env);
#endif
 if(argc>3)
  for(u=3;u<argc;u++)
  {
   strcat(buf,argv[u]);
   strcat(buf," ");
  }
 return WEXITSTATUS(system(buf));
}
