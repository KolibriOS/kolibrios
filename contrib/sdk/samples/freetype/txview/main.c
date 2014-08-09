#include <stdio.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <kos32sys.h>
#include <pixlib2.h>

typedef struct
{
  int  l;
  int  t;
  int  r;
  int  b;
}rect_t;

typedef struct
{
    FT_Face face;
    int     height;
    int     base;

    FT_Glyph glyph[256];

}font_t;

typedef struct
{
    bitmap_t bitmap;
    font_t   *font;

    char    *text;
    char   **line;
    int      lines;
    int      txlines;
    int      startline;
    int      endline;
    int      w;
    int      h;
}tview_t;

int init_tview(tview_t *txv, int width, int height, char *text, int size);
void txv_set_size(tview_t *txv, int txw, int txh);
void txv_set_font_size(tview_t *txv, int size);

int txv_scroll_up(tview_t *txv);
int txv_scroll_down(tview_t *txv);

void* init_fontlib();
int draw_text_ext(bitmap_t *winbitmap, FT_Face face, char *text, int len, rect_t *rc, int color);

void draw_window(void)
{
    BeginDraw();
    DrawWindow(0,0,0,0,NULL,0,0x73);
    EndDraw();
}

tview_t txv;

int main(int argc, char *argv[])
{
    ufile_t uf;
    oskey_t   key;
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

    init_pixlib(0);
    init_fontlib();

    init_tview(&txv, clw, clh, uf.data, uf.size);

    BeginDraw();
    DrawWindow(10, 40, clw+TYPE_3_BORDER_WIDTH*2,
               clh+TYPE_3_BORDER_WIDTH+get_skin_height(), "Text example", 0x000000, 0x73);
    blit_bitmap(&txv.bitmap, TYPE_3_BORDER_WIDTH, get_skin_height(), txv.w, txv.h, 0, 0);

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

                get_proc_info(proc_info);

                winx = *(uint32_t*)(proc_info+34);
                winy = *(uint32_t*)(proc_info+38);
                winw = *(uint32_t*)(proc_info+42)+1;
                winh = *(uint32_t*)(proc_info+46)+1;

                txw = winw - TYPE_3_BORDER_WIDTH*2;
                txh = winh - TYPE_3_BORDER_WIDTH - get_skin_height();

                if( (txw != clw) ||
                    (txh != clh) )
                {
                    txv_set_size(&txv, txw, txh);
                    clw = txw;
                    clh = txh;
                };

                draw_window();
                blit_bitmap(&txv.bitmap, TYPE_3_BORDER_WIDTH, get_skin_height(), txv.w, txv.h, 0, 0);
                break;
            }
		case 2:
            key = get_key();
            printf("key %d\n", key.code);
            switch(key.code)
            {
                case 27:
                    return;

                case 45:
                    txv_set_font_size(&txv, txv.font->height-3);
                    blit_bitmap(&txv.bitmap, TYPE_3_BORDER_WIDTH, get_skin_height(), txv.w, txv.h, 0, 0);
                    break;

                case 61:
                    txv_set_font_size(&txv, txv.font->height+1);
                    blit_bitmap(&txv.bitmap, TYPE_3_BORDER_WIDTH, get_skin_height(), txv.w, txv.h, 0, 0);
                    break;
                case 177:
                    if( txv_scroll_up(&txv) )
                        blit_bitmap(&txv.bitmap, TYPE_3_BORDER_WIDTH, get_skin_height(), txv.w, txv.h, 0, 0);
                    break;

                case 178:
                    if( txv_scroll_down(&txv) )
                        blit_bitmap(&txv.bitmap, TYPE_3_BORDER_WIDTH, get_skin_height(), txv.w, txv.h, 0, 0);
                    break;
            }
            break;

        case 3:
			// button pressed; we have only one button, close
			return;

        case 6:
//            pos = get_mouse_pos();
//            buttons = get_mouse_buttons();
            wheels = get_mouse_wheels();

            if( wheels & 0xFFFF)
            {
                int r;

                if((short)wheels > 0)
                    r = txv_scroll_up(&txv);
                else
                    r = txv_scroll_down(&txv);

                if( r )
                    blit_bitmap(&txv.bitmap, TYPE_3_BORDER_WIDTH, get_skin_height(), txv.w, txv.h, 0, 0);
            }
		}
	}
    return 0;
}
