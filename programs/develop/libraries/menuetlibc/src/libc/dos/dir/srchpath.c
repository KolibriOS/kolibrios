/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <dir.h>
 
/* Search PATH for FILE.
   If successful, store the full pathname in static buffer and return a
   pointer to it.
   If not sucessful, return NULL.
   This is what the Borland searchpath() library function does.
*/
 
char *
searchpath(const char *file)
{
  static char found[PATH_MAX];
  static char *path;

  memset(found, 0, sizeof(found));
 
  /* Get the PATH and store it for reuse.  */
  if (path == 0)
  {
    char *p = getenv("PATH");
 
    path = (char *)calloc(p ? strlen(p) + 3 : 2, sizeof(char));
    if (path == (char *)0)
      return (char *)0;
 
    /* Prepend `.' to the PATH, so current directory
       is always searched.  */
    path[0] = '.';
 
    if (p)
    {
      register char *s;
 
      path[1] = ';';
      strcpy(path+2, p);
 
      /* Convert to more plausible form.  */
      for (s = path; *s; ++s)
      {
	if (*s == '\\')
	  *s = '/';
	if (isupper(*s))
	  *s = tolower(*s);
      }
    }
    else
      path[1] = 0;
  }
  if (strpbrk (file, "/\\:") != 0)
  {
    strcpy(found, file);
    return found;
  }
  else
  {
    char *test_dir = path;
 
    do {
      char *dp;
 
      dp = strchr(test_dir, ';');
      if (dp == (char *)0)
	dp = test_dir + strlen(test_dir);
 
      if (dp == test_dir)
	strcpy(found, file);
      else
      {
	strncpy(found, test_dir, dp - test_dir);
	found[dp - test_dir] = '/';
	strcpy(found + (dp - test_dir) + 1, file);
      }

      if (__file_exists(found))
	return found;

      if (*dp == 0)
	break;
      test_dir = dp + 1;
    } while (*test_dir != 0);
  }
 
  return NULL;
}
