#include<stdio.h>
#include<string.h>

static char tmp_buf[1024];

int main(int argc,char * argv[])
{
 printf("CREATE %s\n",argv[1]);
 while(!feof(stdin))
 {
  fscanf(stdin,"%s ",&tmp_buf);
  printf("ADDMOD %s\n",tmp_buf);
 }
 printf("SAVE\n");
 printf("END\n");
 return 0;
}
