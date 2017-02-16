#include <stdio.h>
#include <string.h>
#include "winlib.h"

typedef struct
{
    ctrl_t   ctrl;

    uint32_t state;

    char     *caption;
    int       capt_len;
}button_t;

static void button_on_draw(button_t *btn)
{
    color_t color = 0xFFD7D7D7;
    char t = (char)btn->ctrl.id;
    rect_t rc, rci;

    if(btn->ctrl.style & 1)
    {
        send_message(btn->ctrl.parent, MSG_OWNERDRAW, btn, 0);
        return;
    }

    if(btn->state & bPressed)
        color = 0xFFB0B0B0;
    else if(btn->state & bHighlight)
        color = 0xFFE7E7E7;

    rc = rci = btn->ctrl.rc;
    rci.l++;
    rci.t++;

    px_hline(btn->ctrl.ctx, rc.l, rc.t, btn->ctrl.w, 0xFF646464);
    px_fill_rect(btn->ctrl.ctx,  &rci, color);
    px_hline(btn->ctrl.ctx, rc.l, rc.b-1, btn->ctrl.w, 0xFF646464);
    px_vline(btn->ctrl.ctx, rc.l, rc.t+1, btn->ctrl.h-2, 0xFF646464);
    px_vline(btn->ctrl.ctx, rc.r-1, rc.t+1, btn->ctrl.h-2, 0xFF646464);

    rc.l+= 4;
    rc.t+= 6;

    draw_text_ext(btn->ctrl.ctx, btn->ctrl.font, btn->caption, btn->capt_len, &rc, 0xFF000000);
};

static void button_on_mouseenter(button_t *btn)
{
    btn->state|= bHighlight;
    send_message(&btn->ctrl, MSG_DRAW, 0, 0);
}

static void button_on_mouseleave(button_t *btn)
{
    if( (ctrl_t*)btn != mouse_capture) {
        btn->state &= ~bHighlight;
        send_message(&btn->ctrl, MSG_DRAW, 0, 0);
    };
}

static void button_on_lbuttondown(button_t *btn, int x, int y)
{
    capture_mouse((ctrl_t*)btn);
    btn->state|= bPressed;
    send_message(&btn->ctrl, MSG_DRAW, 0, 0);
};

static void button_on_lbuttonup(button_t *btn, int x, int y)
{
    int action;

    action = (btn->state & bPressed) ? MSG_COMMAND : 0;

    release_mouse();

    if( pt_in_rect( &btn->ctrl.rc, x, y) )
        btn->state = bHighlight;
    else
        btn->state = 0;

    send_message(&btn->ctrl, MSG_DRAW, 0, 0);

    if(action)
        send_message(btn->ctrl.parent,MSG_COMMAND,btn->ctrl.id,(int)btn);
};

static void button_on_mousemove(button_t *btn, int x, int y)
{
    int  old_state;

    if( !(btn->state & bHighlight))
    {
        btn->state|= bHighlight;
        send_message(&btn->ctrl, MSG_DRAW, 0, 0);
    };

    if( (ctrl_t*)btn != mouse_capture)
        return;

    old_state = btn->state;

    if( pt_in_rect(&btn->ctrl.rc, x, y) )
        btn->state |= bPressed;
    else
        btn->state &= ~bPressed;

    if( old_state ^ btn->state)
        send_message(&btn->ctrl, MSG_DRAW, 0, 0);
}

int button_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    button_t *btn = (button_t*)ctrl;

    switch( msg )
    {
        HANDLE_MSG(btn, MSG_DRAW, button_on_draw);
        HANDLE_MSG(btn, MSG_MOUSEENTER, button_on_mouseenter);
        HANDLE_MSG(btn, MSG_MOUSELEAVE, button_on_mouseleave);
        HANDLE_MSG(btn, MSG_LBTNDOWN, button_on_lbuttondown);
        HANDLE_MSG(btn, MSG_LBTNDBLCLK, button_on_lbuttondown);
        HANDLE_MSG(btn, MSG_LBTNUP, button_on_lbuttonup);
        HANDLE_MSG(btn, MSG_MOUSEMOVE, button_on_mousemove);
    }
    return 0;
};

ctrl_t *create_button(char *caption, uint32_t style, int id, int x, int y,
                      int w, int h, ctrl_t *parent)
{
    button_t  *btn;
    int        len;

    if( !parent )
        return NULL;

    btn = create_control(sizeof(button_t), id, x, y, w, h, parent);
    btn->ctrl.style = style;
    btn->ctrl.handler = button_proc;
    btn->state = 0;
    btn->caption = caption;

    if( !caption )
        btn->capt_len = 0;
    else
    {
        len = strlen(caption);
        btn->capt_len = len;
        if( len )
            btn->caption = strdup(caption);
        else
            btn->caption = NULL;
    }

    return &btn->ctrl;
};

