#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "winlib.h"

#define PANEL_CORNER_W       8
#define FRAME_WIDTH          7

#define ID_PLAY             100
#define ID_STOP             101
#define ID_PROGRESS         102
#define ID_VOL_LEVEL        103
#define ID_VOL_CTRL         104

extern uint32_t main_cursor;

extern int res_panel_left[];
extern int res_panel_right[];
extern int res_panel_body[];

extern int res_play_btn[];
extern int res_play_btn_pressed[];

extern int res_pause_btn[];
extern int res_pause_btn_pressed[];

extern int res_stop_btn[];
extern int res_stop_btn_pressed[];

//extern int res_minimize_btn[];
//extern int res_minimize_btn_hl[];
//extern int res_minimize_btn_pressed[];

void update_panel_size(window_t *win);
static void render_time_text(panel_t *panel);

int panel_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2);

int init_panel(window_t *win)
{
    button_t     *btn;
    progress_t   *prg;
    level_t      *lvl;
    slider_t     *sld;

    panel_t *panel = &win->panel;
    ctx_t   *ctx = &panel->ctx;

    link_initialize(&panel->ctrl.link);
    list_initialize(&panel->ctrl.child);

    panel->ctrl.handler = panel_proc;
    panel->ctrl.parent  = (ctrl_t*)win;

    panel->layout = 0;

    ctx->pixmap_data = user_alloc(1920*PANEL_HEIGHT*4);
    ctx->offset_x = 0;
    ctx->offset_y = 0;

    panel->ctrl.ctx = ctx;

    btn = create_button(NULL, ID_PLAY,0,19,32,32,&panel->ctrl);
    panel->play_btn = btn;

    btn->img_default = res_pause_btn;
    btn->img_hilite  = res_pause_btn;
    btn->img_pressed = res_pause_btn_pressed;

    btn = create_button(NULL, ID_STOP,0,19,24,24,&panel->ctrl);
    panel->stop_btn = btn;

    btn->img_default = res_stop_btn;
    btn->img_hilite  = res_stop_btn;
    btn->img_pressed = res_stop_btn_pressed;

    prg = create_progress(NULL,ID_PROGRESS,0,4,0,10,&panel->ctrl);
    panel->prg = prg;

    lvl = create_level(NULL, ID_VOL_LEVEL, 0, 20, 96, 10, &panel->ctrl);
    lvl->vol = -1875;
    panel->lvl = lvl;

    sld = create_slider(NULL, ID_VOL_CTRL, 0, 20, 96+12, 12, &panel->ctrl);
    panel->sld = sld;

//    btn = create_button(NULL, ID_MINIMIZE,0,5,16,18,(ctrl_t*)cpt);
//    cpt->minimize_btn = btn;

//    btn->img_default = res_minimize_btn;
//    btn->img_hilite  = res_minimize_btn_hl;
//    btn->img_pressed = res_minimize_btn_pressed;

    update_panel_size(win);

    return 1;
};


static void panel_update_layout(panel_t *panel)
{
    progress_t *prg = panel->prg;
    level_t    *lvl = panel->lvl;

    if(panel->layout == 0)
    {
        prg->ctrl.rc.l = panel->ctrl.rc.l;
        prg->ctrl.rc.t = panel->ctrl.rc.t+7;
        prg->ctrl.rc.r = panel->ctrl.rc.r;
        prg->ctrl.rc.b = prg->ctrl.rc.t + prg->ctrl.h;
        prg->ctrl.w    = prg->ctrl.rc.r - prg->ctrl.rc.l;

        lvl->ctrl.rc.l = panel->ctrl.rc.l;
        lvl->ctrl.rc.t = panel->ctrl.rc.t+7;
        lvl->ctrl.rc.r = panel->lvl->ctrl.rc.l + panel->lvl->ctrl.w;
        lvl->ctrl.rc.b = panel->lvl->ctrl.rc.t + panel->lvl->ctrl.h;
    }
    else
    {
        lvl->ctrl.rc.l = panel->ctrl.rc.l;
        lvl->ctrl.rc.t = panel->ctrl.rc.t+7;
        lvl->ctrl.rc.r = lvl->ctrl.rc.l + lvl->ctrl.w;
        lvl->ctrl.rc.b = lvl->ctrl.rc.t + lvl->ctrl.h;

        prg->ctrl.rc.l = lvl->ctrl.rc.r;
        prg->ctrl.rc.t = panel->ctrl.rc.t+7;
        prg->ctrl.rc.r = panel->ctrl.rc.r;
        prg->ctrl.rc.b = prg->ctrl.rc.t + prg->ctrl.h;
        prg->ctrl.w    = prg->ctrl.rc.r - prg->ctrl.rc.l;
    };
};

void panel_set_layout(panel_t *panel, int layout)
{
    panel->layout = layout;
    panel->lvl->visible = layout;

    panel_update_layout(panel);

    send_message(&panel->prg->ctrl, MSG_PAINT, 0, 0);

    if(layout)
        send_message(&panel->lvl->ctrl, MSG_PAINT, 0, 0);
};

void update_panel_size(window_t *win)
{
    panel_t  *panel  = &win->panel;

    panel->ctx.pixmap_pitch = win->w*4;

    panel->ctx.offset_x = 0;
    panel->ctx.offset_y = win->h-PANEL_HEIGHT;

    panel->draw.l       = 0;
    panel->draw.t       = win->h-PANEL_HEIGHT;
    panel->draw.r       = win->w;
    panel->draw.b       = win->h;

    panel->ctrl.rc.l    = FRAME_WIDTH;
    panel->ctrl.rc.t    = win->h-PANEL_HEIGHT;
    panel->ctrl.rc.r    = win->w-FRAME_WIDTH;
    panel->ctrl.rc.b    = win->h-FRAME_WIDTH;

    panel->ctrl.w       = win->w;
    panel->ctrl.h       = PANEL_HEIGHT;
    win->client.b       = win->h-PANEL_HEIGHT;

    panel->play_btn->ctrl.rc.l = win->w/2 - 16;
    panel->play_btn->ctrl.rc.t = panel->ctrl.rc.t+19;
    panel->play_btn->ctrl.rc.r = panel->play_btn->ctrl.rc.l + panel->play_btn->ctrl.w;
    panel->play_btn->ctrl.rc.b = panel->play_btn->ctrl.rc.t + panel->play_btn->ctrl.h;

    panel->stop_btn->ctrl.rc.l = win->w/2 - 44;
    panel->stop_btn->ctrl.rc.t = panel->ctrl.rc.t+23;
    panel->stop_btn->ctrl.rc.r = panel->stop_btn->ctrl.rc.l + panel->stop_btn->ctrl.w;
    panel->stop_btn->ctrl.rc.b = panel->stop_btn->ctrl.rc.t + panel->stop_btn->ctrl.h;

    panel->sld->ctrl.rc.l = panel->ctrl.rc.l;
    panel->sld->ctrl.rc.t = panel->ctrl.rc.t+28;
    panel->sld->ctrl.rc.r = panel->sld->ctrl.rc.l + panel->sld->ctrl.w;
    panel->sld->ctrl.rc.b = panel->sld->ctrl.rc.t + panel->sld->ctrl.h;

    panel_update_layout(panel);
};


void draw_panel(panel_t *panel)
{
    ctx_t  *ctx = &panel->ctx;
    int *pixmap, *src;
    int  i, j, w;

    blit_raw(ctx, res_panel_left, 0, 0,
             PANEL_CORNER_W, PANEL_HEIGHT, PANEL_CORNER_W*4);

    w = panel->ctrl.w - (2*PANEL_CORNER_W);
    if( w > 0)
    {
        pixmap = (int*)ctx->pixmap_data;
        pixmap+= PANEL_CORNER_W;
        src = res_panel_body;

        for(i = 0; i < PANEL_HEIGHT; i++)
        {
            for(j = 0; j < w; j++)
                pixmap[j] = src[i];
            pixmap+= ctx->pixmap_pitch/4;
        }
    };

    blit_raw(&panel->ctx, res_panel_right, panel->ctrl.w - PANEL_CORNER_W, 0,
             PANEL_CORNER_W, PANEL_HEIGHT, PANEL_CORNER_W*4);


    ctrl_t *child;
    child  = (ctrl_t*)panel->ctrl.child.next;

    while( &child->link != &panel->ctrl.child)
    {
        send_message(child, MSG_PAINT, 0, 0);
        child = (ctrl_t*)child->link.next;
    };

    /* a full panel repaint just erased the time text in the pixmap; put it
     * back so it does not blink out until the next second ticks over */
    render_time_text(panel);
};

int panel_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    panel_t *panel = (panel_t*)ctrl;
    window_t *win  = get_parent_window(ctrl);

    ctrl_t *child;
    int x, y;

    x = ((pos_t)arg2).x;
    y = ((pos_t)arg2).y;

    switch( msg )
    {
        case 1:
            draw_panel((panel_t*)ctrl);
            break;

        case MSG_MOUSEMOVE:
            child = get_child(ctrl, x, y);
            if( win->child_over )
            {
                if(child == win->child_over)
                    send_message(child, msg, 0, arg2);
                else
                    send_message(win->child_over, MSG_MOUSELEAVE, 0, arg2);
            }
            else if( child )
                send_message(child, MSG_MOUSEENTER, 0, arg2);

            win->child_over = child;
            if( child )
                send_message(child,msg,0,arg2);
            else if(main_cursor != 0)
            {
               set_cursor(0);
               main_cursor = 0;
            }
            break;

        case MSG_WHEELUP:
        case MSG_WHEELDOWN:
            /* let the window handle the wheel (volume) wherever it is over
             * the panel, not only above a particular child control */
            return send_message(get_parent_window(ctrl), msg, arg1, arg2);

        case MSG_COMMAND:
            switch((short)arg1)
            {
                case ID_PLAY:
                case ID_STOP:
                case ID_PROGRESS:
                case ID_VOL_CTRL:
                    win = get_parent_window(ctrl);
                    send_message(win, msg, arg1, arg2);
                    break;

                default:
                    break;
            };

        default:
            child = get_child(ctrl, x, y);
            if(child)
                return send_message(child, msg, 0, arg2);
    }
    return 1;
};

void blit_panel(panel_t *panel)
{
    ctx_t  *ctx = &panel->ctx;

    Blit(ctx->pixmap_data, panel->draw.l, panel->draw.t,
         0, 0, panel->ctrl.w, panel->ctrl.h,
         panel->ctrl.w, panel->ctrl.h, ctx->pixmap_pitch);
};

extern int win_font;
int draw_text_ext(void *pixmap, uint32_t pitch, int face, char *text, rect_t *rc, int color);
int text_width_ext(int face, char *text);

/* elapsed / total time, bottom-right of the panel, in the same white as the
 * window-caption text. Rendered straight into the panel pixmap (like the
 * caption text) so it blits cleanly with no per-digit artefacts; the
 * panel-body gradient is restored under it first so old text is erased. */
#define TIME_COLOR    0xFFFFFF   /* white - same as the caption title text */
#define TIME_MAXW     120
#define TIME_Y0       25
#define TIME_Y1       44
#define TIME_BASE     40

static void fmt_time(char *buf, int sec)
{
    int h, m, s;

    if(sec < 0)                      /* clamp BEFORE splitting into h/m/s */
        sec = 0;

    h = sec / 3600;
    m = (sec / 60) % 60;
    s = sec % 60;

    if(h)
        sprintf(buf, "%d:%02d:%02d", h, m, s);
    else
        sprintf(buf, "%d:%02d", m, s);
}

/* last shown values, so a full panel repaint can restore the text instead of
 * leaving a blank until the next second ticks over */
static int time_elapsed = -1;
static int time_total   = 0;

/* render the cached time into the panel pixmap (no blit) */
static void render_time_text(panel_t *panel)
{
    ctx_t  *ctx    = &panel->ctx;
    int    *pixmap = (int*)ctx->pixmap_data;
    int     pitch4 = ctx->pixmap_pitch / 4;
    int     panw   = panel->ctrl.w;
    char    buf[48], es[20], ts[20];
    rect_t  rc;
    int     x, y, x0, x1, tw;

    if(time_elapsed < 0)             /* nothing shown yet */
        return;
    if(panw < 260)                   /* too narrow: would clash with the left */
        return;

    fmt_time(es, time_elapsed);
    if(time_total > 0)
    {
        fmt_time(ts, time_total);
        sprintf(buf, "%s / %s", es, ts);
    }
    else
        strcpy(buf, es);             /* duration unknown: elapsed only */

    x1 = panw - 12;
    x0 = x1 - TIME_MAXW;

    /* restore the panel-body gradient under the text (one colour per row) */
    for(y = TIME_Y0; y < TIME_Y1; y++)
    {
        int  c   = res_panel_body[y];
        int *row = pixmap + y*pitch4;
        for(x = x0; x < x1; x++)
            row[x] = c;
    }

    tw = text_width_ext(win_font, buf);   /* right-align to x1 */
    if(tw > TIME_MAXW)
        tw = TIME_MAXW;

    rc.l = x1 - tw;
    rc.t = TIME_Y0;
    rc.r = x1;
    rc.b = TIME_BASE;
    draw_text_ext(ctx->pixmap_data, ctx->pixmap_pitch, win_font, buf, &rc, TIME_COLOR);
}

void draw_panel_time(panel_t *panel, int elapsed, int total)
{
    ctx_t *ctx  = &panel->ctx;
    int    panw = panel->ctrl.w;
    int    x0   = panw - 12 - TIME_MAXW;

    time_elapsed = elapsed;
    time_total   = total;

    render_time_text(panel);

    if(panw < 260)
        return;

    Blit(ctx->pixmap_data, panel->draw.l + x0, panel->draw.t + TIME_Y0,
         x0, TIME_Y0, TIME_MAXW, TIME_Y1 - TIME_Y0,
         panw, PANEL_HEIGHT, ctx->pixmap_pitch);
}

