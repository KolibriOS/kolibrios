/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

void xfree(void *_ptr);
void
xfree(void *_ptr)
{
  if (_ptr)
    free(_ptr);
}
