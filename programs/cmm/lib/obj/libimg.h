//Asper
#ifndef INCLUDE_LIBIMG_H
#define INCLUDE_LIBIMG_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_MEM_H
#include "../lib/mem.h"
#endif

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
//dword img_resize    = #aimg_resize;
//dword img_is_img    = #aimg_is_img;
//dword img_to_rgb2   = #aimg_to_rgb2;
//dword img_scale     = #aimg_scale;
//dword img_flip      = #aimg_flip;
//dword img_rotate    = #aimg_rotate;

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
//char aimg_resize[]    = "img_resize";
//char aimg_is_img[]    = "img_is_img";
//char aimg_to_rgb2[]   = "img_to_rgb2";
//char aimg_scale[]     = "img_scale";
//char aimg_flip[]      = "img_flip";
//char aimg_rotate[]    = "img_rotate";

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

struct libimg_image
{
    dword checksum; // ((Width ROL 16) OR Height) XOR Data[0]        ; ignored so far
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
    $push eax
    img_destroy stdcall(image);
    $pop eax
    if (!EAX) {
        notify("'LibImg convertation error!'E");
    } else {
        image = EAX;
        set_vars();
    }
}

:dword create_image(dword type, dword width, dword height) {
    img_create stdcall(width, height, type);
    return EAX;
}

// size - output parameter, error code / the size of encoded data
:dword encode_image(dword image_ptr, dword options, dword specific_options, dword* size) {
    img_encode stdcall(image_ptr, options, specific_options);
    ESDWORD[size] = ECX;
    
    return EAX;
}

#ifdef LANG_RUS
#define TEXT_FILE_SAVED_AS "'Файл сохранен как "
#else
#define TEXT_FILE_SAVED_AS "'File saved as "
#endif
:void save_image(dword _image_pointer, _w, _h, _path)
{
    char save_success_message[4096+200];
    dword encoded_data=0;
    dword encoded_size=0;
    dword image_ptr = 0;
    
    image_ptr = create_image(IMAGE_BPP24, _w, _h);

    if (image_ptr == 0) {
        notify("'Error saving file, probably not enought memory!' -E");
    }
    else {
        EDI = image_ptr;
        memmov(EDI.libimg_image.imgsrc, _image_pointer, _w * _h * 3);

        encoded_data = encode_image(image_ptr, LIBIMG_FORMAT_PNG, 0, #encoded_size);

        img_destroy stdcall(image_ptr);

        if(encoded_data == 0) {
            notify("'Error saving file, incorrect data!' -E");
        }
        else {
            if (CreateFile(encoded_size, encoded_data, _path) == 0) {
                strcpy(#save_success_message, TEXT_FILE_SAVED_AS);
                strcat(#save_success_message, _path);
                strcat(#save_success_message, "' -O");
                notify(#save_success_message);
            }
            else {
                notify("'Error saving image file!\nNot enough space? Path wrong?\nFile system is not writable?..' -E");
            }
        }
    }
}



/////////////////////////////
/*
//  DRAW ICON PATTERN / TEMP
*/
/////////////////////////////

:void DrawIcon32(dword x,y, _bg, icon_n) {
    static dword bg;
    static dword pure_img32;
    if (!pure_img32) || (bg!=_bg) {
        bg = _bg;
        if (pure_img32) img_destroy stdcall(pure_img32);
        img_from_file stdcall("/sys/icons32.png");
        pure_img32 = EAX;
        //now fill transparent with another color
        EDX = ESDWORD[EAX+4] * ESDWORD[EAX+8] * 4 + ESDWORD[EAX+24];
        for (ESI = ESDWORD[EAX+24]; ESI < EDX; ESI += 4) {
            if (DSDWORD[ESI]==0x00000000) DSDWORD[ESI] = bg;
        }
    }
    img_draw stdcall(pure_img32, x, y, 32, 32, 0, icon_n*32);
}

:void DrawIcon16(dword x,y, _bg, icon_n) {
    static dword bg;
    static dword pure_img16;
    dword bgshadow;
    if (!pure_img16) || (bg!=_bg) {
        bg = _bg;
        bgshadow = MixColors(bg, 0, 220);
        if (pure_img16) img_destroy stdcall(pure_img16);
        img_from_file stdcall("/sys/icons16.png");
        pure_img16 = EAX;
        //now fill transparent with another color
        EDX = ESDWORD[EAX+4] * ESDWORD[EAX+8] * 4 + ESDWORD[EAX+24];
        for (ESI = ESDWORD[EAX+24]; ESI < EDX; ESI += 4) {
            if (DSDWORD[ESI]==0xffFFFfff) DSDWORD[ESI] = bg;
            if (DSDWORD[ESI]==0xffCACBD6) DSDWORD[ESI] = bgshadow;
        }
    }
    img_draw stdcall(pure_img16, x, y, 16, 16, 0, icon_n*16);
}

#endif