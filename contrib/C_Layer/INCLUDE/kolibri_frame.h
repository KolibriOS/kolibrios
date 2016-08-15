#ifndef KOLIBRI_FRAME_H
#define KOLIBRI_FRAME_H

enum {
	TOP,
	BOTTON
};

typedef struct {
	uint32_t type;
	uint32_t x_w;
	uint32_t y_h;
	color_t ext_col;
	color_t int_col;
	uint32_t draw_text_flag;
	char *text_pointer;
	uint32_t text_position;
	uint32_t font_number;
	uint32_t font_size_y;
	color_t font_color;
	color_t font_bg_color;
}frame;

inline frame* kolibri_frame(frame* f, uint32_t x_w, uint32_t y_h, color_t ext_col, color_t int_col, char *text, uint32_t text_position, color_t font_color, color_t font_bgcolor)
{
    f->type = 0;
    f->x_w = x_w;
    f->y_h = y_h;
    f->ext_col = ext_col;
    f->int_col = int_col;
    f->draw_text_flag = text != NULL;
    f->text_pointer = text;
    f->text_position = text_position;
    f->font_number = 0;  // 0 == font 6x9, 1==8x16
    f->font_size_y = 9;
    f->font_color = font_color | 0x80000000;
    f->font_bg_color = font_bgcolor;

    return f;
}

inline frame* kolibri_new_frame(uint32_t x_w, uint32_t y_h, color_t ext_col, color_t int_col, char *text, uint32_t text_position, color_t font_color, color_t font_bgcolor)
{
    frame *new_frame = (frame *)malloc(sizeof(frame));
    return kolibri_frame(new_frame, x_w, y_h, ext_col, int_col, text, text_position, font_color, font_bgcolor);
}

inline frame* kolibri_frame_def(frame* f, uint32_t x_w, uint32_t y_h, char *text)
{
    return kolibri_frame(f, x_w, y_h, 0x00FCFCFC, 0x00DCDCDC, text, TOP, kolibri_color_table.color_work_text, kolibri_color_table.color_work_area);
}

inline frame* kolibri_new_frame_def(uint32_t x_w, uint32_t y_h, char *text)
{
    return kolibri_new_frame(x_w, y_h, 0x00FCFCFC, 0x00DCDCDC, text, TOP, kolibri_color_table.color_work_text, kolibri_color_table.color_work_area);
}

inline void gui_add_frame(kolibri_window *wnd, frame* f)
{
    kolibri_window_add_element(wnd, KOLIBRI_FRAME, f);
}


extern void (*frame_draw)(frame *) __attribute__((__stdcall__));

#endif /* KOLIBRI_FRAME_H */
