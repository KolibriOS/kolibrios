#include <stdio.h>

int fputc(int sym, FILE *stream)
{
    if(!fwrite(&sym, sizeof(char), 1, stream)){
        return EOF;
    }
    return sym;
}