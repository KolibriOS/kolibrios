#include<menuet/os.h>

void __menuet__get_screen_max(__u16 * x,__u16 * y)
{
 __u32 v;
 __asm__ __volatile__("int $0x40":"=a"(v):"0"(14));
 if(x) *x=v>>16;
 if(y) *y=v & 0xFFFF;
}
