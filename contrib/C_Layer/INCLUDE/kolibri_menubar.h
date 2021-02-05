#ifndef KOLIBRI_MENUBAR_H
#define KOLIBRI_MENUBAR_H

typedef struct
{
	uint32_t type;   // 1 ���� ��� �������, ������ �����

	uint32_t x_w;   // ������� �����
	uint32_t y_h;

	char* text_pointer;
	char* pos_pointer;
	char* text_end;
	uint32_t mouse_pos;
	uint32_t mouse_keys;

	uint32_t x_w1;  // �������
	uint32_t y_h1;

	color_t bckg_col;  // ��� �������� �����
	color_t frnt_col;  // ��� ���������� �������� ������
	color_t menu_col;  // ��� ���������� ����� (��������)
	uint32_t select;
	uint32_t out_select;
	char* buf_adress;
	char* procinfo;
	uint32_t click;
	uint32_t cursor;
	uint32_t cursor_old;
	uint32_t interval;
	uint32_t cursor_max;
	uint32_t extended_key;
	color_t menu_sel_col;  // ���� ���� ���������� ���������
	color_t bckg_text_col; // ���� ������ ������������ ������
	color_t frnt_text_col;  // ���� ������ ���������� ������
	uint32_t mouse_keys_old;
	uint32_t font_height;
	uint32_t cursor_out;
	uint32_t get_mouse_flag;
} menubar;


static inline menubar* kolibri_menubar(menubar* bar, uint32_t x_w, uint32_t y_h, uint16_t sub_w, uint16_t sub_h, char **menutext,
                                color_t sel_font, color_t unsel_font, color_t top_bg, color_t top_select, color_t sub_bg, color_t sub_select)
{
    static char procinfo[1024];
    memset(bar, 0, sizeof(menubar));
    bar->type = 0;
    bar->x_w = x_w;
    bar->y_h = y_h;

    // count summary length
    char *pc, **mitem;
    int len = 0;
    for(mitem = menutext; *mitem; mitem++) len += strlen(*mitem) + 1;

    // copy menu items in needed format
    bar->text_pointer = malloc(len + 1);   // need to be freed manual at closing secondary windows with menu
    for (pc = bar->text_pointer, mitem = menutext; *mitem; pc += strlen(*mitem++) + 1)
        strcpy(pc, *mitem);
    *pc = 0;
    bar->text_end = pc;
    bar->pos_pointer = strchr(bar->text_pointer, 0) + 1;

    bar->x_w1 = X_Y(x_w >> 16, sub_w);
    bar->y_h1 = X_Y((y_h >> 16) + (y_h & 0xFFFF), sub_h);

    bar->interval = 16;
    bar->font_height = 8;

    bar->bckg_col = top_bg;
    bar->frnt_col = top_select;
    bar->menu_col = sub_bg;
    bar->menu_sel_col = sub_select;
    bar->bckg_text_col = unsel_font;
    bar->frnt_text_col = sel_font;
    bar->procinfo = procinfo;

    return bar;
}

static inline menubar* kolibri_new_menubar(uint32_t x_w, uint32_t y_h, uint16_t sub_w, uint16_t sub_h, char **menutext,
                                color_t sel_font, color_t unsel_font, color_t top_bg, color_t top_select, color_t sub_bg, color_t sub_select)
{
    menubar *new_bar = (menubar*)malloc(sizeof(menubar));
    return kolibri_menubar(new_bar, x_w, y_h, sub_w, sub_h, menutext, sel_font, unsel_font, top_bg, top_select, sub_bg, sub_select);
}

static inline menubar* kolibri_menubar_def(menubar* bar, uint32_t x_w, uint32_t y_h, uint16_t sub_w, uint16_t sub_h, char **menutext)
{
    return kolibri_menubar(bar, x_w, y_h, sub_w, sub_h, menutext,
                           kolibri_color_table.color_work_button_text, kolibri_color_table.color_work_text, kolibri_color_table.color_work_area,
                           kolibri_color_table.color_work_button, kolibri_color_table.color_grab_bar_button, kolibri_color_table.color_work_button);
}

static inline menubar* kolibri_new_menubar_def(uint32_t x_w, uint32_t y_h, uint16_t sub_w, uint16_t sub_h, char **menutext)
{
    return kolibri_new_menubar(x_w, y_h, sub_w, sub_h, menutext,
                           kolibri_color_table.color_work_button_text, kolibri_color_table.color_work_text, kolibri_color_table.color_work_area,
                           kolibri_color_table.color_work_button, kolibri_color_table.color_grab_bar_button, kolibri_color_table.color_work_button);
}

static inline void gui_add_menubar(kolibri_window *wnd, menubar* bar)
{
    kolibri_window_add_element(wnd, KOLIBRI_MENU_BAR, bar);
}


extern void (*menu_bar_draw)(menubar *) __attribute__((__stdcall__));
extern void (*menu_bar_mouse)(menubar *) __attribute__((__stdcall__));
extern void (*menu_bar_activate)(menubar *) __attribute__((__stdcall__));

#endif /* KOLIBRI_MENUBAR_H */
