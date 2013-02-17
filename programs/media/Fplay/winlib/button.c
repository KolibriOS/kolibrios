
#include "system.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "winlib.h"

extern int res_level[];
extern int res_slider[];
extern int res_vol_slider[];
extern int res_progress_bar[];
extern int res_prg_level[];

extern ctrl_t  *mouse_capture;
uint32_t main_cursor;

static int button_proc(ctrl_t *btn, uint32_t msg, uint32_t arg1, uint32_t arg2);
static int spinbtn_proc(ctrl_t *btn, uint32_t msg, uint32_t arg1, uint32_t arg2);

ctrl_t *create_control(size_t size, int id, int x, int y,
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


button_t *create_button(char *caption, int id, int x, int y,
                        int w, int h, ctrl_t *parent)
{
    button_t  *btn;
    int        len;

    if( !parent )
        return NULL;

    btn = (button_t*)create_control(sizeof(button_t), id, x, y, w, h, parent);
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

    btn->img_default = NULL;
    btn->img_hilite  = NULL;
    btn->img_pressed = NULL;

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
    int *src;
    ctx_t *ctx;
    int x, y;

    ctx = btn->ctrl.ctx;

    x = btn->ctrl.rc.l - ctx->offset_x;
    y = btn->ctrl.rc.t - ctx->offset_y;

    src = btn->img_default;

    if(btn->state & bPressed)
        src = btn->img_pressed;
    else if(btn->state & bHighlight)
        src = btn->img_hilite;

    blit_raw(ctx, src, x, y, btn->ctrl.w, btn->ctrl.h, btn->ctrl.w*4);

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
            send_message(&btn->ctrl, MSG_PAINT, 0, 0);
            break;

        case MSG_MOUSELEAVE:
//            printf("mouse leave\n");
            if( (ctrl_t*)btn != mouse_capture) {
                btn->state &= ~bHighlight;
                send_message(&btn->ctrl, MSG_PAINT, 0, 0);
            };
            break;

        case MSG_LBTNDOWN:
        case MSG_LBTNDBLCLK:
//            printf("push button\n");
            capture_mouse((ctrl_t*)btn);
            btn->state|= bPressed;
            send_message(&btn->ctrl, MSG_PAINT, 0, 0);
            break;

        case MSG_LBTNUP:

 //           printf("button action\n");
            if(btn->state & bPressed)
                action = MSG_COMMAND;

            release_mouse();

            x = ((pos_t)arg2).x;
            y = ((pos_t)arg2).y;

            if( pt_in_rect( &btn->ctrl.rc, x, y) )
                state = bHighlight;
            else
                state = 0;

            if(action)
                send_message(btn->ctrl.parent,MSG_COMMAND,btn->ctrl.id,(int)btn);

            btn->state = state;
            send_message(&btn->ctrl, MSG_PAINT, 0, 0);
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
                send_message(&btn->ctrl, MSG_PAINT, 0, 0);
            };

            if( (ctrl_t*)btn != mouse_capture)
                return 0;

            x = ((pos_t)arg2).x;
            y = ((pos_t)arg2).y;

            old_state = btn->state;

            if( pt_in_rect(&btn->ctrl.rc, x, y) )
                btn->state |= bPressed;
            else
                btn->state &= ~bPressed;

            if( old_state ^ btn->state)
                send_message(&btn->ctrl, MSG_PAINT, 0, 0);
    }
    return 0;
};


int draw_progress(progress_t *prg, int background)
{
    int *pixmap, *src;
    ctx_t *ctx;
    int i, j;
    int x, y;
    rect_t rc = prg->ctrl.rc;

    int len = prg->ctrl.w;

    ctx = prg->ctrl.ctx;

    x = prg->ctrl.rc.l - ctx->offset_x;
    y = prg->ctrl.rc.t - ctx->offset_y;

    if( background )
    {
        src = res_progress_bar;

        pixmap = (int*)ctx->pixmap->data;
        pixmap+= y * ctx->pixmap->pitch/4 + x;

        for(i=0; i < 10; i++)
        {
            for(j = 0; j < len; j++)
                pixmap[j] = *src;

            pixmap+= ctx->pixmap->pitch/4;
            src++;
        };
    };


    len = prg->current*prg->ctrl.w/(prg->max - prg->min);

    src = res_prg_level;

    pixmap = (int*)ctx->pixmap->data;
    pixmap+= y*ctx->pixmap->pitch/4 + x;

    for(i=0; i < prg->ctrl.h ;i++)
    {
        for(j=0; j < len; j++)
            pixmap[j] = *src;
        pixmap+= ctx->pixmap->pitch/4;
        src++;
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
            draw_progress(prg, 1);
            update_rect(ctrl);
            break;

        case MSG_LBTNDOWN:
            prg->pos = ((pos_t)arg2).x - ctrl->rc.l;
            send_message(ctrl->parent,MSG_COMMAND,ctrl->id,(int)ctrl);
            break;

        case PRG_PROGRESS:
            draw_progress(prg, 0);
            update_rect(ctrl);
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

    prg = (progress_t*)create_control(sizeof(progress_t), id, x, y, w, h, parent);

    prg->ctrl.handler = prg_proc;

    prg->min        = 0;
    prg->max        = 1;
    prg->current    = 0;
    prg->pos        = 0;

    return prg;
};

int draw_level(level_t *lvl)
{
    int *pixmap;
    ctx_t *ctx;
    int i, j;
    int x, y;

    int len;
    double level;

    ctx = lvl->ctrl.ctx;

    x = lvl->ctrl.rc.l - ctx->offset_x;
    y = lvl->ctrl.rc.t - ctx->offset_y;

    level = (log2(lvl->current+1)-7)*12 + lvl->vol/50 ;

    len = level;

    if(len < 0)
        len = 0;
    if(len > 96)
        len = 96;

    pixmap = (int*)ctx->pixmap->data;

    pixmap+=  y*ctx->pixmap->pitch/4 + x;

    for(i=0; i < 10; i++)
    {
        for(j = 0; j < 96; j++)
           pixmap[j] = 0xFF1C1C1C;
           pixmap+= ctx->pixmap->pitch/4;
    };

    blit_raw(ctx, lvl->img_level, x, y, len, 10, 96*4);

    return 0;
};


int lvl_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    level_t *lvl = (level_t*)ctrl;
//    int pos;

    switch( msg )
    {
        case MSG_PAINT:
            if(lvl->visible)
            {
                draw_level(lvl);
                update_rect(ctrl);
            };
            break;

//        case MSG_LBTNDOWN:
//            prg->pos = ((pos_t)arg2).x - ctrl->rc.l;
//            send_message(ctrl->parent,MSG_COMMAND,ctrl->id,(int)ctrl);
//            break;

        default:
            break;
    }
    return 0;
};

level_t    *create_level(char *caption, int id, int x, int y,
                         int w, int h, ctrl_t *parent)
{
    level_t  *lvl;

    if( !parent )
        return NULL;

    lvl = (level_t*)create_control(sizeof(level_t), id, x, y, w, h, parent);
    lvl->ctrl.handler = lvl_proc;

    lvl->min          = 0;
    lvl->max          = 1;
    lvl->current      = 0;
    lvl->pos          = 0;
    lvl->visible      = 0;
    lvl->img_level    = res_level;

    return lvl;
};


int draw_slider(slider_t *sld)
{
    int *pixmap;
    ctx_t *ctx;
    int i, j;
    int x, y;

    int32_t len;
    double level;

    ctx = sld->ctrl.ctx;

    x = sld->ctrl.rc.l - ctx->offset_x;
    y = sld->ctrl.rc.t - ctx->offset_y;

    len = 96 + 12;

    pixmap = (int*)ctx->pixmap->data;
    pixmap+=  y*ctx->pixmap->pitch/4 + x;

    for(i=0; i < 11; i++)
    {
        for(j = 0; j < len; j++)
           pixmap[j] = 0xFF1C1C1C;
           pixmap+= ctx->pixmap->pitch/4;
    };

    blit_raw(ctx, sld->img_vol_slider, x+6, y+4, 96, 4, 96*4);

    blit_raw(ctx, res_slider, x+sld->pos, y, 12, 11, 12*4);

    return 0;
};


int sld_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    slider_t *sld = (slider_t*)ctrl;
    int pos;

    switch( msg )
    {
        case MSG_PAINT:
            draw_slider(sld);
            update_rect(ctrl);
            break;

        case MSG_LBTNDOWN:
            capture_mouse(ctrl);
            sld->mode = 1;

            pos = ((pos_t)arg2).x - ctrl->rc.l - 6;
            if( pos < 0 )
                pos = 0;
            else if(pos > 96)
                pos = 96;
            if( sld->pos != pos)
            {
                sld->pos = pos;
                send_message(ctrl->parent,MSG_COMMAND,ctrl->id,(int)ctrl);
            };
           break;

        case MSG_LBTNUP:
            if(sld->mode)
            {
                release_mouse();
                sld->mode = 0;
            };
            break;

        case MSG_MOUSEMOVE:
            if(sld->mode)
            {
                pos = ((pos_t)arg2).x - ctrl->rc.l - 6;
                if( pos < 0 )
                    pos = 0;
                else if(pos > 96)
                    pos = 96;
                if( sld->pos != pos)
                {
                    sld->pos = pos;
//                printf("slider pos %d\n", sld->pos);
                    send_message(ctrl->parent,MSG_COMMAND,ctrl->id,(int)ctrl);
                }
            };
            break;

        case MSG_MOUSEENTER:
            panel_set_layout(ctrl->parent, 1);
//            printf("level on\n");
            break;

        case MSG_MOUSELEAVE:
            panel_set_layout(ctrl->parent, 0);
//            printf("level off\n");
            break;


        default:
            break;
    }
    return 0;
};


slider_t  *create_slider(char *caption, int id, int x, int y,
                         int w, int h, ctrl_t *parent)
{

    slider_t  *sld;

    if( !parent )
        return NULL;

    sld = (slider_t*)create_control(sizeof(slider_t), id, x, y, w, h, parent);
    sld->ctrl.handler = sld_proc;

    sld->min     = -5000;
    sld->max     = 0;
    sld->current = 0;
    sld->pos     = 60;
    sld->mode    = 0;
    sld->img_vol_slider    = res_vol_slider;

    return sld;
};

