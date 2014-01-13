
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "winlib.h"

#define DBG(format,...)

int draw_frame(window_t *win);
static int draw_window(window_t *win);

uint32_t main_cursor;
uint32_t cursor_ns;
uint32_t cursor_we;
uint32_t cursor_nwse;
uint32_t cursor_nesw;

int win_font;

static window_t Window;

static pos_t    old_pos;

ctrl_t  *mouse_capture = NULL;

static link_t    timers;
static uint32_t  realtime;
static uint32_t  wait_time;
static uint32_t  exp_time;

static int need_update;

#define LOAD_FROM_MEM                1


void adjust_frame(window_t *win);

//#include "timer.h"
ctrl_t *win_get_child(window_t *win, int x, int y)
{
    ctrl_t *child = NULL;

    if( win )
    {
        if(pt_in_rect(&win->client, x, y))
        {
            ctrl_t *tmp = (ctrl_t*)win->child.next;

            while( &tmp->link != &win->child )
            {
                if(pt_in_rect(&tmp->rc, x, y))
                {
                    child = get_child(tmp, x, y);
                    return child == NULL ? tmp : child;
                };
                tmp = (ctrl_t*)tmp->link.next;
            };
        }
        else child = (ctrl_t*)(&win->frame);
    };
    return child;
};


void init_frame(window_t *win);

window_t *create_window(char *caption, int style, int x, int y,
                          int w, int h, handler_t handler)
{
    char proc_info[1024];
    int stride;

//    __asm__ __volatile__("int3");


//    ctx_t *ctx = &Window.client_ctx;

    if(handler==0) return 0;

    BeginDraw();
    DrawWindow(x, y, w-1, h-1,
               NULL,0,0x41);
    EndDraw();

    get_proc_info(proc_info);

    x = *(uint32_t*)(proc_info+34);
    y = *(uint32_t*)(proc_info+38);
    w = *(uint32_t*)(proc_info+42)+1;
    h = *(uint32_t*)(proc_info+46)+1;

    Window.handler = handler;
 //   Window.ctx = ctx;

    list_initialize(&Window.link);
    list_initialize(&Window.child);


//    Window.bitmap.width  = 1920;
//    Window.bitmap.height = 1080;
//    Window.bitmap.flags  = 0;

 //   if( create_bitmap(&Window.bitmap) )
 //   {
 //       printf("not enough memory for window bitmap\n");
 //       return 0;
  //  }

 //   ctx->pixmap   = &Window.bitmap;
 //   ctx->offset_x = 0;
 //   ctx->offset_y = 0;

    Window.rc.l = x;
    Window.rc.t = y;
    Window.rc.r = x + w;
    Window.rc.b = y + h;

    Window.w = w;
    Window.h = h;

    Window.caption_txt = caption;
    Window.style = style;

    Window.child_over  = NULL;
    Window.child_focus = NULL;

    init_caption(&Window);
    init_panel(&Window);
    init_frame(&Window);
    send_message((ctrl_t*)&Window, MSG_SIZE, 0, 0);
    return &Window;
};


int def_window_proc(ctrl_t  *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    ctrl_t   *child;

    window_t  *win = (window_t*)ctrl;

    switch(msg)
    {
        case MSG_PAINT:
            draw_window(win);
            break;

        case 2:
            child  = (ctrl_t*)win->child.next;
            while( &child->link != &win->child)
            {
                send_message(child, 2, arg1, arg2);
                child = (ctrl_t*)child->link.next;
            };
            break;

        case MSG_MOUSEMOVE:
            child = win_get_child(win, arg2 & 0xFFFF, (arg2>>16));
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

        case MSG_SIZE:
            break;

        default:
            child = win_get_child(win, arg2 & 0xFFFF, (arg2>>16));
            win->child_over = child;
            if(child) send_message(child, msg, 0, arg2);
    };

    return 0;
}

static int draw_window(window_t *win)
{
    ctrl_t *child;
    void   *ctx;
    rect_t *rc = &win->client;

    draw_caption(&win->caption);
    draw_panel(&win->panel);
    send_message(win, MSG_DRAW_CLIENT,0,0);

//    draw_frame(win);

//    child  = (ctrl_t*)win->child.next;

//    while( &child->link != &win->child)
//    {
//        send_message(child, 1, 0, 0);
//        child = (ctrl_t*)child->link.next;
//    };

    return 0;
};

void blit_client(window_t *win)
{
    int w, h;

    w = win->client.r - win->client.l;
    h = win->client.b - win->client.t;

    Blit(win->ctx->pixmap->data, win->client.l, win->client.t,
         0, 0, w, h, w, h,win->ctx->pixmap->pitch);
};


int show_window(window_t *win, int state)
{
    win->win_state = state;

    draw_window(win);

    BeginDraw();
    DrawWindow(win->rc.l, win->rc.t, win->w-1, win->h-1,
               NULL,0,0x41);
    EndDraw();

    blit_caption(&win->caption);
    blit_panel(&win->panel);
//    blit_client(win);
    return 0;
}

void window_update_layout(window_t *win)
{
    char proc_info[1024];

    int new_w, new_h;
    uint8_t  state;

    int winx, winy, winw, winh;

//    __asm__ __volatile__("int3");

    get_proc_info(proc_info);

    winx = *(uint32_t*)(proc_info+34);
    winy = *(uint32_t*)(proc_info+38);
    winw = *(uint32_t*)(proc_info+42)+1;
    winh = *(uint32_t*)(proc_info+46)+1;

    state  = *(uint8_t*)(proc_info+70);

    if(state & 2)
    {   win->win_state = MINIMIZED;
        return;
    }
    if(state & 4)
    {
        win->win_state = ROLLED;
        return;
    };

    if(state & 1)
        state = MAXIMIZED;
    else
        state = NORMAL;

    if( (winx != win->rc.l) || (winy != win->rc.t) )
    {
        win->rc.l = winx;
        win->rc.t = winy;
        win->rc.r = winx + win->w;
        win->rc.b = winy + win->h;
    };

//    if( winw  == win->w &&
//        winh  == win->h &&
//        state == win->win_state)
//        return;

    if(win->win_state != FULLSCREEN)
        win->win_state = state;

#if 0
    int old_size;
    int new_size;
    int pitch;


    old_size = win->bitmap.pitch * win->bitmap.height;
    old_size = (old_size+4095) & ~4095;

    pitch = ALIGN(win->w*4, 16);

    new_size = pitch * win->h;
    new_size = (new_size+4095) & ~4095;

    if( new_size < old_size)
        user_unmap(win->bitmap.data, new_size, old_size-new_size);

    win->bitmap.width = win->w;
    win->bitmap.pitch = pitch;
#endif

    win->rc.r = winx + winw;
    win->rc.b = winy + winh;
    win->w = winw;
    win->h = winh;

    update_caption_size(win);
    update_panel_size(win);
    adjust_frame(win);

    send_message((ctrl_t*)win, MSG_SIZE, 0, 0);
    draw_window(win);
};


int send_mouse_message(window_t *win, uint32_t msg)
{
    ctrl_t *child;

    if(mouse_capture)
        return send_message(mouse_capture, msg, 0, old_pos.val);

    if(pt_in_rect(&win->client, old_pos.x, old_pos.y))
        return send_message((ctrl_t*)win, msg, 0, old_pos.val);

    if(win->win_state == FULLSCREEN)
        return 0;

    if(pt_in_rect(&win->caption.ctrl.rc, old_pos.x, old_pos.y))
    {
        return send_message(&win->caption.ctrl, msg, 0, old_pos.val);
    }

    if(pt_in_rect(&win->panel.ctrl.rc, old_pos.x, old_pos.y))
    {
//        old_pos.x-= win->panel.ctrl.rc.l;
//        old_pos.y-= win->panel.ctrl.rc.t;
        return send_message(&win->panel.ctrl, msg, 0, old_pos.val);
    }


    return send_message(&win->frame, msg, 0, old_pos .val);

//    if( ( old_pos.x < win->rc.r) && ( old_pos.y < win->rc.b))
//        send_message((ctrl_t*)win, msg, 0, old_pos.val);
};

void do_sys_draw(window_t *win)
{
//    printf("%s win:%x\n", __FUNCTION__, win);

    window_update_layout(win);

    BeginDraw();
    DrawWindow(0,0,0,0, NULL, 0x000000,0x41);
//    DefineButton(15, 15, 0x00000001, 0);
    EndDraw();

    send_message((ctrl_t*)win, MSG_DRAW_CLIENT, 0, 0);

    if(win->win_state == FULLSCREEN)
    {
        need_update=0;
        return;
    };

    blit_caption(&win->caption);
    blit_panel(&win->panel);

//    blit_client(win);
    need_update=0;
};

static void do_sys_mouse(window_t *win)
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
    pos = get_mouse_pos();

    if(pos.val != old_pos.val)
    {
        mouse_action = 0x80000000;
        old_pos = pos;
    };
//    printf("pos x%d y%d\n", pos.x, pos.y);

    buttons = get_mouse_buttons();
    wheels = get_mouse_wheels();

    if( wheels & 0xFFFF){
        wheels = (short)wheels>0 ? MSG_WHEELDOWN : MSG_WHEELUP;
        send_mouse_message(win, wheels);
    }

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


void  run_window(window_t *win)
{
    int ev;
    oskey_t   key;

//    buttons   = get_mouse_buttons();
//    wheels    = get_mouse_wheels();
    realtime  = get_tick_count();
    exp_time  = -1;

    while(1)
    {
        wait_time = exp_time - realtime;

        ev = wait_for_event(wait_time);

        realtime = get_tick_count();

//        if(exp_time < realtime)
//            exp_time = update_timers(realtime);

        switch(ev)
        {
            case MSG_PAINT:
                do_sys_draw(win);
                continue;

            case 2:
                printf("key pressed\n");
                key = get_key();
                if( key.state == 0)
                    send_message((ctrl_t*)win, ev, 0, key.code);
                continue;

            case 3:
                printf("Button pressed\n");
                continue;

            case 6:
                do_sys_mouse(win);
                continue;

            default:
                printf("event %d\n", ev);
                continue;
        };
    };
}

void render_time(void *render);

void  run_render(window_t *win, void *render)
{
    int ev;
    oskey_t   key;
    int button;

    realtime  = get_tick_count();
    exp_time  = -1;

    while(win->win_command != WIN_CLOSED)
    {
        wait_time = exp_time - realtime;

        ev = check_os_event();

        realtime = get_tick_count();

//        if(exp_time < realtime)
//            exp_time = update_timers(realtime);

        switch(ev)
        {
            case MSG_PAINT:
                do_sys_draw(win);
                break;

            case 2:
                key = get_key();
                if( key.state == 0)
                    send_message((ctrl_t*)win, ev, 0, key.code);
                break;

            case 3:
                button = get_os_button();
                if(button == 1)
                    win->win_command = WIN_CLOSED;
                break;

            case 6:
                do_sys_mouse(win);
                break;

            default:
                break;
        };

        render_time(render);
    };
};



extern unsigned char res_cursor_ns[];
extern unsigned char res_cursor_we[];
extern unsigned char res_cursor_nwse[];
extern unsigned char res_cursor_nesw[];

int init_resources()
{
    cursor_ns   = load_cursor(res_cursor_ns, LOAD_FROM_MEM);
    cursor_we   = load_cursor(res_cursor_we, LOAD_FROM_MEM);
    cursor_nwse = load_cursor(res_cursor_nwse, LOAD_FROM_MEM);
    cursor_nesw = load_cursor(res_cursor_nesw, LOAD_FROM_MEM);

    win_font =  init_fontlib();

    return 1;
}

int  fini_winlib()
{
    int ret;

    ret =  destroy_cursor(cursor_nesw);
    ret |= destroy_cursor(cursor_nwse);
    ret |= destroy_cursor(cursor_we);
    ret |= destroy_cursor(cursor_ns);

    return ret;
};



void  init_winlib(void)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(40), "b"(0xC0000027));
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(66), "b"(1),"c"(1));

    init_resources();
    list_initialize(&timers);
};

//ctx_t *get_window_ctx()
//{
//    return &Window.client_ctx;
//};

void update_rect(ctrl_t *ctrl)
{
    int ctx_w, ctx_h;
    int src_x, src_y;

    src_x = ctrl->rc.l - ctrl->ctx->offset_x;
    src_y = ctrl->rc.t - ctrl->ctx->offset_y;

    ctx_w = ctrl->parent->w;
    ctx_h = ctrl->parent->h;

    Blit(ctrl->ctx->pixmap->data, ctrl->rc.l, ctrl->rc.t, src_x, src_y,
         ctrl->w, ctrl->h, ctx_w, ctx_h, ctrl->ctx->pixmap->pitch);

//    need_update++;
};


void Blit(void *bitmap, int dst_x, int dst_y,
                        int src_x, int src_y, int w, int h,
                        int src_w, int src_h, int stride)
{
    volatile struct blit_call bc;

    bc.dstx = dst_x;
    bc.dsty = dst_y;
    bc.w    = w;
    bc.h    = h;
    bc.srcx = src_x;
    bc.srcy = src_y;
    bc.srcw = src_w;
    bc.srch = src_h;
    bc.stride = stride;
    bc.bitmap = bitmap;

    __asm__ __volatile__(
    "int $0x40"
    ::"a"(73),"b"(0x20),"c"(&bc.dstx));

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

ctrl_t *capture_mouse(ctrl_t *newm)
{
    ctrl_t *old = mouse_capture;

    mouse_capture = newm;

    __asm__ __volatile__(
    "int $0x40"
    ::"a"(40), "b"(0x80000027));

    return old;
}

void release_mouse(void)
{
    mouse_capture = NULL;
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(40), "b"(0xC0000027));
}
