
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include "sound.h"
#include "fplay.h"


typedef struct
{
    AVPicture      picture;
    double         pts;
    volatile int   ready;
}vframe_t;

vframe_t           frames[4];

struct SwsContext *cvt_ctx = NULL;

int vfx    = 0;
int dfx    = 0;

render_t   *render;

int width;
int height;

AVRational video_time_base;
AVFrame  *Frame;

volatile uint32_t driver_lock;

void get_client_rect(rect_t *rc);


int init_video(AVCodecContext *ctx)
{
    int        i;

    width = ctx->width;
    height = ctx->height;

    printf("w = %d  h = %d\n\r", width, height);

//  __asm__ __volatile__("int3");

    render = create_render(ctx->width, ctx->height,
                           ctx->pix_fmt, HW_BIT_BLIT|HW_TEX_BLIT);
    if( render == NULL)
    {
        printf("Cannot create render\n\r");
        return 0;
    };

    Frame = avcodec_alloc_frame();
    if ( Frame == NULL )
    {
        printf("Cannot alloc video frame\n\r");
        return 0;
    };

    for( i=0; i < 4; i++)
    {
        int ret;

//        printf("alloc picture %d %d %x\n",
//                   ctx->width, ctx->height, ctx->pix_fmt );

        ret = avpicture_alloc(&frames[i].picture, ctx->pix_fmt,
                               ctx->width, ctx->height);
        if ( ret != 0 )
        {
            printf("Cannot alloc video buffer\n\r");
            return 0;
        };

        frames[i].pts    = 0;
        frames[i].ready  = 0;
    };

    create_thread(video_thread, ctx, 1024*1024);

    delay(50);
    return 1;
};

int frameFinished=0;
static int frame_count;

int decode_video(AVCodecContext  *ctx, queue_t *qv)
{
    AVPacket   pkt;
    double     pts;
    double av_time;

    if(frames[dfx].ready != 0 )
        return 1;

    if( get_packet(qv, &pkt) == 0 )
        return 0;

    frameFinished = 0;

    ctx->reordered_opaque = pkt.pts;

    if(avcodec_decode_video2(ctx, Frame, &frameFinished, &pkt) <= 0)
        printf("video decoder error\n");

    if(frameFinished)
    {
        AVPicture *dst_pic;


        if( pkt.dts == AV_NOPTS_VALUE &&
            Frame->reordered_opaque != AV_NOPTS_VALUE)
        pts = Frame->reordered_opaque;
        else if(pkt.dts != AV_NOPTS_VALUE)
            pts= pkt.dts;
        else
            pts= 0;

//        pts = *(int64_t*)av_opt_ptr(avcodec_get_frame_class(),
//                                Frame, "best_effort_timestamp");

//        if (pts == AV_NOPTS_VALUE)
//            pts = 0;

        pts *= av_q2d(video_time_base);

        dst_pic = &frames[dfx].picture;

        av_image_copy(dst_pic->data, dst_pic->linesize, Frame->data,
                      Frame->linesize, ctx->pix_fmt, ctx->width, ctx->height);

        frames[dfx].pts = pts*1000.0;
        frames[dfx].ready = 1;

        dfx++;
        dfx&= 3;
    };
    av_free_packet(&pkt);

    return 1;
}

extern volatile uint32_t status;
rect_t     win_rect;

int check_events()
{
    int ev;

    ev = check_os_event();

    switch(ev)
    {
       case 1:
            render_adjust_size(render);
            BeginDraw();
            DrawWindow(0,0,0,0, NULL, 0x000000,0x73);
            EndDraw();
            break;

        case 3:
            if(get_os_button()==1)
                status = 0;
            break;
    };
    return 1;
}


extern char *movie_file;

int video_thread(void *param)
{
    rect_t rc;
    AVCodecContext *ctx = param;

    BeginDraw();
    DrawWindow(10, 10, width+9, height+26, movie_file, 0x000000,0x73);
    EndDraw();

    render_adjust_size(render);

    while( status != 0)
    {
        double ctime;
        double fdelay;

        check_events();

        if(frames[vfx].ready == 1 )
        {
            ctime = get_master_clock();
            fdelay = (frames[vfx].pts - ctime);

//            printf("pts %f time %f delay %f\n",
//                    frames[vfx].pts, ctime, fdelay);

            if(fdelay < 0.0 )
            {
                int  next_vfx;
                fdelay = 0;
                next_vfx = (vfx+1) & 3;
                if( frames[next_vfx].ready == 1 )
                {
                    if(frames[next_vfx].pts <= ctime)
                    {
                        frames[vfx].ready = 0;                  // skip this frame
                        vfx++;
                        vfx&= 3;
                     }
                    else
                    {
                        if( (frames[next_vfx].pts - ctime) <
                            ( ctime - frames[vfx].pts) )
                        {
                            frames[vfx].ready = 0;                  // skip this frame
                            vfx++;
                            vfx&= 3;
                            fdelay = (frames[next_vfx].pts - ctime);
                        }
                    }
                };
            };

            if(fdelay > 10.0)
            {
                delay( (uint32_t)(fdelay/10.0));
            };

//            blit_bitmap(&frames[vfx].bitmap, 5, 22, width, height);
//                    frames[vfx].frame->linesize[0]);
            render->draw(render, &frames[vfx].picture);
            frames[vfx].ready = 0;
            vfx++;
            vfx&= 3;
        }
        else
        {
            yield();
        };
    };
    return 0;
};


void draw_hw_picture(render_t *render, AVPicture *picture);
void draw_sw_picture(render_t *render, AVPicture *picture);

render_t *create_render(uint32_t width, uint32_t height,
                        uint32_t ctx_format, uint32_t flags)
{
    render_t *ren;

    render = (render_t*)malloc(sizeof(*ren));
    memset(ren, 0, sizeof(*ren));

    render->ctx_width  = width;
    render->ctx_height = height;
    render->ctx_format = ctx_format;

    mutex_lock(&driver_lock);
    render->caps = InitPixlib(flags);
    mutex_unlock(&driver_lock);

    if(render->caps==0)
    {
        printf("FPlay render engine: Hardware acceleration disabled\n");
        render->draw = draw_sw_picture;
    }
    else
    {
        render->target = 0;
        render->draw   = draw_hw_picture;
    };

    render->state = EMPTY;
    return render;
};

int render_set_size(render_t *render, int width, int height)
{
    int i;

    render->win_width  = width;
    render->win_height = height;
    render->win_state = NORMAL;

//    printf("%s %dx%d\n",__FUNCTION__, width, height);

    if(render->state == EMPTY)
    {
        if(render->caps & HW_TEX_BLIT)
        {
            for( i=0; i < 4; i++)
            {
                render->bitmap[i].width  = render->ctx_width;
                render->bitmap[i].height = render->ctx_height;

                if( create_bitmap(&render->bitmap[i]) != 0 )
                {
                    status = 0;
/*
 *  Epic fail. Need  exit_thread() here
 *
*/
                    return 0;
                };
            }
        }
        else
        {
            render->bitmap[0].width  = width;
            render->bitmap[0].height = height;

            if( create_bitmap(&render->bitmap[0]) != 0 )
                return 0;
        };
        render->state = INIT;
        return 0;
    };

    if(render->caps & HW_TEX_BLIT)          /*  hw scaler  */
        return 0;

    render->bitmap[0].width  = width;
    render->bitmap[0].height = height;
    resize_bitmap(&render->bitmap[0]);

    return 0;
};

void render_adjust_size(render_t *render)
{
    char proc_info[1024];

    uint32_t right, bottom, new_w, new_h;
    uint32_t s, sw, sh;
    uint8_t  state;

    get_proc_info(proc_info);

    right  = *(uint32_t*)(proc_info+62)+1;
    bottom = *(uint32_t*)(proc_info+66)+1;
    state  = *(uint8_t*)(proc_info+70);

    if(state & 2)
    {   render->win_state = MINIMIZED;
        return;
    }
    if(state & 4)
    {
        render->win_state = ROLLED;
        return;
    };

    render->win_state = NORMAL;

    if( right  == render->win_width &&
        bottom == render->win_height)
        return;

    new_w = bottom*render->ctx_width/render->ctx_height;
    new_h = right*render->ctx_height/render->ctx_width;

//    printf("right %d bottom %d\n", right, bottom);
//    printf("new_w %d new_h %d\n", new_w, new_h);

    s  = right * bottom;
    sw = right * new_h;
    sh = bottom * new_w;

    if( abs(s-sw) > abs(s-sh))
        new_h = bottom;
    else
        new_w = right;

    if(new_w < 64)
    {
        new_w = 64;
        new_h = 64*render->ctx_height/render->ctx_width;
    };
    __asm__ __volatile__(
    "int $0x40"
     ::"a"(67), "b"(-1), "c"(-1),
     "d"(new_w+9),"S"(new_h+26)
     :"memory" );
    render_set_size(render, new_w, new_h);

};

void draw_hw_picture(render_t *render, AVPicture *picture)
{
    int      dst_width, dst_height;
    uint8_t     *data[4];
    int      linesize[4];

    if(render->win_state != NORMAL)
        return;

    if(render->caps & HW_TEX_BLIT)
    {
        dst_width  = render->ctx_width;
        dst_height = render->ctx_height;
    }
    else
    {
        dst_width  = render->win_width;
        dst_height = render->win_height;
    };

    cvt_ctx = sws_getCachedContext(cvt_ctx,
              render->ctx_width, render->ctx_height, render->ctx_format,
              dst_width, dst_height, PIX_FMT_BGRA,
              SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if(cvt_ctx == NULL)
    {
        printf("Cannot initialize the conversion context!\n");
        return ;
    };
//    printf("sws_getCachedContext\n");
    data[0] = render->bitmap[render->target].data;
    data[1] = render->bitmap[render->target].data+1;
    data[2] = render->bitmap[render->target].data+2;
    data[3] = render->bitmap[render->target].data+3;

    linesize[0] = render->bitmap[render->target].pitch;
    linesize[1] = render->bitmap[render->target].pitch;
    linesize[2] = render->bitmap[render->target].pitch;
    linesize[3] = render->bitmap[render->target].pitch;

    sws_scale(cvt_ctx, (const uint8_t* const *)picture->data,
              picture->linesize, 0, render->ctx_height, data, linesize);
//    printf("sws_scale\n");

    blit_bitmap(&render->bitmap[render->target], 5, 22,
                 render->win_width, render->win_height);
//    printf("blit_bitmap\n");

    delay(2);
    render->target++;
    render->target&= 3;
}

void draw_sw_picture(render_t *render, AVPicture *picture)
{
    uint8_t     *data[4];
    int      linesize[4];

    if(render->win_state != NORMAL)
        return;

    cvt_ctx = sws_getCachedContext(cvt_ctx,
              render->ctx_width, render->ctx_height,
              render->ctx_format,
              render->win_width, render->win_height,
              PIX_FMT_BGRA, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if(cvt_ctx == NULL)
    {
        printf("Cannot initialize the conversion context!\n");
        return ;
    }

    data[0] = render->bitmap[0].data;
    data[1] = render->bitmap[0].data+1;
    data[2] = render->bitmap[0].data+2;
    data[3] = render->bitmap[0].data+3;


    linesize[0] = render->bitmap[0].pitch;
    linesize[1] = render->bitmap[0].pitch;
    linesize[2] = render->bitmap[0].pitch;
    linesize[3] = render->bitmap[0].pitch;

    sws_scale(cvt_ctx, (const uint8_t* const *)picture->data,
              picture->linesize, 0, render->ctx_height, data, linesize);

    blit_bitmap(&render->bitmap[0], 5, 22,
                render->win_width, render->win_height);
}






