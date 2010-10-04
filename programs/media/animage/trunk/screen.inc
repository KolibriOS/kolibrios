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
	and	ecx,0xffffff
	mov	ebp,ecx
	shr	ecx,16
	mov	[ebx],bp
	mov	[ebx+2],cl
	add	ebx,3;
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
	and	ecx,0xffffff
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
	add	edx,3*2
	add	edx,[OffsetYBigPixel]
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
	and	ecx,0xffffff
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
	add	edx,3*4
	add	edx,[OffsetYBigPixel]
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
	and	ecx,0xffffff
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
	;mcall	18,14
	mov	ebx,[ScreenPointer]
	mov	ecx,[WorkScreen_SizeX]
	mov	edx,[WorkScreen_SizeY]
	shl	ecx,16
	add	ecx,edx
	mov	edx,8*65536+20+15+49+5+1

	mcall	7
	ret
;----------------------------------------------------------
;--------------------clear screen--------------------------
;----------------------------------------------------------
cleare_screen:
	mov	eax,[ScreenPointer]
	mov	ebx,[WorkScreen_SizeX]
	imul	ebx,[WorkScreen_SizeY]
	lea	ebx,[ebx+ebx*2]
	shr	ebx,3
	inc	ebx
	mov	esi,0xffffff

clear_screen_loop:
	mov	[eax],esi
	mov	[eax+3],esi
	mov	[eax+6],si
	add	eax,4+4
	dec	ebx
	jnz	clear_screen_loop
	ret
;----------------------------------------------------------
;-------------cleare work arrea(work screen)---------------
;----------------------------------------------------------
cleare_work_arrea:
	mov	eax,[PointerToPicture]
	mov	ebx,[Picture_SizeX]
	imul	ebx,[Picture_SizeY]
	lea	ebx,[ebx+ebx*2]
	shr	ebx,3
	inc	ebx
	mov	esi,0xffffff

clear_work_arrea_loop:
	mov	[eax],esi
	mov	[eax+3],esi
	mov	[eax+6],si
	add	eax,4+4
	dec	ebx
	jnz	clear_work_arrea_loop
	ret
;-----------------------------------------------------------
;calculate position work screen on a picture
;-----------------------------------------------------------
CalculatePositionScreen:
	mov	eax,[Picture_SizeX]
	mov	ebx,[Picture_SizeY]
	mov	ecx,[CounterX]
	mov	edx,[CounterY]
	sub	eax,ecx
	sub	ebx,edx
	cmp	[PosX],eax
	jle	no_limit_screen_x
	mov	[PosX],eax

no_limit_screen_x:
	cmp	[PosY],ebx
	jle	no_limit_screen_y
	mov	[PosY],ebx

no_limit_screen_y:
	cmp	[PosX],0
	jns	no_minimum_screen_x
	mov	[PosX],0

no_minimum_screen_x:
	cmp	[PosY],0
	jns	no_minimum_screen_y
	mov	[PosY],0

no_minimum_screen_y:
	ret
;-----------------------------------------------------------
;-----------calculate cordinats on work picture--------------
;-----------------------------------------------------------
GetScreenCordinats:
	mov	eax,[MouseX]
	mov	ebx,[MouseY]
	sub	eax,9
	sub	ebx,87
	mov	ecx,[k]
	cdq
	idiv	ecx
	mov	[ScreenX],eax
	mov	eax,ebx
	cdq
	idiv	ecx
	mov	[ScreenY],eax
	ret