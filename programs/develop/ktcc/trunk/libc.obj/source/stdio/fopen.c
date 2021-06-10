#include <stdio.h>
#include <stdlib.h>

FILE *fopen(const char *restrict _name, const char *restrict _mode) {
    FILE *out = malloc(sizeof(FILE));
    return freopen(_name, _mode, out);
}
