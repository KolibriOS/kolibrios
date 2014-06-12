#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<menuet/os.h>

#define _MAX_HANDLES		64

#define _IO_READ		1
#define _IO_WRITE		2
#define _IO_BUFDIRTY		4

#define IODEBUG(x...)		/* */

typedef struct
{
 int size;
 int oflags;
 int flags;
 int pointer;
 char filename[512];
} _io_struct;

extern _io_struct _io_handles[_MAX_HANDLES];
