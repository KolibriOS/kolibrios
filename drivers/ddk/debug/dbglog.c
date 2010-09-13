
#include <ddk.h>
#include <mutex.h>
#include <syscall.h>

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
} FILEINFO;

#pragma pack(pop)

typedef struct
{
  char *path;
  int  offset;
} dbgfile_t;

static dbgfile_t dbgfile;

#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)
#define __va_copy(d,s)	__builtin_va_copy(d,s)

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list    va_list;

#define arg(x) va_arg (ap, u32_t)

int dbg_open(char *path)
{
    FILEINFO info;

    dbgfile.offset = 0;

    if(get_fileinfo(path,&info))
    {
        if(!create_file(path))
        {
            dbgfile.path = path;
            return true;
        }
        else return false;
    };
    set_file_size(path, 0);
    dbgfile.path   = path;
    dbgfile.offset = 0;
    return true;
};

int vsnprintf(char *s, size_t n, const char *format, va_list arg);


int printf(const char* format, ...)
{
    char  txtbuf[256];
    int   len = 0;

    va_list ap;

    va_start(ap, format);
    if (format)
        len = vsnprintf(txtbuf, 256, format, ap);
    va_end(ap);

    if( len )
        SysMsgBoardStr(txtbuf);

    return len;
}


int dbgprintf(const char* format, ...)
{
    char      txtbuf[256];
    unsigned  writes;
    int       len = 0;

    va_list   ap;

    va_start(ap, format);
    if (format)
      len = vsnprintf(txtbuf, 256, format, ap);
    va_end(ap);

    if( len )
    {
        SysMsgBoardStr(txtbuf);

/*  do not write into log file if interrupts disabled */

        if ( (get_eflags() & (1 << 9)) && dbgfile.path)
        {
            write_file(dbgfile.path,txtbuf,dbgfile.offset,len,&writes);
            dbgfile.offset+=writes;
        };
    };
    return len;
}

int xf86DrvMsg(int skip, int code, const char* format, ...)
{
    char      txtbuf[256];
    unsigned  writes;
    va_list   ap;

    int       len = 0;

    va_start(ap, format);
    if (format)
        len = vsnprintf(txtbuf, 256, format, ap);
    va_end(ap);

    if( len )
    {
        SysMsgBoardStr(txtbuf);

        if(dbgfile.path)
        {
            write_file(dbgfile.path,txtbuf,dbgfile.offset,len,&writes);
            dbgfile.offset+=writes;
        };
    };
    return len;
}

int snprintf(char *s, size_t n, const char *format, ...)
{
	va_list ap;
	int retval;

	va_start(ap, format);
	retval = vsnprintf(s, n, format, ap);
	va_end(ap);

	return retval;
}



