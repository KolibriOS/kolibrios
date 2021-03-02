#include <stdio.h>

int fgetc(FILE* stream)
{
    int c, rc;
    rc = fread(&c, sizeof(char), 1, stream);
    if(rc<1){
        return EOF;
    }
    return c;
}
