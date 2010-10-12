;*****************************************************************************
; Example for Box_lib: checkbox
; Copyright (c) 2007-2010, Alexey  Teplov aka <Lrz>
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;	 * Redistributions of source code must retain the above copyright
;	   notice, this list of conditions and the following disclaimer.
;	 * Redistributions in binary form must reproduce the above copyright
;	   notice, this list of conditions and the following disclaimer in the
;	   documentation and/or other materials provided with the distribution.
;	 * Neither the name of the <organization> nor the
;	   names of its contributors may be used to endorse or promote products
;	   derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Alexey Teplov ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;******************************************************************************

use32
	org 0x0
;------------ <head> from kolibrios programm
	db 'MENUET01'
	dd 0x01
	dd START
	dd MEM_END
	dd PRIL_END
	dd PRIL_END
	dd 0x0
	dd cur_dir_path
;------------ </head>
;------------ <include macros>
include '../../../../macros.inc'
include '../load_lib.mac'
include 'box_lib.mac'
;include 'macros.inc'
;include 'load_lib.mac'
;------------ </include macros>
;------------ <init library>
@use_library
;------------ </init library>
;---------------------------------------------------------------------
;--- Start of program ----------------------------------------------
;---------------------------------------------------------------------
START:
	mcall	40,0x27

sys_load_library  library_name, cur_dir_path, library_path, system_path, \
err_message_found_lib, head_f_l, myimport, err_message_import, head_f_i
       	test	eax,eax
	jnz	exit
;init checkboxes
	push	dword check1
	call	[init_checkbox]

	push	dword check2
	call	[init_checkbox]

	push	dword check3
	call	[init_checkbox]

	
window:
	call draw_window		;первоначально необходимо нарисовать окно
align 4
still:				;основной обработчик
	mcall	10		;Ожидать события
	dec  eax
	jz   window
	dec  eax
	jz   key
	dec  eax
	jz   button

	push	dword check1
	call	[check_box_mouse]

	push	dword check2
	call	[check_box_mouse]

	push	dword check3
	call	[check_box_mouse]


	jmp still    ;если ничего из перечисленного то снова в цикл
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
button:
	mcall	17	;получить идентификатор нажатой клавиши
	test ah,ah	;если в ah 0, то перейти на обработчик событий still
	jz  still
exit:	mcall	-1
key:
	mcall	2	;загрузим значение 2 в регистор eax и получим код нажатой клавиши

	jmp still

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
align 4
draw_window:		;рисование окна приложения
	mcall	12,1
	mcall	0,(50*65536+390),(30*65536+200),0x33AABBCC,0x805080DD,hed

	push	dword check1
	call	[check_box_draw]

	push	dword check2
	call	[check_box_draw]

	push	dword check3
	call	[check_box_draw]


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
head_f_l		db 'System error',0
err_message_import	db 'Error on load import library box_lib.obj',0

myimport:   

init_checkbox	dd	aInit_checkbox
check_box_draw	dd	aCheck_box_draw
check_box_mouse dd	aCheck_box_mouse
version_ch	dd	aVersion_ch
		dd	0,0


aInit_checkbox	 db 'init_checkbox',0
aCheck_box_draw  db 'check_box_draw',0
aCheck_box_mouse db 'check_box_mouse',0
aVersion_ch	 db 'version_ch',0
;---------------------------------------------------------------------
check1 check_box (20 shl 16 + 12),(45 shl 16 + 12),6,0xC0AABBCC,0,0x80000000,check_text,ch_flag_en
check2 check_box (20 shl 16 + 22),(60 shl 16 + 12),6,0xC0AABBCC,0,0x80000000,check_text2
check3 check_box (20 shl 16 + 12),(75 shl 16 + 22),6,0xC0AABBCC,0,0x80000000,check_text3
;---------------------------------------------------------------------
hed		db 'CheckBox Exemples <Lrz> date 12.10.2010',0
hed_end:
;---------------------------------------------------------------------
check_text	db 'First checkbox',0
check_text2	db 'Second checkbox',0
check_text3	db 'Number 3 checkbox',0
;---------------------------------------------------------------------
MEM_END:
cur_dir_path	rb 1024
library_path	rb 1024
		rb 1024		;for stack
PRIL_END:


