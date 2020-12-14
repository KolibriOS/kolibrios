#include<menuet/os.h>

void send_message(int pid,void * msg_ptr,int message_size)
{
 __asm__ __volatile__("int $0x40"::"a"(60),"b"(2),"c"(pid),"d"(msg_ptr),"S"(message_size));
}

void define_receive_area(msgrcva_t * rcva_ptr,int size)
{
 __asm__ __volatile__("int $0x40"::"a"(60),"b"(1),"c"(rcva_ptr),"d"(size));
}
