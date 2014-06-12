#include<menuet/os.h>
#include<stdlib.h>
#include<stdarg.h>
#include<stdio.h>

static inline int vdprintf_help(char c)
{ 
 int d0;
 if(c=='\n')
 {
  c='\r';
  __asm__ __volatile__("int $0x40":"=&a"(d0):"0"(63),"b"(1),"c"(c));
  c='\n';
  __asm__ __volatile__("int $0x40":"=&a"(d0):"0"(63),"b"(1),"c"(c));
  return 0;
 }
 __asm__ __volatile__("int $0x40":"=&a"(d0):"0"(63),"b"(1),"c"(c));
 return 0 ;
}

static char log_buf[1024];

static char xputs(char * s)
{
 for(;*s;s++) vdprintf_help(*s);
}

int __libclog_vprintf(const char *fmt, va_list args)
{
 int ret_val;
 ret_val = vsprintf(log_buf,fmt,args);
 xputs(log_buf);
 __menuet__delay100(1);
 return ret_val;
}

int __libclog_printf(const char * fmt,...)
{
 int v;
 va_list ap;
 va_start(ap,fmt);
 v=__libclog_vprintf(fmt,ap);
 __menuet__delay100(1);
 return v;
}
