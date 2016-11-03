#include <sys/types.h>
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "winlib/winlib.h"
#include "fplay.h"

extern uint32_t  hw2d ;

int64_t _lseeki64(int fd, int64_t offset,  int origin )
{
    int off = offset;
    return lseek(fd, off, origin);
}

int put_packet(queue_t *q, AVPacket *pkt)
{
    AVPacketList *q_pkt;

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
