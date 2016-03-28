;----------------------------------------------------------
;--------Move picture from array to work screeen-----------
;----------------------------------------------------------
MovePictureToWorkScreen:
	call	cleare_screen
	mov	eax,[Picture_SizeX]
	mov	ebx,[Picture_SizeY]
	mov	ecx,[MaxWorkScreen_SizeX]
	mov	edx,[MaxWorkScreen_SizeY]
	mov	esi,[k]
	imul	eax,esi
	imul	ebx,esi
	cmp	eax,ecx
	jle	lab1

	mov	eax,[MaxWorkScreen_SizeX]
	mov	[WorkScreen_SizeX],eax
	jmp	lab2

lab1:
	mov	[WorkScreen_SizeX],eax

lab2:
	cmp	ebx,edx
	jle	lab3
	mov	ebx,[MaxWorkScreen_SizeY]
	mov	[WorkScreen_SizeY],ebx
	jmp	lab4

lab3:
	mov	[WorkScreen_SizeY],ebx

lab4:
	mov	eax,[WorkScreen_SizeX]
	mov	ebx,[k]
	cdq
	idiv	ebx
	mov	[CounterX],eax
	mov	eax,[WorkScreen_SizeY]
	cdq
	idiv	ebx
	mov	[CounterY],eax

	mov	eax,[WorkScreen_SizeX]
	mov	ecx,eax
	mov	ebx,[k]
	cdq
	idiv	ebx
	imul	eax,ebx
	sub	ecx,eax
	lea	ecx,[ecx+ecx*2]

	mov	eax,[WorkScreen_SizeX]
	mov	ebx,[k]
	dec	ebx
	imul	eax,ebx
	lea	eax,[eax+eax*2]
	add	eax,ecx
	mov	[OffsetYWorkScreen],eax

	mov	ebx,[Picture_SizeX]
	mov	eax,[CounterX]
	sub	ebx,eax
	lea	ebx,[ebx+ebx*2]
	mov	[OffsetYPicture],ebx

	mov	eax,[WorkScreen_SizeX]
	mov	ebx,[k]
	sub	eax,ebx
	lea	eax,[eax+eax*2]
	mov	[OffsetYBigPixel],eax

	mov	eax,[PosX]
	mov	ebx,[PosY]
	mov	ecx,[Picture_SizeX]
	imul	ecx,ebx
	add	eax,ecx
	lea	eax,[eax+eax*2]
	add	eax,[PointerToPicture]

	mov	ebx,[ScreenPointer]
	mov	edi,[CounterY]

;if size of picture natural(mastab is 1) than move picture to work screen
	cmp	[k],1
	jne	no_zoom_1_

screen_y_1:
	mov	esi,[CounterX]

screen_x_1:
	mov	ecx,[eax]
	mov	ebp,ecx
	shr	ecx,16
	mov	[ebx],bp
	mov	[ebx+2],cl
	add	ebx,3
	add	eax,3
	dec	esi
	jnz	screen_x_1
	add	eax,[OffsetYPicture]
	add	ebx,[OffsetYWorkScreen]
	dec	edi
	jnz	screen_y_1
	jmp	fps

no_zoom_1_:
	cmp	[k],2
	jne	no_zoom_2

screen_y_2:
	mov	esi,[CounterX]

screen_x_2:
	mov	ecx,[eax]
	mov	ebp,ecx
	shr	ecx,16
	mov	edx,ebx
	mov	[edx],bp
	mov	[edx+2],cl
	mov	[edx+3],bp
	mov	[edx+3+2],cl
	add	edx,3*2
	add	edx,[OffsetYBigPixel]
	mov	[edx],bp
	mov	[edx+2],cl
	mov	[edx+3],bp
	mov	[edx+3+2],cl
	add	ebx,3*2
	add	eax,3
	dec	esi
	jnz	screen_x_2
	add	eax,[OffsetYPicture]
	add	ebx,[OffsetYWorkScreen]
	dec	edi
	jnz	screen_y_2
	jmp	fps

no_zoom_2:
	cmp	[k],4
	jne	no_zoom_4

screen_y_4:
	mov	esi,[CounterX]

screen_x_4:
	mov	ecx,[eax]
	mov	ebp,ecx
	shr	ecx,16
	mov	edx,ebx
	mov	[edx],bp
	mov	[edx+2],cl
	mov	[edx+3],bp
	mov	[edx+3+2],cl
	mov	[edx+6],bp
	mov	[edx+6+2],cl
	mov	[edx+9],bp
	mov	[edx+9+2],cl
	add	edx,3*4
	add	edx,[OffsetYBigPixel]
	mov	[edx],bp
	mov	[edx+2],cl
	mov	[edx+3],bp
	mov	[edx+3+2],cl
	mov	[edx+6],bp
	mov	[edx+6+2],cl
	mov	[edx+9],bp
	mov	[edx+9+2],cl
	add	edx,3*4
	add	edx,[OffsetYBigPixel]
	mov	[edx],bp
	mov	[edx+2],cl
	mov	[edx+3],bp
	mov	[edx+3+2],cl
	mov	[edx+6],bp
	mov	[edx+6+2],cl
	mov	[edx+9],bp
	mov	[edx+9+2],cl
	add	edx,3*4
	add	edx,[OffsetYBigPixel]
	mov	[edx],bp
	mov	[edx+2],cl
	mov	[edx+3],bp
	mov	[edx+3+2],cl
	mov	[edx+6],bp
	mov	[edx+6+2],cl
	mov	[edx+9],bp
	mov	[edx+9+2],cl
	add	ebx,4*3
	add	eax,3
	dec	esi
	jnz	screen_x_4
	add	eax,[OffsetYPicture]
	add	ebx,[OffsetYWorkScreen]
	dec	edi
	jnz	screen_y_4
	jmp	fps

no_zoom_4:
;if zoom more than 4
screen_y:
	mov	esi,[CounterX]

screen_x:
	mov	ecx,[eax]
	mov	ebp,ecx
	shr	ecx,16
;---------------------------------------------------------
	push	ebx eax
	mov	edx,[k]

big_pixel_y:
	mov	eax,[k]

big_pixel_x:
	mov	[ebx],bp
	mov	[ebx+2],cl
	add	ebx,3
	dec	eax
	jnz	big_pixel_x
	add	ebx,[OffsetYBigPixel]
	dec	edx
	jnz	big_pixel_y
	pop	eax ebx
;---------------------------------------------------------
	mov	edx,[k]
	lea	edx,[edx+edx*2]
	add	ebx,edx
	add	eax,3
	dec	esi
	jnz	screen_x
	add	eax,[OffsetYPicture]
	add	ebx,[OffsetYWorkScreen]
	dec	edi
	jnz	screen_y

fps:
	;calculate selection rect
	cmp [crossing],0
	je .no_selection_rect
	mov edx,[rectangular_shade_y]
	sub edx,[PosY]
	mov ecx,[rectangular_shade_x]
	sub ecx,[PosX]
	mov ebx,[crossing_old_y]
	sub ebx,[PosY]
	mov eax,[crossing_old_x]
	sub eax,[PosX]
	cmp [k],1
	je .zo_1
		cmp eax,ecx
		jle @f
			xchg eax,ecx
		@@:
		cmp ebx,edx
		jle @f
			xchg ebx,edx
		@@:
		inc eax
		inc ebx
		imul eax,[k]
		imul ebx,[k]
		imul ecx,[k]
		imul edx,[k]
		dec eax
		dec ebx
	.zo_1:
	stdcall draw_selection_rect, eax,ebx,ecx,edx
	.no_selection_rect:

	mov	ecx,[WorkScreen_SizeX]
	shl	ecx,16
	add	ecx,[WorkScreen_SizeY]
	mov	edx,((ci_edit_wnd_x_pos+ci_edit_wnd_border) shl 16)\
		+ (ci_edit_wnd_y_pos+ci_edit_wnd_border)
	mcall SF_PUT_IMAGE, [ScreenPointer]
	ret
;----------------------------------------------------------
align 4
proc draw_selection_rect, x0:dword, y0:dword, x1:dword, y1:dword
	mov eax,0xffff3fff ;2:2
	mov edx,0xff1f ;1:1

	mov edi,[x0]
	mov ebx,[y0]
	mov ecx,[x1]
	cmp edi,ecx
	jle @f
		xchg edi,ecx
	@@:
	call draw_line_h
	mov ebx,[y1]
	call draw_line_h

	mov edi,[x0]
	mov ecx,[y0]
	cmp ebx,ecx
	jle @f
		xchg ebx,ecx
	@@:
	call draw_line_w
	mov edi,[x1]
	call draw_line_w
	ret
endp

;input:
; eax - color1(g,r):color2(g,r)
;  dx - color1(b):color2(b)
; edi - x0
; ebx - y0
; ecx - x1 (x1 >= x0)
align 4
draw_line_h:
	cmp edi,ecx
	je .end_f
	cmp ecx,1
	jl .end_f
	cmp edi,[WorkScreen_SizeX]
	jge .end_f
	cmp ebx,0
	jl .end_f
	cmp ebx,[WorkScreen_SizeY]
	jge .end_f
push ebx ecx edi
	cmp ecx,[WorkScreen_SizeX]
	jl @f
		mov ecx,[WorkScreen_SizeX]
	@@:
	cmp edi,0
	jge @f
		xor edi,edi
	@@:
	sub ecx,edi
	imul ebx,[WorkScreen_SizeX]
	add edi,ebx
	lea	edi,[edi+edi*2]
	add edi,[ScreenPointer]
	cld
	.line_t:
		stosw
		mov byte[edi],dl
		inc edi
		bt ecx,1
		jc @f
		bt ecx,0
		jc @f
			ror eax,16
			ror dx,8
		@@:
		dec ecx
		jnz .line_t
pop edi ecx ebx
	.end_f:
	ret

;input:
; eax - color1(g,r):color2(g,r)
;  dx - color1(b):color2(b)
; edi - x0
; ebx - y0
; ecx - y1 (y1 >= y0)
align 4
draw_line_w:
	cmp edi,0
	jl .end_f
	cmp edi,[WorkScreen_SizeX]
	jge .end_f
	cmp ebx,ecx
	je .end_f
	cmp ebx,[WorkScreen_SizeY]
	jge .end_f
	cmp ecx,1
	jl .end_f
push ebx ecx edi
	cmp ecx,[WorkScreen_SizeY]
	jl @f
		mov ecx,[WorkScreen_SizeY]
	@@:
	cmp ebx,0
	jge @f
		xor ebx,ebx
	@@:
	sub ecx,ebx
	cmp ecx,1
	jl .end_f
	imul ebx,[WorkScreen_SizeX]
	add edi,ebx
	lea	edi,[edi+edi*2]
	add edi,[ScreenPointer]
	mov	ebx,[WorkScreen_SizeX]
	lea	ebx,[ebx+ebx*2]
	sub ebx,2
	cld
	.line_r:
		stosw
		mov byte[edi],dl
		add edi,ebx
		bt ecx,1
		jc @f
		bt ecx,0
		jc @f
			ror eax,16
			ror dx,8
		@@:
		dec ecx
		jnz .line_r
pop edi ecx ebx
	.end_f:
	ret
;----------------------------------------------------------
;--------------------clear screen--------------------------
;----------------------------------------------------------
align 4
cleare_screen:
	mov edi,[ScreenPointer]
	mov ebx,[WorkScreen_SizeX]
	imul ebx,[WorkScreen_SizeY]
	lea ebx,[ebx+ebx*2]
	mov eax,0xffffffff
	mov ecx,ebx
	shr ecx,2
	cld
	rep stosd
	mov ecx,ebx
	and ecx,3
	rep stosb
	ret
;----------------------------------------------------------
;-------------cleare work arrea(work screen)---------------
;----------------------------------------------------------
cleare_work_arrea:
	mov edi,[PointerToPicture]
	mov ebx,[Picture_SizeX]
	imul ebx,[Picture_SizeY]
	lea ebx,[ebx+ebx*2]
	mov eax,0xffffffff
	mov ecx,ebx
	shr ecx,2
	cld
	rep stosd
	mov ecx,ebx
	and ecx,3
	rep stosb
	ret
;-----------------------------------------------------------
;calculate position work screen on a picture
;-----------------------------------------------------------
CalculatePositionScreen:
	mov eax,[Picture_SizeX]
	sub eax,[CounterX]
	cmp [PosX],eax
	jle @f
		mov [PosX],eax
	@@:
	cmp [PosX],0
	jns @f
		mov [PosX],0
	@@:
	mov ebx,[Picture_SizeY]
	sub ebx,[CounterY]
	cmp [PosY],ebx
	jle @f
		mov [PosY],ebx
	@@:
	cmp [PosY],0
	jns @f
		mov [PosY],0
	@@:
	ret
;-----------------------------------------------------------
;-----------calculate cordinats on work picture--------------
;-----------------------------------------------------------
GetScreenCordinats:
	mov eax,[MouseX]
	mov ebx,[MouseY]
	sub eax,ci_edit_wnd_x_pos+ci_edit_wnd_border
	cmp eax,0
	jge @f
		xor eax,eax
	@@:
	sub ebx,ci_edit_wnd_y_pos+ci_edit_wnd_border
	cmp ebx,0
	jge @f
		xor ebx,ebx
	@@:
	mov ecx,[k]
	cdq
	idiv ecx
	mov [ScreenX],eax
	mov eax,ebx
	cdq
	idiv ecx
	mov [ScreenY],eax
	ret