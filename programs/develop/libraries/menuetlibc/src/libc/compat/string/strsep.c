/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <string.h>

char *
strsep(char **stringp, const char *delim)
{
  char *s;
  const char *spanp;
  int c, sc;
  char *tok;

  if ((s = *stringp) == 0)
    return 0;

  tok = s;
  while (1)
  {
    c = *s++;
    spanp = delim;
    do {
      if ((sc = *spanp++) == c)
      {
	if (c == 0)
	  s = 0;
	else
	  s[-1] = 0;
	*stringp = s;
	return tok;
      }
    } while (sc != 0);
  }
}
