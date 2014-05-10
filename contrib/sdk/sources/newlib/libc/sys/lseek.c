
#include <errno.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <sys/kos_io.h>
#include "glue.h"
#include "io.h"


_off_t
_DEFUN (lseek, (fd, pos, whence),
     int fd _AND
     _off_t pos _AND
     int whence)

{
    fileinfo_t  info;
    __io_handle *ioh;
    _off_t ret;

    if( (fd < 0) || (fd >=64) )
    {
        errno = EBADF;
        return (-1);
    };

    ioh = &__io_tab[fd];

    switch(whence)
    {
        case SEEK_SET:
            ret = pos;
            break;
        case SEEK_CUR:
            ret = ioh->offset + pos;
            break;
        case SEEK_END:
        {
            get_fileinfo(ioh->name, &info);
            ret = pos + info.size;
            break;
        }
        default:
            errno = EINVAL;
            return (-1);
    };

    ioh->offset = ret;

    return( ret );
};
