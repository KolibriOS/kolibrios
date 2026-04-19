#include <stdio.h>
#include "conio.h"
#include <errno.h>

char *fgets(char *str, int n, FILE *stream)
{
    int i=0, sym_code;

    if(!stream || !str){
        errno = EINVAL;
        return NULL;
    }
    
    i = fread(str, n-1, sizeof(char), stream);
    if(i<1){ return NULL; }
    return str;
}
    
