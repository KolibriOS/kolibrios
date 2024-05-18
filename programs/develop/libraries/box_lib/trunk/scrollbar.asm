;**************************************************************
; ScrollBar Macro for KolibriOS
; Copyright (c) 2009-2012, Marat Zakiyanov aka Mario79, aka Mario
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;	 * Redistributions of source code must retain the above copyright
;	 notice, this list of conditions and the following disclaimer.
;	 * Redistributions in binary form must reproduce the above copyright
;	 notice, this list of conditions and the following disclaimer in the
;	 documentation and/or other materials provided with the distribution.
;	 * Neither the name of the <organization> nor the
;	 names of its contributors may be used to endorse or promote products
;	 derived from this software without specific prior written permission.
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
; 15.11.2011 add scroll type 1 by IgorA
;*****************************************************************************
macro scroll_bar_exit
{
popa
ret 4
}
;*****************************************************************************
macro draw_up_arrow_vertical
{
	push	ebx
	
	xor	eax,eax
	mov	ax,sb_size_x
	shr	eax,1
	shl	eax,16
	add	ebx,eax
	
	xor	eax,eax
	mov	ax,sb_btn_high
	shr	eax,1
	shl	eax,16
	add	ecx,eax
	
	mov	edx,sb_line_col
	sub	ebx,4 shl 16
	mov	bx,7
	mov	cx,1
	mcall	SF_DRAW_RECT
	sub	ecx,1 shl 16
	add	ebx,1 shl 16
	mov	bx,5
	int	0x40
	sub	ecx,1 shl 16
	add	ebx,1 shl 16
	mov	bx,3
	int	0x40
	sub	ecx,1 shl 16
	add	ebx,1 shl 16
	mov	bx,1
	int	0x40
	pop	ebx
}
;*****************************************************************************
macro draw_up_arrow_vertical_type2
{
	push	ebx
	
	mov	bx,sb_size_x
	shr	bx,1
	add	bx,sb_start_x

	mov	cx,sb_btn_high
	shr	cx,1
	add	cx,sb_start_y
	
	mov	ax,bx
	shl	eax,16
	mov	ax,cx
	
	sub	cx,2
	
	shl	ebx,16
	shl	ecx,16
	
	mov	cx,ax
	shr	eax,16
	mov	bx,ax
	
	add	cx,1
	sub	bx,3

	mcall	SF_DRAW_LINE,,,sb_line_col

	add	bx,6
	
	mcall
	pop	ebx
}
;*****************************************************************************
macro draw_down_arrow_vertical
{
	push	ebx
	
	xor	eax,eax
	mov	ax,sb_size_x
	shr	eax,1
	shl	eax,16
	add	ebx,eax
	
	xor	eax,eax
	mov	ax,sb_btn_high
	shr	eax,1
	shl	eax,16
	add	ecx,eax
	
	mov	edx,sb_line_col
	sub	ebx,4 shl 16
	mov	bx,7
	sub	ecx,2 shl 16
	mov	cx,1
	mcall	SF_DRAW_RECT
	add	ecx,1 shl 16
	add	ebx,1 shl 16
	mov	bx,5
	int	0x40
	add	ecx,1 shl 16
	add	ebx,1 shl 16
	mov	bx,3
	int	0x40
	add	ecx,1 shl 16
	add	ebx,1 shl 16
	mov	bx,1
	int	0x40
	pop	ebx
}
;*****************************************************************************
macro draw_down_arrow_vertical_type2
{
	push	ebx
	
	mov	bx,sb_size_x
	shr	bx,1
	add	bx,sb_start_x

	mov	ax,sb_btn_high
	shr	ax,1
	mov	cx,sb_start_y
	add	cx,sb_size_y
	sub	cx,ax
	
	mov	ax,bx
	shl	eax,16
	mov	ax,cx
	
	add	cx,1
	
	shl	ebx,16
	shl	ecx,16
	
	mov	cx,ax
	shr	eax,16
	mov	bx,ax
	
	sub	cx,2
	sub	bx,3

	mcall	SF_DRAW_LINE,,,sb_line_col

	add	bx,6
	
	mcall
	pop	ebx
}
;*****************************************************************************
macro draw_runner_center_vertical
{
	push	ebx ecx
	
	xor	eax,eax
	mov	ax,sb_size_x
	shr	eax,1
	shl	eax,16
	add	ebx,eax
	
	mov	edx,sb_run_size
	shr	edx,1
	shl	edx,16
	add	ecx,edx
	mov	edx,sb_line_col
	sub	ecx,8 shl 16
	sub	ebx,5 shl 16
	mov	bx,10
	add	ecx,4 shl 16
	mov	cx,1
	mcall	SF_DRAW_RECT
	add	ecx,3 shl 16
	sub	ebx,1 shl 16
	mov	bx,12
	int	0x40
	add	ebx,1 shl 16
	mov	bx,10
	add	ecx,3 shl 16
	int	0x40
	pop	ecx ebx
}
;*****************************************************************************
macro draw_up_arrow_horizontal
{
	push	ecx
	
	xor	eax,eax
	mov	ax,sb_btn_high
	shr	eax,1
	shl	eax,16
	add	ebx,eax
	
	xor	eax,eax
	mov	ax,sb_size_y
	shr	eax,1
	shl	eax,16
	add	ecx,eax
	
	mov	edx,sb_line_col
	sub	ecx,4 shl 16
	mov	cx,7
	mov	bx,1
	
	mcall	SF_DRAW_RECT
	sub	ebx,1 shl 16
	add	ecx,1 shl 16
	mov	cx,5
	int	0x40
	sub	ebx,1 shl 16
	add	ecx,1 shl 16
	mov	cx,3
	int	0x40
	sub	ebx,1 shl 16
	add	ecx,1 shl 16
	mov	cx,1
	int	0x40
	pop	ecx
}
;*****************************************************************************
macro draw_up_arrow_horizontal_type2
{
	push	ebx ecx
	
	mov	bx,sb_btn_high
	shr	bx,1
	add	bx,sb_start_x

	mov	cx,sb_size_y
	shr	cx,1
	add	cx,sb_start_y
	
	mov	ax,bx
	shl	eax,16
	mov	ax,cx
	
	sub	bx,2
	
	shl	ebx,16
	shl	ecx,16
	
	mov	cx,ax
	shr	eax,16
	mov	bx,ax
	
	add	bx,1
	sub	cx,3

	mcall	SF_DRAW_LINE,,,sb_line_col

	add	cx,6
	
	mcall
	pop	ecx ebx
}
;*****************************************************************************
macro draw_down_arrow_horizontal
{
	push	ecx
	
	xor	eax,eax
	mov	ax,sb_btn_high
	shr	eax,1
	shl	eax,16
	add	ebx,eax
	
	xor	eax,eax
	mov	ax,sb_size_y
	shr	eax,1
	shl	eax,16
	add	ecx,eax
	
	mov	edx,sb_line_col
	sub	ecx,4 shl 16
	mov	cx,7
	sub	ebx,2 shl 16
	mov	bx,1
	mcall	SF_DRAW_RECT
	add	ebx,1 shl 16
	add	ecx,1 shl 16
	mov	cx,5
	int	0x40
	add	ebx,1 shl 16
	add	ecx,1 shl 16
	mov	cx,3
	int	0x40
	add	ecx,1 shl 16
	add	ebx,1 shl 16
	mov	cx,1
	int	0x40
	pop	ecx
}
;*****************************************************************************
macro draw_down_arrow_horizontal_type2
{
	push	ebx ecx
	
	mov	ax,sb_btn_high
	shr	ax,1
	mov	bx,sb_start_x
	add	bx,sb_size_x
	sub	bx,ax
	
	mov	cx,sb_size_y
	shr	cx,1
	add	cx,sb_start_y
	
	mov	ax,bx
	shl	eax,16
	mov	ax,cx
	
	add	cx,1
	
	shl	ebx,16
	shl	ecx,16
	
	mov	cx,ax
	shr	eax,16
	mov	bx,ax
	
	sub	cx,2
	sub	bx,3

	mcall	SF_DRAW_LINE,,,sb_line_col

	add	cx,6
	
	mcall
	pop	ecx ebx
}
;*****************************************************************************
macro draw_runner_center_horizontal
{
	push	ebx ecx
	
	xor	eax,eax
	mov	ax,sb_size_y
	shr	eax,1
	shl	eax,16
	add	ecx,eax
	
	mov	edx,sb_run_size
	shr	edx,1
	shl	edx,16
	add	ebx,edx
	mov	edx,sb_line_col
	sub	ebx,8 shl 16
	sub	ecx,5 shl 16
	mov	cx,10
	add	ebx,4 shl 16
	mov	bx,1
	mcall	SF_DRAW_RECT
	add	ebx,3 shl 16
	sub	ecx,1 shl 16
	mov	cx,12
	int	0x40
	add	ecx,1 shl 16
	mov	cx,10
	add	ebx,3 shl 16
	int	0x40
	pop	ecx ebx
}
;*****************************************************************************
sb_size_x	equ [edi]
sb_start_x	equ [edi+2]
sb_size_y	equ [edi+4]
sb_start_y	equ [edi+6]
sb_btn_high	equ [edi+8]
sb_type		equ [edi+12]
sb_max_area	equ [edi+16]
sb_cur_area	equ [edi+20]
sb_position	equ [edi+24]
sb_bckg_col	equ [edi+28]
sb_frnt_col	equ [edi+32]
sb_line_col	equ [edi+36]
sb_redraw	equ [edi+40]
sb_delta	equ [edi+44]
sb_delta2	equ [edi+46]
sb_r_size_x	equ [edi+48]
sb_r_start_x	equ [edi+50]
sb_r_size_y	equ [edi+52]
sb_r_start_y	equ [edi+54]
sb_m_pos	equ [edi+56]
sb_m_pos_2	equ [edi+60]
sb_m_keys	equ [edi+64]
sb_run_size	equ [edi+68]
sb_position2	equ [edi+72]
sb_work_size	equ [edi+76]
sb_all_redraw	equ [edi+80]
sb_ar_offset	equ [edi+84]

;*****************************************************************************
;*****************************************************************************
; draw event
;*****************************************************************************
;*****************************************************************************
align 16
scroll_bar_vertical:
.draw:
	pusha
	mov	edi,dword [esp+36]
	mov	sb_delta,word 0
	call	.draw_1
	mov	sb_all_redraw,dword 0
scroll_bar_exit
.draw_1:
	pusha
;*********************************
	xor	eax,eax
	mov	ax,sb_size_y
	mov	edx,sb_btn_high
	shl	edx,1
	sub	eax,edx
	mov	sb_work_size,eax
;*********************************
	mov	eax,sb_work_size
	mov	ebx,sb_max_area
	cmp	ebx,sb_cur_area
	ja	@f
	jmp	.no_size

@@: 
	imul	eax,sb_cur_area
	xor	edx,edx
	div	ebx
	shl	edx,1
	cmp	edx,ebx
	jb	@f
	
	inc	eax
	
@@:
	cmp	eax,10
	jae	@f
	mov	eax,10
@@:
.no_size:
	mov	sb_run_size,eax
;*********************************
	cmp	word sb_delta,1
	je	.@@_3
	mov	eax,sb_work_size
	sub	eax,sb_run_size
	mov	ebx,sb_max_area
	cmp	ebx,sb_cur_area
	ja	@f
	xor	eax,eax
	jmp	.@@_1
@@:
	sub	ebx,sb_cur_area
	imul	eax,sb_position
	xor	edx,edx
	div	ebx
	shl	edx,1
	cmp	edx,ebx
	jb	@f
	
	inc	eax
	
@@:
.@@_1:
	mov	sb_position2,eax
	xor	edx,edx
	mov	dx,sb_size_y
	sub	edx,sb_btn_high
	sub	edx,sb_btn_high
	sub	edx,sb_run_size
	cmp	sb_position2,edx
	jbe	.@@_3
	mov	sb_position2,edx
.@@_3:
;*********************************	
	mov	ebx,sb_start_x
	shl	ebx,16
	inc	ebx
	mov	ecx,sb_size_y
	mov	edx,sb_line_col
	mov	eax,SF_DRAW_RECT
	cmp	dword sb_all_redraw,0
	je	@f
	int	0x40	; left extreme line
@@:
	push	ebx
	ror	ebx,16
	add	bx,sb_size_x
	rol	ebx,16
	cmp	dword sb_all_redraw,0
	je	@f
	int	0x40	; right extreme line
@@:
	pop	ebx

	push	ecx
	
	add	ebx,1 shl 16
	mov	bx,sb_size_x
	dec	ebx	
	mov	cx,1
	cmp	dword sb_all_redraw,0
	je	@f
	int	0x40	; top button - extreme line
@@:
	push	ecx
	add	ecx,1 shl 16
	add	ecx,sb_btn_high
	sub	ecx,2
	mov	edx,sb_frnt_col
	cmp	dword sb_all_redraw,0
	je	.no_draw_top_button
	int	0x40	; top button filling
;-----------------------------------------------------------------------------
; scrollbar type 2
	cmp	word sb_type,2
	jne	.no_type2
;*********************************
draw_up_arrow_vertical_type2
;*********************************
	jmp	.no_draw_top_button
.no_type2:
;*********************************
draw_up_arrow_vertical
;*********************************
.no_draw_top_button:
	pop	ecx
	
	ror	ecx,16
	add	ecx,sb_btn_high  ;14 shl 16
	rol	ecx,16
	mov	edx,sb_line_col
	cmp	dword sb_all_redraw,0
	je	@f
	mcall	SF_DRAW_RECT	;  top button - bottom line
@@:
	pop	ecx
;********************************* 
	ror	ecx,16	
	add	ecx,sb_btn_high  ;15 shl 16
	inc	ecx
	rol	ecx,16
	sub	ecx,sb_btn_high  ;30
	sub	ecx,sb_btn_high
	push	ecx
	mov	cx,0
	add	ecx,sb_position2
	dec	cx
	mov	eax,SF_DRAW_RECT
	test	cx,0x8000
	jnz	@f
	mov	edx,sb_bckg_col
	int	0x40	; top interval
@@:
;********************************* 
	shr	ecx,16
	add	ecx,sb_position2
	dec	ecx
	shl	ecx,16
	inc	ecx
	mov	edx,sb_line_col
	int	0x40	; runner - top extreme line
	
	add	ecx,1 shl 16
	mov	cx,0
	add	ecx,sb_run_size
	mov	sb_r_size_x,ebx
	mov	sb_r_size_y,ecx
	sub	ecx,2
	mov	edx,sb_frnt_col
	int	0x40	; runner filling
;-----------------------------------------------------------------------------
; scrollbar type 2
	cmp	word sb_type,2
	je	@f
;*********************************
draw_runner_center_vertical
;********************************* 
@@:
;-----------------------------------------------------------------------------
; scrollbar type 1 - stylish frame
	cmp	word sb_type,1
	jne	@f
	push	eax ebx ecx edx
	movzx	eax,word sb_start_x
	xor	ebx,ebx
	add	bx,sb_r_start_y
	dec	ebx
	movzx	ecx,word sb_size_x
	mov	edx,sb_run_size
	dec	edx
	; drawing a stylish frame on the slider
	stdcall draw_edge,eax,ebx,ecx,edx,sb_bckg_col,sb_frnt_col,sb_line_col
	pop	edx ecx ebx eax
@@:
;-----------------------------------------------------------------------------
	shr	ecx,16
	add	ecx,sb_run_size
	sub	ecx,2
	shl	ecx,16  
	inc	ecx
	mov	edx,sb_line_col
	int	0x40	; runner - bottom extreme line
	
	add	ecx,1 shl 16
	mov	cx,sb_start_y
	add	cx,sb_size_y
	sub	ecx,sb_btn_high
	dec	ecx
	mov	eax,ecx
	shr	eax,16
	sub	cx,ax
	test	cx,0x8000
	jnz	@f
	mov	edx,sb_bckg_col
	mcall	SF_DRAW_RECT	; bottom interval
@@:	
	pop	ecx	
;*********************************	
	mov	ax,cx
	shr	ecx,16
	add	cx,ax
	sub	ecx,2
	shl	ecx,16
	inc	ecx
	mov	edx,sb_line_col
	mov	eax,SF_DRAW_RECT
	cmp	dword sb_all_redraw,0
	je	@f
	int	0x40	; bottom button - top line
@@:
	push	ecx
	add	ecx,1 shl 16
	add	cx,sb_btn_high
	sub	ecx,2
	mov	edx,sb_frnt_col
	cmp	dword sb_all_redraw,0
	je	.no_draw_bottom_button
	int	0x40	; bottom button filling
;-----------------------------------------------------------------------------
; scrollbar type 2
	cmp	word sb_type,2
	jne	.no_type2_1
;*********************************
draw_down_arrow_vertical_type2
;*********************************
	jmp	.no_draw_bottom_button
.no_type2_1:
;********************************* 
draw_down_arrow_vertical
;********************************* 
.no_draw_bottom_button:
	pop	ecx
	
	ror	ecx,16
	add	ecx,sb_btn_high
	rol	ecx,16
	
	mov	edx,sb_line_col
	mov	cx,1
	cmp	dword sb_all_redraw,0
	je	@f
	mcall	SF_DRAW_RECT	; bottom button - extreme line
;-----------------------------------------------------------------------------
; scrollbar type 1 - stylish frame
	cmp	word sb_type,1
	jne	@f
	movzx	eax,word sb_start_x
	movzx	ebx,word sb_start_y
	movzx	ecx,word sb_size_x
	; drawing a stylish frame on the top button
	stdcall	draw_edge,eax,ebx,ecx,sb_btn_high,\
		sb_bckg_col,sb_frnt_col,sb_line_col
	add	bx,sb_size_y
	sub	ebx,sb_btn_high
	dec	ebx
	; drawing a stylish frame on the bottom  button
	stdcall	draw_edge,eax,ebx,ecx,sb_btn_high,\
		sb_bckg_col,sb_frnt_col,sb_line_col
@@:
;-----------------------------------------------------------------------------
	popa
	ret
;*****************************************************************************
;*****************************************************************************
; mouse event
;*****************************************************************************
;*****************************************************************************
.mouse:
	pusha
	mov	edi,dword [esp+36]
	mcall	SF_MOUSE_GET,SSF_WINDOW_POSITION
	mov	sb_m_pos,eax
	cmp	dword sb_m_pos_2,0
	jne	@f
	
	mov	sb_m_pos_2,eax	
@@:
	mcall	SF_MOUSE_GET,SSF_BUTTON
	mov	sb_m_keys,eax	

	cmp	sb_m_keys,eax
	je	@f
	
	mov	sb_m_keys,eax
	mov	sb_delta,dword 0
	mov	eax,sb_m_pos
	mov	sb_m_pos_2,eax
	jmp	.continue_2
@@:
	cmp	dword sb_m_keys,0
	jne	 @f
	mov	eax,sb_m_pos
	mov	sb_m_pos_2,eax
	jmp	.correct_1	;.exit_sb	
@@:
	mov	sb_delta,word 1

.continue_2:
	mov	eax,sb_m_pos
	test	eax,0x80000000
	jnz	.exit_sb
	
	test	eax,0x8000
	jnz	.exit_sb
	
	mov	ebx,eax
	shr	ebx,16	; x position
	shl	eax,16
	shr	eax,16	; y position

	mov	cx,sb_start_y
	cmp	ax,cx
	jb	.exit_sb
	
	cmp	word sb_delta2,0
	je	@f
	push	ecx
	add	cx,sb_btn_high
	cmp	ax,cx
	pop	ecx
	jb	.exit_sb
	
@@:
	add	cx,sb_size_y
	cmp	ax,cx
	ja	.exit_sb

	cmp	word sb_delta2,0
	je	@f
	sub	cx,sb_btn_high
	cmp	ax,cx
	ja	.exit_sb
	
@@:
	cmp	word sb_delta2,1
	je	@f
	
	cmp	dword sb_m_keys,0
	je	.exit_sb
	
	mov	cx,sb_start_x
	cmp	bx,cx
	jb	.exit_sb
	
	add	cx,sb_size_x
	cmp	bx,cx
	ja	.exit_sb

	mov	cx,sb_r_start_y
	cmp	ax,cx
	jb	.no_runner
	
	add	cx,sb_r_size_y
	cmp	ax,cx
	ja	.no_runner

	mov	sb_delta2,word 1	
@@:
	push	eax
	mov	ax,sb_m_pos
	cmp	ax,sb_m_pos_2
	je	.correct	
	
	shl	eax,16
	shr	eax,16
	mov	ebx,sb_m_pos_2
	shl	ebx,16
	shr	ebx,16
	cmp	eax,ebx
	jb	.sub
	
	sub	eax,ebx
	mov	ebx,eax
	add	sb_position2,ebx
	xor	eax,eax
	mov	ax,sb_size_y
	sub	eax,sb_btn_high
	sub	eax,sb_btn_high
	sub	eax,sb_run_size
	cmp	sb_position2,eax
	jbe	@f
	
	mov	sb_position2,eax
@@:
	mov	eax,sb_m_pos
	mov	sb_m_pos_2,eax
	pop	eax
	call	.draw_1
	jmp	.no_runner_1
	
.correct:
	pop	eax
	jmp	.exit_sb
.correct_1:
	mov	sb_delta,dword 0
	jmp	.exit_sb

.sub:
	sub	ebx,eax
	sub	sb_position2,ebx
	test	sb_position2,dword 0x80000000
	jz	@f
	
	mov	sb_position2,dword 0
@@:
	mov	eax,sb_m_pos
	mov	sb_m_pos_2,eax
	pop	eax
	call	.draw_1
	jmp	.no_runner_1

;*****************************************************************************
.no_runner:
	mov	sb_delta,dword 0
.no_runner_1:
	xor	ecx,ecx
	mov	cx,sb_start_y
	add	cx,sb_btn_high	;15
	cmp	word sb_delta,1
	je	.scroll_sb
	
	cmp	ax,cx
	ja	.scroll_sb
	
	cmp	dword sb_m_keys,0
	je	.exit_sb
	
	mov	eax,sb_ar_offset
	cmp	sb_position,eax  ;dword 0
	jbe	@f
	
	sub	sb_position,eax
	jmp	.all_sb
@@:
	xor	eax,eax
	mov	sb_position,eax ;dword 0
	mov	sb_position2,eax ;dword 0
	jmp	.all_sb
;*****************************************************************************
.scroll_sb:
	add	cx,sb_size_y
	sub	cx,sb_btn_high
	sub	cx,sb_btn_high
	cmp	word sb_delta,1
	je	@f
	
	cmp	ax,cx
	ja	.down_sb
	
@@:
	mov	ebx,sb_btn_high	;16  ;15
	add	bx,sb_start_y
	sub	cx,bx
	sub	ax,bx
	
.scroll_sb_1:	
	mov	ebx,sb_run_size
;*****************************************************************************	
	cmp	word sb_delta,1
	je	.continue
	
@@:
	mov	edx,eax
	push	ebx
	shr	ebx,1 
	sub	edx,ebx
	pop	ebx
	mov	sb_position2,edx
	test	sb_position2,dword 0x80000000
	jz	.test_1
	
	mov	sb_position2,dword 0
	jmp	.continue
	
.test_1: 
	xor	edx,edx
	mov	dx,sb_size_y
	sub	edx,sb_btn_high
	sub	edx,sb_btn_high
	sub	edx,sb_run_size
	cmp	sb_position2,edx
	jbe	.continue
	
	mov	sb_position2,edx

.continue:	
	mov	eax,sb_position2
	sub	ecx,ebx
.continue_1:
;*****************************************************************************  
	mov	ebx,eax

	mov	eax,sb_max_area
	cmp	eax,sb_cur_area
	ja	@f

	xor	eax,eax
	inc	eax
	mov	sb_position2,eax
	jmp	.all_sb
@@:
	sub	eax,sb_cur_area
	inc	eax
	shl	eax,10
	xor	edx,edx
	div	ecx
	shl	edx,1
	cmp	edx,ecx
	jb	@f
	
	inc	eax
	
@@:
	imul	eax,ebx
	shr	eax,10
	
	cmp	sb_position,eax
	je	.exit_sb
	
	cmp	eax,0
	ja	@f
	
	xor	eax,eax
	
@@:
.store_position:
	mov	edx,sb_max_area
	sub	edx,sb_cur_area
	cmp	edx,eax
	ja	@f

	mov	sb_position,edx
	jmp	.all_sb

@@:
	mov	sb_position,eax
.all_sb:
	mov	sb_redraw,dword 1
	call	.draw_1
	mov	eax,sb_m_pos
	mov	sb_m_pos_2,eax
	jmp	.exit_sb
;*****************************************************************************
.down_sb:
	cmp	dword sb_m_keys,0
	je	.exit_sb
	
	mov	eax,sb_max_area
	mov	ebx,sb_ar_offset
	sub	eax,sb_cur_area
	push	eax
	sub	eax,ebx
	cmp	sb_position,eax
	pop	eax
	jae	@f
	
	add	sb_position,ebx
	jmp	.all_sb
@@:
	mov	sb_position,eax
	xor	eax,eax
	mov	ax,sb_size_y
	sub	eax,sb_btn_high
	sub	eax,sb_btn_high
	sub	eax,sb_run_size
	mov	sb_position2,eax
	jmp	.all_sb	
.exit_sb:	
scroll_bar_exit


;*****************************************************************************
;*****************************************************************************
;*****************************************************************************
;*****************************************************************************
;*****************************************************************************
align 16
scroll_bar_horizontal:
.draw:
	pusha
	mov	edi,dword [esp+36]
	mov	sb_delta,word 0
	call	.draw_1
	mov	sb_all_redraw,dword 0
scroll_bar_exit
.draw_1:
	pusha
;*********************************
	xor	eax,eax
	mov	ax,sb_size_x	;sb_size_y
	mov	edx,sb_btn_high
	shl	edx,1
	sub	eax,edx
	mov	sb_work_size,eax
;*********************************
	mov	eax,sb_work_size	;sb_max_area
	mov	ebx,sb_max_area
	cmp	ebx,sb_cur_area
	ja	@f

	jmp	.no_size

@@: 
	imul	eax,sb_cur_area
	xor	edx,edx
	div	ebx
	shl	edx,1
	cmp	edx,ebx
	jb	@f
	
	inc	eax
	
@@:
	cmp	eax,10
	jae	@f
	mov	eax,10
@@:
.no_size:
	mov	sb_run_size,eax
;*********************************
	cmp	word sb_delta,1
	je	.@@_3
	mov	eax,sb_work_size
	sub	eax,sb_run_size
	mov	ebx,sb_max_area
	cmp	ebx,sb_cur_area
	ja	@f
	xor	eax,eax
	jmp	.@@_1
@@:
	sub	ebx,sb_cur_area
	imul	eax,sb_position
	xor	edx,edx
	div	ebx
	shl	edx,1
	cmp	edx,ebx
	jb	@f
	
	inc	eax
	
@@:
.@@_1:
	mov	sb_position2,eax
	xor	edx,edx
	mov	dx,sb_size_x
	sub	edx,sb_btn_high
	sub	edx,sb_btn_high
	sub	edx,sb_run_size
	cmp	sb_position2,edx
	jbe	.@@_3
	mov	sb_position2,edx
.@@_3:
;*********************************	
	mov	ebx,sb_size_x
	mov	ecx,sb_start_y
	shl	ecx,16
	inc	ecx	
	mov	edx,sb_line_col
	mov	eax,SF_DRAW_RECT
	cmp	dword sb_all_redraw,0
	je	@f
	int	0x40	; top extreme line
@@:
	push	ecx
	ror	ecx,16
	add	cx,sb_size_y
	rol	ecx,16
	cmp	dword sb_all_redraw,0
	je	@f
	int	0x40	; bottom extreme line
@@:
	pop	ecx

	push	ebx
	
	add	ecx,1 shl 16
	mov	cx,sb_size_y
	dec	ecx	
	mov	bx,1
	cmp	dword sb_all_redraw,0
	je	@f
	int	0x40	; left button - extreme line
@@:
	push	ebx
	add	ebx,1 shl 16
	add	ebx,sb_btn_high
	sub	ebx,2
	mov	edx,sb_frnt_col
	cmp	dword sb_all_redraw,0
	je	.no_draw_top_button
	int	0x40	; left  button filling
;-----------------------------------------------------------------------------
; scrollbar type 2
	cmp	word sb_type,2
	jne	.no_type2
;*********************************
draw_up_arrow_horizontal_type2
;*********************************
	jmp	.no_draw_top_button
.no_type2:
;*********************************	
draw_up_arrow_horizontal
;********************************* 
.no_draw_top_button:
	pop	ebx
	
	ror	ebx,16
	add	ebx,sb_btn_high  ;14 shl 16
	rol	ebx,16
	mov	edx,sb_line_col
	cmp	dword sb_all_redraw,0
	je	@f
	mcall	SF_DRAW_RECT	;  left  button - right line
@@:
	pop	ebx
;********************************* 
	ror	ebx,16	
	add	ebx,sb_btn_high  ;15 shl 16
	inc	ebx
	rol	ebx,16
	sub	ebx,sb_btn_high  ;30
	sub	ebx,sb_btn_high
	push	ebx
	mov	bx,0
	add	ebx,sb_position2
	dec	bx
	mov	eax,SF_DRAW_RECT
	test	bx,0x8000
	jnz	@f
	mov	edx,sb_bckg_col
	int	0x40	; left interval
@@:
;********************************* 
	shr	ebx,16
	add	ebx,sb_position2
	dec	ebx
	shl	ebx,16
	inc	ebx
	mov	edx,sb_line_col
	int	0x40	; runner - left extreme line
	
	add	ebx,1 shl 16
	mov	bx,0
	add	ebx,sb_run_size
	mov	sb_r_size_x,ebx
	mov	sb_r_size_y,ecx
	sub	ebx,2
	mov	edx,sb_frnt_col
	int	0x40	; runner filling
;-----------------------------------------------------------------------------
; scrollbar type 2
	cmp	word sb_type,2
	je	@f
;*********************************
draw_runner_center_horizontal
;********************************* 
@@:	
;-----------------------------------------------------------------------------
; scrollbar type 1 - stylish frame
	cmp	word sb_type,1
	jne	@f
	push	eax ebx ecx edx
	xor	eax,eax
	add	ax,sb_r_start_x
	dec	eax
	movzx	ebx,word sb_start_y
	mov	ecx,sb_run_size
	dec	ecx
	movzx	edx,word sb_size_y
	; drawing a stylish frame on the slider
	stdcall	draw_edge,eax,ebx,ecx,edx,\
		sb_bckg_col,sb_frnt_col,sb_line_col
	pop	edx ecx ebx eax
@@:
;-----------------------------------------------------------------------------
	shr	ebx,16
	add	ebx,sb_run_size
	sub	ebx,2
	shl	ebx,16  
	inc	ebx
	mov	edx,sb_line_col
	int	0x40	; runner - bottom extreme line
	
	add	ebx,1 shl 16
	mov	bx,sb_start_x
	add	bx,sb_size_x
	sub	ebx,sb_btn_high
	dec	ebx
	mov	eax,ebx
	shr	eax,16
	sub	bx,ax
	test	bx,0x8000
	jnz	@f
	mov	edx,sb_bckg_col
	mcall	SF_DRAW_RECT	; bottom interval
@@:
	pop	ebx	
;*********************************	
	mov	ax,bx
	shr	ebx,16
	add	bx,ax
	sub	ebx,2
	shl	ebx,16
	inc	ebx
	mov	edx,sb_line_col
	mov	eax,SF_DRAW_RECT
	cmp	dword sb_all_redraw,0
	je	@f
	int	0x40	; bottom button - top line
@@:
	push	ebx
	add	ebx,1 shl 16
	add	bx,sb_btn_high
	sub	ebx,2
	mov	edx,sb_frnt_col
	cmp	dword sb_all_redraw,0
	je	.no_draw_bottom_button
	int	0x40	; bottom button filling
;-----------------------------------------------------------------------------
; scrollbar type 2
	cmp	word sb_type,2
	jne	.no_type2_1
;*********************************
draw_down_arrow_horizontal_type2
;*********************************
	jmp	.no_draw_bottom_button
.no_type2_1:
;********************************* 
draw_down_arrow_horizontal
;*********************************
.no_draw_bottom_button: 
	pop	ebx
	
	ror	ebx,16
	add	ebx,sb_btn_high  ;14 shl 16
	rol	ebx,16
	
	mov	edx,sb_line_col
	mov	bx,1
	cmp	dword sb_all_redraw,0
	je	@f
	mcall	SF_DRAW_RECT	; bottom button - extreme line
;-----------------------------------------------------------------------------
; scrollbar type 1 - stylish frame
	cmp	word sb_type,1
	jne	@f
	; drawing a stylish frame on the left button
	movzx	eax,word sb_start_x
	movzx	ebx,word sb_start_y
	movzx	edx,word sb_size_y
	stdcall	draw_edge,eax,ebx,sb_btn_high,edx,\
		sb_bckg_col,sb_frnt_col,sb_line_col
	
	movzx	eax,word sb_start_x
	add	ax,sb_size_x
	sub	eax,sb_btn_high
	dec	eax
	; drawing a stylish frame on the right button
	stdcall	draw_edge,eax,ebx,sb_btn_high,edx,\
		sb_bckg_col,sb_frnt_col,sb_line_col
@@:
;-----------------------------------------------------------------------------
	popa
	ret
;*****************************************************************************
; mouse event
;*****************************************************************************
.mouse:
	pusha
	mov	 edi,dword [esp+36]
	mcall	SF_MOUSE_GET,SSF_WINDOW_POSITION
	mov	sb_m_pos,eax
	cmp	dword sb_m_pos_2,0
	jne	@f
	
	mov	sb_m_pos_2,eax	
@@:
	mcall	SF_MOUSE_GET,SSF_BUTTON
	mov	sb_m_keys,eax	

	cmp	sb_m_keys,eax
	je	@f
	
	mov	sb_m_keys,eax
 
	mov	sb_delta,dword 0
	
	mov	eax,sb_m_pos
	mov	sb_m_pos_2,eax
	jmp	.continue_2
	
@@:
	cmp	dword sb_m_keys,0
	jne	 @f
	mov	eax,sb_m_pos
	mov	sb_m_pos_2,eax
	jmp	.correct_1	;.exit_sb
@@:
	mov	sb_delta,word 1

.continue_2:
	mov	eax,sb_m_pos
	test	eax,0x80000000
	jnz	.exit_sb
	
	test	eax,0x8000
	jnz	.exit_sb
	
	mov	ebx,eax
	shr	eax,16	; x position
	shl	ebx,16
	shr	ebx,16	; y position

	mov	cx,sb_start_x	;y
	cmp	ax,cx
	jb	.exit_sb
	
	cmp	word sb_delta2,0
	je	@f
	push	ecx
	add	cx,sb_btn_high
	cmp	ax,cx
	pop	ecx
	jb	.exit_sb
	
@@:
	add	cx,sb_size_x	;y
	cmp	ax,cx
	ja	.exit_sb

	cmp	word sb_delta2,0
	je	@f
	sub	cx,sb_btn_high
	cmp	ax,cx
	ja	.exit_sb
	
@@:
	cmp	word sb_delta2,1
	je	@f
	
	cmp	dword sb_m_keys,0
	je	.exit_sb
	
	mov	cx,sb_start_y	;x
	cmp	bx,cx
	jb	.exit_sb
	
	add	cx,sb_size_y	;x
	cmp	bx,cx
	ja	.exit_sb
	
	mov	cx,sb_r_start_x
	cmp	ax,cx
	jb	.no_runner
	
	add	cx,sb_r_size_x
	cmp	ax,cx
	ja	.no_runner

	mov	sb_delta2,word 1
@@:
	push	eax
	mov	eax,sb_m_pos
	mov	ebx,sb_m_pos_2
	shr	eax,16
	shr	ebx,16
	cmp	eax,ebx
	je	.correct	
	
;	shl	eax,16
;	shr	eax,16
;	mov	ebx,sb_m_pos_2
;	shl	ebx,16
;	shr	ebx,16
;	cmp	eax,ebx
	jb	.sub
	
	sub	eax,ebx
	mov	ebx,eax
	add	sb_position2,ebx
	xor	eax,eax
	mov	ax,sb_size_x	;y
	sub	eax,sb_btn_high
	sub	eax,sb_btn_high
	sub	eax,sb_run_size
	cmp	sb_position2,eax
	jbe	@f
	
	mov	sb_position2,eax
@@:
	mov	eax,sb_m_pos
	mov	sb_m_pos_2,eax
	pop	eax
	call	.draw_1
	jmp	.no_runner_1
	
.correct:
	pop	eax
	jmp	.exit_sb
.correct_1:
	mov	sb_delta,dword 0
	jmp	.exit_sb

.sub:
	sub	ebx,eax
	sub	sb_position2,ebx
	test	sb_position2,dword 0x80000000
	jz	@f
	
	mov	sb_position2,dword 0
@@:
	mov	eax,sb_m_pos
	mov	sb_m_pos_2,eax
	pop	eax
	call	.draw_1
	jmp	.no_runner_1

;*****************************************************************************
.no_runner:
	mov	sb_delta,dword 0
.no_runner_1:
	xor	ecx,ecx
	mov	cx,sb_start_x	;y
	add	cx,sb_btn_high	;15
	cmp	word sb_delta,1
	je	.scroll_sb
	
	cmp	ax,cx
	ja	.scroll_sb
	
	cmp	dword sb_m_keys,0
	je	.exit_sb
	
	mov	eax,sb_ar_offset
	cmp	sb_position,eax  ;dword 0
	jbe	@f
	
	sub	sb_position,eax
	jmp	.all_sb
@@:
	xor	eax,eax
	mov	sb_position,eax ;dword 0
	mov	sb_position2,eax  ;dword 0
	jmp	.all_sb
;*****************************************************************************
.scroll_sb:
	add	cx,sb_size_x	;y
	sub	cx,sb_btn_high
	sub	cx,sb_btn_high
	cmp	word sb_delta,1
	je	@f
	
	cmp	ax,cx
	ja	.down_sb
	
@@:
	mov	ebx,sb_btn_high	;16  ;15
	add	bx,sb_start_x	;y
	sub	cx,bx
	sub	ax,bx
	
.scroll_sb_1:	
	mov	ebx,sb_run_size
;*****************************************************************************	
	cmp	word sb_delta,1
	je	.continue
	
@@:
	mov	edx,eax

	push	ebx
	shr	ebx,1 
	sub	edx,ebx
	pop	ebx
	mov	sb_position2,edx
	test	sb_position2,dword 0x80000000
	jz	 .test_1
	
	mov	sb_position2,dword 0
	jmp	.continue
	
.test_1: 
	xor	edx,edx
	mov	dx,sb_size_x	 ;y
	sub	edx,sb_btn_high
	sub	edx,sb_btn_high
	sub	edx,sb_run_size
	cmp	sb_position2,edx
	jbe	.continue
	
	mov	sb_position2,edx

.continue:	
	mov	eax,sb_position2
	sub	ecx,ebx
.continue_1:
;*****************************************************************************  
	mov	ebx,eax

	mov	eax,sb_max_area
	cmp	eax,sb_cur_area
	ja	@f

	xor	eax,eax
	inc	eax
	mov	sb_position2,eax
	jmp	.all_sb
@@:
	sub	eax,sb_cur_area
	inc	eax
	shl	eax,10
	xor	edx,edx
	div	ecx
	shl	edx,1
	cmp	edx,ecx
	jb	@f
	
	inc	eax
	
@@:
	imul  eax,ebx
	shr	eax,10
	
	cmp	sb_position,eax
	je	.exit_sb
	
	cmp	eax,0
	ja	@f
	
	xor	eax,eax
	
@@:
.store_position:
	mov	edx,sb_max_area
	sub	edx,sb_cur_area
	cmp	edx,eax
	ja	@f

	mov	sb_position,edx
	jmp	.all_sb

@@:
	mov	sb_position,eax
.all_sb:
	mov	sb_redraw,dword 1
	call	.draw_1
	mov	eax,sb_m_pos
	mov	sb_m_pos_2,eax
	jmp	.exit_sb
;*****************************************************************************
.down_sb:
	cmp	dword sb_m_keys,0
	je	.exit_sb
	
	mov	eax,sb_max_area
	mov	ebx,sb_ar_offset
	sub	eax,sb_cur_area
	push	eax
	sub	eax,ebx
	cmp	sb_position,eax
	pop	eax
	jae	@f
	
	add	sb_position,ebx
	jmp	.all_sb
@@:
	mov	sb_position,eax
	xor	eax,eax
	mov	ax,sb_size_x
	sub	eax,sb_btn_high
	sub	eax,sb_btn_high
	sub	eax,sb_run_size
	mov	sb_position2,eax
	jmp	.all_sb
;*****************************************************************************
.exit_sb:
scroll_bar_exit
