/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdlib.h>

char *
getpass(const char *prompt)
{
  static char password_buffer[9];

  if (getlongpass(prompt, password_buffer, 9) < 0)
    return 0;
  return password_buffer;
}

