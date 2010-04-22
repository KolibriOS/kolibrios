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
;version_ed     dd aVersion_ed

;check_box_draw	dd aCheck_box_draw
;check_box_mouse dd aCheck_box_mouse
;version_ch	dd aVersion_ch

;option_box_draw  dd aOption_box_draw
;option_box_mouse dd aOption_box_mouse
;version_op	 dd aVersion_op

scrollbar_ver_draw  dd aScrollbar_ver_draw
scrollbar_ver_mouse dd aScrollbar_ver_mouse
scrollbar_hor_draw  dd aScrollbar_hor_draw
scrollbar_hor_mouse dd aScrollbar_hor_mouse
;version_scrollbar   dd aVersion_scrollbar

;dinamic_button_draw  dd aDbutton_draw
;dinamic_button_mouse dd aDbutton_mouse
;version_dbutton      dd aVersion_dbutton

menu_bar_draw	 dd aMenu_bar_draw
menu_bar_mouse	 dd aMenu_bar_mouse
;version_menu_bar dd aVersion_menu_bar

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
;aVersion_ed	db 'version_ed',0

;aCheck_box_draw  db 'check_box_draw',0
;aCheck_box_mouse db 'check_box_mouse',0
;aVersion_ch	 db 'version_ch',0

;aOption_box_draw  db 'option_box_draw',0
;aOption_box_mouse db 'option_box_mouse',0
;aVersion_op	  db 'version_op',0

aScrollbar_ver_draw  db 'scrollbar_v_draw',0
aScrollbar_ver_mouse db 'scrollbar_v_mouse',0
aScrollbar_hor_draw  db 'scrollbar_h_draw',0
aScrollbar_hor_mouse db 'scrollbar_h_mouse',0
;aVersion_scrollbar   db 'version_scrollbar',0

;aDbutton_draw	  db 'dbutton_draw',0
;aDbutton_mouse	  db 'dbutton_mouse',0
;aVersion_dbutton  db 'version_dbutton',0

aMenu_bar_draw	  db 'menu_bar_draw',0
aMenu_bar_mouse   db 'menu_bar_mouse',0
;aVersion_menu_bar db 'version_menu_bar',0

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
.size_x   dw 16 ;+0
.start_x  dw 50 ;+2
.y:
.size_y   dw 300 ; +4
.start_y  dw  50 ; +6
.btn_high dd  15 ; +8
.type     dd   1 ;+12
.max_area dd 100 ;+16
.cur_area dd  30 ;+20
.position dd   0 ;+24
.bckg_col dd 0xeeeeee ;+28
.frnt_col dd 0xbbddff ;+32
.line_col dd   0 ;+36
.redraw   dd   0 ;+40
.delta    dw   0 ;+44
.delta2   dw   0 ;+46
.run_x:
.r_size_x  dw 0 ;+48
.r_start_x dw 0 ;+50
.run_y:
.r_size_y   dw 0 ;+52
.r_start_y  dw 0 ;+54
.m_pos      dd 0 ;+56
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
.ar_offset  dd 1 ;+84
;---------------------------------------------------------------------

align 4
menu_data_1:
.type: dd 0   ;+0
.x:
.size_x  dw 35 ;+4
.start_x dw  5 ;+6
.y:
.size_y  dw 15 ;+8
.start_y dw  5 ;+10
.text_pointer: dd menu_text_area   ;+12
.pos_pointer:  dd menu_text_area.1 ;+16
.text_end   dd menu_text_area.end  ;+20
.mouse_pos  dd 0 ;+24
.mouse_keys dd 0 ;+28
.x1:
.size_x1  dw 70  ;+32
.start_x1 dw  5  ;+34
.y1:
.size_y1  dw 15  ;+36
.start_y1 dw 20  ;+38
.bckg_col dd 0xeeeeee ;+40
.frnt_col dd 0xff ;+44
.menu_col dd 0xffffff ;+48
.select   dd 0 ;+52
.out_select dd 0 ;+56
.buf_adress dd 0 ;+60
.procinfo dd procinfo ;0 ;+64
.click   dd 0 ;+68
.cursor  dd 0 ;+72
.cursor_old dd 0 ;+76
.interval   dd 16 ;+80
.cursor_max  dd 0 ;+84
.extended_key dd 0 ;+88
.menu_sel_col dd 0x00cc00 ;+92
.bckg_text_col dd 0 ;+96
.frnt_text_col dd 0xffffff ;+100
.mouse_keys_old dd 0 ;+104
.font_height dd 8 ;+108
.cursor_out  dd 0 ;+112
.get_mouse_flag dd 0 ;+116
;-------------------------

TE_PANEL_NULL equ 0
TE_PANEL_FIND equ 1
TE_PANEL_SYNTAX equ 2
TE_PANEL_WIDTH equ 150 ;°шЁшэр ярэхыш

count_of_dir_list_files equ 15

;------------------------------------------------------------------------------
struct TexSelect
  x0 dd ?
  y0 dd ?
  x1 dd ?
  y1 dd ?
ends
;------------------------------------------------------------------------------
ted_wnd_l equ dword[edi+ 0] ;окно редактора: отступ слева
ted_wnd_t equ dword[edi+ 4] ;окно редактора: отступ сверху
ted_wnd_w equ dword[edi+ 8] ;окно редактора: ширина
ted_wnd_h equ dword[edi+12] ;окно редактора: высота
ted_rec_l equ dword[edi+16] ;ширина панели в окне
ted_rec_t equ dword[edi+20] ;высота панели в окне
ted_rec_w equ dword[edi+24] ;ширина символа (знакоместа) в окне
ted_rec_h equ dword[edi+28] ;высота символа (знакоместа) в окне
ted_drag_m equ byte[edi+32] ;выделение от мыши
ted_drag_k equ byte[edi+33] ;выделение от клавиатуры
ted_sel_x0 equ dword[edi+34] ;координата выделения x0
ted_sel_y0 equ dword[edi+38] ;координата выделения y0
ted_sel_x1 equ dword[edi+42] ;координата выделения x1
ted_sel_y1 equ dword[edi+46] ;координата выделения y1
;запасные координаты выделения
ted_seln_x0 equ dword[edi+50]
ted_seln_y0 equ dword[edi+54]
ted_seln_x1 equ dword[edi+58]
ted_seln_y1 equ dword[edi+62]

ted_tex     equ dword[edi+66] ;указатель на память для хранения текста
ted_tex_1   equ dword[edi+70] ;указатель на 1-ю стркутру символа
ted_tex_end equ dword[edi+74] ;указатель на последнюю структуру символа
ted_cur_x   equ dword[edi+78] ;координата x курсора
ted_cur_y   equ dword[edi+82] ;координата y курсора
ted_cur_ins equ  byte[edi+86] ;режим работы курсора (обычный или замена)
ted_mode_color equ byte[edi+87] ;режим выделения слов цветом (0-выкл. 1-вкл.)
ted_mode_invis equ byte[edi+88] ;режим показа непечатаемых символов
ted_gp_opt     equ byte[edi+89] ;опции возвращаемые функцией GetPos
ted_colors_text_count equ dword[edi+90] ;колличество цветов текста
ted_key_words_count   equ dword[edi+94] ;колличество ключевых слов
ted_color_cursor   equ dword[edi+98] ;цвет курсора
ted_color_wnd_capt equ dword[edi+102] ;цвет полей вокруг окна
ted_color_wnd_work equ dword[edi+106] ;цвет фона окна
ted_color_wnd_bord equ dword[edi+110] ;цвет текста на полях
ted_color_select   equ dword[edi+114] ;цвет выделения
ted_color_cur_text equ dword[edi+118] ;цвет символа под курсором
ted_color_wnd_text equ dword[edi+122] ;цвет текста в окне
ted_syntax_file    equ dword[edi+126] ;указатель на начало файла синтаксиса
ted_syntax_file_end equ dword[edi+130] ;указатель на конец файла синтаксиса
ted_text_colors     equ dword[edi+134] ;указатель на массив цветов текста
ted_help_text_f1    equ dword[edi+138] ;указатель на текст справки (по нажатии F1)
ted_help_id         equ dword[edi+142] ;идентификатор для справки
ted_key_words_data  equ dword[edi+146] ;указатель на структуры ключевых слов TexColViv
ted_tim_ch          equ dword[edi+150] ;колличество изменений в файле
ted_tim_undo        equ dword[edi+154] ;колличество отмененных действий
ted_tim_ls          equ dword[edi+158] ;время последнего сохранения
ted_tim_co          equ dword[edi+162] ;время последней цветовой разметки
ted_el_focus        equ dword[edi+166] ;указатель на переменную элемента в фокусе
ted_err_save        equ  word[edi+170] ;ошибка сохранения файла
ted_key_new         equ  byte[edi+172] ;символ, который будет добавлятся с клавиатуры
ted_symbol_new_line equ  byte[edi+173] ;символ завершения строки
ted_scr_w           equ dword[edi+174] ;вертикальный скроллинг
ted_scr_h           equ dword[edi+178] ;горизонтальный скроллинг
ted_panel_id        equ  byte[edi+182] ;номер открытой панели
;------------------------------------------------------------------------------
sb_offs_max_area equ 16
sb_offs_cur_area equ 20
sb_offs_position equ 24
sb_offs_redraw   equ 40
;------------------------------------------------------------------------------
ted_opt_ed_move_cursor equ 1 ;двигать курсор после добавления текста
ted_opt_ed_change_time equ 2 ;добавлять изменения при редактировании текста
ted_offs_sel  equ 34 ;смещение структуры sel
ted_offs_seln equ 50 ;смещение структуры seln
ted_offs_count_colors equ 90 ;смещение переменной колличества цветов
ted_offs_key_new equ 172
ted_offs_symbol_new_line equ 173 ;смещение символа новой строки
;------------------------------------------------------------------------------
align 4
tedit0: ;структура текстового редактора
	.wnd BOX 0,50,440,150 ;+ 0
	.rec BOX 30,23,7,10   ;+16
	.drag_m db 0 ;+32 выделение от мыши
	.drag_k db 0 ;+33 выделение от клавиатуры
	.sel  TexSelect 0,0,0,0 ;+34 структура выделения
	.seln TexSelect ;+50 дополнительная структура выделения
	.tex     dd 0 ;+66 text memory pointer
	.tex_1   dd 0 ;+70 text first symbol pointer
	.tex_end dd 0 ;+74 text end memory pointer
	.cur_x   dd 0 ;+78
	.cur_y   dd 0 ;+82
	.cur_ins db 1 ;+86
	.mode_color db 1 ;+87 режим выделения слов цветом (0-выкл. 1-вкл.)
	.mode_invis db 0 ;+88 режим показа непечатаемых символов
	.gp_opt     db 0 ;+89 опции возвращаемые функцией GetPos
; \/ не разрывать следующие переменные:
	.count_colors_text dd 1 ;+90 колличество цветов текста
	.count_key_words   dd 0 ;+94 колличество ключевых слов
	.color_cursor    dd 0x808080 ;+98 цвет курсора
	.color_wnd_capt  dd     0x80 ;+102 цвет полей вокруг окна
	.color_wnd_work  dd      0x0 ;+106 цвет фона окна
	.color_wnd_bord  dd 0xd0d0d0 ;+110 цвет текста на полях
	.color_select    dd 0x208080 ;+114 цвет выделения
	.color_cur_text  dd 0xff0000 ;+118 цвет символа под курсором
	.color_wnd_text  dd 0xffff00 ;+122 цвет текста в окне
; /\ другие, идущие ниже можно разрывать.
	.syntax_file     dd 0 ;+126 указатель на начало файла синтаксиса
	.syntax_file_end dd 0 ;+130 указатель на конец файла синтаксиса
	.text_colors     dd 0 ;+134 указатель на массив цветов текста
	.help_text_f1    dd 0 ;+138 указатель на текст справки (по нажатии F1)
	.help_id         dd -1 ;+142 идентификатор для справки
	.key_words_data  dd 0 ;+146 указатель на структуры ключевых слов TexColViv
	.tim_ch   dd ? ;+150 количество изменений в файле
	.tim_undo dd ? ;+154 количество отмененных действий
	.tim_ls   dd ? ;+158 время последнего сохранения
	.tim_co   dd ? ;+162 время последней цветовой разметки
	.el_focus dd el_focus ;+166 указатель на переменную элемента в фокусе
	.err_save    dw 0 ;+170 ошибка сохранения файла
	.key_new     db 0 ;+172 символ, который будет добавлятся с клавиатуры
	.symbol_new_line db 20 ;+173 символ завершения строки
	.scr_w dd wScr ;+174 вертикальный скроллинг
	.scr_h dd hScr ;+178 горизонтальный скроллинг
	.panel_id db 0 ;+182 номер открытой панели
;------------------------------------------------------------------------------

FkPos: rd 256
conv_tabl rb 128 ; таблица для конвертирования scan-кода в ascii-код


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


if lang eq ru

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
     '  (код ошибки ='
  .err: db '?'
  db ')',0
  db 'Закрыть',0
  db 0

msgbox_4:
  db 1,0
  db 'Внимание',0 ;+2 = +MB_TEXT_OFFSET
  db 'Ошибка при открытии файла!',13,\
     'Возможно имя файла введено не правильно.',13,\
     '  (код ошибки ='
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
  dd but_no_msg_OpenFile
  dd 0

msgbox_6:
  db 1,0
  db 'Внимание',0 ;+2 = +MB_TEXT_OFFSET
  db 'Ошибка при сохранении файла!',13,\
     'Возможно имя файла введено не правильно.',13,\
     '  (код ошибки ='
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
txtRow db 'Строка',0
txtCol db 'Знак',0
txtOtm db 'Отмены',0
txtBuf db 'Буфер: '
buf db BUF_SIZE dup(0) ;buf тёхуфр яюёых txtBuf

else

menu_text_area:
db 'File',0
.1:
db 'New',0
db 'Open',0
db 'Save',0
db 'Exit',0
.end:
db 0

;--------
msgbox_1:
  dw 0
  db 'Warning',0 ;+2 = +MB_TEXT_OFFSET
  db 'File is large, because not all open.',13
  db '  1. If you save it will lose "not opened" characters.',13
  db '  2. The document can not add text, because the memory is busy.',0
  db 'Ok',0 ;button1
  db 0
msgbox_2:
  db 3,0
  db 'Warning',0 ;+2 = +MB_TEXT_OFFSET
  db 'Save changes to the file?',0
  db 'Save',0
  db 'Discard',0
  db 'Cancel',0
  db 0
msgbox_2_funct:
  dd On_SaveAndNewFile
  dd On_NewFile
  dd 0

msgbox_3:
  db 1,0
  db 'Warning',0 ;+2 = +MB_TEXT_OFFSET
  db 'Error opening the file with the color settings!',13,\
     'Because color selection of words is not possible.',13,\
     '  (error code ='
  .err: db '?'
  db ')',0
  db 'Close',0
  db 0

msgbox_4:
  db 1,0
  db 'Warning',0 ;+2 = +MB_TEXT_OFFSET
  db 'Error opening file!',13,\
     'Maybe the file name is not entered correctly.',13,\
     '  (error code ='
  .err: db '?'
  db ')',0
  db 'Close',0
  db 0

msgbox_5:
  db 3,0
  db 'Warning',0 ;+2 = +MB_TEXT_OFFSET
  db 'Save changes to the file?',0
  db 'Discard',0
  db 'Cancel',0
  db 0
msgbox_5_funct:
  dd but_no_msg_OpenFile
  dd 0

msgbox_6:
  db 1,0
  db 'Warning',0 ;+2 = +MB_TEXT_OFFSET
  db 'Error saving file!',13,\
     'Maybe the file name is not entered correctly.',13,\
     '  (error code ='
  .err: db '?'
  db ')',0
  db 'Close',0
  db 0
msgbox_7:
  db 1,0
  db 'Text not found.',0 ;+2 = +MB_TEXT_OFFSET
  db 'Reached end of the document.',13,\
     'Try to go to the beginning of the document',13,\
     'or change the query and search again.',0
  db 'Close',0
  db 0
msgbox_8:
  db 3,0
  db 'Warning',0 ;+2 = +MB_TEXT_OFFSET
  db 'To quit, quit the file.',13,\
     'Try to keep changes to a file',13,\
     'or start a new blank file.',0
  db 'Cancel',0
  db 0

txtFindCapt db 'Search',0
txtFindNext db 'Find next',0
txtFormatCapt db 'Format',0
txtFormatApply db 'Apply',0
txtRow db 'Rows',0
txtCol db 'Cols',0
txtOtm db 'Undo',0
txtBuf db 'Buffer: '
buf db BUF_SIZE dup(0) ;buf always after txtBuf

end if
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

