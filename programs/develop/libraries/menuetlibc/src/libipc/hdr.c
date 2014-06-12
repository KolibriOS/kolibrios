#include"ipc.h"
#include<stdlib.h>

ipc_hdr_t * create_ipc(unsigned long size)
{
 ipc_hdr_t * hdr=(ipc_hdr_t *)malloc(size+sizeof(ipc_hdr_t));
 if(!hdr) return NULL;
 hdr->lock=0;
 hdr->free_ptr=8;
 return hdr;
}

void register_ipc_mem(ipc_hdr_t * hdr)
{
 int d0,d1,d2;
 __asm__ __volatile__(
     "int $0x40"
     :"=a"(d0),"=b"(d1),"=c"(d2)
     :"0"(60),"1"(1),"2"(hdr)
     :"memory");
}
