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

; тут описываются процедуры которые используются в secondary loader
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
color_sym_white	equ	15
if DEBUG
decode:
;input eax - число, es:di куда писать, cx=10
        cmp     eax,ecx
        jb      @f
        xor     edx,edx
        div     ecx
        push    edx
        call    decode
        pop     eax
    @@: or      al,0x30
        mov     [ds:di],al
        inc     di
        ret

end if


putchar:
; in: al=character
        mov     ah, 0Eh
        mov     bh, 0
        int     10h
        ret

printplain:
; in: si->string
        pushad
        lodsb
@@:
        call    putchar
        lodsb
        test    al,al
        jnz     @b
        mov     al,13 
        call    putchar

        mov     al,10 
        call    putchar
        popad
        ret
getkey:
; get number in range [bl,bh] (bl,bh in ['0'..'9'])
; in: bx=range
; out: ax=digit (1..9, 10 for 0)
        mov     ah, 0
        int     16h
        cmp     al, bl
        jb      getkey
        cmp     al, bh
        ja      getkey
        push    ax
        call    putchar
        pop     ax
        and     ax, 0Fh
        jnz     @f
        mov     al, 10
@@:
        ret

;setcursor:
; in: dl=column, dh=row
;        mov     ah, 2
;        mov     bh, 0
;        int     10h
;        ret

;macro _setcursor row,column
;{
;        mov     dx, row*256 + column
;        call    setcursor
;}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
get_firs_sym:
.start:
	mov	al,byte [es:di]
	inc	di
	dec	cx
	jcxz	.exit

	cmp	al,0xa		;cmp al,0xa
	jz      ._entry

        cmp	al,';'
	jnz     .start

.first_com:

	mov	al,0xa
        repnz   scasb 
        jcxz    .exit
._entry:
	mov	al,byte [es:di]
.first_sp:
	cmp	al,' '
	jnz     .not_space

;	mov	al,' '		;cut ' ' 
	repe    scasb 
	dec	di
	inc	cx
	mov	al,byte [es:di]
.not_space:
        cmp	al,';'
	jz	.first_com
.exit:	ret	
;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
show_name_section:
	push	si
       	push	ini_data_
	pop	es


	mov	al,']'
	repnz   scasb 
	test	cx,cx
	jz      error.incorect_section_def
.find_val_name_fb1:
	mov	al,'n'
.find_val_name_fb:
	repnz	scasb
	jcxz	.not_name_sec_fb

	mov	si,parse_name

	push	cx
	push	di

	mov	cx,parse_name_e -parse_name
	repe	cmpsb
	pop	di
	pop	cx
	jz	.yaaa_find_value	


	jmp	.find_val_name_fb

.yaaa_find_value:
        sub	cx,parse_name_e -parse_name 
	add	di,parse_name_e -parse_name 

	mov	ax,0x3d20	   ; ah='='
	repe    scasb 
	test	cx,cx
	jz	.not_name_sec_fb

        cmp	ah,byte [es:di-1]    ;find '='
	jnz	.find_val_name_fb1
	
	repe    scasb              ;cut ' '
	inc	cx
	dec	di


;все вырезали и все готово для вывода имени секции ))
	push	es
	pop	ds

.def_sect_name:
        push	0xb800
	pop	es
;clear array for message
	xor	ax,ax
if DEBUG
	mov	ax,0x0720
end if

	mov	cx,39
	mov	si,di
        mov 	di,dx
        sub	di,2
        rep	stosw
;//////////////////////


        mov	di,dx
	mov	ah,color_sym_white;color_sym_lightblue
	mov 	cx,36
	lodsb
	sub 	di,2
	cmp	al,'"'
	jz	@f
	cmp	al,"'"
       	jnz	.end_sh_name_sec
@@:	lodsb
@@:
	stosw
	lodsb
	cmp	al,'"'
	jz	.end_sh_name_sec
	cmp	al,"'"
      	jz	.end_sh_name_sec
	loop	@b
	
	mov	al,'}'
	mov	ah,color_sym_yellow
	stosw
.end_sh_name_sec:
	push	cs
	pop	ds

	pop	si
	ret

.not_name_sec_fb:    ;нет имени в названии секции - значит так и скажем об этом 
	push	cs
	pop	ds
        mov	di,default_section_name
        jmp	.def_sect_name

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;процедура поиска вверх следующей секции
;в point_default содержиться указатель на дефаулт секцию, и в пределах врейма мы бегаем по заранее пропарсеными значениям указателей
;для того что бы отобразить и пропарсить следующий фрейм, нам нужно получить за пердыдущий или следующий указатель
find_before_sect:
	mov	di,point_default
.e:
	push	ini_data_
	pop	es
	mov	cx,di	;предположим будем просматривать к началу, текущая позиция di = сколько символов от начала документа имеется
	mov	bx,cx	;копия

;настроили указатель на дефаулт секцию
;будем искать вверх 
.find_start_section:
        std	;установка флага направления - будем просматирвать к началу нашего ини файла
;будем искать начало секции т.е. '[' этот символ
	mov	al,0xa
        repnz	scasb	;просканируем на наличее символа начала секции
	jcxz	.go_    ;мы просмотрели до начала файла, но так и ничего не нашли ;(( по тихому выйдем ) 

       	mov	find_sec_di,di	;сохраним данные
	mov	cx,di  ;

	sub	bx,cx
	mov	cx,bx	;в сx значение - кол-во символов
	cld
	call	get_firs_sym
.ret_go:
	jcxz	._not_section	; в данном случае имеем конструкцию 0xa ... ; hello [секция] обломс ищем далее

        cmp	di,point_loader ; секцию loader мы не заносим иначе крах 
	jz	._not_section
;все удачно мы нашли вхождение секции предыдущей
	cmp	al,'['
	jnz	._not_section
	mov	point_default,di
.exit_scan_sect:
	ret
;;;;;;;; восстановим значения и продолжим поиски начала секции которая нас устроит ))
._not_section:
	mov	di,find_sec_di
	mov	cx,di
	mov	bx,cx
	jmp	.find_start_section
.go_:
	cld
	mov	cx,bx	;в сx значение - кол-во символов

	mov	al,byte [es:di]
	push	word .f_go
	cmp	al,' '
	jz	@f
	jmp	get_firs_sym.not_space
@@:
        jmp	get_firs_sym.first_sp

.f_go:
	jcxz	.exit_scan_sect	; в данном случае имеем конструкцию 0xa ... ; hello [секция] обломс ищем далее

        cmp	di,point_loader ; секцию loader мы не заносим иначе крах 
	jz	.exit_scan_sect
;все удачно мы нашли вхождение секции предыдущей
	cmp	al,'['
	jnz	.exit_scan_sect
	mov	point_default,di
	ret







;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
find_next_sect:
	mov	di,point_default
	push	ini_data_
	pop	es
	mov	cx,save_cx;di	;предположим будем просматривать к концу, текущая позиция di = сколько символов от начала документа имеется
	sub	cx,di	;сейчас в cx остаток т.е.  сколько можно крутить до конца и не вылазить на начало
	jmp	.let_s_go
.h:
	push	ini_data_
	pop	es
	mov	cx,save_cx;di	;предположим будем просматривать к концу, текущая позиция di = сколько символов от начала документа имеется
;	sub	cx,di	;сейчас в cx остаток т.е.  сколько можно крутить до конца и не вылазить на начало

	mov	al,byte [es:di]
	push	word .let_s_go_ret
	cmp	al,' '
	jz	@f
	jmp	get_firs_sym.not_space
@@:
        jmp	get_firs_sym.first_sp




;настроили указатель на дефаулт секцию
;будем искать вниз 
.let_s_go:
	call	get_firs_sym
.let_s_go_ret:
	jcxz	.exit_scan_sect	; в данном случае имеем конструкцию 0xa ... ; hello [секция] обломс ищем далее
	cmp	al,'['
	jnz	.let_s_go
	cmp	di,point_loader
	jz	.let_s_go
;все удачно мы нашли вхождение секции предыдущей
	mov	point_default,di
.exit_scan_sect:
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;
;clean old cursor
clean_active_cursor:
;не изменяет значение ax
;отображение курсора по умолчанию
	lea	si,point_to_hframe
	mov	di,962-160
	mov	dx,point_default
	mov	cx,18
.clean_show_cur:
        mov	bx,[si]
	add	di,160
	cmp	bx,dx
	jz	.clean_cursor_
       	sub	si,2
	loop    .clean_show_cur

;	jmp $

.clean_cursor_:
        push	0xb800
	pop	es
	push	ax
	mov	point_to_point_def,si
        xor	ax,ax
if DEBUG
	mov	ax,0x0720
end if
	stosw
	add	di,68
	stosw
	pop	ax
	ret	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;установка таймера и отображение счетчика времени 
gettime:
        mov     ah,0
        int     1Ah
        xchg    ax, cx
        shl     eax, 10h
        xchg    ax, dx
        ret
newtimer:
	push	ds

        push    cs
        pop     ds

        pushf
        call    far dword [old_timer] 

        pushad
        call    gettime

        sub     eax, dword[start_timer]
	mov	bx, word [value_timeout]
	imul	bx,18
        sub     bx, ax
        jbe     .timergo

	push	es

        push	0xb800
	pop	es
	mov	ax,bx

        mov     bx, 18
        xor     dx, dx
        div     bx

        mov	bx,10
        mov	di,3734
	call	.decode

	xor	ax,ax
	stosw

; wait 5/4/3/2 seconds, 1 second
	pop	es
        popad
	pop	ds

        iret
.timergo:
        push    0
        pop     es
        mov     eax,dword [old_timer] 
        mov     [es:8*4], eax
	mov	dword [timer_],eax
        mov	sp, word [start_stack]  
	mov	bp,word [save_bp_from_timer]
;;не восстановленый стек :( 
	sti
        jmp	parse_start.parse_run_only


.decode:
;input ax - число, es:di куда писать, bx=10
        cmp     ax,bx
        jb      @f
        xor     dx,dx
        div     bx
        push    dx
        call    .decode
        pop     ax
    @@: or      al,0x30
	push	ax
	mov	ah,9
        stosw
	pop	ax
        ret

show_bl_sc_sect:
;1) отображение списка секций. Если секция не имет имя - ошибка - вывод Section unname
;проверка на наличее имени.
;входные данные es:di -указатель на секцию - cx размер секции
;	push	bp
	mov	bx,point_to_eframe
	lea	si,point_to_hframe
	mov	dx,966

.home_show_fb:
	cmp	si,bx
	jb	._show_space_fb
	mov	di,[si]
	sub	si,2
	mov	cx,[si]
	sub	cx,di		;home first section it's end before section 
        call	show_name_section
	add	dx,160
	jmp	.home_show_fb
._show_space_fb:
	sub	dx,4
        push	0xb800
	pop	es
@@:
        cmp	dx,0xE64
	ja	.exit_show_fb
	mov	di,dx
;clear array for message
	xor	ax,ax
if DEBUG
	mov	ax,0x0720
end if
	mov	cx,39
        rep	stosw
;//////////////////////

	add	dx,160	
	jmp	@b
.exit_show_fb:
;        pop	bp
	ret

