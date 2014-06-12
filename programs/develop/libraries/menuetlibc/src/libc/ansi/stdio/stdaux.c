/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <libc/file.h>
#include <libc/stdiohk.h>

FILE __dj_stdaux = {
  0, 0, 0, 0,
  _IORW | _IONBF,
  4
};
