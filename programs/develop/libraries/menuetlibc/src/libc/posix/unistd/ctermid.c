/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <string.h>

static char def_termid[] = "con";

char *
ctermid(char *_s)
{
  if (!_s)
    return def_termid;
  strcpy(_s, def_termid);
  return _s;
}
