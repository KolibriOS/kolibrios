#ifndef KOLIBRI_EDITBOX_H
#define KOLIBRI_EDITBOX_H

#include "kolibri_colors.h"

struct edit_box {
  unsigned int width; 
    unsigned int left;
    unsigned int top; 
    unsigned int color; 
    unsigned int shift_color; 
    unsigned int focus_border_color; 
    unsigned int blur_border_color;
    unsigned int text_color;
    unsigned int max;
    char *text;
    unsigned int mouse_variable;
    unsigned int flags;

/* The following struct members are not used by the users of API */
    unsigned int size;
    unsigned int pos;
    unsigned int offset;
    unsigned int cl_curs_x;
    unsigned int cl_curs_y;
    unsigned int shift;
    unsigned int shift_old;
};

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

struct edit_box* kolibri_new_edit_box(unsigned int tlx, unsigned int tly, unsigned int max_chars)
{
    unsigned int PIXELS_PER_CHAR = 7;
    struct edit_box *new_textbox = (struct edit_box *)malloc(sizeof(struct edit_box));
    char *text_buffer = (char *)calloc(max_chars + 1, sizeof(char));

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
    new_textbox -> mouse_variable = 1; /* let the mouse take control? */
    new_textbox -> flags = 0x00004002; /*ed_focus + ed_always_focus */

    /* If these lines are uncommented, the executable will crash for no reason at start */
    /* Even though these lines are not ever read it ALWAYS causes a crash, even crashes MTDBG. What gives? */

    new_textbox -> size = 0;
    new_textbox -> pos = 0;
    new_textbox -> offset = 0;
    new_textbox -> cl_curs_x = 0;
    new_textbox -> cl_curs_y = 0;
    new_textbox -> shift = 0;
    new_textbox -> shift_old = 0;

    return new_textbox;
}

extern void (*edit_box_draw)(struct edit_box *) __attribute__((__stdcall__));

/* editbox_key is a wrapper written in assembly to handle key press events for editboxes */
/* because inline assembly in GCC is a PITA and interferes with the EAX (AH) register */
/* which edit_box_key requires */
extern void editbox_key(struct edit_box *) __attribute__((__stdcall__));

extern void (*edit_box_mouse)(struct edit_box *) __attribute__((__stdcall__));

#endif /* KOLIBRI_EDITBOX_H */
