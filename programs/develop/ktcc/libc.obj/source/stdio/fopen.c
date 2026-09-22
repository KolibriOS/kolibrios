#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "_stdio.h"

FILE* fopen(const char* restrict _name, const char* restrict _mode)
{
    FILE* stream = malloc(sizeof(FILE));

    if (stream == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    FILE* out = fopen_r(_name, _mode, stream);

    if (out == NULL) {
        free(stream);
    }

    return out;
}
