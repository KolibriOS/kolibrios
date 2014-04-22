/*
FUNCTION
<<system>>---execute command string

INDEX
	system
INDEX
	_system_r

ANSI_SYNOPSIS
	#include <stdlib.h>
	int system(char *<[s]>);

	int _system_r(void *<[reent]>, char *<[s]>);

TRAD_SYNOPSIS
	#include <stdlib.h>
	int system(<[s]>)
	char *<[s]>;

	int _system_r(<[reent]>, <[s]>)
	char *<[reent]>;
	char *<[s]>;

DESCRIPTION

Use <<system>> to pass a command string <<*<[s]>>> to <</bin/sh>> on
your system, and wait for it to finish executing.

Use ``<<system(NULL)>>'' to test whether your system has <</bin/sh>>
available.

The alternate function <<_system_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
<<system(NULL)>> returns a non-zero value if <</bin/sh>> is available, and
<<0>> if it is not.

With a command argument, the result of <<system>> is the exit status
returned by <</bin/sh>>.

PORTABILITY
ANSI C requires <<system>>, but leaves the nature and effects of a
command processor undefined.  ANSI C does, however, specify that
<<system(NULL)>> return zero or nonzero to report on the existence of
a command processor.

POSIX.2 requires <<system>>, and requires that it invoke a <<sh>>.
Where <<sh>> is found is left unspecified.

Supporting OS subroutines required: <<_exit>>, <<_execve>>, <<_fork_r>>,
<<_wait_r>>.
*/

#include <_ansi.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <_syslist.h>
#include <reent.h>


int
_DEFUN(_system_r, (ptr, s),
     struct _reent *ptr _AND
     _CONST char *s)
{
  if (s == NULL)
    return 0;
  errno = ENOSYS;
  return -1;
}

#ifndef _REENT_ONLY

int
_DEFUN(system, (s),
     _CONST char *s)
{
  return _system_r (_REENT, s);
}

#endif
