#ifndef KOLIBRI_LIBIMG_H
#define KOLIBRI_LIBIMG_H

#include <stddef.h>
extern int kolibri_libimg_init(void);

//list of format id's
#define LIBIMG_FORMAT_BMP       1
#define LIBIMG_FORMAT_ICO       2
#define LIBIMG_FORMAT_CUR       3
#define LIBIMG_FORMAT_GIF       4
#define LIBIMG_FORMAT_PNG       5
#define LIBIMG_FORMAT_JPEG      6
#define LIBIMG_FORMAT_TGA       7
#define LIBIMG_FORMAT_PCX       8
#define LIBIMG_FORMAT_XCF       9
#define LIBIMG_FORMAT_TIFF      10
#define LIBIMG_FORMAT_PNM       11
#define LIBIMG_FORMAT_WBMP      12
#define LIBIMG_FORMAT_XBM       13
#define LIBIMG_FORMAT_Z80       14

#define IMAGE_BPP8i  1  // indexed
#define IMAGE_BPP24  2
#define IMAGE_BPP32  3
#define IMAGE_BPP15  4
#define IMAGE_BPP16  5
#define IMAGE_BPP1   6
#define IMAGE_BPP8g  7  // grayscale
#define IMAGE_BPP2i  8
#define IMAGE_BPP4i  9
#define IMAGE_BPP8a 10 

//error codes
#define LIBIMG_ERROR_OUT_OF_MEMORY      1
#define LIBIMG_ERROR_FORMAT             2
#define LIBIMG_ERROR_CONDITIONS         3
#define LIBIMG_ERROR_BIT_DEPTH          4
#define LIBIMG_ERROR_ENCODER            5
#define LIBIMG_ERROR_SRC_TYPE           6
#define LIBIMG_ERROR_SCALE              7
#define LIBIMG_ERROR_INTER              8
#define LIBIMG_ERROR_NOT_INPLEMENTED    9
#define LIBIMG_ERROR_INVALID_INPUT      10

//encode flags (byte 0x02 of _common option)
#define LIBIMG_ENCODE_STRICT_SPECIFIC   0x01
#define LIBIMG_ENCODE_STRICT_BIT_DEPTH  0x02
#define LIBIMG_ENCODE_DELETE_ALPHA      0x08
#define LIBIMG_ENCODE_FLUSH_ALPHA       0x10


#define FLIP_VERTICAL   0x01
#define FLIP_HORIZONTAL 0x02

#define ROTATE_90_CW    0x01
#define ROTATE_180      0x02
#define ROTATE_270_CW   0x03
#define ROTATE_90_CCW   ROTATE_270_CW
#define ROTATE_270_CCW  ROTATE_90_CW

extern void*    (*img_decode __attribute__((__stdcall__)))(void* file_data, uint32_t length, uint32_t options);
extern void*    (*img_encode __attribute__((__stdcall__)))(void* image_data, uint32_t length, uint32_t option);
extern void*    (*img_create __attribute__((__stdcall__)))(uint32_t width, uint32_t height, uint32_t type);
extern void     (*img_to_rgb2 __attribute__((__stdcall__)))(void* image_data, void *rgb_data);
extern void*    (*img_to_rgb __attribute__((__stdcall__)))(void *image_data);
extern uint32_t (*img_flip __attribute__((__stdcall__)))(void* image_data, uint32_t flip);
extern uint32_t (*img_flip_layer __attribute__((__stdcall__)))(void *image_data, uint32_t flip);
extern uint32_t (*img_rotate __attribute__((__stdcall__)))(void* image_data, uint32_t rotate);
extern uint32_t (*img_rotate_layer __attribute__((__stdcall__)))(void* image_data, uint32_t rotate);
extern void     (*img_draw __attribute__((__stdcall__)))(void *image_data, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t xoff,  uint32_t yoff);
extern uint32_t (*img_count __attribute__((__stdcall__)))(void *image_data);
extern uint32_t (*img_destroy __attribute__((__stdcall__)))(void *image_data);
extern uint32_t (*img_destroy_layer __attribute__((__stdcall__)))(void* image_data);

#endif /* KOLIBRI_LIBIMG_H */
