
#include <errno.h>
#include <sys/types.h>
#include <sys/ksys.h>
#include "glue.h"
#include "io.h"

int
_DEFUN (ftruncate, (fd, len),
    int fd _AND
    off_t len)
{
    __io_handle *ioh;
    if ((fd < 0) || (fd >=64))
    {
        errno = EBADF;
        return (-1);
    }
    ioh = &__io_tab[fd];
    if (_ksys_file_set_size(ioh->name, len))
        return (-1);

    return 0;
}
