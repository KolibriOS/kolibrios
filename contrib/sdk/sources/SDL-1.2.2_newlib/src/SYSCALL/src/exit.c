#include<menuet/os.h>

void __menuet__sys_exit(void)
{
 __asm__ __volatile__("int $0x40"::"a"(0xFFFFFFFF));
}
