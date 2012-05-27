
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include "system.h"
#include "../winlib/winlib.h"
#include "sound.h"
#include "fplay.h"

#define CAPTION_HEIGHT      24

extern int res_pause_btn[];
extern int res_pause_btn_pressed[];

extern int res_play_btn[];
extern int res_play_btn_pressed[];

extern int64_t stream_duration;

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

render_t   *main_render;

int width;
int height;

AVRational video_time_base;
AVFrame  *Frame;

volatile uint32_t driver_lock;

void get_client_rect(rect_t *rc);

void flush_video()
{
    int i;

    for(i = 0; i < 4; i++)
    {    
        frames[i].pts    = 0;
        frames[i].ready  = 0;
    };
    vfx    = 0;
    dfx    = 0;
};

int init_video(AVCodecContext *ctx)
{
    int        i;

    width = ctx->width;
    height = ctx->height;


    printf("w = %d  h = %d\n\r", width, height);

//    __asm__ __volatile__("int3");

    main_render = create_render(ctx->width, ctx->height,
                           ctx->pix_fmt, HW_BIT_BLIT|HW_TEX_BLIT);
//    render = create_render(ctx->width, ctx->height,
//                           ctx->pix_fmt, 0);
//
    if( main_render == NULL)
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

int decode_video(AVCodecContext  *ctx, queue_t *qv)
{
    AVPacket   pkt;
    double     pts;
    int frameFinished;
    double current_clock;

    if(frames[dfx].ready != 0 )
        return 1;

    if( get_packet(qv, &pkt) == 0 )
        return 0;

    current_clock = -90.0 + get_master_clock();

    if( pkt.dts == AV_NOPTS_VALUE &&
        Frame->reordered_opaque != AV_NOPTS_VALUE)
        pts = Frame->reordered_opaque;
    else if(pkt.dts != AV_NOPTS_VALUE)
        pts= pkt.dts;
    else
        pts= 0;
        
  
    pts *= av_q2d(video_time_base)*1000.0;

    if( 1 /*pts > current_clock*/)
    {
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

            av_image_copy(dst_pic->data, dst_pic->linesize,
                      (const uint8_t**)Frame->data,
                      Frame->linesize, ctx->pix_fmt, ctx->width, ctx->height);

            frames[dfx].pts = pts*1000.0;
//            printf("pts %f\n", frames[dfx].pts);

            frames[dfx].ready = 1;

            dfx++;
            dfx&= 3;
        };
    };
    av_free_packet(&pkt);

    return 1;
}

extern volatile enum player_state player_state;
//rect_t     win_rect;

extern int64_t rewind_pos;

int MainWindowProc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    window_t  *win;

    win = (window_t*)ctrl;

    switch(msg)
    {
        case MSG_SIZE:
            //printf("MSG_SIZE\n");
            render_adjust_size(main_render, win);
            break;

        case MSG_DRAW_CLIENT:
            render_draw_client(main_render);
            break;

        case MSG_LBTNDOWN:
            if(player_state == PAUSE)
            {
                win->panel.play_btn->img_default = res_pause_btn;
                win->panel.play_btn->img_hilite  = res_pause_btn;
                win->panel.play_btn->img_pressed = res_pause_btn_pressed;
                send_message(win->panel.play_btn, MSG_PAINT, 0, 0);
                player_state = PAUSE_2_PLAY;

            }
            else if(player_state == PLAY)
            {
                win->panel.play_btn->img_default = res_play_btn;
                win->panel.play_btn->img_hilite  = res_play_btn;
                win->panel.play_btn->img_pressed = res_play_btn_pressed;
                send_message(win->panel.play_btn, MSG_PAINT, 0, 0);
                player_state = PAUSE;
            }
            break;

        case MSG_COMMAND:
            switch((short)arg1)
            {
                case ID_PLAY:
                    if(player_state == PAUSE)
                    {
                        win->panel.play_btn->img_default  = res_pause_btn;
                        win->panel.play_btn->img_hilite   = res_pause_btn;
                        win->panel.play_btn->img_pressed = res_pause_btn_pressed;
                        player_state = PAUSE_2_PLAY;
                    }
                    else if(player_state == PLAY)
                    {
                        win->panel.play_btn->img_default = res_play_btn;
                        win->panel.play_btn->img_hilite  = res_play_btn;
                        win->panel.play_btn->img_pressed  = res_play_btn_pressed;
                        player_state = PAUSE;
                    }
                    break;

                case 101:  //ID_PROGRESS:
                    if(player_state != REWIND)
                    {
                        progress_t *prg = (progress_t*)arg2;
                    
                        rewind_pos = (int64_t)prg->pos *
                              (prg->max - prg->min)/prg->ctrl.w;
                              
//                        printf("progress action %f\n", (double)rewind_pos);
                        player_state = REWIND;
                        main_render->win->panel.prg->current = rewind_pos;
                        send_message(&main_render->win->panel.ctrl, MSG_PAINT, 0, 0);
                    };
                    break;
                      
                default:
                    break;
            }
            break;

        default:
            def_window_proc(ctrl,msg,arg1,arg2);
    };
    return 0;
};

#define VERSION_A          1

void render_time(render_t *render)
{
    double ctime;            /*    milliseconds    */
    double fdelay;           /*    milliseconds    */

//again:

    if(player_state == CLOSED)
    {
        render->win->win_command = WIN_CLOSED;
        return;
    }
    else if(player_state != PLAY)
    {
        yield();
        return;
    };

#ifdef VERSION_A
    if(frames[vfx].ready == 1 )
    {
        int sys_time;

        ctime = get_master_clock();
        fdelay = (frames[vfx].pts - ctime);

//        printf("pts %f time %f delay %f\n",
//                frames[vfx].pts, ctime, fdelay);

        if(fdelay > 15.0)
        {
            delay(1);
//            yield();
            return;
        };

        ctime = get_master_clock();
        fdelay = (frames[vfx].pts - ctime);

        sys_time = get_tick_count();

//      if(fdelay < 0)
//            printf("systime %d pts %f time %f delay %f\n",
//                    sys_time*10, frames[vfx].pts, ctime, fdelay);

        main_render->draw(main_render, &frames[vfx].picture);
        main_render->win->panel.prg->current = frames[vfx].pts*1000;
        send_message(&render->win->panel.prg->ctrl, MSG_PAINT, 0, 0);
        frames[vfx].ready = 0;
        vfx++;
        vfx&= 3;
    }
    else yield();

#else

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
           int val = fdelay;
           printf("pts %f time %f delay %d\n",
                   frames[vfx].pts, ctime, val);
           delay(val/10);
        };

        ctime = get_master_clock();
        fdelay = (frames[vfx].pts - ctime);

        printf("pts %f time %f delay %f\n",
                frames[vfx].pts, ctime, fdelay);

        main_render->draw(main_render, &frames[vfx].picture);
        main_render->win->panel.prg->current = frames[vfx].pts;
//        send_message(&render->win->panel.prg->ctrl, MSG_PAINT, 0, 0);
        frames[vfx].ready = 0;
        vfx++;
        vfx&= 3;
    }
    else yield();
#endif

}




extern char *movie_file;

int video_thread(void *param)
{
    window_t    *MainWindow;

    init_winlib();

    MainWindow = create_window(movie_file,0,
                               10,10,width,height+29+55,MainWindowProc);

    MainWindow->panel.prg->max = stream_duration;
//    printf("MainWindow %x\n", MainWindow);

    main_render->win = MainWindow;

    show_window(MainWindow, NORMAL);

    render_draw_client(main_render);
    player_state = PAUSE_2_PLAY;

    run_render(MainWindow, main_render);

//    printf("exit thread\n");
    player_state = CLOSED;
    return 0;
};


void draw_hw_picture(render_t *render, AVPicture *picture);
void draw_sw_picture(render_t *render, AVPicture *picture);

render_t *create_render(uint32_t width, uint32_t height,
                        uint32_t ctx_format, uint32_t flags)
{
    render_t *render;

//    __asm__ __volatile__("int3");

    render = (render_t*)malloc(sizeof(render_t));
    memset(render, 0, sizeof(render_t));

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

    render->layout = 0;
    render->rcvideo.l = 0;
    render->rcvideo.t = 0;
    render->rcvideo.r = width;
    render->rcvideo.b = height;

    if( render->win_height > height )
    {
        int yoffs;
        yoffs = (render->win_height-height)/2;
        if(yoffs)
        {
            render->rctop.t = 0;
            render->rctop.b = yoffs;
            render->rcvideo.t  = yoffs;
            render->layout |= HAS_TOP;
        }

        yoffs = render->win_height-(render->rcvideo.t+render->rcvideo.b);
        if(yoffs)
        {
            render->rcbottom.t = render->rcvideo.t+render->rcvideo.b;
            render->rcbottom.b = yoffs;
            render->layout |= HAS_BOTTOM;
        }
    }

    if( render->win_width > width )
    {
        int xoffs;
        xoffs = (render->win_width-width)/2;
        if(xoffs)
        {
            render->rcleft.r  = xoffs;
            render->rcvideo.l = xoffs;
            render->layout |= HAS_LEFT;
        }
        xoffs = render->win_width-(render->rcvideo.l+render->rcvideo.r);
        if(xoffs)
        {
            render->rcright.l = render->rcvideo.l+render->rcvideo.r;
            render->rcright.r = xoffs;
            render->layout |= HAS_RIGHT;
        }
    };

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
                    player_state = CLOSED;
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

void render_adjust_size(render_t *render, window_t *win)
{
    uint32_t right, bottom, new_w, new_h;
    uint32_t s, sw, sh;
    uint8_t  state;


    right  = win->w;
    bottom = win->h-CAPTION_HEIGHT-55;
    render->win_state  = win->win_state;

    if(render->win_state == MINIMIZED)
        return;

    if(render->win_state == ROLLED)
        return;

    if( right  == render->win_width &&
        bottom == render->win_height)
        return;

    new_w = bottom*render->ctx_width/render->ctx_height;
    new_h = right*render->ctx_height/render->ctx_width;

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

    render->win_width  = win->w;
    render->win_height = win->h-CAPTION_HEIGHT-55;
    render_set_size(render, new_w, new_h);
};

void draw_hw_picture(render_t *render, AVPicture *picture)
{
    int      dst_width, dst_height;
    bitmap_t   *bitmap;
    uint8_t    *data[4];
    int      linesize[4];
    int ret;

    if(render->win_state == MINIMIZED ||
       render->win_state == ROLLED)
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

    bitmap = &render->bitmap[render->target];

    ret = lock_bitmap(bitmap);
    if( ret != 0)
    {
        printf("Cannot lock the bitmap!\n");
        return ;
    }

//    printf("sws_getCachedContext\n");
    data[0] = bitmap->data;
    data[1] = bitmap->data+1;
    data[2] = bitmap->data+2;
    data[3] = bitmap->data+3;

    linesize[0] = bitmap->pitch;
    linesize[1] = bitmap->pitch;
    linesize[2] = bitmap->pitch;
    linesize[3] = bitmap->pitch;

    sws_scale(cvt_ctx, (const uint8_t* const *)picture->data,
              picture->linesize, 0, render->ctx_height, data, linesize);
//    printf("sws_scale\n");

    blit_bitmap(bitmap, render->rcvideo.l,
                 CAPTION_HEIGHT+render->rcvideo.t,
                 render->rcvideo.r, render->rcvideo.b);
    render->last_bitmap = bitmap;
//    printf("blit_bitmap\n");


    render->target++;
    render->target&= 3;
}

void draw_sw_picture(render_t *render, AVPicture *picture)
{
    uint8_t     *data[4];
    int      linesize[4];

    if(render->win_state == MINIMIZED ||
       render->win_state == ROLLED)
        return;

    cvt_ctx = sws_getCachedContext(cvt_ctx,
              render->ctx_width, render->ctx_height,
              render->ctx_format,
              render->rcvideo.r, render->rcvideo.b,
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

    blit_bitmap(&render->bitmap[0], render->rcvideo.l,
                render->rcvideo.t+CAPTION_HEIGHT,
                render->rcvideo.r, render->rcvideo.b);
    render->last_bitmap = &render->bitmap[0];
}

void render_draw_client(render_t *render)
{
    if(render->win_state == MINIMIZED ||
       render->win_state == ROLLED)
        return;

    if((player_state == PAUSE) ||
       (player_state == PLAY_INIT) )
    {
         if(frames[vfx].ready == 1 )
            main_render->draw(main_render, &frames[vfx].picture);
         else
            draw_bar(0, CAPTION_HEIGHT, render->win_width,
                 render->rcvideo.b, 0);
    };

    if(render->layout & HAS_TOP)
        draw_bar(0, CAPTION_HEIGHT, render->win_width,
                 render->rctop.b, 0);
    if(render->layout & HAS_LEFT)
        draw_bar(0, render->rcvideo.t+CAPTION_HEIGHT, render->rcleft.r,
                 render->rcvideo.b, 0);
    if(render->layout & HAS_RIGHT)
        draw_bar(render->rcright.l, render->rcvideo.t+CAPTION_HEIGHT,
                 render->rcright.r, render->rcvideo.b, 0);
    if(render->layout & HAS_BOTTOM)
        draw_bar(0, render->rcbottom.t+CAPTION_HEIGHT,
                 render->win_width, render->rcbottom.b, 0);
}





