#include<limits.h>

float strtof(const char* s, char** sret)
{
    return strntold(s, sret, UINT_MAX);
}
