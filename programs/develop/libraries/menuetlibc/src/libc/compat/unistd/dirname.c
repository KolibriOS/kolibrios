/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *
dirname (const char *fname)
{
  const char *p  = fname;
  const char *slash = NULL;

  if (fname)
  {
    size_t dirlen;
    char * dirpart;

    if (*fname && fname[1] == ':')
    {
      slash = fname + 1;
      p += 2;
    }

    /* Find the rightmost slash.  */
    while (*p)
    {
      if (*p == '/' || *p == '\\')
	slash = p;
      p++;
    }

    if (slash == NULL)
    {
      fname = ".";
      dirlen = 1;
    }
    else
    {
      /* Remove any trailing slashes.  */
      while (slash > fname && (slash[-1] == '/' || slash[-1] == '\\'))
	slash--;

      /* How long is the directory we will return?  */
      dirlen = slash - fname + (slash == fname || slash[-1] == ':');
      if (*slash == ':' && dirlen == 1)
	dirlen += 2;
    }

    dirpart = (char *)malloc (dirlen + 1);
    if (dirpart != NULL)
    {
      strncpy (dirpart, fname, dirlen);
      if (slash && *slash == ':' && dirlen == 3)
	dirpart[2] = '.';	/* for "x:foo" return "x:." */
      dirpart[dirlen] = '\0';
    }

    return dirpart;
  }

  return NULL;
}
