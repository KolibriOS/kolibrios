/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <stddef.h>
#include <termios.h>

int
cfsetspeed (struct termios *termiosp, speed_t speed)
{
  /* check arguments */
  if (termiosp == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  /* set input and output speed */
  termiosp->c_ispeed = speed;
  termiosp->c_ospeed = speed;

  return 0;
}
