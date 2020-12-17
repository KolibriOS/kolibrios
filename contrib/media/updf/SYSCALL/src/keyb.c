#include<menuet/os.h>

int __menuet__getkey(void)
{
 __u16 __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(2));
 if(!(__ret & 0xFF)) return (__ret>>8)&0xFF; else return 0;
}
