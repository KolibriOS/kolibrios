/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <stdlib.h>
#include <stdio.h>
#include "atexit.h"

/*
TODO
static void __close_all()
{
}

static void __free_all_mem()
{
}
*/

void exit(int status)
{
    __run_atexit();

    if(status)
    {
        fprintf(stderr, "Exit code: %d\n", status);
    }

    //__close_all();
    //__free_all_mem();

    _exit(status);
}
