align 4
boxlib_import:
  edit_box_draw  dd aEdit_box_draw
  edit_box_key   dd aEdit_box_key
  edit_box_mouse dd aEdit_box_mouse
;  version_ed     dd aVersion_ed

  check_box_draw  dd aCheck_box_draw
  check_box_mouse dd aCheck_box_mouse
;  version_ch      dd aVersion_ch

;  option_box_draw dd aOption_box_draw
;  option_box_mouse dd aOption_box_mouse
;  version_op dd aVersion_op

  scrollbar_ver_draw  dd aScrollbar_ver_draw
;  scrollbar_ver_mouse dd aScrollbar_ver_mouse
  scrollbar_hor_draw  dd aScrollbar_hor_draw
;  scrollbar_hor_mouse dd aScrollbar_hor_mouse
;  version_scrollbar   dd aVersion_scrollbar

  tl_data_init dd sz_tl_data_init
  tl_data_clear dd sz_tl_data_clear
  tl_info_clear dd sz_tl_info_clear
  tl_key dd sz_tl_key
  tl_mouse dd sz_tl_mouse
  tl_draw dd sz_tl_draw
  tl_info_undo dd sz_tl_info_undo
  tl_info_redo dd sz_tl_info_redo
  tl_node_add dd sz_tl_node_add
  tl_node_set_data dd sz_tl_node_set_data
  tl_node_get_data dd sz_tl_node_get_data
  tl_node_delete dd sz_tl_node_delete
  tl_cur_beg dd sz_tl_cur_beg
  tl_cur_next dd sz_tl_cur_next
  tl_cur_perv dd sz_tl_cur_perv
  tl_node_close_open dd sz_tl_node_close_open
  tl_node_lev_inc dd sz_tl_node_lev_inc
  tl_node_lev_dec dd sz_tl_node_lev_dec
  tl_node_move_up dd sz_tl_node_move_up
  tl_node_move_down dd sz_tl_node_move_down
  tl_save_mem dd sz_tl_save_mem
  tl_load_mem dd sz_tl_load_mem
  tl_get_mem_size dd sz_tl_get_mem_size

  dd 0,0

  aEdit_box_draw  db 'edit_box',0
  aEdit_box_key   db 'edit_box_key',0
  aEdit_box_mouse db 'edit_box_mouse',0
;  aVersion_ed     db 'version_ed',0

  aCheck_box_draw  db 'check_box_draw',0
  aCheck_box_mouse db 'check_box_mouse',0
;  aVersion_ch      db 'version_ch',0

;  aOption_box_draw db 'option_box_draw',0
;  aOption_box_mouse db 'option_box_mouse',0
;  aVersion_op db 'version_op',0

  aScrollbar_ver_draw  db 'scrollbar_v_draw',0
;  aScrollbar_ver_mouse db 'scrollbar_v_mouse',0
  aScrollbar_hor_draw  db 'scrollbar_h_draw',0
;  aScrollbar_hor_mouse db 'scrollbar_h_mouse',0
;  aVersion_scrollbar   db 'version_scrollbar',0

  sz_tl_data_init db 'tl_data_init',0
  sz_tl_data_clear db 'tl_data_clear',0
  sz_tl_info_clear db 'tl_info_clear',0
  sz_tl_key db 'tl_key',0
  sz_tl_mouse db 'tl_mouse',0
  sz_tl_draw db 'tl_draw',0
  sz_tl_info_undo db 'tl_info_undo',0
  sz_tl_info_redo db 'tl_info_redo',0
  sz_tl_node_add db 'tl_node_add',0
  sz_tl_node_set_data db 'tl_node_set_data',0
  sz_tl_node_get_data db 'tl_node_get_data',0
  sz_tl_node_delete db 'tl_node_delete',0
  sz_tl_cur_beg db 'tl_cur_beg',0
  sz_tl_cur_next db 'tl_cur_next',0
  sz_tl_cur_perv db 'tl_cur_perv',0
  sz_tl_node_close_open db 'tl_node_close_open',0
  sz_tl_node_lev_inc db 'tl_node_lev_inc',0
  sz_tl_node_lev_dec db 'tl_node_lev_dec',0
  sz_tl_node_move_up db 'tl_node_move_up',0
  sz_tl_node_move_down db 'tl_node_move_down',0
  sz_tl_save_mem db 'tl_save_mem',0
  sz_tl_load_mem db 'tl_load_mem',0
  sz_tl_get_mem_size db 'tl_get_mem_size',0

align 4
libini_import:
  lib_init_ini dd alib_init0
  ini_get_str  dd aini_get_str
dd 0,0
  alib_init0   db 'lib_init',0
  aini_get_str db 'ini_get_str',0

align 4
libimg_import:
  lib_init    dd alib_init1
  img_is_img  dd aimg_is_img
  img_info    dd aimg_info
  img_from_file dd aimg_from_file
  img_to_file dd aimg_to_file
  img_from_rgb dd aimg_from_rgb
  img_to_rgb  dd aimg_to_rgb
  img_to_rgb2 dd aimg_to_rgb2
  img_decode  dd aimg_decode
  img_encode  dd aimg_encode
  img_create  dd aimg_create
  img_destroy dd aimg_destroy
  img_destroy_layer dd aimg_destroy_layer
  img_count   dd aimg_count
  img_lock_bits dd aimg_lock_bits
  img_unlock_bits dd aimg_unlock_bits
  img_flip    dd aimg_flip
  img_flip_layer dd aimg_flip_layer
  img_rotate  dd aimg_rotate
  img_rotate_layer dd aimg_rotate_layer
  img_draw    dd aimg_draw

  dd 0,0

  alib_init1   db 'lib_init',0
  aimg_is_img  db 'img_is_img',0 ;определяет по данным, может ли библиотека сделать из них изображение
  aimg_info    db 'img_info',0
  aimg_from_file db 'img_from_file',0
  aimg_to_file db 'img_to_file',0
  aimg_from_rgb db 'img_from_rgb',0
  aimg_to_rgb  db 'img_to_rgb',0 ;преобразование изображения в данные RGB
  aimg_to_rgb2 db 'img_to_rgb2',0
  aimg_decode  db 'img_decode',0 ;автоматически определяет формат графических данных
  aimg_encode  db 'img_encode',0
  aimg_create  db 'img_create',0
  aimg_destroy db 'img_destroy',0
  aimg_destroy_layer db 'img_destroy_layer',0
  aimg_count   db 'img_count',0
  aimg_lock_bits db 'img_lock_bits',0
  aimg_unlock_bits db 'img_unlock_bits',0
  aimg_flip    db 'img_flip',0
  aimg_flip_layer db 'img_flip_layer',0
  aimg_rotate  db 'img_rotate',0
  aimg_rotate_layer db 'img_rotate_layer',0
  aimg_draw    db 'img_draw',0

align 4
strlib_import:
  str_copy dd astr_copy
  str_cat dd astr_cat
  str_catchar dd astr_catchar
  str_inschar dd astr_inschar
  str_lowcase dd astr_lowcase
  str_upcase dd astr_upcase

  str_len dd astr_len
  str_cmp dd astr_cmp
  str_cmpi dd astr_cmpi
  str_poschar dd astr_poschar

  dd 0,0
  astr_copy    db 'str_copy',0
  astr_cat     db 'str_cat',0
  astr_catchar db 'str_catchar',0
;  str_ins     db 'str_ins',0
  astr_inschar db 'str_inschar',0
;  str_sub     db 'str_sub',0
  astr_lowcase db 'str_lowcase',0
  astr_upcase db 'str_upcase',0

  astr_len  db 'str_len',0
  astr_cmp  db 'str_cmp',0
  astr_cmpi db 'str_cmpi',0
;  str_pos  db 'str_pos',0
  astr_poschar db 'str_poschar',0