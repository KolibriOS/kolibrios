#include <stdio.h>
#include <string.h>

int fputs(const char *str, FILE *stream){
    for(size_t i = 0; i < strlen(str); i++){
        if (fputc(str[i], stream) == EOF) {
        	return EOF;
        }
    }
    return 0;
}