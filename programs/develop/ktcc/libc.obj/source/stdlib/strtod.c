/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <limits.h>

double strtod(const char* s, char** sret)
{
    return strntold(s, sret, UINT_MAX);
}
