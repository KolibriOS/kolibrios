#ifndef KOLIBRI_PATHSHOW_H
#define KOLIBRI_PATHSHOW_H

typedef struct __attribute__ ((__packed__)) {
	uint32_t type;
	uint32_t x_y;
	uint16_t font_size_x;  // 6 - for font 0, 8 - for font 1
	uint16_t area_size_x;
    uint32_t font_number;  // 0 - monospace, 1 - variable, as fn4 (2bit only 0-3)
    uint32_t background_flag; // as fn4, if 0, bk_color unneeded
    color_t  font_color;      // as fn4
    color_t  background_color; // as fn4
    char* text_pointer;       // 4096 ?
    char* work_area_pointer;  // 4096 ?
    uint32_t temp_text_length;
} pathview;



inline pathview* kolibri_pathview(pathview* p, uint32_t x_y, uint32_t width, uint32_t font_number, uint32_t is_bkgr, char* text, char* temp_buf, color_t font, color_t  bkgr)
{
    p->type = p->temp_text_length = 0;
    p->x_y = x_y;
    p->font_size_x = font_number == 0 ? 6 : 8;  // need correction for bigger fonts
    p->area_size_x = width;
    p->font_number = font_number;
    p->background_flag = is_bkgr;
    p->text_pointer = text;
    p->work_area_pointer = temp_buf;
    p->font_color = font;
    p->background_color = bkgr;

    return p;
}

inline pathview* kolibri_new_pathview(pathview* p, uint32_t x_y, uint32_t width, uint32_t font_number, uint32_t is_bkgr, char* text, char* temp_buf, color_t font, color_t  bkgr)
{
    pathview *new_pv = (pathview *)malloc(sizeof(pathview));
    return kolibri_pathview(new_pv, x_y, width, font_number, is_bkgr, text, temp_buf, font, bkgr);
}

inline void gui_add_pathview(kolibri_window *wnd, pathview* pv)
{
    kolibri_window_add_element(wnd, KOLIBRI_PATHSHOW, pv);
}


extern void (*path_show_prepare)(pathview *) __attribute__((__stdcall__));
extern void (*path_show_draw)(pathview *) __attribute__((__stdcall__));

#endif /* KOLIBRI_PATHSHOW_H */
