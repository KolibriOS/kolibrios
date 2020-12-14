#include<menuet/os.h>

void __menuet__set_background_size(__u32 xsz,__u32 ysz)
{
 __asm__ __volatile__("int $0x40"::"a"(15),"b"(1),"c"(xsz),"d"(ysz));
}

void __menuet__write_background_mem(__u32 pos,__u32 color)
{
 __asm__ __volatile__("int $0x40"::"a"(15),"b"(2),"c"(pos),"d"(color));
}

void __menuet__draw_background(void)
{
 __asm__ __volatile__("int $0x40"::"a"(15),"b"(3));
}

void __menuet__set_background_draw_type(int type)
{
 __asm__ __volatile__("int $0x40"::"a"(15),"b"(3),"c"(type));
}

void __menuet__background_blockmove(char * src_ptr,__u32 bgr_dst,__u32 count)
{
 __asm__ __volatile__("int $0x40"::"a"(15),"b"(3),"c"(src_ptr),"d"(bgr_dst),"S"(count));
}
