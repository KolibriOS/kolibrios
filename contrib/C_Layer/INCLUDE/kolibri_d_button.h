#ifndef KOLIBRI_DBUTTON_H
#define KOLIBRI_DBUTTON_H

typedef struct {
	uint32_t type;
	uint32_t x_w;
	uint32_t y_h;
	uint32_t mouse_pos;
    uint32_t mouse_keys;
    uint32_t mouse_keys_old;
    void*    active_raw;        //active bitmap
    void*    passive_raw;       //passive bitmap
    void*    click_raw;         //pressed bitmap
    uint32_t resolution_raw; // bpp, as esi fn65
    void*    palette_raw;    // palette, as edi fn65
    uint32_t offset_raw;     // width as ebp fn65
    uint32_t select;         // internal state: 0 - passive, 2 - pressed, 1 - clicked
    uint32_t click;          // clicked - 1, zero it after tested
} pict_button;


inline pict_button* kolibri_pict_button(pict_button* b, uint32_t x_w, uint32_t y_h, void* active_pict, void* passive_pict, void* pressed_pict, uint32_t bpp, void* palette, int32_t offset_line)
{
    b->type = b->mouse_pos = b->mouse_keys = b->mouse_keys_old = b->select = b->click = 0;
    b->x_w = x_w;
    b->y_h = y_h;
    b->active_raw = active_pict;
    b->passive_raw = passive_pict;
    b->click_raw = pressed_pict;
    b->resolution_raw = bpp;
    b->palette_raw = palette;
    b->offset_raw = offset_line;

    return b;
}

inline pict_button* kolibri_new_pict_button(uint32_t x_w, uint32_t y_h, void* active_pict, void* passive_pict, void* pressed_pict, uint32_t bpp, void* palette, int32_t offset_line)
{
    pict_button *new_d_but = (pict_button *)malloc(sizeof(pict_button));
    return kolibri_pict_button(new_d_but, x_w, y_h, active_pict, passive_pict, pressed_pict, bpp, palette, offset_line);
}

inline void gui_add_pict_button(kolibri_window *wnd, pict_button* db)
{
    kolibri_window_add_element(wnd, KOLIBRI_D_BUTTON, db);
}


extern void (*dynamic_button_draw)(pict_button *) __attribute__((__stdcall__));
extern void (*dynamic_button_mouse)(pict_button *) __attribute__((__stdcall__));

#endif /* KOLIBRI_DBUTTON_H */
