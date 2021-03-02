/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <stdio.h>
#include <stdlib.h>

#include "conio.h"
#include "format_print.h"

int vprintf ( const char * format, va_list arg )
{
  int len = 0;
  char s[4096];

  __con_init();
  len = vsnprintf(s, 4096, format, arg);
  __con_write_string(s, len);
  return(len);
}
