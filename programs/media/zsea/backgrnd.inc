;---------------------------------------------------------------------
; УСТАНОВИТЬ ФОН
background:
	cmp	[soi],0
	je	.end
	cmp	[bgrmode],dword 2
	jbe	@f
	jmp	.end
@@:
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
	mcall	15,5	;15,5 set data
	mov	ecx,[raw_pointer_2]
	mcall	68,13
	jmp	.set_mode
@@:
	mcall	15,5	; 15,5 set data
.set_mode:
	mov	ecx,[bgrmode]
	mcall	15,4 ; 15,4 set mode
	dec	ebx ; 15,3 redraw background
	mcall
.end:
	ret
;---------------------------------------------------------------------