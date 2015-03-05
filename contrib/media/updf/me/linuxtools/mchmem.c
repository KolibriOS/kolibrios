#define OFF		20

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc,char * argv[])
{
 FILE * f;
 char * buf;
 unsigned long sz,newsz;
 if(argc<3)
 {
  printf("Usage:\n");
  printf("%s filename memsize_hex\n",argv[0]);
  printf("Example:\n\t%s test.app 100000\n",argv[0]);
  return -1;
 }
 sscanf(argv[2],"%x",&newsz);
 if(newsz<0x10000 || newsz>0x2000000) /* Min 64kB max 32MB */
 {
  printf("Impossibly large memory size %x\n",newsz);
  return -1;
 }
 f=fopen(argv[1],"rb");
 if(!f)
 {
  printf("Unable to open file\n");
  return -1;
 }
 fseek(f,0,SEEK_END);
 sz=ftell(f);
 fseek(f,0,SEEK_SET);
 buf=malloc(sz);
 if(!buf)
 {
  printf("Unable to allocate temporary buffer\n");
  fclose(f);
  return -1;
 }
 fread(buf,1,sz,f);
 fclose(f);
 f=fopen(argv[1],"wb");
 *((unsigned long *)(buf+OFF))=newsz;
 fwrite(buf,1,sz,f);
 fclose(f);
 free(buf);
 return 0;
}
