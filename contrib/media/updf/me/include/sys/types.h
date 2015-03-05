/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_sys_types_h_
#define __dj_include_sys_types_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/djtypes.h>
  
typedef int		dev_t;
typedef int		ino_t;
typedef int		mode_t;
typedef int		nlink_t;

__DJ_gid_t
#undef __DJ_gid_t
#define __DJ_gid_t
__DJ_off_t
#undef __DJ_off_t
#define __DJ_off_t
__DJ_pid_t
#undef __DJ_pid_t
#define __DJ_pid_t
__DJ_size_t
#undef __DJ_size_t
#define __DJ_size_t
__DJ_ssize_t
#undef __DJ_ssize_t
#define __DJ_ssize_t
__DJ_uid_t
#undef __DJ_uid_t
#define __DJ_uid_t

/* Allow including program to override.  */
#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif

typedef struct fd_set {
  unsigned char fd_bits [((FD_SETSIZE) + 7) / 8];
} fd_set;

#define FD_SET(n, p)    ((p)->fd_bits[(n) / 8] |= (1 << ((n) & 7)))
#define FD_CLR(n, p)	((p)->fd_bits[(n) / 8] &= ~(1 << ((n) & 7)))
#define FD_ISSET(n, p)	((p)->fd_bits[(n) / 8] & (1 << ((n) & 7)))
#define FD_ZERO(p)	memset ((void *)(p), 0, sizeof (*(p)))

__DJ_time_t
#undef __DJ_time_t
#define __DJ_time_t

#define __socklen_t_defined
typedef unsigned int socklen_t;
typedef unsigned short sa_family_t;

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_sys_types_h_ */
