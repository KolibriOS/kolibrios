#include<menuet/os.h>

void __menuet__reset_mpu401(void)
{
 __asm__ __volatile__("int $0x40"::"a"(20),"b"(1));
}

void __menuet__write_mpu401(__u8 d)
{
 __asm__ __volatile__("int $0x40"::"a"(20),"b"(2),"c"(d));
}
