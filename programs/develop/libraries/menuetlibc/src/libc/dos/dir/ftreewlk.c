/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * Recursively descent the directory hierarchy rooted in DIR,
 * calling FUNC for each object in the hierarchy.  We cannot
 * use ftw(), because it uses some non-ANSI functions which
 * will pollute ANSI namespace, while we need this function
 * in some ANSI functions (e.g., rename()).  Thus, this function
 * is closely modeled on ftw(), but uses DOS directory search
 * functions and structures instead of opendir()/readdir()/stat().
 *
 * Copyright (c) 1995 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely as long as this copyright notice is
 * left intact.  There is no warranty on this software.
 *
 */

#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <dir.h>

#define FA_ALL  (FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_LABEL|FA_DIREC|FA_ARCH)

int
__file_tree_walk(const char *dir,
               int (*func)(const char *, const struct ffblk *))
{
  struct ffblk  ff;
  unsigned char searchspec[FILENAME_MAX];
  unsigned char found[FILENAME_MAX], *dir_end;
  int e = errno;

  if (dir == 0 || func == 0)
    {
      errno = EFAULT;
      return -1;
    }

  if (*dir == '\0')
    {
      errno = ENOENT;
      return -1;
    }

  /* Construct the search spec for findfirst().  Treat ``d:'' as ``d:.''.  */
  strcpy(searchspec, dir);
  dir_end = searchspec + strlen(searchspec) - 1;
  if (*dir_end == ':')
    {
      *++dir_end = '.';
      *++dir_end = '\0';
    }
  else if (*dir_end == '/' || *dir_end == '\\')
    *dir_end   = '\0';
  else
    ++dir_end;
  strcpy(dir_end, "/*.*");

  /* Prepare the buffer where the full pathname of the found files
     will be placed.  */
  strcpy(found, dir);
  dir_end = found + strlen(found) - 1;
  if (*dir_end == ':')
    {
      *++dir_end = '.';
      dir_end[1] = '\0';
    }
  if (*dir_end != '/' && *dir_end != '\\')
    {
      /* Try to preserve user's forward/backward slash style.  */
      *++dir_end = strchr(found, '\\') == 0 ? '/': '\\';
      *++dir_end   = '\0';
    }
  else
    ++dir_end;

  if (findfirst(searchspec, &ff, FA_ALL))
    return -1;

  do
    {
      int func_result;
      unsigned char *p = dir_end;

      /* Skip `.' and `..' entries.  */
      if (ff.ff_name[0] == '.' &&
          (ff.ff_name[1] == '\0' || ff.ff_name[1] == '.'))
        continue;

      /* Construct full pathname in FOUND[].  */
      strcpy(dir_end, ff.ff_name);

      /* Convert name of found file to lower-case.  Cannot use
         strlwr() because it's non-ANSI.  Sigh... */
      while (*p)
        *p++ = tolower(*p);

      /* Invoke FUNC() on this file.  */
      if ((func_result = (*func)(found, &ff)) != 0)
        return func_result;

      /* If this is a directory, walk its siblings.  */
      if (ff.ff_attrib & 0x10)
        {
          int subwalk_result;

          if ((subwalk_result = __file_tree_walk(found, func)) != 0)
            return subwalk_result;
        }
    } while (findnext(&ff) == 0);

  if (errno == ENMFILE)     /* normal case: tree exhausted */
    {
      errno = e;    /* restore errno from previous syscall */
      return 0;
    }

  return -1;                /* error; errno set by findnext() */
}

#ifdef  TEST

#include <stdlib.h>

int
ff_walker(const char *path, const struct ffblk *ff)
{
  printf("%s:\t%lu\t", path, ff->ff_fsize);
  if (ff->ff_attrib & 1)
    printf("R");
  if (ff->ff_attrib & 2)
    printf("H");
  if (ff->ff_attrib & 4)
    printf("S");
  if (ff->ff_attrib & 8)
    printf("V");
  if (ff->ff_attrib & 0x10)
    printf("D");
  if (ff->ff_attrib & 0x20)
    printf("A");
  printf("\n");

  if (strcmp(ff->ff_name, "XXXXX") == 0)
    return 8;
  return 0;
}

int
main(int argc, char *argv[])
{
  if (argc > 1)
    {
      char msg[80];

      sprintf(msg, "file_tree_walk: %d",
                   file_tree_walk(argv[1], ff_walker));
      if (errno)
        perror(msg);
      else
        puts(msg);
    }
  else
    printf("Usage: %s dir\n", argv[0]);

  return 0;
}

#endif
