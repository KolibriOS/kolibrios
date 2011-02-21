/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>

int
puts(const char *s)
{
  int c;

  while ((c = *s++))
    putchar(c);
  return putchar('\n');
}
