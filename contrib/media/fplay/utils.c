
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <stdio.h>
#include <fcntl.h>
#include "winlib/winlib.h"
#include "fplay.h"

extern uint32_t  hw2d ;

#if 0
#define FUTEX_INIT      0
#define FUTEX_DESTROY   1
#define FUTEX_WAIT      2
#define FUTEX_WAKE      3

int __fastcall mutex_init(mutex_t *mutex)
{
    unsigned int handle;

    mutex->lock = 0;

    asm volatile(
    "int $0x40\t"
    :"=a"(handle)
    :"a"(77),"b"(FUTEX_INIT),"c"(mutex));
    mutex->handle = handle;

    return handle;
};

int __fastcall mutex_destroy(mutex_t *mutex)
{
    int retval;

    asm volatile(
    "int $0x40\t"
    :"=a"(retval)
    :"a"(77),"b"(FUTEX_DESTROY),"c"(mutex->handle));

    return retval;
};

#define exchange_acquire(ptr, new) \
  __atomic_exchange_4((ptr), (new), __ATOMIC_ACQUIRE)

#define exchange_release(ptr, new) \
  __atomic_exchange_4((ptr), (new), __ATOMIC_RELEASE)

void __fastcall mutex_lock(mutex_t *mutex)
{
    int tmp;

    if( __sync_fetch_and_add(&mutex->lock, 1) == 0)
        return;

    while (exchange_acquire (&mutex->lock, 2) != 0)
    {
        asm volatile(
        "int $0x40\t"
        :"=a"(tmp)
        :"a"(77),"b"(FUTEX_WAIT),
        "c"(mutex->handle),"d"(2),"S"(0));
   }
}

int __fastcall mutex_trylock (mutex_t *mutex)
{
  int zero = 0;

  return __atomic_compare_exchange_4(&mutex->lock, &zero, 1,0,__ATOMIC_ACQUIRE,__ATOMIC_RELAXED);
}

void  __fastcall mutex_unlock(mutex_t *mutex)
{
    int prev;

    prev = exchange_release (&mutex->lock, 0);

    if (prev != 1)
    {
        asm volatile(
        "int $0x40\t"
        :"=a"(prev)
        :"a"(77),"b"(FUTEX_WAKE),
        "c"(mutex->handle),"d"(1));
    };
};
#endif

int64_t _lseeki64(int fd, int64_t offset,  int origin )
{
    int off = offset;
    return lseek(fd, off, origin);
}

int put_packet(queue_t *q, AVPacket *pkt)
{
    AVPacketList *q_pkt;

    /* duplicate the packet */
//    if (av_dup_packet(pkt) < 0)
//        return -1;

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

void blit_raw(ctx_t *ctx, void *raw, int x, int y, int w, int h, int pitch)
{
    int *dst;
    int *src = raw;
    int i, j;

    dst = ctx->pixmap_data;
    dst+=  y * ctx->pixmap_pitch/4 + x;

    for(i=0; i < h; i++)
    {
        for(j = 0; j < w; j++)
            dst[j] = src[j];
        dst+= ctx->pixmap_pitch/4;
        src+= pitch/4;
    };
};
