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

#ifndef INCLUDE_LIBIO_H
#include "../lib/obj/libio.h"
#endif

//library
dword libimg = #alibimg;
char alibimg[] = "/sys/lib/libimg.obj";
	
dword libimg_init = #alibimg_init;
dword img_is_img  = #aimg_is_img;
dword img_to_rgb2 = #aimg_to_rgb2;
dword img_decode  = #aimg_decode;
dword img_destroy = #aimg_destroy;
dword img_draw    = #aimg_draw;
dword img_create  = #aimg_create;
dword img_encode  = #aimg_encode;

//dword img_flip    = #aimg_flip;
//dword img_rotate  = #aimg_rotate;
$DD 2 dup 0

//import  libimg                     , \
char alibimg_init[] = "lib_init";
char aimg_is_img[]  = "img_is_img";
char aimg_to_rgb2[] = "img_to_rgb2";
char aimg_decode[]  = "img_decode";
char aimg_destroy[] = "img_destroy";
char aimg_draw[]    = "img_draw";
char aimg_create[]    = "img_create";
char aimg_encode[]    = "img_encode";
//char aimg_flip[]    = "img_flip";
//char aimg_rotate[]  = "img_rotate ";

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

struct _Image
{
   dword Checksum; // ((Width ROL 16) OR Height) XOR Data[0]        ; ignored so far
   dword Width;
   dword Height;
   dword Next;
   dword Previous;
   dword Type;     // one of Image.bppN
   dword Data;
   dword Palette;  // used iff Type eq Image.bpp1, Image.bpp2, Image.bpp4 or Image.bpp8i
   dword Extended;
   dword Flags;    // bitfield
   dword Delay;    // used iff Image.IsAnimated is set in Flags
};

// values for Image.Type
// must be consecutive to allow fast switch on Image.Type in support functions
#define Image_bpp8i  1  // indexed
#define Image_bpp24  2
#define Image_bpp32  3
#define Image_bpp15  4
#define Image_bpp16  5
#define Image_bpp1   6
#define Image_bpp8g  7  // grayscale
#define Image_bpp2i  8
#define Image_bpp4i  9
#define Image_bpp8a 10  // grayscale with alpha channel; application layer only!!! 
                        // kernel doesn't handle this image type, 
                        // libimg can only create and destroy such images


:dword load_image(dword filename)
{
        //align 4
        dword img_data=0;
        dword img_data_len=0;
        dword fh=0;
        dword image=0;

        byte tmp_buf[40];
        $and     img_data, 0
        //$mov     eax, filename
        //$push    eax        
        //invoke  file.open, eax, O_READ
        file_open stdcall (filename, O_READ);
        $or      eax, eax
        $jnz      loc05  
        $stc
        return 0;
    @loc05:    
        $mov     fh, eax
        //invoke  file.size
        file_size stdcall (filename);
        $mov     img_data_len, ebx
        //stdcall mem.Alloc, ebx
        mem_Alloc(EBX);
        
        $test    eax, eax
        $jz      error_close
        $mov     img_data, eax
        //invoke  file.read, [fh], eax, [img_data_len]
        file_read stdcall (fh, EAX, img_data_len);
        $cmp     eax, -1
        $jz      error_close
        $cmp     eax, img_data_len
        $jnz     error_close
        //invoke  file.close, [fh]
        file_close stdcall (fh);
        $inc     eax
        $jz      error_
//; img.decode checks for img.is_img
//;       //invoke  img.is_img, [img_data], [img_data_len]
//;       $or      eax, eax
//;       $jz      exit
        //invoke  img.decode, [img_data], [img_data_len], 0
        EAX=img_data;
        img_decode stdcall (EAX, img_data_len,0);
        $or      eax, eax
        $jz      error_
        $cmp     image, 0
        $pushf
        $mov     image, eax
        //call    init_frame
        $popf
        //call    update_image_sizes
        mem_Free(img_data);//free_img_data(img_data);
        $clc
        return image;

@error_free:
        //invoke  img.destroy, [image]
        img_destroy stdcall (image);
        $jmp     error_

@error_pop:
        $pop     eax
        $jmp     error_
@error_close:
        //invoke  file.close, [fh]
        file_close stdcall (fh);
@error_:
        mem_Free(img_data);
        $stc
        return 0;
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

:void DrawLibImage(dword image_pointer,x,y,w,h,offx,offy) {
    img_draw stdcall (
        image_pointer, 
        x, 
        y, 
        w, 
        h, 
        offx, 
        offy
    );  
}

//NOTICE: DO NOT FORGET TO INIT libio AND libimg!!!
:void save_image(dword _image_pointer, _w, _h, _path)
{
    char save_success_message[4096+200];
    dword encoded_data=0;
    dword encoded_size=0;
    dword image_ptr = 0;
    
    image_ptr = create_image(Image_bpp24, _w, _h);

    if (image_ptr == 0) {
        notify("'Error saving file, probably not enought memory!' -E");
    }
    else {
        EDI = image_ptr;
        memmov(EDI._Image.Data, _image_pointer, _w * _h * 3);

        encoded_data = encode_image(image_ptr, LIBIMG_FORMAT_PNG, 0, #encoded_size);

        img_destroy stdcall(image_ptr);

        if(encoded_data == 0) {
            notify("'Error saving file, incorrect data!' -E");
        }
        else {
            if (CreateFile(encoded_size, encoded_data, _path) == 0) {
                sprintf(#save_success_message, "'File saved as %s' -O", _path);
                notify(#save_success_message);
            }
            else {
                notify("'Error saving image file!\nProbably not enought space or file system is not writable!\nPlease, check saving path.' -E");
            }
        }
    }
}

#ifndef INCLUDE_LIBIMG_LOAD_SKIN_H
#include "../lib/patterns/libimg_load_skin.h"
#endif


#endif