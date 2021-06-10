/* Written by turbocat2001 (Logaev Maxim) */

#ifndef KOLIBRI_LIBIMG_H
#define KOLIBRI_LIBIMG_H

#include <stddef.h>
#include <stdbool.h>

extern int kolibri_libimg_init(void);

#define _stdcall __attribute__((__stdcall__))

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

#pragma pack(push, 1)
typedef struct{
  uint32_t Checksum; // ((Width ROL 16) OR Height) XOR Data[0]        ; ignored so far
  uint32_t Width;
  uint32_t Height;
  uint32_t Next;
  uint32_t Previous;
  uint32_t Type;     // one of Image.bppN
  uint32_t* Data;
  uint32_t Palette;  // used iff Type eq Image.bpp1, Image.bpp2, Image.bpp4 or Image.bpp8i
  uint32_t Extended;
  uint32_t Flags;    // bitfield
  uint32_t Delay;    // used iff Image.IsAnimated is set in Flags
} Image;
#pragma pack(pop)

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

// scale type
#define LIBIMG_SCALE_NONE       0 
#define LIBIMG_SCALE_INTEGER    1   
#define LIBIMG_SCALE_TILE       2    
#define LIBIMG_SCALE_STRETCH    3  
#define LIBIMG_SCALE_FIT_BOTH   LIBIMG_SCALE_STRETCH
#define LIBIMG_SCALE_FIT_MIN    4
#define LIBIMG_SCALE_FIT_RECT   LIBIMG_SCALE_FIT_MIN
#define LIBIMG_SCALE_FIT_WIDTH  5  
#define LIBIMG_SCALE_FIT_HEIGHT 6 
#define LIBIMG_SCALE_FIT_MAX    7     

// interpolation algorithm
#define LIBIMG_INTER_NONE       0     // use it with LIBIMG_SCALE_INTEGER, LIBIMG_SCALE_TILE, etc
#define LIBIMG_INTER_BILINEAR   1
#define LIBIMG_INTER_BICUBIC    2
#define LIBIMG_INTER_LANCZOS    3
#define LIBIMG_INTER_DEFAULT   LIBIMG_INTER_BILINEAR

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

extern Image*   (*img_decode _stdcall)(void* file_data, uint32_t size, uint32_t b_color);
extern Image*   (*img_encode _stdcall)(Image* img, uint32_t length, uint32_t option);
extern Image*   (*img_create _stdcall)(uint32_t width, uint32_t height, uint32_t type);
extern void     (*img_to_rgb2 _stdcall)(Image* img, void *rgb_data);
extern Image*   (*img_to_rgb _stdcall)(Image* img);
extern bool     (*img_flip _stdcall)(Image* img, uint32_t flip);
extern bool     (*img_flip_layer _stdcall)(Image *img, uint32_t flip);
extern bool     (*img_rotate _stdcall)(Image *img, uint32_t rotate);
extern bool     (*img_rotate_layer _stdcall)(Image* data, uint32_t rotate);
extern void     (*img_draw _stdcall)(Image *img, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t xoff,  uint32_t yoff);
extern int32_t  (*img_count _stdcall)(Image *img);
extern bool     (*img_destroy _stdcall)(Image *img);
extern bool     (*img_destroy_layer _stdcall)(Image* img);
extern Image*   (*img_blend _stdcall)(Image* dst, Image* src, uint32_t out_x, uint32_t out_y, uint32_t in_x, uint32_t in_y, uint32_t width, uint32_t height); 
extern Image*   (*img_convert _stdcall)(Image *src, Image *dst, uint32_t dst_type, uint32_t, uint32_t); 
extern Image*   (*img_resize_data _stdcall)(Image *src, uint32_t width, uint32_t height);
extern Image*   (*img_scale _stdcall)(Image* src, uint32_t crop_x, uint32_t crop_y, uint32_t crop_width, uint32_t crop_height, Image* dst, uint32_t scale_type, uint32_t inter, uint32_t new_width, uint32_t new_height);

void img_fill_color(Image* img, uint32_t width, uint32_t height, uint32_t color){
    for (uint32_t i = 0; i < width*height; i++) {
        img->Data[i] = color;
    }
}

#endif /* KOLIBRI_LIBIMG_H */
