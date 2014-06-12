/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <libc/file.h>
#include <libc/stdiohk.h>

FILE __dj_stdout = {
  0, 0, 0, 0,
  _IOWRT | _IOFBF,
  1
};
