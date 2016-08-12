#ifndef KOLIBRI_SCROLLBAR_H
#define KOLIBRI_SCROLLBAR_H

typedef struct {
//	uint16_t xsize;
//    uint16_t xpos;
//    uint16_t ysize;
//    uint16_t ypos;
    uint32_t x_w;
    uint32_t y_h;
    uint32_t btn_height;
    uint32_t type;
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
    uint32_t all_redraw;
    uint32_t ar_offset;
} scrollbar;

scrollbar* kolibri_new_scrollbar(uint32_t x_w, uint32_t y_h, uint32_t btn_height, uint32_t max_area,
	uint32_t cur_area, uint32_t position, uint32_t back_color, uint32_t front_color, uint32_t line_color)
{
    scrollbar *sb = (scrollbar *)calloc(1, sizeof(scrollbar));

    sb->x_w = x_w;
    sb->y_h = y_h;
    sb->btn_height = btn_height;
    sb->type = 1;
    sb->max_area = max_area;
    sb->cur_area = cur_area;
    sb->position = position;
    sb->line_color = 0; //line_color; // 0
    sb->back_color = 0xeeeeee; // back_color;  // 0xeeeeee
    sb->front_color = 0xbbddff; //front_color; // 0xbbddff
    sb->ar_offset = max_area / 30; // temporary step 3%
    sb->all_redraw = 1;
    return sb;
};

//use_optionbox_driver

extern void (*scrollbar_h_draw)(scrollbar*) __attribute__((__stdcall__));
extern void (*scrollbar_h_mouse)(scrollbar*) __attribute__((__stdcall__));
extern void (*scrollbar_v_draw)(scrollbar*) __attribute__((__stdcall__));
extern void (*scrollbar_v_mouse)(scrollbar*) __attribute__((__stdcall__));

#endif /* KOLIBRI_SCROLLBAR_H */
