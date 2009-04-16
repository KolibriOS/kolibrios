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
; THIS SOFTWARE IS PROVIDED BY Alexey Teplov aka <Lrz> ''AS IS'' AND ANY
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

;тут распологается модуль с помощью которого будут парситься все остальные секции
color_sym_black	equ	0
color_sym_blue	equ	1
color_sym_green equ	2
color_sym_turquoise equ 3
color_sym_red	equ	4

color_sym_lightgray equ 7

color_sym_lightblue equ	9
color_sym_lettuce   equ 10
color_sym_pink	equ	12
color_sym_yellow equ	14

macro use_any_sec
{
;узнаем работу предыдущего шага т.е. чему = timeout, если он 0, то визуальная часть не будет отображена на дисплее с выбором загрузочных секций.
;иначе мы ее должны отобразить и ждать заявленое время для выбора и конигурирования пукнктов секции от пользователя.

if DEBUG        
        pusha
	mov	ax,word [value_timeout] ;идет проверка на наличее значения timeout, для более быстрой работы, этот параметр должен быть уже обработан,т.е. в этом случае при его =0 будет сформирован указатель только на дефолтную секцию, иначе информация будет собрана по всем секциям и составлены указатели в блоке памяти
;        mov     ax,cx
        mov     cx,0x0a
        mov     di,show_db1
        mov     dword[ds:di],'    '
	mov	word [ds:di+4],'  '
        call    decode
;Show size
        mov     si,show_db1
        call    printplain
;
        popa
end if  

	test	ax,ax
	jz	.parse_run_only

;отобразим полный список всех найденых секций.
if DEBUG
	pusha
	mov	si,show_all_sect
	call	printplain
	popa
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov     al, 0xf6        ; Сброс клавиатуры, разрешить сканирование
        out     0x60, al
        xor     cx, cx
.wait_loop:       ; variant 2
; reading state of port of 8042 controller
        in      al, 64h
        and     al, 00000010b  ; ready flag
; wait until 8042 controller is ready
        loopnz  .wait_loop


; set keyboard typematic rate & delay
        mov     al, 0xf3
        out     0x60, al
        xor     cx, cx
@@:
        in      al, 64h
        test    al, 2
        loopnz  @b
        mov     al, 0
        out     0x60, al
        xor     cx, cx
@@:
        in      al, 64h
        test    al, 2
        loopnz  @b
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; get start time
        call    gettime
        mov     dword [start_timer],eax
        mov     word [timer_],newtimer
        mov     word [timer_+2],cs
;установить свое прерывание на таймер т.е. код будет перрываться ~18 раз в сек и переходить на обработчик
        cli
        push    0
        pop     es
        push    dword [es:8*4]
        pop     dword [old_timer]
        push    dword [timer_]
        pop     dword [es:8*4]    
        sti

;процедура формирования буфера для скролинга секций
;if DEBUG        
;        pusha
;	mov	ax,point_default
;        mov     ax,cx
;        mov     cx,0x0a
;        mov     di,show_db1
;        mov     dword[ds:di],'    '
;	mov	word [ds:di+4],'  '
;        call    decode                                                                    
;Show size
;        mov     si,show_db1
;        call    printplain
;
;	xor	ax,ax
; 	int	0x16
;        popa
;end if  
;;;;;;;;;;;;;размер предыдущей сеции установим =0
	mov	save_descript_size,18
;отобразить black screen
show_bl_sc 	;es=0xb800
.show_all_scr:
get_frame_buffer  ;es=0x2000
;отображение секций
	call	show_bl_sc_sect   ;es=0xb800
;отобразить активный курсор
.show_active_cursor:
show_act_cursor
show_descript	;макрос по отображению описания секции

;отобразить Press any key ....
	mov	eax,dword [old_timer]
	cmp	eax,dword [timer_]
	jz	.interrupt_16

show_timer_message
	mov word [start_stack],sp
.interrupt_16:
	xor	ax,ax	;получим информацию о том что нажато
	int	0x16
;check on change
	mov	ebx,dword [old_timer]
	cmp	ebx,dword [timer_]
	jz	@f
;restore timer interrupt
	cli
        push    0
        pop     es
;        mov     eax,dword [old_timer]  ; восстановим прежднее прерывание
        mov     [es:8*4],ebx
        mov     dword [timer_],ebx
	sti
	
	push	ax	
clear_timer_msg
	pop	ax
@@:
	call    clean_active_cursor   ;clean old cursor   ;es=0xb800

	cmp	ah,0x48  ;реакция системы на события
	jz	.up
	cmp	ah,0x50
	jz	.down
	cmp	ah,0x49
	jz	.pgup
	cmp	ah,0x51
	jz	.pgdown
	cmp	ah,0x47
	jz	.home
	cmp	ah,0x4f
	jz	.end

	cmp	al,0xD
	jnz	.show_active_cursor

	jmp	.end_show_all		;парсинг секции которая указана в point_default
.up:
	mov	si,point_to_point_def	;значение указателя
	add	si,2
	lea	ax,point_to_hframe

	cmp	si,ax
	ja	@f

	mov	point_to_point_def,si
	mov	ax,[si]
	mov	point_default,ax
       	jmp	.show_active_cursor
@@:     
	call	find_before_sect
	jmp	.show_all_scr



.down:
	mov	si,point_to_point_def	;значение указателя
	mov	ax,point_to_eframe	;указатель на последний элемент
	sub	si,2
	cmp	si,ax
	jb	@f

	mov	point_to_point_def,si
	mov	ax,[si]
	mov	point_default,ax
       	jmp	.show_active_cursor

@@:	call	find_next_sect
        jmp	.show_all_scr

.pgup:
        mov	cx,size_show_section
@@:
	push	cx
        call	find_before_sect
	pop	cx
	loop	@b
        jmp	.show_all_scr


.pgdown:
        mov	cx,size_show_section
@@:
	push	cx
        call	find_next_sect
	pop	cx
	loop	@b
        jmp	.show_all_scr

.home:
	xor	di,di
	call	find_next_sect.h	
        jmp	.show_all_scr

.end:
	mov	di,save_cx
	call	find_before_sect.e
	jmp	.show_all_scr




; тут мы будем парсить только дефолтную секцию и выполнять ее ничего не предлагая пользователю из диалогов. 
.parse_run_only:
if DEBUG
	pusha
	mov	si,no_show_only_w
	call	printplain
	popa
end if


.end_show_all:
}















;show black screen SL
macro show_bl_sc
{
;;;;;;;;;;;;;;;
;очистим экран и выведем меню 
; draw frames
	xor	ax,ax
if DEBUG
	mov	ax,0x0720
end if
        push    0xb800
        pop     es
        xor     di, di
; draw top
        mov     cx, 25 * 80
	rep     stosw
;;;;;;;;;;;;;;;;;;;;;;; show 'Secondary Loader v0.xxx'
	mov	di,164
	mov	si,version
	mov	cx,version_end-version
	mov	ah,color_sym_yellow
@@:
	lodsb
	stosw
	loop @b	
;;;;;;;;;;;;;;;;;;;;;;; show firm ))
	mov	di,286
	mov	ah,color_sym_pink;color_sym_red
	mov	al,'K'
	stosw
	mov	al,' '
	stosw
	mov	ah,color_sym_lightgray;color_sym_lightblue;color_sym_pink
	mov	si,soft_mes
	mov	cx,soft_mes_end- soft_mes
@@:
	lodsb
	stosw
	loop @b	
;;;;;;;;;;;;;;;;;;;;;;; show '__________________________'
	mov	di,480
	mov	ah,color_sym_yellow
	mov	al,'Д'
	mov	cx,61
	rep	stosw
;;;;;;;;;;;;;;;;;;;;;;; show 'Select section'
	mov	di,804
	mov	si,select_section
	mov	cx,select_section_end - select_section
	mov	ah,color_sym_lightgray
@@:
	lodsb
	stosw
	loop @b	
;;;;;;;;;;;;;;;;;;;;;;; show 'Section description'
	mov	di,880
	mov	si,section_description
	mov	cx,section_description_end - section_description
;	mov	ah,color_sym_lightgray
@@:
	lodsb
	stosw
	loop @b	

}

macro show_timer_message
{
;;;;;;;;;;;;;;;;;;;;; show Press any key
;;;;;;;;;;;;;;;;;;;;; show ramk

	xor	ax,ax
	mov	di,3360
	mov	cx,80*4
	rep	stosw

        mov     di,3362     
        mov     ah,color_sym_pink
	mov	al,0xDA
	stosw
	mov	al,0xc4
	mov	cx,76
	rep	stosw
	mov	al,0xBF
	stosw
	add	di,4
	mov	al,0xb3
	stosw
	add	di,152
	stosw
	add	di,4
	stosw
	add	di,152
	stosw
	add	di,4
	mov	al,0xc0
	stosw
	mov	al,0xc4
	mov	cx,76
	rep	stosw
	mov	al,0xd9
	stosw
;;;;;;;;;;;;;;;;;;;;;;;;ramk is complete show
;show first message
	mov	si,start_msg
	mov	cx,start_msg_e-start_msg
	mov	di,3526
@@:
	lodsb
	stosw
	loop @b	
;;;;;;;;;;;;;;;;;;;; show press Enter to....
	add	di,44
	mov	si,time_msg
	mov 	cx,time_msg_e-time_msg
@@:
	lodsb
	stosw
	loop @b	
}








macro get_frame_buffer
{
	mov	cx,save_cx	;it's placed size of ini file
        les     di,dword [file_data]

        mov	si,di	;point frame
	mov	bx,cx
        mov	dx,size_show_section
;	mov	point_to_hframe,di	; внесем значение, так подстраховка не более

	mov	al,byte [es:di]
	push	word .first_ret_bl_sc
	cmp	al,' '
	jz	.first_bl_sc
	jmp	get_firs_sym.not_space
.first_bl_sc:
        jmp	get_firs_sym.first_sp

.start_hbl:              
	call	get_firs_sym	;get first symbol on new line
	test	cx,cx
	jz      error.correct_exit_bl	;critical error not found default point it's not possible because it's param chacking before
        cmp	al,'['
        jnz	.start_hbl

        mov	si,di    ;point frame
	mov	bx,cx
        mov	dx,size_show_section
        jmp	.analisist_al


.start_bl:
	call	get_firs_sym	;get first symbol on new line
.first_ret_bl_sc:                     ;первый возврат
	test	cx,cx
	jz      error.correct_exit_bl	;critical error not found default point it's not possible because it's param chacking before
.analisist_al:
        cmp	al,'['
	jnz	.start_bl
;просматриваем ini файл с начала в поисках секции указаной как default 
;поиск фрейма в котором содержиться значение default
.found_sect_bl:
	cmp	di,point_loader
	jz	.start_bl
	cmp	di,point_default
	jz	.save_point_def

	dec	dx
	jnz	.start_bl

	jmp	.start_hbl


.save_point_def:
;итак далее мы должны заполнить frame буфер адресов секций, что бы потом по нему быстро перемещаться не вычисляя снова адреса
        mov	di,si	;указатель на начало 
	mov	cx,bx
	lea	si,point_to_hframe
	mov	dx,size_show_section+1   ;т.к. у нас структура содержит размер между первым и вторым указателем, то нам нужно на 1 адрес больше обсчитать секций.
;переходим на обработку значения указателя
	mov	al,byte [es:di]
	push	word .first_ret_mfb
	cmp	al,' '
	jz	.first_bl_mbf
	jmp	get_firs_sym.not_space
.first_bl_mbf:
        jmp	get_firs_sym.first_sp

.start_mfb:
	call	get_firs_sym	;get first symbol on new line
.first_ret_mfb:                     ;первый возврат
        jcxz	.val_buff_comp	;.end_loader     ;found or not found parametrs in section  exit in section
        cmp	al,'['
	jnz	.start_mfb

.found_sect_mfb:
	cmp	di,point_loader     ;if we have section loader
	jz	.start_mfb

        mov	[si],di

	sub	si,2
	dec	dx
	jnz	.start_mfb
;bufer is full
        jmp	@f
.val_buff_comp:
	push	save_cx
	pop	word [si]
	sub	si,2
@@:

	add	si,4
	mov	point_to_eframe,si

}

macro show_act_cursor
{
;отображение курсора по умолчанию
	lea	si,point_to_hframe
	mov	di,962-160
	mov	ax,point_default
	mov	cx,size_show_section
.home_show_cur:
        mov	bx,[si]
	add	di,160
	cmp	bx,ax
	jz	.show_cursor_activ
      	sub	si,2
	loop    .home_show_cur

.show_cursor_activ:
;        push	0xb800
;	pop	es
	mov	point_to_point_def,si
        mov	ax,(color_sym_red*0x100+0x10)
	stosw
	add	di,68
	inc	ax
	stosw
}

macro clear_timer_msg
{
	push	0xb800
	pop	es
	xor	ax,ax
if DEBUG
	mov	ax,0x0720
end if
;;;;;;;;;;;;;;;;;;;;; show Press any key
	mov	di,3360
	mov	cx,80*4
	rep	stosw

;show sect
	push	ini_data_
	pop	es
	call	show_bl_sc_sect	;es=0xb800

}

macro show_descript
;Этот макрос показывает краткое описание, если оно есть у секции в пункте 
;Section description
{
local .start_p_sh_d
local .exit
local .rest_value_loop_sh_d
local .end_sh_desc_sec
local .loop_message
local .show_mess_prev_eq
	mov	di,point_default
	push	ini_data_
	mov	si,point_to_point_def
	pop	es
	sub	si,2
	mov	cx,[si]		;загрузим указатель наследующию секцию
	sub	cx,di		;вот теперь имеем истиный размер
;di - указатель на дефолтную секцию т.е. выбранную cx - размер области. для просмотра

.start_p_sh_d:
	call	get_firs_sym	;get first symbol on new line
	test	cx,cx
	jz      .exit	;нету? ну ладно - следующее значение тогда ) 
        cmp	al,'d'
        jnz	.start_p_sh_d
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        mov     bx,cx
        mov     ax,di

        mov     si,parse_descript
        mov     cx,parse_descript_e - parse_descript
        repe    cmpsb
	jnz	.rest_value_loop_sh_d	   ;is not compare

        sub     bx,parse_descript_e - parse_descript ;correct cx
        add     bx,cx
        mov     cx,bx
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; разбор аля ' = '
	mov	ax,0x3d20	   ;cut al=' ' ah='='
	repe    scasb 
	jcxz	.rest_value_loop_sh_d          ;not found param timeout
        
        cmp	ah,byte [es:di-1]    ;find '='
	jnz	.rest_value_loop_sh_d
	
	repe    scasb              ;cut ' '
	inc	cx
	dec	di
;;;;;;;;;;;;;;;;;;;;di указывает на строчку, которую нам нужно выводить.
;строчка будет выводиться блоками по 37 символов.
;настроим куда будем выводить т.е. начало
;es:di - указывают на строчку из которой мы берем символ, ds:si куда будем выводить
	push	di
	pop	si

	push	es
	pop	ds

	push	0xb800
	pop	es
	
	mov	di,1040
	mov	bx,18
	mov	find_sec_di,di
	mov	save_cx_d,bx
;;;;;;;;;;;;;;;;;;;;;;;;;;	
;clean string

	push	di
	xor	ax,ax

@@:	mov	cx,38
	push	di
	rep	stosw
	pop	di

	cmp	save_descript_size,bx
	jz	@f


	add	di,160
	dec	bx
	jnz	@b

@@:     pop	di
;enter in mess
.show_mess_prev_eq:
	lodsb
	mov	ah,color_sym_lettuce;color_sym_turquoise
;	sub 	di,2
	cmp	al,'"'
	jz	.loop_message
	cmp	al,"'"
       	jnz	.end_sh_desc_sec

.loop_message:        
	mov 	cx,38
@@:
	lodsb
	cmp	al,'"'
	jz	.end_sh_desc_sec
	cmp	al,"'"
      	jz	.end_sh_desc_sec
	stosw
	loop	@b
	
	add	find_sec_di,160
	mov	di,find_sec_di
	dec	save_cx_d
	cmp	save_cx_d,0
	jnz	.loop_message

.end_sh_desc_sec:
	push	save_cx_d
        pop	save_descript_size

	push	cs
	pop	ds
	jmp	.exit


.rest_value_loop_sh_d:
        mov	di,ax
	mov	cx,bx
	jmp	.start_p_sh_d

.exit:
}