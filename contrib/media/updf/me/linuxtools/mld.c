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
  fprintf(stderr,"Usage: %s outprogramname file1.o file2.o ...\n",argv[0]);
  return 1;
 }
#if (HAS_DEVENV == 1)
 sprintf(buf,"ld -T/dev/env/MENUETDEV/include/scripts/menuetos_app_v01.ld "
	      "-nostdlib -L/dev/env/MENUETDEV/lib -o %s "
              "/dev/env/MENUETDEV/stub/crt0.o ",argv[1]);
#else
 sprintf(buf,"ld -T%s/include/scripts/menuetos_app_v01.ld "
	      "-nostdlib -L%s/lib -o %s "
              "%s/stub/crt0.o ",__dev_env,__dev_env,argv[1],__dev_env);
#endif
 for(u=2;u<argc;u++)
 {
  strcat(buf,argv[u]);
  strcat(buf," ");
 }
 strcat(buf,"-lc");
 return WEXITSTATUS(system(buf));
}
