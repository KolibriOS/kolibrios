struct EDIT_BOX
  width 	     dd ?
  left		     dd ?
  top		     dd ?
  color 	     dd ?
  shift_color	     dd ?
  focus_border_color dd ?
  blur_border_color  dd ?
  text_color	     dd ?
  max		     dd ?
  text		     dd ?
  mouse_variable     dd ?
  flags 	     dd ?
  size		     dd ?
  pos		     dd ?
  offset	     dd ?
  cl_curs_x	     dw ?
  cl_curs_y	     dw ?
  shift 	     dw ?
  shift_old	     dw ?
  height	     dd ?
  char_width	     dd ?
ends

struct CHECK_BOX2
  left_s	     dd ?
  top_s 	     dd ?
  ch_text_margin     dd ?
  color 	     dd ?
  border_color	     dd ?
  text_color	     dd ?
  text		     dd ?
  flags 	     dd ?
  size_of_str	     dd ?
ends

struct OPTION_BOX
  option_group	     dd ?
  left		     dw ?
  top		     dw ?
  ch_text_margin     dd ?
  ch_size	     dd ?
  color 	     dd ?
  border_color	     dd ?
  text_color	     dd ?
  text		     dd ?
  ch_text_length     dd ?
  flags 	     dd ?
ends

struct TREE_LIST
  box		     BOX
  data_info	     dd ?
  info_size	     dd ?
  info_max_count     dd ?
  style 	     dd ?
  data_nodes	     dd ?
  data_img	     dd ?
  img_cx	     dw ?
  img_cy	     dw ?
  data_img_sys	     dd ?
  ch_tim	     dd ?
  tim_undo	     dd ?
  cur_pos	     dd ?
  col_bkg	     dd ?
  col_zag	     dd ?
  col_txt	     dd ?
  capt_cy	     dd ?
  info_capt_offs     dw ?
  info_capt_len      dw ?
  el_focus	     dd ?
  p_scroll	     dd ?
  on_press	     dd ?
ends

struct TEXT_EDITOR
  wnd		     BOX
  rec		     BOX
  drag_m	     db ?
  drag_k	     db ?
  sel_x0	     dd ?
  sel_y0	     dd ?
  sel_x1	     dd ?
  sel_y1	     dd ?
  seln_x0	     dd ?
  seln_y0	     dd ?
  seln_x1	     dd ?
  seln_y1	     dd ?
  tex		     dd ?
  tex_1 	     dd ?
  tex_end	     dd ?
  cur_x 	     dd ?
  cur_y 	     dd ?
  max_chars	     dd ?
  colors_text_count  dd ?
  key_words_count    dd ?
  color_cursor	     dd ?
  color_wnd_capt     dd ?
  color_wnd_work     dd ?
  color_wnd_bord     dd ?
  color_select	     dd ?
  color_cur_text     dd ?
  color_wnd_text     dd ?
  syntax_file	     dd ?
  syntax_file_size   dd ?
  text_colors	     dd ?
  help_text_f1	     dd ?
  help_id	     dd ?
  key_words_data     dd ?
  tim_ch	     dd ?
  tim_undo	     dd ?
  tim_ls	     dd ?
  tim_co	     dd ?
  el_focus	     dd ?
  err_save	     dd ?
  panel_id	     dd ?
  key_new	     dd ?
  symbol_new_line    dd ?
  scr_w 	     dd ?
  scr_h 	     dd ?
  arr_key_pos	     dd ?
  buffer	     dd ?
  buffer_find	     dd ?
  cur_ins	     db ?
  mode_color	     db ?
  mode_invis	     db ?
  gp_opt	     db ?
  fun_on_key_ctrl_o  dd ?
  fun_on_key_ctrl_f  dd ?
  fun_on_key_ctrl_n  dd ?
  fun_on_key_ctrl_s  dd ?
  buffer_size	     dd ?
  fun_find_err	     dd ?
  fun_init_synt_err  dd ?
  fun_draw_panel_buttons dd ?
  fun_draw_panel_find dd ?
  fun_draw_panel_syntax dd ?
  fun_save_err	     dd ?
  increase_size      dd ?
  ptr_free_symb      dd ?
  font_size	     dd ?
ends

TED_OFFS_SEL = TEXT_EDITOR.sel_x0
TED_OFFS_SELN = TEXT_EDITOR.seln_x0
TED_OFFS_COUNT_COLORS = TEXT_EDITOR.colors_text_count
TED_OFFS_KEY_NEW = TEXT_EDITOR.key_new
TED_OFFS_SYMBOL_NEW_LINE = TEXT_EDITOR.symbol_new_line

struct SCROLL_BAR
  x_size	     dw ?
  x_pos 	     dw ?
  y_size	     dw ?
  y_pos 	     dw ?
  btn_height	     dd ?
  type		     dd ?
  max_area	     dd ?
  cur_area	     dd ?
  position	     dd ?
  bg_color	     dd ?
  front_color	     dd ?
  line_color	     dd ?
  redraw	     dd ?
  delta 	     dw ?
  delta2	     dw ?
  r_size_x	     dw ?
  r_start_x	     dw ?
  r_size_y	     dw ?
  r_start_y	     dw ?
  m_pos 	     dd ?
  m_pos_2	     dd ?
  m_keys	     dd ?
  run_size	     dd ?
  position2	     dd ?
  work_size	     dd ?
  all_redraw	     dd ?
  ar_offset	     dd ?
ends

SB_OFFS_SIZE_X = SCROLL_BAR.x_size
SB_OFFS_START_X = SCROLL_BAR.x_pos
SB_OFFS_SIZE_Y = SCROLL_BAR.y_size
SB_OFFS_START_Y = SCROLL_BAR.y_pos
SB_OFFS_MAX_AREA = SCROLL_BAR.max_area
SB_OFFS_CUR_AREA = SCROLL_BAR.cur_area
SB_OFFS_POSITION = SCROLL_BAR.position
SB_OFFS_BCKG_COL = SCROLL_BAR.bg_color
SB_OFFS_FRNT_COL = SCROLL_BAR.front_color
SB_OFFS_LINE_COL = SCROLL_BAR.line_color
SB_OFFS_REDRAW = SCROLL_BAR.redraw
SB_OFFS_DELTA2 = SCROLL_BAR.delta2
SB_OFFS_ALL_REDRAW = SCROLL_BAR.all_redraw

struct TOOLTIP
  txt		     dd ?
  next		     dd ?
  zone_y	     dd ?
  zone_x	     dd ?
  zone_h	     dd ?
  zone_w	     dd ?
  col_txt	     dd ?
  col_bkg	     dd ?
  tm_wait	     dd ?
  font_sz	     dd ?
  mouse 	     dd ?
  bg_color	     dd ?
  front_color	     dd ?
  line_color	     dd ?
  redraw	     dd ?
  delta 	     dw ?
  delta2	     dw ?
  r_size_x	     dw ?
  r_start_x	     dw ?
  r_size_y	     dw ?
  r_start_y	     dw ?
  m_pos 	     dd ?
  m_pos_2	     dd ?
  m_keys	     dd ?
  run_size	     dd ?
  position2	     dd ?
  work_size	     dd ?
  all_redraw	     dd ?
  ar_offset	     dd ?
ends






