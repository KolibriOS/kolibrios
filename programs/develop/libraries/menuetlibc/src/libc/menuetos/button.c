#include<menuet/os.h>

void __menuet__make_button(__u16 x1,__u16 y1,__u16 xsize,__u16 ysize,
                           int id,__u32 color)
{
 __u32 a,b;
 a=(x1<<16)|xsize;
 b=(y1<<16)|ysize;
 __asm__ __volatile__("int $0x40"::"a"(8),"b"(a),"c"(b),"d"(id),"S"(color));
}

int __menuet__get_button_id(void)
{
 __u16 __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(17));
 if((__ret & 0xFF)==0) return (__ret>>8)&0xFF; else return -1;
}
