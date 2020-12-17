#include<menuet/os.h>

void __menuet__putpixel(__u32 x,__u32 y,__u32 color)
{
 __asm__ __volatile__("int $0x40"::"a"(1),"b"(x),"c"(y),"d"(color));
}
