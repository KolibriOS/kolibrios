#ifndef __L_BUF2D_H_INCLUDED_
#define __L_BUF2D_H_INCLUDED_
//
// buf2d.obj
//

struct buf_2d_header
{
	void* img_data;
	short int left, top;
	long size_x, size_y;
	unsigned long color; //color
	unsigned char bit_pp; //bit in pixel
};

const long BUF2D_OPT_CROP_TOP    = 1;
const long BUF2D_OPT_CROP_LEFT   = 2;
const long BUF2D_OPT_CROP_BOTTOM = 4;
const long BUF2D_OPT_CROP_RIGHT  = 8;
const long BUF2D_BIT_OPT_CROP_TOP    = 0;
const long BUF2D_BIT_OPT_CROP_LEFT   = 1;
const long BUF2D_BIT_OPT_CROP_BOTTOM = 2;
const long BUF2D_BIT_OPT_CROP_RIGHT  = 3;

//
// buf2d - import table
//
void (__stdcall* import_buf2d)() = (void (__stdcall*)())&"lib_init";
void (__stdcall* buf2d_create)(buf_2d_header* buf) = (void (__stdcall*)(buf_2d_header*))&"buf2d_create";
void (__stdcall* buf2d_create_f_img)(buf_2d_header* buf, void* img_data) = (void (__stdcall*)(buf_2d_header*, void*))&"buf2d_create_f_img";
void (__stdcall* buf2d_clear)(buf_2d_header* buf, long color) = (void (__stdcall*)(buf_2d_header*, long))&"buf2d_clear";
void (__stdcall* buf2d_draw)(buf_2d_header* buf) = (void (__stdcall*)(buf_2d_header*))&"buf2d_draw";
void (__stdcall* buf2d_delete)(buf_2d_header* buf) = (void (__stdcall*)(buf_2d_header*))&"buf2d_delete";
void (__stdcall* buf2d_resize)(buf_2d_header* buf, long n_w, long n_h, long opt) = (void (__stdcall*)(buf_2d_header*, long, long, long))&"buf2d_resize";
void (__stdcall* buf2d_rotate)(buf_2d_header* buf, long angle) = (void (__stdcall*)(buf_2d_header*, long))&"buf2d_rotate";
void (__stdcall* buf2d_line)(buf_2d_header* buf, long x1, long y1, long x2, long y2, long color) = (void (__stdcall*)(buf_2d_header*, long, long, long, long, long))&"buf2d_line";
void (__stdcall* buf2d_line_sm)(buf_2d_header* buf, long x1, long y1, long x2, long y2, long color) = (void (__stdcall*)(buf_2d_header*, long, long, long, long, long))&"buf2d_line_sm";
void (__stdcall* buf2d_rect_by_size)(buf_2d_header* buf, long x1, long y1, long w, long h, long color) = (void (__stdcall*)(buf_2d_header*, long, long, long, long, long))&"buf2d_rect_by_size";
void (__stdcall* buf2d_filled_rect_by_size)(buf_2d_header* buf, long x1, long y1, long w, long h, long color) = (void (__stdcall*)(buf_2d_header*, long, long, long, long, long))&"buf2d_filled_rect_by_size";
void (__stdcall* buf2d_circle)(buf_2d_header* buf, long x, long y, long r, long color) = (void (__stdcall*)(buf_2d_header*, long, long, long, long))&"buf2d_circle";
void (__stdcall* buf2d_img_hdiv2)(buf_2d_header* buf) = (void (__stdcall*)(buf_2d_header*))&"buf2d_img_hdiv2";
void (__stdcall* buf2d_img_wdiv2)(buf_2d_header* buf) = (void (__stdcall*)(buf_2d_header*))&"buf2d_img_wdiv2";
void (__stdcall* buf2d_conv_24_to_8)(buf_2d_header* buf24, long opt_rgb) = (void (__stdcall*)(buf_2d_header*, long))&"buf2d_conv_24_to_8";
void (__stdcall* buf2d_conv_24_to_32)(buf_2d_header* buf24, buf_2d_header* buf8) = (void (__stdcall*)(buf_2d_header*, buf_2d_header*))&"buf2d_conv_24_to_32";
void (__stdcall* buf2d_bit_blt)(buf_2d_header* buf0, long x, long y, buf_2d_header* buf1) = (void (__stdcall*)(buf_2d_header*, long, long, buf_2d_header*))&"buf2d_bit_blt";
void (__stdcall* buf2d_bit_blt_transp)(buf_2d_header* buf0, long x, long y, buf_2d_header* buf32) = (void (__stdcall*)(buf_2d_header*, long, long, buf_2d_header*))&"buf2d_bit_blt_transp";
void (__stdcall* buf2d_bit_blt_alpha)(buf_2d_header* buf0, long x, long y, buf_2d_header* buf8) = (void (__stdcall*)(buf_2d_header*, long, long, buf_2d_header*))&"buf2d_bit_blt_alpha";
void (__stdcall* buf2d_curve_bezier)(buf_2d_header* buf, long p1, long p2, long p3, long color) = (void (__stdcall*)(buf_2d_header*, long, long, long, long))&"buf2d_curve_bezier";
void (__stdcall* buf2d_convert_text_matrix)(buf_2d_header* buf) = (void (__stdcall*)(buf_2d_header*))&"buf2d_convert_text_matrix";
void (__stdcall* buf2d_draw_text)(buf_2d_header* buf, buf_2d_header* buf_txt, char* txt, long x, long y, long color) = (void (__stdcall*)(buf_2d_header*, buf_2d_header*, char*, long, long, long))&"buf2d_draw_text";
void (__stdcall* buf2d_crop_color)(buf_2d_header* buf, long color, long opt) = (void (__stdcall*)(buf_2d_header*, long, long))&"buf2d_crop_color";
void (__stdcall* buf2d_offset_h)(buf_2d_header* buf, long dy) = (void (__stdcall*)(buf_2d_header*, long))&"buf2d_offset_h";
void (__stdcall* buf2d_flood_fill)(buf_2d_header* buf, long x, long y, long opt, long color1, long color2) = (void (__stdcall*)(buf_2d_header*, long, long, long, long, long))&"buf2d_flood_fill";
void (__stdcall* buf2d_set_pixel)(buf_2d_header* buf, long x, long y, long color) = (void (__stdcall*)(buf_2d_header*, long, long, long))&"buf2d_set_pixel";
long (__stdcall* buf2d_get_pixel)(buf_2d_header* buf, long x, long y) = (long (__stdcall*)(buf_2d_header*, long, long))&"buf2d_get_pixel";
void (__stdcall* buf2d_flip_h)(buf_2d_header* buf) = (void (__stdcall*)(buf_2d_header*))&"buf2d_flip_h";
void (__stdcall* buf2d_flip_v)(buf_2d_header* buf) = (void (__stdcall*)(buf_2d_header*))&"buf2d_flip_v";
void (__stdcall* buf2d_filter_dither)(buf_2d_header* buf, long opt) = (void (__stdcall*)(buf_2d_header*, long))&"buf2d_filter_dither";

/*
void (__stdcall* buf2d_vox_brush_create)(buf_2d_header* buf, long* opt) = (void (__stdcall*)(buf_2d_header*, long*))&"buf2d_vox_brush_create";
"buf2d_vox_brush_delete
"buf2d_vox_obj_get_img_w_3g
"buf2d_vox_obj_get_img_h_3g
"buf2d_vox_obj_draw_1g
"buf2d_vox_obj_draw_3g
"buf2d_vox_obj_draw_3g_scaled
"buf2d_vox_obj_draw_pl
"buf2d_vox_obj_draw_pl_scaled
"buf2d_vox_obj_draw_3g_shadows
*/
asm{
	dd 0,0
}


bool buf2d_create_ex(buf_2d_header* buf, long l, long t, long w, long h, unsigned long c=0xffffff, unsigned char bit_pp=24)
{
	buf->left=l;
	buf->top=t;
	buf->size_x=w;
	buf->size_y=h;
	buf->color=c;
	if(bit_pp!=8 && bit_pp!=24 && bit_pp!=32) return false;
	buf->bit_pp=bit_pp;
	buf2d_create(buf);
	return true;
}

#endif