;---------------------------------------------------------------------
; ÓÑÒÀÍÎÂÈÒÜ ÔÎÍ
background:
	cmp	[soi],0
	je	.end

	cmp	[bgrmode],dword 2
	ja	.stretch_crop

	mov	ecx,[img_width]	;	øèðèíà
	test	ecx,ecx
	jz	.end
	mov	edx,[img_high]	; âûñîòà 
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
;	cmp	[bgrmode],dword 3
;	jne	.stretch_inscribe
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
	push	dword 0				; edi - background color
	push	dword 1				; esi - filtering
	push	dword 0				; edx - scaling mode
	push	dword [stretch_start_coordinates]	; ecx - start_coordinates
	push	ebx				; ebx - new_size
	push	dword image_file		; eax -  convert data table

	call	[Scaling_Start]
	call	background_1
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
;.stretch_inscribe:
;	ret
;---------------------------------------------------------------------
background_1:
	mov	ecx,[screen_width]	;[crop_img_width]	; øèðèíà
	test	ecx,ecx
	jz	.end
	mov	edx,[screen_high]	;[crop_img_high]	; âûñîòà 
	test	edx,edx
	jz	.end
	mcall	15,1	; set size

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