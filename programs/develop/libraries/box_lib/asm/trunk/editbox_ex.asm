;заголовок приложения
use32		     ; транслятор, использующий 32 разрядных команды
    org 0x0		   ; базовый адрес кода, всегда 0x0
    db 'MENUET01'	 ; идентификатор исполняемого файла (8 байт)
    dd 0x1		  ; версия формата заголовка исполняемого файла
    dd start		    ; адрес, на который система передаёт управление
			; после загрузки приложения в память
    dd i_end		    ; размер приложения
    dd mem		    ; Объем используемой памяти, для стека отведем 0х100 байт и выровним на грницу 4 байта
    dd mem		    ; расположим позицию стека в области памяти, сразу за телом программы. Вершина стека в диапазоне памяти, указанном выше
    dd 0x0		; указатель на строку с параметрами.
    dd cur_dir_path


include '../../../../../macros.inc'
include '../../trunk/box_lib.mac'
include '../../load_lib.mac'
	@use_library	;use load lib macros
start:
;universal load library/librarys
sys_load_library  library_name, cur_dir_path, library_path, system_path, \
err_message_found_lib, head_f_l, myimport, err_message_import, head_f_i
;if return code =-1 then exit, else nornary work
	cmp	eax,-1
	jz	exit
	mcall	40,0x27 	;установить маску для ожидаемых событий
red_win:
    call draw_window		;первоначально необходимо нарисовать окно
align 4
still:				;основной обработчик
	mcall	10		;Ожидать события
	dec  eax
	jz   red_win
	dec  eax
	jz   key
	dec  eax
	jz   button

	push	dword edit1
	call	[edit_box_mouse]

	push	dword edit2
	call	[edit_box_mouse]

	push	dword check1
	call	[check_box_mouse]

	push	dword check2
	call	[check_box_mouse]

	push	dword Option_boxs
	call	[option_box_mouse]

	push	dword Option_boxs2
	call	[option_box_mouse]

	jmp still    ;если ничего из перечисленного то снова в цикл
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
button:
	mcall	17	;получить идентификатор нажатой клавиши
	test ah,ah	;если в ah 0, то перейти на обработчик событий still
	jz  still
exit:	mcall	-1
key:
	mcall	2	;загрузим значение 2 в регистор eax и получим код нажатой клавиши

	push	dword edit1
	call	[edit_box_key]

	push	dword edit2
	call	[edit_box_key]

	jmp still

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
align 4
draw_window:		;рисование окна приложения
	mcall	12,1
	mcall	0,(50*65536+390),(30*65536+200),0x33AABBCC,0x805080DD,hed

	push	dword edit1
	call	[edit_box_draw]

	push	dword edit2
	call	[edit_box_draw]

	push	dword check1
	call	[check_box_draw]

	push	dword check2
	call	[check_box_draw]

	push	dword Option_boxs
	call	[option_box_draw]	 

	push	dword Option_boxs2
	call	[option_box_draw]

	mcall	12,2
    ret
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;DATA данные
;Всегда соблюдать последовательность в имени.
system_path	 db '/sys/lib/'
library_name	 db 'box_lib.obj',0
; Если есть желание разъединить, то нужно использовать следующию конструкцию
;system_path      db '/sys/lib/box_lib.obj',0
;... любая последовательность других команд и определений.
;library_name     db 'box_lib.obj',0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

err_message_found_lib	db 'Sorry I cannot load library box_lib.obj',0
head_f_i:
head_f_l	db 'System error',0
err_message_import	db 'Error on load import library box_lib.obj',0

myimport:   

edit_box_draw	dd	aEdit_box_draw
edit_box_key	dd	aEdit_box_key
edit_box_mouse	dd	aEdit_box_mouse
version_ed	dd	aVersion_ed

check_box_draw	dd	aCheck_box_draw
check_box_mouse dd	aCheck_box_mouse
version_ch	dd	aVersion_ch

option_box_draw  dd	 aOption_box_draw
option_box_mouse dd	 aOption_box_mouse
version_op	 dd	 aVersion_op

		dd	0
		dd	0

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




check1 check_box 10,45,6,12,0x80AABBCC,0,0,check_text,14,ch_flag_en
check2 check_box 10,60,6,12,0x80AABBCC,0,0,check_text2,15

edit1 edit_box 350,3,5,0xffffff,0x6f9480,0,0xAABBCC,0,308,hed,mouse_dd,ed_focus,hed_end-hed-1,hed_end-hed-1
edit2 edit_box 350,3,25,0xffffff,0x6a9480,0,0,0,99,ed_buffer,mouse_dd,ed_figure_only

op1 option_box option_group1,10,90,6,12,0xffffff,0,0,op_text.1,op_text.e1-op_text.1
op2 option_box option_group1,10,105,6,12,0xFFFFFF,0,0,op_text.2,op_text.e2-op_text.2
op3 option_box option_group1,10,120,6,12,0xffffff,0,0,op_text.3,op_text.e3-op_text.3
op11 option_box option_group2,120,90,6,12,0xffffff,0,0,op_text.1,op_text.e1-op_text.1
op12 option_box option_group2,120,105,6,12,0xffffff,0,0,op_text.2,op_text.e2-op_text.2
op13 option_box option_group2,120,120,6,12,0xffffff,0,0,op_text.3,op_text.e3-op_text.3

option_group1	dd op1	;указатели, они отображаются по умолчанию, когда выводится 
option_group2	dd op12 ;приложение
Option_boxs	dd  op1,op2,op3,0
Option_boxs2	dd  op11,op12,op13,0
hed db	 'BOXs load from lib <Lrz> date 27.04.2009',0
hed_end:
rb  256
check_text db 'First checkbox'
check_text2 db 'Second checkbox'
op_text:		; Сопровождающий текст для чек боксов
.1 db 'Option_Box #1' 
.e1:
.2 db 'Option_Box #2'
.e2:
.3 db 'Option_Box #3'
.e3:
ed_buffer	rb 100
;-----------------------
;sc      system_colors

mouse_dd	rd 1
p_info	process_information
cur_dir_path	rb 4096
library_path	rb 4096
i_end:
rb 1024
mem: