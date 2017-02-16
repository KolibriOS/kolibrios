#ifndef __WINLIB_H__
#define __WINLIB_H__

#include <stdlib.h>
#include <kos32sys.h>
#include "control.h"

enum win_state{
      NORMAL, MINIMIZED, ROLLED, MAXIMIZED, FULLSCREEN
};


typedef struct
{
    link_t       link;
    link_t       child;

    handler_t   *handler;
    ctrl_t      *parent;

    ctx_t       *ctx;
    font_t      *font;

    uint32_t     id;
    uint32_t     style;

    rect_t       rc;
    int          w;
    int          h;

    rect_t       saved;
    rect_t       client;
    int          clw;
    int          clh;

    char        *caption_txt;
    ctrl_t      *child_over;
    ctrl_t      *child_focus;

    enum win_state win_state;
    enum win_state saved_state;

}window_t;


#define HANDLE_MSG(ctrl, message, fn)    \
    case (message): return HANDLE_##message((ctrl), (arg1), (arg2), (fn))

/* void ctrl_on_draw(ctrl_t *ctrl) */
#define HANDLE_MSG_DRAW(ctrl, arg1, arg2, fn) \
    ((fn)(ctrl),0)

/* void ctrl_on_ownerdraw(ctrl_t *ctrl, ctrl_t *child) */
#define HANDLE_MSG_OWNERDRAW(ctrl, arg1, arg2, fn) \
    ((fn)((ctrl),((ctrl_t*)arg1)),0)

/* void ctrl_on_poschanging(ctrl_t *ctrl, rect_t *pos) */
#define HANDLE_MSG_POSCHANGING(ctrl, arg1, arg2, fn) \
    ((fn)((ctrl),((rect_t*)arg2)),0)

/* void ctrl_on_poschange(ctrl_t *ctrl, rect_t *pos) */
#define HANDLE_MSG_POSCHANGE(ctrl, arg1, arg2, fn) \
    ((fn)((ctrl),((rect_t*)arg2)),0)

/* void ctrl_on_mouseenter(ctrl_t *ctrl) */
#define HANDLE_MSG_MOUSEENTER(ctrl, arg1, arg2, fn) \
    ((fn)(ctrl),0)

/* void ctrl_on_mouseleave(ctrl_t *ctrl) */
#define HANDLE_MSG_MOUSELEAVE(ctrl, arg1, arg2, fn) \
    ((fn)(ctrl),0)

/* void ctrl_on_lbuttondown(ctrl_t *ctrl, int x, int y) */
#define HANDLE_MSG_LBTNDOWN(ctrl, arg1, arg2, fn) \
    ((fn)((ctrl), ((pos_t)arg2).x, ((pos_t)arg2).y), 0L)

#define HANDLE_MSG_LBTNDBLCLK(ctrl, arg1, arg2, fn) \
    ((fn)((ctrl), ((pos_t)arg2).x, ((pos_t)arg2).y), 0L)

/* void ctrl_on_lbuttonup(ctrl_t *ctrl, int x, int y) */
#define HANDLE_MSG_LBTNUP(ctrl, arg1, arg2, fn) \
    ((fn)((ctrl), ((pos_t)arg2).x, ((pos_t)arg2).y), 0L)

/* void ctrl_on_mousemove(ctrl_t *ctrl, int x, int y) */
#define HANDLE_MSG_MOUSEMOVE(ctrl, arg1, arg2, fn) \
    ((fn)((ctrl), ((pos_t)arg2).x, ((pos_t)arg2).y), 0L)

/* void ctrl_on_command(ctrl_t *ctrl, int id, ctrl_t *child, int notify) */
#define HANDLE_MSG_COMMAND(ctrl,arg1,arg2,fn)       \
    ((fn)((ctrl),(int)(arg1 & 0xFFFF),(ctrl_t*)(arg2),(int)(arg1>>16)),0)

window_t  *create_window(char *caption, int style, int x, int y,
                            int w, int h, handler_t handler);
int handle_system_events(window_t *win);
void show_window(window_t *win);

extern ctrl_t  *mouse_capture;

static inline ctrl_t *capture_mouse(ctrl_t *newm)
{
    ctrl_t *old = mouse_capture;

    mouse_capture = newm;

    __asm__ __volatile__(
    "int $0x40"
    ::"a"(40), "b"(0x80000027));

    return old;
}

static void release_mouse(void)
{
    mouse_capture = NULL;
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(40), "b"(0xC0000027));
}

extern font_t *sym_font;

int draw_text_ext(ctx_t *ctx, font_t *font, char *text, int len, rect_t *rc, color_t color);

#endif /* __WINLIB_H__  */
