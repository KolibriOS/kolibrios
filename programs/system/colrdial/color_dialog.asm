;*****************************************************************************
; Color Dialog - for Kolibri OS
; Copyright (c) 2013, Marat Zakiyanov aka Mario79, aka Mario
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
;---------------------------------------------------------------------
;Some documentation for memory
;
;area name db 'FFFFFFFF_color_dialog',0 ; FFFFFFFF = PID
;
; communication area data
; flag  ; +0
; dw 0   ; 0 - empty, 1 - OK, color selected
;          2 - use another method/not found program, 3 - cancel
;
; type of dialog:  0-Palette&Tone
; dw 0 ; +2
;
; window X size ; +4
; dw 0
;
; window X position ; +6
; dw 0
;
; window y size ; +8
; dw 0
;
; window Y position ; +10
; dw 0
;
; ColorDialog WINDOW SLOT ; +12
; dd 0
;
; Color type ; +16
; dd 0
;
; Color value ; +20
; dd 0

; First start flag ; +24
; dd 0 ; 0 - first start, 1 - subsequent starts
;
; Old colors ; +28
; rd 10
;
; Free area ; +68
;---------------------------------------------------------------------
  use32
  org	 0x0

  db	 'MENUET01'
  dd	 0x01
  dd	 START
  dd	 IM_END
  dd	 I_END
  dd	 stacktop
  dd	 param
  dd	 path
;---------------------------------------------------------------------
include '../../macros.inc'
include '../../develop/libraries/box_lib/load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
;include 'lang.inc'
;include '../../debug.inc'
@use_library
;---------------------------------------------------------------------
p_start_x = 10
p_start_y = 10

p_size_x = 20
p_size_y = 256
;--------------------------------------
t_start_x = 40
t_start_y = 10
;--------------------------------------
w_start_x = 200
w_start_y = 200

w_size_x = 400
w_size_y = 350
;--------------------------------------
c_start_x = t_start_x + p_size_y + 10
c_start_y = 10

c_size_x = 40
c_size_y = 20
;---------------------------------------------------------------------
x_minimal_size equ 350
y_minimal_size equ 250
;---------------------------------------------------------------------
START:
	mcall	68,11
	mcall	66,1,1
	mcall	40,0x27
;	mcall	40,0x7
	call	get_communication_area
	call	get_active_pocess
	call	clear_colors_history
;--------------------------------------
load_libraries	l_libs_start,end_l_libs
	test	eax,eax
	jnz	button.exit_2
;--------------------------------------
	xor	eax,eax
	mov	al,p_size_x
	mov	[palette_SIZE_X],eax
	mov	ax,p_size_y
	mov	[palette_SIZE_Y],eax
	mov	[tone_SIZE_X],eax
	mov	[tone_SIZE_Y],eax
	mov	eax,0xff0000
	mov	[tone_color],eax
	mov	[selected_color],eax
	call	prepare_scrollbars_position_from_color
;--------------------------------------
	mov	ecx,[palette_SIZE_Y]
	imul	ecx,[palette_SIZE_X]
	lea	ecx,[ecx*3]
	inc	ecx	;reserve for stosd
	mcall	68,12
	mov	[palette_area],eax
;--------------------------------------
	call	create_palette
;--------------------------------------
	mov	ecx,[tone_SIZE_Y]
	imul	ecx,[tone_SIZE_X]
	lea	ecx,[ecx*3]
	inc	ecx	;reserve for stosd
	mcall	68,12
	mov	[tone_area],eax
;--------------------------------------
	call    create_tone
;---------------------------------------------------------------------
align 4
red:
	call	draw_window
;---------------------------------------------------------------------
align 4
still:
	mcall	10

	cmp	eax,1
	je	red

	cmp	eax,2
	je	key

	cmp	eax,3
	je	button

	cmp	eax,6
	je	mouse

	jmp	still
;---------------------------------------------------------------------
align 4
button:
	mcall	17

	cmp	ah, 2
	je	palette_button

	cmp	ah, 3
	je	tone_button

	cmp	ah, 4
	je	color_button

	cmp	ah, 30
	jb	@f

	cmp	ah, 39
	ja	@f

	sub	ah,30
	movzx	eax,ah
	shl	eax,2
	add	eax,[communication_area]
	add	eax,28
	mov	eax,[eax]
	mov	[selected_color],eax
	call	prepare_scrollbars_position_from_color
	call	draw_selected_color
	call	draw_scrollbars
	jmp	still
;--------------------------------------
align 4
@@:
	cmp	ah, 1
	jne	still
;--------------------------------------
align 4
.exit:
	mov	eax,[communication_area]
	mov	[eax],word 3
	jmp	@f
; dps "CD flag value: cancel "
;--------------------------------------
align 4
.exit_1:
;--------------------------------------
	call	scroll_colors_history
;--------------------------------------
align 4
@@:
;	mov	ax,[eax]
;	and	eax,0xffff
; dps "CD flag value: "
; dpd eax
; newline

	call	get_window_param
	mov	ebx,[communication_area]
	mov	ecx,procinfo
;	mov	eax,[window_x]
	mov	eax,[ecx+34]
	shl	eax,16
	add	eax,[ecx+42]
	mov	[ebx+4],eax
;	mov	eax,[window_y]
	mov	eax,[ecx+38]
	shl	eax,16
	add	eax,[ecx+46]
	mov	[ebx+8],eax
;--------------------------------------
align 4
.exit_2:
	mcall	-1
;---------------------------------------------------------------------
align 4
get_window_param:
	mcall	9,procinfo,-1
	mov	eax,[ebx+66]
	inc	eax
;	mov	[window_high],eax
	mov	eax,[ebx+62]
	inc	eax
;	mov	[window_width],eax
	mov	eax,[ebx+70]
;	mov	[window_status],eax
	ret
;---------------------------------------------------------------------
align 4
get_communication_area:
	xor	eax,eax
	mov	al,[param]
	test	eax,eax
	jz	@f
	mcall	68,22,param,,0x01
	mov	[communication_area],eax
;	movzx	ebx,word [eax+2]
;	mov	[color_dialog_type],ebx

	mov	ebx,[eax+4]
;	cmp	bx,word x_minimal_size ;300
;	jb	@f
	mov	bx,510
	mov	[window_x],ebx
	mov	ebx,[eax+8]
;	cmp	bx,word y_minimal_size ;200
;	jb	@f
	mov	bx,340
	mov	[window_y],ebx
@@:
	ret
;---------------------------------------------------------------------
align 4
get_active_pocess:
	mcall	9,procinfo,-1
	mov	ecx,[ebx+30]	; PID
	mcall	18,21
	mov	[active_process],eax	; WINDOW SLOT
	mov	ebx,[communication_area]
	test	ebx,ebx
	jz	.1
	mov	[ebx+12],eax	; WINDOW SLOT to com. area
.1:
	ret
;---------------------------------------------------------------------
align 4
clear_colors_history:
	mov	edi,[communication_area]
	cmp	[edi+24],dword 1
	je	@f
	mov	[edi+24],dword 1
	add	edi,28
	mov	ecx,10
	cld
	mov	eax,0xffffff
	rep	stosd
@@:
	ret
;---------------------------------------------------------------------
align 4
scroll_colors_history:
	mov	edi,[communication_area]
	add	edi,28
	mov	eax,[selected_color]
	cmp	[edi],eax
	je	@f
	mov	ecx,9
	mov	esi,edi
	add	esi,32
	add	edi,36
	std
	rep	movsd
	mov	edi,[communication_area]
	mov	[edi+28],eax
@@:
	ret
;---------------------------------------------------------------------
align 4
palette_button:
	mcall	37,1
	and	eax,0xffff
	sub	eax,p_start_y
	imul	eax,p_size_x
	lea	eax,[eax+eax*2]
	add	eax,[palette_area]
	mov	eax,[eax]
	mov	[tone_color],eax
	mov	[selected_color],eax
	call	prepare_scrollbars_position_from_color
	call	create_and_draw_tone
	call	draw_selected_color
	call	draw_scrollbars
	jmp	still
;---------------------------------------------------------------------
align 4
tone_button:
	mcall	37,1
	mov	ebx,eax
	and	eax,0xffff
	shr	ebx,16
	sub	eax,t_start_y
	imul	eax,p_size_y
	sub	ebx,t_start_x
	add	eax,ebx
	lea	eax,[eax+eax*2]
	add	eax,[tone_area]
	mov	eax,[eax]
	mov	[selected_color],eax
	call	prepare_scrollbars_position_from_color
	call	draw_selected_color
	call	draw_scrollbars
	jmp	still
;---------------------------------------------------------------------
align 4
color_button:
	mov	eax,[communication_area]
	mov	[eax],word 1
	mov	ebx,[selected_color]
	and	ebx,0xffffff
	mov	[eax+20],ebx
; dps "CD flag value: OK "
	jmp	button.exit_1
;---------------------------------------------------------------------
align 4
prepare_scrollbars_position_from_color:
; in: eax = selected color
	movzx	ebx,al
	mov	[scroll_bar_data_blue.position],ebx
	shr	eax,8
	mov	bl,al
	mov	[scroll_bar_data_green.position],ebx
	shr	eax,8
	mov	bl,al
	mov	[scroll_bar_data_red.position],ebx
	ret
;---------------------------------------------------------------------
align 4
prepare_color_from_scrollbars_position:
; out: ebx = selected color
	mov	eax,[scroll_bar_data_red.position]
	movzx	ebx,al
	shl	ebx,8
	mov	eax,[scroll_bar_data_green.position]
	mov	bl,al
	shl	ebx,8
	mov	eax,[scroll_bar_data_blue.position]
	mov	bl,al
	ret
;---------------------------------------------------------------------
align 4
key:
	mcall	2
	jmp	still
;---------------------------------------------------------------------
align 4
mouse:
	cmp	[scroll_bar_data_red.delta2],0
	jne	.red
	cmp	[scroll_bar_data_green.delta2],0
	jne	.green
	cmp	[scroll_bar_data_blue.delta2],0
	jne	.blue
;--------------------------------------
align 4
.red:
	push	dword scroll_bar_data_red
	call	[scrollbar_ver_mouse]
	cmp	[scroll_bar_data_red.delta2],0
	jne	@f
;--------------------------------------
align 4
.green:
	push	dword scroll_bar_data_green
	call	[scrollbar_ver_mouse]
	cmp	[scroll_bar_data_green.delta2],0
	jne	@f
;--------------------------------------
align 4
.blue:
	push	dword scroll_bar_data_blue
	call	[scrollbar_ver_mouse]
;	cmp	[scroll_bar_data_blue.delta2],0
;	jne	@f
;--------------------------------------
align 4
@@:
	call	prepare_color_from_scrollbars_position
	cmp	[selected_color],ebx
	je	still
	mov	[selected_color],ebx
	call	draw_selected_color
	jmp	still
;---------------------------------------------------------------------
align 4
draw_selected_color:
	mcall	13,<c_start_x,c_size_x>,<c_start_y,c_size_y>,[selected_color]
	mcall	13,<c_start_x+c_size_x+10,c_size_x>,<c_start_y,c_size_y>,0xffffff
	mov	ecx,[selected_color]
	and	ecx,0xffffff
	mcall	47,0x00060100,,<c_start_x+c_size_x+13,c_start_y+(c_size_y-6)/2>,0
	ret
;---------------------------------------------------------------------
align 4
create_and_draw_tone:
	call    create_tone
	call    draw_tone
	ret
;---------------------------------------------------------------------
align 4
draw_tone:
	mcall	65,[tone_area],<[tone_SIZE_X],[tone_SIZE_Y]>,<t_start_x,t_start_y>,24
	ret
;---------------------------------------------------------------------
draw_scrollbars:
	push	dword scroll_bar_data_red
	call	[scrollbar_ver_draw]
	push	dword scroll_bar_data_green
	call	[scrollbar_ver_draw]
	push	dword scroll_bar_data_blue
	call	[scrollbar_ver_draw]
	ret
;---------------------------------------------------------------------
align 4
draw_history_frame:
	mov	[frame_data.x],dword (c_start_x+c_size_x*2+10*2)*65536+80
	mov	[frame_data.y],dword (p_start_y+5)*65536+(p_size_y-5)

	mov	[frame_data.draw_text_flag],dword 1

	mov	[frame_data.text_pointer],dword history_text
	push	dword frame_data
	call	[Frame_draw]
	ret
;---------------------------------------------------------------------
align 4
draw_button_row:
	mov	edx,0x60000000 + 30		; BUTTON ROW
;	mov	ebx,220*65536+14
	mov	ebx,(c_start_x+c_size_x*2+10*3)*65536+14
	mov	ecx,25*65536+14
	mov	eax,8
;-----------------------------------
align 4
.newb:
	mcall
	add	ecx,24*65536
	inc	edx
	cmp	edx,0x60000000 + 39
	jbe	.newb
	ret
;---------------------------------------------------------------------
align 4
draw_color_value:
	mov	ebx,(c_start_x+c_size_x*3+10)*65536+(c_size_x-1)
	mov	ecx,28*65536+11
	mov	edx,0xffffff
	mov	eax,13
	mov	edi,10
	mov	esi,[communication_area]
	add	esi,28
;-----------------------------------
align 4
@@:
	mcall
	pusha
	mov	edx,ebx
	add	edx,2 shl 16
	shr	ecx,16
	mov	dx,cx
	add	dx,2
	mov	ecx,[esi]
	and	ecx,0xffffff
	mcall	47,0x00060100,,,0
	popa

	add	ecx,24*65536
	add	esi,4
	dec	edi
	jnz	@b

	ret
;---------------------------------------------------------------------
align 4
draw_colours:
	mov	edi,10
	mov	esi,[communication_area]
	add	esi,28
;	mov	ebx,220*65536+14
	mov	ebx,(c_start_x+c_size_x*2+10*3)*65536+14
	mov	ecx,27*65536+14
	mov	eax,13
	mov	[frame_data.draw_text_flag],dword 0
;--------------------------------------
align 4
newcol:
	mov	edx,[esi]
	mcall

	push	ebx ecx

	sub	ebx,2 shl 16
	add	bx,4
	sub	ecx,2 shl 16
	add	cx,4

	mov	[frame_data.x],ebx
	mov	[frame_data.y],ecx

	push	dword frame_data
	call	[Frame_draw]

	pop	ecx ebx

	add	ecx,24*65536
	add	esi,4

	dec	edi
	jnz	newcol

	ret
;----------------------------------------------------------------------
align 4
draw_window:
	mcall	12,1
;	mcall	0, <w_start_x,w_size_x>, <w_start_y,w_size_y>, 0x33AABBCC,,title
	xor	esi,esi
	mcall	0,[window_x],[window_y], 0x34EEEeee,,title
	mcall	8,<p_start_x,[palette_SIZE_X]>,<p_start_y,[palette_SIZE_Y]>,0x60000002
	mcall	,<t_start_x,[tone_SIZE_X]>,<t_start_y,[tone_SIZE_Y]>,0x60000003
	mcall	,<296,80>,<280,22>,4,0x37A4D4
	mcall	,<402,80>,        ,1
	mcall   4,<332,289>,0x802C7B9E,OK_Cancel
	mcall   ,<331,288>,0x80FFFfff
	xor	ebp,ebp
	mcall	65,[palette_area],<[palette_SIZE_X],[palette_SIZE_Y]>,<p_start_x,p_start_y>,24
	call	draw_tone
	call	draw_selected_color
	xor	eax,eax
	inc	eax
	mov	[scroll_bar_data_red.all_redraw],eax
	mov	[scroll_bar_data_green.all_redraw],eax
	mov	[scroll_bar_data_blue.all_redraw],eax
	call	draw_scrollbars
	call	draw_history_frame
	call	draw_button_row
	call	draw_colours
	call	draw_color_value
	mcall	12,2
	ret
;---------------------------------------------------------------------
include 'palette.inc'
;---------------------------------------------------------------------
include 'tone.inc'
;---------------------------------------------------------------------
include 'i_data.inc'
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
include 'u_data.inc'
;---------------------------------------------------------------------
I_END:
;---------------------------------------------------------------------
