#ifndef KOLIBRI_LIBIMG_H
#define KOLIBRI_LIBIMG_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
	extern "C" {
#endif

#pragma pack(push, 1)
/// @brief Image
typedef struct {
  /// @brief ((Width ROL 16) OR Height) XOR Data[0]
  /// @note Ignored so far
  uint32_t Checksum;

  /// @brief Image width
  uint32_t Width;

  /// @brief Image height
  uint32_t Height;

  uint32_t Next;

  uint32_t Previous;

  /// @brief One of Image bppN
  uint32_t Type;

  /// @brief Image, array of colors
  uint32_t* Data;

  /// @brief Used if Type eq Image.bpp1, Image.bpp2, Image.bpp4 or Image.bpp8i
  uint32_t Palette;

  uint32_t Extended;

  /// @brief Bitfield
  uint32_t Flags;

  /// @brief Used if Image is animated is set in Flags
  uint32_t Delay;

} Image;
#pragma pack(pop)

/// @brief List of bppN
enum BPP
{
	/// @brief indexed
	IMAGE_BPP8i = 1,
	/// @brief "True color" 24bit
	IMAGE_BPP24 = 2,
	/// @brief 32bit
	IMAGE_BPP32 = 3,
	IMAGE_BPP15 = 4,
	/// @brief 16bit
	IMAGE_BPP16 = 5,
	IMAGE_BPP1 = 6,
	/// @brief grayscale
	IMAGE_BPP8g = 7,
	IMAGE_BPP2i = 8,
	IMAGE_BPP4i = 9,
	IMAGE_BPP8a = 10
};

/// @brief List of format id's
enum Formats
{
	/// @brief bmp format
	LIBIMG_FORMAT_BMP = 1,
	/// @brief ico format
	LIBIMG_FORMAT_ICO = 2,
	/// @brief cur format
	LIBIMG_FORMAT_CUR = 3,
	/// @brief gif format
	LIBIMG_FORMAT_GIF = 4,
	/// @brief png format
	LIBIMG_FORMAT_PNG = 5,
	/// @brief jpeg format
	LIBIMG_FORMAT_JPEG = 6,
	/// @brief tga format
	LIBIMG_FORMAT_TGA = 7,
	/// @brief pcx format
	LIBIMG_FORMAT_PCX = 8,
	/// @brief xcf format
	LIBIMG_FORMAT_XCF = 9,
	/// @brief tiff format
	LIBIMG_FORMAT_TIFF = 10,
	/// @brief Portable anymap format
	LIBIMG_FORMAT_PNM = 11,
	/// @brief Wireless Application Protocol Bitmap format
	LIBIMG_FORMAT_WBMP = 12,
	/// @brief X BitMap format
	LIBIMG_FORMAT_XBM = 13,

	LIBIMG_FORMAT_Z80 = 14
};

/// @brief List of scale type
enum Scale
{
	LIBIMG_SCALE_NONE = 0,
	LIBIMG_SCALE_INTEGER = 1,
	LIBIMG_SCALE_TILE = 2,
	LIBIMG_SCALE_STRETCH = 3,
	LIBIMG_SCALE_FIT_BOTH = LIBIMG_SCALE_STRETCH,
	LIBIMG_SCALE_FIT_MIN = 4,
	LIBIMG_SCALE_FIT_RECT = LIBIMG_SCALE_FIT_MIN,
	LIBIMG_SCALE_FIT_WIDTH = 5,
	LIBIMG_SCALE_FIT_HEIGHT = 6,
	LIBIMG_SCALE_FIT_MAX = 7
};   

/// @brief List of interpolation algorithms
enum Inter
{
	/// @note use it with LIBIMG_SCALE_INTEGER, LIBIMG_SCALE_TILE, etc
	LIBIMG_INTER_NONE = 0,
	/// @brief Bilinear algorithm
	LIBIMG_INTER_BILINEAR = 1,
	/// @brief Bicubic algorithm
	LIBIMG_INTER_BICUBIC = 2,
	/// @brief Lanczos algorithm
	LIBIMG_INTER_LANCZOS = 3,
	/// @brief Default algorithm
	LIBIMG_INTER_DEFAULT = LIBIMG_INTER_BILINEAR
};

/// @brief error codes
enum Errors
{
	LIBIMG_ERROR_OUT_OF_MEMORY = 1,
	LIBIMG_ERROR_FORMAT = 2,
	LIBIMG_ERROR_CONDITIONS = 3,
	LIBIMG_ERROR_BIT_DEPTH = 4,
	LIBIMG_ERROR_ENCODER = 5,
	LIBIMG_ERROR_SRC_TYPE = 6,
	LIBIMG_ERROR_SCALE = 7,
	LIBIMG_ERROR_INTER = 8,
	LIBIMG_ERROR_NOT_INPLEMENTED = 9,
	LIBIMG_ERROR_INVALID_INPUT = 10
};

/// @brief encode flags (byte 0x02 of _common option)
enum Encode
{
	LIBIMG_ENCODE_STRICT_SPECIFIC = 0x01,
	LIBIMG_ENCODE_STRICT_BIT_DEPTH = 0x02,
	LIBIMG_ENCODE_DELETE_ALPHA = 0x08,
	LIBIMG_ENCODE_FLUSH_ALPHA = 0x10
};

enum Flip
{
	FLIP_VERTICAL = 0x01,
	FLIP_HORIZONTAL = 0x02
};

enum Rotate
{
	ROTATE_90_CW = 0x01,
	ROTATE_180 = 0x03,
	ROTATE_270_CW = 0x03,
	ROTATE_90_CCW = ROTATE_270_CW,
	ROTATE_270_CCW = ROTATE_90_CW

};

/// @brief Initialize library libimg
/// @return -1 if unsuccessful
extern int kolibri_libimg_init(void);

#define _stdcall __attribute__((__stdcall__))

/// @brief Decode RAW data to Image data
/// @param file_data
/// @param size image size
/// @param b_color 
/// @return Pointer to image
extern Image*   (*img_decode)(void* file_data, uint32_t size, uint32_t b_color) _stdcall;


/// @brief Encode image
/// @param img image
/// @param length length
/// @param option Value from enum Encode
/// @return result
extern Image*   (*img_encode)(Image* img, uint32_t length, uint32_t option) _stdcall;


/// @brief Create image
/// @param width New image width
/// @param height New image height
/// @param type value from enum BPP
/// @return Pointer to image
/// @note create empety image
extern Image*   (*img_create)(uint32_t width, uint32_t height, uint32_t type) _stdcall;

extern void     (*img_to_rgb2)(Image* img, void *rgb_data) _stdcall;

extern Image*   (*img_to_rgb)(Image* img) _stdcall;


extern bool     (*img_flip)(Image* img, uint32_t flip) _stdcall;

extern bool     (*img_flip_layer)(Image *img, uint32_t flip) _stdcall;


extern bool     (*img_rotate)(Image *img, uint32_t rotate) _stdcall;

extern bool     (*img_rotate_layer)(Image* data, uint32_t rotate) _stdcall;

/// @brief Draw image
/// @param img image
/// @param x coord
/// @param y coord
/// @param w image width
/// @param h image height
/// @param xoff 0 
/// @param yoff 0
extern void     (*img_draw)(Image *img, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t xoff,  uint32_t yoff) _stdcall;

/// @brief 
/// @param img
/// @return 
extern int32_t  (*img_count)(Image *img) _stdcall;

/// @brief Destroy image
/// @param img Image to be deleted
extern bool     (*img_destroy)(Image *img) _stdcall;

/// @brief Destroy layer
/// @param img layer to be deleted
extern bool     (*img_destroy_layer)(Image* img) _stdcall;

/// @brief "Blend" two images
/// @param dst first(out) image
/// @param src second(in) image
/// @param out_x coordinate along the X axis in the first image with which blending takes place
/// @param out_y coordinate along the Y axis in the first image with which blending takes place
/// @param in_x coordinate along the X axis in the second image with which blending takes place
/// @param in_y coordinate along the Y axis in the second image with which blending takes place
/// @param width first image width
/// @param height first image height
/// @return first image
/// @note all this is drawn on firts image
extern Image*   (*img_blend)(Image* dst, Image* src, uint32_t out_x, uint32_t out_y, uint32_t in_x, uint32_t in_y, uint32_t width, uint32_t height) _stdcall;

/// @brief Convert image to other format
/// @param src Pointer to the image to convert
/// @param dst Pointer to the converted image
/// @param dst_type new image format
/// @return Pointer to the converted image
extern Image*   (*img_convert)(Image *src, Image *dst, uint32_t dst_type, uint32_t, uint32_t) _stdcall; 

/// @brief Resize image data (Image.Data)
/// @param src
/// @param width
/// @param height 
extern Image*   (*img_resize_data)(Image *src, uint32_t width, uint32_t height) _stdcall;

/// @brief Resize image
/// @param src Image to be resized
/// @param crop_x 0
/// @param crop_y 0
/// @param crop_width Width image to be resized
/// @param crop_height Height image to be resized
/// @param dst NULL
/// @param scale_type Scale type, value from enum Scale
/// @param inter Interpolation algorithm, value from enum Inter
/// @param new_width new image width
/// @param new_height new image heidth
/// @return Pointer to resized image
extern Image*   (*img_scale)(Image* src, uint32_t crop_x, uint32_t crop_y, uint32_t crop_width, uint32_t crop_height, Image* dst, uint32_t scale_type, uint32_t inter, uint32_t new_width, uint32_t new_height) _stdcall;


#ifdef __cplusplus
}
#endif

/// @brief Fill the image with color
/// @param img color fill image
/// @param width width of the filled area
/// @param height The height of the filled area
/// @param color Сolor
void img_fill_color(Image* img, uint32_t width, uint32_t height, uint32_t color) 
{
	uint32_t i;
    for (i = 0; i < width*height; i++) {
        img->Data[i] = color;
    }
}

/// @brief Fill the image with color
/// @param img image
/// @param color Сolor
void img_fill_color(Image* img, uint32_t color) 
{
    for (uint32_t i = 0; i < img->Width * img->Height; i++) {
        img->Data[i] = color;
    }
}

/**
 * \author turbocat2001 (Logaev Maxim)
 * \example main.c
 * example of using libimg library
 */

#undef _stdcall
#endif /* KOLIBRI_LIBIMG_H */
