#include<limits.h>

long double strtold(const char* s, char** sret)
{
    return strntold(s, sret, UINT_MAX);
}
