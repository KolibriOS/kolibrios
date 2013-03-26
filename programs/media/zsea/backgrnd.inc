;---------------------------------------------------------------------
; УСТАНОВИТЬ ФОН
background:
	cmp	[soi],0
	je	.end

	cmp	[bgrmode],dword 2
	ja	.stretch_crop

	mov	ecx,[img_width]	;	ширина
	test	ecx,ecx
	jz	.end
	mov	edx,[img_high]	; высота 
	test	edx,edx
	jz	.end
	mcall	15,1	; set size

	mov	esi,ecx
	imul	esi,edx
	lea	esi,[esi*3]
	mov	ecx,[soi]

	xor	edx,edx

	cmp	[img_resolution],24
	je	@f
	
	push	dword image_file
	call	[convert_Conv_24b]
	
	mov	ecx,[raw_pointer_2]
	add	ecx,44
	mcall	15,5		;15,5 set data
	mcall	68,13,[raw_pointer_2]
	jmp	.set_mode
@@:
	mcall	15,5		; 15,5 set data
.set_mode:
	mcall	15,4,[bgrmode]	; 15,4 set mode
	dec	ebx		; 15,3 redraw background
	mcall
.end:
	ret
;---------------------------------------------------------------------
.stretch_crop:
	cmp	[bgrmode],dword 3
	jne	.stretch_inscribe
	xor	eax,eax
	mov	[stretch_start_coordinates],eax
    
	mcall	14

	mov	ebx,eax
	and	eax,0xffff
	inc	eax
	mov	[screen_high],eax
	shr	ebx,16
	inc	ebx
	mov	[screen_width],ebx
	
	xchg	eax,ebx
	call	calculate_relation
	mov	[screen_relation],eax
	
	mov	eax,[img_width]
	mov	ebx,[img_high]
	call	calculate_relation
	mov	[picture_relation],eax
	
	cmp	eax,[screen_relation]
	jb	@f
; Y = Yimg
; X = Yimg*Xscreen/Yscreen
	mov	eax,[img_high]
	mov	[stretch_img_high],eax
	mov	eax,[img_high]
	imul	eax,dword [screen_width]
	mov	ebx,[screen_high]
	call	integer_division	
	mov	[stretch_img_width],eax
	jmp	.continue
@@:
; X = Ximg
; Y = Ximg*Yscreen/Xscreen
	mov	eax,[img_width]
	mov	[stretch_img_width],eax
	mov	eax,[img_width]
	imul	eax,dword [screen_high]
	mov	ebx,[screen_width]
	call	integer_division
	mov	[stretch_img_high],eax
.continue:
	mov	eax,[Stretch_Crop_Corner]
	mov	ebx,eax
	shr	eax,16
	shl	eax,2
	mov	eax,[eax+call_to_calculate_stretch_start_coordinates_Y]
	call	eax
	and	ebx,0xffff
	shl	ebx,2
	mov	ebx,[ebx+call_to_calculate_stretch_start_coordinates_X]	
	call	ebx
	
	mov	ebx,[screen_width]
	shl	ebx,16
	add	ebx,[screen_high]
	
	mov	eax,[stretch_img_width]
	shl	eax,16
	add	eax,[stretch_img_high]	
	
	push	dword eax			; eax - crop size
	push	dword [background_color]	; edi - background color
	push	dword 1				; esi - filtering
	push	dword 0				; edx - scaling mode
	push	dword [stretch_start_coordinates]	; ecx - start_coordinates
	push	ebx				; ebx - new_size
	push	dword image_file		; eax -  convert data table

	call	[Scaling_Start]
	call	background_1
	ret
;---------------------------------------------------------------------
.stretch_inscribe:
	xor	eax,eax
	mov	[stretch_start_coordinates],eax
    
	mcall	14

	mov	ebx,eax
	and	eax,0xffff
	inc	eax
	mov	[screen_high],eax
	shr	ebx,16
	inc	ebx
	mov	[screen_width],ebx
	
	xchg	eax,ebx
	call	calculate_relation
	mov	[screen_relation],eax
	
	mov	eax,[img_width]
	mov	ebx,[img_high]
	call	calculate_relation
	mov	[picture_relation],eax
	
	cmp	eax,[screen_relation]
	ja	@f
; Y = Yimg
; X = Yimg*Xscreen/Yscreen
	mov	eax,[screen_high]	;[img_high]
	mov	[stretch_img_high],eax
	mov	eax,[screen_high]	;[img_high]
	imul	eax,dword [img_width]	;[screen_width]
	mov	ebx,[img_high]		;[screen_high]
	call	integer_division	
	mov	[stretch_img_width],eax
	jmp	.continue_1
@@:
; X = Ximg
; Y = Ximg*Yscreen/Xscreen
	mov	eax,[screen_width]	;[img_width]
	mov	[stretch_img_width],eax
	mov	eax,[screen_width]	;[img_width]
	imul	eax,dword [img_high]	;[screen_high]
	mov	ebx,[img_width]		;[screen_width]
	call	integer_division
	mov	[stretch_img_high],eax
.continue_1:
;	mov	ebx,[screen_width]
;	shl	ebx,16
;	add	ebx,[screen_high]
	
	mov	ebx,[stretch_img_width]
	shl	ebx,16
	add	ebx,[stretch_img_high]	

	
	push	dword 0				; eax - crop size
	push	dword [background_color]	; edi - background color
	push	dword 1				; esi - filtering
	push	dword 0				; edx - scaling mode
	push	dword [stretch_start_coordinates]	; ecx - start_coordinates
	push	ebx				; ebx - new_size
	push	dword image_file		; eax -  convert data table

	call	[Scaling_Start]
	
	mov	eax,[raw_pointer_2]
	mov	[crop_raw_pointer],eax
	mov	[crop_raw_pointer_2],eax

	mov	eax,[raw_pointer_2]
	mov	eax,[eax+12]		;overall depth of the pixel
	cmp	al,24
	je	@f
	
	push	dword crop_image_file
	call	[convert_Conv_24b]
	
	mov	ecx,[raw_pointer_2]
	mcall	68,13
@@:
	mov	ecx,[screen_width]
	imul	ecx,[screen_high]
	lea	ecx,[ecx*3]
	add	ecx,44
	
;	mcall	68,20,,[crop_raw_pointer_2]
;	mov	[crop_raw_pointer_2],eax
	mov	eax,[crop_raw_pointer_2]
	mov	[crop_raw_pointer],eax

	mcall	68,12
	mov	[crop_raw_pointer_2],eax

; fill of color	
	sub	ecx,44
	mov	edi,eax
	add	edi,44
	mov	eax,[background_color]	;0xaaaaaa
	cld
@@:
	stosb
	ror	eax,8
	stosb
	ror	eax,8
	stosb
	rol	eax,16
	sub	ecx,2
	loop	@b

; copy of data
	mov	eax,[screen_width]
	sub	eax,[stretch_img_width]
	mov	ebx,eax
	lea	eax,[eax*3]
	shr	ebx,1
	lea	ebx,[ebx*3]
	
	mov	edi,[screen_high]
	sub	edi,[stretch_img_high]
	shr	edi,1
	imul	edi,[screen_width]
	lea	edi,[edi*3]
	add	edi,[crop_raw_pointer_2]
	add	edi,44
	add	edi,ebx
	
	mov	esi,[crop_raw_pointer]
	add	esi,44

	mov	ebx,[stretch_img_width]
	lea	ebx,[ebx*3]
	
	mov	ecx,[stretch_img_high]
@@:
	push	ecx
	mov	ecx,ebx
	rep	movsb
	pop	ecx
	add	edi,eax
	loop	@b
	
; header
	mov	esi,[crop_raw_pointer]	
	mov	edi,[crop_raw_pointer_2]
	mov	ecx,44/4
	rep	stosd

	mcall	68,13,[crop_raw_pointer]
;set the background	
	mov	eax,[crop_raw_pointer_2]
	mov	ebx,[screen_width]	
	mov	[eax+4],ebx
	mov	ebx,[screen_high]
	mov	[eax+8],ebx
	
	mov	ecx,[screen_width]	; ширина
	test	ecx,ecx
	jz	@f
	mov	edx,[screen_high]	; высота 
	test	edx,edx
	jz	@f
	mcall	15,1	; set size
	
	mov	esi,ecx
	imul	esi,edx
	lea	esi,[esi*3]
	
	xor	edx,edx
	mov	ecx,[crop_raw_pointer_2]
	add	ecx,44
	mcall	15,5		;15,5 set data
	xor	ecx,ecx
	inc	ecx
	mcall	15,4		; 15,4 set mode
	dec	ebx		; 15,3 redraw background
	mcall
@@:
	mcall	68,13,[crop_raw_pointer_2]
	ret	
;---------------------------------------------------------------------
call_to_calculate_stretch_start_coordinates_X:
	dd	calculate_stretch_start_coordinates_XL
	dd	calculate_stretch_start_coordinates_XC
	dd	calculate_stretch_start_coordinates_XR
;---------------------------------------------------------------------
call_to_calculate_stretch_start_coordinates_Y:
	dd	calculate_stretch_start_coordinates_YU
	dd	calculate_stretch_start_coordinates_YC
	dd	calculate_stretch_start_coordinates_YD
;---------------------------------------------------------------------
calculate_stretch_start_coordinates_XC:
	xor	ecx,ecx
.1:
	mov	eax,[img_width]
	sub	eax,[stretch_img_width]
	test	ecx,ecx
	jnz	@f
	shr	eax,1
@@:
	mov	[stretch_start_coordinates.x],ax
calculate_stretch_start_coordinates_XL:
	ret
;---------------------------------------------------------------------
calculate_stretch_start_coordinates_XR:
	xor	ecx,ecx
	inc	ecx
	jmp	calculate_stretch_start_coordinates_XC.1
;---------------------------------------------------------------------
calculate_stretch_start_coordinates_YC:
	xor	ecx,ecx
.1:
	mov	eax,[img_high]
	sub	eax,[stretch_img_high]
	test	ecx,ecx
	jnz	@f
	shr	eax,1
@@:
	mov	[stretch_start_coordinates.y],ax
calculate_stretch_start_coordinates_YU:
	ret
;---------------------------------------------------------------------
calculate_stretch_start_coordinates_YD:
	xor	ecx,ecx
	inc	ecx
	jmp	calculate_stretch_start_coordinates_YC.1
;---------------------------------------------------------------------
background_1:
	mov	ecx,[screen_width]	;[crop_img_width]	; ширина
	test	ecx,ecx
	jz	.end
	mov	edx,[screen_high]	;[crop_img_high]	; высота 
	test	edx,edx
	jz	.end
	mcall	15,1	; set size
.1:
	mov	esi,ecx
	imul	esi,edx
	lea	esi,[esi*3]
	mov	ecx,[raw_pointer_2]	;[soi]
	mov	eax,[ecx+28]
	add	ecx,eax

	xor	edx,edx

	mov	eax,[raw_pointer_2]
	mov	eax,[eax+12]		;overall depth of the pixel
	cmp	al,24
	je	@f

	mov	eax,[raw_pointer_2]
	mov	[crop_raw_pointer],eax
	
	push	dword crop_image_file
	call	[convert_Conv_24b]
	
	mov	ecx,[crop_raw_pointer_2]
	add	ecx,44
	mcall	15,5		;15,5 set data
	mcall	68,13,[crop_raw_pointer_2]
	jmp	.set_mode
@@:
	mcall	15,5		; 15,5 set data
.set_mode:
	xor	ecx,ecx
	inc	ecx
;	mov	ecx,[bgrmode]
	mcall	15,4		; 15,4 set mode
	dec	ebx		; 15,3 redraw background
	mcall
.end:
	mov	ecx,[raw_pointer_2]
	mcall	68,13
	ret
;---------------------------------------------------------------------