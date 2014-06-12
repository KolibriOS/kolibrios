/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

void
cfree(void *_ptr)
{
  free(_ptr);
}
