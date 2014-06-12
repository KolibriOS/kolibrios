/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <sys/stat.h>
#include <io.h>
 
int
chmod(const char *filename, int pmode)
{
  int dmode;
  unsigned attr = _chmod(filename, 0, 0);
 
  if (attr == -1)
    return -1;
 
  if(pmode & S_IWUSR)           /* Only implemented toggle is write/nowrite */
    dmode = 0;                  /* Normal file */
  else
    dmode = 1;                  /* Readonly file */
 
  /* Must clear the directory and volume bits, otherwise 214301 fails.
     Unused bits left alone (some network redirectors use them).  */
  if (_chmod(filename, 1, (attr & 0xffe6) | dmode) == -1)
    return -1;
  return 0;
}

