#include <stdio.h>
#include <string.h>

int fputs(const char *str, FILE *stream){
    int s_code;
    for(int i=0; i<strlen(str) && s_code!=EOF; i++){
        s_code = fputc(str[i], stream);
    }
    return s_code;
}