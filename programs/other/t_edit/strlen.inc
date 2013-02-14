; strlen function
;
; Copyright (c) 2003 Thomas Mathys
; killer@vantage.ch
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;
;%ifndef _STRLEN_INC
;%define _STRLEN_INC


;********************************************************************
;       returns the length of an asciiz string
;       input           :       esi = pointer to string
;       output          :       eax = string length
;       destroys        :       nothing
;********************************************************************
strlen:
	push	ecx edi
	pushfd
	cld				; !
	mov	ecx,-1
	mov	edi,esi 		; find terminating zero
	xor	al,al
	repne	scasb
	mov	eax,edi 		; calculate string length
	sub	eax,esi
	dec	eax
	popfd
	pop	edi ecx
	ret



; linlen function
;
; Copyright (c) 2009 Igor Afanasiev

linlen:
	push	ecx edi
	pushfd
	cld
	mov	ecx,eax
	inc ecx
	mov	edi,esi 		; find terminating zero
	mov	al,13
	repne	scasb
	mov	eax,edi 		; calculate string length
	sub	eax,esi
	dec	eax
	popfd
	pop	edi ecx
	ret

;description:
; проверяет содержится ли строка str1 в строке str0
; проверка делается только начиная с первых символов, указанных в str0 и str1
; пример 1: если str0='aaabbbccc', str1='bbb' совпадения не будет
; пример 2: если str0='aaabbbccc', str1='aaa' совпадение будет
;output:
; al = 0 если строка str1 содержится в str0
; al != 0 если строка str1 не содержится в str0
align 4
proc str_instr uses edi esi, str0:dword, str1:dword
	;xor eax,eax
	mov edi,[str0]
	mov esi,[str1]
	cld
	@@:
		mov al,[esi]
		cmp al,0
		je .e1
		inc esi
		scasb ;сравниваем символы
	jz @b ;если совпали, то переходим к сравнению следующих
	;сюда попадаем если строки не совпали
	sub al,[edi-1]
	.e1: ;сюда попадаем если строка str1 (esi) закончилась
	ret
endp

