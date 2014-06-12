#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <io.h>

#include <libc/dosio.h>

ssize_t read(int handle, void* buffer, size_t count)
{
 return dosemu_read(handle,buffer,count);
}
