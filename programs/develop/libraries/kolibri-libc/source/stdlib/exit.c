/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include "../stdio/conio.h"
#include <sys/ksys.h>

void exit(int status)
{
    if(__con_is_load){
        __con_exit(status);
    }
    _ksys_exit();
}
