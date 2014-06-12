/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* ------------------------- rename() for DJGPP -------------------------- */

/*
 *  An implementation of rename() which can move both files AND
 *  directories on the same filesystem (in the DOS world this means
 *  the same logical drive).  Most cases are simply passed to the
 *  DOS Int 21h/AH=56h function.  Special treatment is required for
 *  renaming (moving) directories which don't share their parent
 *  directory, because DOS won't allow this.  This is called ``Prune
 *  and graft'' operation.  Most of the code below handles this
 *  special case.  It recursively creates subdirectories at the
 *  target path, moves regular files there, then deletes the (empty)
 *  directories at the source.
 *
 *  An alternative (and much faster) implementation would be to access
 *  the parent directories of the source and the target at the disk
 *  sector level and rewrite them with BIOS calls.  However, this won't
 *  work for networked drives and will leave the file system in an
 *  inconsistent state, should the machine crash before the operation
 *  is completed.  (A hybrid approach which uses the fast method when
 *  possible and the slow one otherwise, is left as an exercise for the
 *  ambitious readers. ;-)
 */

#include <libc/stubs.h>
#include <libc/bss.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <io.h>
#include <sys/stat.h>
#include <dir.h>
#include <fcntl.h>

// \begin{diamond}[23.02.2007]
// this is the only solution allowed by existing Kolibri system functions
// it is better than nothing :)
// But renaming of large files will be time-consuming operation...
// and renaming of directories is impossible...

int rename(const char *old, const char *new)
{
	int f1,f2;
	char* data;
	int bytes;
	f1 = dosemu_open(old,O_RDONLY);
	if (f1 < 0) {errno = ENOENT; return -1;}
	f2 = dosemu_open(new,O_WRONLY|O_CREAT|O_EXCL);
	if (f2 < 0) {dosemu_close(f1); errno = EACCES; return -1;}
	data = malloc(32768);
	if (!data) {dosemu_close(f2); dosemu_close(f1); errno = ENOMEM; return -1;}
	do
	{
		bytes = dosemu_read(f1, data, 32768);
		if (bytes >= 0)
			bytes = dosemu_write(f2, data, bytes);
	} while (bytes == 32768);
	free(data);
	dosemu_close(f2);
	dosemu_close(f1);
	if (bytes == -1)
	{
		errno = EACCES;
		return -1;
	}
	remove(old);
	return 0;
}

// \end{diamond}[23.02.2007]
