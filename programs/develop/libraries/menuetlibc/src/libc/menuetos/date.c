#include<menuet/os.h>

__u32 __menuet__get_date(void)
{
 __u32 __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(29));
 return __ret;
}