
#include <stdio.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <kos32sys.h>

#include "winlib.h"


typedef struct tview *tview_t;

tview_t *create_tview(ctx_t *ctx, int width, int height);
void txv_get_margins(const tview_t *txv, rect_t *margins);
void txv_set_margins(tview_t *txv, const rect_t *margins);
void txv_set_size(tview_t *txv, int txw, int txh);
void txv_set_font_size(tview_t *txv, int size);
int  txv_get_font_size(tview_t *txv);
void txv_set_text(tview_t *txv, char *text, int size);
int txv_scroll_up(tview_t *txv);
int txv_scroll_down(tview_t *txv);
int txv_page_up(tview_t *txv);
int txv_page_down(tview_t *txv);

typedef struct
{
    volatile int lock;
    unsigned int handle;
}mutex_t;


void* init_fontlib();

void draw_window(void)
{
    BeginDraw();
    DrawWindow(0,0,0,0,NULL,0,0x73);
    EndDraw();
}

tview_t *txv;



int main(int argc, char *argv[])
{
    ufile_t  uf;
    oskey_t  key;
    ctx_t   *ctx;
    rect_t   margins = {4,2,20,0};

    int clw = 640;
    int clh = 480;


    __asm__ __volatile__(
    "int $0x40"
    ::"a"(40), "b"(0xc0000027));

    if(argc < 2)
        uf = load_file("/RD/1/EXAMPLE.ASM");
    else uf = load_file(argv[1]);

    if(uf.data == NULL ||
        uf.size == 0)
        return 0;

    ctx = create_context(TYPE_3_BORDER_WIDTH, get_skin_height(), clw, clh);

    init_fontlib();

    txv = create_tview(ctx, clw, clh);
    txv_set_margins(txv, &margins);
    txv_set_text(txv, uf.data, uf.size);

    BeginDraw();
    DrawWindow(10, 40, clw+TYPE_3_BORDER_WIDTH*2,
               clh+TYPE_3_BORDER_WIDTH+get_skin_height(), "Text example", 0x000000, 0x73);
    show_context(ctx);
    EndDraw();

	for (;;)
	{
        uint32_t wheels;
        int      buttons;
        pos_t    pos;

        switch (get_os_event())
		{
            case 1:
            {
                char proc_info[1024];
                int winx, winy, winw, winh;
                int txw, txh;
                char state;

                draw_window();

                get_proc_info(proc_info);
                state  = *(char*)(proc_info+70);
                if(state & (WIN_STATE_MINIMIZED|WIN_STATE_ROLLED))
                    continue;

                winx = *(uint32_t*)(proc_info+34);
                winy = *(uint32_t*)(proc_info+38);
                winw = *(uint32_t*)(proc_info+42)+1;
                winh = *(uint32_t*)(proc_info+46)+1;

                txw = winw - TYPE_3_BORDER_WIDTH*2;
                txh = winh - TYPE_3_BORDER_WIDTH - get_skin_height();

                if( (txw != clw) ||
                    (txh != clh) )
                {
                    resize_context(ctx, txw, txh);
                    txv_set_size(txv, txw, txh);
                    clw = txw;
                    clh = txh;
                };

                show_context(ctx);
                break;
            }
		case 2:
            key = get_key();
//            printf("key %d\n", key.code);
            switch(key.code)
            {
                case 27:
                    return 0;

                case 45:
                    txv_set_font_size(txv, txv_get_font_size(txv) - 2);
                    show_context(ctx);
                    break;

                case 61:
                    txv_set_font_size(txv, txv_get_font_size(txv) + 2);
                    show_context(ctx);
                    break;

                case 177:
                    if( txv_scroll_down(txv) )
                        show_context(ctx);
                    break;

                case 178:
                    if( txv_scroll_up(txv) )
                        show_context(ctx);
                    break;
                case 183:
                    if( txv_page_down(txv) )
                        show_context(ctx);
                    break;
                case 184:
                    if( txv_page_up(txv) )
                        show_context(ctx);
                    break;
            }
            break;

        case 3:
			// button pressed; we have only one button, close
            return 0;

        case 6:
//            pos = get_mouse_pos();
//            buttons = get_mouse_buttons();
            wheels = get_mouse_wheels();

            if( wheels & 0xFFFF)
            {
                int r = 0;

                if((short)wheels > 0)
                    r = txv_scroll_down(txv);
                else if((short)wheels < 0)
                    r = txv_scroll_up(txv);

                if( r )
                    show_context(ctx);
            }
		}
	}
    return 0;
}
