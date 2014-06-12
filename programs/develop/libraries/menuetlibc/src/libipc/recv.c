#include"ipc.h"
#include<stdlib.h>
#include<string.h>

inline int ipc_messages_avail(ipc_hdr_t * hdr)
{
 return hdr->free_ptr!=8;
}

static inline void lock_msg_queue(ipc_hdr_t * hdr)
{
 int d0;
 __asm__ __volatile__(
     "2:\t"
     "movb $1,%%al\n\t"
     "xchgb %%al,%0\n\t"
     "andb %%al,%%al\n\t"
     "jnz 2b\n\t"
     "incb %0"
     :"=m"(hdr->lock),"=a"(d0)
     :"m"(hdr->lock)
     :"memory");
}

static inline void unlock_msg_queue(ipc_hdr_t * hdr)
{
 __asm__ __volatile__(
     "movl $0,%0"
     :"=m"(hdr->lock)
     :"m"(hdr->lock)
     :"memory");
}

ipc_msg_t * ipc_receive_msg(ipc_hdr_t * hdr)
{
 ipc_msg_t * msg, * tmp;
 lock_msg_queue(hdr);
 if(!ipc_messages_avail(hdr))
 {
  unlock_msg_queue(hdr);
  return NULL;
 }
 tmp=(ipc_msg_t *)hdr->__mem;
 msg=(ipc_msg_t *)malloc(tmp->msg_length);
 if(!msg)
 {
  unlock_msg_queue(hdr);
  return NULL;
 }
 memcpy(msg,tmp,tmp->msg_length);
 if(hdr->free_ptr>(8+tmp->msg_length))
 {
  memcpy(tmp,tmp+1,hdr->free_ptr-8-tmp->msg_length);
  hdr->free_ptr-=tmp->msg_length;
 }
 unlock_msg_queue(hdr);
 return msg;
}
