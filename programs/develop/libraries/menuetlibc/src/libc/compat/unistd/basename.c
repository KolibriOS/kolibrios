/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
#include <unistd.h>
#include <libc/unconst.h>

char *
basename (const char *fname)
{
  const char *base = fname;

  if (fname && *fname)
  {
    if (fname[1] == ':')
    {
      fname += 2;
      base = fname;
    }

    while (*fname)
    {
      if (*fname == '\\' || *fname == '/')
	base = fname + 1;
      fname++;
    }
  }

  return unconst (base, char *);
}
