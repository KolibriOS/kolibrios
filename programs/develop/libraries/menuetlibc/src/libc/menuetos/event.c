#include<menuet/os.h>

int __menuet__wait_for_event(void)
{
 __u32 __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(10));
 return __ret;
}

int __menuet__check_for_event(void)
{
 __u32 __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(11));
 return __ret;
}

void __menuet__set_bitfield_for_wanted_events(__u32 ev)
{
 __asm__ __volatile__("int $0x40"::"a"(40),"b"(ev));
}
