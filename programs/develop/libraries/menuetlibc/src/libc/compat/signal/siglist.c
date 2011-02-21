/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */

#include <signal.h>
#include <string.h>
#include <stdlib.h>

char *sys_siglist[NSIG + 1]; /* initially all-zero */

static const char *known_signal[] = {
  "Abort termination",
  "Floating-point exception",
  "Illegal instruction",
  "Segmentation violation",
  "Software termination signal",
  "Alarm clock",
  "Hangup",
  "Interrupt",
  "Kill",
  "Write on pipe with no one to read it",
  "Quit",
  "User-defined signal 1",
  "User-defined signal 2",
  "Floating-point co-processor not present",
  "Debugger/Breakpoint instruction",
  "Timer tick signal",
  "Profiler signal"
};

static char unknown_signal[]  = "Unknown signal";

static void
put_hex_digits (char *str, int num, size_t idx)
{
  static char xdigits[] = "0123456789ABCDEF";

  str[idx] = xdigits[num / 16];
  str[idx + 1] = xdigits[num & 15];
}

static char *
xstrdup (const char *src)
{
  if (src)
  {
    size_t src_size = strlen (src) + 1;
    char *new = (char *)malloc (src_size);

    if (new)
    {
      memcpy (new, src, src_size);
      return new;
    }
  }

  return NULL;
}

static int signum;

static void
fill_dull_names (const char *template, size_t tpl_size, int count)
{
  int i;

  for (i = 0; i < count; i++)
  {
    char *signame = (char *)malloc (tpl_size);

    memcpy (signame, template, tpl_size);
    put_hex_digits (signame, i, tpl_size - 3);
    sys_siglist[signum++] = signame;
  }
}

static void __attribute__((constructor))
init_sys_siglist (void)
{
  static char int_name[]   = "Interrupt XXh";
  static size_t int_size   = sizeof(int_name);
  static char excpt_name[] = "Exception XXh";
  static size_t excpt_size = sizeof(excpt_name);
  int i;

  signum = 0;

  fill_dull_names (int_name, int_size, 256);
  fill_dull_names (excpt_name, excpt_size, 32);

  for (i = 0; i < 17; i++)
    sys_siglist[signum++] = xstrdup (known_signal[i]);

  for (i = 305; i < 320; i++)
    sys_siglist[signum++] = xstrdup (unknown_signal);

  sys_siglist[signum] = 0;
}
