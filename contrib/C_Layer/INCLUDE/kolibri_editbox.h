#ifndef KOLIBRI_EDITBOX_H
#define KOLIBRI_EDITBOX_H

#include "kolibri_colors.h"

/// @brief flags meaning
enum EditBoxFlags
{
    ed_pass =                        0b1,
    ed_focus =                      0b10,
    ed_shift =                     0b100,
    ed_shift_on =                 0b1000,
    ed_shift_bac =               0b10000,
    ed_left_fl =                0b100000,
    ed_offset_fl =             0b1000000,
    ed_insert =               0b10000000,
    ed_mouse_on            = 0b100000000,
    ed_ctrl_on            = 0b1000000000,
    ed_alt_on            = 0b10000000000,
    ed_disabled =         0b100000000000,
    ed_always_focus  = 0b100000000000000,
    ed_figure_only  = 0b1000000000000000,
    ed_shift_on_off = 0b1111111111110111,
    ed_shift_off = 0b1111111111111011,
    ed_shift_bac_cl = 0b1111111111101111,
    ed_shift_cl = 0b1111111111100011,
    ed_shift_mcl = 0b1111111111111011,
    ed_right_fl = 0b1111111111011111,
    ed_offset_cl = 0b1111111110111111,
    ed_insert_cl = 0b1111111101111111,
    ed_mous_adn_b = 0b100011000,
    ed_mouse_on_off = 0b1111111011111111,
    ed_mouse_on_off = !(ed_mouse_on),
    ed_ctrl_off = !(ed_ctrl_on),
    ed_alt_off = !(ed_alt_on),
    
};

typedef struct edit_box_t {
  unsigned int width;
    unsigned int left;
    unsigned int top;
    unsigned int color;
    /// @brief selected text color
    unsigned int shift_color;
    unsigned int focus_border_color;
    unsigned int blur_border_color;
    unsigned int text_color;
    unsigned int max;
    char        *text;
    /// @note  must be pointer edit_box** to save focused editbox
    void        *mouse_variable;
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
} edit_box;

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
/// ���� flags �� �������� ed_focus, ���������� ����
/// ���� flags �������� ed_mouse_on ��� ed_disabled, ���������� ����
/// �� ����� ������� ch - ��� �������, ������ � ������ ASCII
{
    __asm__ __volatile__ (
             "push %2\n\t"
             "call *%1 \n\t"::"a"(ch.val), "m"(edit_box_key), "m"(e):);
}

extern void (*edit_box_mouse)(edit_box *) __attribute__((__stdcall__));
/// ��� ������ �� ����� �������, �������� *mouse_variable! � ���������� ���� ed_mouse_on


extern void (*edit_box_set_text)(edit_box *, char *) __attribute__((__stdcall__));
extern volatile unsigned press_key;
#endif /* KOLIBRI_EDITBOX_H */
