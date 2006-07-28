; string2dword - a useless string to double word conversion routine
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


;********************************************************************
;	converts an asciiz string into an unsigned doubleword.
;	(base 10 is assumed)
;
;	-	first, leading whitespaces are skipped
;	-	then the function converts the string, until it
;		finds the terminating zero, another character it
;		cannot convert or the number becomes too large.
;
;	input		:	esi = pointer to string
;	output		:	eax = unsigned doubleword
;				the function tries to convert as
;				many digits as possible, before it
;				stops. if the value of the dword
;				becomes too large, 0xffffffff is
;				returned.
;	destroys	:	nothing
;********************************************************************
string2dword:
	push	ebx
	push	ecx
	push	edx
	push	esi
	pushfd

	xor	ebx,ebx			; ebx : dword

	; skip leading whitespaces
.skipspaces:
	lodsb
	cmp	al,32			; space
	je	.skipspaces
	cmp	al,12			; ff
	je	.skipspaces
	cmp	al,10			; lf
	je	.skipspaces
	cmp	al,13			; cr
	je	.skipspaces
	cmp	al,9			; ht
	je	.skipspaces
	cmp	al,11			; vt
	je	.skipspaces

	; convert string
	dec	esi			; esi -> 1st non-whitespace
.convert:
	xor	eax,eax			; get character
	lodsb
	sub	al,'0'			; convert to digit
	cmp	al,9			; is digit in range [0,9] ?
	ja	.done			; nope -> stop conversion
	mov	ecx,eax			; save new digit
	mov	eax,10			; dword = dword * 10
	mul	ebx
	jc	.overflow
	add	eax,ecx			; + new digit
	jc	.overflow
	mov	ebx,eax
	jmp	.convert

.overflow:
	mov	ebx,0xffffffff
.done:
	mov	eax,ebx
	popfd
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	ret

