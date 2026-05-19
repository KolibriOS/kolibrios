/*
* SPDX-License-Identifier: GPL-2.0-only
* Copyright (C) 2026 KolibriOS team
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ksys.h>

void abort(void)
{
    ksys_thread_t t;
    _ksys_thread_info(&t, -1);
    fprintf(stderr, "\nAborted pid: %d\n", t.pid);

    _exit(128);
}
