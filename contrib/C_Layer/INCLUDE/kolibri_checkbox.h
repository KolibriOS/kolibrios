#ifndef KOLIBRI_CHECKBOX_H
#define KOLIBRI_CHECKBOX_H

#include "kolibri_colors.h"

enum CHECKBOX_FLAGS {
     CHECKBOX_IS_SET = 0x00000002
     /* Add more flags later */
};

typedef struct {
	union 
	{
		/// @brief left padding + width (in format: x position * 65536 + x length)
		unsigned int left_s;
		struct 
		{
			uint16_t x;
			uint16_t width;
		};
		
	};
	union 
	{
		/// @brief top margin + height (in the format: y position * 65536 + y length).
		unsigned int top_s;
		struct
		{
			uint16_t y;
			uint16_t height;
		};
		
	};

	/// @brief distance from the check box rectangle to the inscription
	unsigned int ch_text_margin;

	/// @brief color inside checkbox
	unsigned int color;

	/// @brief Border color
    unsigned int border_color;

	/// @brief text color
    unsigned int text_color;

	/// @brief Pointer to text
    char *text;

	/// @brief Flags
    unsigned int flags;

	/// @brief length of the string, calculated when the component is initialized
    /// @note Users can use members above this 
    unsigned int size_of_str;
} check_box;

/// @brief This function should be called when entering information from the keyboard.
/// @param checkbox Pointer to checkbox
extern void (*check_box_draw2)(check_box * checkbox) __attribute__((__stdcall__));

/// @brief This function should be called when entering information from the mouse.
/// @param checkbox Pointer to checkbox
extern void (*check_box_mouse2)(check_box * checkbox)__attribute__((__stdcall__));

/// @brief This function should be called when the application is initialized; in fact, it counts the number of characters in the drain for output. If you don’t need to display signatures for the checkbox, then you don’t have to call it.
/// @param checkbox Pointer to checkbox
extern void (*init_checkbox2)(check_box * checkbox)__attribute__((__stdcall__));

/// @brief Create check_box
/// @param tlx 
/// @param tly 
/// @param sizex 
/// @param sizey
/// @param label_text
/// @return Pointer to new checkbox
check_box* kolibri_new_check_box(unsigned int tlx, unsigned int tly, unsigned int sizex, unsigned int sizey, char *label_text)
{
     check_box* new_checkbox = (check_box *)malloc(sizeof(check_box));
     new_checkbox -> left_s = (tlx << 16) + sizex;
     new_checkbox -> top_s  = (tly << 16) + sizey;
     new_checkbox -> ch_text_margin = 10;
     new_checkbox -> color = kolibri_color_table.color_work_area | 0x80000000; // 0xFFFFFFFF; // 0x80AABBCC, 31-bit mus be set asciiz
     new_checkbox -> border_color = kolibri_color_table.color_work_graph;
     new_checkbox -> text_color = kolibri_color_table.color_work_text;
     new_checkbox -> text = label_text;
     new_checkbox -> flags = 0x00000008;

     (*init_checkbox2)(new_checkbox); // count text width for mouse action and set flags

     return new_checkbox;
}


#endif /* KOLIBRI_CHECKBOX_H */
