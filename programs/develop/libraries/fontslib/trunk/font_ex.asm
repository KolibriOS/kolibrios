; Copyright (c) 2009, <Lrz>
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;       * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;       * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation and/or other materials provided with the distribution.
;       * Neither the name of the <organization> nor the
;       names of its contributors may be used to endorse or promote products
;       derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Alexey Teplov nickname <Lrz> ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*****************************************************************************

;заголовок приложения
use32		     ; транслятор, использующий 32 разрядных команды
    org 0x0		   ; базовый адрес кода, всегда 0x0
    db 'MENUET01'	 ; идентификатор исполняемого файла (8 байт)
    dd 0x1		  ; версия формата заголовка исполняемого файла
    dd start		    ; адрес, на который система передаёт управление
			; после загрузки приложения в память
    dd mem		  ; размер приложения
    dd mem		    ; Объем используемой памяти, для стека отведем 0х100 байт и выровним на грницу 4 байта
    dd mem		    ; расположим позицию стека в области памяти, сразу за телом программы. Вершина стека в диапазоне памяти, указанном выше
    dd 0x0		; указатель на строку с параметрами.
    dd way_of_ini
include '../../../../macros.inc'
include '../../../../KOSfuncs.inc'
include '../../../../load_lib.mac'
include '../../box_lib/trunk/box_lib.mac'
	@use_library	;use load lib macros
start:
;universal load library/librarys
sys_load_libraries l_libs_start,end_l_libs
;if return code =-1 then exit, else nornary work
	cmp	eax,-1
	jz	exit
	mcall	40,0x27 	;установить маску для ожидаемых событий

	mov  eax,48
	mov  ebx,3
	mov  ecx,sc
	mov  edx,sizeof.system_colors
	mcall
	mov  eax,dword [sc.work]
	mov  dword [con_colors+4],eax

;       mcall   66,1,0
       call [initialization_font]	; инициализация списка шрифтов 
       push dword (8 shl 16 +16)	; поиск нужного шрифта в наборе шрифтов (пока доступен только 8х16)
       call [get_font]
	test	eax,eax 		;нашли ? 
	jnz	exit
;;;;;;;;;;;;;;;;;;;;
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

	jmp still    ;если ничего из перечисленного то снова в цикл
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
button:
	mcall	17	;получить идентификатор нажатой клавиши
	test ah,ah	;если в ah 0, то перейти на обработчик событий still
	jz  still
exit:	
	call	[free_fulder_info]
	call	[free_font]
	mcall	-1
key:
	mcall	2	;загрузим значение 2 в регистор eax и получим код нажатой клавиши

	push	dword edit1
	call	[edit_box_key]
 

	mcall	13,<20,650>,<40,16>, dword[con_colors+4]


	push	dword 20 shl 16 + 40	; esp+12= dd x shl 16 + y x- координата по Х, y - координата по Y
	push	dword con_colors	; esp+8 = dd point to color of background and font
	push	dword text		; esp+4 = dd point to ASCIIZ
; esp+0 = dd back
	call	[font_draw_on_string]	; вывести по глифам строчку


	jmp still

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
align 4
draw_window:		;рисование окна приложения
	mcall	12,1
	xor  eax,eax		 ;обнулить eax
	mov  ebx,50*65536+680	 ;[координата по оси x]*65536 + [размер по оси x]
	mov  ecx,30*65536+200	 ;[координата по оси y]*65536 + [размер по оси y]
	mov  edx,[sc.work]	 ; color of work area RRGGBB,8->color gl
	or   edx,0x34000000
	mov  edi,hed
	mcall			 ;нарисовать окно приложения
	
	push	dword edit1
	call	[edit_box_draw]
;
	push	dword 20 shl 16 + 40	; esp+12= dd x shl 16 + y x- координата по Х, y - координата по Y
	push	dword con_colors	; esp+8 = dd point to color of background and font
	push	dword text; esp+4 = dd point to ASCIIZ
; esp+0 = dd back
	call	[font_draw_on_string]	; вывести по глифам строчку
	mov eax,12		 ;Функция 12 - начать/закончить перерисовку окна.
	mov ebx,2		 ;Подфункция 2 - закончить перерисовку окна.
	mcall
	ret
;;;;;;;;;;;;
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;DATA данные
l_libs_start:
library01 l_libs library_name, library_path, system_path, font_import
library02 l_libs library_name1, library_path, system_path1, import_box_lib
end_l_libs:

;
system_path	 db '/sys/lib/'
library_name	 db 'fonts_lib.obj',0

system_path1	  db '/sys/lib/'
library_name1	  db 'box_lib.obj',0

align 4
import_box_lib:   

edit_box_draw	dd	aEdit_box_draw
edit_box_key	dd	aEdit_box_key
edit_box_mouse	dd	aEdit_box_mouse
version_ed	dd	aVersion_ed

		dd	0,0

aEdit_box_draw	db 'edit_box_draw',0
aEdit_box_key	db 'edit_box_key',0
aEdit_box_mouse db 'edit_box_mouse',0
aVersion_ed	db 'version_ed',0

font_import:
initialization_font	dd	a_initialization_font
get_font		dd	a_get_font
free_fulder_info	dd	a_free_fulder_info
free_font		dd	a_free_font
font_draw_on_string	dd	a_font_draw_on_string
show_all_glif		dd	a_show_all_glif
Version_fn	      dd      a_Version_fn
		      dd      0,0

a_initialization_font	db 'initialization_font',0
a_get_font		db 'get_font',0
a_free_fulder_info	db 'free_fulder_info',0
a_free_font		db 'free_font',0
a_font_draw_on_string	db 'font_draw_on_string',0
a_show_all_glif 	db 'show_all_glif',0
a_Version_fn	      db 'version_fn',0



edit1 edit_box 350,175,5,0xffffff,0x6f9480,0,0xAABBCC,0,test_leght,text,ed_focus,text_end-text-1,text_end-text-1

text db   'Пример использования библиотеки шрифтов fonts_lib.obj',0
text_end:
rb  256
test_leght = ($-text)-1

hed db	 "Font's demo <Lrz>",0
align 4
con_colors	dd	0x1E1EFF, 0x96FFCF

align 4
sc     system_colors
way_of_ini	rb 4096
library_path	rb 4096

align 4
i_end:
rb 1024
mem:
		;конец кода