;----------------------------------------------------------
;input:
; eax - start X
; ebx - start Y
; ecx - size X
; edx - size Y
; esi - colos styles (1...7)
draw_volume_rectangle:
	mov	[line_x],eax
	mov	[line_y],ebx
	mov	[line_size_x],ecx
	mov	[line_size_y],edx
	cmp	esi,1
	jne	no_volume_light_rectangle

	;mov esi,dword[syscolors+20]
	mov	[color_rectangle],0xc7cbcf ;esi
	mov	[color_line1],0xaeabae
	mov	[color_line2],0xffffff
	mov	[color_line3],0xc7cbcf ;esi
	jmp	exit_colors
;----------------------------------------------------------
no_volume_light_rectangle:
	cmp	esi,2
	jne	no_pressed_rectangle

	mov	[color_rectangle],0x666666
	mov	[color_line1],0xaeabae
	mov	[color_line2],0x666666
	mov	[color_line3],0x666666
	jmp	exit_colors
;----------------------------------------------------------
no_pressed_rectangle:
	cmp	esi,3
	jne	no_pressed_panel

	mov esi,dword[syscolors+20]
	mov	[color_rectangle],esi
	mov	[color_line1],0xaeabae
	mov	[color_line2],7000
	mov	[color_line3],0xffffff
	jmp	exit_colors
;----------------------------------------------------------
no_pressed_panel:
	cmp	esi,4
	jne	no_pressed_light_panel

	mov	[color_rectangle],0xffffff
	mov	[color_line1],0xaeabae
	mov	[color_line2],7000
	mov	[color_line3],0xffffff
	jmp	exit_colors
;----------------------------------------------------------
no_pressed_light_panel:
	cmp	esi,5
	jne	no_light_pressed_rectangle

	mov	[color_rectangle],0x666666
	mov	[color_line1],0xaeabae
	mov	[color_line2],0x666666
	mov	[color_line3],0x666666
	jmp	exit_colors
;----------------------------------------------------------
no_light_pressed_rectangle:
	cmp	esi,6
	jne	no_work_rectangle

	mov esi,dword[syscolors+20]
	mov [color_rectangle],esi
	mov	[color_line1],0xaeabae
	mov	[color_line2],esi
	mov	[color_line3],esi
	jmp	exit_colors
;----------------------------------------------------------
no_work_rectangle:
	cmp	esi,7
	jne	no_work_rectangle_2

	mov esi,dword[syscolors+20]
	mov	[color_rectangle],esi
	mov	[color_line1],esi
	mov	[color_line2],esi
	mov	[color_line3],esi
	jmp	exit_colors
;----------------------------------------------------------
no_work_rectangle_2:
exit_colors:
;draw	rectangle
	mov	ebx,[line_x]
	mov	ecx,[line_y]
	shl	ebx,16
	shl	ecx,16
	add	ebx,[line_size_x]
	add	ecx,[line_size_y]
	mov	edx,[color_rectangle]
	mcall SF_DRAW_RECT
;line	1
	mov	edx,[color_line1]
	mov	ebx,[line_x]
	mov	ecx,[line_y]
	shl	ebx,16
	shl	ecx,16
	add	ebx,[line_x]
	add	ecx,[line_y]
	add	ebx,[line_size_x]
	call	draw_line
;line	2
	mov	edx,[color_line1]
	mov	ebx,[line_x]
	mov	ecx,[line_y]
	shl	ebx,16
	shl	ecx,16
	add	ebx,[line_x]
	add	ecx,[line_y]
	add	ecx,[line_size_y]
	call	draw_line
;line	3
	mov	edx,[color_line1]
	mov	ebx,[line_x]
	mov	ecx,[line_y]
	add	ecx,[line_size_y]
	shl	ebx,16
	shl	ecx,16
	add	ebx,[line_x]
	add	ecx,[line_y]
	add	ebx,[line_size_x]
	add	ecx,[line_size_y]
	call	draw_line
;line	4
	mov	edx,[color_line1]
	mov	ebx,[line_x]
	mov	ecx,[line_y]
	add	ebx,[line_size_x]
	shl	ebx,16
	shl	ecx,16
	add	ebx,[line_x]
	add	ecx,[line_y]
	add	ebx,[line_size_x]
	add	ecx,[line_size_y]
	call	draw_line
;light	line	1
	mov	edx,[color_line2]
	mov	ebx,[line_x]
	mov	ecx,[line_y]
	inc	ebx
	inc	ecx
	shl	ebx,16
	shl	ecx,16
	add	ebx,[line_x]
	add	ecx,[line_y]
	add	ebx,[line_size_x]
	dec	ebx
	inc	ecx
	call	draw_line
;light	line	2
	mov	edx,[color_line2]
	mov	ebx,[line_x]
	mov	ecx,[line_y]
	inc	ebx
	inc	ecx
	shl	ebx,16
	shl	ecx,16
	add	ebx,[line_x]
	add	ecx,[line_y]
	add	ecx,[line_size_y]
	dec	ecx
	inc	ebx
	call	draw_line
;light	line	3
	mov	edx,[color_line3]
	mov	ebx,[line_x]
	mov	ecx,[line_y]
	add	ebx,[line_size_x]
	dec	ebx
	inc	ecx
	shl	ebx,16
	shl	ecx,16
	add	ebx,[line_x]
	add	ecx,[line_y]
	add	ebx,[line_size_x]
	add	ecx,[line_size_y]
	dec	ebx
	dec	ecx
	call	draw_line
;light	line	4
	mov	edx,[color_line3]
	mov	ebx,[line_x]
	mov	ecx,[line_y]
	add	ecx,[line_size_y]
	inc	ebx
	dec	ecx
	shl	ebx,16
	shl	ecx,16
	add	ebx,[line_x]
	add	ecx,[line_y]
	add	ecx,[line_size_y]
	add	ebx,[line_size_x]
	dec	ecx
	dec	ebx
	call	draw_line
	ret
;----------------------------------------------------------
;input:
; eax - rectangle X
; ebx - rectangle Y
; ecx - point X
; edx - point Y
; esi - rectangle size X
; edi - rectangle size Y
;output:
; eax - 1 if point in rectangle
columnus:
	sub	eax,ecx
	jns	@f
	neg	eax
	cmp	eax,esi
	ja	@f
	sub	ebx,edx
	jns	@f
	neg	ebx
	cmp	ebx,edi
	ja	@f
	mov	eax,1
	jmp	columnus_true
@@:
	xor	eax,eax
columnus_true:
	ret
;----------------------------------------------------------
print_text:
	mov	[text_x],eax
	mov	[text_y],ebx
	mov	ebx,[text_x]
	shl	ebx,16
	add	ebx,[text_y]
	mcall SF_DRAW_TEXT
	ret
;----------------------------------------------------------
draw_line:
	mcall SF_DRAW_LINE
	ret
;----------------------------------------------------------
;lightlin 12508927
;lines 5669590
;workpan 9350879
;btnpress 9089258
line_x		dd	0
line_y		dd	0
line_size_x	dd	0
line_size_y	dd	0
color_line1	dd	0
color_line2	dd	0
color_line3	dd	0
color_rectangle	dd	0
;--------------------
text_x		dd	0
text_y		dd	0
;--------------------