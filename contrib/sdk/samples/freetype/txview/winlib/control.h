#ifndef __CONTROL_H__
#define __CONTROL_H_

#include <stdint.h>
#include <pxdraw.h>
#include "link.h"

typedef struct font font_t;

typedef struct control  ctrl_t;

typedef int (handler_t)(ctrl_t*, uint32_t, uint32_t, uint32_t);

struct control
{
    link_t     link;
    link_t     child;

    handler_t *handler;
    ctrl_t    *parent;

    ctx_t     *ctx;
    font_t    *font;
    uint32_t   id;
    uint32_t   style;

    rect_t     rc;
    int        w;
    int        h;
};

void *create_control(size_t size, int id, int x, int y,
                         int w, int h, ctrl_t *parent);

#define  bPressed              2
#define  bHighlight            1


ctrl_t *create_button(char *caption, uint32_t style, int id, int x, int y,
                      int w, int h, ctrl_t *parent);

typedef struct
{
  ctrl_t    ctrl;

  uint32_t  state;

  int       pix_range;
  int       min_range;
  int       max_range;
  int       page_size;
  int       thumb_pos;

  rect_t    tl_rect;
  rect_t    br_rect;

  ctrl_t   *btn_up;
  ctrl_t   *btn_down;
  ctrl_t   *thumb;
}scroller_t;


#define  MSG_SYS_PAINT     0x001
#define  MSG_SYS_KEY       0x002
#define  MSG_SYS_BUTTON    0x003
#define  MSG_SYS_MOUSE     0x006

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

#define  MSG_CREATE        0x020
#define  MSG_SIZE          0x021
#define  MSG_DRAW          0x022
#define  MSG_OWNERDRAW     0x023
#define  MSG_POSCHANGING   0x024
#define  MSG_POSCHANGE     0x025

#define  MSG_COMMAND       0x030

static inline int pt_in_rect(rect_t *rc, int x, int y)
{
    if( (x >= rc->l) && (x <  rc->r) &&
        (y >= rc->t) && (y <  rc->b) )
        return 1;
    return 0;
};


#define  send_message( ctrl, msg, arg1, arg2)                   \
                      (ctrl)->handler( (ctrl_t*)(ctrl),         \
                      (uint32_t)(msg), (uint32_t)(arg1), (uint32_t)(arg2))

#define __ALIGN_MASK(x,mask)  (((x)+(mask))&~(mask))
#define ALIGN(x,a)            __ALIGN_MASK(x,(typeof(x))(a)-1)


#endif /* __CONTROL_H_ */
