#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<menuet/os.h>
#include<menuet/textcon.h>
#include<ctype.h>
#include<stdarg.h>
#include<errno.h>

static int did_con_init=0;

static char * WindowTitle="xtest";

static char con_buffer[0x100000];
static char * conp=con_buffer+0;
static unsigned long sz=0;

void check_board(void)
{
 int d0,d1;
 do {
  __asm__ __volatile__("int $0x40":"=a"(d0),"=b"(d1):"0"(63),"1"(2));
  if(d1!=1) break;
  _lcon_putch(d0&0xff);
  *conp++=(d0&0xff);
  sz++;
 } while(1);
}

void paint_wnd(void)
{
 __menuet__window_redraw(1);
 __menuet__define_window(100,100,20+(NR_CHARS_X*CHAR_SIZE_X),
    30+(NR_CHARS_Y*CHAR_SIZE_Y),0x03000080,0x800000FF,0x000080);
 __menuet__write_text(5,5,0xFFFFFF,WindowTitle,strlen(WindowTitle));
 if(did_con_init) _lcon_flush_console();
 __menuet__window_redraw(2);
}

static char kpf_buf[1024];

void _kph_pf(char * p)
{
 for(;p && *p;p++) _lcon_putch(*p);
}

void kprintf(const char * fmt,...)
{
 va_list ap;
 va_start(ap,fmt);
 vsprintf(kpf_buf,fmt,ap);
 va_end(ap);
 _kph_pf(kpf_buf);  
}

int event_loop(void)
{
 int i;
 i=__menuet__check_for_event();
 switch(i)
 {
  case 1:
   paint_wnd(); return 0;
  case 2:
   return __menuet__getkey();
  case 3:
   if(__menuet__get_button_id()==1)
   {
    exit(0);
   }
   return 0;
 }
 return 1;
}

void main(void)
{
 unsigned long xtmp;
 int a,b,c;
 FILE * f=fopen("example","rb");
 did_con_init=0;
 paint_wnd();
 init_consoles();
 did_con_init=1;
 xtmp=__menuet__getsystemclock();
 a=(xtmp>>16)&0xff;
 b=(xtmp>>8)&0xff;
 c=xtmp&0xff;
 kprintf("h/m/s=%x:%x:%x %u:%u:%u\n",a,b,c,a,b,c);
#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)
 BCD_TO_BIN(a);
 BCD_TO_BIN(b);
 BCD_TO_BIN(c);
 kprintf("h/m/s=%x:%x:%x %u:%u:%u\n",a,b,c,a,b,c);
 __asm__ __volatile__("int $0x40":"=a"(xtmp):"0"(29)); 
 a=(xtmp>>16)&0xff;
 b=(xtmp>>8)&0xff;
 c=xtmp&0xff;
 kprintf("y/d/m=%x:%x:%x %u:%u:%u\n",a,b,c,a,b,c);
#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)
 BCD_TO_BIN(a);
 BCD_TO_BIN(b);
 BCD_TO_BIN(c);
 kprintf("y/d/m=%x:%x:%x %u:%u:%u\n",a,b,c,a,b,c);
 for(;;) 
 {
  check_board();
  event_loop();
 }
}
