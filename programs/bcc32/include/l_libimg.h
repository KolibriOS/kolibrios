#ifndef __L_LIBIMG_H_INCLUDED_
#define __L_LIBIMG_H_INCLUDED_
//
// libimg.obj
//

struct Image{
  long Checksum; // ((Width ROL 16) OR Height) XOR Data[0]        ; ignored so far
  long Width;
  long Height;
  long Next;
  long Previous;
  long Type;     // one of Image.bppN
  long Data;
  long Palette;  // used iff Type eq Image.bpp1, Image.bpp2, Image.bpp4 or Image.bpp8i
  long Extended;
  long Flags;    // bitfield
  long Delay;    // used iff Image.IsAnimated is set in Flags
};

//
// libimg - import table
//
void   (__stdcall* import_libimg)() = (void (__stdcall*)())&"lib_init";
//"img_is_img";
//"img_info";
//"img_from_file";
//"img_to_file";
//"img_from_rgb";
//"img_to_rgb";
void   (__stdcall* img_to_rgb2)(Image* img, unsigned char* out) = (void (__stdcall*)(Image*, unsigned char*))&"img_to_rgb2";
//"img_encode";
Image* (__stdcall* img_decode)(unsigned char* data, long length, long options) = (Image* (__stdcall*)(unsigned char*, long, long))&"img_decode";
Image* (__stdcall* img_create)(long width, long height, long type) = (Image* (__stdcall*)(long, long, long))&"img_create";
bool   (__stdcall* img_destroy)(Image* img) = (bool (__stdcall*)(Image*))&"img_destroy";
//"img_destroy_layer";
//"img_count";
//"img_lock_bits";
//"img_unlock_bits";
//"img_flip";
//"img_flip_layer";
//"img_rotate";
//"img_rotate_layer";
//"img_draw";
//"img_scale";
//"img_get_scaled_size";
Image* (__stdcall* img_convert)(Image* src, Image* dst, long dst_type, long flags, long param) = (Image* (__stdcall*)(Image*, Image*, long, long, long))&"img_convert";
//"img_formats_table";
asm{
	dd 0,0
}

#endif