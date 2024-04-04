#ifndef KOLIBRI_BUF2D_H
#define KOLIBRI_BUF2D_H

#include <sys/ksys.h>

/*ToDo
 * voxel function
 */

/// @brief Inilizate buf2d Library
/// @return -1 if unsuccess
extern int kolibri_buf2d_init(void);

typedef struct __attribute__ ((__packed__)) 
{
	/// @brief Pointer to buffer
	unsigned int *buf_pointer;
	/// @brief 
	uint16_t left;
	uint16_t top;
	/// @brief Buffer width
	unsigned int width;
	/// @brief Buffer heigth
	unsigned int height;
	/// @brief Background color
	unsigned int bgcolor;
	uint8_t color_bit;
} buf2d_struct;

enum BUF2D_ALGORITM_FILTR {
	SIERRA_LITE,
	FLOYD_STEINBERG,
	BURKERS,
	HEAVYIRON_MOD,
	ATKINSON
};

enum BUF2D_OPT_CROP {
	BUF2D_OPT_CROP_TOP = 1,
	BUF2D_OPT_CROP_LEFT = 2,
	BUF2D_OPT_CROP_BOTTOM = 4,
	BUF2D_OPT_CROP_RIGHT = 8
};

/// @brief Params for function buf2d_resize
enum ResizeParams
{
	BUF2D_Resize_ChangeSize = 1,
	/// @brief Change image in buffer
	BUF2D_Resize_ChangeImage = 2
};

/// @brief Creates a buffer and clears it with the background color
/// @param buffer Pointer to buf2d_struct
extern void (*buf2d_create_asm)(buf2d_struct * buffer) __attribute__((__stdcall__));

/// @brief Draws a segment of a bezier curve using three points.
/// @param buffer Poiter to buf2d_struct
/// @param p1 point 1
/// @param p2 point 2
/// @param p3 point 3
/// @param color Color of curve
extern void (*buf2d_curve_bezier_asm)(buf2d_struct *buffer, ksys_pos_t p1, ksys_pos_t p2, ksys_pos_t p3, ksys_color_t color) __attribute__((__stdcall__));

buf2d_struct* buf2d_create(uint16_t tlx, uint16_t tly, unsigned int sizex, unsigned int sizey, ksys_color_t font_bgcolor, uint8_t color_bit)
{
    buf2d_struct *new_buf2d_struct = (buf2d_struct *)malloc(sizeof(buf2d_struct));
    new_buf2d_struct -> left = tlx;
	new_buf2d_struct -> top = tly;
	new_buf2d_struct -> width = sizex;
	new_buf2d_struct -> height = sizey;
	new_buf2d_struct -> bgcolor = font_bgcolor;
	new_buf2d_struct -> color_bit = color_bit;
	buf2d_create_asm(new_buf2d_struct);
    return new_buf2d_struct;
}

void buf2d_curve_bezier(buf2d_struct *buf, unsigned int p0_x, unsigned int p0_y, unsigned int p1_x, unsigned int p1_y, unsigned int p2_x, unsigned int p2_y, ksys_color_t color)
{
	buf2d_curve_bezier_asm(buf, (p0_x<<16)+p0_y, (p1_x<<16)+p1_y, (p2_x<<16)+p2_y, color);
}

/// @brief Draws a buffer on the screen (works through system f. 7).
/// @param buffer Poiter to buf2d_struct
/// @note Only buffers with a color depth of 24 bits are drawn.
extern void (*buf2d_draw)(buf2d_struct *buffer) __attribute__((__stdcall__));

/// @brief Clear buffer with specified color
/// @param buffer Poiter to buf2d_struct
/// @param color Color
extern void (*buf2d_clear)(buf2d_struct *buffer, ksys_color_t color) __attribute__((__stdcall__));

/// @brief Frees memory occupied by the buffer image.
/// @param buffer Poiter to buf2d_struct
extern void (*buf2d_delete)(buf2d_struct *buffer) __attribute__((__stdcall__));

/// @brief Rotates the buffer 90 or 180 degrees
/// @param buffer Poiter to buf2d_struct
/// @param angle number, 90 or 180
extern void (*buf2d_rotate)(buf2d_struct * buffer, unsigned int angle) __attribute__((__stdcall__));

/// @brief Changes the size of a buffer or image in a buffer
/// @param buffer Poiter to buf2d_struct
/// @param newWidth New buffer width
/// @param newHeight New buffer height
/// @param param param, value from enum ResizeParams
extern void (*buf2d_resize)(buf2d_struct *buffer, unsigned int newWidth, unsigned int newHeight, unsigned int param) __attribute__((__stdcall__));

/// @brief Draws a line in the buffer with the specified color and coordinates
/// @param buffer Poiter to buf2d_struct
extern void (*buf2d_line)(buf2d_struct *buffer, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int) __attribute__((__stdcall__));

/// @brief Draws a line in the buffer and takes the same parameters as the buf2d_line function. This function draws a smoother line that is more beautiful than buf2d_line, but is slower due to the calculation of mixed colors of the background and the line itself.
/// @param buffer Poiter to buf2d_struct
extern void (*buf2d_line_sm)(buf2d_struct *buffer, unsigned int, unsigned int, unsigned int, unsigned int) __attribute__((__stdcall__));

/// @brief Draws a rectangular frame in two coordinates, the 2nd coordinate is specified by size. If the dimensions are negative, the frame is drawn in the opposite direction
/// @param buffer Poiter to buf2d_struct
/// @param coord_x coord of rectangular frame by X axis
/// @param coord_y coord of rectangular frame by Y axis
/// @param width 
/// @param height
/// @param color Color of rectangular frame
/// @note for negative values coord_x and coord_y these are the coordinates of the lower right corner
extern void (*buf2d_rect_by_size)(buf2d_struct *buffer, unsigned int coord_x, unsigned int coord_y, unsigned int width, unsigned int height, ksys_color_t color) __attribute__((__stdcall__));

/// @brief Draws a filled rectangle in two coordinates, the 2nd coordinate is specified by size
/// @param buffer Poiter to buf2d_struct
/// @param coord_x coord of rectangle by X axis
/// @param coord_y coord of rectangle by Y axis
/// @param width
/// @param height
/// @param color Color of rectangle
/// @note If the size is set to 1*1, then a dot of 1 pixel size will be drawn, i.e. the size of the rectangle in pixels will be equal to the specified one
/// @note If the dimensions are negative, the rectangle is drawn in the opposite direction.
extern void (*buf2d_filled_rect_by_size)(buf2d_struct *buffer, unsigned int coord_x, unsigned int coord_y, unsigned int width, unsigned int height, ksys_color_t color) __attribute__((__stdcall__));

/// @brief Draws a circle with the specified color and radius in the buffer
/// @param buffer Poiter to buf2d_struct
/// @param coord_x coord of circle by X axis
/// @param coord_y coord of circle by Y axis
/// @param color Color of circle
extern void (*buf2d_circle)(buf2d_struct *buffer, unsigned int coord_x, unsigned int coord_y, unsigned int radius, ksys_color_t color) __attribute__((__stdcall__));

/// @brief Compresses the image in the buffer by 2 times in height, while the size of the buffer itself does not decrease.
/// @param buffer Poiter to buf2d_struct
extern void (*buf2d_img_hdiv2)(buf2d_struct *buffer) __attribute__((__stdcall__));

/// @brief Compresses the width of the image in the buffer by 2 times, while the size of the buffer itself does not decrease.
/// @param buffer Poiter to buf2d_struct
extern void (*buf2d_img_wdiv2)(buf2d_struct *buffer) __attribute__((__stdcall__));

/// @brief Converting a buffer from 24-bit to 8-bit. When converting, you indicate which color to take: 0-blue, 1-green, 2-red. Other colors are lost during conversion.
/// @param buffer Poiter to buf2d_struct
/// @param color what color to take: 0-blue, 1-green, 2-red.
extern void (*buf2d_conv_24_to_8)(buf2d_struct *buffer, unsigned int color) __attribute__((__stdcall__));

/// @brief Converting a buffer from 24-bit to 32-bit. The conversion also specifies an 8-bit buffer that will be used to create the alpha channel.
/// @param buffer Poiter to buf2d_struct
extern void (*buf2d_conv_24_to_32)(buf2d_struct *buffer, unsigned int) __attribute__((__stdcall__));

/// @brief Draws an image from another buffer to the buffer at the specified coordinates
/// @param bufferA Poiter to buf2d_struct
/// @param coord_x determines the position on the X axis of the picture to be drawn in bufferA.
/// @param coord_y determines the position on the Y axis of the picture to be drawn in bufferA.
/// @param bufferB Poiter to buf2d_struct
/// @note The buffer in which you draw (receiver) must be 24-bit, and the one in which you draw (source) must be 24 or 32-bit
/// @note If the source buffer is 32-bit, then its transparency is not taken into account when drawing; to take into account transparency, the buf2d_bit_blt_transp function is used
extern void (*buf2d_bit_blt)(buf2d_struct *bufferA, unsigned int coord_x, unsigned int coord_y, buf2d_struct *bufferB) __attribute__((__stdcall__));

/// @brief Draws an image from another buffer into the buffer at the specified coordinates, taking into account transparency
/// @param bufferA Poiter to buf2d_struct
/// @param coord_x determines the position on the X axis of the picture to be drawn in bufferA.
/// @param coord_y determines the position on the Y axis of the picture to be drawn in bufferA.
/// @param bufferB Poiter to buf2d_struct
/// @note The buffer that will be drawn must be 32-bit, and the one in which it is drawn must be 24-bit.
extern void (*buf2d_bit_blt_transp)(buf2d_struct *bufferA, unsigned int coord_x, unsigned int coord_y, buf2d_struct *bufferB) __attribute__((__stdcall__));

/// @brief Draws an image from another buffer into the buffer at the specified coordinates, taking into account transparency
/// @param bufferA Poiter to buf2d_struct
/// @param coord_x determines the position on the X axis of the picture to be drawn in bufferA.
/// @param coord_y determines the position on the Y axis of the picture to be drawn in bufferA.
/// @param bufferB Poiter to buf2d_struct
/// @note The buffer that will be drawn must be 8-bit, and the one in which it is drawn must be 24-bit.
extern void (*buf2d_bit_blt_alpha)(buf2d_struct *bufferA, unsigned int coord_x, unsigned int coord_y, buf2d_struct *bufferB) __attribute__((__stdcall__));

/// @brief Converts a matrix with text of size 16*16 to size 1*256. It is necessary to create matrices for drawing text.
/// @param buffer Poiter to buf2d_struct
/// @note Before using this function, it is assumed that there is an image with a full set of characters measuring 16 columns by 16 rows, from which an 8-bit buffer was previously created
extern void (*buf2d_convert_text_matrix)(buf2d_struct *buffer) __attribute__((__stdcall__));

/// @brief Draws text in a buffer using a 1*256 character text matrix.
/// @param bufferA Structure of the buffer in which the text will be drawn
/// @param bufferB buffer structure with a text matrix in 8-bit format, size 1 * 256 characters
/// @param text text that will be output to bufferA
/// @param color
extern void (*buf2d_draw_text)(buf2d_struct *bufferA, buf2d_struct *bufferB, const char *text, unsigned int, ksys_color_t color) __attribute__((__stdcall__));

/// @brief Trimming the buffer according to the specified color. The function is used to reduce the memory occupied by the buffer. Отрезаются крайние части буфера имеющие одинаковый цвет
/// @param buffer structure of the buffer that will be trimmed
/// @param color color by which the buffer will be cut
/// @param param constants indicating which sides to trim the buffer from, value from enum BUF2D_OPT_CROP
extern void (*buf2d_crop_color)(buf2d_struct *buffer, ksys_color_t color, unsigned int param) __attribute__((__stdcall__));

/// @brief Shift the image in the buffer up or down in height
/// @param buffer Poiter to buf2d_struct
extern void (*buf2d_offset_h)(buf2d_struct *buffer, unsigned int, unsigned int, unsigned int) __attribute__((__stdcall__));

/// @brief Function for recursively filling with the selected color. There are two filling options
/// @param buffer Pointer to buf2d_struct
/// @param start_x coords
/// @param start_y coords
/// @param algo 0 or 1
/// @param color depends on the value of algo. 1) if 0 - the color to which the area will be filled; 2) if 1 - the color of the area that will be filled;
extern void (*buf2d_flood_fill)(buf2d_struct *buffer, unsigned int start_x, unsigned int start_y, unsigned int algo, ksys_color_t color) __attribute__((__stdcall__));

/// @brief The function places a point
/// @param buffer Poiter to buf2d_struct
/// @param x coord of point
/// @param y coord of point
/// @param color color of point
extern void (*buf2d_set_pixel)(buf2d_struct *buffer, unsigned int x, unsigned int y, ksys_color_t color) __attribute__((__stdcall__));

/// @brief Get pixel color
/// @param buffer Pointer to buf2d_struct
/// @param x coord of pixel
/// @param y coord of pixel
extern unsigned int (*buf2d_get_pixel)(buf2d_struct *buffer, unsigned int x, unsigned int y) __attribute__((__stdcall__));


extern void (*buf2d_flip_h)(buf2d_struct *buffer) __attribute__((__stdcall__));

/// @brief Flips the image in the buffer vertically (top and bottom are swapped)
/// @param buffer 24-bit buffer structure
extern void (*buf2d_flip_v)(buf2d_struct *buffer) __attribute__((__stdcall__));

extern void (*buf2d_filter_dither)(buf2d_struct *, unsigned int) __attribute__((__stdcall__));

/*!
 * \brief buf2d lib
 * <a href="http://wiki.kolibrios.org/wiki/Buf2d/ru">wiki</a>
 */

#endif /* KOLIBRI_BUF2D_H */
