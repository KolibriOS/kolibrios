#ifndef KOLIBRI_RASTERWORKS_H
#define KOLIBRI_RASTERWORKS_H

/// @brief List of parameters
enum Params
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
/// @param params ffeewwhh  \
hh - char height  \
ww - char width	; 0 = auto (proportional) \
ee - encoding	; 1 = cp866, 2 = UTF-16LE, 3 = UTF-8 \
ff - Parameters from the Params list
/// @note All flags combinable, except align right + align center
/// @note The text is drawn on the image, in order for changes to occur in the window, you need to draw the image after calling this function
extern void (*drawText)(void *canvas, int x, int y, const char *string, int charQuantity, int fontColor, int params) __attribute__((__stdcall__));

/// @brief Calculate amount of valid chars in UTF-8 string
/// @note Supports zero terminated string (set byteQuantity = -1)
/// @param string
/// @param byteQuantity
extern int (*countUTF8Z)(const char *string, int byteQuantity) __attribute__((__stdcall__));

/// @brief Calculate amount of chars that fits given width
extern int (*charsFit)(int areaWidth, int charHeight) __attribute__((__stdcall__));

/// @brief Calculate string width in pixels
/// @param charQuantity Characters
/// @param charHeight character height
extern int (*strWidth)(int charQuantity, int charHeight) __attribute__((__stdcall__));

/**
 * \example rasterworks_example.c
 * example of using rasterworks library
 */

#endif /* KOLIBRI_RASTERWORKS_H */
