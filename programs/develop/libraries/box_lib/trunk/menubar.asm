;**************************************************************
; MenuBar Macro for KolibriOS
; Copyright (c) 2009, Marat Zakiyanov aka Mario79, aka Mario
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
macro menu_bar_exit
{
popa        
ret 4
}
;*****************************************************************************
align 16
menu_bar:
m_type			equ [edi]
m_size_x		equ [edi+4]
m_start_x		equ [edi+6]
m_size_y		equ [edi+8]
m_start_y		equ [edi+10]
m_text_pointer		equ [edi+12]
m_pos_pointer		equ [edi+16]
m_text_end		equ [edi+20]
m_ret_key		equ [edi+24]
m_mouse_keys		equ [edi+28]
m_size_x1		equ [edi+32]
m_start_x1		equ [edi+34]
m_size_y1		equ [edi+36]
m_start_y1		equ [edi+38]
m_bckg_col		equ [edi+40]
m_frnt_col		equ [edi+44]
m_menu_col		equ [edi+48]
m_select		equ [edi+52]
m_out_select		equ [edi+56]
m_buf_adress		equ [edi+60]
m_procinfo		equ [edi+64]
m_click			equ [edi+68]
m_cursor		equ [edi+72]
m_cursor_old		equ [edi+76]
m_interval		equ [edi+80]
m_cursor_max		equ [edi+84]
m_extended_key		equ [edi+88]
m_menu_sel_col		equ [edi+92]
m_bckg_text_col		equ [edi+96]
m_frnt_text_col		equ [edi+100]
m_mouse_keys_old	equ [edi+104]
m_font_height		equ [edi+108]
m_cursor_out		equ [edi+112]
m_get_mouse_flag	equ [edi+116]
;*****************************************************************************
;*****************************************************************************
; draw event
;*****************************************************************************
;*****************************************************************************
.draw:
	pusha
	mov	edi,dword [esp+36]
	call	.draw_1
	menu_bar_exit
.draw_1:
	call	.calc_m_cursor_max
	dec	dword m_cursor_max
	mov	eax,m_cursor_max
	imul	eax,m_interval
	mov	m_size_y1,ax
	
	mov	ebx,m_size_x
	mov	ecx,m_size_y
	cmp	dword m_select,1
	je	.active
	mov	edx,m_bckg_col
	jmp	@f
.active:
	mov	edx,m_frnt_col
@@:
	mcall	SF_DRAW_RECT
	shr	ecx,16
	mov	bx,cx
	movzx	eax,word m_size_y
	call	.calculate_font_offset
	cmp	dword m_select,1
	je	.active_1
	mov	ecx,m_bckg_text_col
	add	ecx,0x80000000
	jmp	@f
.active_1:
	mov	ecx,m_frnt_text_col
	add	ecx,0x80000000
@@:
	mov	edx,m_text_pointer
	mcall	SF_DRAW_TEXT
	ret

.draw_2:
	mcall	SF_DRAW_RECT,m_size_x1,m_size_y1,m_menu_col
	ret

.calculate_font_offset:
	sub	eax,m_font_height
	shr	eax,1
	add	ebx,eax
	add	ebx,4 shl 16
	ret
	
.draw_3:
	mov	ebx,m_size_x1
	mov	ecx,m_size_y1

	push	ebx ecx
	mov	eax,m_cursor_old
	imul	eax,m_interval
	shl	eax,16
	add	ecx,eax
	mov	cx,m_interval
	mcall	SF_DRAW_RECT,,,m_menu_col
	pop	ecx ebx
    
	push	ebx ecx
	mov	eax,m_cursor
	imul	eax,m_interval
	shl	eax,16
	add	ecx,eax
	mov	cx,m_interval
	mov	edx,m_menu_sel_col
	mcall	SF_DRAW_RECT
	pop	ecx ebx
	
	shr	ecx,16
	mov	bx,cx
	mov	eax,m_interval
	call	.calculate_font_offset
	mov	edx,m_pos_pointer
	xor	ebp,ebp
@@:
	cmp	ebp,m_cursor
	jne	.no_active_text
	mov	ecx,m_frnt_text_col
	mov	eax,m_menu_sel_col
	jmp	.active_text
.no_active_text:
	mov	ecx,m_bckg_text_col
	mov	eax,m_menu_col
.active_text:
	add	ecx,0xC0000000
	push	edi
	mov	edi,eax
	mcall	SF_DRAW_TEXT
	pop	edi
	call	.get_next_text
	inc	ebp
	add	ebx,m_interval
	jmp	@r
.draw_end:
	ret
	
.calc_m_cursor_max:
	mov	edx,m_pos_pointer
	mov	m_cursor_max,dword 0
@@:
	inc	dword m_cursor_max
	call	.get_next_text
	jmp	@r
;*****************************************************************************
.get_next_text:
	mov	esi,edx
@@:
	cmp	esi,m_text_end
	je	.get_next_text_end
	cld
	lodsb
	test	al,al
	jnz	@r
	mov	edx,esi
	ret
.get_next_text_end:
	add	esp,4
	ret
;*****************************************************************************
;*****************************************************************************
; mouse event
;*****************************************************************************
;*****************************************************************************
.activate:
	pusha
	mov	edi,dword [esp+36]
	jmp	.start_loop
.mouse:
	pusha
	mov	edi,dword [esp+36]

	call	.processing_real_mouse

	test	eax,0x80000000
	jnz	.exit_menu
	test	eax,0x8000
	jnz	.exit_menu

	mov	ebx,eax
	shr	ebx,16   ; x position
	shl	eax,16
	shr	eax,16   ; y position
	 
	mov	cx,m_start_x
	cmp	bx,cx
	jb	.exit_menu
     
	add	cx,m_size_x
	dec	cx
	cmp	bx,cx
	ja	.exit_menu

	mov	cx,m_start_y
	cmp	ax,cx
	jb	.exit_menu
     
	add	cx,m_size_y
	cmp	ax,cx
	ja	.exit_menu

	test	dword m_mouse_keys,1b
	jnz	@f
	cmp	dword m_select,1
	je	.exit_menu_1
	mov	dword m_select,1
	call	.draw_1
	jmp	.exit_menu_1
@@:
	cmp	dword m_get_mouse_flag,1
	mov	m_get_mouse_flag,dword 0
	je	@f
	
	mov	eax,m_mouse_keys
	cmp	eax,m_mouse_keys_old
	je	.exit_menu_1
@@:
	cmp	dword m_type,1
	jne	.start_loop

	xor	eax,eax
	inc	eax
	mov	m_cursor_out,eax
	mov	m_click,eax
	menu_bar_exit

.start_loop:
	mov	m_select,dword 1
	call	.draw_1
	call	.allocate_menu_area
	call	.get_menu_area
	 
	call	.draw_2
.red:
	call	.draw_3
.still:
	mcall	SF_WAIT_EVENT
	cmp	eax,1
	je	.exit_menu_3
	cmp	eax,2
	je	.key_menu
	cmp	eax,3
	je	.exit_menu_3
	cmp	eax,6
	je	.mouse_menu
	jmp	.still
	
.key_menu:
	mcall	SF_GET_KEY

	cmp	dword m_extended_key,1
	je	.extended_key
	test	al,al
	jnz	.key_menu_end
	cmp	ah, 0xE0
	jne	@f
	mov	m_extended_key,dword 1
.key_menu_end:
	jmp	.still
@@:
	cmp	ah,72  ;Arrow Up
	je	.menu_key_72_1
	cmp	ah,80 ; Arrow Down
	je	.menu_key_80_1
	cmp	ah,28  ; Enter
	je	.menu_key_28_1
	cmp	ah,1  ; Esc
	je	.menu_key_1_1
	cmp	ah,75  ; L-Arrow down
	je	.menu_key_75_1
	cmp	ah,77  ; R-Arrow down
	je	.menu_key_77_1
	
.key_menu_end_1:
	cmp	ah,208 ; Arrow Down
	je	.key_menu_end
	cmp	ah,200 ; Arrow Up
	je	.key_menu_end
	cmp	ah,156 ; Enter
	je	.key_menu_end
	cmp	ah,129 ; Esc
	je	.key_menu_end
	cmp	ah,199  ;Home
	je	.key_menu_end
	cmp	ah,207  ;End
	je	.key_menu_end
	cmp	ah,201  ;Page UP
	je	.key_menu_end
	cmp	ah,209  ;Page Down
	je	.key_menu_end
	cmp	ah,42	; NumLock ON
	je	.key_menu_end
	cmp	ah,170  ; NumLock ON
	je	.key_menu_end
	cmp	ah,210  ; Insert
	je	.key_menu_end
	cmp	ah,211  ; Delete
	je	.key_menu_end
	cmp	ah,157  ; Ctrl up
	je	.key_menu_end
	cmp	ah,184  ; Alt up
	je	.key_menu_end
	cmp	ah,170  ; L-Shift up
	je	.key_menu_end
	cmp	ah,182  ; R-Shift up
	je	.key_menu_end
	cmp	ah,203  ; L-Arrow up
	je	.key_menu_end
	cmp	ah,205  ; R-Arrow up
	je	.key_menu_end

	jmp	.exit_menu_3

;---------------------------------------------------------------------
.extended_key:
	mov	m_extended_key, dword 0
.menu_key_80:
	cmp	ah,80	; arrow down
	jne	.menu_key_72
.menu_key_80_1:
	mov	eax,m_cursor_max
	dec	eax
	cmp	eax,m_cursor
	je	.still ;@f
	mov	ebx,m_cursor
	mov	m_cursor_old,ebx
	inc	dword m_cursor
;@@:
	jmp	.red
;---------------------------------------------------------------------
.menu_key_72:
	cmp	ah,72	;arrow up
	jne	.menu_key_71
.menu_key_72_1:
	cmp	m_cursor,dword 0
	je	.still  ;@f
	mov	ebx,m_cursor
	mov	m_cursor_old,ebx
	dec	dword m_cursor
;@@:
	jmp	.red
;---------------------------------------------------------------------
.menu_key_71:
	cmp	ah,71    ;Home
	je	@f
	cmp	ah,73    ;PageUp
	jne	.menu_key_79
@@:
	cmp	dword m_cursor,0
	je	.still
	mov	ebx,m_cursor
	mov	m_cursor_old,ebx
	mov	m_cursor,dword 0
	jmp	.red
;---------------------------------------------------------------------
.menu_key_79:
	cmp	ah,79    ; End
	je	@f
	cmp	ah,81    ; PageDown
	jne	.menu_key_1
@@:
	mov	ecx,m_cursor_max
	dec	ecx
	cmp	m_cursor,ecx
	je	.still
	mov	ebx,m_cursor
	mov	m_cursor_old,ebx
	mov	m_cursor,ecx
	jmp	.red
;---------------------------------------------------------------------
.menu_key_1:
	cmp	ah,1 ; Esc
	jne	.menu_key_28
.menu_key_1_1:
	jmp	.exit_menu_3
;---------------------------------------------------------------------
.menu_key_28:
	cmp	ah,28 ; Enter
	jne	.menu_key_75
.menu_key_28_1:
	xor	eax,eax
	mov	m_mouse_keys_old,eax
	inc	eax
	mov	m_mouse_keys,eax
	jmp	.exit_menu_2
;---------------------------------------------------------------------
.menu_key_75:
	cmp	ah,75  ; L-Arrow down
	jne	.menu_key_77
.menu_key_75_1:
	mov	m_ret_key,dword 1
	jmp	.exit_menu_3
;---------------------------------------------------------------------
.menu_key_77:
	cmp	ah,77  ; R-Arrow down
	jne	.key_menu_end_1
.menu_key_77_1:
	mov	m_ret_key,dword 2
	jmp	.exit_menu_3
;---------------------------------------------------------------------
.processing_real_mouse:
	mcall	SF_MOUSE_GET,SSF_BUTTON
	mov	ebx,m_mouse_keys
	mov	m_mouse_keys_old,ebx
	mov	m_mouse_keys,eax
	 
	mcall	SF_MOUSE_GET,SSF_WINDOW_POSITION
	ret
;---------------------------------------------------------------------
.allocate_menu_area:
	call	.free_menu_area
	movzx	ecx,word m_size_x1
	movzx	eax,word m_size_y1
	imul	ecx,eax
	lea	ecx,[ecx*3]
	mcall	SF_SYS_MISC,SSF_MEM_ALLOC
	mov	m_buf_adress,eax
	ret
;---------------------------------------------------------------------
.free_menu_area:
	cmp	dword m_buf_adress,0
	je	@f
	mcall	SF_SYS_MISC,SSF_MEM_FREE,m_buf_adress
	xor	eax,eax
	mov	m_buf_adress,eax
@@:
	ret
;---------------------------------------------------------------------
.get_menu_area:
	mcall	SF_THREAD_INFO, m_procinfo,-1
	 
	mov	cx,m_size_x1
	shl	ecx,16
	mov	cx,m_size_y1
	 
	mov	dx,m_start_x1
	mov	eax,m_procinfo
	add	dx,[eax+34]
	add	dx,[eax+54]
	shl	edx,16
	mov	dx,m_start_y1
	add	dx,[eax+38]
	add	dx,[eax+58]
	 
	mcall	SF_GET_IMAGE, m_buf_adress
	ret
;---------------------------------------------------------------------
.put_menu_area:
	mov	cx,m_size_x1
	shl	ecx,16
	mov	cx,m_size_y1
	 
	mov	dx,m_start_x1
	shl	edx,16
	mov	dx,m_start_y1
	 
	mcall	SF_PUT_IMAGE, m_buf_adress
	call	.free_menu_area
	ret
;---------------------------------------------------------------------
.mouse_menu:
	call	.processing_real_mouse

	test	eax,0x80000000
	jnz	.still
	test	eax,0x8000
	jnz	.still
	 
	mov	ebx,eax
	shr	ebx,16
	shl	eax,16
	shr	eax,16
	 
	xor	ecx,ecx
	mov	cx,m_start_y1
	cmp	ax,cx
	jbe	.close
	add	cx,m_size_y1
	cmp	ax,cx
	jae	.close
	mov	cx,m_start_x1
	cmp	bx,cx
	jbe	.close
	add	cx,m_size_x1
	cmp	bx,cx
	jae	.close
	sub	ax,m_start_y1
	mov	ebx,m_interval
	xor	edx,edx
	div	ebx
	mov	ebx,m_cursor
	cmp	eax,ebx
	je	.no_red
	mov	m_cursor_old,ebx
	cmp	eax,m_cursor_max
	jb	@f
	mov	eax,m_cursor_max
@@:
	mov	m_cursor,eax
	test	m_mouse_keys,dword 1b
	jz	.red
	jmp	.exit_menu_2
.no_red:
	test	m_mouse_keys,dword 1b
	jz	.still
	jmp	.exit_menu_2
.close:
	test	m_mouse_keys,dword 1b
	jz	.still
	jmp	.exit_menu_3
	 
.exit_menu:
	cmp	dword m_select,0
	je	.exit_menu_1
	mov	m_select,dword 0
	 
	call	.draw_1

	test	m_mouse_keys,dword 1b
	jz	.exit_menu_1
	 
.exit_menu_3:
	mov	m_select,dword 0
	call	.put_menu_area
	xor	eax,eax
	mov	m_cursor_out,eax
	jmp	.exit
	
.exit_menu_2:
	mov	m_select,dword 0
	call	.put_menu_area
	mov	edx,m_pos_pointer
	
	mov	ebx,m_cursor
@@:
	cmp	ebx,0
	jz	@f
	dec	ebx
	call	.get_next_text
	jmp	@r
	
@@:
	mov	m_out_select,edx
	mov	eax,m_cursor
	inc	eax
	mov	m_cursor_out,eax
	
	mov	eax,m_mouse_keys
	cmp	eax,m_mouse_keys_old
	jne	.exit
	xor	eax,eax
	mov	m_cursor_out,eax
.exit:
	call	.draw_1
	mov	m_click,dword 1
	jmp	@f
.exit_menu_1:
	mov	m_click,dword 0
@@:
menu_bar_exit
