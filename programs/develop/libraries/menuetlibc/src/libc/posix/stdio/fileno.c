/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <libc/file.h>

#undef fileno
int
fileno(FILE *stream)
{
  return stream ? stream->_file : -1;
}
