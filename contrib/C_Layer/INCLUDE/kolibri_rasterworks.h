#ifndef KOLIBRI_RASTERWORKS_H
#define KOLIBRI_RASTERWORKS_H

#include <sys/ksys.h>

/// @brief Encoding codes
enum RasterworksEncoding
{
	Rasterworks_cp688 = 1,
	Rasterworks_UTF8 = 2,
	Rasterworks_UTF16LE = 3,
};

/// @brief List of parameters
enum RasterworksParams
{
  /// @brief Bold text
  Bold = 0b1,

  /// @brief Italics
  Italic = 0b10,

  /// @brief Underscore
  Underline = 0b100,

  /// @brief Strikethrough
  StrikeThrough = 0b1000,

  /// @brief Right alignment
  AlignRight = 0b00010000,

  /// @brief Center alignment
  AlignCenter = 0b00100000,

  /// @brief 32bpp canvas insted of 24bpp
  Use32bit = 0b010000000
};

/// @brief Initialize the RasterWorks library
/// @return -1 if unsuccessful
extern int kolibri_rasterworks_init(void);

/// @brief Draw text on 24bpp or 32bpp image
/// @param canvas Pointer to image (array of colors)
/// @param x Coordinate of the text along the X axis
/// @param y Coordinate of the text along the Y axis
/// @param string Pointer to string
/// @param charQuantity String length
/// @param fontColor Text color
/// @param params ffeewwhh 
/// hh - char height 
/// ww - char width	; 0 = auto (proportional)
/// ee - encoding	; 1 = cp866, 2 = UTF-16LE, 3 = UTF-8
/// ff - Parameters from the RasterworksParams list
/// @note All flags combinable, except align right + align center
/// @note The text is drawn on the image, in order for changes to occur in the window, you need to draw the image after calling this function
extern void (*drawText)(void *canvas, int x, int y, const char *string, int charQuantity, ksys_color_t fontColor, uint32_t params) __attribute__((__stdcall__));

/// @brief Calculate amount of valid chars in UTF-8 string
/// @note Supports zero terminated string (set byteQuantity = -1)
/// @param string pointer to string
/// @param byteQuantity
/// @return amount of valid chars in UTF-8 string
extern int (*countUTF8Z)(const char *string, int byteQuantity) __attribute__((__stdcall__));

/// @brief Calculate amount of chars that fits given width
/// @param areaWidth width of area
/// @param charHeight char height
/// @return amount of chars that fits given width
extern int (*charsFit)(int areaWidth, int charHeight) __attribute__((__stdcall__));

/// @brief Calculate string width in pixels
/// @param charQuantity Characters
/// @param charHeight character height
/// @return string width in pixels
extern int (*strWidth)(int charQuantity, int charHeight) __attribute__((__stdcall__));

/// @brief Draw text on 24bpp or 32bpp image, the same as drawText but easier to use
/// @param canvas Pointer to image (array of colors)
/// @param canvasWidth image width
/// @param canvasHeight image height
/// @param x Coordinate of the text along the X axis
/// @param y Coordinate of the text along the Y axis
/// @param string Pointer to string
/// @param width char width (0 - auto)
/// @param height char height
/// @param fontColor Text color
/// @param flags value from enum RasterworksParams
/// @param encoding value fram enum RasterworksEncoding
/// @note don't forget free(buff)
void *DrawText(const void *canvas, int canvasWidth, int canvasHeight, int x, int y, const char *string, uint8_t width, uint8_t height, ksys_color_t fontColor, uint8_t flags, uint8_t encoding = Rasterworks_UTF8)
{
	const int l = canvasHeight * canvasHeight * 3 * sizeof(char);

	void *buff = malloc(l);
	*((int *)buff) = canvasWidth;
	*((int *)buff + 1) = canvasHeight;
	memcpy(buff+8, canvas, l);

	drawText(buff, x, y, string, countUTF8Z(string, -1), fontColor, (flags >> 24) + (width >> 16) + (height >> 8) + encoding);

	memcpy(buff, buff+8, l);

	return buff;
}

/**
 * \example rasterworks_example.c
 * example of using rasterworks library
*/

#endif /* KOLIBRI_RASTERWORKS_H */
