/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <unistd.h>
#include <io.h>
#include <assert.h>

static char msg[] = "Abort!\n";

void abort()
{
 __libclog_printf(msg);
 exit(-1);
}
