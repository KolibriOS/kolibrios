/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <pc.h>

int
getlongpass(const char *prompt, char *password, int max_length)
{
  char *p = password;
  int c, count = 0;

  fflush(stdout);
  /* If we can't prompt, abort */
  if (fputs(prompt, stderr) < 0)
  {
    *p = '\0';
    return -1;
  }

  while (1)
  {
    /* Get a character with no echo */
    c = getkey();

    /* Exit on interrupt (^c or ^break) */
    if (c == '\003' || c == 0x100)
      exit(1);

    /* Terminate on end of line or file (^j, ^m, ^d, ^z) */
    if (c == '\r' || c == '\n' || c == '\004' || c == '\032')
      break;

    /* Back up on backspace */
    if (c == '\b')
    {
      if (count)
	count--;
      else if (p > password)
	  p--;
      continue;
    }

    /* Ignore DOS extended characters */
    if ((c & 0xff) != c)
      continue;

    /* Add to password if it isn't full */
    if (p < password + max_length - 1)
      *p++ = c;
    else
      count++;
  }
  *p = '\0';

  fputc('\n', stderr);

  return 0;
}
