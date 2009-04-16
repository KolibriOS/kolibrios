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

;блок макросов по обработке секции [loader]
;входные данные:
;es:di - указатель на секцию начинающиюся с '[' встечающиюся после 0хa
;cx - счетчик кол-во байт для проверке в кадре
;
macro use_parse_loader
{
.parse_loader:
;//////////////////
;/ parse  [loader]
;//////////////////
        mov     bx,cx 	;cохраним в регистры значения счетчика и указателя
        mov     ax,di

;	mov	word [bp-4],.start  ;is alredy set, see up
        mov     si,parse_loader
        mov     cx,parse_loader_e - parse_loader
        repe    cmpsb
        jnz     error.rest_value        ;цепочка не совпала :( перейдем далее  т.е. будем снова искать)) 

	;сохраним указательна loader, что бы потом больше его не искать
	mov	point_loader,ax
        sub     bx,parse_loader_e - parse_loader ;correct cx
        add     bx,cx
        mov     cx,bx

if DEBUG        
        pusha
        mov     si,lm_l_found
        call    printplain
        popa
end if  
;/////////////////end check [loader]. [loader] is found
;parsing section [loader]
;first found end section,let's found '[' -it's start next section
;in previosly steep bx =cx we are not need save cx, save only di - point 
	mov	dx,di
@@:
        call	get_firs_sym
        jcxz	.loader_f_end ;.end_loader         ; end даже если мы не нашли секцию предположим что секция [loader] стоит в конце
	cmp	al,'['
	jnz	@b

.loader_f_end:
	sub	bx,cx	;bx = n byte presend in section [loader]
	mov	di,dx	;restore di 
;////////////////parse parametrs in section [loader]
;//timeout=5
;//default=main
;        mov	di,dx	;set pointer on section [loader]   i think it's not need
	mov	cx,bx	;set counter for parsing section [loader] cx= кол-ву символов в секции [loader]
	mov	ret_on_ch,.get_next_str	; return point 
;;;;;;; parse timeout & default
.get_next_str:
	call	get_firs_sym	;get first symbol on new line

	test	cx,cx
	jz	.end_loader
;        jcxz	.end_loader	;завершение парсинга значений timeout & default 
        cmp	al,'t'
	jz	.loader_timeout
	cmp	al,'d'
	jnz     .get_next_str
;//////[loader].default 
;input di point to data cx=size [loader]
        mov     bx,cx
        mov     ax,di

        mov     si,parse_l_default
        mov     cx,parse_l_default_e - parse_l_default
        repe    cmpsb

	jnz	error.rest_value      ;is not compare цепочка не совпала

        sub     bx,parse_l_default_e - parse_l_default ;correct cx
        add     bx,cx
        mov     cx,bx

	test	status_flag,flag_found_default
	jz	.correct_is_not_set          

	mov	si,found_equal_default                          ;мы нашли что флаг уже установлен, информируем
	call	printplain
	jmp	.get_next_str

.correct_is_not_set:
	mov	ax,0x3d20	   ;cut al=' ' ah='='
	repe    scasb 
	test	cx,cx
	jz	.end_loader
        
        cmp	ah,byte [es:di-1]    ;find '='
	jnz	.get_next_str
	
	repe    scasb              ;cut ' '
	inc	cx
	dec	di
;сейчас es:di указывают на название секции, имя секции по дефолту не должно быть loader т.е. иначе возможно зацикливание
;установим указатель si на это значение и сначала проверим

;получение длинны секции
; cx=bx содержит длинну остатка секции
; di=ax указатель на текущию секцию
        mov     bx,cx
        mov     dx,di

@@:	mov	al,byte [es:di]
	inc	di
	dec 	cx
	test	cx,cx
	jz	error.error_get_size_d_sect  ;переход на обработку ошибки по нахождению длины дефолтной секции
	cmp	al,' '
	jz      @b
	cmp	al,0xd
	jz	.found_size_d_sect
	cmp	al,0xa
	jnz	@b
.found_size_d_sect:
;
	inc	cx	;correct cx
	mov	ax,bx
	sub	bx,cx	; в bx длина секции которая определена по дефолту
	mov	save_cx_d,bx
	mov	di,dx

	mov	cx,bx	;set size default section
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;проверка на =loader
;save in reg point and счетчик
;check on loader 
        mov     bx,ax
        mov     ax,dx

        mov     si,parse_loader
	inc	si	;set only loader and 6 char in counter
        repe    cmpsb
        jnz     .check_section        ;цепочка не совпала :( перейдем далее )) значит не исключение

	jmp	error.default_eq_loader    ;error критическая ошибка т.е. в дефолте присутствует имя [loader]

.check_section:		;поиск соответствующей секции нам нужно будет узнать адрес этой секции
        mov	cx,bx
	mov	di,ax

;/////////////////////////////
;	mov	ret_on_ch,.start_d     ;set return 
	mov	si,di   ;установим указатель на нашу секцию, которая по дефолту

	push	di      ;save point di

	push 	cx	;save cx
;установим указатель es:di на начало ini файла
	mov	cx,save_cx	;it's placed size of ini file
        les     di,dword [file_data]


	mov	al,byte [es:di]
	push	word .first_ret_d
	cmp	al,' '
	jz	.first_sp_1_d
	jmp	get_firs_sym.not_space
.first_sp_1_d:
        jmp	get_firs_sym.first_sp

.start_d:
	call	get_firs_sym	;get first symbol on new line
.first_ret_d:                     ;первый возврат
        jcxz	.correct_exit	;.end_loader     ;found or not found parametrs in section  exit in section
        cmp	al,'['
        jz	.found_sect_d
	jmp	.start_d
;просматриваем ini файл с начала в поисках секции указаной как default
;идет проверка на наличее значения timeout, для более быстрой работы, этот параметр должен быть уже обработан,т.е. в этом случае при его =0 будет сформирован указатель только на дефолтную секцию, иначе информация будет собрана по всем секциям и составлены указатели в блоке памяти	
.found_sect_d:

;check on name section
        mov     bx,cx
        mov     ax,di
	push	si	;save point
	
;        mov     si,parse_loader
        mov     cx,save_cx_d ;load size section
	push	es
	pop	ds	

	inc	di
        repe    cmpsb
        push	cs
	pop	ds
	pop	si
 
        jnz     .not_compare_d_s        ;цепочка не совпала :( перейдем далее )) значит не исключение
	cmp	byte[es:di],']'
	jnz	.not_compare_d_s	;нет в конце нашей секции завершающего символа :(



;set flag -we have found default -not enter again in this prosedure
	or	status_flag,flag_found_default
	pop	cx
	pop	di
	mov	point_default,ax	;point to [

if DEBUG        
        pusha
        mov     si,lm_lf_default_f
        call    printplain
        popa
end if  
	
	jmp	.get_next_str

.not_compare_d_s:

        mov     cx,bx
        mov     di,ax
	jmp	.start_d	

.correct_exit:
        pop	cx ;восстановим значение счетчика
        pop	di


if DEBUG        
        pusha
        mov     si,lm_lf_default
        call    printplain
        popa
end if  
	jmp	.get_next_str

;//////////[loader].timeout
.loader_timeout:
        mov     bx,cx
        mov     ax,di

        mov     si,parse_l_timeout
        mov     cx,parse_l_timeout_e - parse_l_timeout
        repe    cmpsb
	jnz	error.rest_value	   ;is not compare

        sub     bx,parse_l_timeout_e - parse_l_timeout ;correct cx
        add     bx,cx
        mov     cx,bx

	test	status_flag,flag_found_timeout
	jz	.correct_is_not_set_t          

	mov	si,found_equal_timeout                          ;мы нашли что флаг уже установлен, информируем
	call	printplain
	jmp	.get_next_str

.correct_is_not_set_t:
	mov	ax,0x3d20	   ;cut al=' ' ah='='
	repe    scasb 
	jcxz	.timeout_sec_end_d          ;not found param timeout
        
        cmp	ah,byte [es:di-1]    ;find '='
	jnz	.get_next_str
	
	repe    scasb              ;cut ' '
	inc	cx
	dec	di
;get timeout value
;2 знакa может быть обработано т.е. значение от 0 до 99 секунд
	push	cx
	xor	bx,bx
        mov	cx,2
@@:	mov	al,byte [es:di]
        cmp	al,'0'
	jb	.end_get_val_t
	cmp	al,'9'
	ja	.end_get_val_t
	imul	bx,10
	xor 	al,0x30
        add	bl,al
.end_get_val_t:
	inc	di
        loop	@b
	mov	word [value_timeout],bx
;	pop 	cx

if DEBUG        
        pusha
        mov     si,lm_lf_timeout
        call    printplain
        popa
end if  

	jmp	@f
.timeout_sec_end_d:
	mov	word [value_timeout],default_timeout_value
	mov	si,set_default_timeout_val
	call	printplain
@@:	pop	cx
	jmp	.get_next_str

;///////here end block loader
.end_loader:
if DEBUG        
        pusha
        mov     si,lm_l_end
        call    printplain
        popa
end if  

}