#include"ipc.h"
#include<stdlib.h>

void ipc_send_message(int dst_pid,ipc_msg_t * msg)
{
 int d0,d1,d2,d3,d4;
 __asm__ __volatile__(
     "addl $4,%%edx\n\t"
     "int $0x40"
     :"=a"(d0),"=b"(d1),"=c"(d2),"=d"(d3),"=S"(d4)
     :"0"(60),"1"(2),"2"(dst_pid),"3"(msg),"4"(msg->msg_length)
     :"memory");
}
