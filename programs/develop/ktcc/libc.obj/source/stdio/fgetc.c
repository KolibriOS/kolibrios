#include <stdio.h>
#include <stdint.h>

int fgetc(FILE* stream)
{
    uint8_t c = 0;
    
    if (fread(&c, sizeof(c), 1, stream) == 1) {
        return c;
    } else {
        return EOF;
    }
}
