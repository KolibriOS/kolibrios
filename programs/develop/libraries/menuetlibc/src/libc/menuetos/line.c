#include<menuet/os.h>

void __menuet__line(__u16 x1,__u16 y1,__u16 x2,__u16 y2,__u32 color)
{
 __u32 b,c;
 b=(x1<<16)|x1;
 c=(y1<<16)|y2;
 __asm__ __volatile__("int $0x40"::"a"(38),"b"(b),"c"(c),"d"(color));
}
