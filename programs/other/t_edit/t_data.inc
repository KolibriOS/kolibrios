;---  фрээ√х яЁюуЁрьь√  ----------------------------------------------
fn_col_option dd def_col_file
fn_syntax_dir db 'info/',0
def_col_file db 'default.syn',0

system_dir0 db '/sys/lib/'
boxlib_name db 'box_lib.obj',0

system_dir1 db '/sys/lib/'
msgbox_name db 'msgbox.obj',0

fn_icon_tl_sys db 'tl_sys_16.bmp',0
fn_icon_tl_nod db 'tl_nod_16.bmp',0
;---------------------------------------------------------------------
fileinfo:
  dd 5
  dd 0
  dd 0
  dd 0
  dd file_info
string:
  db 0
  dd file_name

;plugin dd 0

;---------------------------------------------------------------------
align 4
myimport:
edit_box_draw  dd aEdit_box_draw
edit_box_key   dd aEdit_box_key
edit_box_mouse dd aEdit_box_mouse
version_ed     dd aVersion_ed

check_box_draw	dd aCheck_box_draw
check_box_mouse dd aCheck_box_mouse
version_ch	dd aVersion_ch

option_box_draw  dd aOption_box_draw
option_box_mouse dd aOption_box_mouse
version_op	 dd aVersion_op

scrollbar_ver_draw  dd aScrollbar_ver_draw
scrollbar_ver_mouse dd aScrollbar_ver_mouse
scrollbar_hor_draw  dd aScrollbar_hor_draw
scrollbar_hor_mouse dd aScrollbar_hor_mouse
version_scrollbar   dd aVersion_scrollbar

dinamic_button_draw  dd aDbutton_draw
dinamic_button_mouse dd aDbutton_mouse
version_dbutton      dd aVersion_dbutton

menu_bar_draw	 dd aMenu_bar_draw
menu_bar_mouse	 dd aMenu_bar_mouse
version_menu_bar dd aVersion_menu_bar

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
dd 0,0

aEdit_box_draw	db 'edit_box',0
aEdit_box_key	db 'edit_box_key',0
aEdit_box_mouse db 'edit_box_mouse',0
aVersion_ed	db 'version_ed',0

aCheck_box_draw  db 'check_box_draw',0
aCheck_box_mouse db 'check_box_mouse',0
aVersion_ch	 db 'version_ch',0

aOption_box_draw  db 'option_box_draw',0
aOption_box_mouse db 'option_box_mouse',0
aVersion_op	  db 'version_op',0

aScrollbar_ver_draw  db 'scrollbar_v_draw',0
aScrollbar_ver_mouse db 'scrollbar_v_mouse',0
aScrollbar_hor_draw  db 'scrollbar_h_draw',0
aScrollbar_hor_mouse db 'scrollbar_h_mouse',0
aVersion_scrollbar   db 'version_scrollbar',0

aDbutton_draw	  db 'dbutton_draw',0
aDbutton_mouse	  db 'dbutton_mouse',0
aVersion_dbutton  db 'version_dbutton',0

aMenu_bar_draw	  db 'menu_bar_draw',0
aMenu_bar_mouse   db 'menu_bar_mouse',0
aVersion_menu_bar db 'version_menu_bar',0

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
;--------------------------------------------------
align 4
msgbox_lib_import:
  mb_create dd amb_create
  mb_reinit dd amb_reinit
  mb_setfunctions dd amb_setfunctions
  dd 0,0
  amb_create db 'mb_create',0
  amb_reinit db 'mb_reinit',0
  amb_setfunctions db 'mb_setfunctions',0
;---------------------------------------------------------------------   
align 4
wScr:
.x:
.size_x     dw 16 ;+0
.start_x    dw 50 ;+2
.y:
.size_y     dw 300 ;+4
.start_y    dw 50 ;+6
.btn_high   dd 15 ;+8
.type	    dd 1  ;+12
.max_area   dd 100  ;+16
.cur_area   dd 30  ;+20
.position   dd 0  ;+24
.bckg_col   dd 0xeeeeee ;+28
.frnt_col   dd 0xbbddff ;+32
.line_col   dd 0  ;+36
.redraw     dd 0  ;+40
.delta	    dw 0  ;+44
.delta2     dw 0  ;+46
.run_x:
.r_size_x   dw 0  ;+48
.r_start_x  dw 0  ;+50
.run_y:
.r_size_y   dw 0 ;+52
.r_start_y  dw 0 ;+54
.m_pos	    dd 0 ;+56
.m_pos_2    dd 0 ;+60
.m_keys     dd 0 ;+64
.run_size   dd 0 ;+68
.position2  dd 0 ;+72
.work_size  dd 0 ;+76
.all_redraw dd 0 ;+80
.ar_offset  dd 1 ;+84
;---------------------------------------------------------------------
align 4
hScr:
.x:
.size_x     dw 150 ;+0
.start_x    dw 0 ;+2
.y:
.size_y     dw 16 ;+4
.start_y    dw 50 ;+6
.btn_high   dd 15 ;+8
.type	    dd 1  ;+12
.max_area   dd 100 ;+16
.cur_area   dd 30  ;+20
.position   dd 0  ;+24
.bckg_col   dd 0xeeeeee ;+28
.frnt_col   dd 0xbbddff ;+32
.line_col   dd 0  ;+36
.redraw     dd 0  ;+40
.delta	    dw 0  ;+44
.delta2     dw 0  ;+46
.run_x:
.r_size_x   dw 0  ;+48
.r_start_x  dw 0  ;+50
.run_y:
.r_size_y   dw 0 ;+52
.r_start_y  dw 0 ;+54
.m_pos	    dd 0 ;+56
.m_pos_2    dd 0 ;+60
.m_keys     dd 0 ;+64
.run_size   dd 0 ;+68
.position2  dd 0 ;+72
.work_size  dd 0 ;+76
.all_redraw dd 0 ;+80
.ar_offset  dd 1 ;+84
;---------------------------------------------------------------------   
align 4
ws_dir_lbox: ;фюўхЁэшщ ёъЁюыышэу фы  ¤ыхьхэЄр ListBox
.x:
.size_x     dw 16 ;+0
.start_x    dw 0 ;+2
.y:
.size_y     dw 30 ;+4
.start_y    dw 70 ;+6
.btn_high   dd 15 ;+8
.type	    dd 1  ;+12
.max_area   dd 100  ;+16
.cur_area   dd 30  ;+20
.position   dd 0  ;+24
.bckg_col   dd 0xeeeeee ;+28
.frnt_col   dd 0xbbddff ;+32
.line_col   dd 0  ;+36
.redraw     dd 0  ;+40
.delta	    dw 0  ;+44
.delta2     dw 0  ;+46
.run_x:
.r_size_x   dw 0  ;+48
.r_start_x  dw 0  ;+50
.run_y:
.r_size_y   dw 0 ;+52
.r_start_y  dw 0 ;+54
.m_pos	    dd 0 ;+56
.m_pos_2    dd 0 ;+60
.m_keys     dd 0 ;+64
.run_size   dd 0 ;+68
.position2  dd 0 ;+72
.work_size  dd 0 ;+76
.all_redraw dd 0 ;+80
.ar_offset  dd 1 ;+84;---------------------------------------------------------------------

align 4
menu_data_1:
.type:			dd 0   ;+0
.x:
.size_x 		dw 35  ;+4
.start_x		dw 5   ;+6
.y:
.size_y 		dw 15	;+8
.start_y		dw 5  ;+10
.text_pointer:	dd menu_text_area  ;0 ;+12
.pos_pointer:	dd menu_text_area.1 ;0 ;+16
.text_end		dd menu_text_area.end ;0 ;+20
.mouse_pos		dd 0  ;+24
.mouse_keys		dd 0  ;+28
.x1:
.size_x1		dw 70  ;+32
.start_x1		dw 5   ;+34
.y1:
.size_y1		dw 15	;+36
.start_y1		dw 20  ;+38
.bckg_col	dd 0xeeeeee ;+40
.frnt_col	dd 0xff ;+44
.menu_col	dd 0xffffff ;+48
.select 	dd 0 ;+52
.out_select	dd 0 ;+56
.buf_adress		dd 0 ;+60
.procinfo		dd procinfo ;0 ;+64
.click			dd 0 ;+68
.cursor 		dd 0 ;+72
.cursor_old		dd 0 ;+76
.interval		dd 16 ;+80
.cursor_max		dd 0 ;+84
.extended_key	dd 0 ;+88
.menu_sel_col	dd 0x00cc00 ;+92
.bckg_text_col	dd 0 ; +96
.frnt_text_col	dd 0xffffff ;+100
.mouse_keys_old dd 0 ;+104
.font_height	dd 8 ;+108
.cursor_out		dd 0 ;+112
.get_mouse_flag dd 0 ;+116
;-------------------------

TE_PANEL_NULL equ 0
TE_PANEL_FIND equ 1
TE_PANEL_SYNTAX equ 2
TE_PANEL_WIDTH equ 150 ;°шЁшэр ярэхыш

count_of_dir_list_files equ 15

el_focus dd 0
mouse_dd dd 0
tree1 tree_list 264,count_of_dir_list_files+2, tl_key_no_edit+tl_draw_par_line+tl_list_box_mode,\
    16,16, 0x8080ff,0x0000ff,0xffffff, 0,70,TE_PANEL_WIDTH-17,120, 0,0,0, el_focus,\
    ws_dir_lbox,0

;dir_list memory
tree_file_struct:
  dd 1
  dd 0,0,count_of_dir_list_files
  dd dir_mem
  db 0
  dd file_name ;sys_path

dir_mem rb 32+304*count_of_dir_list_files


menu_text_area:
db 'Файл',0
.1:
db 'Создать',0
db 'Открыть',0
db 'Сохранить',0
db 'Выход',0
.end:
db 0

;--------
msgbox_1:
  dw 0
  db 'Внимание',0 ;+2 = +MB_TEXT_OFFSET
  db 'Файл большой, потому открыт не весь.',13
  db '  1. При его сохранении будут потеряны "не открывшиеся" символы.',13
  db '  2. В документ нельзя добавлять текст, т. к. память занята.',0
  db 'Я понял(а)',0 ;button1
  db 0
msgbox_2:
  db 3,0
  db 'Внимание',0 ;+2 = +MB_TEXT_OFFSET
  db 'Сохранить изменения в файле?',0
  db 'Сохранить',0
  db 'Не сохранять',0
  db 'Отмена',0
  db 0
msgbox_2_funct:
  dd On_SaveAndNewFile
  dd On_NewFile
  dd 0

msgbox_3:
  db 1,0
  db 'Внимание',0 ;+2 = +MB_TEXT_OFFSET
  db 'Ошибка при открытии файла с цветовыми настройками!',13,\
     'Потому выделение слов цветом не возможно.',13,\
     '  (код ошибки='
  .err: db '?'
  db ')',0
  db 'Закрыть',0
  db 0

msgbox_4:
  db 1,0
  db 'Внимание',0 ;+2 = +MB_TEXT_OFFSET
  db 'Ошибка при открытии файла!',13,\
     'Возможно имя файла введено не правильно.',13,\
     '  (код ошибки='
  .err: db '?'
  db ')',0
  db 'Закрыть',0
  db 0

msgbox_5:
  db 3,0
  db 'Внимание',0 ;+2 = +MB_TEXT_OFFSET
  db 'Сохранить изменения в файле?',0
  db 'Не сохранять',0
  db 'Отмена',0
  db 0
msgbox_5_funct:
;  dd but_SaveFile
  dd On_OpenFile
  dd 0

msgbox_6:
  db 1,0
  db 'Внимание',0 ;+2 = +MB_TEXT_OFFSET
  db 'Ошибка при сохранении файла!',13,\
     'Возможно имя файла введено не правильно.',13,\
     '  (код ошибки='
  .err: db '?'
  db ')',0
  db 'Закрыть',0
  db 0
msgbox_7:
  db 1,0
  db 'Текст не найден',0 ;+2 = +MB_TEXT_OFFSET
  db 'Достигнут конец документа.',13,\
     'Попробуйте перейти в начало документа',13,\
     'или изменить запрос и повторить поиск.',0
  db 'Закрыть',0
  db 0
msgbox_8:
  db 3,0
  db 'Внимание',0 ;+2 = +MB_TEXT_OFFSET
  db 'Для выхода завершите работу с файлом.',13,\
     'Попробуйте сохранить изменения в файле',13,\
     'или начать новый пустой.',0
  db 'Отмена',0
  db 0

txtFindCapt db 'Поиск',0
txtFindNext db 'Найти далее',0
txtFormatCapt db 'Формат',0
txtFormatApply db 'Применить',0
;------
align 4
EvChar db 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

EvUpper db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79
    db 80,81,82,83,84,85,86,87,88,89,90,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
EvLover db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111
    db 112,113,114,115,116,117,118,119,120,121,122,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
conv_table dd 0


KM_SHIFT equ 0x00010000
KM_CTRL equ 0x00020000
KM_ALT equ 0x00040000
KM_NUMLOCK equ 0x00080000

; KEY CODES
KEY_F1 equ 0x0000003B
KEY_F2 equ 0x0000003C
KEY_F3 equ 0x0000003D

