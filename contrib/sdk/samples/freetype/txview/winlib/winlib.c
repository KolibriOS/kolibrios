#include <stdio.h>
#include <string.h>
#include "winlib.h"

#define ID_SCROLLER_UP       0x30
#define ID_SCROLLER_DOWN     0x31
#define ID_SCROLLER_THUMB    0x32

typedef int v2si __attribute__ ((vector_size (8)));

font_t *create_font(void *face, int size);

static pos_t    old_pos;
ctrl_t  *mouse_capture = NULL;



void show_window(window_t *win)
{
    BeginDraw();
    DrawWindow(0,0,0,0,NULL,0,0x73);
    if( (win->win_state != MINIMIZED) &&
        (win->win_state != ROLLED) )
        show_context(win->ctx);
    EndDraw();
}

window_t  *create_window(char *caption, int style, int x, int y,
                            int w, int h, handler_t handler)
{
    char proc_info[1024];
    window_t *win;

    if(handler==NULL)
        return NULL;

    win = malloc(sizeof(*win));
    if(win == NULL)
        return NULL;

    BeginDraw();
    DrawWindow(x, y, w+TYPE_3_BORDER_WIDTH*2,
               h+TYPE_3_BORDER_WIDTH+get_skin_height(), caption, 0x000000, 0x73);
    EndDraw();

    GetProcInfo(proc_info);

    x = *(uint32_t*)(proc_info+34);
    y = *(uint32_t*)(proc_info+38);
    w = *(uint32_t*)(proc_info+42)+1;
    h = *(uint32_t*)(proc_info+46)+1;

    win->handler = handler;

    list_initialize(&win->link);
    list_initialize(&win->child);

    win->rc.l = x;
    win->rc.t = y;
    win->rc.r = x + w;
    win->rc.b = y + h;

    win->w = w;
    win->h = h;

    win->client.l = TYPE_3_BORDER_WIDTH;
    win->client.t = get_skin_height();
    win->client.r = w - TYPE_3_BORDER_WIDTH;
    win->client.b = h - TYPE_3_BORDER_WIDTH;
    win->clw = win->client.r - win->client.l;
    win->clh = win->client.b - win->client.t;

    win->caption_txt = caption;
    win->style = style;

    win->child_over  = NULL;
    win->child_focus = NULL;

    win->ctx = create_context(win->client.l, win->client.t, win->clw, win->clh);
    clear_context(win->ctx, 0xFFFFFFFF);

    win->font = create_font(NULL, 14);

    send_message((ctrl_t*)win, MSG_CREATE, 0, 0);
    send_message((ctrl_t*)win, MSG_DRAW, 0, 0);

    ctrl_t *child;

    child  = (ctrl_t*)win->child.next;

    while( &child->link != &win->child)
    {
        send_message(child, MSG_DRAW, 0, 0);
        child = (ctrl_t*)child->link.next;
    };

    show_window(win);

    return win;
};


void handle_sys_paint(window_t *win)
{
    char proc_info[1024];
    int winx, winy, winw, winh;
    uint8_t  state;

    GetProcInfo(proc_info);

    winx = *(uint32_t*)(proc_info+34);
    winy = *(uint32_t*)(proc_info+38);
    winw = *(uint32_t*)(proc_info+42)+1;
    winh = *(uint32_t*)(proc_info+46)+1;

    state  = *(uint8_t*)(proc_info+70);

    if(state & 2)
    {
        win->win_state = MINIMIZED;
        return;
    }
    else if(state & 4)
    {
        win->win_state = ROLLED;
        show_window(win);
        return;
    };

    if(state & 1)
        state = MAXIMIZED;
    else
        state = NORMAL;

    if( (win->w != winw) ||
        (win->h != winh) )
    {
        ctrl_t *child;

        win->client.l = TYPE_3_BORDER_WIDTH;
        win->client.t = get_skin_height();
        win->client.r = winw - TYPE_3_BORDER_WIDTH;
        win->client.b = winh - TYPE_3_BORDER_WIDTH;
        win->clw = win->client.r - win->client.l;
        win->clh = win->client.b - win->client.t;

        resize_context(win->ctx, win->clw, win->clh);

        clear_context(win->ctx, 0xFFFFFFFF);

        send_message((ctrl_t*)win, MSG_SIZE, 0, 0);
        send_message((ctrl_t*)win, MSG_DRAW, 0, 0);

        child  = (ctrl_t*)win->child.next;

        while( &child->link != &win->child)
        {
            send_message(child, MSG_DRAW, 0, 0);
            child = (ctrl_t*)child->link.next;
        };
    }

    win->rc.l = winx;
    win->rc.t = winy;
    win->rc.r = winx + winw;
    win->rc.b = winy + winh;
    win->w = winw;
    win->h = winh;
    win->win_state = state;

    show_window(win);
};


ctrl_t *get_child(ctrl_t *ctrl, int x, int y)
{
    ctrl_t *child = NULL;

    ctrl_t *tmp = (ctrl_t*)ctrl->child.next;

    while( &tmp->link != &ctrl->child )
    {
        if(pt_in_rect(&tmp->rc, x, y))
        {
            child = get_child(tmp, x, y);
            return child == NULL ? tmp : child;
        };
        tmp = (ctrl_t*)tmp->link.next;
    };
    return child;
};


int send_mouse_message(window_t *win, uint32_t msg)
{
    ctrl_t *child;

    if(mouse_capture)
        return send_message(mouse_capture, msg, 0, old_pos.val);

    child = get_child((ctrl_t*)win, old_pos.x, old_pos.y);

    if(msg == MSG_MOUSEMOVE)
    {
        if( win->child_over )
        {
            if(child == win->child_over)
                send_message(child, MSG_MOUSEMOVE, 0, old_pos.val);
            else
                send_message(win->child_over, MSG_MOUSELEAVE, 0, old_pos.val);
        }
        else if( child )
            send_message(child, MSG_MOUSEENTER, 0, old_pos.val);

        win->child_over = child;
    };

    if( child )
        return send_message(child, msg, 0, old_pos.val);

    if(pt_in_rect(&win->client, old_pos.x, old_pos.y))
        return send_message((ctrl_t*)win, msg, 0, old_pos.val);

};

#define DBG(x)

static void handle_sys_mouse(window_t *win)
{
    static uint32_t  mouse_click_time;
    static int  mouse_action;
    static int  old_buttons;
    int         buttons;
    uint32_t    wheels;
    uint32_t    click_time;
    int         action;

    pos_t       pos;

    mouse_action = 0;
    pos = get_mouse_pos(POS_WINDOW);

    if(pos.val != old_pos.val)
    {
        mouse_action = 0x80000000;
        old_pos = pos;
    };
//    printf("pos x%d y%d\n", pos.x, pos.y);

    buttons = get_mouse_buttons();
    wheels = get_mouse_wheels();

//    if( wheels & 0xFFFF){
//        wheels = (short)wheels>0 ? MSG_WHEELDOWN : MSG_WHEELUP;
//        send_mouse_message(win, wheels);
//    }

    if((action = (buttons ^ old_buttons))!=0)
    {
        mouse_action|= action<<3;
        mouse_action|= buttons & ~old_buttons;
    }
    old_buttons = buttons;

    if(mouse_action & 0x80000000) {
        DBG("mouse move \n\r");
        send_mouse_message(win, MSG_MOUSEMOVE);
    };

    if(mouse_action & 0x09)
    {
        if((mouse_action & 0x09)==0x09)
        {
//            printf("left button down x= %d y= %d\n\r", old_x.x, old_x.y);
            click_time = get_tick_count();
            if(click_time < mouse_click_time+35) {
                mouse_click_time = click_time;
                send_mouse_message(win,MSG_LBTNDBLCLK);
            }
            else {
              mouse_click_time = click_time;
              send_mouse_message(win,MSG_LBTNDOWN);
            };
        }
        else {
//            printf("left button up \n\r");
            send_mouse_message(win,MSG_LBTNUP);
        }
    };

    if(mouse_action & 0x12)
    {
        if((mouse_action & 0x12)==0x12) {
            DBG("right button down \n\r");
            send_mouse_message(win,MSG_RBTNDOWN);
        }
        else {
            DBG("right button up \n\r");
            send_mouse_message(win,MSG_RBTNUP);
        };
    };
    if(mouse_action & 0x24)
    {
        if((mouse_action & 0x24)==0x24){
            DBG("middle button down \n\r");
            send_mouse_message(win,MSG_MBTNDOWN);
        }
        else {
            DBG("middle button up \n\r");
            send_mouse_message(win,MSG_MBTNUP);
        };
    };
};


int handle_system_events(window_t *win)
{
    oskey_t   key;

    for (;;)
    {
        switch (get_os_event())
        {
            case MSG_SYS_PAINT:
                handle_sys_paint(win);
                break;

            case MSG_SYS_KEY:
                key = get_key();
                printf("key %d\n", key.code);
                break;

            case MSG_SYS_BUTTON:
                // button pressed; we have only one button, close
                return 0;

            case MSG_SYS_MOUSE:
                handle_sys_mouse(win);
                break;
        };

        if( (win->win_state != MINIMIZED) &&
            (win->win_state != ROLLED) )
            show_context(win->ctx);
    }

    return 0;
}

void *create_control(size_t size, int id, int x, int y,
                         int w, int h, ctrl_t *parent)
{

    ctrl_t  *ctrl;

    if( !parent )
        return NULL;

    ctrl = (ctrl_t*)malloc(size);

    link_initialize(&ctrl->link);
    list_initialize(&ctrl->child);

    ctrl->parent  = parent;

    ctrl->ctx     = parent->ctx;
    ctrl->font    = parent->font;
    ctrl->id      = id;

    ctrl->rc.l    = x;
    ctrl->rc.t    = y ;

    ctrl->rc.r    = x + w;
    ctrl->rc.b    = y + h;

    ctrl->w       = w;
    ctrl->h       = h;

    list_append(&ctrl->link, &parent->child);

    return ctrl;
};


int move_ctrl(ctrl_t *ctrl, int x, int y, int w, int h)
{
    rect_t rc;
    rc.l = x;
    rc.t = y;
    rc.r = w;
    rc.b = h;

    send_message(ctrl, MSG_POSCHANGING, 0, &rc);

    ctrl->rc.l = rc.l;
    ctrl->rc.t = rc.t;
    ctrl->rc.r = rc.l + rc.r;
    ctrl->rc.b = rc.t + rc.b;
    ctrl->w = rc.r;
    ctrl->h = rc.b;

    send_message(ctrl, MSG_POSCHANGE, 0, &rc);

    return 1;
};



/*
 *
 *
 *
 *
 *
 *
 *
 *
*/

int def_ctrl_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    switch( msg )
    {

    };

    return 0;
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
*/


static void scroller_on_draw(scroller_t *scrl)
{
    if(scrl->btn_up)
        send_message((ctrl_t*)scrl->btn_up, MSG_DRAW, 0, 0);

    if(scrl->btn_down)
        send_message((ctrl_t*)scrl->btn_down, MSG_DRAW, 0, 0);

    if(scrl->thumb)
        send_message((ctrl_t*)scrl->thumb, MSG_DRAW, 0, 0);

    if(scrl->tl_rect.t != scrl->tl_rect.b)
        px_fill_rect(scrl->ctrl.ctx, &scrl->tl_rect, 0xFFE1D8D0);

    if(scrl->br_rect.t != scrl->br_rect.b)
        px_fill_rect(scrl->ctrl.ctx, &scrl->br_rect, 0xFFE1D8D0);
}

static void scroller_on_ownerdraw(scroller_t *scrl, ctrl_t *child)
{
typedef struct
{
    ctrl_t   ctrl;

    uint32_t state;

    char     *caption;
    int       capt_len;
}button_t;

    button_t *btn;
    color_t color = 0xFFD7D7D7;
    char t = (char)child->id;
    rect_t rc;

    switch(child->id)
    {
        case ID_SCROLLER_UP:
        case ID_SCROLLER_DOWN:
            btn = (button_t*)child;
            if(btn->state & bPressed)
                color = 0xFFB0B0B0;
            else if(btn->state & bHighlight)
                color = 0xFFE7E7E7;

            rc = btn->ctrl.rc;
            px_fill_rect(btn->ctrl.ctx, &rc, color);
            rc.l+= 3;
            rc.t+= 5;
            draw_text_ext(btn->ctrl.ctx, sym_font, &t, 1, &rc, 0xFF000000);
            break;

        case ID_SCROLLER_THUMB:
            btn = (button_t*)child;
            color = 0xFFB0B0B0;
            if(btn->state & bPressed)
                color = 0xFF707070;
            else if(btn->state & bHighlight)
                color = 0xFF909090;

            rc = btn->ctrl.rc;
            px_fill_rect(btn->ctrl.ctx, &rc, color);
    }
};

static void scroller_update_layout(scroller_t *scrl)
{

//    th_size = scrl->pix_range*scrl->page_size/
//              (scrl->max_range-scrl->min_range);

//    if(th_size > scrl->pix_range)
//        th_size = scrl->pix_range;

    scrl->tl_rect.l = scrl->ctrl.rc.l;
    scrl->br_rect.l = scrl->ctrl.rc.l;

    scrl->tl_rect.r = scrl->ctrl.rc.r;
    scrl->br_rect.r = scrl->ctrl.rc.r;

    scrl->tl_rect.t = scrl->btn_up->rc.b;
    scrl->tl_rect.b = scrl->thumb->rc.t;

    scrl->br_rect.t = scrl->thumb->rc.b;
    scrl->br_rect.b = scrl->btn_down->rc.t;

    scrl->pix_range = scrl->ctrl.h - 40 - 20;
};

static void scroller_on_poschange(scroller_t *scrl, rect_t *pos)
{
    move_ctrl(scrl->btn_up, pos->l, pos->t, pos->r, pos->r);
    move_ctrl(scrl->btn_down, pos->l, pos->t+pos->b-pos->r, pos->r, pos->r);
    move_ctrl(scrl->thumb, pos->l, pos->t+pos->r, pos->r, pos->r);

    scroller_update_layout(scrl);
    scroller_on_draw(scrl);
};

static void scroller_on_command(scroller_t *ctrl, int id, ctrl_t *child, int notify)
{
    scroller_t *scrl = (scroller_t*)ctrl;
    int thumb_pos = scrl->thumb_pos;

    switch(id)
    {
        case ID_SCROLLER_UP:
            if(scrl->thumb_pos > scrl->min_range)
                scrl->thumb_pos--;
            printf("scroll up\n");
            break;
        case ID_SCROLLER_DOWN:
            if(scrl->thumb_pos < scrl->max_range)
                scrl->thumb_pos++;
            printf("scroll down\n");
            break;
    };

    if(thumb_pos != scrl->thumb_pos)
    {
        rect_t rc = scrl->thumb->rc;
        int offset = scrl->pix_range*scrl->thumb_pos;
        offset /= scrl->max_range - scrl->min_range;
        rc.t = scrl->ctrl.rc.t + scrl->btn_up->h + offset;
        rc.r = scrl->thumb->w;
        rc.b = scrl->thumb->h;
        move_ctrl(scrl->thumb, rc.l, rc.t, rc.r, rc.b);
        scroller_update_layout(scrl);
        scroller_on_draw(scrl);
    }
}

int scroller_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    scroller_t *scrl = (scroller_t*)ctrl;

    switch( msg )
    {
        HANDLE_MSG(scrl, MSG_DRAW, scroller_on_draw);
        HANDLE_MSG(scrl, MSG_COMMAND, scroller_on_command);
        HANDLE_MSG(scrl, MSG_OWNERDRAW, scroller_on_ownerdraw);
        HANDLE_MSG(scrl, MSG_POSCHANGE, scroller_on_poschange);
    };

    return 0;
};


scroller_t *create_scroller(uint32_t style, int id, int x, int y,
                            int w, int h, ctrl_t *parent)
{
    scroller_t  *scrl;

    if( !parent )
        return NULL;

    scrl = create_control(sizeof(scroller_t), id, x, y, w, h, parent);
    scrl->ctrl.handler = scroller_proc;

    scrl->min_range = 0;
    scrl->max_range = 100;
    scrl->thumb_pos = 0;
    scrl->page_size = 1;

    scrl->btn_up = create_button(NULL, 1, ID_SCROLLER_UP, x, y, w, w, (ctrl_t*)scrl);
    scrl->btn_down = create_button(NULL, 1, ID_SCROLLER_DOWN, x, y+h-w, w, w, (ctrl_t*)scrl);
    scrl->thumb = create_button(NULL, 1, ID_SCROLLER_THUMB, x, w, w, w, (ctrl_t*)scrl);

    scroller_update_layout(scrl);

    return scrl;
};


