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
BUF_SIZE equ 4096 ;buffer for copy|paste
maxSyntaxFileSize equ 310000
CAPT_PATH_WIDTH equ 50 ;ширина подписи перед текстовым полем

include '../../macros.inc'
include '../../proc32.inc'
include '../../develop/libraries/box_lib/load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../dll.inc'
include 'te_data.inc'
include 'te_work.inc' ;text work functions

@use_library_mem mem.Alloc,mem.Free,mem.ReAlloc,dll.Load

;Макрос для загрузки изображений с использованием библиотеки libimg.obj
;для использования макроса нужны переменные:
; - run_file_70 FileInfoBlock
; - image_data dd 0
macro load_image_file path,buf,size
{
	;path - может быть переменной или строковым параметром
	if path eqtype '' ;проверяем задан ли строкой параметр path
		jmp @f
			local .path_str
			.path_str db path ;формируем локальную переменную
			db 0
		@@:
		;32 - стандартный адрес по которому должен быть буфер с системным путем
		copy_path .path_str,[32],file_name,0x0
	else
		copy_path path,[32],file_name,0x0 ;формируем полный путь к файлу изображения, подразумеваем что он в одной папке с программой
	end if
 
	stdcall mem.Alloc, dword size ;выделяем память для изображения
	mov [buf],eax
 
	mov eax,70 ;70-я функция работа с файлами
	mov [run_file_70.Function], 0
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], dword size
	m2m [run_file_70.Buffer], [buf]
	mov byte[run_file_70+20], 0
	mov [run_file_70.FileName], file_name
	mov ebx,run_file_70
	int 0x40 ;загружаем файл изображения
	cmp ebx,0xffffffff
	je @f
		;определяем вид изображения и переводим его во временный буфер image_data
		stdcall dword[img_decode], dword[buf],ebx,0
		mov dword[image_data],eax
		;преобразуем изображение к формату rgb
		stdcall dword[img_to_rgb2], dword[image_data],dword[buf]
		;удаляем временный буфер image_data
		stdcall dword[img_destroy], dword[image_data]
	@@:
}
 
image_data dd 0 ;указатель на временную память. для нужен преобразования изображения
icon_tl_sys dd 0 ;указатель на память для хранения системных иконок

align 4
start:
	mcall 48,3,sc,sizeof.system_colors

	mcall 68,11
	or eax,eax
	jz button.exit

	mcall 66,1,1 ;scan code
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

; читаем файл с курсорами и линиями
	load_image_file 'tl_sys_16.png', icon_tl_sys,54+3*256*13
	mov eax,dword[icon_tl_sys]
	mov dword[tree1.data_img_sys],eax
;---------------------------------------------------------------------
; читаем bmp файл с иконками узлов
	load_image_file 'tl_nod_16.png', icon_tl_sys,54+3*256*2
	mov eax,dword[icon_tl_sys]
	mov dword[tree1.data_img],eax
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
	mcall 2
	stdcall [tl_key],tree1

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
		stdcall [edit_box_key],edit1
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
	stdcall [ted_save_file], tedit0,run_file_70,[edit1.text]
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

hed db 'TextEditor syntax file converter 01.07.14',0 ;подпись окна
conv_tabl rb 128 ; таблица для конвертирования scan-кода в ascii-код

txt122 db 'Загр. файл',0
txt148 db 'Сохр. файл',0
txt_inp_file db 'Исх. файл:',0
txt_out_file db 'Вых. файл:',0

txt_no_kpack db 'Открываемый файл сжат Kpack-ом.',13,10,'Для работы с файлом распакуйте его используя системную программу Kpack.',13,10,'Работа со сжатыми файлами пока не поддерживается.',0

head_f_i:
head_f_l db 'System error',0
err_message_found_lib_0 db 'Sorry I cannot found library ',39,'box_lib.obj',39,0
err_message_import_0 db 'Error on load import library ',39,'box_lib.obj',39,0
err_message_found_lib_1 db 'Sorry I cannot found library ',39,'libimg.obj',39,0
err_message_import_1 db 'Error on load import library ',39,'libimg.obj',39,0

;library structures
l_libs_start:
	lib0 l_libs lib_name_0, sys_path, file_name, system_dir_0, err_message_found_lib_0, head_f_l, import_box_lib,err_message_import_0, head_f_i
	lib1 l_libs lib_name_1, sys_path, file_name, system_dir_1, err_message_found_lib_1, head_f_l, import_libimg,err_message_import_1, head_f_i
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
