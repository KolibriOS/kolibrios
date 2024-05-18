;**************************************************************
; Path Show Macro for KolibriOS
; Copyright (c) 2010, Marat Zakiyanov aka Mario79, aka Mario
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;     * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;     * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation and/or other materials provided with the distribution.
;     * Neither the name of the <organization> nor the
;       names of its contributors may be used to endorse or promote products
;       derived from this software without specific prior written permission.
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
macro path_show_start
{
	pusha
	mov	edi,dword [esp+36]
}
;*****************************************************************************
macro path_show_exit
{
popa
ret 4
}
;*****************************************************************************
align 16
path_show: 
ps_type			equ [edi]	;dword
ps_start_y		equ [edi+4]	;word
ps_start_x		equ [edi+6]	;word
ps_font_size_x		equ [edi+8]	;word
ps_area_size_x		equ [edi+10]	;word
ps_font_number		equ [edi+12]	;dword
ps_background_flag	equ [edi+16]	;dword
ps_font_color		equ [edi+20]	;dword
ps_background_color	equ [edi+24]	;dword
ps_text_pointer		equ [edi+28]	;dword
ps_work_area_pointer	equ [edi+32]	;dword
ps_temp_text_length	equ [edi+36]	;dword
;*****************************************************************************
;*****************************************************************************
; draw event
;*****************************************************************************
;*****************************************************************************
.prepare:
path_show_start
;-------------------------------------
	mov	esi,ps_text_pointer
	xor	eax,eax
	xor	ecx,ecx
	dec	ecx
	cld
@@:
	lodsb
	inc	ecx
	test	eax,eax
	jnz	@b
	mov	ps_temp_text_length,ecx
	movzx	eax,word ps_font_size_x
	imul	ecx,eax
	movzx	eax,word ps_area_size_x
	cmp	ecx,eax
	jae	.cut
;-------------------------------------
	mov	esi,ps_text_pointer
	mov	edi,ps_work_area_pointer
	xor	eax,eax
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
	jmp	.exit
;-------------------------------------
.cut:
; copy the first 6 characters of path
	mov	esi,ps_text_pointer
	push	edi
	mov	edi,ps_work_area_pointer
	mov	ecx,6
	rep	movsb
; insert a line break '...'	
	mov	al,byte '.'
	mov	ecx,3
	rep	stosb
	mov	ecx,edi
; calculate the display length, in characters
	pop	edi
	movzx	ebx,word ps_font_size_x
	movzx	eax,word ps_area_size_x
	xor	edx,edx
	div	ebx
	sub	eax,9
; eax - maximum length of display area, the number of characters	
	mov	esi,ps_temp_text_length
	add	esi,ps_text_pointer
	sub	esi,eax
; esi - pointer of the last segment of the displayed text
	mov	edi,ecx
	mov	ecx,eax
	rep	movsb
	xor	eax,eax
	stosb
;-------------------------------------
.exit:
path_show_exit
;*****************************************************************************
;*****************************************************************************
; draw event
;*****************************************************************************
;*****************************************************************************
.draw:
path_show_start
;-------------------------------------
	mov	ebx,ps_start_y
	xor	ecx,ecx
	or	ecx,0x80000000
	mov	eax,ps_background_flag
	and	eax,1b
	shl	eax,30
	add	ecx,eax
	mov	eax,ps_font_number
	and	eax,11b
	shl	eax,28
	add	ecx,eax
	mov	eax,ps_font_color
	and	eax,0xffffff
	add	ecx,eax
	mov	edx,ps_work_area_pointer
	mov	eax,ps_background_color
	and	eax,0xffffff
	xor	esi,esi
	mov	edi,eax
	mcall	SF_DRAW_TEXT
path_show_exit
;*****************************************************************************
