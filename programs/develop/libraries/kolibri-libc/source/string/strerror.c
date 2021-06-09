/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <string.h>
#include <errno.h>

int _errno;

char* strerror(int err)
{
    char *msg;
    switch(err){
        case 0:            msg = "No errors"; break;
        case EPERM:        msg = "Operation not permitted"; break;
        case ENOENT:       msg = "No such file or directory"; break;
        case ESRCH:        msg = "No such process"; break;
        case EINTR:        msg = "Interrupted system call"; break;
        case EIO:          msg = "Input/output error"; break;
        case ENXIO:        msg = "Device not configured"; break;
        case E2BIG:        msg = "Argument list too long"; break;
        case ENOEXEC:      msg = "Exec format error"; break;
        case EBADF:        msg = "Bad file descriptor"; break;
        case ECHILD:       msg = "No child processes"; break;
        case EDEADLK:      msg = "Resource deadlock avoided"; break;
        case ENOMEM:       msg = "Cannot allocate memory"; break;
        case EACCES:       msg = "Permission denied"; break;
        case EFAULT:       msg = "Bad address"; break;
        case ENOTBLK:      msg = "Block device required"; break;
        case EBUSY:        msg = "Device / Resource busy"; break;
        case EEXIST:       msg = "File exists"; break;
        case EXDEV:        msg =  "Cross-device link"; break;
        case ENODEV:       msg = "Operation not supported by device"; break;
        case ENOTDIR:      msg = "Not a directory"; break;
        case EISDIR:       msg = "Is a directory"; break;
        case EINVAL:       msg = "Invalid argument"; break;
        case ENFILE:       msg = "Too many open files in system"; break;
        case EMFILE:       msg = "Too many open files"; break;
        case ENOTTY:       msg = "Inappropriate ioctl for device"; break;
        case ETXTBSY:      msg = "Text file busy"; break;
        case EFBIG:        msg = "File too large"; break;
        case ENOSPC:       msg = "No space left on device"; break;
        case ESPIPE:       msg = "Illegal seek"; break;
        case EROFS:        msg = "Read-only file system"; break;
        case EMLINK:       msg = "Too many links"; break;
        case EPIPE:        msg = "Broken pipe"; break;

        // math software
        case EDOM:         msg = "Numerical argument out of domain"; break;
        case ERANGE:       msg = "Result too large"; break;

        // should be rearranged
        case EHOSTDOWN:    msg = "Host is down"; break;
        case EHOSTUNREACH: msg = "No route to host"; break;
        case ENOTEMPTY:    msg = "Directory not empty"; break;

        // quotas & mush
        case EPROCLIM:     msg = "Too many processes"; break;
        case EUSERS:       msg = "Too many users"; break;
        case EDQUOT:       msg = "Disc quota exceeded"; break;

        // Intelligent device errors
        case EPWROFF:      msg = "Device power is off"; break;
        case EDEVERR:      msg = "Device error, e.g. paper out"; break;
        case EOVERFLOW:    msg = "Value too large to be stored in data type"; break;

        // Socket errors
        case ENOBUFS:      msg = "Broken buffer"; break;
        case EINPROGRESS:  msg = "Operation now in progress"; break;
        case EOPNOTSUPP:   msg = "Operation not supported on transport endpoint"; break;
        case EWOULDBLOCK:  msg = "Operation would block"; break;
        case ENOTCONN:     msg = "Transport endpoint is not connected"; break;
        case EALREADY:     msg = "Operation already in progress"; break;
        case EMSGSIZE:     msg = "Message too long"; break;
        case EADDRINUSE:   msg = "Address already in use"; break;
        case ECONNREFUSED: msg = "Connection refused"; break;
        case ECONNRESET:   msg = "Connection reset by peer"; break;
        case EISCONN:      msg = "Transport endpoint is already connected"; break;
        case ETIMEDOUT:    msg = "Connection timed out"; break;
        case ECONNABORTED: msg = "Software caused connection abort"; break;
        default:           msg = "Unknown error"; break;
    }
    return msg;
}
