#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <io.h>

#include <libc/dosio.h>

int open(const char* filename, int oflag, ...)
{
/* int fd, dmode, bintext;
 bintext = oflag & (O_TEXT | O_BINARY);
 if (!bintext) bintext = _fmode & (O_TEXT | O_BINARY);
 if (!bintext) bintext = O_BINARY;
 oflag &= ~(O_TEXT | O_BINARY);
 dmode = (*((&oflag)+1) & S_IWUSR) ? 1 : 0;
 fd = _open(filename, oflag & 0xff);
 if (fd == -1 && oflag & O_CREAT)
 fd = _creat(filename, dmode);
 if (fd == -1)  return fd;
 if (oflag & O_TRUNC)
  if (_write(fd, 0, 0) < 0)
   return -1;
 if(oflag & O_APPEND)
  lseek(fd, 0, SEEK_END);
  return fd;*/
	return dosemu_open(filename, oflag);
}
