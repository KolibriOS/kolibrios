#include<menuet/os.h>

void __menuet__bar(__u16 x1,__u16 y1,__u16 xsize,__u16 ysize,__u32 color)
{
 __u32 a,b;
 a=(x1<<16)|xsize;
 b=(y1<<16)|ysize;
 __asm__ __volatile__("int $0x40"::"a"(13),"b"(a),"c"(b),"d"(color));
}
