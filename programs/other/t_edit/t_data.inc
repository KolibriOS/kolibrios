fn_col_option dd def_col_file
fn_syntax_dir db 'info/',0
def_col_file db 'default.syn',0

exit_code dd 0 ;��६����� �ᯮ��㥬�� ��� ��室� �� �ணࠬ��
panel_but rb 21 ;����ன�� �⮡ࠦ���� ������ �� ������

;���祭�� ��������� �� 㬮�砭��, ��� ini 䠩��
ini_def_window_t equ 20
ini_def_window_l equ 200
ini_def_window_w equ 570
ini_def_window_h equ 470
ini_def_symbol_w equ 7
ini_def_symbol_h equ 10
ini_def_font_s equ 0
ini_def_scroll_type equ 0
;���ᠭ�� ��ࠬ��஢ ��� ini 䠩��
ini_name db 't_edit.ini',0
ini_sec_window db 'Window',0
key_window_t db 't',0
key_window_l db 'l',0
key_window_w db 'w',0
key_window_h db 'h',0
key_symbol_w db 'symbol_w',0
key_symbol_h db 'symbol_h',0
key_font_s db 'font_s',0
key_scroll_type db 'scroll_type',0
key_but_new db 'but_new',0
key_but_open db 'but_open',0
key_but_save db 'but_save',0
key_but_save_as db 'but_save_as',0
key_but_select db 'but_select',0
key_but_cut db 'but_cut',0
key_but_copy db 'but_copy',0
key_but_paste db 'but_paste',0
key_but_find db 'but_find',0
key_but_replace db 'but_replace',0
key_but_key_words db 'but_key_words',0
key_but_upper db 'but_upper',0
key_but_lower db 'but_lower',0
key_but_reverse db 'but_reverse',0
key_but_undo db 'but_undo',0
key_but_redo db 'but_redo',0
key_but_invisible db 'but_invisible',0
key_but_syntax_list db 'but_syntax_list',0
key_but_syntax_mode db 'but_syntax_mode',0
key_but_convert_1251_866 db 'but_convert_1251_866',0
key_but_convert_866_1251 db 'but_convert_866_1251',0
ini_sec_options db 'Options',0
key_synt_file:
	db 'syntax_'
	.numb: db '0',0
key_synt_ext:
	db 'ext_'
	.numb: db '0',0
ini_def_synt_f db 0

align 4
max_synt_auto_open equ 10
synt_auto_open:
	rb 64*max_synt_auto_open
	db 0

system_dir_0 db '/sys/lib/'
lib_name_0 db 'box_lib.obj',0

system_dir_1 db '/sys/lib/'
lib_name_1 db 'msgbox.obj',0

system_dir_2 db '/sys/lib/'
lib_name_2 db 'proc_lib.obj',0

system_dir_3 db '/sys/lib/'
lib_name_3 db 'libimg.obj',0

system_dir_4 db '/sys/lib/'
lib_name_4 db 'libini.obj',0

system_dir_5 db '/sys/lib/'
lib_name_5 db 'kmenu.obj',0


;---------------------------------------------------------------------
align 4
import_box_lib:
	dd alib_init0 ;�㭪�� ����᪠���� ����ᮬ 1 ࠧ �� ������祭�� �������⥪�, ��⮬� � �ணࠬ�� ��⪠ �� ��� �� �㦭�

	edit_box_draw  dd aEdit_box_draw
	edit_box_key   dd aEdit_box_key
	edit_box_mouse dd aEdit_box_mouse
	;edit_box_set_text dd aEdit_box_set_text
	;version_ed     dd aVersion_ed

	init_checkbox	dd ainit_checkbox
	check_box_draw	dd acheck_box_draw
	check_box_mouse	dd acheck_box_mouse

	option_box_draw	 dd aOption_box_draw
	option_box_mouse dd aOption_box_mouse
	;version_op      dd aVersion_op

	scrollbar_ver_draw  dd aScrollbar_ver_draw
	scrollbar_ver_mouse dd aScrollbar_ver_mouse
	scrollbar_hor_draw  dd aScrollbar_hor_draw
	scrollbar_hor_mouse dd aScrollbar_hor_mouse
	;version_scrollbar   dd aVersion_scrollbar

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
	;tl_node_close_open dd sz_tl_node_close_open
	tl_node_lev_inc dd sz_tl_node_lev_inc
	tl_node_lev_dec dd sz_tl_node_lev_dec

	ted_but_sumb_upper dd sz_ted_but_sumb_upper
	ted_but_sumb_lover dd sz_ted_but_sumb_lover
	ted_but_convert_by_table dd sz_ted_but_convert_by_table
	ted_can_save dd sz_ted_can_save
	ted_clear dd sz_ted_clear
	ted_delete dd sz_ted_delete
	ted_draw dd sz_ted_draw
	ted_init dd sz_ted_init
	ted_init_scroll_bars dd sz_ted_init_scroll_bars
	ted_init_syntax_file dd sz_ted_init_syntax_file
	ted_is_select dd sz_ted_is_select
	ted_key dd sz_ted_key
	ted_mouse dd sz_ted_mouse
	ted_open_file dd sz_ted_open_file
	ted_save_file dd sz_ted_save_file
	ted_text_add dd sz_ted_text_add
	ted_but_select_word dd sz_ted_but_select_word
	ted_but_cut dd sz_ted_but_cut
	ted_but_copy dd sz_ted_but_copy
	ted_but_paste dd sz_ted_but_paste
	ted_but_undo dd sz_ted_but_undo
	ted_but_redo dd sz_ted_but_redo
	ted_but_reverse dd sz_ted_but_reverse
	ted_but_find dd sz_ted_but_find
	ted_but_replace dd sz_ted_but_replace
	ted_text_colored dd sz_ted_text_colored
	ted_go_to_position dd sz_ted_go_to_position
	version_text_edit dd sz_ted_version

dd 0,0
	alib_init0 db 'lib_init',0

	aEdit_box_draw	db 'edit_box_draw',0
	aEdit_box_key	db 'edit_box_key',0
	aEdit_box_mouse db 'edit_box_mouse',0
	;aEdit_box_set_text db 'edit_box_set_text',0
	;aVersion_ed    db 'version_ed',0

	ainit_checkbox		db 'init_checkbox2',0
	acheck_box_draw		db 'check_box_draw2',0
	acheck_box_mouse	db 'check_box_mouse2',0

	aOption_box_draw	db 'option_box_draw',0
	aOption_box_mouse	db 'option_box_mouse',0
	;aVersion_op      db 'version_op',0

	aScrollbar_ver_draw  db 'scrollbar_v_draw',0
	aScrollbar_ver_mouse db 'scrollbar_v_mouse',0
	aScrollbar_hor_draw  db 'scrollbar_h_draw',0
	aScrollbar_hor_mouse db 'scrollbar_h_mouse',0
	;aVersion_scrollbar   db 'version_scrollbar',0

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
	;sz_tl_node_close_open db 'tl_node_close_open',0
	sz_tl_node_lev_inc db 'tl_node_lev_inc',0
	sz_tl_node_lev_dec db 'tl_node_lev_dec',0

	sz_ted_but_sumb_upper	db 'ted_but_sumb_upper',0
	sz_ted_but_sumb_lover	db 'ted_but_sumb_lover',0
	sz_ted_but_convert_by_table db 'ted_but_convert_by_table',0
	sz_ted_can_save 		db 'ted_can_save',0
	sz_ted_clear			db 'ted_clear',0
	sz_ted_delete			db 'ted_delete',0
	sz_ted_draw				db 'ted_draw',0
	sz_ted_init				db 'ted_init',0
	sz_ted_init_scroll_bars db 'ted_init_scroll_bars',0
	sz_ted_init_syntax_file db 'ted_init_syntax_file',0
	sz_ted_is_select		db 'ted_is_select',0
	sz_ted_key				db 'ted_key',0
	sz_ted_mouse			db 'ted_mouse',0
	sz_ted_open_file		db 'ted_open_file',0
	sz_ted_save_file		db 'ted_save_file',0
	sz_ted_text_add 		db 'ted_text_add',0
	sz_ted_but_select_word	db 'ted_but_select_word',0
	sz_ted_but_cut			db 'ted_but_cut',0
	sz_ted_but_copy 		db 'ted_but_copy',0
	sz_ted_but_paste		db 'ted_but_paste',0
	sz_ted_but_undo 		db 'ted_but_undo',0
	sz_ted_but_redo 		db 'ted_but_redo',0
	sz_ted_but_reverse		db 'ted_but_reverse',0
	sz_ted_but_find			db 'ted_but_find',0
	sz_ted_but_replace		db 'ted_but_replace',0
	sz_ted_text_colored		db 'ted_text_colored',0
	sz_ted_go_to_position	db 'ted_go_to_position',0
	sz_ted_version db 'version_text_edit',0

align 4
import_proclib:
	OpenDialog_Init dd aOpenDialog_Init
	OpenDialog_Start dd aOpenDialog_Start
dd 0,0
	aOpenDialog_Init db 'OpenDialog_init',0
	aOpenDialog_Start db 'OpenDialog_start',0

align 4
import_msgbox_lib:
	mb_create dd amb_create
	mb_reinit dd amb_reinit
	mb_setfunctions dd amb_setfunctions
dd 0,0
	amb_create db 'mb_create',0
	amb_reinit db 'mb_reinit',0
	amb_setfunctions db 'mb_setfunctions',0

align 4
import_libimg:
	dd alib_init1
	img_to_rgb2 dd aimg_to_rgb2
	img_decode  dd aimg_decode
	img_destroy dd aimg_destroy
dd 0,0
	alib_init1   db 'lib_init',0
	aimg_to_rgb2 db 'img_to_rgb2',0
	aimg_decode  db 'img_decode',0 ;��⮬���᪨ ��।���� �ଠ� ����᪨� ������
	aimg_destroy db 'img_destroy',0

align 4
import_libini:
	dd alib_init2
	ini_get_str   dd aini_get_str
	ini_get_int   dd aini_get_int
	ini_get_color dd aini_get_color
dd 0,0
	alib_init2     db 'lib_init',0
	aini_get_str   db 'ini_get_str',0
	aini_get_int   db 'ini_get_int',0
	aini_get_color db 'ini_get_color',0

align 4
import_libkmenu:
	kmenu_init                     dd akmenu_init
	kmainmenu_draw                 dd akmainmenu_draw
	kmainmenu_dispatch_cursorevent dd akmainmenu_dispatch_cursorevent
	ksubmenu_new                   dd aksubmenu_new
	ksubmenu_delete                dd aksubmenu_delete
	ksubmenu_draw                  dd aksubmenu_draw
	ksubmenu_add                   dd aksubmenu_add
	kmenuitem_new                  dd akmenuitem_new
	kmenuitem_delete               dd akmenuitem_delete
	kmenuitem_draw                 dd akmenuitem_draw
dd 0,0
	akmenu_init                     db 'kmenu_init',0
	akmainmenu_draw                 db 'kmainmenu_draw',0
	akmainmenu_dispatch_cursorevent db 'kmainmenu_dispatch_cursorevent',0
	aksubmenu_new                   db 'ksubmenu_new',0
	aksubmenu_delete                db 'ksubmenu_delete',0
	aksubmenu_draw                  db 'ksubmenu_draw',0
	aksubmenu_add                   db 'ksubmenu_add',0
	akmenuitem_new                  db 'kmenuitem_new',0
	akmenuitem_delete               db 'kmenuitem_delete',0
	akmenuitem_draw                 db 'kmenuitem_draw',0

;---------------------------------------------------------------------
align 4
wScr scrollbar 16,50, 300,50, 15, 100,30,0, 0,0,0, 1

align 4
hScr scrollbar 150,0,  16,50, 15, 100,30,0, 0,0,0, 1

;���୨� �஫���� ��� ������� ListBox
align 4
ws_dir_lbox scrollbar 16,0, 30,70, 15, 100,30,0, 0,0,0, 1

;---------------------------------------------------------------------
align 4
OpenDialog_data:
.type			dd 0
.procinfo		dd procinfo	;+4
.com_area_name	dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_path		dd plugin_path ;+16
.dir_default_path	dd default_dir ;+20
.start_path		dd file_name ;+24 ���� � ������� ������ 䠩���
.draw_window	dd draw_window	;+28
.status 		dd 0	;+32
.openfile_path	dd openfile_path	;+36 ���� � ���뢠����� 䠩��
.filename_area	dd filename_area	;+40 ��� ���뢠����� 䠩��
.filter_area	dd Filter
.x:
.x_size 		dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size 		dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

default_dir db '/sys',0 ;��४��� �� 㬮�砭��

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_name:
	db 'opendial',0
communication_area_default_path:
	db '/sys/File managers/',0

Filter:
dd Filter.end - Filter.1
.1:
db 'ASM',0
db 'CPP',0
db 'INC',0
db 'MAC',0
db 'INI',0
db 'TXT',0
db 'H',0
db 'C',0
db 'HTM',0
db 'HTML',0
db 'PY',0
db 'SH',0
db 'BAT',0
db 'LUA',0
db 'INF',0
db 'LOG',0
db 'DBG',0
db 'DAT',0
.end:
db 0

txt_Info db '" -tI',0

TED_PANEL_NULL    equ 0 ;��� ����⮩ ������
TED_PANEL_FIND    equ 1 ;������ ���᪠
TED_PANEL_REPLACE equ 2 ;������ ������
TED_PANEL_SYNTAX  equ 3 ;������ �롮� 䠩��� ���ᢥ⪨
TED_PANEL_GOTO    equ 4 ;������ ���室�
TED_PANEL_WIDTH  equ 150 ;�ਭ� ������

count_of_dir_list_files equ 15 ;���ᨬ��쭮� �᫮ 䠩��� ᨭ⠪�� �����뢠���� � ���� ������� tree1

;------------------------------------------------------------------------------
struct TexSelect
  x0 dd ?
  y0 dd ?
  x1 dd ?
  y1 dd ?
ends
;------------------------------------------------------------------------------
align 4
tedit0: ;������� ⥪�⮢��� ।����
	.wnd BOX 0,50,440,150 ;+ 0
	.rec BOX 30,25,ini_def_symbol_w,ini_def_symbol_h ;+16
	.drag_m db 0 ;+32 �뤥����� �� ���
	.drag_k db 0 ;+33 �뤥����� �� ����������
	.sel  TexSelect 0,0,0,0 ;+34 ������� �뤥�����
	.seln TexSelect ;+50 �������⥫쭠� ������� �뤥�����
	.tex	  dd 0 ;+66 text memory pointer
	.tex_1	  dd 0 ;+70 text first symbol pointer
	.tex_end  dd 0 ;+74 text end memory pointer
	.cur_x	  dd 0 ;+78 ���न��� x �����
	.cur_y	  dd 0 ;+82 ���न��� y �����
	.max_chars dd maxChars ;+86 ���ᨬ��쭮� �᫮ ᨬ����� � ����� ���㬥��
	.count_colors_text dd 1 ;+90 �������⢮ 梥⮢ ⥪��
	.count_key_words   dd 0 ;+94 �������⢮ ���祢�� ᫮�
	.color_cursor	   dd 0x808080 ;+98 梥� �����
	.color_wnd_capt    dd	  0x80 ;+102 梥� ����� ����� ����
	.color_wnd_work    dd	   0x0 ;+106 梥� 䮭� ����
	.color_wnd_bord    dd 0xd0d0d0 ;+110 梥� ⥪�� �� �����
	.color_select	   dd 0x208080 ;+114 梥� �뤥�����
	.color_cur_text    dd 0xff0000 ;+118 梥� ᨬ���� ��� ����஬
	.color_wnd_text    dd 0xffff00 ;+122 梥� ⥪�� � ����
	.syntax_file	   dd 0 ;+126 㪠��⥫� �� ��砫� 䠩�� ᨭ⠪��
	.syntax_file_size  dd maxSyntaxFileSize ;+130 ���ᨬ���� ࠧ��� 䠩�� ᨭ⠪��
	.text_colors	   dd 0 ;+134 㪠��⥫� �� ���ᨢ 梥⮢ ⥪��
	.help_text_f1	   dd 0 ;+138 㪠��⥫� �� ⥪�� �ࠢ�� (�� ����⨨ F1)
	.help_id	   dd -1 ;+142 �����䨪��� ��� �ࠢ��
	.key_words_data    dd 0 ;+146 㪠��⥫� �� �������� ���祢�� ᫮� TexColViv
	.tim_ch      dd ? ;+150 ������⢮ ��������� � 䠩��
	.tim_undo    dd ? ;+154 ������⢮ �⬥������ ����⢨�
	.tim_ls      dd ? ;+158 �६� ��᫥����� ��࠭����
	.tim_co      dd ? ;+162 �६� ��᫥���� 梥⮢�� ࠧ��⪨
	.el_focus    dd el_focus ;+166 㪠��⥫� �� ��६����� ������� � 䮪��
	.err_save    db 0 ;+170 �訡�� ��࠭���� 䠩��
	.panel_id    db 0 ;+171 ����� ����⮩ ������
	.key_new     db 0 ;+172 ᨬ���, ����� �㤥� ���������� � ����������
	.symbol_new_line db 20 ;+173 ᨬ��� �����襭�� ��ப�
	.scr_w	     dd wScr ;+174 ���⨪���� �஫����
	.scr_h	     dd hScr ;+178 ��ਧ��⠫�� �஫����
	.arr_key_pos dd 0 ;+182 㪠��⥫� �� ���ᨢ ����権 ���祢�� ᫮�
	.buffer      dd buf ;+186 㪠��⥫� �� ���� ����஢����/��⠢��
	.buffer_find dd buf_find ;+190 㪠��⥫� �� ���� ��� ���᪠
	.cur_ins     db 1 ;+194 ०�� ࠡ��� ����� (����� ��� ������)
	.mode_color  db 1 ;+195 ०�� �뤥����� ᫮� 梥⮬ (0-�몫. 1-���.)
	.mode_invis  db 0 ;+196 ०�� ������ �����⠥��� ᨬ�����
	.gp_opt      db 0 ;+197 ��樨 �����頥�� �㭪樥� ted_get_pos_by_cursor
	.fun_on_key_ctrl_all dd ted_but_ctrl_all ;+198 㪠��⥫� �� �㭪�� ��뢠���� �� ����⨨ Ctrl+N,O,S,F,H,G
	dd 0,0,0 ;��१�ࢨ஢���
	.buffer_size	   dd BUF_SIZE ;+214 ࠧ��� ���� ����஢����/��⠢��
	.fun_find_err	   dd ted_on_find_err ;+218 㪠��⥫� �� �㭪�� ��뢠���� �᫨ ���� �����稫�� ��㤠筮
	.fun_init_synt_err dd 0   ;+222
	.fun_draw_panel_buttons dd draw_but_toolbar  ;+226 㪠��⥫� �� �㭪�� �ᮢ���� ������ � ��������
	.fun_draw_panels	dd draw_panels   ;+230 㪠��⥫� �� �㭪�� �ᮢ���� ������ ���᪠/������
	dd 0 ;+234 ��१�ࢨ஢���
	.fun_save_err		dd ted_save_err_msg  ;+238 㪠��⥫� �� �㭪�� ��뢠���� �᫨ ��࠭���� 䠩�� �����稫��� ��㤠筮
	.increase_size dd 200 ;+242 �᫮ ᨬ����� �� ����� �㤥� 㢥稢����� ������ �� ��墠⪥
	.ptr_free_symb dd ?   ;+246 �ᯮ������ ����� ������� ��� �᪮७�� ��⠢�� ⥪��
	.font_size dd 0   ;+250 �����⥫� ��� ࠧ��� ����
;------------------------------------------------------------------------------

conv_tabl rb 128 ; ⠡��� ��� �������஢���� scan-���� � ascii-���

el_focus dd tedit0
mouse_dd dd 0
tree1 tree_list 264,count_of_dir_list_files+2, tl_key_no_edit+tl_draw_par_line+tl_list_box_mode,\
    16,16, 0x8080ff,0x0000ff,0xffffff, 0,70,TED_PANEL_WIDTH-17,120, 0,0,0, el_focus,\
    ws_dir_lbox,0

;dir_list memory
tree_file_struct:
	dd 1,0,0,count_of_dir_list_files
	dd dir_mem
	db 0
.f: dd file_name

align 4
opt_gr1 dd opt1
opt_grlist1 dd opt1,opt2,opt3,0 ;end option group

opt1 option_box opt_gr1, 5, 90,6,12,0xd0d0ff, 0xff, 0x80ff,txt_f_opt_next,txt_f_opt_next.end-txt_f_opt_next
opt2 option_box opt_gr1, 5,108,6,12,0xd0d0ff, 0xff, 0x80ff,txt_f_opt_perv,txt_f_opt_perv.end-txt_f_opt_perv
opt3 option_box opt_gr1, 5,126,6,12,0xd0d0ff, 0xff, 0x80ff,txt_f_opt_beg,txt_f_opt_beg.end-txt_f_opt_beg

check_boxes:
ch1 check_box2 (5 shl 16+12),(144 shl 16 +12),5,0xffffff,0,0x80ff,ch_text.1,ch_flag_en+ch_flag_middle
check_boxes_end:



if lang eq ru_RU

msgbox_1:
  db '"������',13,10,'������� �����: '
.z: rb 10
msgbox_2:
  db 3,0
  db '��������',0
  db '���࠭��� ��������� � 䠩��?',0
  db '���࠭���',0
  db '�� ��࠭���',0
  db '�⬥��',0
  db 0
msgbox_3:
  db 1,0
  db '��������',0
  db '�訡�� �� ����⨨ 䠩�� � 梥⮢묨 ����ன����!',13,\
     '��⮬� �뤥����� ᫮� 梥⮬ �� ��������.',13,\
     '  (��� �訡�� ='
  .err: db '?'
  db ')',0
  db '�������',0
  db 0
msgbox_4:
  db 1,0
  db '��������',0
  db '�訡�� �� ����⨨ 䠩��!',13,\
     '�������� ��� 䠩�� ������� �� �ࠢ��쭮.',13,\
     '  (��� �訡�� ='
  .err: db '?'
  db ')',0
  db '�������',0
  db 0
msgbox_6:
  db 1,0
  db '��������',0
  db '�訡�� �� ��࠭���� 䠩��!',13,\
     '�������� ��� 䠩�� ������� �� �ࠢ��쭮.',13,\
     '  (��� �訡�� ='
  .err: db '?'
  db ')',0
  db '�������',0
  db 0
msgbox_7:
  db '"����� �� ������',13,10
  db '���஡�� ��� �������� ����� � ������� ����." -tI',0
msgbox_9:
  db 3,0
  db '��������',0
  db '�� ������ ',39,'������ ������ 䠩��',39,'.',13
  .fdp:
  rb 100
  db 0
  db '�������',0
  db 0

txtFindCapt db '����:',0
txtFindNext db '���� �����',0
txtRerlaceCapt db '��� ��������:',0
txtRerlace db '��������',0
txtRerlaceAll db '�������� ��',0
txtFormatCapt db '��ଠ�',0
txtFormatApply db '�ਬ�����',0
txtReplace db '��������',0
txtReplaceAll db '�������� ��',0
txtGotoCapt db '���室 �� ��ப�:',0
txtGoto db '��३�',0
txt_f_opt_next: db '����'
.end: db 0
txt_f_opt_perv: db '�����'
.end: db 0
txt_f_opt_beg: db '� ��砫�'
.end: db 0
ch_text:
.1 db '���뢠�� ॣ����',0

else ; Default to en_US

msgbox_1:
  db '"Replace',13,10,'Made replacements: '
.z: rb 10
msgbox_2:
  db 3,0
  db 'Warning',0
  db 'Save changes to the file?',0
  db 'Save',0
  db 'Discard',0
  db 'Cancel',0
  db 0
msgbox_3:
  db 1,0
  db 'Warning',0
  db 'Error opening the file with the color settings!',13,\
     'Because color selection of words is not possible.',13,\
     '  (error code ='
  .err: db '?'
  db ')',0
  db 'Close',0
  db 0
msgbox_4:
  db 1,0
  db 'Warning',0
  db 'Error opening file!',13,\
     'Maybe the file name is not entered correctly.',13,\
     '  (error code ='
  .err: db '?'
  db ')',0
  db 'Close',0
  db 0
msgbox_6:
  db 1,0
  db 'Warning',0
  db 'Error saving file!',13,\
     'Maybe the file name is not entered correctly.',13,\
     '  (error code ='
  .err: db '?'
  db ')',0
  db 'Close',0
  db 0
msgbox_7:
  db '"Text not found.',13,10
  db 'Try change the query and search again." -tI',0
msgbox_9:
  db 3,0
  db 'Warning',0
  db 'Error run ',39,'open file dialog',39,'.',13
  .fdp:
  rb 100
  db 0
  db 'Close',0
  db 0

txtFindCapt db 'Search:',0
txtFindNext db 'Find next',0
txtRerlaceCapt db 'Replace with:',0
txtRerlace db 'Replace',0
txtRerlaceAll db 'Replace all',0
txtFormatCapt db 'Format',0
txtFormatApply db 'Apply',0
txtReplace db 'Replace',0
txtReplaceAll db 'Replace all',0
txtGotoCapt db 'Go to line:',0
txtGoto db 'Go to',0
txt_f_opt_next: db 'Down'
.end: db 0
txt_f_opt_perv: db 'Up'
.end: db 0
txt_f_opt_beg: db 'At first'
.end: db 0
ch_text:
.1 db 'Case sensitive',0

end if

;�㭪樨 ��� ������ � ᮮ�饭���
msgbox_2N_funct:
	dd On_SaveAndNewFile
	dd On_NewFile
	dd 0
msgbox_2O_funct:
	dd On_SaveAndOpenFile
	dd ted_but_open_file.no_msg
	dd 0
msgbox_2E_funct:
	dd On_SaveAndExit
	dd On_Exit
	dd 0

KM_SHIFT equ 0x00010000
KM_CTRL equ 0x00020000
KM_ALT equ 0x00040000
KM_NUMLOCK equ 0x00080000

; KEY CODES
KEY_F1 equ 0x0000003B
KEY_F2 equ 0x0000003C
KEY_F3 equ 0x0000003D
