//BOX_LIB - Asper
#ifndef INCLUDE_BOX_LIB_H
#define INCLUDE_BOX_LIB_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_DLL_H
#include "../lib/dll.h"
#endif

dword boxlib = #aEdit_box_lib;
char aEdit_box_lib[]="/sys/lib/box_lib.obj";

dword box_lib_init   = #aboxlib_init;

dword edit_box_draw     = #aEdit_box_draw;
dword edit_box_key      = #aEdit_box_key;
dword edit_box_mouse    = #aEdit_box_mouse;
dword edit_box_set_text = #aEdit_box_set_text;
dword version_ed        = #aVersion_ed;

dword scrollbar_v_draw  = #aScrollbar_v_draw;
dword scrollbar_v_mouse = #aScrollbar_v_mouse;
dword scrollbar_h_draw  = #aScrollbar_h_draw;
dword scrollbar_h_mouse = #aScrollbar_h_mouse;
dword version_scrollbar = #aVersion_scrollbar;

dword PathShow_prepare  = #aPathShow_prepare;
dword PathShow_draw     = #aPathShow_draw;

dword progressbar_draw = #aProgressbar_draw;
dword progressbar_progress = #aProgressbar_progress;

dword frame_draw = #aFrame_draw;

$DD 2 dup 0

char aEdit_box_draw []    = "edit_box";
char aEdit_box_key  []    = "edit_box_key";
char aEdit_box_mouse[]    = "edit_box_mouse";
char aEdit_box_set_text[] = "edit_box_set_text";
char aVersion_ed    []    = "version_ed";

char aboxlib_init[]        = "lib_init";
char aScrollbar_v_draw [] = "scrollbar_v_draw";
char aScrollbar_v_mouse[] = "scrollbar_v_mouse";
char aScrollbar_h_draw [] = "scrollbar_h_draw";
char aScrollbar_h_mouse[] = "scrollbar_h_mouse";
char aVersion_scrollbar[] = "version_scrollbar";

char aPathShow_prepare [] = "PathShow_prepare";
char aPathShow_draw    [] = "PathShow_draw";

char aProgressbar_draw  [] = "progressbar_draw";
char aProgressbar_progress[] = "progressbar_progress";

char aFrame_draw[] = "frame_draw";


struct PathShow_data
{
dword type;
word start_y,
	start_x,
	font_size_x,    // 6 - for font 0, 8 - for font 1
	area_size_x;
dword font_number,  // 0 - monospace, 1 - variable
	background_flag,
	font_color,
	background_color,
	text_pointer,
	work_area_pointer,
	temp_text_length;
};
/*
char temp[128];
PathShow_data PathShow = {0, 100,20, 6, 200, 0, 1, 0x0, 0xFFFfff, #email_text, #temp, 0};
PathShow_prepare stdcall(#PathShow);
PathShow_draw stdcall(#PathShow);
*/

//editbox flags
#define ed_pass                        1b
#define ed_focus                      10b   //focused
#define ed_shift                     100b   //flag is set when Shift is pressed
#define ed_shift_on                 1000b
#define ed_shift_bac               10000b   //bit for Shift reset, if set the smth is selected
#define ed_left_fl                100000b
#define ed_offset_fl             1000000b
#define ed_insert               10000000b
#define ed_mouse_on            100000000b
#define ed_mouse_adn_b         100011000b
#define ed_disabled         100000000000b
#define ed_always_focus  100000000000000b
#define ed_figure_only  1000000000000000b   //numbers only
#define ed_shift_cl     1111111111100011b
#define ed_shift_mcl    1111111111111011b
#define ed_shift_off    1111111111111011b
#define ed_shift_on_off 1111111111110111b
#define ed_shift_bac_cl 1111111111101111b
#define ed_right_fl     1111111111011111b
#define ed_offset_cl    1111111110111111b
#define ed_insert_cl    1111111101111111b
#define ed_mouse_on_off 1111111011111111b

struct edit_box{
dword width, 
	left,
	top, 
	color, 
	shift_color, 
	focus_border_color, 
	blur_border_color,
	text_color,
	max,
	text,
	mouse_variable,
	flags,
	size,
	pos,
	offset,
	cl_curs_x,
	cl_curs_y,
	shift,
	shift_old,
	height,
	char_width;
};

:void EditBox_UpdateText(dword ed, _flags)
{
	dword ed_text;
	ESI = ed;
	ESI.edit_box.offset = ESI.edit_box.shift = ESI.edit_box.shift_old = 0;
	ESI.edit_box.flags = _flags;
	ed_text = ESI.edit_box.text;
	ESI.edit_box.pos = ESI.edit_box.size = strlen(ed_text);
}

struct scroll_bar
{
	word size_x,
	start_x,
	size_y,
	start_y;
	dword btn_height,
	type,
	max_area,
	cur_area,
	position,
	bckg_col,
	frnt_col,
	line_col,
	redraw;
	word delta,
	delta2,
	r_size_x,
	r_start_x,
	r_size_y,
	r_start_y;
	dword m_pos,
	m_pos_2,
	m_keys,
	run_size,
	position2,
	work_size,
	all_redraw,
	ar_offset;
};

struct progress_bar
{
  dword
	value,
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

struct frame
{
	dword type;
	word size_x; //start_x, size_x => Mario, WTF? Is this so complex to use x/y/w/h ?                  
	word start_x;                 
	word size_y;                  
	word start_y;                
	dword ext_col;            
	dword int_col;            
	dword flags;  // see FR_FLAGS
	dword text_pointer;          
	dword text_position;   //  0-up,1-bottom
	dword font_number;     //  0-monospace,1-variable
	dword font_size_y;           
	dword font_color;            
	dword font_backgr_color;
};

// FR_FLAGS = [x][yyy][z]
// z        -  Caption
// yyy      -  BorderStyle
// x        -  BackStyle
#define FR_CAPTION 00001b // [z]
#define FR_DOUBLE  00000b // [yyy]
#define FR_RAISED  00010b // [yyy]
#define FR_SUNKEN  00100b // [yyy]
#define FR_ETCHED  00110b // [yyy]
#define FR_RIDGED  01000b // [yyy]
#define FR_FILLED  10000b // [x]

:frame frame123 = { 0, 260, 10, 60, 16, NULL, 0xFFFfff, 1, NULL, 0, 1, 12, 0x000111, 0xCCCccc };
:void DrawFrame(dword x,y,w,h,text)
{
	frame123.font_color = system.color.work_text;
	frame123.ext_col = system.color.work_graph;
	frame123.int_col = system.color.work_light;
	frame123.font_backgr_color = system.color.work;

	frame123.start_x = x;
	frame123.start_y = y;
	frame123.size_x = w;
	frame123.size_y = h;
	frame123.text_pointer = text;
	if (!text) frame123.flags=0; else frame123.flags=FR_CAPTION;
	frame_draw stdcall (#frame123);
}


#endif