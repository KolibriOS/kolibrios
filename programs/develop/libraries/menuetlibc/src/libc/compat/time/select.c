/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* An implementation of select()

   Copyright 1995 by Morten Welinder
   This file maybe freely distributed and modified as long as the
   copyright notice remains.

   Notes: In a single process system as Dos this really boils down to
   something that can check whether a character from standard input
   is ready.  However, the code is organised in a way to make it easy
   to extend to multi process systems like WinNT and OS/2.  */

#include <libc/stubs.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libc/file.h>
#include <libc/local.h>
#include <libc/dosio.h>
#include <sys/fsext.h>

inline static int fp_output_ready(FILE *fp)
{
  return !ferror(fp);
}

/* This is as close as we get, I think.  For a file connected to a printer
   we could of course go ask the BIOS, but this should be enough.  */

inline static int fp_except_ready(FILE *fp)
{
  return ferror (fp);
}

inline static int fp_input_ready (FILE *fp)
{
  /* I think if there is something in the buffer, we should return
     ``ready'', even if some error was encountered.  Let him consume
     the buffered characters, *then* return ``not ready''.  */
  if (fp->_cnt)
    return 1;

  /* The `feof' part is only correct in a single-tasked environment.  */
  if (ferror (fp) || feof (fp))
    return 0;

  /* There is nothing in the buffer (perhaps because we read unbuffered).
     We don't know if we are ready.  Return ``ready'' anyway and let
     read() or write() tell the truth.  */
  return 1;
}

/* The Dos call 4407 always returns TRUE for disk files.  So the
   following really is meaningful for character devices only...  */

inline static int fd_output_ready(int fd)
{
 return 0;
}

inline static int fd_input_ready(int fd)
{
 return 0;
}

int select(int nfds,fd_set *readfds,fd_set *writefds,fd_set *exceptfds,
	struct timeval *timeout)
{
  int ready;
  fd_set oread, owrite, oexcept;
  struct timeval now, then;

  if (nfds > FD_SETSIZE)
  {
    errno = EINVAL;
    return -1;
  }

  FD_ZERO (&oread);
  FD_ZERO (&owrite);
  FD_ZERO (&oexcept);
  ready = 0;

  if (timeout)
  {
    if (timeout->tv_usec < 0)
    {
      errno = EINVAL;
      return -1;
    }
    gettimeofday (&now, 0);
    then.tv_usec = timeout->tv_usec + now.tv_usec;
    then.tv_sec = timeout->tv_sec + now.tv_sec + then.tv_usec / 1000000;
    then.tv_usec %= 1000000;
  }

  do {
    int i;
    int fd0 = 0;
    __file_rec *fr = __file_rec_list;
    FILE *fp;

    /* First, check the file handles with low-level DOS calls.  */
    for (i = 0; i < nfds; i++)
    {
      register int ioctl_result;
      __FSEXT_Function *func = __FSEXT_get_function(i);
      int fsext_ready = -1;

      if (func)
	func(__FSEXT_ready, &fsext_ready, &i);

      if (readfds && FD_ISSET (i, readfds))
      {
	if (fsext_ready != -1)
	{
	  if (fsext_ready & __FSEXT_ready_read)
	    ready++, FD_SET(i, &oread);
	}
        else if ((ioctl_result = fd_input_ready (i)) == -1)
          return -1;
        else if (ioctl_result)
          ready++, FD_SET (i, &oread);
      }
      if (writefds && FD_ISSET (i, writefds))
      {
        if (fsext_ready != -1)
	{
	  if (fsext_ready & __FSEXT_ready_write)
	    ready++, FD_SET(i, &owrite);
	}
        else if ((ioctl_result = fd_output_ready (i)) == -1)
          return -1;
        else if (ioctl_result)
          ready++, FD_SET (i, &owrite);
      }
      if (exceptfds && FD_ISSET (i, exceptfds))
      {
        if (fsext_ready != -1)
	{
	  if (fsext_ready & __FSEXT_ready_error)
	    ready++, FD_SET(i, &oexcept);
	}
      }
    }

    /* Now look at the table of FILE ptrs and reset the bits for file
       descriptors which we *thought* are ready, but for which the flags
       say they're NOT ready.  */
    for (i = 0; fr; i++)
    {
      if (i >= fd0 + fr->count) /* done with this subtable, go to next */
      {
	fd0 += fr->count;
	fr = fr->next;
      }
      if (fr)
      {
        fp = fr->files[i - fd0];
        if (fp->_flag)
        {
          int this_fd = fileno(fp);

          if (this_fd < nfds)
          {
            if (readfds && FD_ISSET (this_fd, readfds) &&
                FD_ISSET (this_fd, &oread) && !fp_input_ready (fp))
              ready--, FD_CLR (this_fd, &oread);
            if (writefds && FD_ISSET (this_fd, writefds) &&
                FD_ISSET (this_fd, &owrite) && !fp_output_ready (fp))
              ready--, FD_CLR (this_fd, &owrite);

            /* For exceptional conditions, ferror() is the only one
               which can tell us an exception is pending.  */
            if (exceptfds && FD_ISSET (this_fd, exceptfds) &&
                fp_except_ready (fp))
              ready++, FD_SET (this_fd, &oexcept);
          }
        }
      }
    }

    /* Exit if we found what we were waiting for.  */
    if (ready > 0)
    {
      if (readfds)
	*readfds = oread;
      if (writefds)
	*writefds = owrite;
      if (exceptfds)
	*exceptfds = oexcept;
      return ready;
    }

    /* Exit if we hit the time limit.  */
    if (timeout)
    {
      gettimeofday (&now, 0);
      if (now.tv_sec > then.tv_sec
	  || (now.tv_sec = then.tv_sec && now.tv_usec >= then.tv_usec))
	return 0;
    }
  } while (1);
}
