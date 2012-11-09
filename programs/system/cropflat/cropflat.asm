;*****************************************************************************
; CROPFLAT - set limits of screen - for Kolibri OS
; Copyright (c) 2012, Marat Zakiyanov aka Mario79, aka Mario
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;        * Redistributions of source code must retain the above copyright
;          notice, this list of conditions and the following disclaimer.
;        * Redistributions in binary form must reproduce the above copyright
;          notice, this list of conditions and the following disclaimer in the
;          documentation and/or other materials provided with the distribution.
;        * Neither the name of the <organization> nor the
;          names of its contributors may be used to endorse or promote products
;          derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Marat Zakiyanov ''AS IS'' AND ANY
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
;
; Example run with parameters:
; CROPFLAT XS800 YS480
;
;------------------------------------------------------------------------------
	use32
	org 0x0

	db 'MENUET01'
	dd 0x01
	dd START
	dd IM_END
	dd I_END
	dd stack_top
	dd boot_param
	dd 0x0

include '../../macros.inc'
;include '../../debug.inc'
;------------------------------------------------------------------------------
START:
	mcall	14
	mov	ebx,eax
	shr	eax,16
	inc	eax
	mov	[x_size],eax

;	dps	"CROPFLAT current X size: "
;	dpd	eax
;	newline

	and	ebx,0xffff
	inc	ebx
	mov	[y_size],ebx

;	dps	"CROPFLAT current Y size: "
;	dpd	ebx
;	newline
;------------------------------------------------------------------------------
	mov	bx,word 'XS'
	call	find_value
	test	eax,eax
	jnz	.y
	
	call	convert_ASCII_to_BIN
	test	ebx,ebx
	jz	@f
	
	mov	[x_size],ebx
;--------------------------------------
@@:
;	dps	"CROPFLAT new X size: "
;	dpd	ebx
;	newline
;------------------------------------------------------------------------------
.y:
	mov	bx,word 'YS'
	call	find_value
	test	eax,eax
	jnz	.set
	
	call	convert_ASCII_to_BIN
	test	ebx,ebx
	jz	@f

	mov	[y_size],ebx
;--------------------------------------
@@:
;	dps	"CROPFLAT new Y size: "
;	dpd	ebx
;	newline
;------------------------------------------------------------------------------
.set:
	mcall	18,24,[x_size],[y_size]
;--------------------------------------
.exit:
	mcall	-1
;------------------------------------------------------------------------------	
find_value:
; in:
; bx - word
; out:
; eax - 0 for valid value, -1 for invalid value
; esi - ASCII value
	mov	esi,boot_param
	mov	ecx,254
	cld
;--------------------------------------
.loop:
	lodsw
	cmp	ax,bx
	je	@f

	dec	esi
	loop	.loop

	mov	eax,-1
	ret
;--------------------------------------
@@:
	xor	eax,eax
	ret
;------------------------------------------------------------------------------
convert_ASCII_to_BIN:
; in:
; esi - ASCII value
; out:
; ebx - BIN value
	mov	ecx,4
	xor	ebx,ebx
	cld
;--------------------------------------
.loop:
	lodsb
	cmp	al,0x30	; 0
	jb	@f
	
	cmp	al,0x39	; 9
	ja	@f
	
	sub	al,0x30
; multiply by 10
	lea	ebx,[ebx+ebx*4] ; multiply by 5
	shl	ebx,1		; multiply by 2
	movzx	eax,al
	add	ebx,eax
	loop	.loop
;--------------------------------------
@@:
	ret
;------------------------------------------------------------------------------
IM_END:
;------------------------------------------------------------------------------
align 4
x_size	rd 1
y_size	rd 1
;------------------------------------------------------------------------------
align 4
boot_param:
	rb 256+1 ; +1 for reserve
;------------------------------------------------------------------------------
align 4
	rb 512
stack_top:
;------------------------------------------------------------------------------
I_END:
;------------------------------------------------------------------------------
