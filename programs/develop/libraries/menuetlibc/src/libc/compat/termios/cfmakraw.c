/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include <stddef.h>
#include <termios.h>
#include <sys/exceptn.h>

void cfmakeraw (struct termios *termiosp)
{
  /* check arguments */
  if (termiosp == NULL)
    {
      errno = EINVAL;
      return;
    }
  termiosp->c_iflag &= ~(BRKINT|ICRNL|IGNBRK|IGNCR|INLCR|ISTRIP|PARMRK|IXON);
#if defined (IMAXBEL)
  termiosp->c_iflag &= ~IMAXBEL;
#endif
  termiosp->c_oflag &= ~OPOST;
  termiosp->c_lflag &= ~(ECHONL|ICANON|IEXTEN|ISIG);
  termiosp->c_cflag &= ~(CSIZE|PARENB);
  termiosp->c_cflag |= CS8;
  termiosp->c_cc[VMIN] = 1;
  termiosp->c_cc[VTIME] = 0;
}
