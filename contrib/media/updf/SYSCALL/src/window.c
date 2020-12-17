#include<menuet/os.h>

void __menuet__define_window(__u16 x1,__u16 y1,__u16 xsize,__u16 ysize,
     __u32 body_color,__u32 grab_color,__u32 frame_color)
{
 __u32 a,b;
 a=(x1<<16)|xsize;
 b=(y1<<16)|ysize;
 __asm__ __volatile__("int $0x40"::"a"(0),"b"(a),"c"(b),"d"(body_color),"S"(grab_color),
                      "D"(frame_color));
}

void __menuet__window_redraw(int status)
{
 __asm__ __volatile__("int $0x40"::"a"(12),"b"(status));
}
