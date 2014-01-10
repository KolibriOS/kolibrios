
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "winlib.h"

#define CAPTION_CORNER_W    8
#define FRAME_WIDTH         7

extern uint32_t  main_cursor;

extern uint32_t  cursor_ns;
extern uint32_t  cursor_we;
extern uint32_t  cursor_nwse;
extern uint32_t  cursor_nesw;

extern ctrl_t  *mouse_capture;

static int frame_proc(ctrl_t *ctrl, uint32_t msg,
                      uint32_t arg1, uint32_t arg2);

void adjust_frame(window_t *win)
{
    frame_t *fr = &win->frame;

    fr->left.l  = 0;
    fr->left.t  = win->client.t;
    fr->left.r  = FRAME_WIDTH;
    fr->left.b  = win->h-FRAME_WIDTH;

    fr->right.l = win->w - FRAME_WIDTH;
    fr->right.t = win->client.t;
    fr->right.r = win->w;
    fr->right.b = win->h-FRAME_WIDTH;

    fr->bottom.l = 0;
    fr->bottom.t = win->h - FRAME_WIDTH;
    fr->bottom.r = win->w;
    fr->bottom.b = win->h;

    win->client.l = FRAME_WIDTH;
    win->client.r = win->w - FRAME_WIDTH;
//    win->client.b = win->h - FRAME_WIDTH;
//    printf("Left: l:%d t:%d r:%d b:%d\n",
//            fr->left.l,fr->left.t,fr->left.r,fr->left.b);
//    printf("Left: l:%d t:%d r:%d b:%d\n",
//            fr->right.l,fr->right.t,fr->right.r,fr->right.b);
//    printf("Left: l:%d t:%d r:%d b:%d\n",
//            fr->bottom.l,fr->bottom.t,fr->bottom.r,fr->bottom.b);

};

void init_frame(window_t *win)
{
    frame_t *fr = &win->frame;

    link_initialize(&fr->link);
    list_initialize(&fr->child);

    fr->handler = frame_proc;
    fr->parent  = (ctrl_t*)win;

    adjust_frame(win);
};


extern int res_border_left[];
extern int res_border_right[];

int draw_frame(window_t *win)
{
    int *pixmap, *src;
    int  i, j;

    int w;

    frame_t *fr = &win->frame;

    pixmap = (int*)win->ctx->pixmap->data;
    pixmap+= CAPTION_HEIGHT*win->w;
    src = res_border_left;

    for(fr->left.t; i < fr->left.b; i++)
    {
        for(j = 0; j < FRAME_WIDTH; j++)
            pixmap[j] = src[j];

        pixmap+= win->ctx->pixmap->pitch/4;
    };


    pixmap = (int*)win->ctx->pixmap->data;
    pixmap+= (CAPTION_HEIGHT+1)*win->w - FRAME_WIDTH;
    src = res_border_right;

    for(i=fr->right.t; i < fr->right.b; i++)
    {
        for(j = 0; j < FRAME_WIDTH; j++)
            pixmap[j] = src[j];

        pixmap+= win->ctx->pixmap->pitch/4;
    };

    pixmap = (int*)win->ctx->pixmap->data;

    pixmap+= fr->bottom.t * win->w;

    for(i=0; i < FRAME_WIDTH; i++)
    {
        for(j = 0; j < win->w; j++)
            pixmap[j] = 0x808080;

        pixmap+= win->ctx->pixmap->pitch/4;
    };

    ctrl_t *child;
    child  = (ctrl_t*)fr->child.next;

    while( &child->link != &fr->child)
    {
        send_message(child, 1, 0, 0);
        child = (ctrl_t*)child->link.next;
    };

    return 0;
};

int frame_proc(ctrl_t *ctrl, uint32_t msg, uint32_t arg1, uint32_t arg2)
{
    static  pos_t spos;
    static  track_mode;

    uint32_t cursor;
    ctrl_t   *child;

    frame_t  *fr  = (frame_t*)ctrl;
    window_t *win = (window_t*)fr->parent;
    rect_t  *rc = NULL;
    int  x, y;

    if(win->win_state == FULLSCREEN)
        return 0;
    
    x = ((pos_t)arg2).x;
    y = ((pos_t)arg2).y;


//    child = get_child(ctrl, x, y);
//    if(child)
//    {
//        return send_message(child, msg, 0, arg2);
//    };

    if( (msg == MSG_LBTNDOWN) ||
        (msg == MSG_MOUSEMOVE) )
    {
        x = ((pos_t)arg2).x;
        y = ((pos_t)arg2).y;

        if( pt_in_rect(&fr->left, x, y))
        {
            rc = &fr->left;
            if( (y+24) > win->h)
                 cursor = cursor_nesw;
            else
                cursor = cursor_we;
            set_cursor(cursor);
            main_cursor = cursor;
        }
        else if( pt_in_rect(&fr->right, x, y))
        {
//            printf("pos x%d y%d\n", x, y);

            rc = &fr->right;
            if( (y+24) > win->h)
                 cursor = cursor_nwse;
            else
                 cursor = cursor_we;
//            printf("Set cursor %x\n", cursor);
            set_cursor(cursor);
            main_cursor = cursor;
        }
        else if( pt_in_rect(&fr->bottom, x, y))
        {
            rc = &fr->bottom;
            cursor = cursor_ns;
            if(x+24 > win->w)
                cursor = cursor_nwse;
            else if(x < rc->l+24)
                cursor = cursor_nesw;
            set_cursor(cursor);
            main_cursor = cursor;
        }
    };

    switch( msg )
    {
        case MSG_LBTNDOWN:
            if( rc != NULL)
            {
                int relx, rely;

                capture_mouse(ctrl);
                spos = get_cursor_pos();
                fr->track = rc;

                relx = spos.x - win->rc.l;
                rely = spos.y - win->rc.t;
//                printf("relx %d rely %d\n", relx, rely);

                if(fr->track == &fr->left ||
                   fr->track == &fr->right)
                {
                    if(rely+24 > win->h)
                        track_mode = 1;
                };
                if(fr->track == &fr->bottom)
                {
                    if(relx < 24)
                        track_mode = 2;
                    else if(relx+24 > win->w)
                       track_mode = 3;
                }

                break;
            };

        case MSG_LBTNUP:
            release_mouse();
            fr->track     = NULL;
            track_mode    = 0;
            break;

        case MSG_MOUSEMOVE:
            if(mouse_capture == ctrl)
            {
                pos_t npos;
                npos = get_cursor_pos();
//                printf("cursor pos %dx%d\n", npos.x, npos.y);

                if( npos.val != spos.val)
                {
                    int w, h;

                    rect_t nrc = win->rc;
                    spos = npos;

                    if(fr->track == &fr->left)
                    {
                        nrc.l = npos.x-2;
                        if(nrc.l < 0)
                            nrc.l = 0;
                        if(track_mode==1)
                            nrc.b = npos.y+2;
                    }
                    else if(fr->track == &fr->right)
                    {
                        nrc.r = npos.x+2;
                        if(track_mode==1)
                            nrc.b = npos.y+2;
                    }
                    else if(fr->track == &fr->bottom)
                    {
                        nrc.b = npos.y+2;
                        if(track_mode==2)
                            nrc.l = npos.x-2;
                        else if (track_mode==3)
                            nrc.r = npos.x+2;
                    };

                    w = nrc.r - nrc.l;
                    h = nrc.b - nrc.t;

                    if(w <310)
                        w = 310;
                    if(h < 120)
                        h = 120;

                    __asm__ __volatile__(
                    "int $0x40"
                    ::"a"(67), "b"(nrc.l), "c"(nrc.t),
                    "d"(w-1),"S"(h-1) );
                };
            }
    };

    return 1;
}

