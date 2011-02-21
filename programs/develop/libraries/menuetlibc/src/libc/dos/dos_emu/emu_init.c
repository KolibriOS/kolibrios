#include "dosemuin.h"

_io_struct _io_handles[_MAX_HANDLES];

void dosemu_inithandles(void)
{
 int i;
 for(i=0;i<_MAX_HANDLES;i++)
  _io_handles[i].oflags=-1;
 _io_handles[0].oflags=1;
 _io_handles[1].oflags=1;
 _io_handles[2].oflags=1;
 _io_handles[3].oflags=1;
}

/* If you want to do some actions for closing handles,
   you must add it to this function
   and uncomment call to atexit(dosemu_atexit) in crt1.c.
   In this case I recommend to implement all referenced functions
   here (and not in dosemu.c) to avoid linking dosemu.o
   in programs which do not use I/O system. - diamond */
//void dosemu_atexit()
//{}

char __curdir_buf[1024];
extern char __menuet__app_path_area[];

void init_dir_stack(void)
{
	strcpy(__curdir_buf,__menuet__app_path_area);
	*strrchr(__curdir_buf,'/') = 0;
}
