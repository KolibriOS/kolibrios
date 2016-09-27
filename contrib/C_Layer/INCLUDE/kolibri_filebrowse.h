#ifndef KOLIBRI_FILEBROWSE_H
#define KOLIBRI_FILEBROWSE_H

typedef struct {
	uint32_t type;
	uint32_t x_w;  // 10, 400
	uint32_t y_h; // 45, 550
	uint32_t icon_size_xy;  // x_y (16, 16)
	uint16_t line_size_x;
	uint16_t line_size_y;  // 18 or 17  - высота линии
	uint16_t type_size_x;
	uint16_t size_size_x;
	uint16_t date_size_x;
	uint16_t attributes_size_x;
	uint32_t icon_assoc_area;
	uint32_t icon_raw_area;   // z_icons.png
	uint32_t icon_resolution_raw;  // ...
	uint32_t palette_raw;      // ...
	uint32_t directory_path_area;
	uint32_t file_name_area;
	uint32_t select_flag;
	color_t background_color;  // 0xffffff
	color_t select_color; // 0xbbddff
    color_t select_text_color;
    color_t text_color;
    color_t reduct_text_color; // 0xff0000
    color_t marked_text_color;
    uint32_t max_panel_line;            // // moved to scrollbar->cur_area, - максимальное число строк в окне
	uint32_t select_panel_counter;  // 1 if focused
	uint32_t folder_block;   //  количество блоков данных входа каталога (БДВК) ????? format BDVK == bdfe,, // moved to scrollbar->max_area
	uint32_t start_draw_line;       // moved to scrollbar->position and back
	uint16_t start_draw_cursor_line;
    void* folder_data;      // ??? format 32 byte - header, +4 = number, +32 - bdvk[], size of rec(bdvk cp866) = 304byte
    uint32_t temp_counter;
    uint32_t file_name_length;
    uint32_t marked_file;
    uint32_t extension_size;
    uint32_t extension_start;
    void* type_table; //dd features_table ; +122   ? table format
    char* ini_file_start;   // icons.ini
    char* ini_file_end;     // start + filesize
    uint32_t draw_scroll_bar;  // 1 = need redraw sb after key()
    uint32_t font_size_xy;  // x_y	(6, 9)
    uint32_t mouse_keys;
    uint32_t mouse_keys_old;
    uint32_t mouse_pos;
    uint32_t mouse_keys_delta;
    uint32_t mouse_key_delay; // 50
    uint32_t mouse_keys_tick;
    uint16_t start_draw_cursor_line_2;
    uint32_t all_redraw;         // 1 - force draw, 2 - ????
    uint32_t selected_BDVK_adress;
    uint16_t key_action;   // fill before key(), 1..12, wiki
    uint16_t key_action_num; // fill before key()  fn2 >> 8
    char* name_temp_area 		dd name_temp_area ;+180
    uint32_t max_name_temp_size;
    uint32_t display_name_max_length;
    uint32_t draw_panel_selection_flag;
    uint32_t mouse_pos_old;
    uint32_t marked_counter;
    char* keymap_pointer 		dd keymap_area ;+204


} file_browser;
/*
features_table:
.type_table:
	db '<DIR> '
;---------------------------------------------------------------------
.size_table:
	db '1023b '
;---------------------------------------------------------------------
.date_table:
	db '00.00.00 00:00 '
;---------------------------------------------------------------------
.year_table:
	db '    '

name_temp_area:
	rb 256

keymap_area:
	rb 128

inline frame* kolibri_filebrowser(frame* f, uint32_t x_w, uint32_t y_h, color_t ext_col, color_t int_col, char *text, uint32_t text_position, color_t font_color, color_t font_bgcolor)
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

FileBrowser_draw - рисование элемента
FileBrowser_mouse - эта функция должна вызываться при вводе информации с мыши
FileBrowser_key

extern void (*frame_draw)(frame *) __attribute__((__stdcall__));
/*
#endif /* KOLIBRI_FILEBROWSE_H */
