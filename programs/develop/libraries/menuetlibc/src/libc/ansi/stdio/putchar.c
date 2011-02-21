/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <libc/file.h>

#undef putchar
int
putchar(int c)
{
  return fputc(c, stdout);
}
