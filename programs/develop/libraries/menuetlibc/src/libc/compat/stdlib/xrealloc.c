/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <io.h>
#include <unistd.h>

static char msg[] = "Fatal: xrealloc would have returned NULL\r\n";

void * xrealloc(void *ptr, size_t _sz);
void *
xrealloc(void *ptr, size_t _sz)
{
  void *rv;

  if (ptr == 0)
    rv = malloc(_sz?_sz:1);
  else
    rv = realloc(ptr, _sz?_sz:1);

  if (rv == 0)
  {
    __libclog_printf(msg);
    _exit(1);
  }
  return rv;
}
