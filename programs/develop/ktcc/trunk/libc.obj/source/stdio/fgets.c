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
    
    while (i<n-1){
        sym_code = fgetc(stream);
        if(sym_code =='\n' || sym_code == EOF){ break; }
        str[i]=(char)sym_code;
        i++;
    }
    
    if(i<1){ return NULL; }
    return str;
}
    