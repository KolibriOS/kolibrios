#include <stdio.h>
#include <stdlib.h>
#include "_stdio.h"

FILE* freopen(const char* restrict _name, const char* restrict _mode, FILE* restrict out)
{
    fclose_r(out);

    if (_name == NULL) {
        _name = out->name;
    }

    return fopen_r(_name, _mode, out);
}
