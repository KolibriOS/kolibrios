/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <libc/file.h>
#include <libc/local.h>

void
_fwalk(void (*func)(FILE *))
{
  __file_rec *fr;
  int i;

  for (fr=__file_rec_list; fr; fr=fr->next)
    for (i=0; i<fr->count; i++)
      if (fr->files[i]->_flag)
	func(fr->files[i]);
}
