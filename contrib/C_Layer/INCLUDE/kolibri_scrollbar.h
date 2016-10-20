#ifndef KOLIBRI_SCROLLBAR_H
#define KOLIBRI_SCROLLBAR_H

typedef struct __attribute__ ((__packed__)) {
//	uint16_t xsize;
//    uint16_t xpos;
//    uint16_t ysize;
//    uint16_t ypos;
    uint32_t x_w;
    uint32_t y_h;
    uint32_t btn_height;
    uint32_t type;  // type 1 - stylish frame, type 2 - ?, type 0 - ?
    uint32_t max_area;
    uint32_t cur_area;
    uint32_t position;
    uint32_t back_color;
    uint32_t front_color;
    uint32_t line_color;
    uint32_t redraw;
    uint16_t delta;
    uint16_t delta2;
    uint16_t r_size_x;
    uint16_t r_start_x;
    uint16_t r_size_y;
    uint16_t r_start_y;
    uint32_t m_pos;
    uint32_t m_pos2;
    uint32_t m_keys;
    uint32_t run_size;
    uint32_t position2;
    uint32_t work_size;
    uint32_t all_redraw;  // need to be set =1 before each redraw
    uint32_t ar_offset;
} scrollbar;

inline scrollbar* kolibri_scrollbar(scrollbar* sb, uint32_t x_w, uint32_t y_h, uint32_t btn_height, uint32_t max_area,
	uint32_t cur_area, uint32_t position, uint32_t back_color, uint32_t front_color, uint32_t line_color)
{
    memset(sb, 0, sizeof(scrollbar));

    sb->x_w = x_w;
    sb->y_h = y_h;
    sb->btn_height = btn_height;
    sb->type = 1;
    sb->max_area = max_area;
    sb->cur_area = cur_area;
    sb->position = position;
    sb->line_color = line_color;
    sb->back_color = back_color;  // 0xeeeeee
    sb->front_color = front_color; // 0xbbddff
    sb->ar_offset = max_area / 30; // temporary step 3%
    sb->all_redraw = 1;
    return sb;
};

inline scrollbar* kolibri_new_scrollbar(uint32_t x_w, uint32_t y_h, uint32_t btn_height, uint32_t max_area,
	uint32_t cur_area, uint32_t position, uint32_t back_color, uint32_t front_color, uint32_t line_color)
{
    scrollbar *sb = (scrollbar *)malloc(sizeof(scrollbar));

    return kolibri_scrollbar(sb, x_w, y_h, btn_height, max_area, cur_area, position, back_color, front_color, line_color);
};

inline scrollbar* kolibri_scrollbar_def(scrollbar* sb, uint32_t x_w, uint32_t y_h, uint32_t max_area, uint32_t cur_area, uint32_t position)
{
    return kolibri_scrollbar(sb, x_w, y_h, 15, max_area, cur_area, position, kolibri_color_table.color_work_area, kolibri_color_table.color_work_button, kolibri_color_table.color_work_button_text);
};

inline scrollbar* kolibri_new_scrollbar_def(uint32_t x_w, uint32_t y_h, uint32_t max_area, uint32_t cur_area, uint32_t position)
{
    return kolibri_new_scrollbar(x_w, y_h, 15, max_area, cur_area, position, kolibri_color_table.color_work_area, kolibri_color_table.color_work_button, kolibri_color_table.color_work_button_text);
};

inline void gui_add_scrollbar_h(kolibri_window *wnd, scrollbar* sb)
{
    kolibri_window_add_element(wnd, KOLIBRI_SCROLL_BAR_H, sb);
}

inline void gui_add_scrollbar_v(kolibri_window *wnd, scrollbar* sb)
{
    kolibri_window_add_element(wnd, KOLIBRI_SCROLL_BAR_V, sb);
}

extern void (*scrollbar_h_draw)(scrollbar*) __attribute__((__stdcall__));
extern void (*scrollbar_h_mouse)(scrollbar*) __attribute__((__stdcall__));
extern void (*scrollbar_v_draw)(scrollbar*) __attribute__((__stdcall__));
extern void (*scrollbar_v_mouse)(scrollbar*) __attribute__((__stdcall__));

#endif /* KOLIBRI_SCROLLBAR_H */
