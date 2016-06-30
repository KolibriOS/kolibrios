#ifndef KOLIBRI_CHECKBOX_H
#define KOLIBRI_CHECKBOX_H

#include "kolibri_colors.h"

enum CHECKBOX_FLAGS {
     CHECKBOX_IS_SET = 0x00000002
     /* Add more flags later */
};

struct check_box {
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
};

struct check_box* kolibri_new_check_box(unsigned int tlx, unsigned int tly, unsigned int sizex, unsigned int sizey, char *label_text)
{
     struct check_box* new_checkbox = (struct check_box *)malloc(sizeof(struct check_box));
     new_checkbox -> left_s = (tlx << 16) + sizex;
     new_checkbox -> top_s  = (tly << 16) + sizey;
     new_checkbox -> ch_text_margin = 10;
     new_checkbox -> color = 0xFFFFFFFF;
     new_checkbox -> border_color = kolibri_color_table.color_work_graph;
     new_checkbox -> text_color = kolibri_color_table.color_work_text;
     new_checkbox -> text = label_text;
     new_checkbox -> flags = 0x00000008;

     return new_checkbox;
}

extern void (*check_box_draw2)(struct check_box *) __attribute__((__stdcall__));
extern void (*check_box_mouse2)(struct check_box *)__attribute__((__stdcall__));

#endif /* KOLIBRI_CHECKBOX_H */
