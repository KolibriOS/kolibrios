/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* ftw() for DJGPP.
 *
 * Recursively descent the directory hierarchy rooted in DIR,
 * calling FUNC for each object in the hierarchy.
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
#include <sys/stat.h>
#include <dirent.h>
#include <io.h>
#include <ftw.h>

static int
walk_dir(char *path, int (*func)(const char *, struct stat *, int))
{
  DIR *dp;
  struct dirent *de;
  struct stat stbuf;
  int flag;
  int e = errno;
  int pathlen = strlen(path);

  if ((dp = opendir(path)) == 0)
    return -1;

  for (errno = 0; (de = readdir(dp)) != 0; errno = 0)
    {
      int func_result;
      char lastc = de->d_name[de->d_namlen - 1];

      /* Skip `.' and `..' entries.  Checking the last char is enough,
         because readdir will never return a filename which ends with
         a dot.  */
      if (lastc == '.')
        continue;

      /* Append found name to directory path.  */
      if (pathlen + de->d_namlen + 1 > FILENAME_MAX)
        {
          (void)closedir(dp);
          errno = ENAMETOOLONG;
          return -1;
        }
      if (path[pathlen-1] == '/' || path[pathlen-1] == '\\')
        pathlen--;
      path[pathlen] = '/';
      strcpy(path + pathlen + 1, de->d_name);

      if (stat(path, &stbuf) < 0)
        flag = FTW_NS;
      else if (S_ISDIR(stbuf.st_mode))
        flag = FTW_D;
      else if (S_ISLABEL(stbuf.st_mode))
        flag = FTW_VL;
      else
        flag = FTW_F;

      /* Invoke FUNC() on this object.  */
      errno = e;
      if ((func_result = (*func)(path, &stbuf, flag)) != 0)
        {
          (void)closedir(dp);
          return func_result;
        }

      /* If this is a directory, walk its siblings.  */
      if (flag == FTW_D)
        {
          int subwalk_result;

          errno = e;
          if ((subwalk_result = walk_dir(path, func)) != 0)
            {
              (void)closedir(dp);
              return subwalk_result;
            }
        }

      /* Erase D_NAME[] from PATH.  */
      path[pathlen] = '\0';
    }

  (void)closedir(dp);
  if (errno == 0)   /* normal case: this subtree exhausted */
    {
      errno = e;/* restore errno from previous syscall */
      return 0;
    }
  else
    return -1;      /* with whatever errno was set by readdir() */
}

int
ftw(const char *dir, int (*func)(const char *, struct stat *, int),
    int ignored)
{
  int flag;
  unsigned char pathbuf[FILENAME_MAX];
  int dirattr;
  int len;
  int e = errno;

  ignored = ignored;        /* pacify -Wall */

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

  strcpy(pathbuf, dir);
  len = strlen(pathbuf);
  if (pathbuf[len-1] == ':')
    {
      pathbuf[len++] = '.';
      pathbuf[len] = '\0';
    }

  /* Fail for non-directories.  */
  errno = 0;
  dirattr = _chmod(pathbuf, 0, 0);
  if (errno == ENOENT)
    return -1;
  else if ((dirattr & 0x10) != 0x10)
    {
      errno = ENOTDIR;
      return -1;
    }
  else
    {
      int func_result;
      struct stat stbuf;

      if (stat(pathbuf, &stbuf) < 0)
        flag = FTW_NS;
      else
        flag = FTW_D;
      errno = e;
      if ((func_result = (*func)(pathbuf, &stbuf, flag)) != 0)
        return func_result;
      
      return walk_dir(pathbuf, func);
    }
}

#ifdef  TEST

#include <stdlib.h>

int
ftw_walker(const char *path, struct stat *sb, int flag)
{
  char *base;

  printf("%s:\t%u\t", path, sb->st_size);
  if (S_ISLABEL(sb->st_mode))
    printf("V");
  if (S_ISDIR(sb->st_mode))
    printf("D");
  if (S_ISCHR(sb->st_mode))
    printf("C");
  if (sb->st_mode & S_IRUSR)
    printf("r");
  if (sb->st_mode & S_IWUSR)
    printf("w");
  if (sb->st_mode & S_IXUSR)
    printf("x");

  if (flag == FTW_NS)
    printf("  !!no_stat!!");
  printf("\n");

  base = strrchr(path, '/');
  if (base == 0)
    base = strrchr(path, '\\');
  if (base == 0)
    base = strrchr(path, ':');
  if (strcmp(base == 0 ? path : base + 1, "xxxxx") == 0)
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
                   ftw(argv[1], ftw_walker, 0));
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
