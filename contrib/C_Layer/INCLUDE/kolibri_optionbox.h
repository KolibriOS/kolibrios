#ifndef KOLIBRI_OPTIONBOX_H
#define KOLIBRI_OPTIONBOX_H

typedef struct __attribute__ ((__packed__)) option_box_t {
    struct option_box_t **selected;
    uint16_t posx;
    uint16_t posy;
    uint32_t text_margin; // = 4 ���������� �� �������������� ��� ����� �� �������
    uint32_t size;       // 12 ������ �������� ��� �����
    color_t color;
    color_t border_color; // individual border
    color_t text_color;
    char *text;
    uint32_t text_len;
    uint32_t flags;
}option_box;

extern void (*option_box_draw)(option_box **) __attribute__((__stdcall__));
extern void (*option_box_mouse)(option_box **)__attribute__((__stdcall__));

static inline option_box* gui_optionbox(option_box* ob, uint32_t x_y, char* text, option_box**select)
{
    ob->selected = select;
    ob->posx = x_y >> 16;
    ob->posy = x_y & 0xFFFF;
    ob->text_margin = 4;
    ob->size = 12;
    ob->color = kolibri_color_table.color_work_button_text;
    ob->border_color = kolibri_color_table.color_work_button;
    ob->text_color = kolibri_color_table.color_work_text | 0x80000000;
    ob->text = text;
    ob->text_len = strlen(text);
    ob->flags = 0; // not used

    return ob;
}

static inline option_box* gui_new_optionbox(uint32_t x_y, char* text, option_box**select)
{
    option_box* ob = malloc(sizeof(option_box));

    return gui_optionbox(ob, x_y, text, select);
}

#define gui_optionbox_def(a,b,c,d) gui_optionbox(a,b,c,d)

#define gui_new_optionbox_def(a,b,c) gui_new_optionbox(a,b,c)

static inline void gui_add_optiongroup(kolibri_window *wnd, option_box** option_group)
{
    kolibri_window_add_element(wnd, KOLIBRI_OPTIONGROUP, option_group);
}


#endif /* KOLIBRI_OPTIONBOX_H */
