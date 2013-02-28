
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include "system.h"
#include "../winlib/winlib.h"
#include "sound.h"
#include "fplay.h"
#include <math.h>

extern int res_pause_btn[];
extern int res_pause_btn_pressed[];

extern int res_play_btn[];
extern int res_play_btn_pressed[];

extern int64_t stream_duration;
extern volatile int sound_level_0;
extern volatile int sound_level_1;

typedef struct
{
    AVPicture      picture;
    double         pts;
    volatile int   ready;
}vframe_t;

vframe_t           frames[4];
volatile int      frames_count = 0;

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
    frames_count = 0;
    vfx    = 0;
    dfx    = 0;
};

int init_video(AVCodecContext *ctx)
{
    int        i;

    width = ctx->width;
    height = ctx->height;


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
        return -1;

    if( get_packet(qv, &pkt) == 0 )
        return 0;

/*
    current_clock = -90.0 + get_master_clock();

    if( pkt.dts == AV_NOPTS_VALUE &&
        Frame->reordered_opaque != AV_NOPTS_VALUE)
        pts = Frame->reordered_opaque;
    else if(pkt.dts != AV_NOPTS_VALUE)
        pts= pkt.dts;
    else
        pts= 0;
        
  
    pts *= av_q2d(video_time_base)*1000.0;
*/
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
            frames_count++;
        };
    };
    av_free_packet(&pkt);

    return 1;
}

extern volatile enum player_state player_state;
extern volatile enum player_state decoder_state;
extern volatile enum player_state sound_state;

//rect_t     win_rect;

extern int64_t rewind_pos;

static void player_stop()
{
    window_t  *win;    
    
    win = main_render->win;
    
    rewind_pos = 0;
    
    win->panel.play_btn->img_default    = res_play_btn;
    win->panel.play_btn->img_hilite     = res_play_btn;
    win->panel.play_btn->img_pressed    = res_play_btn_pressed;
    win->panel.prg->current             = rewind_pos;
    
    send_message(&win->panel.ctrl, MSG_PAINT, 0, 0);
    player_state = STOP;
    decoder_state = PLAY_2_STOP;
    sound_state = PLAY_2_STOP;
    render_draw_client(main_render);
//    printf("stop player\n");
    
};

int MainWindowProc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    window_t  *win;

    win = (window_t*)ctrl;

    switch(msg)
    {
        case MSG_SIZE:
            //printf("MSG_SIZE\n");
            if(main_render)
            {
                render_adjust_size(main_render, win);
                render_draw_client(main_render);
            };
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
                send_message(&win->panel.play_btn->ctrl, MSG_PAINT, 0, 0);
                player_state = PLAY;
                sound_state  = PAUSE_2_PLAY;

            }
            else if(player_state == PLAY)
            {
                win->panel.play_btn->img_default = res_play_btn;
                win->panel.play_btn->img_hilite  = res_play_btn;
                win->panel.play_btn->img_pressed = res_play_btn_pressed;
                send_message(&win->panel.play_btn->ctrl, MSG_PAINT, 0, 0);
                player_state = PAUSE;
                sound_state  = PLAY_2_PAUSE;
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
                        player_state = PLAY;
                        sound_state = PAUSE_2_PLAY;
                    }
                    else if(player_state == PLAY)
                    {
                        win->panel.play_btn->img_default = res_play_btn;
                        win->panel.play_btn->img_hilite  = res_play_btn;
                        win->panel.play_btn->img_pressed  = res_play_btn_pressed;
                        player_state = PAUSE;
                        sound_state  = PLAY_2_PAUSE;
                    }
                    else if(player_state == STOP)
                    {
                        win->panel.play_btn->img_default  = res_pause_btn;
                        win->panel.play_btn->img_hilite   = res_pause_btn;
                        win->panel.play_btn->img_pressed = res_pause_btn_pressed;
                        rewind_pos = 0;    
                        send_message(&win->panel.ctrl, MSG_PAINT, 0, 0);
                        player_state = PLAY;
                        decoder_state = PREPARE;
                    }
                    break;
                    
                case ID_STOP:
                    player_stop();
                    break;
                    
                case ID_PROGRESS:
                    if(player_state != REWIND)
                    {
                        progress_t *prg = (progress_t*)arg2;
                    
                        rewind_pos = (prg->max - prg->min)*prg->pos/prg->ctrl.w;
                              
//                        printf("progress action pos: %d time: %f\n", prg->pos, (double)rewind_pos);
                        player_state  = REWIND;
                        decoder_state = REWIND;
                        sound_state   = PLAY_2_STOP;
                        if(rewind_pos < prg->current)
                        {
                            prg->current  = rewind_pos;
                            rewind_pos = -rewind_pos;
                        }
                        else
                            prg->current  = rewind_pos;
                        
                        win->panel.play_btn->img_default  = res_pause_btn;
                        win->panel.play_btn->img_hilite   = res_pause_btn;
                        win->panel.play_btn->img_pressed  = res_pause_btn_pressed;
                        send_message(&prg->ctrl, MSG_PAINT, 0, 0);
                    };
                    break;
                    
                case ID_VOL_CTRL:      
                {
                    slider_t *sld = (slider_t*)arg2;
                    int      peak;
                    int      level;
                    
                    peak = sld->min + sld->pos * (sld->max - sld->min)/(96);
//                    level = (log2(peak+16384)*10000.0)/15 - 10000;
                    level =  peak;
                     
//                    printf("level %d\n", level);
                    set_audio_volume(level, level);
                    send_message(&sld->ctrl, MSG_PAINT, 0, 0);
                    win->panel.lvl->vol = level;
                }
                
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
    progress_t  *prg = main_render->win->panel.prg; 
    level_t     *lvl = main_render->win->panel.lvl; 
      
    double      ctime;            /*    milliseconds    */
    double      fdelay;           /*    milliseconds    */

//again:

    if(player_state == CLOSED)
    {
        render->win->win_command = WIN_CLOSED;
        return;
    }
    else if(player_state == REWIND)
    {
        yield();
        return;   
    }
    else if (decoder_state == STOP && frames_count  == 0 &&
              player_state  != STOP)
    {          
        player_stop();
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
        prg->current = frames[vfx].pts*1000;
//        printf("current %f\n", prg->current);
        lvl->current = vfx & 1 ? sound_level_1 : sound_level_0;
        
        send_message(&prg->ctrl, PRG_PROGRESS, 0, 0);
        
        if(main_render->win->panel.layout)
            send_message(&lvl->ctrl, MSG_PAINT, 0, 0);
        
        frames_count--;
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
    AVCodecContext *ctx = param;
    window_t  *MainWindow;
    
    init_winlib();

    MainWindow = create_window(movie_file,0,
                               10,10,width,height+CAPTION_HEIGHT+PANEL_HEIGHT,MainWindowProc);

    MainWindow->panel.prg->max = stream_duration;
//    printf("MainWindow %x\n", MainWindow);

    show_window(MainWindow, NORMAL);

//    __asm__ __volatile__("int3");

    main_render = create_render(MainWindow, ctx, HW_BIT_BLIT|HW_TEX_BLIT);
    if( main_render == NULL)
    {
        printf("Cannot create render\n\r");
        return 0;
    };

    render_draw_client(main_render);
    player_state = PLAY;

    run_render(MainWindow, main_render);

    destroy_render(main_render);
    fini_winlib();

    player_state = CLOSED;
    return 0;
};


void draw_hw_picture(render_t *render, AVPicture *picture);
void draw_sw_picture(render_t *render, AVPicture *picture);

render_t *create_render(window_t *win, AVCodecContext *ctx, uint32_t flags)
{
    render_t *render;

    uint32_t right, bottom, draw_w, draw_h;
    uint32_t s, sw, sh;
    uint8_t  state;

//    __asm__ __volatile__("int3");

    render = (render_t*)malloc(sizeof(render_t));
    memset(render, 0, sizeof(render_t));

    render->win = win;

    render->ctx_width  = ctx->width;
    render->ctx_height = ctx->height;
    render->ctx_format = ctx->pix_fmt;

    mutex_lock(&driver_lock);
    render->caps = init_pixlib(flags);
    mutex_unlock(&driver_lock);
    
    right  = win->w;
    bottom = win->h-CAPTION_HEIGHT-PANEL_HEIGHT;

 //   printf("window width %d height %d\n",
 //                   right, bottom);

    render->win_state  = win->win_state;

    draw_w = bottom*render->ctx_width/render->ctx_height;
    draw_h = right*render->ctx_height/render->ctx_width;

    if(draw_w > right)
    {
        draw_w = right;
        draw_h = right*render->ctx_height/render->ctx_width;
    };
    
    if(draw_h > bottom)
    {
        draw_h = bottom;
        draw_w = bottom*render->ctx_width/render->ctx_height;
    };

    render->win_width  = win->w;
    render->win_height = win->h-CAPTION_HEIGHT-PANEL_HEIGHT;
   
    render_set_size(render, draw_w, draw_h);
    

    if(render->caps==0)
    {
        render->bitmap[0].width  = draw_w;
        render->bitmap[0].height = draw_h;

        if( create_bitmap(&render->bitmap[0]) != 0 )
        {
            free(render);
            return NULL;
        }
        render->draw = draw_sw_picture;
    }
    else
    {
        int width, height, flags;
        int i;
        
        if(render->caps & HW_TEX_BLIT)
        {
            sna_create_mask();

            width  = render->ctx_width;     
            height = render->ctx_height;     
            flags  = HW_TEX_BLIT;     
        }
        else
        {
            width  = draw_w;     
            height = draw_h;;     
            flags  = HW_BIT_BLIT;     
        }

        for( i=0; i < 2; i++)
        {
            render->bitmap[i].width  = width;
            render->bitmap[i].height = height;
            render->bitmap[i].flags  = flags;

            if( create_bitmap(&render->bitmap[i]) != 0 )
            {
                player_state = CLOSED;
                free(render);
                return NULL;
            };
        }
        
        render->state = INIT;
        render->target = 0;
        render->draw   = draw_hw_picture;
    };
    
    printf("FPlay %s render engine: context %dx%d picture %dx%d\n",
           render->caps==0 ? "software":"hardware",
           render->ctx_width, render->ctx_height,
           draw_w, draw_h);
             
    return render;
};

void destroy_render(render_t *render)
{

    destroy_bitmap(&render->bitmap[0]);
    
    if(render->caps & (HW_BIT_BLIT|HW_TEX_BLIT))          /* hw blitter */
        destroy_bitmap(&render->bitmap[1]);
    
    done_pixlib();  
};

void render_set_size(render_t *render, int width, int height)
{
    int i;

    render->layout = 0;
    render->rcvideo.l = 0;
    render->rcvideo.t = 0;
    render->rcvideo.r = width;
    render->rcvideo.b = height;

//    printf("render width %d height %d\n",width, height);

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
};

void render_adjust_size(render_t *render, window_t *win)
{
    uint32_t right, bottom, new_w, new_h;
    uint32_t s, sw, sh;
    uint8_t  state;

    right  = win->w;
    bottom = win->h-CAPTION_HEIGHT-PANEL_HEIGHT;

 //   printf("window width %d height %d\n",
 //                   right, bottom);

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

    if(new_w > right)
    {
        new_w = right;
        new_h = right*render->ctx_height/render->ctx_width;
    };
    if(new_h > bottom)
    {
        new_h = bottom;
        new_w = bottom*render->ctx_width/render->ctx_height;
    };

    render->win_width  = win->w;
    render->win_height = win->h-CAPTION_HEIGHT-PANEL_HEIGHT;
    render_set_size(render, new_w, new_h);

    if(render->caps & HW_TEX_BLIT)          /*  hw scaler  */
        return;

    render->bitmap[0].width  = new_w;
    render->bitmap[0].height = new_h;
    resize_bitmap(&render->bitmap[0]);

    if(render->caps & HW_BIT_BLIT)          /* hw blitter */
    {
        render->bitmap[1].width  = new_w;
        render->bitmap[1].height = new_h;
        resize_bitmap(&render->bitmap[1]);
    };
    return;
};

void draw_hw_picture(render_t *render, AVPicture *picture)
{
    int      dst_width, dst_height;
    bitmap_t   *bitmap;
    uint8_t    *data[4];
    int      linesize[4];
    int ret;

    if(render->win->win_state == MINIMIZED ||
       render->win->win_state == ROLLED)
        return;

    if(render->caps & HW_TEX_BLIT)
    {
        dst_width  = render->ctx_width;
        dst_height = render->ctx_height;
    }
    else
    {
        dst_width  = render->rcvideo.r;
        dst_height = render->rcvideo.b;
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
        printf("Cannot lock bitmap!\n");
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
    render->target&= 1;
}

void draw_sw_picture(render_t *render, AVPicture *picture)
{
    uint8_t     *data[4];
    int      linesize[4];

    if(render->win->win_state == MINIMIZED ||
       render->win->win_state == ROLLED)
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

    lock_bitmap(&render->bitmap[0]);

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

    if(player_state == PAUSE) 
    {
         if(frames[vfx].ready == 1 )
            main_render->draw(main_render, &frames[vfx].picture);
         else
            draw_bar(0, CAPTION_HEIGHT, render->win_width,
                 render->rcvideo.b, 0);
    }
    else if( player_state == STOP )
    {
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





