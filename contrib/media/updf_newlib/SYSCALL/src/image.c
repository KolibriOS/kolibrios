#include<menuet/os.h>

void __menuet__putimage(__u16 x1,__u16 y1,__u16 xsize,__u16 ysize,char * image)
{
 __u32 a,b;
 a=(xsize<<16)|ysize;
 b=(x1<<16)|y1;
 __asm__ __volatile__("int $0x40"::"a"(7),"b"(image),"c"(a),"d"(b));
}
