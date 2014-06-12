/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <locale.h>
#include <string.h>

char *setlocale(int category, const char *locale)
{
  static char CLOCALE[] = "C";
  if (locale == 0)
    return CLOCALE;
  if (strcmp(locale, CLOCALE) && strcmp(locale, "POSIX") && locale[0])
    return 0;
  return CLOCALE;
}
