#include<stdio.h>
#include<stdlib.h>

char * xbuf;
unsigned long sz;

int main(void)
{
 FILE * fp;
 chdir("/hd/1/menuetos/doom");
 __libclog_printf("Opening file ...");
 fp=fopen("doom1.wad","rb");
 if(!fp)
 {
  __libclog_printf("failed\n");
  return 1;
 }
 __libclog_printf("OK\n");
 fseek(fp,0,SEEK_END);
 sz=ftell(fp);
 fseek(fp,0,SEEK_SET);
 xbuf=malloc(sz);
 if(!xbuf)
 {
  __libclog_printf("Unable to malloc %u bytes\n",sz);
  return 1;
 }
 __libclog_printf("Reading ...");
 fread(xbuf,1,sz,fp);
 __libclog_printf("done\n");
 fclose(fp);
 return 0;
}
