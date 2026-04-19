/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <stdio.h>
#include <string.h>
#include "conio.h"
#include "sys/ksys.h"

int puts(const char *str)
{
    con_init();
    con_write_asciiz(str);
    con_write_asciiz("\n");
    return strlen(str);
}