/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <string.h>

void *
memccpy(void *t, const void *f, int c, size_t n)
{
  c &= 0xff;
  if (n)
  {
    unsigned char *tt = (unsigned char *)t;
    const unsigned char *ff = (const unsigned char *)f;

    do {
      if ((*tt++ = *ff++) == c)
	return tt;
    } while (--n != 0);
  }
  return 0;
}
