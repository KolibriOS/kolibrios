
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <stdio.h>
#include <fcntl.h>
#include <winlib.h>
#include "fplay.h"

extern uint32_t  hw2d ;

void mutex_lock(volatile uint32_t *val)
{
    uint32_t tmp;

    __asm__ __volatile__ (
"0:\n\t"
    "mov %0, %1\n\t"
    "testl %1, %1\n\t"
    "jz 1f\n\t"

    "movl $68, %%eax\n\t"
    "movl $1,  %%ebx\n\t"
    "int  $0x40\n\t"
    "jmp 0b\n\t"
"1:\n\t"
    "incl %1\n\t"
    "xchgl %0, %1\n\t"
    "testl %1, %1\n\t"
    "jnz 0b\n"
    : "+m" (*val), "=&r"(tmp)
    ::"eax","ebx" );
}


int64_t _lseeki64(int fd, int64_t offset,  int origin )
{
    int off = offset;
    return lseek(fd, off, origin);
}


int put_packet(queue_t *q, AVPacket *pkt)
{
    AVPacketList *q_pkt;

    /* duplicate the packet */
    if (av_dup_packet(pkt) < 0)
        return -1;

    q_pkt = av_malloc(sizeof(AVPacketList));
    if (!q_pkt)
        return -1;

    q_pkt->pkt = *pkt;
    q_pkt->next = NULL;

    mutex_lock(&q->lock);

    if (!q->last_pkt)
        q->first_pkt = q_pkt;
    else
        q->last_pkt->next = q_pkt;

    q->last_pkt = q_pkt;
    q->size += q_pkt->pkt.size + sizeof(*q_pkt);
    q->count++;

    mutex_unlock(&q->lock);

    return 0;
}

int get_packet(queue_t *q, AVPacket *pkt)
{
    AVPacketList *q_pkt;
    int ret = 0;

    mutex_lock(&q->lock);

    q_pkt = q->first_pkt;
    if (q_pkt)
    {
        q->first_pkt = q_pkt->next;
        if (!q->first_pkt)
            q->last_pkt = NULL;

        q->count--;
        q->size -= q_pkt->pkt.size + sizeof(*q_pkt);
        *pkt = q_pkt->pkt;
        av_free(q_pkt);
        ret = 1;
    };

    mutex_unlock(&q->lock);

    return ret;
}
