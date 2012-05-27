
#include "system.h"
#include <stdlib.h>
#include <string.h>
#include "winlib.h"


extern ctrl_t  *mouse_capture;
uint32_t main_cursor;

static int button_proc(ctrl_t *btn, uint32_t msg, uint32_t arg1, uint32_t arg2);
static int spinbtn_proc(ctrl_t *btn, uint32_t msg, uint32_t arg1, uint32_t arg2);


button_t *create_button(char *caption, int id, int x, int y,
                        int w, int h, ctrl_t *parent)
{
    button_t  *btn;
    int        len;

    if( !parent )
        return NULL;

    btn = (button_t*)malloc(sizeof(button_t));

    link_initialize(&btn->link);
    list_initialize(&btn->child);

    btn->handler = button_proc;
    btn->parent  = parent;

    btn->ctx     = parent->ctx;
    btn->id      = id;
    btn->style   = 0;

    btn->rc.l   = x;
    btn->rc.t   = y ;

    btn->rc.r   = x + w;
    btn->rc.b   = y + h;

    btn->w      = w;
    btn->h      = h;

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

    btn->img_default = NULL;
    btn->img_hilite  = NULL;
    btn->img_pressed = NULL;

    list_append(&btn->link, &parent->child);

    return btn;
};

#if 0
int draw_button(button_t *btn)
{
    void *bitmap;

    bitmap = btn->img_default;

    if(btn->state & bPressed)
        bitmap = btn->img_pressed;
    else if(btn->state & bHighlight)
        bitmap = btn->img_hilite;

    if( bitmap )
        draw_bitmap(bitmap, btn->rc.l, btn->rc.t, btn->w, btn->h);

    if( btn->caption && btn->capt_len)
    {
        int txt_w;
        int txt_x, txt_y;
        txt_w = btn->capt_len*8-2;

        txt_x = btn->rc.l + 1 + (btn->w - txt_w)/2;
        txt_y = btn->rc.t + 9;

        if(btn->state & bPressed){
            txt_x++;
            txt_y++;
        };
        draw_text(btn->caption, txt_x, txt_y, btn->capt_len, 0x10000000);
    };
    return 0;
};
#endif

int draw_button_cairo(button_t *btn)
{
    int *pixmap, *src;
    ctx_t *ctx;
    int i, j;
    int x, y;

    ctx = btn->ctx;

    x = btn->rc.l - ctx->offset_x;
    y = btn->rc.t - ctx->offset_y;

    pixmap = ctx->pixmap;

    pixmap+=  y*ctx->stride/4 + x;

    src = btn->img_default;

    if(btn->state & bPressed)
        src = btn->img_pressed;
    else if(btn->state & bHighlight)
        src = btn->img_hilite;

    for(i=0; i < btn->h ;i++)
    {
        for(j=0; j<btn->w; j++)
            pixmap[j] = src[j];
        pixmap+= ctx->stride/4;
        src+=btn->w;
    };

    return 0;
};


int draw_spin_cairo(button_t *btn)
{
    void *ctx;

    return 0;
};


int button_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    int  x, y;
    int  state;
    int  old_state;
    int  action=0;

    button_t *btn = (button_t*)ctrl;

    switch( msg )
    {
        case MSG_PAINT:
            draw_button_cairo(btn);
            update_rect((ctrl_t*)btn);
            break;

        case MSG_MOUSEENTER:
//            printf("mouse enter\n");
            btn->state|= bHighlight;
            send_message(btn, MSG_PAINT, 0, 0);
            break;

        case MSG_MOUSELEAVE:
//            printf("mouse leave\n");
            if( (ctrl_t*)btn != mouse_capture) {
                btn->state &= ~bHighlight;
                send_message(btn, MSG_PAINT, 0, 0);
            };
            break;

        case MSG_LBTNDOWN:
        case MSG_LBTNDBLCLK:
//            printf("push button\n");
            capture_mouse((ctrl_t*)btn);
            btn->state|= bPressed;
            send_message(btn, MSG_PAINT, 0, 0);
            break;

        case MSG_LBTNUP:

 //           printf("button action\n");
            if(btn->state & bPressed)
                action = MSG_COMMAND;

            release_mouse();

            x = ((pos_t)arg2).x;
            y = ((pos_t)arg2).y;

            if( pt_in_rect( &btn->rc, x, y) )
                state = bHighlight;
            else
                state = 0;

            if(action)
                send_message(btn->parent,MSG_COMMAND,btn->id,(int)btn);

            btn->state = state;
            send_message(btn, MSG_PAINT, 0, 0);
            break;

        case MSG_MOUSEMOVE:

            if(main_cursor != 0)
            {
                set_cursor(0);
                main_cursor = 0;
            }

            if( ! (btn->state & bHighlight))
            {
                btn->state|= bHighlight;
                send_message(btn, MSG_PAINT, 0, 0);
            };

            if( (ctrl_t*)btn != mouse_capture)
                return 0;

            x = ((pos_t)arg2).x;
            y = ((pos_t)arg2).y;

            old_state = btn->state;

            if( pt_in_rect(&btn->rc, x, y) )
                btn->state |= bPressed;
            else
                btn->state &= ~bPressed;

            if( old_state ^ btn->state)
                send_message(btn, MSG_PAINT, 0, 0);
    }
    return 0;
};


int draw_progress(progress_t *prg)
{
    int *pixmap, src;
    ctx_t *ctx;
    int i, j;
    int x, y;

    int len;

    ctx = prg->ctrl.ctx;

    x = prg->ctrl.rc.l - ctx->offset_x;
    y = prg->ctrl.rc.t - ctx->offset_y;


    len = prg->current*prg->ctrl.w/(prg->max - prg->min);
    pixmap = ctx->pixmap;

    pixmap+=  y*ctx->stride/4 + x;

    src = 0x32ebfb; //btn->img_default;

    for(i=0; i < prg->ctrl.h ;i++)
    {
        for(j=0; j < len; j++)
            pixmap[j] = src;
        pixmap+= ctx->stride/4;
    };

    return 0;
};


int prg_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    progress_t *prg = (progress_t*)ctrl;
    int pos;
    
    switch( msg )
    {
        case MSG_PAINT:
            draw_progress(prg);
            update_rect(ctrl);
            break;
            
        case MSG_LBTNDOWN:
            prg->pos = ((pos_t)arg2).x - ctrl->rc.l;
            send_message(ctrl->parent,MSG_COMMAND,ctrl->id,(int)ctrl);
            break;
            
        default:
            break;
    }
    return 0;
};


progress_t *create_progress(char *caption, int id, int x, int y,
                        int w, int h, ctrl_t *parent)
{
    progress_t  *prg;
    int        len;

    if( !parent )
        return NULL;

    prg = (progress_t*)malloc(sizeof(progress_t));

    link_initialize(&prg->ctrl.link);
    list_initialize(&prg->ctrl.child);

    prg->ctrl.handler = prg_proc;
    prg->ctrl.parent  = parent;

    prg->ctrl.ctx     = parent->ctx;
    prg->ctrl.id      = id; 
    
    prg->ctrl.rc.l   = x;
    prg->ctrl.rc.t   = y ;

    prg->ctrl.rc.r   = x + w;
    prg->ctrl.rc.b   = y + h;

    prg->ctrl.w      = w;
    prg->ctrl.h      = h;

    prg->min        = 0;
    prg->max        = 1;
    prg->current    = 0;
    prg->pos        = 0;

    list_append(&prg->ctrl.link, &parent->child);

    return prg;
};

