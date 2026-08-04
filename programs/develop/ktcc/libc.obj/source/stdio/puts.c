/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <stdio.h>
#include <string.h>
#include "../sys/_conio.h"
#include <sys/ksys.h>

int puts(const char *str)
{
    size_t len = strlen(str);
    console_write(str, len);
    return len;
}
