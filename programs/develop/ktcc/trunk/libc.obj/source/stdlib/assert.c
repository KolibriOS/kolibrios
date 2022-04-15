#include <stdio.h>
#include <stdlib.h>

#pragma GCC push_options
#pragma GCC optimize("O0")

void __assert_fail(const char* expr, const char* file, int line, const char* func)
{
    fprintf(stdout, "Assertion failed: %s (%s: %s: %d)\n", expr, file, func, line);
    exit(0);
}

#pragma GCC pop_options