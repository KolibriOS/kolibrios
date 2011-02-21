#include<stdio.h>

int main(int argc,char * argv[])
{
 int i;
 FILE * f;
 f=fopen(argv[1],"a");
 if(!f) return -1;
 for(i=2;i<argc;i++) fprintf(f,"%s ",argv[i]);
 fclose(f);
 return 0;
}
