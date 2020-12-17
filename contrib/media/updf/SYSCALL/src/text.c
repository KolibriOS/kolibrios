#include<menuet/os.h>

void __menuet__write_text(__u16 x,__u16 y,__u32 color,char * text,int len)
{
 __asm__ __volatile__("int $0x40"::"a"(4),"b"((x<<16)|y),"c"(color),"d"((__u32)text),"S"(len));
}
