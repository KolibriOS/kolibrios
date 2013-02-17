#ifndef __WINLIB_H__
#define __WINLIB_H__

#include "control.h"

#define CAPTION_HEIGHT      24
#define PANEL_HEIGHT        55

typedef struct
{
    link_t      link;
    link_t      child;

    handler_t  *handler;
    ctrl_t     *parent;

    ctx_t      *ctx;
    uint32_t    id;
    uint32_t    style;

    rect_t      rc;
    int         w;
    int         h;

    rect_t      left;    /*  left  border            */
    rect_t      right;   /*  right border            */
    rect_t      bottom;  /*  bottom border           */

    button_t   *close_btn;
    rect_t     *track;
}frame_t;

typedef struct
{
    ctrl_t     ctrl;
    ctx_t      ctx;
    bitmap_t   bitmap;
    char      *text;
    ctrl_t    *child_over;
    button_t  *close_btn;
    button_t  *minimize_btn;

}caption_t;

typedef struct
{
    ctrl_t      ctrl;
    ctx_t       ctx;
    bitmap_t    bitmap;
    rect_t      draw;
    ctrl_t     *child_over;
    int         layout;
    progress_t *prg;
    level_t    *lvl;
    slider_t   *sld;
    button_t   *play_btn;
    button_t   *stop_btn;
}panel_t;

typedef struct
{
    link_t       link;
    link_t       child;

    handler_t   *handler;
    ctrl_t      *parent;

    ctx_t       *ctx;
    uint32_t     id;
    uint32_t     style;

    rect_t       rc;
    int          w;
    int          h;

    rect_t       client;

    ctx_t        client_ctx;
    bitmap_t     bitmap;

    char        *caption_txt;
    ctrl_t      *child_over;
    ctrl_t      *child_focus;

    caption_t    caption;
    panel_t      panel;
    frame_t      frame;

    enum win_state{
      NORMAL, MINIMIZED, ROLLED, MAXIMIZED
    }win_state;
    enum win_command{
        WIN_CLOSED=1
    }win_command;

}window_t;

#define get_parent_window(x) ((window_t*)((x)->parent))

ctrl_t *win_get_child(window_t *win, int x, int y);

void init_winlib(void);

void draw_caption(caption_t *cpt);
void draw_panel(panel_t *panel);
void blit_caption(caption_t *cpt);
int  init_caption(window_t *win);
int  init_panel(window_t *win);



window_t  *create_window(char *caption, int style, int x, int y,
                            int w, int h, handler_t handler);

int show_window(window_t *win, int state);

int def_window_proc(ctrl_t  *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2);

void frame_run(window_t *win);

button_t *create_button(char *caption, int id, int x, int y,
                        int w, int h, ctrl_t *parent);
progress_t *create_progress(char *caption, int id, int x, int y,
                            int w, int h, ctrl_t *parent);
level_t    *create_level(char *caption, int id, int x, int y,
                         int w, int h, ctrl_t *parent);
scroller_t *create_scroller(uint32_t style, int id, int x, int y,
                            int w, int h, ctrl_t *parent);
slider_t  *create_slider(char *caption, int id, int x, int y,
                         int w, int h, ctrl_t *parent);


//static uint32_t update_timers(uint32_t realtime);

int set_timer(ctrl_t *ctrl, ostimer_t *timer, uint32_t delay);

void update_rect(ctrl_t *ctrl);


#endif
