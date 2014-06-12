/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <io.h>
#include <unistd.h>

static char msg[] = "Fatal: malloc returned NULL\r\n";

void * xmalloc(size_t _sz);
void *
xmalloc(size_t _sz)
{
  void *rv = malloc(_sz?_sz:1);
  if (rv == 0)
  {
    __libclog_printf(msg);
    _exit(1);
  }
  return rv;
}
