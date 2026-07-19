
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
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

/* Prefer the hardware blitter, but trust it only after it proves itself: at
 * startup video_thread runs a present self-test (draw through the GL path,
 * read the pixels back through the kernel, compare). If the test fails - as
 * on Intel GMA 3150 under the KMS driver, where EGL/Mesa initialize fine but
 * the GL present never reaches the visible framebuffer - this flag is cleared
 * and the render is rebuilt on the software kernel blitter, which always
 * presents correctly under both KMS and the legacy display driver. */
int         g_want_hw = 1;       /* self-test may clear this at startup */

int         g_show_fps = 0;      /* Tab toggles the fps overlay */
/* Default OFF: fn18.14 spins in the kernel polling VGA port 0x3DA, and if the
 * vblank bit ever stops toggling (non-VGA state, driver quirk) the wait never
 * returns - the video thread runs the window event loop, so the WHOLE window
 * freezes (seen on real hardware right after a seek, when the first post-seek
 * frame hits the blit). V enables it for setups where 0x3DA is reliable. */
int         g_vsync    = 0;      /* V toggles: wait for retrace before sw blit */

extern volatile int g_skip_level;   /* decoder.c: adaptive frame-skip level */

/* KolibriOS syscall 18.14: block until the start of the monitor's vertical
 * retrace (polls VGA port 0x3DA). Starting the frame blit here lets it land on
 * screen ahead of the scanning ray, removing the tearing seen on the software
 * blitter, which writes straight into the visible framebuffer. */
static void wait_retrace(void)
{
    __asm__ __volatile__("int $0x40"::"a"(18), "b"(14));
}

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

/* Toggle between NORMAL and FULLSCREEN. Used by the Enter key and by the
 * middle mouse button. */
static void toggle_fullscreen(window_t *win)
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

        /* Leaving fullscreen: window_update_layout re-renders the caption and
         * the panel into their pixmaps and the child controls blit themselves,
         * but nothing blits the panel BACKGROUNDS to the screen - fn67 does
         * not deliver a redraw event here, so without these the controls
         * float on black until the next external window refresh. */
        blit_caption(&win->caption);
        blit_panel(&win->panel);
    }
}

/* Seek by delta microseconds relative to the current position. Mirrors the
 * progress-bar click path (ID_PROGRESS): a negative rewind_pos tells the
 * decoder to seek backwards. Bound to the Left/Right arrow keys (+-5 s). */
static void seek_relative(window_t *win, int64_t delta)
{
    progress_t *prg = win->panel.prg;
    int64_t     target;

    if(player_state == REWIND)
        return;
    if(player_state != PLAY && player_state != PAUSE)
        return;
    if(prg->max <= 1.0f)             /* unknown duration - no reliable seeks */
        return;

    target = (int64_t)prg->current + delta;
    if(target < 0)
        target = 0;
    if(prg->max > 0.0f && target > (int64_t)prg->max)
        target = (int64_t)prg->max;

    rewind_pos    = target;
    player_state  = REWIND;
    decoder_state = REWIND;
    sound_state   = PLAY_2_STOP;

    if((float)rewind_pos < prg->current)
    {
        prg->current = rewind_pos;
        rewind_pos = -rewind_pos;
    }
    else
        prg->current = rewind_pos;

    win->panel.play_btn->img_default = res_pause_btn;
    win->panel.play_btn->img_hilite  = res_pause_btn;
    win->panel.play_btn->img_pressed = res_pause_btn_pressed;
    send_message(&prg->ctrl, MSG_PAINT, 0, 0);
}

extern char g_next_file[1024];       /* fplay.c: exec'd after full teardown */
char *get_moviefile();

static int g_saved_vol = -1;         /* slider pos saved while muted, -1 = not muted */

/* "VOL: XX%" overlay, shown in the video area for 1.5 s after every volume
 * change (drawn by draw_fps_into alongside the Tab info block). */
static int      g_vol_pct   = 0;
static uint32_t g_vol_until = 0;     /* tick count the overlay expires at */

static int vol_overlay_active(void)
{
    /* signed diff survives tick wraparound */
    return g_vol_until != 0 &&
           (int32_t)(g_vol_until - get_tick_count()) > 0;
}

/* Set the volume slider to an absolute position (0..96) and apply it. */
static void set_volume_pos(window_t *win, int pos)
{
    slider_t *sld = win->panel.sld;
    int       level;

    if(pos < 0)  pos = 0;
    if(pos > 96) pos = 96;

    /* arm the overlay even when the position did not move (Up at max, etc.) -
     * the user asked for volume feedback, show the current value */
    g_vol_pct   = (pos*100 + 48)/96;
    g_vol_until = get_tick_count() + 150;          /* 1.5 s */

    if(pos == sld->pos)
        return;

    sld->pos = pos;
    level = sld->min + sld->pos * (sld->max - sld->min)/96;

    set_audio_volume(level, level);
    win->panel.lvl->vol = level;
    send_message(&sld->ctrl, MSG_PAINT, 0, 0);
}

/* Nudge the volume by delta steps (mouse wheel, Up/Down keys). A manual change
 * cancels a pending mute-restore. */
static void change_volume(window_t *win, int delta)
{
    g_saved_vol = -1;
    set_volume_pos(win, win->panel.sld->pos + delta);
}

/* M: toggle mute - drop the volume to zero, remember the level, restore it. */
static void toggle_mute(window_t *win)
{
    if(g_saved_vol < 0)
    {
        g_saved_vol = win->panel.sld->pos;
        set_volume_pos(win, 0);
    }
    else
    {
        int v = g_saved_vol;
        g_saved_vol = -1;
        set_volume_pos(win, v);
    }
}

/* 1/2/3: resize the window so the video area is num/den of its native size. */
static void resize_video(window_t *win, int num, int den)
{
    int vw, vh, ww, wh;

    if(main_render == NULL)
        return;

    vw = (int)main_render->ctx_width  * num / den;
    vh = (int)main_render->ctx_height * num / den;
    if(vw < 64) vw = 64;
    if(vh < 48) vh = 48;

    ww = vw;
    wh = vh + CAPTION_HEIGHT + PANEL_HEIGHT;

    win->win_state = NORMAL;         /* leave a maximized/fullscreen state */

    __asm__ __volatile__(
    "int $0x40"
    ::"a"(67), "b"(win->rc.l), "c"(win->rc.t), "d"(ww-1), "S"(wh-1));

    window_update_layout(win);
}

/* O: pick a file in the open dialog, then restart the player with it.
 * The new instance is NOT spawned here: this one still owns the sound
 * stream, the DRM/vaapi context and the display pipeline, and a second
 * instance initializing those against a live sibling dies before it even
 * creates its window. Instead save the choice and shut down normally -
 * main() execs the new instance as its very last step, when everything is
 * released, exactly like a fresh launch from the file manager.
 *
 * get_moviefile() BLOCKS until the child open-dialog process returns, so it
 * must NOT run on the window/video thread (this handler's thread) - that
 * would freeze the picture the whole time the dialog is open. Run it on its
 * own thread; the video keeps playing behind the dialog. g_dialog_running
 * guards against a second O launching a second dialog over the shared static
 * dialog state. */
static volatile int g_dialog_running = 0;

static int open_dialog_thread(void *param)
{
    window_t *win     = param;
    char     *newfile = get_moviefile();

    if(newfile != NULL && newfile[0] != 0)   /* not cancelled */
    {
        strncpy(g_next_file, newfile, sizeof(g_next_file)-1);
        g_next_file[sizeof(g_next_file)-1] = 0;

        /* publish g_next_file BEFORE the close flag the main loop polls, so
         * main() cannot tear down and read a half-written path */
        __asm__ __volatile__("":::"memory");
        win->win_command = WIN_CLOSED;
    }

    g_dialog_running = 0;
    return 0;
}

static void open_file(window_t *win)
{
    if(g_dialog_running)                      /* dialog already open */
        return;

    g_dialog_running = 1;

    if(create_thread(open_dialog_thread, win, 128*1024) < 0)
    {
        /* could not spawn - fall back to the old blocking path so O still
         * works (video freezes meanwhile, as before) */
        g_dialog_running = 0;
        open_dialog_thread(win);
    }
}

int MainWindowProc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    window_t  *win = (window_t*)ctrl;
    static int spc_down = 0;
    static int ent_down = 0;
    static int tab_down = 0;
    static int m_down   = 0;
    static int o_down   = 0;
    static int v_down   = 0;
    static int num_down = 0;
    static int e0_key   = 0;   /* set after a 0xE0 extended-key prefix */

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
        {
            short scan = (short)arg2;
            int   ext  = e0_key;            /* was this scancode E0-prefixed? */
            e0_key = (scan == 0xE0);        /* prefix applies to the NEXT key */

            switch(scan)
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
                        toggle_fullscreen(win);
                        ent_down = 1;
                    };
                    break;

                case 0x9C:
                    ent_down = 0;
                    break;

                case 0x4B:              /* Left arrow - rewind 5 s     */
                    if(ext)
                        seek_relative(win, -5000000LL);
                    break;

                case 0x4D:              /* Right arrow - forward 5 s   */
                    if(ext)
                        seek_relative(win, 5000000LL);
                    break;

                case 0x48:              /* Up arrow - louder           */
                    if(ext)
                        change_volume(win, 6);
                    break;

                case 0x50:              /* Down arrow - quieter        */
                    if(ext)
                        change_volume(win, -6);
                    break;

                case 0x32:              /* M - toggle mute             */
                    if(m_down == 0)
                    {
                        m_down = 1;
                        toggle_mute(win);
                    }
                    break;

                case 0xB2:              /* M released                  */
                    m_down = 0;
                    break;

                case 0x18:              /* O - open file               */
                    if(o_down == 0)
                    {
                        o_down = 1;
                        open_file(win);
                    }
                    break;

                case 0x98:              /* O released                  */
                    o_down = 0;
                    break;

                case 0x02:              /* 1 - 50% of native size      */
                    if(num_down == 0)
                    {
                        num_down = 1;
                        resize_video(win, 1, 2);
                    }
                    break;

                case 0x03:              /* 2 - 100% (native) size      */
                    if(num_down == 0)
                    {
                        num_down = 1;
                        resize_video(win, 1, 1);
                    }
                    break;

                case 0x04:              /* 3 - 200% of native size     */
                    if(num_down == 0)
                    {
                        num_down = 1;
                        resize_video(win, 2, 1);
                    }
                    break;

                case 0x82:              /* 1 / 2 / 3 released          */
                case 0x83:
                case 0x84:
                    num_down = 0;
                    break;

                case 0x2F:              /* V - toggle vsync (anti-tearing) */
                    if(v_down == 0)
                    {
                        v_down = 1;
                        g_vsync = !g_vsync;
                    }
                    break;

                case 0xAF:              /* V released                  */
                    v_down = 0;
                    break;

                case 0x0F:              /* Tab - cycle the info overlay:
                                         * 1 = full info, 2 = bare fps, 0 = off */
                    if(tab_down == 0)
                    {
                        tab_down = 1;
                        g_show_fps = (g_show_fps + 1) % 3;
                    }
                    break;

                case 0x8F:              /* Tab released                */
                    tab_down = 0;
                    break;
            };
        }

        case MSG_DRAW_CLIENT:
            if(main_render)
            {
                render_draw_client(main_render);
            };
            break;

        case MSG_MBTNDOWN:
            /* Middle mouse button toggles fullscreen, same as the Enter key. */
            toggle_fullscreen(win);
            break;

        case MSG_WHEELUP:           /* wheel up   -> louder  */
            change_volume(win, 6);
            break;

        case MSG_WHEELDOWN:         /* wheel down -> quieter */
            change_volume(win, -6);
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
                    /* max<=1 means the duration is unknown: there is no
                     * position to seek to, so ignore progress-bar clicks
                     * (matches the arrow-key seek guard). */
                    if(player_state != REWIND && win->panel.prg->max > 1.0f)
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

static int g_fps_value = 0;      /* frames displayed in the last second */

/* Count displayed frames; refresh g_fps_value once per second. */
static void fps_tick(void)
{
    static int      frames = 0;
    static uint32_t last   = 0;
    uint32_t        now    = get_tick_count();   /* 1/100 s ticks */

    frames++;
    if(last == 0)
        last = now;
    if(now - last >= 100)                        /* one second elapsed */
    {
        g_fps_value = frames * 100 / (now - last);
        frames = 0;
        last   = now;
    }
}

/* Bake the info overlay straight into the 32bpp BGRA frame buffer (data/pitch),
 * so it is part of the picture and survives the blit. fn4 can render into a
 * user buffer (flag C=1, bit 27) whose layout is [dword w][dword h][pixels],
 * with the pixels inline right after the 8-byte header - pixlib's bitmap is not
 * laid out that way, so we render into a small scratch buffer with a
 * transparent (alpha 0) background and colour-key it onto the frame: the kernel
 * marks glyph pixels with alpha 0xFF, everything else stays 0.
 *
 * Tab cycles g_show_fps: 1 = full info block (fps, codecs, bitrate, source and
 * output resolution), 2 = bare fps number, 0 = hidden. */
#define FPS_OVL_W    256
#define FPS_OVL_H    160
#define FPS_LINE_H   20

/* one line of text into the scratch buffer: fn4, ecx = 0x89.RRGGBB ->
 * A=1 asciiz, FF=0 (6x9), C=1 user buffer, SSS=1 (x2 size); green */
static void ovl_line(uint32_t *ovl, const char *s, int y)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(4), "b"((2 << 16) | y), "c"(0x8900FF00),
      "d"(s), "S"(strlen(s)), "D"(ovl)
    :"memory");
}

static void draw_fps_into(render_t *render, uint8_t *data, uint32_t pitch,
                          int dst_w, int dst_h)
{
    static uint32_t ovl[2 + FPS_OVL_W*FPS_OVL_H];   /* header + pixels */
    uint32_t *px = ovl + 2;
    char      buf[64];
    int       i, x, y, w, h;

    ovl[0] = FPS_OVL_W;
    ovl[1] = FPS_OVL_H;
    for(i = 0; i < FPS_OVL_W*FPS_OVL_H; i++)
        px[i] = 0;                                  /* transparent background */

    int ty = 2;

    if(g_show_fps == 1)                             /* full info block */
    {
        vst_t *vst = render->vst;

        sprintf(buf, "FPS: %d", g_fps_value);
        ovl_line(ovl, buf, ty); ty += FPS_LINE_H;

        sprintf(buf, "video: %s", vst->decoder->name);
        ovl_line(ovl, buf, ty); ty += FPS_LINE_H;

        sprintf(buf, "audio: %s", vst->aCodec ? vst->aCodec->name : "none");
        ovl_line(ovl, buf, ty); ty += FPS_LINE_H;

        if(vst->fCtx->bit_rate > 0)                 /* only when known */
        {
            sprintf(buf, "bitrate: %d kbps", (int)(vst->fCtx->bit_rate/1000));
            ovl_line(ovl, buf, ty); ty += FPS_LINE_H;
        }

        sprintf(buf, "src: %dx%d",
                (int)render->ctx_width, (int)render->ctx_height);
        ovl_line(ovl, buf, ty); ty += FPS_LINE_H;

        sprintf(buf, "out: %dx%d", render->rcvideo.r, render->rcvideo.b);
        ovl_line(ovl, buf, ty); ty += FPS_LINE_H;
    }
    else if(g_show_fps == 2)                        /* bare fps number */
    {
        sprintf(buf, "%d", g_fps_value);
        ovl_line(ovl, buf, ty); ty += FPS_LINE_H;
    }

    if(vol_overlay_active())                        /* volume feedback */
    {
        sprintf(buf, "VOL: %d%%", g_vol_pct);
        ovl_line(ovl, buf, ty);
    }

    w = FPS_OVL_W < dst_w ? FPS_OVL_W : dst_w;
    h = FPS_OVL_H < dst_h ? FPS_OVL_H : dst_h;

    for(y = 0; y < h; y++)
    {
        uint32_t *d = (uint32_t*)(data + y*pitch);
        uint32_t *s = px + y*FPS_OVL_W;
        for(x = 0; x < w; x++)
            if(s[x] & 0xFF000000)                   /* a glyph pixel */
                d[x] = s[x];
    }
}

/* Refresh the elapsed/total time in the panel, but only when the whole second
 * changes (drawing is not free). Driven from prg->current (microseconds),
 * which advances for both video and audio-only (MP3) playback. */
static void update_time_display(render_t *render)
{
    static int  last_sec = -1;
    progress_t *prg = render->win->panel.prg;
    int         elapsed;

    if(render->win->win_state == FULLSCREEN)
        return;

    elapsed = (int)(prg->current / 1000000.0f);
    if(elapsed == last_sec)
        return;
    last_sec = elapsed;

    draw_panel_time(&render->win->panel, elapsed, (int)(prg->max / 1000000.0f));
}

/* While the volume overlay is up (or has just expired and must be erased),
 * re-blit the last frame so the overlay shows and disappears even when no new
 * frames are being drawn - pause, stopped, MP3 cover art. Throttled to ~10 Hz;
 * during normal playback the regular frame flow composites it instead. Re-uses
 * the same last_frame re-draw as render_adjust_size(). */
static void vol_overlay_refresh(render_t *render)
{
    static uint32_t last  = 0;
    static int      shown = 0;
    int      active = vol_overlay_active();
    uint32_t now;

    if(!active && !shown)
        return;
    if(render->last_frame == NULL)
        return;

    now = get_tick_count();
    if(active && now - last < 10)
        return;

    last  = now;
    shown = active;                /* one extra draw after expiry erases it */
    render->draw(render, render->last_frame);
}

/* Frame-pacing state (render_time). The master clock is the sound stream
 * (GetTimeStamp: time_base + played-time, milliseconds). When it is not
 * usable - no audio track, or the post-seek window before the audio thread
 * re-anchors it with SetTimeBase, or a wedged audio thread that never will -
 * frames are paced against the wall clock instead (fn26.9 ticks, anchored so
 * that the current frame is due immediately), and pacing returns to the
 * audio clock as soon as it agrees with the frame timestamps again. */
static int      g_wall_pace = 0;    /* pacing by wall clock, not sound clock */
static double   g_wall_bias = 0;    /* pts == wall_ms + bias  =>  frame due  */
static double   g_held_pts  = -1.0; /* head frame we are waiting to become due */
static uint32_t g_held_tick = 0;    /* tick the wait on it started           */
static double   g_last_ctime = 0;   /* last sound-clock sample...            */
static uint32_t g_last_ctick = 0;   /* ...and the tick it changed (liveness) */

void render_time(render_t *render)
{
    progress_t *prg;
    level_t    *lvl;
    vst_t      *vst;
    double      ctime;            /*    milliseconds    */
    double      fdelay;           /*    milliseconds    */

    /* Always work with the current global render pointer. */
    render = main_render;

    prg = render->win->panel.prg;
    lvl = render->win->panel.lvl;
    vst = render->vst;

    if(player_state == CLOSED)
    {
        render->win->win_command = WIN_CLOSED;
        return;
    }
    else if((player_state == PAUSE) || (player_state == REWIND))
    {
        /* pacing anchors are stale after a pause or a seek */
        g_wall_pace = 0;
        g_held_pts  = -1.0;
        if(player_state == PAUSE)      /* not during REWIND: frames in flux */
            vol_overlay_refresh(render);
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
        vol_overlay_refresh(render);
        delay(1);
        return;
    };

    mutex_lock(&vst->output_lock);
    if(list_empty(&vst->output_list))
    {
        mutex_unlock(&vst->output_lock);

        /* Cover-art audio (an MP3 whose video stream is a single ATTACHED_PIC
         * frame) produces no further video frames, so the frame branch below
         * that advances the bar from vframe->pts never runs again. Drive it
         * from the audio master clock instead. Do this ONLY for cover art: for
         * real video the master clock is real elapsed time, which for a slow
         * codec that cannot keep up races far past the shown frame AND past the
         * duration (0:56/0:04) - real video must track vframe->pts alone.
         * get_master_clock() is ms; prg->current shares prg->max's unit (us). */
        if(vst->has_sound && player_state == PLAY &&
           main_render->win->win_state != FULLSCREEN &&
           (vst->fCtx->streams[vst->vStream]->disposition
                & AV_DISPOSITION_ATTACHED_PIC))
        {
            double ctime = get_master_clock();
            if(ctime > 0.0)
            {
                prg->current = ctime * 1000;
                if(prg->max > 1.0f && prg->current > prg->max)
                    prg->current = prg->max;
                send_message(&prg->ctrl, PRG_PROGRESS, 0, 0);
                update_time_display(main_render);
            }
        }
        vol_overlay_refresh(render);   /* cover art: no frame flow to draw it */
        delay(1);
    }
    else
    {
        vframe_t *vframe;
        double    pts;
        double    wall_ms;
        uint32_t  now;

        /* PEEK the head frame - it is popped only once it is due. A frame
         * that is early stays in the queue and we return after a 10 ms nap,
         * so the video thread (= the window event loop) is never held for
         * long, the frame is presented exactly on time, and the video can
         * never run ahead of the clock (the post-seek 45-fps desync). Only
         * this thread removes frames, so the head can change under us only
         * to an EARLIER frame (the decoder inserts sorted by pts). */
        vframe = list_first_entry(&vst->output_list, vframe_t, list);
        pts = vframe->pts;
        mutex_unlock(&vst->output_lock);

        now     = get_tick_count();
        wall_ms = now * 10.0;

        if(vst->has_sound == 0)
        {
            /* no audio track = no master clock at all: wall-pace the video
             * from the first frame (before, it free-ran at decode speed) */
            if(g_wall_pace == 0)
            {
                g_wall_pace = 1;
                g_wall_bias = pts - wall_ms;
            }
            fdelay = pts - (wall_ms + g_wall_bias);

            /* a backward pts jump (stream wrap, concatenated segments) would
             * leave every following frame "late" and free-run: re-anchor */
            if(fdelay < -3000.0)
            {
                g_wall_bias = pts - wall_ms;
                fdelay = 0.0;
            }
        }
        else
        {
            int clock_live;

            ctime = get_master_clock();

            /* While the ring is really playing the sound clock interpolates
             * sample-accurately, so consecutive reads always differ; a value
             * that has not moved for 300 ms of wall time = frozen clock
             * (stopped ring, wedged audio thread). */
            if(ctime != g_last_ctime)
            {
                g_last_ctime = ctime;
                g_last_ctick = now;
            }
            clock_live = (now - g_last_ctick) < 30;

            fdelay = pts - ctime;                  /* vs the sound clock */

            if(g_wall_pace)
            {
                if(fdelay > -500.0 && fdelay < 500.0)
                    g_wall_pace = 0;  /* clocks agree again - back to audio */
                else
                {
                    if(clock_live)
                    {
                        /* the sound clock is alive but disagrees: slew our
                         * anchor toward it (~200 ms/s) so the offset drains
                         * instead of being locked in - video that fell far
                         * behind a slow decoder catches up and re-syncs */
                        double target = ctime - wall_ms;

                        if(g_wall_bias > target + 2.0)
                            g_wall_bias -= 2.0;
                        else if(g_wall_bias < target - 2.0)
                            g_wall_bias += 2.0;
                        else
                            g_wall_bias = target;
                    }
                    fdelay = pts - (wall_ms + g_wall_bias);

                    if(fdelay < -3000.0)           /* backward pts jump */
                    {
                        g_wall_bias = pts - wall_ms;
                        fdelay = 0.0;
                    }
                }
            }
            else if(fdelay > 3000.0 || fdelay < -3000.0)
            {
                /* clock nowhere near the frames (post-seek window before the
                 * audio thread re-anchors it, or it never will): wall-pace
                 * from here, this frame due now */
                g_wall_pace = 1;
                g_wall_bias = pts - wall_ms;
                fdelay = 0.0;
            }
        }

        if(fdelay > 15.0)
        {
            if(pts != g_held_pts)
            {
                g_held_pts  = pts;
                g_held_tick = now;
            }
            /* Frozen-clock escape: the same frame refusing to become due for
             * a second WHILE the sound clock is not moving means nobody will
             * ever release it - wall-pace from it. A frame legitimately due
             * far in the future (sub-1-fps timelapse) keeps waiting: its
             * clock IS advancing. */
            if(g_wall_pace == 0 &&
               now - g_held_tick > 100 && now - g_last_ctick > 100)
            {
                g_wall_pace = 1;
                g_wall_bias = pts - wall_ms;
                fdelay = 0.0;
            }
            else
            {
                delay(1);
                return;
            }
        }
        g_held_pts = -1.0;

        /* the frame is due (or late) - now pop it */
        mutex_lock(&vst->output_lock);
        vframe = list_first_entry(&vst->output_list, vframe_t, list);
        list_del(&vframe->list);
        vst->frames_count--;
        mutex_unlock(&vst->output_lock);

        /* Adaptive B-frame skip (applied in decode_video): fall >300 ms behind
         * the audio clock -> let the codec drop non-reference frames to lighten
         * decode and keep pace; within 80 ms again -> full quality. Files the
         * CPU can decode in real time keep fdelay near 0 and never trip it.
         *
         * ONLY when the decode itself is the bottleneck, i.e. packets are
         * piling up in q_video undecoded. When the queue is empty the video is
         * late because the READER cannot deliver (slow USB stick, FAT walk) -
         * skipping then just discards frames that did arrive and HALVES the
         * shown fps (30->16) without helping at all. */
        if(fdelay < -300.0 && vst->q_video.count > 8)
            g_skip_level = 1;
        else if(fdelay > -80.0 || vst->q_video.count <= 2)
            g_skip_level = 0;

//        printf("output index: %d pts: %f pkt_pts %f pkt_dts %f\n",
//               vframe->index,vframe->pts,vframe->pkt_pts,vframe->pkt_dts);

        main_render->draw(main_render, vframe);

        /* Remember this frame so render_adjust_size() can re-scale it into the
         * resized bitmap. Matters for a static cover-art frame that is never
         * redrawn on its own; for live video it is simply the newest frame. */
        main_render->last_frame = vframe;

        fps_tick();                   /* update the displayed-frames counter */

        if(main_render->win->win_state != FULLSCREEN)
        {
            prg->current = vframe->pts * 1000;
            if(prg->max > 1.0f && prg->current > prg->max)
                prg->current = prg->max;
            lvl->current = vframe->index & 1 ? sound_level_1 : sound_level_0;

            send_message(&prg->ctrl, PRG_PROGRESS, 0, 0);
            update_time_display(main_render);

            if(main_render->win->panel.layout)
                send_message(&lvl->ctrl, MSG_PAINT, 0, 0);
        }

        vframe->ready = 0;

        mutex_lock(&vst->input_lock);
        list_add_tail(&vframe->list, &vst->input_list);
        mutex_unlock(&vst->input_lock);
    }
}

/* ------------------------------------------------------------------------- *
 * Runtime self-test of the hardware present path.
 *
 * Whether the GL blitter's output actually reaches the visible screen cannot
 * be known in advance: pxInit() succeeding only proves that EGL/Mesa set
 * themselves up, not that the final GL->scanout present works (e.g. on Intel
 * GMA 3150 under the KMS driver everything initializes, yet nothing becomes
 * visible). So we verify it directly: draw a solid-color test frame through
 * the hardware blitter, then read the pixels back through the kernel
 * (syscall 36 reads the real, composited framebuffer - under KMS the LFB is
 * remapped onto the DRM scanout, so it returns what is truly on screen) and
 * compare. Two phases with two different colors prove the screen actually
 * follows our commands rather than accidentally containing the probe color.
 *
 * Details dictated by how pxgl works:
 *  - hw_blit ignores its coordinate arguments and always stretches the whole
 *    bitmap over the whole client rect at (window_x, window_y+CAPTION_HEIGHT),
 *    so the probe paints the entire client area and samples inside it;
 *  - glFlush() submits the batch asynchronously, so the readback is polled;
 *  - each phase uses a FRESHLY created bitmap: a new GEM object starts in the
 *    CPU domain and is clflushed on first use, while later CPU writes to the
 *    same bitmap are never flushed again - on non-LLC GPUs (Gen3/Gen4) the
 *    GPU could then sample stale data and break the test;
 *  - the kernel's readback is never polluted by the mouse cursor (under KMS
 *    the cursor is a hardware overlay plane, under VESA fn36 substitutes the
 *    saved under-cursor pixels).
 * ------------------------------------------------------------------------- */

#define PROBE_BM_SIZE   32       /* probe bitmap side, stretched over client  */
#define PROBE_BLOCK      8       /* readback sample block side                */
#define PROBE_TOLERANCE 24       /* per-channel color tolerance               */
#define PROBE_TRIES     30       /* x 10 ms = up to 300 ms per phase          */

/* syscall 36: read a screen area; buffer gets 3 bytes per pixel (B,G,R),
 * rows top-down, no padding; x/y are absolute screen coordinates. */
static void read_screen_area(void *buf, int x, int y, int w, int h)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(36), "b"(buf), "c"((w << 16) | h), "d"((x << 16) | y)
    :"memory");
}

static int probe_block_matches(int x, int y, uint32_t color)
{
    uint8_t  buf[PROBE_BLOCK*PROBE_BLOCK*3];
    int      eb =  color        & 0xFF;
    int      eg = (color >>  8) & 0xFF;
    int      er = (color >> 16) & 0xFF;
    int      i, d, bad = 0;

    read_screen_area(buf, x, y, PROBE_BLOCK, PROBE_BLOCK);

    for(i = 0; i < PROBE_BLOCK*PROBE_BLOCK; i++)
    {
        uint8_t *p = buf + i*3;

        d = p[0] - eb; if(d < 0) d = -d;
        if(d > PROBE_TOLERANCE) { bad++; continue; }
        d = p[1] - eg; if(d < 0) d = -d;
        if(d > PROBE_TOLERANCE) { bad++; continue; }
        d = p[2] - er; if(d < 0) d = -d;
        if(d > PROBE_TOLERANCE) bad++;
    }
    /* allow a few outliers just in case */
    return bad <= PROBE_BLOCK*PROBE_BLOCK/16;
}

/* One probe phase: blit a solid-color frame through the current (hardware)
 * blitter and poll two sample blocks inside the client rect until they show
 * that color. The blit is re-issued and the window position re-read on every
 * iteration, so a window that gets moved or briefly obscured mid-probe
 * converges instead of pinning a false failure. Returns 1 when the color
 * reached the screen. */
static int probe_phase(render_t *render, uint32_t color)
{
    bitmap_t *bm;
    uint8_t  *data;
    uint32_t  pitch;
    int       i, j, tries;

    bm = pxCreateBitmap(PROBE_BM_SIZE, PROBE_BM_SIZE);
    if(bm == NULL)
        return 0;

    data = pxLockBitmap(bm, &pitch);
    if(data == NULL)
    {
        pxDestroyBitmap(bm);
        return 0;
    }

    for(i = 0; i < PROBE_BM_SIZE; i++)
    {
        uint32_t *row = (uint32_t*)(data + i*pitch);
        for(j = 0; j < PROBE_BM_SIZE; j++)
            row[j] = 0xFF000000 | color;
    }

    for(tries = 0; tries < PROBE_TRIES; tries++)
    {
        char proc_info[1024];
        int  wx, wy;

        /* dst coordinates are ignored on the hw path: the blit stretches the
         * bitmap over the whole client rect at the window's current position */
        mutex_lock(&render->vst->gpu_lock);
        pxBlitBitmap(bm, 0, CAPTION_HEIGHT,
                     render->win_width, render->win_height, 0, 0);
        mutex_unlock(&render->vst->gpu_lock);

        delay(1);

        /* sample where the blit actually painted: fresh window position from
         * fn9, same source hw_blit itself uses */
        get_proc_info(proc_info);
        wx = *(int*)(proc_info+34);
        wy = *(int*)(proc_info+38);

        if(probe_block_matches(wx + 8, wy + CAPTION_HEIGHT + 8, color) &&
           probe_block_matches(wx + render->win_width/2,
                               wy + CAPTION_HEIGHT + render->win_height/2,
                               color))
        {
            pxDestroyBitmap(bm);
            return 1;
        }
    }

    pxDestroyBitmap(bm);
    return 0;
}

static int hw_present_works(render_t *render)
{
    /* client area too small to place the sample blocks - cannot verify,
     * keep the hardware path rather than degrade it untested */
    if(render->win_width < 32 || render->win_height < 32)
        return 1;

    if(probe_phase(render, 0x2060A0) == 0)
        return 0;

    return probe_phase(render, 0xA03020);
}

int video_thread(void *param)
{
    vst_t *vst = param;
    window_t  *MainWindow;

    init_winlib();

    MainWindow = create_window(vst->input_name,0,
                               10,10,vst->vCtx->width,vst->vCtx->height+CAPTION_HEIGHT+PANEL_HEIGHT,MainWindowProc);

    /* only when known: 0 means the container carries no duration; keep the
     * create_progress() default then, and the bar/time display degrade
     * gracefully instead of dividing by garbage */
    if(stream_duration > 0)
        MainWindow->panel.prg->max = stream_duration;

    show_window(MainWindow, NORMAL);

    main_render = create_render(vst, MainWindow, HW_TEX_BLIT|HW_BIT_BLIT);
    if( main_render == NULL)
    {
        mutex_unlock(&vst->decoder_lock);
        printf("Cannot create render\n\r");
        return 0;
    };

    /* The hardware path initialized - now verify its output really reaches
     * the screen (see the probe block above). If it does not, rebuild the
     * render on the software kernel blitter, which always presents. The
     * decoder is still parked on decoder_lock here, so the GPU is ours. */
    if(main_render->caps != 0)
    {
        if(hw_present_works(main_render))
            printf("HW blitter present check passed\n");
        else
        {
            printf("HW blitter present check FAILED, using software\n");

            /* The software blitter cannot show GPU-decoded (vaapi) frames;
             * they are skipped in draw_sw_picture. With a broken present
             * they were invisible anyway, but say so in the log. */
            if(vst->decoder->is_hw)
                printf("warning: vaapi decoder is active, video will not be visible\n");

            destroy_render(main_render);    /* while the hw driver is active */
            main_render = NULL;
            pxFini();                       /* tear down EGL, reset to sw    */
            g_want_hw = 0;

            main_render = create_render(vst, MainWindow, HW_TEX_BLIT|HW_BIT_BLIT);
            if(main_render == NULL)
            {
                mutex_unlock(&vst->decoder_lock);
                printf("Cannot create render\n\r");
                return 0;
            }
        }
    }

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
    main_render = NULL;
    pxFini();                    /* tear down hardware driver / EGL context */
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

    render->caps = pxInit(g_want_hw);

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

    free(render);
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

    /* pxResizeBitmap only reallocates the target - the pixels are not rescaled.
     * A live video repaints them with the next decoded frame, but an audio file
     * with cover art yields a single static frame that is never redrawn, so it
     * would appear broken until the next seek. Re-scale the retained frame into
     * the freshly sized bitmap. (HW_TEX_BLIT returns above: its bitmap is
     * context-sized and the GPU rescales on blit, so no redraw is needed.) */
    if(render->last_frame != NULL && !render->last_frame->is_hw_pic)
        render->draw(render, render->last_frame);

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

    if(g_show_fps || vol_overlay_active())
        draw_fps_into(render, bitmap_data, bitmap_pitch, dst_width, dst_height);

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

    /* A hardware-decoded frame lives in GPU memory and has no CPU-side
     * picture, so the software blitter cannot present it. This happens when
     * pxInit falls back to software while vaapi decoding is active - skip the
     * frame rather than scale garbage. */
    if(vframe->is_hw_pic)
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

    if(g_show_fps || vol_overlay_active())
        draw_fps_into(render, bitmap_data, bitmap_pitch,
                      render->rcvideo.r, render->rcvideo.b);

    /* sync the framebuffer write to vblank to avoid tearing (see wait_retrace) */
    if(g_vsync)
        wait_retrace();

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

    if(render->last_bitmap != NULL)
    {
        /* An MP3 with embedded album art yields exactly one video frame:
         * render_time() blits the cover once, then output_list stays empty
         * forever and main_render->draw() is never called again. On an OS
         * repaint do_sys_draw() erases the whole client area to black and
         * nothing else repaints rcvideo, so the cover turns into a black
         * "hole". Re-present the last frame here so it survives every repaint
         * in every player state. For hardware-decoded (vaapi is_hw_pic) frames
         * draw_hw_picture() returns before setting last_bitmap, so it stays
         * NULL and this branch is a no-op - behaviour is unchanged there. */
        mutex_lock(&render->vst->gpu_lock);
        pxBlitBitmap(render->last_bitmap, render->rcvideo.l, y + render->rcvideo.t,
                     render->rcvideo.r, render->rcvideo.b, 0, 0);
        mutex_unlock(&render->vst->gpu_lock);
    }
    else if(player_state == PAUSE)
    {
//         if(vst->vframe[vst->vfx].ready == 1 )
//            main_render->draw(main_render, &vst->vframe[vst->vfx].picture);
//         else
            draw_bar(0, render->rcvideo.t + y, render->win_width,
                 render->rcvideo.b, 0);
    }
    else if( player_state == STOP )
    {
        /* start at rcvideo.t so the bar covers the actual video rows; the
         * rows above are painted by the HAS_TOP branch below. Without the
         * offset a letterboxed (screen-clamped) layout left a horizontal
         * strip of the client unpainted. */
        draw_bar(0, render->rcvideo.t + y, render->win_width,
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


