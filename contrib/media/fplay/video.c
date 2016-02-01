
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <math.h>
#include <kos32sys.h>
#include <sound.h>

#include "winlib/winlib.h"
#include "fplay.h"

extern int res_pause_btn[];
extern int res_pause_btn_pressed[];

extern int res_play_btn[];
extern int res_play_btn_pressed[];

extern int64_t stream_duration;
extern volatile int sound_level_0;
extern volatile int sound_level_1;

volatile int frames_count = 0;

struct SwsContext *cvt_ctx = NULL;


render_t   *main_render;

int width;
int height;

AVRational video_time_base;
AVFrame  *Frame;

void get_client_rect(rect_t *rc);

void flush_video(vst_t *vst)
{
    int i;

    for(i = 0; i < 4; i++)
    {
        vst->vframe[i].pts   = 0;
        vst->vframe[i].ready = 0;
    };
    vst->vfx = 0;
    vst->dfx = 0;
    frames_count = 0;
};

int init_video(vst_t *vst)
{
    int        i;

    width  = vst->vCtx->width;
    height = vst->vCtx->height;

    Frame = av_frame_alloc();
    if ( Frame == NULL )
    {
        printf("Cannot alloc video frame\n\r");
        return 0;
    };

    create_thread(video_thread, vst, 1024*1024);

    delay(50);
    return 1;
};

int decode_video(vst_t* vst)
{
    AVPacket   pkt;
    double     pts;
    int frameFinished;
    double current_clock;

    if(vst->vframe[vst->dfx].ready != 0 )
        return -1;

    if( get_packet(&vst->q_video, &pkt) == 0 )
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

        vst->vCtx->reordered_opaque = pkt.pts;

        mutex_lock(&vst->gpu_lock);

        if(avcodec_decode_video2(vst->vCtx, Frame, &frameFinished, &pkt) <= 0)
            printf("video decoder error\n");

        if(frameFinished)
        {
            AVPicture *dst_pic;

            if( pkt.dts == AV_NOPTS_VALUE &&
                Frame->reordered_opaque != AV_NOPTS_VALUE)
                pts = Frame->reordered_opaque;
            else if(pkt.dts != AV_NOPTS_VALUE)
                pts = pkt.dts;
            else
                pts = 0;

            pts *= av_q2d(video_time_base);

            dst_pic = &vst->vframe[vst->dfx].picture;

            if(vst->hwdec == 0)
                av_image_copy(dst_pic->data, dst_pic->linesize,
                              (const uint8_t**)Frame->data,
                              Frame->linesize, vst->vCtx->pix_fmt, vst->vCtx->width, vst->vCtx->height);
            else
                va_convert_picture(vst, vst->vCtx->width, vst->vCtx->height, dst_pic);

            vst->vframe[vst->dfx].pts = pts*1000.0;
            vst->vframe[vst->dfx].ready = 1;
            vst->dfx = (vst->dfx + 1) & 3;
            frames_count++;
        };
        mutex_unlock(&vst->gpu_lock);

    };
    av_free_packet(&pkt);

    return 1;
}

extern volatile enum player_state player_state;
extern volatile enum player_state decoder_state;
extern volatile enum player_state sound_state;

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
};

int MainWindowProc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    window_t  *win = (window_t*)ctrl;
    static int spc_down = 0;
    static int ent_down = 0;

    switch(msg)
    {
        case MSG_SIZE:
            if(main_render)
            {
                render_adjust_size(main_render, win);
                render_draw_client(main_render);
            };
            break;

        case MSG_KEY:
            switch((short)arg2)
            {
                case 0x39:
                    if(spc_down == 0)
                    {
                        spc_down = 1;
                        send_message(win, MSG_LBTNDOWN, 0, 0);
                    }
                    break;

                case 0xB9:
                    spc_down = 0;
                    break;

                case 0x1C:
                    if(ent_down == 0)
                    {
                        int screensize;
                        if(win->win_state == NORMAL)
                        {
                            win->saved = win->rc;
                            win->saved_state = win->win_state;

                            screensize = GetScreenSize();
                            __asm__ __volatile__(
                            "int $0x40"
                            ::"a"(67), "b"(0), "c"(0),
                            "d"((screensize >> 16)-1),"S"((screensize & 0xFFFF)-1) );
                            win->win_state = FULLSCREEN;
                            window_update_layout(win);
                        }
                        else if(win->win_state == FULLSCREEN)
                        {
                            __asm__ __volatile__(
                            "int $0x40"
                            ::"a"(67), "b"(win->saved.l), "c"(win->saved.t),
                            "d"(win->saved.r-win->saved.l-1),"S"(win->saved.b-win->saved.t-1));
                            win->win_state = win->saved_state;
                            window_update_layout(win);
//                            if(win->saved_state == MAXIMIZED)
//                            {
//                                blit_caption(&win->caption);
//                                blit_panel(&win->panel);
//                            }
                        }
                        ent_down = 1;
                    };
                    break;

                case 0x9C:
                    ent_down = 0;
                    break;
            };

        case MSG_DRAW_CLIENT:
            if(main_render)
            {
                render_draw_client(main_render);
            };
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
                    level =  peak;

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

#define VERSION_A     1

void render_time(render_t *render)
{
    progress_t *prg = main_render->win->panel.prg;
    level_t    *lvl = main_render->win->panel.lvl;
    vst_t      *vst = main_render->vst;
    double      ctime;            /*    milliseconds    */
    double      fdelay;           /*    milliseconds    */

//again:

    if(player_state == CLOSED)
    {
        render->win->win_command = WIN_CLOSED;
        return;
    }
    else if((player_state == PAUSE) || (player_state == REWIND))
    {
        delay(1);
        return;
    }
    else if (decoder_state == STOP && frames_count  == 0 &&
              player_state  != STOP)
    {
        player_stop();
    }
    else if(player_state != PLAY)
    {
        delay(1);
        return;
    };


#ifdef VERSION_A
    if(vst->vframe[vst->vfx].ready == 1 )
    {
        int sys_time;

        ctime = get_master_clock();
        fdelay = (vst->vframe[vst->vfx].pts - ctime);

//        printf("pts %f time %f delay %f\n",
//                frames[vfx].pts, ctime, fdelay);

        if(fdelay > 15.0)
        {
            delay((int)fdelay/10);
        //    return;
        };
#if 0
        ctime = get_master_clock();
        fdelay = (vst->vframe[vst->vfx].pts - ctime);

//        while(fdelay > 0)
//        {
//            yield();
//            ctime = get_master_clock();
//            fdelay = (frames[vfx].pts - ctime);
//        }

//        sys_time = get_tick_count();

//        if(fdelay < 0)
//            printf("systime %d pts %f time %f delay %f\n",
//                    sys_time*10, frames[vfx].pts, ctime, fdelay);

        printf("pts %f time %f delay %f\n",
                vst->vframe[vst->vfx].pts, ctime, fdelay);
        printf("video cache %d audio cache %d\n", q_video.size/1024, q_audio.size/1024);
#endif

        main_render->draw(main_render, &vst->vframe[vst->vfx].picture);
        if(main_render->win->win_state != FULLSCREEN)
        {
            prg->current = vst->vframe[vst->vfx].pts*1000;
//        printf("current %f\n", prg->current);
            lvl->current = vst->vfx & 1 ? sound_level_1 : sound_level_0;

            send_message(&prg->ctrl, PRG_PROGRESS, 0, 0);

            if(main_render->win->panel.layout)
                send_message(&lvl->ctrl, MSG_PAINT, 0, 0);
        }

        frames_count--;
        vst->vframe[vst->vfx].ready = 0;
        vst->vfx = (vst->vfx + 1) & 3;
    }
    else delay(1);

#else

    if(vst->vframe[vfx].ready == 1 )
    {
        ctime = get_master_clock();
        fdelay = (vst->vrame[vst->vfx].pts - ctime);

//            printf("pts %f time %f delay %f\n",
//                    frames[vfx].pts, ctime, fdelay);

        if(fdelay < 0.0 )
        {
            int  next_vfx;
            fdelay = 0;
            next_vfx = (vst->vfx+1) & 3;
            if( vst->vrame[next_vfx].ready == 1 )
            {
                if(vst->vrame[next_vfx].pts <= ctime)
                {
                    vst->vrame[vst->vfx].ready = 0;                  // skip this frame
                    vst->vfx = (vst->vfx + 1) & 3;
                }
                else
                {
                    if( (vst->vrame[next_vfx].pts - ctime) <
                        ( ctime - frames[vst->vfx].pts) )
                    {
                        vst->vrame[vst->vfx].ready = 0;                  // skip this frame
                        vst->vfx = (vst->vfx + 1) & 3;
                        fdelay = (vst->vrame[next_vfx].pts - ctime);
                    }
                }
            };
        };

        if(fdelay > 10.0)
        {
           int val = fdelay;
           printf("pts %f time %f delay %d\n",
                   vst->vrame[vst->vfx].pts, ctime, val);
           delay(val/10);
        };

        ctime = get_master_clock();
        fdelay = (vst->vrame[vst->vfx].pts - ctime);

        printf("pts %f time %f delay %f\n",
                vst->vrame[vst->vfx].pts, ctime, fdelay);

        main_render->draw(main_render, &vst->vrame[vfx].picture);
        main_render->win->panel.prg->current = vst->vrame[vfx].pts;
//        send_message(&render->win->panel.prg->ctrl, MSG_PAINT, 0, 0);
        vst->vrame[vst->vfx].ready = 0;
        vst->vfx = (vst->vfx + 1) & 3;
    }
    else yield();
#endif

}


extern char *movie_file;

int video_thread(void *param)
{
    vst_t *vst = param;
    window_t  *MainWindow;

    init_winlib();

    MainWindow = create_window(movie_file,0,
                               10,10,width,height+CAPTION_HEIGHT+PANEL_HEIGHT,MainWindowProc);

    MainWindow->panel.prg->max = stream_duration;

    show_window(MainWindow, NORMAL);

    main_render = create_render(vst, MainWindow, HW_TEX_BLIT|HW_BIT_BLIT);
    if( main_render == NULL)
    {
        printf("Cannot create render\n\r");
        return 0;
    };

    __sync_or_and_fetch(&threads_running,VIDEO_THREAD);

    render_draw_client(main_render);
    player_state = PLAY;

    run_render(MainWindow, main_render);

    __sync_and_and_fetch(&threads_running,~VIDEO_THREAD);

    destroy_render(main_render);
    fini_winlib();
    player_state = CLOSED;
    return 0;
};


void draw_hw_picture(render_t *render, AVPicture *picture);
void draw_sw_picture(render_t *render, AVPicture *picture);

render_t *create_render(vst_t *vst, window_t *win, uint32_t flags)
{
    render_t *render;

    uint32_t right, bottom, draw_w, draw_h;
    uint32_t s, sw, sh;
    uint8_t  state;

//    __asm__ __volatile__("int3");

    render = (render_t*)malloc(sizeof(render_t));
    memset(render, 0, sizeof(render_t));

    render->vst = vst;
    render->win = win;

    render->ctx_width  = vst->vCtx->width;
    render->ctx_height = vst->vCtx->height;
    render->ctx_format = vst->vCtx->pix_fmt;

    render->caps = pxInit(1);

    right  = win->w;
    bottom = win->h-CAPTION_HEIGHT-PANEL_HEIGHT;

    printf("window width %d height %d\n",
                    right, bottom);

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

    pxCreateClient(0, CAPTION_HEIGHT, right, bottom);

    if(render->caps==0)
    {
        render->bitmap[0] = pxCreateBitmap(draw_w, draw_h);
        if(render->bitmap[0] == NULL)
        {
            free(render);
            return NULL;
        }
        render->draw = draw_sw_picture;
    }
    else
    {
        int width, height;
        int i;

        if(render->caps & HW_TEX_BLIT)
        {
            width  = render->ctx_width;
            height = render->ctx_height;
        }
        else
        {
            width  = draw_w;
            height = draw_h;;
        }

        for( i=0; i < 2; i++)
        {
            render->bitmap[i] = pxCreateBitmap(width, height);
            if( render->bitmap[i] == NULL )
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
           render->caps & HW_TEX_BLIT ? "hw_tex_blit":
           render->caps & HW_BIT_BLIT ? "hw_bit_blit":"software",
           render->ctx_width, render->ctx_height,
           draw_w, draw_h);

    return render;
};

void destroy_render(render_t *render)
{

    pxDestroyBitmap(render->bitmap[0]);

    if(render->caps & (HW_BIT_BLIT|HW_TEX_BLIT))          /* hw blitter */
        pxDestroyBitmap(render->bitmap[1]);

    pxFini();
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
    bottom = win->h;

    if(win->win_state != FULLSCREEN)
        bottom-= CAPTION_HEIGHT+PANEL_HEIGHT;

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

//    printf("%s r: %d b: %d\n", __FUNCTION__, right, bottom);

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

    render->win_width  = right;
    render->win_height = bottom;
    render_set_size(render, new_w, new_h);

    if(render->caps & HW_TEX_BLIT)          /*  hw scaler  */
    {
        if(render->win->win_state == FULLSCREEN)
            pxResizeClient(render->rcvideo.l, render->rcvideo.t, new_w, new_h);
        else
            pxResizeClient(render->rcvideo.l, render->rcvideo.t+CAPTION_HEIGHT, new_w, new_h);

        return;
    };

    pxResizeBitmap(render->bitmap[0], new_w, new_h);

    if(render->caps & HW_BIT_BLIT)          /* hw blitter */
        pxResizeBitmap(render->bitmap[1], new_w, new_h);

    return;
};

void draw_hw_picture(render_t *render, AVPicture *picture)
{
    int       dst_width;
    int       dst_height;
    bitmap_t *bitmap;
    uint8_t  *bitmap_data;
    uint32_t  bitmap_pitch;
    uint8_t  *data[4];
    int       linesize[4];
    enum AVPixelFormat format;

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

	format = render->vst->hwdec == 0 ? render->ctx_format : AV_PIX_FMT_BGRA
    cvt_ctx = sws_getCachedContext(cvt_ctx,
              render->ctx_width, render->ctx_height, format,
              dst_width, dst_height, AV_PIX_FMT_BGRA,
              SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if(cvt_ctx == NULL)
    {
        printf("Cannot initialize the conversion context!\n");
        return ;
    };

    bitmap = render->bitmap[render->target];

    bitmap_data = pxLockBitmap(bitmap, &bitmap_pitch);
    if( bitmap_data == NULL)
    {
        printf("Cannot lock bitmap!\n");
        return ;
    }

//    printf("sws_getCachedContext\n");
    data[0] = bitmap_data;
    data[1] = bitmap_data+1;
    data[2] = bitmap_data+2;
    data[3] = bitmap_data+3;

    linesize[0] = bitmap_pitch;
    linesize[1] = bitmap_pitch;
    linesize[2] = bitmap_pitch;
    linesize[3] = bitmap_pitch;

    sws_scale(cvt_ctx, (const uint8_t* const *)picture->data,
              picture->linesize, 0, render->ctx_height, data, linesize);
//    printf("sws_scale\n");

    mutex_lock(&render->vst->gpu_lock);

    if(render->caps & HW_TEX_BLIT)
    {

        if(render->win->win_state == FULLSCREEN)
            pxBlitBitmap(bitmap,render->rcvideo.l,render->rcvideo.t,
                 render->rcvideo.r, render->rcvideo.b,0,0);
        else
            pxBlitBitmap(bitmap, render->rcvideo.l,
                    CAPTION_HEIGHT+render->rcvideo.t,
                    render->rcvideo.r, render->rcvideo.b,0,0);
    }
    else
    {
        if(render->win->win_state == FULLSCREEN)
            pxBlitBitmap(bitmap,render->rcvideo.l,render->rcvideo.t,
                    render->rcvideo.r, render->rcvideo.b, 0,0);
        else
            pxBlitBitmap(bitmap, render->rcvideo.l,
                    CAPTION_HEIGHT+render->rcvideo.t,
                    render->rcvideo.r, render->rcvideo.b, 0, 0);
    };
    mutex_unlock(&render->vst->gpu_lock);

    render->last_bitmap = bitmap;
    render->target++;
    render->target&= 1;
}

void draw_sw_picture(render_t *render, AVPicture *picture)
{
    uint8_t  *bitmap_data;
    uint32_t  bitmap_pitch;
    uint8_t  *data[4];
    int      linesize[4];

    if(render->win->win_state == MINIMIZED ||
       render->win->win_state == ROLLED)
        return;

    cvt_ctx = sws_getCachedContext(cvt_ctx,
              render->ctx_width, render->ctx_height,
              render->ctx_format,
              render->rcvideo.r, render->rcvideo.b,
              AV_PIX_FMT_BGRA, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if(cvt_ctx == NULL)
    {
        printf("Cannot initialize the conversion context!\n");
        return ;
    }

    bitmap_data = pxLockBitmap(render->bitmap[0],&bitmap_pitch);

    data[0] = bitmap_data;
    data[1] = bitmap_data+1;
    data[2] = bitmap_data+2;
    data[3] = bitmap_data+3;

    linesize[0] = bitmap_pitch;
    linesize[1] = bitmap_pitch;
    linesize[2] = bitmap_pitch;
    linesize[3] = bitmap_pitch;

     sws_scale(cvt_ctx, (const uint8_t* const *)picture->data,
              picture->linesize, 0, render->ctx_height, data, linesize);

    if(render->win->win_state == FULLSCREEN)
        pxBlitBitmap(render->bitmap[0],render->rcvideo.l,render->rcvideo.t,
                 render->rcvideo.r, render->rcvideo.b,0,0);
    else
        pxBlitBitmap(render->bitmap[0], render->rcvideo.l,
                 CAPTION_HEIGHT+render->rcvideo.t,
                 render->rcvideo.r, render->rcvideo.b,0,0);

    render->last_bitmap = render->bitmap[0];
}

void render_draw_client(render_t *render)
{
    vst_t *vst = render->vst;
    int y;

    if(render->win_state == MINIMIZED ||
       render->win_state == ROLLED )
        return;
    if(render->win_state == FULLSCREEN)
        y = 0;
    else
        y = CAPTION_HEIGHT;

    if(player_state == PAUSE)
    {
         if(vst->vframe[vst->vfx].ready == 1 )
            main_render->draw(main_render, &vst->vframe[vst->vfx].picture);
         else
            draw_bar(0, y, render->win_width,
                 render->rcvideo.b, 0);
    }
    else if( player_state == STOP )
    {
        draw_bar(0,y, render->win_width,
                 render->rcvideo.b, 0);
    };

    if(render->layout & HAS_TOP)
        draw_bar(0, y, render->win_width,
                 render->rctop.b, 0);
    if(render->layout & HAS_LEFT)
        draw_bar(0, render->rcvideo.t+y, render->rcleft.r,
                 render->rcvideo.b, 0);
    if(render->layout & HAS_RIGHT)
        draw_bar(render->rcright.l, render->rcvideo.t+y,
                 render->rcright.r, render->rcvideo.b, 0);
    if(render->layout & HAS_BOTTOM)
        draw_bar(0, render->rcbottom.t+y,
                 render->win_width, render->rcbottom.b, 0);
}


