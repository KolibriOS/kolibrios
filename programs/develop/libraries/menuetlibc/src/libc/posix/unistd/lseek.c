#include <unistd.h>
#include <errno.h>

#include <libc/dosio.h>

off_t lseek(int handle, off_t offset, int whence)
{
 return dosemu_lseek(handle,offset,whence);
}
