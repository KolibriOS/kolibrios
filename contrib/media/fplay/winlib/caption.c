
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "winlib.h"

#define CAPTION_CORNER_W    8

extern int res_caption_left[];
extern int res_caption_right[];
extern int res_caption_body[];

extern int res_close_btn[];
extern int res_close_btn_hl[];
extern int res_close_btn_pressed[];

extern int res_minimize_btn[];
extern int res_minimize_btn_hl[];
extern int res_minimize_btn_pressed[];

extern int res_full_btn[];
extern int res_full_btn_hl[];
extern int res_full_btn_pressed[];

extern uint32_t main_cursor;

void update_caption_size(window_t *win);
void window_update_layout(window_t *win);
int draw_text_ext(void *pixmap, uint32_t pitch, int face, char *text, rect_t *rc, int color);

int caption_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2);


int init_caption(window_t *win)
{
    button_t  *btn;

    caption_t *cpt = &win->caption;
    ctx_t     *ctx = &cpt->ctx;

    link_initialize(&cpt->ctrl.link);
    list_initialize(&cpt->ctrl.child);

    cpt->ctrl.handler = caption_proc;
    cpt->ctrl.parent  = (ctrl_t*)win;

    cpt->text = win->caption_txt;

    ctx->pixmap_data = user_alloc(1920*CAPTION_HEIGHT*4);
    ctx->pixmap_pitch= 1920*4;
    ctx->offset_x = 0;
    ctx->offset_y = 0;

    cpt->ctrl.ctx = ctx;

    btn = create_button(NULL, ID_CLOSE,0,5,18,18,(ctrl_t*)cpt);
    cpt->close_btn = btn;

    btn->img_default = res_close_btn;
    btn->img_hilite  = res_close_btn_hl;
    btn->img_pressed = res_close_btn_pressed;

    btn = create_button(NULL, ID_MINIMIZE,0,5,18,18,(ctrl_t*)cpt);
    cpt->minimize_btn = btn;

    btn->img_default = res_minimize_btn;
    btn->img_hilite  = res_minimize_btn_hl;
    btn->img_pressed = res_minimize_btn_pressed;

    btn = create_button(NULL, ID_FULLSCREEN,0,5,18,18,(ctrl_t*)cpt);
    cpt->full_btn = btn;

    btn->img_default = res_full_btn;
    btn->img_hilite  = res_full_btn_hl;
    btn->img_pressed = res_full_btn_pressed;

    update_caption_size(win);

    return 1;
};


void update_caption_size(window_t *win)
{
    caption_t *cpt = &win->caption;
    ctx_t     *ctx = &cpt->ctx;

    ctx->pixmap_pitch = win->w * 4;

    cpt->ctrl.rc.l    = 0;
    cpt->ctrl.rc.t    = 0;
    cpt->ctrl.rc.r    = win->w;
    cpt->ctrl.rc.b    = CAPTION_HEIGHT;
    cpt->ctrl.w       = win->w;
    cpt->ctrl.h       = CAPTION_HEIGHT;
    win->client.t     = CAPTION_HEIGHT;

    cpt->close_btn->ctrl.rc.l = win->w - 27;
    cpt->close_btn->ctrl.rc.r = cpt->close_btn->ctrl.rc.l +
                           cpt->close_btn->ctrl.w;

    cpt->minimize_btn->ctrl.rc.l = win->w - 27 - 18 - 5;
    cpt->minimize_btn->ctrl.rc.r = cpt->minimize_btn->ctrl.rc.l +
                           cpt->minimize_btn->ctrl.w;

    cpt->full_btn->ctrl.rc.l = win->w - 27 - 18 -18 - 5 - 5;
    cpt->full_btn->ctrl.rc.r = cpt->full_btn->ctrl.rc.l +
                           cpt->full_btn->ctrl.w;
};

extern int win_font;

void draw_caption(caption_t *cpt)
{
    ctx_t  *ctx = &cpt->ctx;
    int    *pixmap, *src;
    rect_t  rc;
    int     i, j, w;

    blit_raw(ctx, res_caption_left, 0, 0,
             CAPTION_CORNER_W, CAPTION_HEIGHT, CAPTION_CORNER_W*4);

    w = cpt->ctrl.w - (2*CAPTION_CORNER_W);
    if( w > 0)
    {
        pixmap = (int*)ctx->pixmap_data;
        pixmap+= CAPTION_CORNER_W;
        src = res_caption_body;

        for(i = 0; i < CAPTION_HEIGHT; i++)
        {
            for(j = 0; j < w; j++)
                pixmap[j] = src[i];
            pixmap+= ctx->pixmap_pitch/4;
        }
    };

    blit_raw(ctx,res_caption_right, cpt->ctrl.w - CAPTION_CORNER_W, 0,
             CAPTION_CORNER_W, CAPTION_HEIGHT,CAPTION_CORNER_W*4);

    rc.l = 8;
    rc.t = 0;
    rc.r = cpt->ctrl.w - 27 - 18 - 18 - 5 - 5 - 8;
    rc.b = 18;

    draw_text_ext(ctx->pixmap_data, ctx->pixmap_pitch, win_font, cpt->text, &rc, 0xFFFFFFFF);

    ctrl_t *child;
    child  = (ctrl_t*)cpt->ctrl.child.next;

    while( &child->link != &cpt->ctrl.child)
    {
        send_message(child, 1, 0, 0);
        child = (ctrl_t*)child->link.next;
    };
};


int caption_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    caption_t *cpt = (caption_t*)ctrl;
    window_t *win  = (window_t*)ctrl->parent;

    ctrl_t *child;
    int x, y;

    x = ((pos_t)arg2).x;
    y = ((pos_t)arg2).y;

    switch( msg )
    {
        case 1:
            break;


        case MSG_MOUSEMOVE:
            child = get_child(ctrl, x, y);
            if( win->child_over )
            {
                if(child == win->child_over)
                    send_message(child, msg, 0, arg2);
                else
                    send_message(win->child_over, MSG_MOUSELEAVE, 0, arg2);
            };

            win->child_over = child;
            if( child )
            {
                send_message(child, MSG_MOUSEENTER, 0, arg2);
                send_message(child,msg,0,arg2);
            }
            else if(main_cursor != 0)
            {
                set_cursor(0);
                main_cursor = 0;
            }
            break;


        case MSG_COMMAND:
            switch((short)arg1)
            {
                case ID_CLOSE:
                    win->win_command = WIN_CLOSED;
                    break;

                case ID_MINIMIZE:
                    __asm__ __volatile__(
                    "int $0x40"
                    ::"a"(18),"b"(10));
                    win->win_state = MINIMIZED;
                    send_message((ctrl_t*)win, MSG_SIZE, 0, 0);
                    break;
                case ID_FULLSCREEN:
                {
                    int screensize;

                    win->saved = win->rc;
                    win->saved_state = win->win_state;
                    screensize = GetScreenSize();
                    __asm__ __volatile__(
                    "int $0x40"
                    ::"a"(67), "b"(0), "c"(0),
                    "d"((screensize >> 16)-1),"S"((screensize & 0xFFFF)-1) );
                    win->win_state = FULLSCREEN;
                    window_update_layout(win);
                };
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



void blit_caption(caption_t *cpt)
{
//    printf("%s w:%d h:%d stride: %d\n",__FUNCTION__,
//            cpt->ctrl.w, cpt->ctrl.h, cpt->ctx.stride);


    Blit(cpt->ctx.pixmap_data, 0, 0, 0, 0, cpt->ctrl.w, cpt->ctrl.h,
         cpt->ctrl.w, cpt->ctrl.h, cpt->ctx.pixmap_pitch);
};

