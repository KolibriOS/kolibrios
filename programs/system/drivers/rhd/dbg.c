
#include "common.h"

#pragma pack(push, 1)
typedef struct
{
  char sec;
  char min;
  char hour;
  char rsv;
}detime_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
  char  day;
  char  month;
  short year;
}dedate_t;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct
{   unsigned    attr;
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

static void _SysMsgBoardStr(char *text)
{
  asm __volatile__
  (
    "call [DWORD PTR __imp__SysMsgBoardStr]"
    :
    :"S" (text)
  );
};

int get_fileinfo(const char *path,FILEINFO *info)
{
   int retval;

   asm __volatile__
      (
       "push 0 \n\t"
       "push 0 \n\t"
       "mov [esp+1], eax \n\t"
       "push ebx \n\t"
       "push 0 \n\t"
       "push 0 \n\t"
       "push 0 \n\t"
       "push 5 \n\t"
       "mov ebx, esp \n\t"
       "mov eax, 70 \n\t"
       "int 0x40 \n\t"
       "add esp, 28 \n\t"
       :"=eax" (retval)
       :"a" (path), "b" (info)
       );
   return retval;
};

int create_file(const char *path)
{
  int retval;
  asm __volatile__(
      "push 0 \n\t"
      "push 0 \n\t"
      "mov [esp+1], eax \n\t"
      "push 0 \n\t"
      "push 0 \n\t"
      "push 0 \n\t"
      "push 0 \n\t"
      "push 2 \n\t"
      "mov ebx, esp \n\t"
      "mov eax, 70 \n\t"
      "int 0x40 \n\t"
      "add esp, 28"
      :"=eax" (retval)
      :"a" (path)
      );
  return retval;
};

int set_file_size(const char *path, unsigned size)
{
  int retval;
  asm __volatile__(
      "push 0 \n\t"
      "push 0 \n\t"
      "mov [esp+1], eax \n\t"
      "push 0 \n\t"
      "push 0 \n\t"
      "push 0 \n\t"
      "push ebx \n\t"
      "push 4 \n\t"
      "mov ebx, esp \n\t"
      "mov eax, 70 \n\t"
      "int 0x40 \n\t"
      "add esp, 28"
      :"=eax" (retval)
      :"a" (path), "b" (size)
      );
  return retval;
};

int write_file(const char *path,const void *buff,
               unsigned offset,unsigned count,unsigned *writes)
{
  int retval;
  asm __volatile__
     ("push ebx \n\t"
      "push 0 \n\t"
      "push 0 \n\t"
      "mov [esp+1], eax \n\t"
      "push ebx \n\t"
      "push edx \n\t"
      "push 0 \n\t"
      "push ecx \n\t"
      "push 3 \n\t"
      "mov ebx, esp \n\t"
      "mov eax, 70 \n\t"
      "int 0x40 \n\t"
      "test esi, esi \n\t"
      "jz 1f \n\t"
      "mov [esi], ebx \n\t"
"1:"
      "add esp, 28 \n\t"
      "pop ebx"
      :"=eax" (retval)
      :"a"(path),"b"(buff),"c"(offset),"d"(count),"S"(writes)
     );
  return retval;
};

char * _putc(char *s, int c)
{
  int i=0;

  switch(c)
  {
      case '\n':
        *s++ = '\r';
        *s++ = '\n';

      case '\r':
        break;

      case '\t':
        do
        {
          *s++ = ' ';
        }
        while (i % 8 != 0);
        break;
      default:
        *s++ = c;
  }
  return s;
}

char *print_string(char *buff, char* s)
{
  int i=0;
  char c;

  while (c=*s++)
  {
    switch(c)
    {
      case '\r':
        break;

      case '\n':
        *buff++ = '\r';
        *buff++ = '\n';
        i=0;

      case '\t':
        do
        {
          *buff++ = ' ';
          i++;
        }
        while (i % 8 != 0);
        break;

      default:
        *buff++ = c;
        i++;
    };
  }
  return buff;
}

char *print_dec(char *buff,int val)
{
    char dbuff[16];
    int i = 14;

    dbuff[15] = '\0';
    do
    {
      dbuff[i] = (val % 10) + '0';
      val = val / 10;
      i--;
    } while(val);

    return print_string(buff, &dbuff[i+1]);
}

const char  hexchars[] = "0123456789ABCDEF";

char *print_hex(char *buff, u32_t val)
{
  int i;
  for (i=sizeof(u32_t)*8-4; i >= 0; i -= 4)
    buff = _putc(buff,hexchars[((u32_t)val >> i) & 0xF]);
  return buff;
}

#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L
#define va_copy(d,s)	__builtin_va_copy(d,s)
#endif
#define __va_copy(d,s)	__builtin_va_copy(d,s)

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

#define arg(x) va_arg (ap, u32_t)

char txtbuf[128];

int printf(const char* format, ...)
{
    u32_t ret = 1;
    u32_t i = 0;
    char *sbuf = txtbuf;

    va_list ap;

    va_start (ap, format);

    if (format == 0)
      return 0;

    while (*format)
    {
      switch (*(format))
      {
        case '%':
next_fmt:
          switch (*(++format))
          {
            case 'l': case '-':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            goto next_fmt;

            case 'c':
              sbuf = _putc (sbuf,arg (i));
              break;
            case 'd':
              sbuf = print_dec (sbuf,arg (i));
              break;
            case 'p':
            case 'x':
              sbuf = print_hex (sbuf,(u32_t) arg (i));
              break;
            case 's':
              sbuf = print_string (sbuf,(char*) arg (i));
              break;
            default:
              sbuf = print_string (sbuf,"?");
              break;
          }
          i++;
          break;

        default:
          sbuf = _putc (sbuf,*format);
          break;
      }
      format++;
    }

    va_end (ap);
    *sbuf=0;
    _SysMsgBoardStr(txtbuf);
    return ret;
}

int dbg_open(char *path)
{
  FILEINFO info;

  dbgfile.offset = 0;

  if(get_fileinfo(path,&info))
  {
    if(!create_file(path))
    {
      dbgfile.path = path;
      return TRUE;
    }
    else
      return FALSE;
  };
  set_file_size(path, 0);
  dbgfile.path   = path;
  dbgfile.offset = 0;
  return TRUE;
};

int vsnprintf(char *s, size_t n, const char *format, va_list arg);

int dbgprintf(const char* format, ...)
{
    unsigned writes;

    int len=0;
//    char *sbuf = txtbuf;

    va_list ap;

    va_start(ap, format);
    if (format)
      len = vsnprintf(txtbuf, 128, format, ap);
    va_end(ap);

    _SysMsgBoardStr(txtbuf);

    if(dbgfile.path)
    {
      write_file(dbgfile.path,txtbuf,dbgfile.offset,len,&writes);
      dbgfile.offset+=writes;
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
/*
int snprintf(char *buf,int count, const char* format, ...)
{
    int len;

  //  u32 ret = 1;
    u32 i = 0;
    char *sbuf = buf;

    va_list ap;

    va_start (ap, format);

    if (format == 0)
      return 0;

    while (*format)
    {
      switch (*(format))
      {
        case '%':
next_fmt:
          switch (*(++format))
          {
            case 'l': case '-':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            goto next_fmt;

            case 'c':
              sbuf = _putc (sbuf,arg (i));
              break;
            case 'd':
              sbuf = print_dec (sbuf,arg (i));
              break;
            case 'p':
            case 'x':
              sbuf = print_hex (sbuf,(u32) arg (i));
              break;
            case 's':
              sbuf = print_string (sbuf,(char*) arg (i));
              break;
            default:
              sbuf = print_string (sbuf,"?");
              break;
          }
          i++;
          break;

        default:
          sbuf = _putc (sbuf,*format);
          break;
      }
      format++;
    }

    va_end (ap);
    *sbuf=0;
    len = sbuf-txtbuf;

    return len;
}
*/

char *
RhdAppendString(char *s1, const char *s2)
{

  if (!s2)
    return s1;
  else
    if (!s1)
      return strdup(s2);
    else
    {
      int len = strlen(s1) + strlen(s2) + 1;
      char *result  = (char *)malloc(len);

      if (!result) return s1;

      strcpy(result,s1);
      strcat(result,s2);
      free(s1);
      return result;
    }

  return 0;
}


