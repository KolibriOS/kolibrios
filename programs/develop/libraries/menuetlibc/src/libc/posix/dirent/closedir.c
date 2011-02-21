/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>
#include <dirent.h>
#include "dirstruc.h"

int
closedir(DIR *dir)
{
  free(dir);
  return 0;
}
