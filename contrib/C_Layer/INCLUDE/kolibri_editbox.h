#ifndef KOLIBRI_EDITBOX_H
#define KOLIBRI_EDITBOX_H

#include "kolibri_colors.h"

/*  flags meaning
ed_figure_only= 1000000000000000b   ;одни символы
ed_always_focus= 100000000000000b   // всегда с курсором (фокусом)
ed_focus=                     10b   ;фокус ввода приложения, мышится самостоятельно
ed_pass=                       1b   ;поле с паролем
ed_shift_on=                1000b   ;если не установлен -значит впервые нажат shift,если был установлен, значит мы уже что - то делали удерживая shift
ed_shift_on_off=1111111111110111b
ed_shift=                    100b   ;включается при нажатии на shift т.е. если нажимаю
ed_shift_off=   1111111111111011b
ed_shift_bac=              10000b   ;бит для очистки выделеного shift т.е. при установке говорит что есть выделение
ed_shift_bac_cl=1111111111101111b   ;очистка при удалении выделения
ed_shift_cl=    1111111111100011b
ed_shift_mcl=   1111111111111011b
ed_left_fl=               100000b
ed_right_fl=    1111111111011111b
ed_offset_fl=            1000000b
ed_offset_cl=   1111111110111111b
ed_insert=              10000000b
ed_insert_cl=   1111111101111111b
ed_mouse_on =          100000000b
ed_mous_adn_b=         100011000b
ed_mouse_on_off=1111111011111111b
ed_mouse_on_off= not (ed_mouse_on)
ed_ctrl_on =          1000000000b
ed_ctrl_off = not (ed_ctrl_on)
ed_alt_on =          10000000000b
ed_alt_off = not (ed_alt_on)
ed_disabled=        100000000000b
*/

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

/* Initializes an Editbox with sane settings, sufficient for most use.
   This will let you create a box and position it somewhere on the screen.
   The text_buffer is a pointer to a character array and needs to be as long as
   AT LEAST MAX_CHARS + 1.If the text_buffer is smaller, it will crash if user
   types more characters than what will fit into the text buffer.

   Allocating buffer space automatically so that programmer can be carefree now.
   This also automatically adjusts the size of edit box so that it can hold enough characters.

   All you need is :

   tlx,tly = Coordinates of the beginning of the edit box.
   max_chars = Limit of number of characters user can enter into edit box.
*/

edit_box* kolibri_new_edit_box(unsigned int tlx, unsigned int tly, unsigned int max_chars, void *editbox_interlock)
{
    unsigned int PIXELS_PER_CHAR = 7;
    edit_box *new_textbox = (edit_box *)calloc(1, sizeof(edit_box));
    char *text_buffer = (char *)calloc(max_chars + 2, sizeof(char)); // +2 as asked in box_lib src

    /* Update blur_border_color and shift_color from box_lib.mac macro */
    /* edit_boxes_set_sys_color */

    new_textbox -> width = max_chars * PIXELS_PER_CHAR;
    new_textbox -> left = tlx;
    new_textbox -> top = tly;
    new_textbox -> color = 0xFFFFFF; /* Always make white edit boxes */
    new_textbox -> shift_color = 0x6a9480;
    new_textbox -> focus_border_color = kolibri_color_table.color_work_graph;
    new_textbox -> blur_border_color = 0x6a9480;
    new_textbox -> text_color = kolibri_color_table.color_work_text; /* Always black text when typing */
    new_textbox -> max = max_chars;
    new_textbox -> text = text_buffer;
    new_textbox -> mouse_variable = editbox_interlock;
    new_textbox -> flags = 0x00000000;

    return new_textbox;
}

extern void (*edit_box_draw)(edit_box *) __attribute__((__stdcall__));

extern void (*edit_box_key)(edit_box *) __attribute__((__stdcall__));
/* editbox_key is a wrapper written in assembly to handle key press events for editboxes */
/* because inline assembly in GCC is a PITA and interferes with the EAX (AH) register */
/* which edit_box_key requires */
__attribute__((__stdcall__)) void editbox_key(edit_box *e, oskey_t ch)
/// если flags не содержит ed_focus, игнорирует ввод
/// если flags содержит ed_mouse_on или ed_disabled, игнорирует ввод
/// на вводе ожидает ch - код клавиши, только в режиме ASCII
{
    __asm__ __volatile__ (
             "push %2\n\t"
             "call *%1 \n\t"::"a"(ch.val), "m"(edit_box_key), "m"(e):);
}

extern void (*edit_box_mouse)(edit_box *) __attribute__((__stdcall__));
/// при щелчке не левой кнопкой, обнуляет *mouse_variable! и сбрасывает флаг ed_mouse_on


extern void (*edit_box_set_text)(edit_box *, char *) __attribute__((__stdcall__));
extern volatile unsigned press_key;
#endif /* KOLIBRI_EDITBOX_H */
