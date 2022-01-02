// writed by maxcodehack and superturbocat2001
// adaptation of clayer for ktcc
#ifndef KOLIBRI_BOXLIB_H
#define KOLIBRI_BOXLIB_H

typedef unsigned color_t;

extern int kolibri_boxlib_init(void);

/*  flags meaning */
#define ed_figure_only 0b1000000000000000   // одни символы
#define ed_always_focus 0b100000000000000   // всегда с курсором (фокусом)
#define ed_focus                     0b10    // фокус ввода приложения, мышится самостоятельно
#define ed_pass                       0b1    // поле с паролем
#define ed_shift_on                0b1000    // если не установлен -значит впервые нажат shift,если был установлен, значит мы уже что - то делали удерживая //shift
#define ed_shift_on_off 0b1111111111110111
#define ed_shift                     0b100    //включается при нажатии на shift т.е. если нажимаю
#define ed_shift_off    0b1111111111111011
#define ed_shift_bac               0b10000   //бит для очистки выделеного shift т.е. при установке говорит что есть выделение
#define ed_shift_bac_cl 0b1111111111101111  //очистка при удалении выделения
#define ed_shift_cl     0b1111111111100011
#define ed_shift_mcl    0b1111111111111011
#define ed_left_fl                0b100000
#define ed_right_fl     0b1111111111011111
#define ed_offset_fl             0b1000000
#define ed_offset_cl    0b1111111110111111
#define ed_insert               0b10000000
#define ed_insert_cl    0b1111111101111111
#define ed_mouse_on            0b100000000
#define ed_mous_adn_b          0b100011000
#define ed_mouse_off          ~ed_mouse_on
#define ed_ctrl_on            0b1000000000
#define ed_ctrl_off            ~ed_ctrl_on
#define ed_alt_on            0b10000000000
#define ed_alt_off             ~ed_alt_on
#define ed_disabled         0b100000000000

// SCROLLBAR
typedef struct {
	uint16_t xsize;
    uint16_t xpos;
    uint16_t ysize;
    uint16_t ypos;
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
} __attribute__ ((__packed__)) scrollbar;

extern void (*scrollbar_h_draw __attribute__((__stdcall__)))(scrollbar*);
extern void (*scrollbar_h_mouse __attribute__((__stdcall__)))(scrollbar*);
extern void (*scrollbar_v_draw __attribute__((__stdcall__)))(scrollbar*);
extern void (*scrollbar_v_mouse __attribute__((__stdcall__)))(scrollbar*);

// CHECKBOX
typedef struct {
    unsigned int left_s;
    unsigned int top_s;
    unsigned int ch_text_margin;
    unsigned int color;
    unsigned int border_color;
    unsigned int text_color;
    char *text;
    unsigned int flags;

    /* Users can use members above this */
    unsigned int size_of_str;
}check_box;

extern void (*check_box_draw2  __attribute__((__stdcall__)))(check_box *);
extern void (*check_box_mouse2  __attribute__((__stdcall__)))(check_box *);
extern void (*init_checkbox2 __attribute__((__stdcall__)))(check_box *);

// DBUTTON
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

extern void (*dynamic_button_draw __attribute__((__stdcall__)))(pict_button *);
extern void (*dynamic_button_mouse __attribute__((__stdcall__)))(pict_button *);

// EDITBOX

#pragma pack(push,1)
typedef struct edit_box_t {
    unsigned int width;
    unsigned int left;
    unsigned int top;
    unsigned int color;
    unsigned int shift_color;   // selected text color
    unsigned int focus_border_color;
    unsigned int blur_border_color;
    unsigned int text_color;
    unsigned int max;
    char        *text;
    void        *mouse_variable; // must be pointer edit_box** to save focused editbox
    unsigned int flags;

    unsigned int size;  // used symbols in buffer without trailing zero
    unsigned int pos;  // cursor position
/* The following struct members are not used by the users of API */
    unsigned int offset;
    unsigned int cl_curs_x;
    unsigned int cl_curs_y;
    unsigned int shift;
    unsigned int shift_old;
    unsigned int height;
    unsigned int char_width;
}edit_box;
#pragma pack(pop)

extern void (*edit_box_draw  __attribute__((__stdcall__)))(edit_box *);
extern void edit_box_key (edit_box *, unsigned int key_val)__attribute__((__stdcall__));
extern void (*edit_box_mouse  __attribute__((__stdcall__)))(edit_box *);
extern void (*edit_box_set_text  __attribute__((__stdcall__)))(edit_box *, char *);

// FRAME
typedef struct {
	uint32_t type;
	uint32_t x_w;
	uint32_t y_h;
	color_t ext_col;
	color_t int_col;
	uint32_t flags;
	char *text_pointer;
	uint32_t text_position;
	uint32_t font_number;
	uint32_t font_size_y;
	color_t font_color;
	color_t font_bg_color;
}frame;

extern void (*frame_draw)(frame *);

// MENUBAR
typedef struct
{
	uint32_t type;   // 1 åñëè íåò ïîäìåíþ, ïðîñòî ïóíêò

	uint32_t x_w;   // âåðõíèé ïóíêò
	uint32_t y_h;

	char* text_pointer;
	char* pos_pointer;
	char* text_end;
	uint32_t mouse_pos;
	uint32_t mouse_keys;

	uint32_t x_w1;  // ïîäìåíþ
	uint32_t y_h1;

	color_t bckg_col;  // ôîí âåðõíåãî ïóêòà
	color_t frnt_col;  // ôîí âûáðàííîãî âåðõíåãî ïóíêòà
	color_t menu_col;  // ôîí âûïàäàþùåé ÷àñòè (ïîäïóêòû)
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
	color_t menu_sel_col;  // öâåò ôîíà âûáðàííîãî ïîäïóíêòà
	color_t bckg_text_col; // öâåò øðèôòà íåâûáðàííîãî ïóíêòà
	color_t frnt_text_col;  // öâåò øðèôòà âûáðàííîãî ïóíêòà
	uint32_t mouse_keys_old;
	uint32_t font_height;
	uint32_t cursor_out;
	uint32_t get_mouse_flag;
} menubar;

extern void (*menu_bar_draw)(menubar *);
extern void (*menu_bar_mouse)(menubar *);
extern void (*menu_bar_activate)(menubar *);

// OPTIONBOX
typedef struct option_box_t {
    struct option_box_t **selected;
    uint16_t posx;
    uint16_t posy;
    uint32_t text_margin; // = 4 ðàññòîÿíèå îò ïðÿìîóãîëüíèêà ÷åê áîêñà äî íàäïèñè
    uint32_t size;       // 12 ðàçìåð êâàäðàòà ÷åê áîêñà
    color_t color;
    color_t border_color; // individual border
    color_t text_color;
    char *text;
    uint32_t text_len;
    uint32_t flags;
} __attribute__ ((__packed__)) option_box;

extern void (*option_box_draw __attribute__((__stdcall__)))(option_box **);
extern void (*option_box_mouse __attribute__((__stdcall__)))(option_box **);

// PATHSHOW
typedef struct {
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
} __attribute__ ((__packed__)) pathview;

extern void (*path_show_prepare  __attribute__((__stdcall__)))(pathview *);
extern void (*path_show_draw __attribute__((__stdcall__)))(pathview *);

// PROGRESSBAR
typedef struct {
	unsigned int value;
    unsigned int left;
    unsigned int top;
    unsigned int width;
    unsigned int height;
    unsigned int style;
    unsigned int min;
    unsigned int max;
    unsigned int back_color;
    unsigned int progress_color;
    unsigned int frame_color;
} progressbar;

extern void (*progressbar_draw __attribute__((__stdcall__)))(progressbar *);
extern void (*progressbar_progress __attribute__((__stdcall__)))(progressbar *);


#endif /* KOLIBRI_BOXLIB_H */
