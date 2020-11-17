#include<menuet/os.h>

void __menuet__dga_get_caps(int * xres,int * yres,int * bpp,int * bpscan)
{
 int p;
 __asm__ __volatile__("int $0x40":"=a"(p):"0"(61),"b"(1));
 if(xres) *xres=(p>>16)&0xFFFF;
 if(yres) *yres=p & 0xFFFF;
 if(bpp)
 {
  __asm__ __volatile__("int $0x40":"=a"(p):"0"(61),"b"(2));
  *bpp=p;
 }
 if(bpscan)
 {
  __asm__ __volatile__("int $0x40":"=a"(p):"0"(61),"b"(3));
  *bpscan=p;
 }
}
