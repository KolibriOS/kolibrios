#include<stdio.h>
#include<stdarg.h>

#if (MCOFF_MENUETOS==1)
static int vdprintf_help(unsigned c)
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

static void xputs(char * p)
{
 for(;*p;p++) vdprintf_help((*p)&0xff);
}

static char dbg_buf[1024];

void dprintf(const char * fmt,...)
{
 va_list ap;
 va_start(ap,fmt);
 vsprintf(dbg_buf,fmt,ap);
 va_end(ap);
 xputs(dbg_buf);
}

#else
void dprintf(const char * fmt,...)
{
 va_list ap;
 va_start(ap,fmt);
 vfprintf(stderr,fmt,ap);
 va_end(ap);
}
#endif
