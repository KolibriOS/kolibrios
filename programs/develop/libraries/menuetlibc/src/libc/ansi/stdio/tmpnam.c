/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <libc/bss.h>

static char *tmp_dir;
static int tmp_len;
static int tmp_bss_count = -1;

static void
try(const char *var)
{
  static char buf[L_tmpnam];

  char *t = getenv(var);
  if (t == 0)
    return;

  tmp_len = strlen(t);
  strcpy(buf, t);
  if (buf[tmp_len - 1] != '/' && buf[tmp_len - 1] != '\\')
    buf[tmp_len++] = '/', buf[tmp_len] = 0;

  if (access(buf, D_OK))
    return;

  tmp_dir = buf;
}

char *
tmpnam(char *s)
{
  static char static_buf[L_tmpnam];
  static char tmpcount[] = "dj000000";
  int i;

  if (tmp_bss_count != __bss_count)
  {
    tmp_bss_count = __bss_count;

    if (tmp_dir == 0) try("TMPDIR");
    if (tmp_dir == 0) try("TEMP");
    if (tmp_dir == 0) try("TMP");
    if (tmp_dir == 0)
    {
      static char def[] = "c:/";
      tmp_dir = def;
      tmp_len = 3;
    }
  }

  if (!s)
    s = static_buf;
  strcpy(s, tmp_dir);

  do {
    /* increment the "count" starting at the first digit (backwards order) */
    for (i=2; tmpcount[i] == '9' && i < 8; tmpcount[i] = '0', i++);
    if (i < 8)
      tmpcount[i]++;

    strcpy(s+tmp_len, tmpcount);

  } while (access(s, F_OK)==0); /* until file doesn't exist */

  return s;
}
