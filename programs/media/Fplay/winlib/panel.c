#include "system.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "winlib.h"

#define PANEL_HEIGHT        55
#define PANEL_CORNER_W      16
#define FRAME_WIDTH          7

#define ID_PLAY           100

extern uint32_t main_cursor;

extern int res_panel_left[];
extern int res_panel_right[];
extern int res_panel_body[];

extern int res_play_btn[];
extern int res_play_btn_pressed[];

extern int res_pause_btn[];
extern int res_pause_btn_pressed[];

//extern int res_minimize_btn[];
//extern int res_minimize_btn_hl[];
//extern int res_minimize_btn_pressed[];

void update_panel_size(window_t *win);

int panel_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2);

int init_panel(window_t *win)
{
    button_t     *btn;
    progress_t   *prg;

    panel_t *panel = &win->panel;
    ctx_t   *ctx = &panel->ctx;

    link_initialize(&panel->ctrl.link);
    list_initialize(&panel->ctrl.child);

    panel->ctrl.handler = panel_proc;
    panel->ctrl.parent  = (ctrl_t*)win;

    ctx->pixmap = user_alloc(1920*PANEL_HEIGHT*4);
    if(!ctx->pixmap)
    {
        printf("not enough memory for caption bitmap\n");
        return 0;
    };

//    printf("win_w %d win_h %d\n", win->w, win->h);

    ctx->stride = win->w*4;
    panel->ctrl.ctx = ctx;

    btn = create_button(NULL, ID_PLAY,0,19,32,32,&panel->ctrl);
    panel->play_btn = btn;

    btn->img_default = res_pause_btn;
    btn->img_hilite  = res_pause_btn;
    btn->img_pressed = res_pause_btn_pressed;

    prg = create_progress(NULL,101,0,4,0,8,&panel->ctrl);
    panel->prg = prg;

//    btn = create_button(NULL, ID_MINIMIZE,0,5,16,18,(ctrl_t*)cpt);
//    cpt->minimize_btn = btn;

//    btn->img_default = res_minimize_btn;
//    btn->img_hilite  = res_minimize_btn_hl;
//    btn->img_pressed = res_minimize_btn_pressed;

    update_panel_size(win);

    return 1;
};

void update_panel_size(window_t *win)
{
    panel_t *panel = &win->panel;

    int old_size;
    int new_size;
    int stride;

    old_size = panel->ctx.stride * PANEL_HEIGHT;
    old_size = (old_size+4095) & ~4095;

    stride = win->w*4;

    new_size = stride * PANEL_HEIGHT;
    new_size = (new_size+4095) & ~4095;

    if( new_size < old_size)
        user_unmap(panel->ctx.pixmap, new_size, old_size-new_size);

    panel->ctx.stride = stride;
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

    panel->play_btn->rc.l = win->w/2 - 16;
    panel->play_btn->rc.t = panel->ctrl.rc.t+19;
    panel->play_btn->rc.r = panel->play_btn->rc.l + panel->play_btn->w;
    panel->play_btn->rc.b = panel->play_btn->rc.t + panel->play_btn->h;

    panel->prg->ctrl.rc.l = 8;
    panel->prg->ctrl.rc.t = panel->ctrl.rc.t+7;
    panel->prg->ctrl.rc.r = panel->ctrl.rc.r-8;
    panel->prg->ctrl.rc.b = panel->prg->ctrl.rc.t+8;
    panel->prg->ctrl.w = panel->prg->ctrl.rc.r -
                        panel->prg->ctrl.rc.l;

    panel->prg->ctrl.h = panel->prg->ctrl.rc.b -
                         panel->prg->ctrl.rc.t;

//    cpt->minimize_btn->rc.l = win->w - 25 - 16 - 5;
//    cpt->minimize_btn->rc.r = cpt->minimize_btn->rc.l +
//                           cpt->minimize_btn->w;

};


void draw_panel(panel_t *panel)
{
    int *pixmap, *src;
    int  i, j, w;

    pixmap = panel->ctx.pixmap;
    src = res_panel_left;

    for(i=0; i < PANEL_HEIGHT; i++)
    {
        for(j=0; j < PANEL_CORNER_W; j++)
            pixmap[j] = src[j];
        pixmap+= panel->ctx.stride/4;
        src+= PANEL_CORNER_W;
    };

    w = panel->ctrl.w - (2*PANEL_CORNER_W);
    if( w > 0)
    {
        pixmap = panel->ctx.pixmap;
        pixmap+= PANEL_CORNER_W;
        src = res_panel_body;

        for(i = 0; i < PANEL_HEIGHT; i++)
        {
            for(j = 0; j < w; j++)
                pixmap[j] = src[i];
            pixmap+= panel->ctx.stride/4;
        }
    };

    pixmap = panel->ctx.pixmap;
    pixmap+= panel->ctrl.w - PANEL_CORNER_W;

    src = res_panel_right;

    for(i = 0; i < PANEL_HEIGHT; i++)
    {
        for(j = 0; j < PANEL_CORNER_W; j++)
            pixmap[j] = src[j];
        pixmap+= panel->ctx.stride/4;
        src+= PANEL_CORNER_W;
    };

    ctrl_t *child;
    child  = (ctrl_t*)panel->ctrl.child.next;

    while( &child->link != &panel->ctrl.child)
    {
        send_message(child, 1, 0, 0);
        child = (ctrl_t*)child->link.next;
    };
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

        case MSG_COMMAND:
            switch((short)arg1)
            {
                case ID_PLAY:
                case 101: 
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
//    printf("%s w:%d h:%d stride: %d\n",__FUNCTION__,
//            cpt->ctrl.w, cpt->ctrl.h, cpt->ctx.stride);

    Blit(panel->ctx.pixmap, panel->draw.l, panel->draw.t,
         0, 0, panel->ctrl.w, panel->ctrl.h,
         panel->ctrl.w, panel->ctrl.h, panel->ctx.stride);
};

