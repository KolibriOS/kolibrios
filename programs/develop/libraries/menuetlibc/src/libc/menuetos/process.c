#include<menuet/os.h>

int __menuet__get_process_table(struct process_table_entry * proctab,int pid)
{
 int __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(9),"b"((__u32)proctab),"c"(pid));
 return __ret;
}
