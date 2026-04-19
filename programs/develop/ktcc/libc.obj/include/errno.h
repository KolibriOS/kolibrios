#ifndef _ERRNO_H_
#define _ERRNO_H_

#include <stddef.h>

#ifdef _BUILD_LIBC
#define errno __errno
extern int __errno;
#else
extern int* __errno;
#define errno *__errno
#endif

#define ENOTSUP   2  // Function is not supported
#define EUNKNFS   3  // Unknown file system
#define ENOTFOUND 5  // File not found
#define EEOF      6  // End of file
#define EFAULT    7  // Pointer lies outside of application memory
#define EDQUOT    8  // Disk is full
#define EFS       9  // File system error
#define EACCES    10 // Access denied
#define EDEV      11 // Device error
#define ENOMEMFS  12 // File system requires more memory

#define ENOMEM   30 // Not enough memory
#define ENOEXEC  31 // Is not executable
#define EPROCLIM 32 // Too many processes
#define EINVAL   33 // Invalid argument

#define EDOM   50 // Numerical argument out of domain
#define ERANGE 51 // Result too large
#define EILSEQ 52 // Illegal byte sequence

#define ENOBUFS      60 // Broken buffer
#define EINPROGRESS  61 // Operation now in progress
#define EOPNOTSUPP   62 // Operation not supported on transport endpoint
#define EWOULDBLOCK  63 // Operation would block
#define ENOTCONN     64 // Transport endpoint is not connected
#define EALREADY     65 // Operation already in progress
#define EMSGSIZE     66 // Message too long
#define EADDRINUSE   67 // Address already in use
#define ECONNREFUSED 68 // Connection refused
#define ECONNRESET   69 // Connection reset by peer
#define EISCONN      70 // Transport endpoint is already connected
#define ETIMEDOUT    71 // Connection timed out
#define ECONNABORTED 72 // Software caused connection abort

/*
* UNIX-like errno
* Will be removed after changing STDIO.
*/

#define ENOTDIR   80
#define EBADF     81
#define EIO       82
#define EISDIR    83
#define ENOENT    84
#define EOVERFLOW 85

#endif // _ERRNO_H_
