
#include <types.h>
#include <core.h>
#include <spinlock.h>
#include <link.h>
#include <mm.h>
#include <slab.h>


slab_cache_t *win_slab;

link_t  win_list;

window_t *win_slot[256];

extern int current_task;

void fill_disp_data(int left, int top, int right,
                    int bottom, int slot);

void update_disp_data(window_t *win)
{
    do{
//        fill_disp_data(win->wrect.left, win->wrect.top,
//                       win->wrect.right,win->wrect.bottom,
//                       win->slot);


        __asm__ __volatile__ (
        "call _set_screen \n\t"
        :
        :"b" (win->wrect.left),
        "a" (win->wrect.top),
        "c" (win->wrect.right-win->wrect.left+1),
        "d" (win->wrect.bottom-win->wrect.top+1),
        "S" (win->slot)
        :"edi");

        __asm__ __volatile__ (
        ""
        :::"eax", "ebx", "ecx", "edx", "esi");

        win = (window_t*)win->link.prev;
    } while(&win->link != &win_list);
}

void insert_window(window_t *win)
{
    if( list_empty(&win_list))
        list_prepend(&win->link, &win_list);
    else
    {
        window_t *tmp;
        tmp = (window_t*)win_list.next;

        while( &tmp->link != &win_list)
        {
            if(win->style <= tmp->style)
                break;
            tmp = (window_t*)tmp->link.next;
        }
        list_insert(&win->link, &tmp->link);
    };
    update_disp_data(win);
};

u32_t sys_create_window(char *caption, u32_t style,
                        int x, int y, int width, int height)
{
    window_t *win;
    int r, b;

    DBG("\ncreate window %s, x %d y %d\n"
        "width %d height %d\n",caption, x,y, width, height);

    win = (window_t*)slab_alloc(win_slab,0);

    link_initialize(&win->link);

    r = x+width-1;
    b = y+height-1;

    win->wrect.left = x;
    win->wrect.top = y;
    win->wrect.right = r;
    win->wrect.bottom = b;

    win->crect.left = x;
    win->crect.top = y;
    win->crect.right = r;
    win->crect.bottom = b;

    win->style = style;
    win->slot  = current_task;

    list_initialize(&win->queue);
    win->qflags = 0;

    win->caption = caption;

    win_slot[current_task] = win;
    return current_task;
}

#define QS_PAINT    1

bool sys_show_window(u32_t handle)
{
    window_t *win;

    win = win_slot[current_task];

    insert_window(win);
    win->qflags |= QS_PAINT;

    return true;
};

void sys_get_event(event_t *ev)
{
    window_t *win;

    win = win_slot[current_task];

    if(win->qflags & QS_PAINT)
    {
        ev->code = 1;
        ev->win = win->slot;
        ev->val1 = 0;
        ev->val2 = 0;
        ev->x = 0;
        ev->y = 0;
        win->qflags&= ~QS_PAINT;
    };
}

static inline draw_bar(int x, int y, int w, int h, u32_t color)
{
    int_draw_bar(x, y, w, h, color);
    __asm__ __volatile__ (
    ""
    :::"ebx", "esi", "edi");
};

static inline hline(int x, int y, int w, color_t color)
{
    int_hline(x, y, w, color);
    __asm__ __volatile__ (
    ""
    :::"esi", "edi");
};

static inline vline(int x, int y, int h, color_t color)
{
    int_vline(x, y, h, color);
    __asm__ __volatile__ (
    ""
    :::"esi", "edi");
};

static inline rectangle(int x, int y, int w, int h, color_t color)
{
    int_rectangle(x, y, w, h, color);
    __asm__ __volatile__ (
    ""
    :::"esi", "edi");
};

extern color_t skin_active;

void sys_def_window_proc(event_t *ev)
{
    window_t *win;

    win = win_slot[current_task];

    if(ev->code =1)
    {
        int w, h;
        color_t *skin = &skin_active;

        w = win->wrect.right-win->wrect.left+1;
        h = win->wrect.bottom - win->wrect.top+1;

        rectangle(win->wrect.left, win->wrect.top,
                 w, h, skin[1]);

        rectangle(win->wrect.left+1, win->wrect.top+1,
                 w-2, h-2, skin[2]);

        rectangle(win->wrect.left+2, win->wrect.top+2,
                 w-4, h-4, skin[2]);

        rectangle(win->wrect.left+3, win->wrect.top+3,
                 w-6, h-6, skin[2]);

        rectangle(win->wrect.left+4, win->wrect.top+4,
                 w-8, h-8, skin[0]);

 //       draw_bar(win->wrect.left+4, win->wrect.top+4,
 //                w-8, h-8, skin[1]);

    };
};
