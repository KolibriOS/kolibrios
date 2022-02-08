//Asper
#ifndef INCLUDE_LIBIMG_H
#define INCLUDE_LIBIMG_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_MEM_H
#include "../lib/mem.h"
#endif

#ifdef __COFF__
extern dword img_decode;
extern dword img_destroy;
extern dword img_draw;
extern dword img_create;
extern dword img_encode;
extern dword img_convert;
extern dword img_from_file;
extern dword img_blend;
//extern dword img_is_img;
//extern dword img_to_rgb2;
//extern dword img_scale;
extern dword img_flip;
extern dword img_rotate;
extern dword img_to_rgb;
#else
#ifndef INCLUDE_DLL_H
#include "../lib/dll.h"
#endif

//library
dword libimg = #alibimg;
char alibimg[] = "/sys/lib/libimg.obj";

dword libimg_init   = #alibimg_init;
dword img_decode    = #aimg_decode;
dword img_destroy   = #aimg_destroy;
dword img_draw      = #aimg_draw;
dword img_create    = #aimg_create;
dword img_encode    = #aimg_encode;
dword img_convert   = #aimg_convert;
dword img_from_file = #aimg_from_file;
dword img_blend     = #aimg_blend;
dword img_flip      = #aimg_flip;
dword img_rotate    = #aimg_rotate;
dword img_scale     = #aimg_scale;
//dword img_is_img    = #aimg_is_img;
//dword img_to_rgb    = #aimg_to_rgb;
//dword img_to_rgb2   = #aimg_to_rgb2;

$DD 2 dup 0

//import
char alibimg_init[]   = "lib_init";
char aimg_decode[]    = "img_decode";
char aimg_destroy[]   = "img_destroy";
char aimg_draw[]      = "img_draw";
char aimg_create[]    = "img_create";
char aimg_encode[]    = "img_encode";
char aimg_convert[]   = "img_convert";
char aimg_from_file[] = "img_from_file";
char aimg_blend[]     = "img_blend";
char aimg_flip[]      = "img_flip";
char aimg_rotate[]    = "img_rotate";
char aimg_scale[]     = "img_scale";
//char aimg_is_img[]    = "img_is_img";
//char aimg_to_rgb[]    = "img_to_rgb";
//char aimg_to_rgb2[]   = "img_to_rgb2";
#endif

#define LIBIMG_FORMAT_BMP       1
#define LIBIMG_FORMAT_ICO       2
#define LIBIMG_FORMAT_CUR       3
#define LIBIMG_FORMAT_GIF       4
#define LIBIMG_FORMAT_PNG       5
#define LIBIMG_FORMAT_JPEG      6
#define LIBIMG_FORMAT_TGA       7
#define LIBIMG_FORMAT_PCX       8
#define LIBIMG_FORMAT_XCF       9
#define LIBIMG_FORMAT_TIFF     10
#define LIBIMG_FORMAT_PNM      11
#define LIBIMG_FORMAT_WBMP     12
#define LIBIMG_FORMAT_XBM      13
#define LIBIMG_FORMAT_Z80      14

// values for Image.Type
// must be consecutive to allow fast switch on Image.Type in support functions
#define IMAGE_BPP8i  1  // indexed
#define IMAGE_BPP24  2
#define IMAGE_BPP32  3
#define IMAGE_BPP15  4
#define IMAGE_BPP16  5
#define IMAGE_BPP1   6
#define IMAGE_BPP8g  7  // grayscale
#define IMAGE_BPP2i  8
#define IMAGE_BPP4i  9
#define IMAGE_BPP8a 10  // grayscale with alpha channel; application layer only!!!
                        // kernel doesn't handle this image type,
                        // libimg can only create and destroy such images

//flip and rotate
#define FLIP_VERTICAL   0x01
#define FLIP_HORIZONTAL 0x02
#define ROTATE_90_CW    0x01
#define ROTATE_180      0x02
#define ROTATE_270_CW   0x03
#define ROTATE_90_CCW   ROTATE_270_CW
#define ROTATE_270_CCW  ROTATE_90_CW

//scale type                    //corresponding img.scale params
#define LIBIMG_SCALE_NONE       0     //do not scale
#define LIBIMG_SCALE_INTEGER    1     //scale factor, reserved 0
#define LIBIMG_SCALE_TILE       2     //new width, new height
#define LIBIMG_SCALE_STRETCH    3     //new width, new height
#define LIBIMG_SCALE_FIT_BOTH   LIBIMG_SCALE_STRETCH
#define LIBIMG_SCALE_FIT_MIN    4     //new width, new height
#define LIBIMG_SCALE_FIT_RECT   LIBIMG_SCALE_FIT_MIN
#define LIBIMG_SCALE_FIT_WIDTH  5     //new width, new height
#define LIBIMG_SCALE_FIT_HEIGHT 6     //new width, new height
#define LIBIMG_SCALE_FIT_MAX    7     //new width, new height

//interpolation algorithm
#define LIBIMG_INTER_NONE       0     //use it with LIBIMG_SCALE_INTEGER, LIBIMG_SCALE_TILE, etc
#define LIBIMG_INTER_BILINEAR   1
#define LIBIMG_INTER_DEFAULT    LIBIMG_INTER_BILINEAR

/*
// error codes
LIBIMG_ERROR_OUT_OF_MEMORY      = 1
LIBIMG_ERROR_FORMAT             = 2
LIBIMG_ERROR_CONDITIONS         = 3
LIBIMG_ERROR_BIT_DEPTH          = 4
LIBIMG_ERROR_ENCODER            = 5
LIBIMG_ERROR_SRC_TYPE           = 6
LIBIMG_ERROR_SCALE              = 7
LIBIMG_ERROR_INTER              = 8
LIBIMG_ERROR_NOT_INPLEMENTED    = 9
LIBIMG_ERROR_INVALID_INPUT      = 10
*/

struct libimg_image
{
    dword checksum;  // ((Width ROL 16) OR Height) XOR Data[0]        ; ignored so far
    dword w;
    dword h;
    dword next;
    dword previous;
    dword type;     // one of Image.bppN
    dword imgsrc;
    dword palette;  // used iff Type eq Image.bpp1, Image.bpp2, Image.bpp4 or Image.bpp8i
    dword extended;
    dword flags;    // bitfield
    dword delay;    // used iff Image.IsAnimated is set in Flags
    dword image;
    void load();
    void convert_into();
    void replace_color();
    void replace_2colors();
    void set_vars();
    void draw();
};

:void libimg_image::set_vars()
{
    $push edi
    EDI = image;
    //checksum = ESDWORD[EDI];
    w = ESDWORD[EDI+4];
    h = ESDWORD[EDI+8];
    //next = ESDWORD[EDI+12];
    //previous = ESDWORD[EDI+16];
    type = ESDWORD[EDI+20];
    imgsrc = ESDWORD[EDI+24];
    //palette = ESDWORD[EDI+28];
    //extended = ESDWORD[EDI+32];
    //flags = ESDWORD[EDI+36];
    //delay = ESDWORD[EDI+40];
    $pop edi
}

:void libimg_image::load(dword file_path)
{
    if (image) img_destroy stdcall(image);
    img_from_file stdcall(file_path);
    if (!EAX) {
        notify("'Error: Image not loaded'E");
    } else {
        image = EAX;
        set_vars();
    }
}

:void libimg_image::replace_color(dword old_color, new_color)
{
    EDX =  w * h * 4 + imgsrc;
    ESI = old_color;
    ECX = new_color;
    FOR (EDI = imgsrc; EDI < EDX; EDI += 4) IF (DSDWORD[EDI]==ESI) DSDWORD[EDI] = ECX;
}

:void libimg_image::replace_2colors(dword old_color1, new_color1, old_color2, new_color2)
{
    EDX =  w * h * 4 + imgsrc;
    ESI = old_color1;
    ECX = new_color1;
    EBX = old_color2;
    EAX = new_color2;
    FOR (EDI = imgsrc; EDI < EDX; EDI += 4) {
        IF (DSDWORD[EDI]==ESI) DSDWORD[EDI] = ECX;
        ELSE IF (DSDWORD[EDI]==EBX) DSDWORD[EDI] = EAX;
    }
}

:void libimg_image::draw(dword _x, _y, _w, _h, _xoff, _yoff)
{
    if (image) img_draw stdcall(image, _x, _y, _w, _h, _xoff, _yoff);
}

:void libimg_image::convert_into(dword _to)
{
    img_convert stdcall(image, 0, _to, 0, 0);
    if (!EAX) {
        notify("'LibImg convertation error!'E");
    } else {
        $push eax
        img_destroy stdcall(image);
        $pop eax
        image = EAX;
        set_vars();
    }
}

:dword save_image(dword _image_pointer, _w, _h, _path)
{
    dword encoded_data=0;
    dword encoded_size=0;
    dword image_ptr = 0;

    img_create stdcall(_w, _h, IMAGE_BPP24);
    image_ptr = EAX;

    if (!image_ptr) {
        return "Error creating image!";
    }
    else {
        EDI = image_ptr;
        memmov(EDI.libimg_image.imgsrc, _image_pointer, _w * _h * 3);

        img_encode stdcall(image_ptr, LIBIMG_FORMAT_PNG, 0);
        encoded_data = EAX;
        encoded_size = ECX;

        img_destroy stdcall(image_ptr);

        if(!encoded_data) {
            return "Error encoding image!";
        }
        else {
            if (!CreateFile(encoded_size, encoded_data, _path)) {
                return 0;
            }
            else {
                return "'Error saving image file!\nNot enough space? Path wrong?\nFile system is not writable?..' -E";
            }
        }
    }
}

#endif