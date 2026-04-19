#include <stdio.h>
#include <string.h>

int fputs(const char *str, FILE *stream){
    size_t str_len = strlen(str);
    if(str_len == fwrite(str, sizeof(char), str_len , stream)){
        return str[str_len-1];
    }else{
        return EOF;
    }
}