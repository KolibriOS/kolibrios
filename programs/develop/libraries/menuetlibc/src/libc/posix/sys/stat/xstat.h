/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * Header for internal stat()/fstat() assist functions.
 *
 */

#ifndef __XSTAT_H
#define __XSTAT_H

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Some errno.h headers do not define EFAULT.  Here the value is
   taken from POSIX IEEE 1003.1.  */
#include <errno.h>
#ifndef EFAULT
#define EFAULT  14
#endif

/* Under MS-DOS, file access permissions are shared by all, except for
   Write permission. */
#define READ_ACCESS     (S_IRUSR | S_IRGRP | S_IROTH)
#define WRITE_ACCESS    S_IWUSR
#define EXEC_ACCESS     (S_IXUSR | S_IXGRP | S_IXOTH)

/* Macro to convert a segment and an offset to a "far offset" suitable
   for _farxxx() functions of DJGPP.  */
#ifndef MK_FOFF
#define MK_FOFF(s,o) ((int)((((unsigned long)(s)) << 4) + (unsigned short)(o)))
#endif

/* Ralph Brown's Interrupt List says this should be the length
   of the buffer for INT 21H AX=6000H.  */
#define MAX_TRUE_NAME   128

extern unsigned short   _osmajor, _osminor;
extern const    char  * _os_flavor;

/* Bits to flag f?stat() failed to use individual undocumented features. */
#define _STFAIL_SDA         1   /* Get SDA call failed */
#define _STFAIL_OSVER       2   /* Unsupported DOS version */
#define _STFAIL_BADSDA      4   /* Bad pointer to SDA */
#define _STFAIL_TRUENAME    8   /* _truename() failed */
#define _STFAIL_HASH     0x10   /* inode defaults to hashing */
#define _STFAIL_LABEL    0x20   /* Root dir, but no volume label */
#define _STFAIL_DCOUNT   0x40   /* dirent_count ridiculously large */
#define _STFAIL_WRITEBIT 0x80   /* fstat() failed to get write access bit */
#define _STFAIL_DEVNO   0x100   /* fstat() failed to get device number */
#define _STFAIL_BADSFT  0x200   /* SFT entry found, but can't be trusted */
#define _STFAIL_SFTIDX  0x400   /* bad SFT index in JFT */
#define _STFAIL_SFTNF   0x800   /* file entry not found in SFT array */

extern unsigned short   _djstat_fail_bits;

extern unsigned short   _djstat_flags;

extern time_t           _file_time_stamp(unsigned int);
extern ino_t            _invent_inode(const char *, unsigned, unsigned long);
extern unsigned short   _get_magic(const char *, int);
extern unsigned short   _get_dos_version(int);
extern char           * _truename(const char *, char *);
extern int              _is_remote_drive(int);
extern int              _is_executable(const char *, int, const char *);
extern short            _get_dev_info(int);
extern long             __filelength(int);
extern int              _is_remote_handle(int);
extern void             _djstat_describe_lossage(FILE *);
extern int              _getftime(int, unsigned int *);

#endif  /* __XSTAT_H */
