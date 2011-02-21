/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <string.h>

#undef bcopy

void *
bcopy(const void *a, void *b, size_t len)
{
  return memmove(b, a, len);
}
