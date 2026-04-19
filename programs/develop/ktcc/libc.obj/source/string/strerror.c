/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <string.h>
#include <errno.h>

int __errno;

char* strerror(int err)
{
    char *msg;
    switch(err){
        case 0:            msg = "No errors"; break;

        case ENOTSUP:      msg = "Function is not supported"; break;
        case EUNKNFS:      msg = "Unknown file system"; break;
        case ENOTFOUND:    msg = "File not found"; break;
        case EEOF:         msg = "End of file"; break;
        case EFAULT:       msg = "Bad address"; break;
        case EDQUOT:       msg = "Disc quota exceeded"; break;
        case EFS:          msg = "File system error"; break;
        case EACCES:       msg = "Permission denied"; break;
        case EDEV:         msg = "Device error"; break;
        case ENOMEMFS:     msg = "File system requires more memory"; break;

        case ENOMEM:       msg = "Not enough memory"; break;
        case ENOEXEC:      msg = "Exec format error"; break;
        case EPROCLIM:     msg = "Too many processes"; break;
        case EINVAL:       msg = "Invalid argument"; break;

        case EDOM:         msg = "Numerical argument out of domain"; break;
        case ERANGE:       msg = "Result too large"; break;
        case EILSEQ:       msg = "Illegal byte sequence"; break;
        
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
