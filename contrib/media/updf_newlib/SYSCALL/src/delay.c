#include<menuet/os.h>

void __menuet__delay100(int m)
{
 __asm__ __volatile__("int $0x40"::"a"(5),"b"(m));
}

void __menuet__idle(void)
{
 __menuet__delay100(10);
}
