#ifndef __QUEUE_H
#define __QUEUE_H

#include<menuet/os.h>
#include<menuet/sem.h>

typedef struct
{
 int in_ptr,out_ptr,size;
 unsigned char * databuf;
 DECLARE_SEMAPHORE_S(q_lock);
} __oe_queue_t;

#define DECL_EMPTY_Q(n,sz,buf)	__oe_queue_t n = {0,0,sz,buf,SEM_INIT}

#define __OE_QUEUE_INQ(x,c) \
    ({ \
	register int __ret,temp; \
	sem_lock(&(x)->q_lock); \
	temp=(x)->in_ptr+1; \
	if(temp>=(x)->size) temp=0; \
	if(temp==(x)->out_ptr) { __ret=-1; goto __OEQI_D; } \
	(x)->databuf[(x)->in_ptr]=(c)&0xFF; \
	(x)->in_ptr=temp; \
	__ret=0; \
__OEQI_D: \
	sem_unlock(&(x)->q_lock); \
	__ret; })
	
#define __OE_QUEUE_DEQ(x,c) \
    ({ \
	register int __ret; \
	register unsigned char __tmp; \
	sem_lock(&(x)->q_lock); \
	if((x)->out_ptr==(x)->in_ptr) { __ret=-1; goto __OEQD_D; } \
	__tmp=(x)->databuf[(x)->out_ptr++]; \
	if((x)->out_ptr>=(x)->size) (x)->out_ptr=0; \
	__ret=0; \
	(c)=__tmp; \
__OEQD_D: \
	sem_unlock(&(x)->q_lock); \
	__ret; })
	
#endif
