#include<menuet/os.h>

int __kolibri__system_tree_access(struct systree_info * info)
{
 int __ret;
 int d0;
 __asm__ __volatile__("int $0x40":"=a"(__ret),"=&b"(d0):"0"(70),"1"((__u32)info));
 return __ret;
}
int __kolibri__system_tree_access2(struct systree_info2 * info)
{
 int __ret;
 int d0;
 __asm__ __volatile__("int $0x40":"=a"(__ret),"=&b"(d0):"0"(70),"1"((__u32)info));
 return __ret;
}
