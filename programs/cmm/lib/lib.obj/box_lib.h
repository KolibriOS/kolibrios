//BOX_LIB - Asper
dword boxlib = #aEdit_box_lib;
char aEdit_box_lib[22]="/sys/lib/box_lib.obj\0";

dword box_lib_init   = #aboxlib_init;

dword edit_box_draw  = #aEdit_box_draw;
dword edit_box_key   = #aEdit_box_key;
dword edit_box_mouse = #aEdit_box_mouse;
dword version_ed     = #aVersion_ed;

dword scrollbar_v_draw  = #aScrollbar_v_draw;
dword scrollbar_v_mouse = #aScrollbar_v_mouse;
dword scrollbar_h_draw  = #aScrollbar_h_draw;
dword scrollbar_h_mouse = #aScrollbar_h_mouse;
dword version_scrollbar = #aVersion_scrollbar;

dword PathShow_prepare = #aPathShow_prepare;
dword PathShow_draw    = #aPathShow_draw;

dword progressbar_draw = #aProgressbar_draw;
dword progressbar_progress = #aProgressbar_progress;

$DD 2 dup 0

char aEdit_box_draw [9]     = "edit_box\0";
char aEdit_box_key  [13]    = "edit_box_key\0";
char aEdit_box_mouse[15]    = "edit_box_mouse\0";
char aVersion_ed    [11]    = "version_ed\0";

char aboxlib_init[9]        = "lib_init\0";
char aScrollbar_v_draw [17] = "scrollbar_v_draw\0";
char aScrollbar_v_mouse[18] = "scrollbar_v_mouse\0";
char aScrollbar_h_draw [17] = "scrollbar_h_draw\0";
char aScrollbar_h_mouse[18] = "scrollbar_h_mouse\0";
char aVersion_scrollbar[18] = "version_scrollbar\0";

char aCheck_box_draw   [15] = "check_box_draw\0";
char aCheck_box_mouse  [16] = "check_box_mouse\0";
char aVersion_ch       [11] = "version_ch\0";

char aOption_box_draw  [16] = "option_box_draw\0";
char aOption_box_mouse [17] = "option_box_mouse\0";
char aVersion_op       [11] = "version_op\0" ;

char aPathShow_prepare [17] = "PathShow_prepare\0";
char aPathShow_draw    [14] = "PathShow_draw\0";

char aProgressbar_draw  [17] = "progressbar_draw\0";
char aProgressbar_progress[21] = "progressbar_progress\0";


struct PathShow_data
{
dword type;//			dd 0	;+0
word start_y,//		dw 28	;+4
	start_x,//		dw 172	;+6
	font_size_x,//		dw 6	;+8	; 6 - for font 0, 8 - for font 1
	area_size_x;//		dw 200	;+10
dword font_number,//		dd 0	;+12	; 0 - monospace, 1 - variable
	background_flag,//	dd 0	;+16
	font_color,//		dd 0x0	;+20
	background_color,//	dd 0x0	;+24
	text_pointer,//		dd openfile_pach	;+28
	work_area_pointer,//	dd text_work_area	;+32
	temp_text_length;//	dd 0	;+36
};
/*
char temp[128];
PathShow_data PathShow = {0, 100,20, 6, 200, 0, 1, 0x0, 0xFFFfff, #email_text, #temp, 0};
PathShow_prepare stdcall(#PathShow);
PathShow_draw stdcall(#PathShow);
*/


struct edit_box{
dword width, left, top, color, shift_color, focus_border_color, blur_border_color,
text_color, max, text, mouse_variable, flags, size, pos, offset, cl_curs_x, cl_curs_y, shift, shift_old;
};

struct scroll_bar
{
word size_x,//  equ [edi]     
start_x,//      equ [edi+2]
size_y,//		equ [edi+4]
start_y;//		equ [edi+6]
dword btn_height, //equ [edi+8]
type,//			equ [edi+12]
max_area,//		equ [edi+16]
cur_area,//		equ [edi+20]
position,//		equ [edi+24]
bckg_col,//		equ [edi+28]
frnt_col,//		equ [edi+32]
line_col,//		equ [edi+36]
redraw;//		equ [edi+40]
word delta,//		equ [edi+44]
delta2,//		equ [edi+46]
r_size_x,//		equ [edi+48]
r_start_x,//	equ [edi+50]
r_size_y,//		equ [edi+52]
r_start_y;//	equ [edi+54]
dword m_pos,//		equ [edi+56]
m_pos_2,//		equ [edi+60]
m_keys,//		equ [edi+64]
run_size,//		equ [edi+68]
position2,//	equ [edi+72]
work_size,//	equ [edi+76]
all_redraw,//	equ [edi+80]
ar_offset;//	equ [edi+84]
};

struct pb //progressbar
{
dword value,
left,
top,
width,
height,
style,
min,
max,
back_color,
progress_color,
frame_color;
};