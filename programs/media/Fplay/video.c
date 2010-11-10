
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "fplay.h"

void video_thread(void *param);

void draw_bitmap(void *bitmap, int x, int y, int w, int h)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(7), "b"(bitmap),
      "c"((w << 16) | h),
      "d"((x << 16) | y));
}

typedef struct
{
    AVFrame       *frame;
    uint8_t       *buffer;
    double         pts;
    volatile int   ready;
}vframe_t;

vframe_t           frames[8];

struct SwsContext *cvt_ctx;

int vfx    = 0;
int dfx    = 0;

int width;
int height;

AVRational video_time_base;
AVFrame  *Frame;

int init_video(AVCodecContext *ctx)
{
    uint32_t   size;
    int        i;

    width = ctx->width;
    height = ctx->height;

    printf("w = %d  h = %d\n\r", width, height);

    Frame = avcodec_alloc_frame();
    if ( Frame == NULL )
    {
        printf("Cannot alloc video buffer\n\r");
        return 0;
    };

    cvt_ctx = sws_getContext(
                            ctx->width,
                            ctx->height,
                            ctx->pix_fmt,
                            ctx->width,
                            ctx->height,
                            PIX_FMT_BGR24,
                            SWS_BILINEAR,
                            NULL, NULL, NULL);
    if(cvt_ctx == NULL)
    {
	    printf("Cannot initialize the conversion context!\n");
	    return 0;
    }

    size = avpicture_get_size(PIX_FMT_RGB24, ctx->width, ctx->height);

    for( i=0; i < 8; i++)
    {
        AVFrame   *frame;

        frame = avcodec_alloc_frame();

        if( frame )
        {
            uint8_t *buffer = (uint8_t*)av_malloc(size);

            if( buffer )
            {
                avpicture_fill((AVPicture *)frame, buffer, PIX_FMT_BGR24,
                               ctx->width, ctx->height);

                frames[i].frame  = frame;
                frames[i].buffer = buffer;
                frames[i].pts = 0;
                frames[i].ready  = 0;
                continue;
            };
        };
        printf("Cannot alloc frame buffer\n\r");
        return 0;
    };

    create_thread(video_thread, 0, 163840);

    return 1;
};

int frameFinished=0;

int decode_video(AVCodecContext  *ctx, AVPacket *pkt)
{
    double     pts;
     AVPicture pict;
     const uint8_t  *data[4];
    double av_time;

   // __asm__("int3");

    if(avcodec_decode_video(ctx, Frame, &frameFinished,
			             pkt->data, pkt->size) <= 0)
        printf("decode error\n");

    if( pkt->dts == AV_NOPTS_VALUE &&
        Frame->reordered_opaque != AV_NOPTS_VALUE)
        pts= Frame->reordered_opaque;
    else if(pkt->dts != AV_NOPTS_VALUE)
        pts= pkt->dts;
    else
        pts= 0;

    pts *= av_q2d(video_time_base);

    if(frameFinished)
    {
        while( frames[dfx].ready != 0 )
            yield();

        pict.data[0] = frames[dfx].frame->data[0];
        pict.data[1] = frames[dfx].frame->data[1];
        pict.data[2] = frames[dfx].frame->data[2];
        pict.data[3] = NULL;

        pict.linesize[0] = frames[dfx].frame->linesize[0];
        pict.linesize[1] = frames[dfx].frame->linesize[1];
        pict.linesize[2] = frames[dfx].frame->linesize[2];
        pict.linesize[3] = 0;

        data[0] = Frame->data[0];
        data[1] = Frame->data[1];
        data[2] = Frame->data[2];
        data[3] = NULL;

        sws_scale(cvt_ctx, data, Frame->linesize, 0, ctx->height,
                  pict.data, pict.linesize);

        frames[dfx].pts = pts*1000.0;
        frames[dfx].ready = 1;

        dfx++;
        dfx&= 7;
    };

    return 0;
}

extern volatile uint32_t status;

typedef unsigned int color_t;
typedef unsigned int count_t;
typedef unsigned int u32_t;

static void DrawWindow(int x, int y, int w, int h, char *name,
                       color_t workcolor, u32_t style)
{

    __asm__ __volatile__(
    "int $0x40"
    ::"a"(0),
      "b"((x << 16) | (w & 0xFFFF)),
      "c"((y << 16) | (h & 0xFFFF)),
      "d"((style << 24) | (workcolor & 0xFFFFFF)),
      "D"(name));
};


static int check_events()
{
    int ev;

    ev = check_os_event();

    switch(ev)
    {
        case 1:
            DrawWindow(10, 10, width+9, height+26, NULL, 0x000000,0x74);
            break;

        case 3:
            if(get_os_button()==1)
                status = 0;
            break;
    };
    return 1;
}


extern char __cmdline[];

void video_thread(void *param)
{
    char *path;

    path = strrchr(__cmdline,'/')+1;

    DrawWindow(10, 10, width+9, height+26, path, 0x000000,0x74);

    while( status != 0)
    {
        double ctime;
        double fdelay;

        check_events();

        if(frames[vfx].ready == 1 )
        {
            ctime = get_master_clock();
            fdelay = (frames[vfx].pts - ctime);
//            printf("ctime %f pts %f delay %f\n",
//                    ctime, frames[vfx].pts, fdelay);

            if(fdelay < 0.0 )
            {
                int  next_vfx;
                fdelay = 0;
                next_vfx = (vfx+1) & 7;
                if( frames[next_vfx].ready == 1 )
                {
                    if(frames[next_vfx].pts <= ctime)
                    {
                        frames[vfx].ready = 0;                  // skip this frame
                        vfx++;
                        vfx&= 7;
                     }
                    else
                    {
                        if( (frames[next_vfx].pts - ctime) <
                            ( ctime - frames[vfx].pts) )
                        {
                            frames[vfx].ready = 0;                  // skip this frame
                            vfx++;
                            vfx&= 7;
                            fdelay = (frames[next_vfx].pts - ctime);
                        }
                    }
                };
            };

            if(fdelay > 10.0)
            {
                delay( (uint32_t)(fdelay/10.0));
            };

            draw_bitmap(frames[vfx].buffer, 0, 0, width, height);
            frames[vfx].ready = 0;
            vfx++;
            vfx&= 7;
        }
        else
        {
            yield();
        };
    };
};

