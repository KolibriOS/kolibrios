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

dword edit_box_draw  = #aEdit_box_draw;
dword edit_box_key   = #aEdit_box_key;
dword edit_box_mouse = #aEdit_box_mouse;
dword version_ed     = #aVersion_ed;

dword menu_bar_draw             = #aMenu_bar_draw;
dword menu_bar_activate         = #aMenu_bar_activate;
dword menu_bar_mouse            = #aMenu_bar_mouse;
dword version_menu_bar          = #aVersion_menu_bar;

dword scrollbar_v_draw  = #aScrollbar_v_draw;
dword scrollbar_v_mouse = #aScrollbar_v_mouse;
dword scrollbar_h_draw  = #aScrollbar_h_draw;
dword scrollbar_h_mouse = #aScrollbar_h_mouse;
dword version_scrollbar = #aVersion_scrollbar;

dword PathShow_prepare = #aPathShow_prepare;
dword PathShow_draw    = #aPathShow_draw;

dword progressbar_draw = #aProgressbar_draw;
dword progressbar_progress = #aProgressbar_progress;

dword check_box_draw = #aCheck_box_draw;
dword check_box_mouse = #aCheck_box_mouse;
dword version_ch = #aVersion_ch;

dword frame_draw = #aFrame_draw;

$DD 2 dup 0

char aEdit_box_draw []    = "edit_box";
char aEdit_box_key  []    = "edit_box_key";
char aEdit_box_mouse[]    = "edit_box_mouse";
char aVersion_ed    []    = "version_ed";

char aMenu_bar_draw    []    = "menu_bar_draw";
char aMenu_bar_activate[]    = "menu_bar_activate";
char aMenu_bar_mouse   []    = "menu_bar_mouse";
char aVersion_menu_bar []    = "version_menu_bar";

char aboxlib_init[]        = "lib_init";
char aScrollbar_v_draw [] = "scrollbar_v_draw";
char aScrollbar_v_mouse[] = "scrollbar_v_mouse";
char aScrollbar_h_draw [] = "scrollbar_h_draw";
char aScrollbar_h_mouse[] = "scrollbar_h_mouse";
char aVersion_scrollbar[] = "version_scrollbar";

char aCheck_box_draw   [] = "check_box_draw2";
char aCheck_box_mouse  [] = "check_box_mouse2";
char aVersion_ch       [] = "version_ch2";

char aOption_box_draw  [] = "option_box_draw";
char aOption_box_mouse [] = "option_box_mouse";
char aVersion_op       [] = "version_op" ;

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

struct menu_data
{
	dword type;
	
	word size_x;
	word start_x;
	
	word size_y;
	word start_y;
	dword text_pointer;
	dword pos_pointer;
	dword text_end;
	dword mouse_pos;
	dword mouse_keys;
	
	word size_x1;
	word start_x1;
	
	word size_y1;
	word start_y1;
	dword bckg_col;
	dword frnt_col;
	dword menu_col;
	dword select;
	dword out_select;
	dword buf_adress;
	dword procinfo;
	dword click;
	dword cursor;
	dword cursor_old;
	dword interval;
	dword cursor_max;
	dword extended_key;
	dword menu_sel_col;
	dword bckg_text_col;
	dword frnt_text_col;
	dword mouse_keys_old;
	dword font_height;
	dword cursor_out;
	dword get_mouse_flag;
};

struct edit_box{
dword width, left, top, color, shift_color, focus_border_color, blur_border_color,
text_color, max, text, mouse_variable, flags, size, pos, offset, cl_curs_x, cl_curs_y, shift, shift_old;
};

struct checkbox2
{
  dword
	left_s,
	top_s,
	ch_text_margin,
	color,
	border_color,
	text_color,
	text,
	flags,
	size_of_str;
};

//flags for checkbox2
#define CH_FLAG_EN 10b      
#define CH_FLAG_TOP 0x0     
#define CH_FLAG_MIDDLE 100b 
#define CH_FLAG_BOTTOM 1000b 

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
	word size_x;                  
	word start_x;                 
	word size_y;                  
	word start_y;                
	dword ext_col;            
	dword int_col;            
	dword draw_text_flag;  // 0-not,1-yes
	dword text_pointer;          
	dword text_position;   //  0-up,1-bottom
	dword font_number;     //  0-monospace,1-variable
	dword font_size_y;           
	dword font_color;            
	dword font_backgr_color;
};     

#endif