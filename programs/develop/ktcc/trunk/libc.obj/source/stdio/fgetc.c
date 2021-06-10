#include <stdio.h>

int fgetc(FILE* stream)
{
    int c=0;
    if(fread(&c, sizeof(char), 1, stream)==1){
        return c;
    }else{
        return EOF;
    }
}
