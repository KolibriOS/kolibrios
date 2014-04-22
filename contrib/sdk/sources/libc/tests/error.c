/* Error handler for noninteractive utilities
   Copyright (C) 1990-1998, 2000-2005, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Written by David MacKenzie <djm@gnu.ai.mit.edu>.  */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "error.h"

/* This variable is incremented each time `error' is called.  */
unsigned int error_message_count;

char *strerror_r ();

/* The calling program should define program_name and set it to the
   name of the executing program.  */
//extern char *program_name;

static void
print_errno_message (int errnum)
{
  char const *s;

  s = strerror (errnum);
  fprintf (stderr, ": %s", s);
}

static void
error_tail (int status, int errnum, const char *message, va_list args)
{
  vfprintf (stderr, message, args);
  va_end (args);

  ++error_message_count;
  if (errnum)
    print_errno_message (errnum);
  putc ('\n', stderr);
  fflush (stderr);
  if (status)
    exit (status);
}


/* Print the program name and error message MESSAGE, which is a printf-style
   format string with optional args.
   If ERRNUM is nonzero, print its corresponding system error message.
   Exit with status STATUS if it is nonzero.  */
void
error (int status, int errnum, const char *message, ...)
{
  va_list args;

//  fflush (stdout);

//  fprintf (stderr, "%s: ", program_name);

  va_start (args, message);
  error_tail (status, errnum, message, args);

};

/* Sometimes we want to have at most one error per line.  This
   variable controls whether this mode is selected or not.  */

int error_one_per_line;

void
error_at_line (int status, int errnum, const char *file_name,
	       unsigned int line_number, const char *message, ...)
{
  va_list args;

  if (error_one_per_line)
    {
      static const char *old_file_name;
      static unsigned int old_line_number;

      if (old_line_number == line_number
	  && (file_name == old_file_name
	      || strcmp (old_file_name, file_name) == 0))
	/* Simply return and print nothing.  */
	return;

      old_file_name = file_name;
      old_line_number = line_number;
    }

//  fflush (stdout);

  fprintf (stderr, file_name != NULL ? "%s:%d: " : " ",
	   file_name, line_number);

  va_start (args, message);
  error_tail (status, errnum, message, args);

}

