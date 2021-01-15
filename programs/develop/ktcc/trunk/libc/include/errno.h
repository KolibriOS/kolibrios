#ifndef _ERRNO_H
#define _ERRNO_H

extern int errno;
/* errors codes from KOS, but minus */
# define E_SUCCESS (0)
# define E_UNSUPPORTED (-2)
# define E_UNKNOWNFS  (-3)
# define E_NOTFOUND (-5)
# define E_EOF  (-6)
# define E_INVALIDPTR (-7)
# define E_DISKFULL  (-8)
# define E_FSYSERROR  (-9)
# define E_ACCESS  (-10)
# define E_HARDWARE  (-11)
# define E_NOMEM  (-12)
/* conversion errors */
# define ERANGE (-20)
# define EINVAL (-21)
/* program run and pipe errors */
# define E_NOMEM2 (-30)
# define E_FILEFMT (-31)
# define E_TOOMANY (-32)
# define E_PARAM (-33)
/* socket error codes*/
#define ENOBUFS      1
#define EINPROGRESS  2
#define EOPNOTSUPP   4
#define EWOULDBLOCK  6
#define ENOTCONN     9
#define EALREADY     10
#define EINVALUE     11
#define EMSGSIZE     12
#define ENOMEM       18
#define EADDRINUSE   20
#define ECONNREFUSED 61
#define ECONNRESET   52
#define EISCONN      56
#define ETIMEDOUT    60
#define ECONNABORTED 53

#endif
