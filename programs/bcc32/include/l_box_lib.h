#ifndef __L_BOX_LIB_H_INCLUDED_
#define __L_BOX_LIB_H_INCLUDED_
//
// box_lib.obj
//

const long ed_pass         = 1; //password mode
const long ed_focus        = 2; //active
const long ed_shift        = 4; //shift holded
const long ed_shift_on     = 8; //selection active
const long ed_shift_bac    = 16; //clear selection
const long ed_left_fl      = 32;
const long ed_offset_fl    = 64;
const long ed_insert       = 128;
const long ed_mouse_on     = 256;
const long ed_ctrl_on      = 512;
const long ed_alt_on       = 0x400;
const long ed_disabled     = 0x800;
const long ed_always_focus = 0x4000;
const long ed_figure_only  = 0x8000;
const long ed_mous_adn_b   = ed_mouse_on | ed_shift_on | ed_shift_bac;
const long ed_shift_off    = ! ed_shift;
const long ed_shift_mcl    = ! ed_shift;
const long ed_shift_on_off = ! ed_shift_on;
const long ed_shift_bac_cl = ! ed_shift_bac;
const long ed_shift_cl     = ! (ed_shift | ed_shift_on | ed_shift_bac);
const long ed_right_fl     = ! ed_left_fl;
const long ed_offset_cl    = ! ed_offset_fl;
const long ed_insert_cl    = ! ed_insert;
const long ed_mouse_on_off = ! ed_mouse_on;
const long ed_ctrl_off     = ! ed_ctrl_on;
const long ed_alt_off      = ! ed_alt_on;

struct edit_box
{
	long width;
	long left;
	long top;
	long color;
	long shift_color;
	long focus_border_color;
	long blur_border_color;
	long text_color;
	long max;
	char* text;
	void* mouse_variable;
	long flags;
	long size;
	long pos;
	long offset;
	short int cl_curs[2]; // x, y
	short int shift[2];
	long height;
	long char_width;
};

const long ch_flag_en = 2;
const long ch_flag_top = 0;
const long ch_flag_middle = 4;
const long ch_flag_bottom = 8;

struct check_box
{
	short int rect[4]; // width, left, height, top
	long text_margin;
	long color;
	long border_color;
	long text_color;
	char* text;
	long flags;
	long size_of_str;
};

struct scrollbar
{
	short x_size, x_pos, y_size, y_pos;
	long btn_height, type, max_area, cur_area, position, bg_color, front_color, line_color, redraw;
	short delta, delta2, r_size_x, r_start_x, r_size_y, r_start_y;
	long m_pos, m_pos_2, m_keys, run_size, position2, work_size, all_redraw, ar_offset;
};

//
// box_lib - import table
//
void   (__stdcall* import_box_lib)() = (void (__stdcall*)())&"lib_init";
//&"version";

void (__stdcall* edit_box_draw)(edit_box* edit) = (void (__stdcall*)(edit_box*))&"edit_box";
void (__stdcall* edit_box_key)(edit_box* edit) = (void (__stdcall*)(edit_box*))&"edit_box_key";
void (__stdcall* edit_box_mouse)(edit_box* edit) = (void (__stdcall*)(edit_box*))&"edit_box_mouse";
void (__stdcall* edit_box_set_text)(edit_box* edit, char* text) = (void (__stdcall*)(edit_box*, char*))&"edit_box_set_text";
//&"version_ed";

void (__stdcall* check_box_init)(check_box* check) = (void (__stdcall*)(check_box*))&"init_checkbox2";
void (__stdcall* check_box_draw)(check_box* check) = (void (__stdcall*)(check_box*))&"check_box_draw2";
void (__stdcall* check_box_mouse)(check_box* check) = (void (__stdcall*)(check_box*))&"check_box_mouse2";
//&"version_ch2";

//void (__stdcall* option_box_draw)(...) = (void (__stdcall*)(...))&"option_box_draw";
//void (__stdcall* option_box_mouse)(...) = (void (__stdcall*)(...))&"option_box_mouse";
//&"version_op";

void (__stdcall* scrollbar_v_draw)(scrollbar* scroll) = (void (__stdcall*)(scrollbar*))&"scrollbar_v_draw";
void (__stdcall* scrollbar_v_mouse)(scrollbar* scroll) = (void (__stdcall*)(scrollbar*))&"scrollbar_v_mouse";
void (__stdcall* scrollbar_h_draw)(scrollbar* scroll) = (void (__stdcall*)(scrollbar*))&"scrollbar_h_draw";
void (__stdcall* scrollbar_h_mouse)(scrollbar* scroll) = (void (__stdcall*)(scrollbar*))&"scrollbar_h_mouse";
//&"version_scrollbar";

//void (__stdcall* dbutton_draw)(...) = (void (__stdcall*)(...))&"dbutton_draw";
//void (__stdcall* dbutton_mouse)(...) = (void (__stdcall*)(...))&"dbutton_mouse";
//&"version_dbutton";

//void (__stdcall* menu_bar_draw)(...) = (void (__stdcall*)(...))&"menu_bar_draw";
//void (__stdcall* menu_bar_mouse)(...) = (void (__stdcall*)(...))&"menu_bar_mouse";
//void (__stdcall* menu_bar_activate)(...) = (void (__stdcall*)(...))&"menu_bar_activate";
//&"version_menu_bar";

//void (__stdcall* FileBrowser_draw)(...) = (void (__stdcall*)(...))&"FileBrowser_draw";
//void (__stdcall* FileBrowser_mouse)(...) = (void (__stdcall*)(...))&"FileBrowser_mouse";
//void (__stdcall* FileBrowser_key)(...) = (void (__stdcall*)(...))&"FileBrowser_key";
//&"version_FileBrowser";

//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_data_init";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_data_clear";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_info_clear";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_key";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_mouse";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_draw";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_info_undo";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_info_redo";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_node_add";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_node_set_data";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_node_get_data";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_node_delete";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_cur_beg";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_cur_next";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_cur_perv";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_node_close_open";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_node_lev_inc";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_node_lev_dec";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_node_move_up";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_node_move_down";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_node_poi_get_info";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_node_poi_get_next_info";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_node_poi_get_data";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_save_mem";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_load_mem";
//void (__stdcall* ...)(...) = (void (__stdcall*)(...))&"tl_get_mem_size";
//&"version_tree_list";

//void (__stdcall* PathShow_prepare)(...) = (void (__stdcall*)(...))&"PathShow_prepare";
//void (__stdcall* PathShow_draw)(...) = (void (__stdcall*)(...))&"PathShow_draw";
//&"version_PathShow";

//void (__stdcall* ted_but_sumb_upper)(...) = (void (__stdcall*)(...))&"ted_but_sumb_upper";
//void (__stdcall* ted_but_sumb_lover)(...) = (void (__stdcall*)(...))&"ted_but_sumb_lover";
//void (__stdcall* ted_but_convert_by_table)(...) = (void (__stdcall*)(...))&"ted_but_convert_by_table";
//void (__stdcall* ted_can_save)(...) = (void (__stdcall*)(...))&"ted_can_save";
//void (__stdcall* ted_clear)(...) = (void (__stdcall*)(...))&"ted_clear";
//void (__stdcall* ted_delete)(...) = (void (__stdcall*)(...))&"ted_delete";
//void (__stdcall* ted_draw)(...) = (void (__stdcall*)(...))&"ted_draw";
//void (__stdcall* ted_init)(...) = (void (__stdcall*)(...))&"ted_init";
//void (__stdcall* ted_init_scroll_bars)(...) = (void (__stdcall*)(...))&"ted_init_scroll_bars";
//void (__stdcall* ted_init_syntax_file)(...) = (void (__stdcall*)(...))&"ted_init_syntax_file";
//void (__stdcall* ted_is_select)(...) = (void (__stdcall*)(...))&"ted_is_select";
//void (__stdcall* ted_key)(...) = (void (__stdcall*)(...))&"ted_key";
//void (__stdcall* ted_mouse)(...) = (void (__stdcall*)(...))&"ted_mouse";
//void (__stdcall* ted_open_file)(...) = (void (__stdcall*)(...))&"ted_open_file";
//void (__stdcall* ted_save_file)(...) = (void (__stdcall*)(...))&"ted_save_file";
//void (__stdcall* ted_text_add)(...) = (void (__stdcall*)(...))&"ted_text_add";
//void (__stdcall* ted_but_select_word)(...) = (void (__stdcall*)(...))&"ted_but_select_word";
//void (__stdcall* ted_but_cut)(...) = (void (__stdcall*)(...))&"ted_but_cut";
//void (__stdcall* ted_but_copy)(...) = (void (__stdcall*)(...))&"ted_but_copy";
//void (__stdcall* ted_but_paste)(...) = (void (__stdcall*)(...))&"ted_but_paste";
//void (__stdcall* ted_but_undo)(...) = (void (__stdcall*)(...))&"ted_but_undo";
//void (__stdcall* ted_but_redo)(...) = (void (__stdcall*)(...))&"ted_but_redo";
//void (__stdcall* ted_but_reverse)(...) = (void (__stdcall*)(...))&"ted_but_reverse";
//void (__stdcall* ted_but_find)(...) = (void (__stdcall*)(...))&"ted_but_find";
//void (__stdcall* ted_but_replace)(...) = (void (__stdcall*)(...))&"ted_but_replace";
//void (__stdcall* ted_text_colored)(...) = (void (__stdcall*)(...))&"ted_text_colored";
//void (__stdcall* ted_go_to_position)(...) = (void (__stdcall*)(...))&"ted_go_to_position";
//&"version_text_edit";

//void (__stdcall* frame_draw)(...) = (void (__stdcall*)(...))&"frame_draw";
//&"version_frame";

//void (__stdcall* progressbar_draw)(...) = (void (__stdcall*)(...))&"progressbar_draw";
//void (__stdcall* progressbar_progress)(...) = (void (__stdcall*)(...))&"progressbar_progress";

//void (__stdcall* tooltip_init)(...) = (void (__stdcall*)(...))&"tooltip_init";
//void (__stdcall* tooltip_delete)(...) = (void (__stdcall*)(...))&"tooltip_delete";
//void (__stdcall* tooltip_test_show)(...) = (void (__stdcall*)(...))&"tooltip_test_show";
//void (__stdcall* tooltip_mouse)(...) = (void (__stdcall*)(...))&"tooltip_mouse";
//void (__stdcall* get_font_size)(...) = (void (__stdcall*)(...))&"get_font_size";
asm{
	dd 0,0
}

#endif