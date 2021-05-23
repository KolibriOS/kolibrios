#include <stdio.h>
#include <errno.h>
#include <sys/ksys.h>

int fputc(int c, FILE *stream)
{
    if(fwrite(&c, sizeof(char), 1, stream)==1){
        return c;
    }else{
        return EOF;
    }
}