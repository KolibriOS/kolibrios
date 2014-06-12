#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>

#include <libc/dosio.h>
#include <libc/bss.h>

#include<menuet/os.h>
#include<menuet/console.h>

static char *sbuf = 0;
static size_t sbuflen = 0;

static int write_count = -1;

ssize_t write(int handle, const void* buffer, size_t count)
{
 return dosemu_write(handle,buffer,count);
}
