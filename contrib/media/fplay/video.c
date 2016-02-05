
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

struct SwsContext *cvt_ctx = NULL;

render_t   *main_render;

void get_client_rect(rect_t *rc);
void run_render(window_t *win, void *render);
void window_update_layout(window_t *win);
int  fini_winlib();

void flush_video(vst_t *vst)
{
    vframe_t *vframe, *tmp;

    mutex_lock(&vst->output_lock);
    mutex_lock(&vst->input_lock);

    list_for_each_entry_safe(vframe, tmp, &vst->output_list, list)
        list_move_tail(&vframe->list, &vst->input_list);

    list_for_each_entry(vframe, &vst->input_list, list)
    {
        vframe->pts   = 0;
        vframe->ready = 0;
    }
    vst->frames_count = 0;

    mutex_unlock(&vst->input_lock);
    mutex_unlock(&vst->output_lock);
};



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

void render_time(render_t *render)
{
    progress_t *prg = main_render->win->panel.prg;
    level_t    *lvl = main_render->win->panel.lvl;
    vst_t      *vst = main_render->vst;
    double      ctime;            /*    milliseconds    */
    double      fdelay;           /*    milliseconds    */

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
    else if (decoder_state == STOP && vst->frames_count  == 0 &&
              player_state  != STOP)
    {
        player_stop();
    }
    else if(player_state != PLAY)
    {
        delay(1);
        return;
    };

    mutex_lock(&vst->output_lock);
    if(list_empty(&vst->output_list))
    {
        mutex_unlock(&vst->output_lock);
        delay(1);
    }
    else
    {
        vframe_t *vframe;
        int sys_time;

        vframe = list_first_entry(&vst->output_list, vframe_t, list);
        list_del(&vframe->list);
        vst->frames_count--;
        mutex_unlock(&vst->output_lock);

        ctime = get_master_clock();
        fdelay = (vframe->pts - ctime);

        if(fdelay > 15.0)
        {
            delay((int)fdelay/10);
        };

//        printf("output index: %d pts: %f pkt_pts %f pkt_dts %f\n",
//               vframe->index,vframe->pts,vframe->pkt_pts,vframe->pkt_dts);

        main_render->draw(main_render, vframe);

        if(main_render->win->win_state != FULLSCREEN)
        {
            prg->current = vframe->pts * 1000;
            lvl->current = vframe->index & 1 ? sound_level_1 : sound_level_0;

            send_message(&prg->ctrl, PRG_PROGRESS, 0, 0);

            if(main_render->win->panel.layout)
                send_message(&lvl->ctrl, MSG_PAINT, 0, 0);
        }

        vframe->ready = 0;

        mutex_lock(&vst->input_lock);
        list_add_tail(&vframe->list, &vst->input_list);
        mutex_unlock(&vst->input_lock);
    }
}

int video_thread(void *param)
{
    vst_t *vst = param;
    window_t  *MainWindow;

    init_winlib();

    MainWindow = create_window(vst->input_name,0,
                               10,10,vst->vCtx->width,vst->vCtx->height+CAPTION_HEIGHT+PANEL_HEIGHT,MainWindowProc);

    MainWindow->panel.prg->max = stream_duration;

    show_window(MainWindow, NORMAL);

    main_render = create_render(vst, MainWindow, HW_TEX_BLIT|HW_BIT_BLIT);
    if( main_render == NULL)
    {
        mutex_unlock(&vst->decoder_lock);
        printf("Cannot create render\n\r");
        return 0;
    };

    __sync_or_and_fetch(&threads_running,VIDEO_THREAD);

    render_draw_client(main_render);
    player_state = PLAY;

    mutex_unlock(&vst->decoder_lock);

    run_render(MainWindow, main_render);

    __sync_and_and_fetch(&threads_running,~VIDEO_THREAD);

    {
        vframe_t *vframe, *tmp;
        flush_video(vst);

        list_for_each_entry_safe(vframe, tmp, &vst->output_list, list)
        {
            list_del(&vframe->list);
            if(vframe->planar != NULL)
                pxDestroyPlanar(vframe->planar);
        }
    }

    destroy_render(main_render);
    fini_winlib();
    player_state = CLOSED;
    return 0;
};


void draw_hw_picture(render_t *render, vframe_t *vframe);
void draw_sw_picture(render_t *render, vframe_t *vframe);

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

static void render_hw_planar(render_t *render, vframe_t *vframe)
{
    vst_t *vst = render->vst;
    planar_t *planar = vframe->planar;

    if(vframe->is_hw_pic != 0 && vframe->format != AV_PIX_FMT_NONE)
    {
        mutex_lock(&render->vst->gpu_lock);

        pxBlitPlanar(planar, render->rcvideo.l,
                    CAPTION_HEIGHT+render->rcvideo.t,
                    render->rcvideo.r, render->rcvideo.b,0,0);
        mutex_unlock(&render->vst->gpu_lock);
    }
};

void draw_hw_picture(render_t *render, vframe_t *vframe)
{
    AVPicture *picture;
    int       dst_width;
    int       dst_height;
    bitmap_t *bitmap;
    uint8_t  *bitmap_data;
    uint32_t  bitmap_pitch;
    uint8_t  *data[4];
    int       linesize[4];
    enum AVPixelFormat format;

    vst_t *vst = render->vst;

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

    if(vframe->is_hw_pic)
    {
        render_hw_planar(render, vframe);
        return;
    };

    picture = &vframe->picture;

    format = render->ctx_format;
    cvt_ctx = sws_getCachedContext(cvt_ctx, render->ctx_width, render->ctx_height, format,
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

void draw_sw_picture(render_t *render, vframe_t *vframe)
{
    AVPicture *picture;
    uint8_t  *bitmap_data;
    uint32_t  bitmap_pitch;
    uint8_t  *data[4];
    int      linesize[4];

    if(render->win->win_state == MINIMIZED ||
       render->win->win_state == ROLLED)
        return;

    picture = &vframe->picture;

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
//         if(vst->vframe[vst->vfx].ready == 1 )
//            main_render->draw(main_render, &vst->vframe[vst->vfx].picture);
//         else
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


