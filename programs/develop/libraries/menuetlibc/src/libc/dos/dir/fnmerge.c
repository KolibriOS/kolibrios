/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dir.h>
#include <string.h>

void
fnmerge (char *path, const char *drive, const char *dir,
	 const char *name, const char *ext)
{
  *path = '\0';
  if (drive && *drive)
  {
    path[0] = drive[0];
    path[1] = ':';
    path[2] = 0;
  }
  if (dir && *dir)
  {
    char last_dir_char = dir[strlen(dir) - 1];

    strcat(path, dir);
    if (last_dir_char != '/' && last_dir_char != '\\')
      strcat(path, strchr(dir, '\\') ? "\\" : "/");
  }
  if (name)
    strcat(path, name);
  if (ext && *ext)
  {
    if (*ext != '.')
      strcat(path, ".");
    strcat(path, ext);
  }
}
