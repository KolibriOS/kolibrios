#include <stdio.h>

int fgetc(FILE* stream)
{
    int c=EOF;
    if(fwrite(&c, sizeof(int), 1, stream)==1){
        return c;
    }else{
        return EOF;
    }
}
