#ifndef __CONTROL_H__
#define __CONTROL_H_

#include <pixlib3.h>
#include "link.h"

typedef struct
{
  int  l;
  int  t;
  int  r;
  int  b;
}rect_t;

typedef struct ctx
{
  int  offset_x;
  int  offset_y;
  int *pixmap_data;
  int  pixmap_pitch;
}ctx_t;

ctx_t *get_window_ctx();

typedef struct tag_control  ctrl_t;

typedef int (handler_t)(ctrl_t*, uint32_t, uint32_t, uint32_t);

struct tag_control
{
    link_t     link;
    link_t     child;

    handler_t *handler;
    ctrl_t    *parent;

    ctx_t     *ctx;
    uint32_t   id;
    uint32_t   style;

    rect_t     rc;
    int        w;
    int        h;
};


typedef struct timer
{
    link_t       link;
    ctrl_t      *ctrl;
    uint32_t     exp_time;            /* expiration time               */
    uint32_t     tmr_arg;             /* random argument               */
} ostimer_t;


typedef struct
{
    ctrl_t  ctrl;

    uint32_t     state;
    ostimer_t    timer;

    char        *caption;
    int          capt_len;

    void        *img_default;
    void        *img_hilite;
    void        *img_pressed;
}button_t;

typedef struct
{
    ctrl_t  ctrl;
    float   min;
    float   max;
    float   current;
    int     pos;
}progress_t;

typedef struct
{
    ctrl_t  ctrl;
    int     min;
    int     max;
    int     current;
    int     pos;
    int     vol;
    int     visible;
    void    *img_level;
}level_t;

typedef struct
{
    ctrl_t  ctrl;
    int     min;
    int     max;
    int     current;
    int     pos;
    int     mode;
    void    *img_slider;
    void    *img_vol_slider;
}slider_t;

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

    uint32_t     state;

    int          pix_range;

    int          min_range;
    int          max_range;
    int          page_size;
    int          thumb_pos;

    rect_t       tl_rect;
    rect_t       br_rect;

    button_t    *btn_up;
    button_t    *btn_down;
    button_t    *thumb;
}scroller_t;

#define  bPressed              2
#define  bHighlight            1

#define  MSG_PAINT         0x001
#define  MSG_KEY           0x002
#define  MSG_BUTTON        0x003
#define  MSG_DRAW_CLIENT   0x004

#define  MSG_LBTNDOWN      0x010
#define  MSG_LBTNUP        0x011
#define  MSG_RBTNDOWN      0x012
#define  MSG_RBTNUP        0x013
#define  MSG_MBTNDOWN      0x014
#define  MSG_MBTNUP        0x015
#define  MSG_WHEELDOWN     0x016
#define  MSG_WHEELUP       0x017

#define  MSG_LBTNDBLCLK    0x018

#define  MSG_MOUSEMOVE     0x019
#define  MSG_MOUSEENTER    0x01A
#define  MSG_MOUSELEAVE    0x01B

#define  MSG_SIZE          0x020

#define  MSG_COMMAND       0x030
#define  MSG_TIMER         0x031

#define  LBN_DBLCLK        0x100
#define  LBOX_READDIR      0x100
#define  LBOX_GETFILENAME  0x101

#define  PRG_PROGRESS      0x102

#define  ID_CLOSE              1
#define  ID_MINIMIZE           2
#define  ID_FULLSCREEN         3

#define  ID_SCROLLER_UP       10
#define  ID_SCROLLER_DOWN     11
#define  ID_SCROLLER_THUMB    12

#define  send_message( ctrl, msg, arg1, arg2)              \
                (ctrl)->handler( (ctrl_t*)(ctrl),          \
                (uint32_t)(msg), (uint32_t)(arg1), (uint32_t)(arg2))

static inline handler_t *subclass_control(ctrl_t *ctrl, handler_t *handler)
{
    handler_t *old = ctrl->handler;
    ctrl->handler = handler;
    return old;
};

//int inline send_message(ctrl_t *ctrl, u32_t msg, u32_t arg1, u32_t arg2)
//{
//  return ctrl->handler(ctrl, msg, arg1, arg2);
//};

static inline int pt_in_rect(rect_t *rc, int x, int y)
{
    if( (x >= rc->l) && (x <  rc->r) &&
        (y >= rc->t) && (y <  rc->b) )
        return 1;
    return 0;
};

ctrl_t *get_child(ctrl_t *ctrl, int x, int y);

ctrl_t *capture_mouse(ctrl_t *newm);
void release_mouse(void);

void blit_raw(ctx_t *ctx, void *raw, int x, int y, int w, int h, int pitch);

#define __ALIGN_MASK(x,mask)  (((x)+(mask))&~(mask))
#define ALIGN(x,a)            __ALIGN_MASK(x,(typeof(x))(a)-1)


#endif
