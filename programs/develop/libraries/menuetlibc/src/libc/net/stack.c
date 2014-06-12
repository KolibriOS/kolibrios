#include<menuet/net.h>

int __menuet__get_stack_config_word(void)
{
 int __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(__NET_stack),"b"(__NET_stack_rd_cfg_word));
 return __ret;
}

__u32 __menuet__get_my_IP(void)
{
 __u32 __ret;
 __asm__("int $0x40":"=a"(__ret):"a"(__NET_stack),"b"(__NET_stack_get_ip));
 return __ret;
}

void __menuet__set_stack_config_word(int cfg)
{
 __asm__("int $0x40"::"a"(__NET_stack),"b"(__NET_stack_wr_cfg_word),
         "c"(cfg));
}

void __menuet__set_my_IP(__u32 my_IP)
{ 
 __asm__("int $0x40"::"a"(__NET_stack),"b"(__NET_stack_put_ip),"c"(my_IP));
}
