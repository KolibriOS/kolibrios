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
	push	ecx
	push	edi
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
	pop	edi
	pop	ecx
	ret



; linlen function
;
; Copyright (c) 2009 Igor Afanasiev

linlen:
	push	ecx
	push	edi
	pushfd
	cld				; !
	mov	ecx,eax
 inc ecx
	mov	edi,esi 		; find terminating zero
	mov	al,13
	repne	scasb
	mov	eax,edi 		; calculate string length
	sub	eax,esi
	dec	eax
	popfd
	pop	edi
	pop	ecx
	ret
;%endif

