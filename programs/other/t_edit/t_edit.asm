;Огромная благодарность Maxxxx32, Diamond, Heavyiron
;и другим программистам, а также
;Теплову Алексею (<Lrz> www.lrz.land.ru)


use32
  org 0x0
  db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
  dd 0x01
  dd start
  dd i_end ; размер приложения
  dd mem
  dd stacktop
  dd file_name ; command line
  dd sys_path

MAX_COLOR_WORD_LEN equ 40
maxChars equ 100002 ;(...+2)
BUF_SIZE equ 1000 ;buffer for copy|paste
maxSyntaxFileSize equ 410000

include '../../proc32.inc'
include '../../macros.inc'
include 'mem.inc'
include '../../develop/libraries/box_lib/load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include 'lang.inc'

include 't_data.inc'
include 'strlen.inc'
include 't_draw.inc' ;draw main window functions
include 't_button.inc' ;text work functions

@use_library_mem mem.Alloc,mem.Free,mem.ReAlloc,0

align 4
start:
  mcall 48,3,sc,sizeof.system_colors

  mcall 68,11
  or eax,eax
  jz button.exit

  mcall 66,1,1 ;scan code
  ;mcall 26,2,1,conv_tabl
  mcall 40,0x27

  mov esi,file_name
  call strlen
  mov ecx,eax
  mov edi,openfile_path
  rep movsb ;копируем имя файла в буфер edit1

load_libraries l_libs_start,load_lib_end

;проверка на сколько удачно загузились библиотеки
	mov	ebp,lib0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
	mcall -1 ;exit not correct
@@:
	mov	ebp,lib1 ;
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
	mcall -1 ;exit not correct
@@:

	cmp dword[version_text_edit],3
	jge @f
		stdcall [mb_create],msgbox_10,thread
		mcall -1
	@@:

;---------------------------------------------------------------------
	stdcall [ted_init], tedit0
	stdcall dword[tl_data_init], tree1

; OpenDialog initialisation
	stdcall [OpenDialog_Init],OpenDialog_data

; init bmp file
  mov ecx,1200*18
  stdcall mem.Alloc,ecx
  mov [bmp_icon],eax

  copy_path fn_icon,sys_path,file_name,0x0

  mov eax,70 ;load icon file
  mov [run_file_70.Function], 0
  mov [run_file_70.Position], 54
  mov [run_file_70.Flags], 0
  mov [run_file_70.Count], 1200*18
  m2m [run_file_70.Buffer], [bmp_icon]
  mov byte[run_file_70+20], 0
  mov [run_file_70.FileName], file_name
  mov ebx,run_file_70
  int 0x40

  cmp ebx,-1
  mov [err_ini0],1
  je @f ;if open file
    mov [err_ini0],0
  @@:

;---------------------------------------------------------------------
; читаем bmp файл с курсорами и линиями
  copy_path fn_icon_tl_sys,sys_path,file_name,0x0

  mov ecx,3*256*13
  stdcall mem.Alloc,ecx
  mov dword[tree1.data_img_sys],eax

  ;mov [run_file_70.Function], 0
  ;mov [run_file_70.Position], 54
  ;mov [run_file_70.Flags], 0
  mov [run_file_70.Count], 3*256*13
  mov [run_file_70.Buffer], eax
  ;mov byte[run_file_70+20], 0
  ;mov [run_file_70.FileName], file_name

  mov eax,70 ;load icon file
  mov ebx,run_file_70
  int 0x40
  cmp ebx,0
  jg @f
    mov dword[tree1.data_img_sys],0
  @@:
;---------------------------------------------------------------------
; читаем bmp файл с иконками узлов
  copy_path fn_icon_tl_nod,sys_path,file_name,0x0

  mov ecx,3*256*2
  stdcall mem.Alloc,ecx
  mov dword[tree1.data_img],eax

;  mov [run_file_70.Function], 0
;  mov [run_file_70.Position], 54
;  mov [run_file_70.Flags], 0
  mov [run_file_70.Count], 3*256*2
  mov [run_file_70.Buffer], eax
;  mov byte[run_file_70+20], 0
;  mov [run_file_70.FileName], file_name

  mov eax,70 ;load icon file
  mov ebx,run_file_70
  int 0x40
  cmp ebx,0
  jg @f
    mov dword[tree1.data_img],0
  @@:
;------------------------------------------------------------------------------
  copy_path fn_syntax_dir,sys_path,file_name,0x0 ;берем путь к папке с файлами синтаксиса
  mov eax,70
  mov ebx,tree_file_struct
  int 0x40

cmp ebx,-1
je .end_dir_init

  mov eax,dir_mem
  add eax,32+4+1+3+4*6+8
mov ecx,ebx
@@:
  cmp byte[eax],'.' ;фильтруем файлы с именами '.' и '..'
  je .filter
    ;0x10000 ;1*2^16 - где 1 номер иконки с книгой
    stdcall dword[tl_node_add], eax,0x10000, tree1

    stdcall dword[tl_cur_next], tree1
  .filter:
  add eax,304
  loop @b
  stdcall dword[tl_cur_beg],tree1 ;ставим курсор на начало списка
.end_dir_init:

;--- load color option file ---
	mov ebx,dword[fn_col_option]
	copy_path ebx,fn_syntax_dir,file_name_rez,0x0
	copy_path file_name_rez,sys_path,file_name,0x0
	stdcall [ted_init_syntax_file], tedit0,run_file_70,file_name

;--- get cmd line ---
  cmp byte[openfile_path+3],0 ;openfile_path
  je @f ;if file names exist
    mov esi,openfile_path
    call strlen ;eax=strlen
    mov [edit1.size],eax
    call but_no_msg_OpenFile
  @@:

align 4
red_win:
  call draw_window

align 4
still:
  mov eax,10
  mcall

  cmp al,1 ;изм. положение окна
  jz red_win
  cmp al,2
  jz key
  cmp al,3
  jz button
  cmp al,6
  jz mouse

  jmp still

align 4
draw_window:
  mcall 12,1

  xor eax,eax
  mov ebx,10*65536+485
  mov ecx,10*65536+320
  mov edx,[sc.work]
  or  edx,0x73000000
  mov edi,hed
  int 0x40

  mov edi,tedit0

  mcall 9,procinfo,-1
  stdcall EvSize,edi

  mov eax,13 ;верхний прямоугольник, для очистки верхней панели
  xor ebx,ebx
  mov ecx,ted_wnd_t
  mov bx,word [procinfo.client_box.width]
  inc bx
  int 0x40

	mov eax,4
	mov ebx,185*65536+9
	mov ecx,[sc.work_text]
	or  ecx,0x80000000
	mov edx,txtFile
	int 0x40

  stdcall [edit_box_draw], dword edit1
  stdcall [menu_bar_draw], dword menu_data_1

  call draw_but_toolbar

  cmp [err_ini0],1
  jne @f
    mov eax,4
    mov ebx,ted_wnd_l
    add ebx,ted_rec_l
    shl ebx,16
    add ebx,ted_wnd_t
    add ebx,ted_rec_t
    mov ecx,0x80ff0000
    mov edx,txtErrIni0
    int 0x40

    add ebx,10 ;move <--y-->
    mov ecx,0x80ff0080
    mov edx,file_name
    int 0x40
  @@:

  cmp [err_ini0],1
  je @f
    stdcall [ted_draw], tedit0
  @@:

  mcall 12,2
  ret

align 4
mouse:
  stdcall [edit_box_mouse], dword edit1

  test word [edit1.flags],10b;ed_focus ; если не в фокусе, выходим
  jne still

  stdcall [ted_mouse], tedit0

  cmp byte[tedit0.panel_id],TED_PANEL_FIND ;if not panel
  jne @f
    stdcall [edit_box_mouse], dword edit2
  @@:
  cmp byte[tedit0.panel_id],TED_PANEL_SYNTAX ;if not panel
  jne .menu_bar_1 ;@f
  stdcall [tl_mouse], tree1
;-----------------------------------------------
.menu_bar_1:
  mov [menu_data_1.get_mouse_flag],1
; mouse event for Menu 1
  push	dword menu_data_1
  call	[menu_bar_mouse]
  cmp	[menu_data_1.click],dword 1
  jne	.mnu_1
  cmp [menu_data_1.cursor_out],dword 4
  je	button.exit	
  cmp [menu_data_1.cursor_out],dword 3
  jne	@f
    stdcall [ted_but_save_file], tedit0,run_file_70,[edit1.text]
  @@:
  cmp [menu_data_1.cursor_out],dword 2
  jne	@f
    call ted_but_open_file
  @@:
  cmp [menu_data_1.cursor_out],dword 1
  jne	@f
    call ted_but_new_file
  @@:
  ;cmp [menu_data_1.cursor_out],dword 0
  ;jne @f
.mnu_1:
  jmp still
;---------------------------------------------------------------------

;output:
; ah = symbol
align 4
proc KeyConvertToASCII, table:dword
  push ebx
  mov ebx,dword[table] ;convert scan to ascii
  ror ax,8
  xor ah,ah
  add bx,ax
  mov ah,byte[ebx]
  pop ebx
  ret
endp

align 4
key:
  mcall 66,3 ;66.3 получить состояние управляющих клавиш
  xor esi,esi
  mov ecx,1
  test al,0x03 ;[Shift]
  jz @f
    mov cl,2
    or esi,KM_SHIFT
  @@:
  test al,0x0c ;[Ctrl]
  jz @f
    or esi,KM_CTRL
  @@:
  test al,0x30 ;[Alt]
  jz @f
    mov cl,3
    or esi,KM_ALT
  @@:
  test al,0x80 ;[NumLock]
  jz @f
    or esi,KM_NUMLOCK
  @@:

  mcall 26,2,,conv_tabl ;26.2 получить раскладку клавиатуры
  mcall 2 ;получаем код нажатой клавиши
  stdcall [tl_key], tree1

  test word [edit1.flags],10b;ed_focus ; если не в фокусе, выходим
  je @f
    cmp ah,0x80 ;if key up
    ja still
    cmp ah,42 ;[Shift] (left)
    je still
    cmp ah,54 ;[Shift] (right)
    je still
    cmp ah,56 ;[Alt]
    je still
    cmp ah,29 ;[Ctrl]
    je still
    cmp ah,69 ;[Pause Break]
    je still

    stdcall KeyConvertToASCII, dword conv_tabl
    stdcall [edit_box_key], dword edit1
    jmp still
  @@:

  test word [edit2.flags],10b;ed_focus ; если не в фокусе, выходим
  je @f
    cmp ah,0x80 ;if key up
    ja still
    cmp ah,42 ;[Shift] (left)
    je still
    cmp ah,54 ;[Shift] (right)
    je still
    cmp ah,56 ;[Alt]
    je still
    cmp ah,29 ;[Ctrl]
    je still
    cmp ah,69 ;[Pause Break]
    je still

    stdcall KeyConvertToASCII, dword conv_tabl
    stdcall [edit_box_key], dword edit2
    jmp still
  @@:

  stdcall [ted_key], tedit0, conv_tabl,esi
  jmp still

align 4
button:
;  cmp [menu_active],1 ;если нажали меню, то сначала реакция на меню
;  jne @f ;mouse.menu_bar_1
;    mov [menu_active],0
;    jmp still
;  @@:

  mcall 17 ;получить код нажатой кнопки
  cmp ah,3
  jne @f
    call ted_but_new_file
  @@:
  cmp ah,4
  jne @f
    call ted_but_open_file
  @@:
  cmp ah,5
  jne @f
    stdcall [ted_but_save_file], tedit0,run_file_70,[edit1.text]
  @@:
  cmp ah,6
  jne @f
    stdcall [ted_but_select_word], tedit0
  @@:
  cmp ah,7
  jne @f
    stdcall [ted_but_cut], tedit0
  @@:
  cmp ah,8
  jne @f
    stdcall [ted_but_copy], tedit0
  @@:
  cmp ah,9
  jne @f
    stdcall [ted_but_paste], tedit0
  @@:
  cmp ah,10
  jne @f
    call ted_but_find
  @@:
  cmp ah,11
  jne @f
    call but_replace
  @@:
  cmp ah,12
  jne @f
    call but_find_key_w
  @@:
  cmp ah,13
  jne @f
    stdcall [ted_but_sumb_upper], tedit0
  @@:
  cmp ah,14
  jne @f
    stdcall [ted_but_sumb_lover], tedit0
  @@:
  cmp ah,15
  jne @f
    stdcall [ted_but_reverse], tedit0
  @@:
  cmp ah,16
  jne @f
    stdcall [ted_but_undo], tedit0
  @@:
  cmp ah,17
  jne @f
    stdcall [ted_but_redo], tedit0
  @@:
  cmp ah,18
  jne @f
    stdcall but_sumb_invis, tedit0
  @@:
  cmp ah,19
  jne @f
    stdcall but_k_words_show, tedit0
  @@:
  cmp ah,20
  jne @f
    stdcall but_synt_show, tedit0
  @@:

  cmp ah,200
  jne @f
    stdcall ted_but_open_syntax, tedit0
  @@:
  cmp ah,201
  jne @f
    stdcall [ted_but_find_next], tedit0
  @@:

  cmp ah,1
  jne still
.exit:
  stdcall [ted_can_save], tedit0
  cmp al,1
  jne @f
    stdcall [mb_create],msgbox_8,thread ;message: save changes in file?
    jmp still
  @@:
  stdcall mem.Free,[bmp_icon]

  stdcall [ted_delete], tedit0
  stdcall dword[tl_data_clear], tree1
  mcall -1 ;выход из программы



txtErrIni0 db 'Не открылся файл с иконками',0
err_ini0 db 0

edit1 edit_box 250, 220, 5, 0xffffff, 0xff80, 0xff0000, 0xff, 0x4080, 4090, openfile_path, mouse_dd, 0
edit2 edit_box TED_PANEL_WIDTH-1, 0, 20, 0xffffff, 0xff80, 0xff0000, 0xff, 0x4080, 300, buf_find, mouse_dd, 0

buf_find db 302 dup(0)

if lang eq ru
  head_f_i0:
  head_f_l0  db 'Системная ошибка',0
  err_message_found_lib0 db 'Не найдена библиотека ',39,'box_lib.obj',39,0
  err_message_import0 db 'Ошибка при импорте библиотеки ',39,'box_lib.obj',39,0
  err_message_found_lib1 db 'Не найдена библиотека ',39,'msgbox.obj',39,0
  err_message_import1 db 'Ошибка при импорте библиотеки ',39,'msgbox.obj',39,0
  err_message_found_lib2 db 'Не найдена библиотека ',39,'proc_lib.obj',39,0
  err_message_import2 db 'Ошибка при импорте библиотеки ',39,'proc_lib.obj',39,0
else
  head_f_i0:
  head_f_l0  db 'System error',0
  err_message_found_lib0 db 'Sorry I cannot found library ',39,'box_lib.obj',39,0
  err_message_import0 db 'Error on load import library ',39,'box_lib.obj',39,0
  err_message_found_lib1 db 'Sorry I cannot found library ',39,'msgbox.obj',39,0
  err_message_import1 db 'Error on load import library ',39,'msgbox.obj',39,0
  err_message_found_lib2 db 'Sorry I cannot found library ',39,'proc_lib.obj',39,0
  err_message_import2 db 'Error on load import library ',39,'proc_lib.obj',39,0
end if

;library structures
l_libs_start:
	lib0 l_libs lib0_name, sys_path, file_name, system_dir0, err_message_found_lib0, head_f_l0, boxlib_import,err_message_import0, head_f_i0
	lib1 l_libs lib1_name, sys_path, file_name, system_dir1, err_message_found_lib1, head_f_l0, msgbox_lib_import, err_message_import1, head_f_i0
	lib2 l_libs lib2_name, sys_path, file_name, system_dir2, err_message_found_lib2, head_f_l0, proclib_import, err_message_import2, head_f_i0
load_lib_end:


i_end:
	rb 1024
	align 16
	procinfo process_information
		rb 1024
	thread:
	rb 1024
stacktop:
	sys_path:
		rb 4096
	file_name:
		rb 4096
	file_name_rez:
		rb 4096
	plugin_path:
		rb 4096
	openfile_path:
		rb 4096
	filename_area:
		rb 256
	file_info:
		rb 40
mem:
