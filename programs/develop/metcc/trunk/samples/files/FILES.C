#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{

  int i;
  char c;
  FILE *f;
  FILE *fin;
  FILE *fout;

  //write to file
  f=fopen("testfile.txt","w");

  for(i=0;i<50;i++)
  {
        fputc('1',f);
  }
  fclose(f);

  //append to file
  f=fopen("testfile.txt","a");

  for(i=0;i<50;i++)
  {
        fputc('2',f);
  }
  fclose(f);

  //copy from testfile.txt to copyfile.txt

  fin=fopen("testfile.txt","r");
  fout=fopen("copyfile.txt","w");

  while((c=fgetc(fin))!=EOF)
  {
        fputc(c,fout);
  }
  fclose(fin);
  fclose(fout);

}