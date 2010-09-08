;
;   RDsave для Kolibri (0.6.5.0 и старше)
;
; version:	1.3
; last update:  08/09/2010
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      select path with OpenDialog,
;               keys 1,2,3,4 for select options
;---------------------------------------------------------------------
;   Mario79 2005
;   Heavyiron 12.02.2007
;   <Lrz>     11.05.2009 - для работы нужна системная библиотека box_lib.obj
;   Компилировать FASM'ом
;
;---------------------------------------------------------------------
include 'lang.inc'
include '..\..\..\macros.inc'

appname equ 'RDsave '
version equ '1.3'
  
use32 	     ; включить 32-битный режим ассемблера
org	 0x0	     ; адресация с нуля

	db 'MENUET01'  ; 8-байтный идентификатор MenuetOS
	dd 0x01	     ; версия заголовка (всегда 1)
	dd START	     ; адрес первой команды
	dd IM_END	     ; размер программы
	dd I_END	     ; количество памяти
	dd stacktop     ; адрес вершины стэка
	dd 0x0	     ; адрес буфера для параметров (не используется)
	dd cur_dir_path

;include '..\..\..\develop\examples\editbox\trunk\editbox.inc'
include '../../../develop/libraries/box_lib/load_lib.mac'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
	@use_library

;use_edit_box
;al equ eax      ; \ decrease kpack'ed size
;purge mov       ; /

;---------------------------------------------------------------------
;---  НАЧАЛО ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------
align 4
START:
	mcall	68,11

load_libraries l_libs_start,end_l_libs

	cmp	eax,-1
	jz	close

	mov	edi,filename_area
	mov	esi,start_temp_file_name
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b


	mov	edi,fname_buf
	mov	esi,path4
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b

;OpenDialog	initialisation
	push    dword OpenDialog_data
	call    [OpenDialog_Init]

; prepare for PathShow
	push	dword PathShow_data_1
	call	[PathShow_prepare]

	mcall	40,100111b
red:			; перерисовать окно
	mcall	48,3,sc,sizeof.system_colors

	call	draw_window	; вызываем процедуру отрисовки окна
;---------------------------------------------------------------------
;---  ЦИКЛ ОБРАБОТКИ СОБЫТИЙ  ----------------------------------------
;---------------------------------------------------------------------
still:
	mcall 10

	dec	eax	 ; перерисовать окно?
	jz	red	 ; если да - на метку red
	dec	eax 
	jz	key
	dec	eax
	jz	button

	jmp	still
;---------------------------------------------------------------------
button:
	mcall	17	; получить идентификатор нажатой кнопки
	cmp	ah,1		 ; кнопка с id=1("закрыть")?
	jne	noclose
close:
	or	 eax,-1 	 ; функция -1: завершить программу
	mcall

noclose:
	push	eax
	call	clear_err
	pop	eax
	push	16
	xor	ebx,ebx
	inc	ebx	; 16.1 = save to /FD/1
	cmp	ah,2
	je	doit
	inc	ebx	; 16.2 = save to /FD/2
	cmp	ah,3
	je	doit
	pop	ebx
	push	18
	mov	bl,6	; 18.6 = save to specified folder
	mov	ecx, path3
	cmp	ah,4
	je	doit

; invoke OpenDialog
	push    dword OpenDialog_data
	call    [OpenDialog_Start]
	cmp	[OpenDialog_data.status],1
	jne	still

; prepare for PathShow
	push	dword PathShow_data_1
	call	[PathShow_prepare]

	call	draw_PathShow

	mov	ecx,fname_buf ;path4
doit:
	pop	eax
	mcall
	call	check_for_error
	jmp	still
;---------------------------------------------------------------------
key:
	mcall	2
	cmp	ah,0x31
	jb	still
	cmp	ah,0x34
	ja	still
	sub	ah,0x30
	inc	ah
	jmp	noclose
;---------------------------------------------------------------------
check_for_error:		      ;Обработчик ошибок
	mov	ecx,[sc.work_text]
	mov	edx,ok
	test	eax,eax
	jz	print
	mov	ecx,0xdd2222
	add	edx,error3 - ok
	dec	eax
	dec	eax
	jz	print
	add	edx,error5 - error3
	dec	eax
	dec	eax
	jz	print
	add	edx,error8 - error5
	dec	eax
	dec	eax
	dec	eax
	jz	print
	add	edx,error9 - error8
	dec	eax
	jz	print
	add	edx,error10 - error9
	dec	eax
	jz	print
	add	edx,error11 - error10
	dec	eax
	jz	print
	add	edx,aUnknownError - error11
print:
	mov	eax,4				   ;надписи
	mov	ebx,20 shl 16 + 148
	or	ecx,0x80000000
	mcall
	ret
;---------------------------------------------------------------------
clear_err:
	mov	eax,13
	mov	ebx,15 shl 16 + 240
	mov	ecx,145 shl 16 +15
	mov	edx,[sc.work]
	mcall
	ret
;---------------------------------------------------------------------
draw_PathShow:
	pusha
	mcall	13,<8,172>,<110,15>,0xffffff
; draw for PathShow
	push	dword PathShow_data_1
	call	[PathShow_draw]
	popa
	ret
;---------------------------------------------------------------------
;---  ОПРЕДЕЛЕНИЕ И ОТРИСОВКА ОКНА  ----------------------------------
;---------------------------------------------------------------------
draw_window:
	mcall	12,1	; функция 12: сообщить ОС об отрисовке окна
			; 1 - начинаем рисовать
					 ; СОЗДАЁМ ОКНО
	xor	eax,eax			 ; функция 0 : определить и отрисовать окно
	mov	ebx,200 shl 16 + 300	 ; [x старт] *65536 + [x размер]
	mov	ecx,200 shl 16 + 190	 ; [y старт] *65536 + [y размер]
	mov	edx,[sc.work]		 ; цвет рабочей области  RRGGBB,8->color gl
	or	edx,0x34000000
	mov	edi,title			; ЗАГОЛОВОК ОКНА
	mcall

	call	draw_PathShow

;отрисовка теней кнопок
	mcall	13,<194,60>,<34,15>,0x444444

	add	ecx,20 shl 16
	mcall

	add	ecx,20 shl 16
	mcall

	add	ecx,40 shl 16
	mcall
;отрисовка кнопок
	sub	ebx,4 shl 16
	sub	ecx,4 shl 16
	mcall	8,,,5,[sc.work_button]	

	sub	ecx,40 shl 16
	dec	edx
	mcall

	sub	ecx,20 shl 16
	dec	edx
	mcall

	sub	ecx,20 shl 16
	dec	edx
	mcall
; надписи
	mov	ecx,[sc.work_text]
	or	ecx,0x80000000
	mcall	4,<45,12>,,label1

	mov	ebx,150 shl 16 + 35
	mov	edx,path1
	mcall

	add	ebx,20
	mov	edx,path2
	mcall

	mov	ebx,75 shl 16 + 75
	mov	edx,path3
	mcall

	mov	ebx,30 shl 16 + 97
	mov	edx,label2
	mcall

	mov	ebx,40 shl 16 + 135
	mov	edx,label3
	mcall

	mov	ecx,[sc.work_button_text]
	or	ecx,0x80000000
	mov	ebx,195 shl 16 + 35
	mov	edx,save
	mcall

	push	edx
	mov	edx,key_help
	call	key_help_correct
	pop	edx

	add ebx,20
	mcall

	push	edx
	mov	edx,key_help+2
	call	key_help_correct
	pop	edx

	add ebx,20
	mcall

	push	edx
	mov	edx,key_help+4
	call	key_help_correct
	pop	edx

	mov	edx,select
	add ebx,40
	mcall

	mov	edx,key_help+6
	call	key_help_correct

	mcall	12,2	; функция 12: сообщить ОС об отрисовке окна
			; 2, закончили рисовать
	ret		; выходим из процедуры
;---------------------------------------------------------------------
key_help_correct:
	push	ebx
	ror	ebx,16
	mov	bx,270
	rol	ebx,16
	pusha
	mov	ecx,ebx
	sub	ebx,3 shl 16
	mov	bx,13
	sub	cx,3
	shl	ecx,16
	mov	cx,13
	mcall	13,,,0xffffff
	popa
	mcall	
	pop	ebx
	ret
;---------------------------------------------------------------------
;---  ДАННЫЕ ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------

title	db appname,version,0

;---------------------------------------------------------------------
PathShow_data_1:
.type			dd 0	;+0
.start_y		dw 113	;+4
.start_x		dw 10	;+6
.font_size_x		dw 6	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw 170	;+10
.font_number		dd 0	;+12	; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0x0	;+20
.background_color	dd 0x0	;+24
.text_pointer		dd fname_buf	;+28
.work_area_pointer	dd text_work_area	;+32
.temp_text_length	dd 0	;+36
;---------------------------------------------------------------------
if lang eq ru
save		db 'Сохранить',0
select		db ' Выбрать',0
label1		db 'Выберите один из вариантов:',0
label2		db 'Или выберите полный путь к файлу:',0
label3		db 'Все папки должны существовать',0
ok		db 'RAM-диск сохранен успешно',0
error3		db 'Неизвестная файловая система',0
error5		db 'Несуществующий путь',0
error8		db 'Нет места на диске',0
error9		db 'Таблица FAT разрушена',0
error10 	db 'Доступ запрещен',0
error11 	db 'Ошибка устройства',0
aUnknownError 	db 'Неизвестная ошибка',0
;---------------------------------------------------------------------
else if lang eq et
save		db 'Salvesta',0
select		db ' Valige',0
label1		db 'Vali №ks variantidest:',0
label2		db 'Vїi valige teekond failinimeni:',0
label3		db 'Kїik kataloogid peavad eksisteerima',0
ok		db 'RAM-ketas salvestatud edukalt',0
error3		db 'Tundmatu failis№steem',0
error5		db 'Vigane teekond',0
error8		db 'Ketas tфis',0
error9		db 'FAT tabel vigane',0
error10 	db 'Juurdepффs keelatud',0
error11 	db 'Seadme viga',0
aUnknownError 	db 'Tundmatu viga',0
;---------------------------------------------------------------------
else
save		db '  Save',0
select		db ' Select',0
label1		db 'Select one of the variants:',0
label2		db '  Or select full path to file:',0
label3		db '    All folders must exist',0
ok		db 'RAM-drive was saved successfully',0
error3		db 'Unknown file system',0
error5		db 'Incorrect path',0
error8		db 'Disk is full',0
error9		db 'FAT table corrupted',0
error10 	db 'Access denied',0
error11 	db 'Device error',0
aUnknownError 	db 'Unknown error',0

end if
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;DATA данные
;Всегда соблюдать последовательность в имени.
system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_ProcLib	db '/sys/lib/proc_lib.obj',0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

head_f_i:
head_f_l	db 'System error',0

err_message_found_lib1	db 'box_lib.obj - Not found!',0
err_message_found_lib2	db 'proc_lib.obj - Not found!',0

err_message_import1	db 'box_lib.obj - Wrong import!',0
err_message_import2	db 'proc_lib.obj - Wrong import!',0

;---------------------------------------------------------------------
l_libs_start:

library01  l_libs system_dir_Boxlib+9, cur_dir_path, library_path, system_dir_Boxlib, \
err_message_found_lib1, head_f_l, Box_lib_import, err_message_import1, head_f_i

library02  l_libs system_dir_ProcLib+9, cur_dir_path, library_path, system_dir_ProcLib, \
err_message_found_lib2, head_f_l, ProcLib_import, err_message_import2, head_f_i

end_l_libs:
;---------------------------------------------------------------------
OpenDialog_data:
.type			dd 1	; Save
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_pach		dd temp_dir_pach	;+16
.dir_default_pach	dd communication_area_default_pach	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd draw_window	;+28
.status			dd 0	;+32
.openfile_pach 		dd fname_buf	;+36
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size			dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size			dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_path:
	db '/sys/File Managers/opendial',0
communication_area_default_pach:
	db '/hd0/1/kolibri',0

Filter:
dd	Filter.end - Filter
.1:
db	'IMG',0
db	'IMA',0
.end:
db	0

start_temp_file_name:	db 'kolibri.img',0

;---------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init		dd aOpenDialog_Init
OpenDialog_Start	dd aOpenDialog_Start
;OpenDialog__Version	dd aOpenDialog_Version
        dd      0
        dd      0
aOpenDialog_Init	db 'OpenDialog_init',0
aOpenDialog_Start	db 'OpenDialog_start',0
;aOpenDialog_Version	db 'Version_OpenDialog',0
;---------------------------------------------------------------------
align 4
Box_lib_import:	
;init_lib		dd a_init
;version_lib		dd a_version


;edit_box_draw		dd aEdit_box_draw
;edit_box_key		dd aEdit_box_key
;edit_box_mouse		dd aEdit_box_mouse
;version_ed		dd aVersion_ed

;check_box_draw		dd aCheck_box_draw
;check_box_mouse	dd aCheck_box_mouse
;version_ch		dd aVersion_ch

;option_box_draw	dd aOption_box_draw
;option_box_mouse	dd aOption_box_mouse
;version_op		dd aVersion_op

;scrollbar_ver_draw	dd aScrollbar_ver_draw
;scrollbar_ver_mouse	dd aScrollbar_ver_mouse
;scrollbar_hor_draw	dd aScrollbar_hor_draw
;scrollbar_hor_mouse	dd aScrollbar_hor_mouse
;version_scrollbar	dd aVersion_scrollbar

;dinamic_button_draw	dd aDbutton_draw
;dinamic_button_mouse	dd aDbutton_mouse
;version_dbutton	dd aVersion_dbutton

;menu_bar_draw		dd aMenu_bar_draw
;menu_bar_mouse		dd aMenu_bar_mouse
;menu_bar_activate	dd aMenu_bar_activate
;version_menu_bar	dd aVersion_menu_bar

;FileBrowser_draw	dd aFileBrowser_draw
;FileBrowser_mouse	dd aFileBrowser_mouse
;FileBrowser_key	dd aFileBrowser_key
;Version_FileBrowser	dd aVersion_FileBrowser

PathShow_prepare	dd sz_PathShow_prepare
PathShow_draw		dd sz_PathShow_draw
;Version_path_show	dd szVersion_path_show
			dd 0
			dd 0

;a_init			db 'lib_init',0
;a_version		db 'version',0

;aEdit_box_draw		db 'edit_box',0
;aEdit_box_key		db 'edit_box_key',0
;aEdit_box_mouse	db 'edit_box_mouse',0
;aVersion_ed		db 'version_ed',0

;aCheck_box_draw	db 'check_box_draw',0
;aCheck_box_mouse	db 'check_box_mouse',0
;aVersion_ch		db 'version_ch',0

;aOption_box_draw	db 'option_box_draw',0
;aOption_box_mouse	db 'option_box_mouse',0
;aVersion_op		db 'version_op',0

;aScrollbar_ver_draw	db 'scrollbar_v_draw',0
;aScrollbar_ver_mouse	db 'scrollbar_v_mouse',0
;aScrollbar_hor_draw	db 'scrollbar_h_draw',0
;aScrollbar_hor_mouse	db 'scrollbar_h_mouse',0
;aVersion_scrollbar	db 'version_scrollbar',0

;aDbutton_draw		db 'dbutton_draw',0
;aDbutton_mouse		db 'dbutton_mouse',0
;aVersion_dbutton	db 'version_dbutton',0

;aMenu_bar_draw		db 'menu_bar_draw',0
;aMenu_bar_mouse		db 'menu_bar_mouse',0
;aMenu_bar_activate	db 'menu_bar_activate',0
;aVersion_menu_bar	db 'version_menu_bar',0

;aFileBrowser_draw	db 'FileBrowser_draw',0
;aFileBrowser_mouse	db 'FileBrowser_mouse',0
;aFileBrowser_key	db 'FileBrowser_key',0
;aVersion_FileBrowser	db 'version_FileBrowser',0

sz_PathShow_prepare	db 'PathShow_prepare',0
sz_PathShow_draw	db 'PathShow_draw',0
;szVersion_path_show	db 'version_PathShow',0
;---------------------------------------------------------------------

path1	db '/fd/1/',0
path2	db '/fd/2/',0
path3	db '/hd0/1/kolibri.img',0
path4	db '/hd0/1/kolibri/kolibri.img',0  ;для резервного сохранения
;---------------------------------------------------------------------
key_help:
	db '1',0
	db '2',0
	db '3',0
	db '4',0
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
sc     system_colors
mouse_dd	rd 1
;---------------------------------------------------------------------
cur_dir_path:
	rb 4096
;---------------------------------------------------------------------
library_path:
	rb 4096
;---------------------------------------------------------------------
temp_dir_pach:
	rb 4096
;---------------------------------------------------------------------
fname_buf:
	rb 4096
;---------------------------------------------------------------------
procinfo:
	rb 1024
;---------------------------------------------------------------------
filename_area:
	rb 256
;---------------------------------------------------------------------
text_work_area:
	rb 1024
;---------------------------------------------------------------------
align 4
	rb 4096
stacktop:
I_END:	; метка конца программы
