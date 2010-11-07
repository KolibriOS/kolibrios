/* Reentrant versions of open system call. */

#include <reent.h>
#include <unistd.h>
#include <fcntl.h>
#include <_syslist.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Some targets provides their own versions of this functions.  Those
   targets should define REENTRANT_SYSCALLS_PROVIDED in TARGET_CFLAGS.  */

#ifdef _REENT_ONLY
#ifndef REENTRANT_SYSCALLS_PROVIDED
#define REENTRANT_SYSCALLS_PROVIDED
#endif
#endif

#ifndef REENTRANT_SYSCALLS_PROVIDED

/* We use the errno variable used by the system dependent layer.  */

/*
FUNCTION
	<<_open_r>>---Reentrant version of open

INDEX
	_open_r

ANSI_SYNOPSIS
	#include <reent.h>
	int _open_r(struct _reent *<[ptr]>,
		    const char *<[file]>, int <[flags]>, int <[mode]>);

TRAD_SYNOPSIS
	#include <reent.h>
	int _open_r(<[ptr]>, <[file]>, <[flags]>, <[mode]>)
	struct _reent *<[ptr]>;
	char *<[file]>;
	int <[flags]>;
	int <[mode]>;

DESCRIPTION
	This is a reentrant version of <<open>>.  It
	takes a pointer to the global data block, which holds
	<<errno>>.
*/


#pragma pack(push, 1)
typedef struct
{
    char sec;
    char min;
    char hour;
    char rsv;
}detime_t;

typedef struct
{
    char  day;
    char  month;
    short year;
}dedate_t;

typedef struct
{
    unsigned    attr;
    unsigned    flags;
    union
    {
        detime_t  ctime;
        unsigned  cr_time;
    };
    union
    {
        dedate_t  cdate;
        unsigned  cr_date;
    };
    union
    {
        detime_t  atime;
        unsigned  acc_time;
    };
    union
    {
        dedate_t  adate;
        unsigned  acc_date;
    };
    union
    {
        detime_t  mtime;
        unsigned  mod_time;
    };
    union
    {
        dedate_t  mdate;
        unsigned  mod_date;
    };
    unsigned    size;
    unsigned    size_high;
} fileinfo_t;

#pragma pack(pop)


#define NULL_HANDLE  (int)-1
#define DUMMY_HANDLE (int)-2

#define _READ   0x0001  /* file opened for reading */
#define _WRITE  0x0002  /* file opened for writing */
#define _UNGET  0x0004  /* ungetc has been done */
#define _BIGBUF 0x0008  /* big buffer allocated */
#define _EOF    0x0010  /* EOF has occurred */
#define _SFERR  0x0020  /* error has occurred on this file */
#define _APPEND 0x0080  /* file opened for append */
#define _BINARY 0x0040  /* file is binary, skip CRLF processing */
#define _TMPFIL 0x0800  /* this is a temporary file */
#define _DIRTY  0x1000  /* buffer has been modified */
#define _ISTTY  0x2000  /* is console device */
#define _DYNAMIC 0x4000 /* FILE is dynamically allocated   */
#define _FILEEXT 0x8000 /* lseek with positive offset has been done */
#define _COMMIT 0x0001  /* extended flag: commit OS buffers on flush */

extern int       _fmode;

int create_file(const char *path)
{
     int retval;
     __asm__ __volatile__ (
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "movl %0, 1(%%esp) \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl $2 \n\t"
     "movl %%esp, %%ebx \n\t"
     "movl $70, %%eax \n\t"
     "int $0x40 \n\t"
     "addl $28, %%esp \n\t"
     :"=a" (retval)
     :"r" (path)
     :"ebx");
  return retval;
};

int set_file_size(const char *path, unsigned size)
{
     int retval;
     __asm__ __volatile__(
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "movl %%eax, 1(%%esp) \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "pushl %%ebx \n\t"
     "push $4 \n\t"
     "movl %%esp, %%ebx \n\t"
     "movl $70, %%eax \n\t"
     "int $0x40 \n\t"
     "addl $28, %%esp \n\t"
     :"=a" (retval)
     :"a" (path), "b" (size));
     return retval;
};

int get_fileinfo(const char *path, fileinfo_t *info)
{
    int retval;

    __asm__ __volatile__ (
    "pushl $0 \n\t"
    "pushl $0 \n\t"
    "movl %1, 1(%%esp) \n\t"
    "pushl %%ebx \n\t"
    "pushl $0 \n\t"
    "pushl $0 \n\t"
    "pushl $0 \n\t"
    "pushl $5 \n\t"
    "movl %%esp, %%ebx \n\t"
    "movl $70, %%eax \n\t"
    "int $0x40 \n\t"
    "addl $28, %%esp \n\t"
    :"=a" (retval)
    :"r" (path), "b" (info));
   return retval;
};


int read_file(const char *path, void *buff,
               size_t offset, size_t count, size_t *reads)
{
    int  retval;
    int  d0;
    __asm__ __volatile__(
    "pushl $0 \n\t"
    "pushl $0 \n\t"
    "movl %%eax, 1(%%esp) \n\t"
    "pushl %%ebx \n\t"
    "pushl %%edx \n\t"
    "pushl $0 \n\t"
    "pushl %%ecx \n\t"
    "pushl $0 \n\t"
    "movl %%esp, %%ebx \n\t"
    "mov $70, %%eax \n\t"
    "int $0x40 \n\t"
    "testl %%esi, %%esi \n\t"
    "jz 1f \n\t"
    "movl %%ebx, (%%esi) \n\t"
"1:"
    "addl $28, %%esp \n\t"
    :"=a" (retval)
    :"a"(path),"b"(buff),"c"(offset),"d"(count),"S"(reads));
    return retval;
};


static inline void debug_out(const char val)
{
    __asm__ __volatile__(
    "int $0x40 \n\t"
    ::"a"(63), "b"(1),"c"(val));
}

int   debugwrite(const char *path, const void *buff,
                 size_t offset, size_t count, size_t *writes)
{
    int ret = count;
    const char *p = buff;

    while (count--)
    {
        debug_out(*p++);
    };
    *writes = ret;
    return ret;
};


int write_file(const char *path,const void *buff,
               size_t offset, size_t count, size_t *writes)
{
     int retval;
     __asm__ __volatile__(
     "pushl $0 \n\t"
     "pushl $0 \n\t"
     "movl %%eax, 1(%%esp) \n\t"
     "pushl %%ebx \n\t"
     "pushl %%edx \n\t"
     "pushl $0 \n\t"
     "pushl %%ecx \n\t"
     "pushl $3 \n\t"
     "movl %%esp, %%ebx \n\t"
     "mov $70, %%eax \n\t"
     "int $0x40 \n\t"
     "testl %%esi, %%esi \n\t"
     "jz 1f \n\t"
     "movl %%ebx, (%%esi) \n\t"
"1:"
     "addl $28, %%esp \n\t"
     :"=a" (retval)
     :"a"(path),"b"(buff),"c"(offset),"d"(count),"S"(writes));
    return retval;
};

static int __openFileHandle(const char *path, int mode, int *err)
{
    fileinfo_t     info;
    __file_handle *handle;

//    path = getfullpath(name);

    *err = get_fileinfo(path, &info);

    if( mode & O_EXCL && mode & O_CREAT )
    {
        if( ! *err)
        {
            *err = EEXIST;
            return -1;
        };
    }

    if( *err)
    {
        if(mode & O_CREAT)
            *err=create_file(path);

        if( *err)
        {
            return -1;
        };
    };
    if( mode & O_TRUNC )
        set_file_size(path, 0);

    if ( !(handle=(__file_handle*)malloc(sizeof( __file_handle) )))
    {
        *err = ENOMEM;
        return -1;
    };

    handle->name   = strdup(path);
    handle->offset = 0;
    handle->write  = write_file;

    *err = 0;

    return (int)handle;
};



int
_DEFUN (_open_r, (ptr, file, flags, dmode),
     struct _reent *ptr _AND
     _CONST char *file _AND
     int flags _AND
     int dmode)
{
    int         hid;
    int         handle;
    int         err = 0;
    unsigned    iomode_flags;
    int         rwmode;

/*
    if (flags & ~(O_RDONLY | O_WRONLY | O_RDWR | O_CREAT | O_APPEND | O_TRUNC))
    {
        ptr->_errno = ENOSYS;
        return -1;
    }
*/

    // First try to get the required slot.
    // No point in creating a file only to not use it.  JBS 99/10/26
    hid = __allocPOSIXHandle( DUMMY_HANDLE );
    if( hid == -1 )
    {
        ptr->_errno = EMFILE;
        return( -1 );
    }

    handle = __openFileHandle( file, flags, &err);

    if( handle == -1 )
    {
        __freePOSIXHandle( hid );
        ptr->_errno = err;
        return( -1 );
    }

    __setOSHandle( hid, handle );   // JBS 99/11/01

    rwmode = flags & ( O_RDONLY | O_WRONLY | O_RDWR | O_NOINHERIT );

    iomode_flags = 0;

    if( rwmode == O_RDWR )       iomode_flags |= _READ | _WRITE;
    else if( rwmode == O_RDONLY) iomode_flags |= _READ;
    else if( rwmode == O_WRONLY) iomode_flags |= _WRITE;
    if( flags & O_APPEND )        iomode_flags |= _APPEND;
    if( flags & (O_BINARY|O_TEXT) ) {
        if( flags & O_BINARY )    iomode_flags |= _BINARY;
    } else {
        if( _fmode == O_BINARY ) iomode_flags |= _BINARY;
    }
    __SetIOMode( hid, iomode_flags );

    ptr->_errno = 0;

    return (hid);
}

int
_DEFUN (open, (file, flags, ...),
        const char *file _AND
        int flags _DOTS)
{
  va_list ap;
  int ret;

  va_start (ap, flags);
  ret = _open_r (_REENT, file, flags, va_arg (ap, int));
  va_end (ap);
  return ret;
}



#endif /* ! defined (REENTRANT_SYSCALLS_PROVIDED) */
