use32
  org 0x0
  db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
  dd 0x01
  dd start
  dd i_end ; размер приложения
  dd mem
  dd stacktop
  dd file_name
  dd sys_path

MAX_COLOR_WORD_LEN equ 40
BUF_SIZE equ 1000 ;buffer for copy|paste
maxSyntaxFileSize equ 310000
CAPT_PATH_WIDTH equ 50 ;ширина подписи перед текстовым полем

include '../../macros.inc'
include '../../proc32.inc'
include '../../develop/libraries/box_lib/load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../t_edit/mem.inc'
include 'te_data.inc'
include 'te_work.inc' ;text work functions

@use_library_mem mem.Alloc,mem.Free,mem.ReAlloc,0

align 4
start:
  mov eax,48
  mov ebx,3
  mov ecx,sc
  mov edx,sizeof.system_colors
  mcall

  mcall 68,11
  or eax,eax
  jz button.exit

  mcall 40,0x27

;-------------------------------------------------
  mov ecx,maxSyntaxFileSize
  stdcall mem.Alloc,ecx
  mov [options_file],eax
  mov [options_file_end],eax
  add [options_file_end],ecx


load_libraries l_libs_start,load_lib_end

;проверка на сколько удачно загузилась наша либа
	mov	ebp,lib0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
	mcall	-1	;exit not correct
@@:

;---------------------------------------------------------------------
  stdcall dword[tl_data_init],dword tree1
  copy_path fn_icon_tl_sys,sys_path,file_name,0

  mov ecx,3*256*13
  stdcall mem.Alloc,ecx
  mov dword[tree1.data_img_sys],eax

  mov [run_file_70.Function], 0
  mov [run_file_70.Position], 54
  mov [run_file_70.Flags], 0
  mov [run_file_70.Count], ecx
  mov [run_file_70.Buffer], eax
  mov byte[run_file_70+20], 0
  mov [run_file_70.FileName], file_name

  mov eax,70 ;load icon file
  mov ebx,run_file_70
  int 0x40
  cmp ebx,0
  jg @f
    mov dword[tree1.data_img_sys],0
  @@:
;---------------------------------------------------------------------
; читаем bmp файл с иконками узлов
  copy_path fn_icon_tl_nod,sys_path,file_name,0

  mov ecx,3*256*2
  stdcall mem.Alloc,ecx
  mov dword[tree1.data_img],eax

;  mov [run_file_70.Function], 0
;  mov [run_file_70.Position], 54
;  mov [run_file_70.Flags], 0
  mov [run_file_70.Count], ecx
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
  copy_path fn_syntax_dir,sys_path,file_name,0 ;берем путь к папке с файлами синтаксиса
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
    push dword tree1
    push dword 0x10000 ;1*2^16 - где 1 номер иконки с книгой
    push dword eax
    call dword[tl_node_add]

    stdcall dword[tl_cur_next],tree1
  .filter:
  add eax,304
  loop @b
  stdcall dword[tl_cur_beg],tree1 ;ставим курсор на начало списка
.end_dir_init:

;--- load color option file ---
stdcall [ted_init], tedit0
mov byte[file_name],0

align 4
red_win:
	call draw_window

align 4
still:
  mcall 10

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
	pushad
	mcall 12,1

	mov edx,[sc.work]
	or  edx,0x33000000
	mov edi,hed
	mcall 0,<10,555>,<10,333>

	mcall 9,procinfo,-1

	mov eax,8 ;кнопка
	mov ebx,5*65536+90
	mov ecx,195*65536+20
	mov edx,200
	mov esi,[sc.work_button];0xd0
	mcall

	;mov eax,8
	mov ebx,100*65536+85
	mov ecx,195*65536+20
	mov edx,201
	mov esi,0xd00000
	mcall

	mov eax,4 ;рисование текста
	mov ebx,10*65536+200
	mov ecx,[sc.work_button_text]
	or  ecx,0x80000000
	mov edx,txt122
	mcall

	mov ebx,105*65536+200
	mov ecx,0xffff00
	or  ecx,0x80000000
	mov edx,txt148
	mcall

	mov ebx,195*65536+10
	mov ecx,[sc.work_text]
	or  ecx,0x80000000
	mov edx,txt_inp_file
	int 0x40

	add ebx,20
	mov edx,txt_out_file
	int 0x40

	push dword PathShow_data_1
	call [PathShow_draw]
	
	stdcall [edit_box_draw],dword edit1
	stdcall [tl_draw],dword tree1

	;scroll 1
	mov [ws_dir_lbox.all_redraw],1
	stdcall [scrollbar_ver_draw],dword ws_dir_lbox
	stdcall [ted_draw], tedit0

	mcall 12,2
	popad
	ret

align 4
mouse:
	stdcall [tl_mouse],tree1
	stdcall [edit_box_mouse],edit1
	stdcall [ted_mouse], tedit0
	jmp still


align 4
key:
	mcall 2
	stdcall [tl_key],tree1
	stdcall [edit_box_key],edit1
;;;stdcall [ted_key], tedit0, conv_tabl,esi
	jmp still

align 4
button:
;  cmp [menu_active],1 ;если нажали меню, то сначала реакция на меню
;  jne @f ;mouse.menu_bar_1
;    mov [menu_active],0
;    jmp still
;  @@:

  mcall 17 ;получить код нажатой кнопки

  cmp ah,200
  jne @f
    call but_OpenSyntax
  @@:
  cmp ah,201
  jne @f
    call but_SaveSyntax
  @@:

  cmp ah,1
  jne still
.exit:
  ;push eax

  stdcall mem.Free,[options_file]

  stdcall [tl_data_clear], tree1
  stdcall [ted_delete], tedit0
  mcall -1 ;выход из программы

align 4
but_OpenSyntax:
  stdcall [tl_node_get_data],tree1
  pop dword [fn_col_option]
  call InitColText
  ret

align 4
but_SaveSyntax:
	stdcall [ted_but_save_file], tedit0,run_file_70,[edit1.text]
	ret

align 4
but_ctrl_o:
	push eax
	call get_wnd_in_focus
	cmp eax,0
	je @f
		stdcall [ted_open_file], eax,str_file_70,[edit1.text]
	@@:
	pop eax
	ret

;создание нового файла
align 4
but_ctrl_n:
	push eax
	call get_wnd_in_focus
	cmp eax,0
	je @f
		stdcall [ted_clear], eax,1
		stdcall [ted_draw], eax
	@@:
	pop eax
	ret

;определяем какое из окон редактора в фокусе
align 4
get_wnd_in_focus:
	xor eax,eax
	cmp dword[el_focus],tedit0
	jne @f
		mov eax,tedit0
	@@:
	;cmp dword[el_focus],tedit1
	;jne @f
	;       mov eax,tedit1
	;@@:
	ret

hed db 'TextEditor syntax file converter 26.08.11',0 ;подпись окна

txt122 db 'Загр. файл',0
txt148 db 'Сохр. файл',0
txt_inp_file db 'Исх. файл:',0
txt_out_file db 'Вых. файл:',0

err_message_found_lib0	 db 'Sorry I cannot found library box_lib.obj',0
head_f_i0:
head_f_l0	  db 'System error',0
err_message_import0	 db 'Error on load import library box_lib.obj',0
err_message_found_lib1	db 'Sorry I cannot found library msgbox.obj',0

;library structures
l_libs_start:
  lib0 l_libs boxlib_name, sys_path, file_name, system_dir0, err_message_found_lib0, head_f_l0, boxlib_import,err_message_import0, head_f_i0
load_lib_end:


i_end:
	rb 1024
	align 16
	procinfo process_information
	thread:
		rb 1024
stacktop:
  sys_path:
    rb 4096
  file_name:
    rb 4096
  file_name_rez:
    rb 4096
mem:
