/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <string.h>

int
setenv (const char *var, const char *val, int replace)
{
  char *prev;

  if (var == (char *)0 || val == (char *)0)
    return -1;

  if ((prev  = getenv (var)) && !replace)
    return 0;
  else
    {
      size_t l_var = strlen (var);
      char *envstr = (char *)alloca (l_var + strlen (val) + 2);
      char *peq    = strchr (var, '=');

      if (*val == '=')
        ++val;
      if (peq)
        l_var = peq - var;

      strncpy (envstr, var, l_var);
      envstr[l_var++] = '=';
      strcpy (envstr + l_var, val);

      return putenv (envstr);
    }
}
