#include <unistd.h>
#include <fcntl.h>

int
fcntl(int fd, int cmd, ...)
{
  switch (cmd)
  {
  case F_DUPFD:
    return dup(fd);
  case F_GETFD:
  case F_SETFD:
  case F_GETFL:
  case F_SETFL:
    return 0;
  case F_GETLK:
  case F_SETLK:
  case F_SETLKW:
    return -1;
  }
  return -1;
}
