#ifndef KOLIBRI_STATICTEXT_H
#define KOLIBRI_STATICTEXT_H

typedef enum {
	cp866,  // 6x9
	CP866,  // 8x16
	UTF16,
	UTF8
} encoding_t;

typedef struct {
	uint32_t start_xy;
	char *text;
	uint32_t color_flags;
	uint32_t bg_color;
}statictext;

typedef struct {
	uint32_t start_xy;
	int32_t number;
	uint32_t color_flags;
	uint32_t bg_color;
	uint16_t width;
}staticnum;

statictext* kolibri_statictext(statictext* st, uint32_t xy, char *text, encoding_t enc, int size, color_t font, color_t bg)
{
    st->start_xy = xy;
    st->text = text;
    st->color_flags = 0x80000000; // asciiz
    st->bg_color = bg;
    if(bg & 0xFFFFFF) st->color_flags |= 0x40000000;// use background
    st->color_flags |= ((enc & 1) << 4) | (size & 7) << 24;
    st->color_flags |= font & 0xFFFFFF;

    return st;
}

statictext* kolibri_statictext_def(statictext* st, uint32_t xy, char *text)
{
    return kolibri_statictext(st, xy, text, 0, 0, kolibri_color_table.color_work_text, 0);
}

statictext* kolibri_new_statictext(uint32_t xy, char *text, encoding_t enc, int size, color_t font, color_t bg)
{
    statictext *st = (statictext*)malloc(sizeof(statictext));

    return kolibri_statictext(st, xy, text, enc, size, font, bg);
}

statictext* kolibri_new_statictext_def(uint32_t xy, char *text)
{
    return kolibri_new_statictext(xy, text, 0, 0, kolibri_color_table.color_work_text, 0);
}

__attribute__((__stdcall__))
void statictext_draw(statictext *st)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(4),
      "b"(st->start_xy),
      "c"(st->color_flags),
      "d"(st->text),
      "D"(st->bg_color)
    :);
}

static inline void gui_add_statictext(kolibri_window *wnd, statictext* st)
{
    kolibri_window_add_element(wnd, KOLIBRI_STATICTEXT, st);
}



staticnum* kolibri_staticnum(staticnum* st, uint32_t xy, int32_t width, int16_t number, encoding_t enc, int size, color_t font, color_t bg)
{
    st->start_xy = xy;
    st->number = number;
    st->color_flags = 0;
    st->bg_color = bg;
    if(bg & 0xFFFFFF) st->color_flags |= 0x40000000;// use background
    st->color_flags |= ((enc & 1) << 4) | (size & 7) << 24;
    st->color_flags |= font & 0xFFFFFF;
    st->width = width;

    return st;
}

staticnum* kolibri_staticnum_def(staticnum* st, uint32_t xy, int16_t width, int32_t number)
{
    return kolibri_staticnum(st, xy, width, number, 0, 0, kolibri_color_table.color_work_text, 0);
}

staticnum* kolibri_new_staticnum(uint32_t xy, int32_t width, int32_t number, encoding_t enc, int size, color_t font, color_t bg)
{
    staticnum *st = (staticnum*)malloc(sizeof(staticnum));

    return kolibri_staticnum(st, xy, width, number, enc, size, font, bg);
}

staticnum* kolibri_new_staticnum_def(uint32_t xy, int32_t width, int32_t number)
{
    return kolibri_new_staticnum(xy, width, number, cp866, 0, kolibri_color_table.color_work_text, 0);
}

static inline void gui_add_staticnum(kolibri_window *wnd, staticnum* sn)
{
    kolibri_window_add_element(wnd, KOLIBRI_STATICNUM, sn);
}


__attribute__((__stdcall__))
void staticnum_draw(staticnum *st)
{
    register uint32_t fmt;
    if (st->width < 0)
        fmt = (-st->width << 16); // leading zeros, decimal
    else
        fmt = (st->width << 16) | 0x80000000; // no leading zeros, decimal

    __asm__ __volatile__(
    "int $0x40"
    ::"a"(47),
      "b"(fmt),
      "c"(st->number),
      "d"(st->start_xy),
      "S"(st->color_flags),
      "D"(st->bg_color)
    :);
}


#endif /* KOLIBRI_STATICTEXT_H */
