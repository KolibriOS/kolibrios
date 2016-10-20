#ifndef KOLIBRI_FILEBROWSE_H
#define KOLIBRI_FILEBROWSE_H

struct  __attribute__ ((__packed__)) fs_dirinfo {
    uint32_t    subfn; // 1 read dir
    uint32_t    start;
    uint32_t    flags;
    uint32_t    size;
    uint32_t    retval;
    union {
        struct __attribute__ ((__packed__)) {
            uint8_t zero;  // 0
            char*   ppath;
        };
        char path[5];  // up to 4096
    } ;
};

static inline
uint32_t sf_file(int subfn, struct fs_dirinfo* dinfo)
// retval 0 if ok
{
    uint32_t retval;
    dinfo->subfn = subfn;

    __asm__ __volatile__(
    "int $0x40 "
    :"=a"(retval)
    :"a"(70),"b"(dinfo)
    :);

    return retval;
};


struct fs_dirheader {
    uint32_t     version; // 1
    uint32_t     curn_blocks;  // number of read dir items (BDFE)
    uint32_t     totl_blocks;  // directory full size
    char         other[20]; // reserved 0
};

enum filetype
{
    fs_readonly = 1,
    fs_hidden = 2,
    fs_system = 4,
    fs_volumelabel = 8,
    fs_folder = 0x10,
    fs_nonarchived = 0x20
};

struct __attribute__ ((__packed__)) fs_filetime {
    uint8_t    sec;
    uint8_t    mm;
    uint8_t    hour;
    uint8_t    zero;
};

struct __attribute__ ((__packed__)) fs_filedate {
    uint8_t    day;
    uint8_t    month;
    uint16_t   year;
};

/// directory entry cp866
struct fsBDFE {
    uint32_t filetype;
    uint32_t encoding; // 0 - cp866, 1 - utf16le
    struct fs_filetime tm_created;
    struct fs_filedate dt_created;
    struct fs_filetime tm_accessed;
    struct fs_filedate dt_accessed;
    struct fs_filetime tm_modified;
    struct fs_filedate dt_modified;
    uint64_t size;
    char   fname[264];
}; // must be sized 304

/// directory entry UTF16LE
struct fsBDFE_16 {
    uint32_t filetype;
    uint32_t encoding; // 0 - cp866, 1 - utf16le
    struct fs_filetime tm_created;
    struct fs_filedate dt_created;
    struct fs_filetime tm_accessed;
    struct fs_filedate dt_accessed;
    struct fs_filetime tm_modified;
    struct fs_filedate dt_modified;
    uint64_t size;
    wchar_t   fname[260];
}; // must be sized 560


typedef struct __attribute__ ((__packed__)) {
	uint32_t type;  // unused
	uint32_t x_w;  // 10, 400
	uint32_t y_h; // 45, 550
	uint32_t icon_size_xy;  // x_y (16, 16)
	uint16_t line_size_x;  // not used
	uint16_t line_size_y;  // 18 or 17  - высота линии - строки с файлом
	uint16_t type_size_x;  // not used
	uint16_t size_size_x;  // not used
	uint16_t date_size_x;  // not used
	uint16_t attributes_size_x; // not used
	uint32_t icon_assoc_area;  // not used
	void* icon_raw_area;   // z_icons.png
	uint32_t icon_resolution_raw;  // ...
	void* icon_palette_raw;      // ...
	uint32_t directory_path_area;  // not used
	uint32_t file_name_area;  // not used
	uint32_t select_flag;  // widget have focus, set auto on mouseclick, but need to reset before mouse()
	color_t background_color;  // self explained, 0xffffff
	color_t select_color; // self explained, 0xbbddff
    color_t select_text_color; // self explained - have a bug - never really used
    color_t text_color; // self explained
    color_t reduct_text_color; // 0xff0000  - spec color for cutted filenames
    color_t marked_text_color; // not used
    uint32_t max_panel_line;    // moved to scrollbar->cur_area, - максимальное число строк в окне, авторасчитывается
	uint32_t select_panel_counter;  // !=0 if want to draw multiselection ???
	uint32_t folder_block;   // auto formed, количество блоков данных входа каталога (БДВК) ????? format BDVK == bdfe,, // moved to scrollbar->max_area
	uint32_t start_draw_line;    // internal - top showed file n. moved to scrollbar->position and back
	uint16_t start_draw_cursor_line;  //internal
    void* folder_data;      // 32 byte - dir_header, +4 = number, +32 - bdvk[], size of rec(bdvk cp866) = 304byte
    uint32_t temp_counter; //internal
    uint32_t file_name_length; //internal
    uint32_t marked_file;  // have a mark 0/1 ?
    uint32_t extension_size;  //internal
    uint32_t extension_start;  //internal
    void* type_table; // type buffer
    char* ini_file_start;   // icons.ini contens - file<>icon association
    char* ini_file_end;     // start + filesize
    uint32_t draw_scroll_bar;  // 1 = need redraw sb after key(), user - resetted
    uint32_t font_size_xy;  // x_y	(6, 9)
    uint32_t mouse_keys;  // saved internal
    uint32_t mouse_keys_old; // saved internal
    uint32_t mouse_pos; // saved internal
    uint32_t mouse_keys_delta; // saved internal
    uint32_t mouse_key_delay; // 50
    uint32_t mouse_keys_tick; // internal timer
    uint16_t start_draw_cursor_line_2;  // internal cursor line
    uint32_t all_redraw;         // 0 - skip draw contens, 1 - force draw, 2 - no draw selection (but draw icons), used when scroll
    struct fsBDFE* selected_BDVK_adress;  // pointer to selected
    uint16_t key_action;   // fill before key(), 1..12, wiki
    uint16_t key_action_num; // fill before key()  fn2 >> 8
    char* name_temp_area;       // temporary string format buffer
    uint32_t max_name_temp_size;  // sizeof ^
    uint32_t display_name_max_length;  // autocounted
    uint32_t draw_panel_selection_flag;  // flag internal showing selected item
    uint32_t mouse_pos_old;  // saved internal
    uint32_t marked_counter;  // number of marked files
    char* keymap_pointer;  // keyboard layout map

} filebrowser;

static inline filebrowser* kolibri_filebrowser(filebrowser* fb, uint32_t x_w, uint32_t y_h, uint32_t font_size_xy, uint32_t icon_size_xy, void* icon_raw_area, void* icon_palette_raw, uint32_t icon_res,
                                         char* ini_file_start, char* ini_file_end,
                                         color_t font_select, color_t bk_select, color_t font_color, color_t bgcolor, color_t reduct_color)
{
    static char name_temp_area[256];
    static char keymap_area[128];
    static char type_table[128] = "<DIR> 1023b 00.00.00 00:00     temp1.asm";

    memset(fb, 0, sizeof(filebrowser));
    fb->x_w = x_w;
    fb->y_h = y_h;
    fb->font_size_xy = font_size_xy;

    fb->icon_size_xy = icon_size_xy;
    fb->icon_raw_area = icon_raw_area;
    fb->icon_palette_raw = icon_palette_raw;
    fb->icon_resolution_raw = icon_res;

    fb->line_size_y = (icon_size_xy & 0xFFFF) + 2;

    // constants
    fb->type_table = type_table;
    fb->mouse_key_delay = 50;
    fb->name_temp_area = name_temp_area;
//    fb->max_name_temp_size = sizeof name_temp_area - 1;
    fb->keymap_pointer = keymap_area;

    // careful - font sizes may be encoded in colors as SysFn4
    fb->background_color = bgcolor;  // self explained, 0xffffff
	fb->select_color = bk_select; // self explained, 0xbbddff
    fb->select_text_color = font_select; // self explained
    fb->text_color = font_color; // self explained
    fb->reduct_text_color = reduct_color; // 0xff0000  - spec color for cutted filenames

    fb->ini_file_start = ini_file_start;
    fb->ini_file_end = ini_file_end;
/*
    void* folder_data;      // 32 byte - dir_header, +4 = number, +32 - bdvk[], size of rec(bdvk cp866) = 304byte
*/
    return fb;
}

static inline filebrowser* kolibri_new_filebrowser(uint32_t x_w, uint32_t y_h, uint32_t font_size_xy, uint32_t icon_size_xy, void* icon_raw_area, void* icon_palette_raw, uint32_t icon_bpp,
                                         char* ini_file_start, char* ini_file_end,
                                         color_t font_select, color_t bk_select, color_t font_color, color_t bgcolor, color_t reduct_color)
{
    filebrowser *new_fb = (filebrowser *)malloc(sizeof(filebrowser));
    return kolibri_filebrowser(new_fb, x_w, y_h, font_size_xy, icon_size_xy, icon_raw_area, icon_palette_raw, icon_bpp, ini_file_start, ini_file_end,
                                         font_select, bk_select, font_color, bgcolor, reduct_color);
}

/* loading files leads to link functions even if no using filebrowser
inline filebrowser* kolibri_filebrowser_def(filebrowser* fb, uint32_t x_w, uint32_t y_h)
{
    // load default icons and icon association
    char icons_ini[]       = "/rd/1/File managers/icons.ini";
    char icons16img[]       = "/rd/1/File managers/z_icons.png";

    return kolibri_filebrowser(fb, x_w, y_h, X_Y(9, 16), X_Y(16, 16), icon_raw_area, icon_palette_raw, icon_res,
                                         ini_file_start, ini_file_end,
                                         0x000000, 0xbbddff, kolibri_color_table.color_work_text, kolibri_color_table.color_work_area, 0xff0000);
}
*/

inline void gui_add_filebrowser(kolibri_window *wnd, filebrowser* f)
{
    kolibri_window_add_element(wnd, KOLIBRI_FILEBROWSE, f);
}

extern void (*filebrowse_draw)(filebrowser *) __attribute__((__stdcall__));
extern void (*filebrowse_key)(filebrowser *) __attribute__((__stdcall__));
extern void (*filebrowse_mouse)(filebrowser *) __attribute__((__stdcall__));

__attribute__((__stdcall__)) static inline void filebrowser_key(filebrowser *fb, oskey_t keypress)
/// wrapper for key, translate keypress (ASCII mode) to action for browser
{
//    if (!fb->select_flag) return;  // same reaction as other controls

    int extended_key = 0, act = 0;

    if(keypress.state) return;
    if (keypress.code == 0xE0){ extended_key = 1; return; }

    act = 0;
    switch(keypress.ctrl_key)  // ascii scancode
    {
    case 80: // arrow down
        act = 1; break;
    case 72: // arrow up
        act = 2; break;
    case 81: // PageDown
        act = 3; break;
    case 73: // PageUp
        act = 4; break;
    case 71: // Home
        act = 5; break;
    case 79: // End
        act = 6; break;
    case 28: // Enter
        act = 7; break;
    case 82: // Insert
        act = 8; break;
    case 78: // NumPad+   select all
        act = 9; break;
    case 74: // NumPad-   deselct
        act = 10; break;
    case 55: // NumPad*  invert selection
        act = 11; break;
    default:
        act = 12; // search by letter
    }
    fb->key_action = act;
    fb->key_action_num = keypress.ctrl_key;

//    debug_board_printf("key pressed [%X] %d, action %d, ext_flag = %d\n", keypress.val, brows.key_action_num, act, extended_key);

    if (extended_key) extended_key = 0;
    (*filebrowse_key)(fb);
}


#endif /* KOLIBRI_FILEBROWSE_H */
