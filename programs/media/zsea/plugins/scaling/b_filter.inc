;---------------------------------------------------------------------
align 4		
.check_filtering_24:
	cmp  [filtering],0
	je   .24_1
	
	mov  ebx,[temp_y1]
	inc  ebx
	cmp  ebx,[y]
	jae   .last_y_24  ;.24_1
	
	mov  ebx,[temp_x1]
	push eax
	mov  eax,[x]
	dec  eax
	lea  eax,[eax*3]
	cmp  ebx,eax
	pop  eax
	jae   .last_x_24 ;.24_1

;	mov  ebx,[esp+4]
;;	test ebx,ebx
;;	jz   .24_1
;	inc  ebx
;	cmp  bx,[new_size.y1]
;	jae   .last_y  ;.24_1

;	mov  ebx,[esp]
;;	test ebx,ebx
;;	jz   .24_1
;	inc  ebx
;	cmp  bx,[new_size.x1]
;	jae   .last_x  ;.24_1

;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[ecx+3]
	call  .mix_RGB
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	add  ecx,[size_x]
	mov  eax,[ecx]
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[ecx+3]
	call  .mix_RGB
	
	call  .RGB_to_EAX
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB
;----------------------------	
	call  .RGB_to_EAX1
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB
	call  .RGB_to_EAX
.24_1:
	ret
;---------------------------------------------------------------------
align 4
.last_y_24:
	mov  ebx,[temp_x1]
	push eax
	mov  eax,[x]
	dec  eax
	lea  eax,[eax*3]
	cmp  ebx,eax
	pop  eax
	jae   .last_x_y_24 ;.24_1
	
	mov  ebx,[esp+4]
;	test ebx,ebx
;	jz   .24_1
	inc  ebx
	cmp  bx,[new_size.x1]
	jae   .last_x_y_24 ;.24_1
;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[ecx+3]
	call  .mix_RGB
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	mov  eax,[background_color]
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
	
	call  .RGB_to_EAX
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB
;----------------------------	
	call  .RGB_to_EAX1
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB
	call  .RGB_to_EAX
;---------------------------------------
	ret
;---------------------------------------------------------------------
align 4
.last_x_24:
	mov  ebx,[temp_y1]
	inc  ebx
	cmp  ebx,[y]
	jae   .last_x_y_24 ;.24_1
	
	mov  ebx,[esp+8]
;	test ebx,ebx
;	jz   .24_1
	inc  ebx
	cmp  bx,[new_size.y1]
	jae   .last_x_y_24 ;.24_1
;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	add  ecx,[size_x]
	mov  eax,[ecx]
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
	
	call  .RGB_to_EAX
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB
;----------------------------	
	call  .RGB_to_EAX1
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB
	call  .RGB_to_EAX
	ret
;---------------------------------------------------------------------
align 4
.last_x_y_24:
;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	mov  eax,[background_color]
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
	
	call  .RGB_to_EAX
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB
;----------------------------	
	call  .RGB_to_EAX1
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB
	call  .RGB_to_EAX
	ret
;---------------------------------------------------------------------
align 4
.copy_RGB_to_RGB1:
	mov  eax,[B_sample]
	mov [B_sample_1],eax
	mov  eax,[G_sample]
	mov [G_sample_1],eax
	mov  eax,[R_sample]
	mov [R_sample_1],eax
	ret
;---------------------------------------
align 4
.mix_RGB:
	xor ebx,ebx
	mov bl,al
	imul ebx,edx
	add [B_sample],ebx
	shr eax,8
	xor ebx,ebx
	mov bl,al
	imul ebx,edx
	add [G_sample],ebx
	shr eax,8
	xor ebx,ebx
	mov bl,al
	imul ebx,edx
	add [R_sample],ebx
	ret
;---------------------------------------
align 4
.mix_RGB_16:
	xor ebx,ebx
	mov bl,al
	and bl,0x1F
	imul ebx,edx
	add [B_sample],ebx
	shr eax,5
	xor ebx,ebx
	mov bl,al
	cmp [resolution],16
	jne @f
	and bl,0x3F
	shr eax,6
	jmp  .mix_RGB_16_G_sample
align 4
@@:
	and bl,0x1F
	shr eax,5
align 4
.mix_RGB_16_G_sample:
	imul ebx,edx
	add [G_sample],ebx
	xor ebx,ebx
	mov bl,al
	and bl,0x1F
	imul ebx,edx
	add [R_sample],ebx
	ret
;---------------------------------------
align 4
.clear_RGB:
	xor ebx,ebx
	mov [B_sample],ebx
	mov [G_sample],ebx
	mov [R_sample],ebx
	ret
;---------------------------------------
align 4
.RGB_to_EAX:
	mov eax,[R_sample]
	shr eax,7	
	and eax,0xff
	shl eax,8
	mov ebx,[G_sample]
	shr ebx,7
	mov al,bl
	shl eax,8
	mov ebx,[B_sample]
	shr ebx,7
	mov al,bl	
	ret
;---------------------------------------
align 4
.RGB_to_EAX_16:
	mov eax,[R_sample]
	shr eax,7	
	and eax,0x1F
	mov ebx,[G_sample]
	shr ebx,7
	cmp [resolution],16
	jne @f
	shl eax,6
	and ebx,0x3F
	jmp .RGB_to_EAX_16_G_sample
align 4
@@:
	shl eax,5
	and ebx,0x1F
align 4
.RGB_to_EAX_16_G_sample:
	add eax,ebx
	shl eax,5
	mov ebx,[B_sample]
	shr ebx,7
	and ebx,0x1F
	add eax,ebx	
	ret
;---------------------------------------
align 4
.RGB_to_EAX1:
	mov eax,[R_sample_1]
	shr eax,7	
	and eax,0xff
	shl eax,8
	mov ebx,[G_sample_1]
	shr ebx,7
	mov al,bl
	shl eax,8
	mov ebx,[B_sample_1]
	shr ebx,7
	mov al,bl	
	ret
;---------------------------------------
align 4
.RGB_to_EAX1_16:
	mov eax,[R_sample_1]
	shr eax,7	
	and eax,0x1F
	mov ebx,[G_sample_1]
	shr ebx,7
	cmp [resolution],16
	jne @f
	shl eax,6
	and ebx,0x3F
	jmp .RGB_to_EAX1_16_G_sample
align 4
@@:
	shl eax,5
	and ebx,0x1F
align 4
.RGB_to_EAX1_16_G_sample:
	add eax,ebx
	shl eax,5
	mov ebx,[B_sample_1]
	shr ebx,7
	and ebx,0x1F
	add eax,ebx	
	ret
;---------------------------------------------------------------------
align 4		
.check_filtering_32:
	cmp  [filtering],0
	je   .32_1
	
	mov  ebx,[temp_y1]
	inc  ebx
	cmp  ebx,[y]
	jae   .last_y_32  ;.24_1
	
	mov  ebx,[temp_x1]
	push eax
	mov  eax,[x]
	dec  eax
	shl  eax,2
	cmp  ebx,eax
	pop  eax
	jae   .last_x_32 ;.24_1

;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[ecx+4]
	call  .mix_RGB
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	add  ecx,[size_x]
	mov  eax,[ecx]
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[ecx+4]
	call  .mix_RGB
	
	call  .RGB_to_EAX
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB
;----------------------------	
	call  .RGB_to_EAX1
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB
	call  .RGB_to_EAX
.32_1:
	ret
;---------------------------------------------------------------------
align 4
.last_y_32:
	mov  ebx,[temp_x1]
	push eax
	mov  eax,[x]
	dec  eax
	shl  eax,2
	cmp  ebx,eax
	pop  eax
	jae   .last_x_y_32 ;.32_1
	
	mov  ebx,[esp+4]
;	test ebx,ebx
;	jz   .24_1
	inc  ebx
	cmp  bx,[new_size.x1]
	jae   .last_x_y_32 ;.32_1
;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[ecx+4]
	call  .mix_RGB
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	mov  eax,[background_color]
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
	
	call  .RGB_to_EAX
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB
;----------------------------	
	call  .RGB_to_EAX1
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB
	call  .RGB_to_EAX
	ret
;---------------------------------------------------------------------
align 4
.last_x_32:
	mov  ebx,[temp_y1]
	inc  ebx
	cmp  ebx,[y]
	jae   .last_x_y_32 ;.32_1
	
	mov  ebx,[esp+8]
;	test ebx,ebx
;	jz   .24_1
	inc  ebx
	cmp  bx,[new_size.y1]
	jae   .last_x_y_32 ;.32_1
;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	add  ecx,[size_x]
	mov  eax,[ecx]
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
	
	call  .RGB_to_EAX
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB
;----------------------------	
	call  .RGB_to_EAX1
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB
	call  .RGB_to_EAX
	ret
;---------------------------------------------------------------------
align 4
.last_x_y_32:
;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	mov  eax,[background_color]
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
	
	call  .RGB_to_EAX
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB
;----------------------------	
	call  .RGB_to_EAX1
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB
	call  .RGB_to_EAX
	ret
;---------------------------------------------------------------------
align 4		
.check_filtering_16:
	cmp  [filtering],0
	je   .16_1
	
	mov  ebx,[temp_y1]
	inc  ebx
	cmp  ebx,[y]
	jae   .last_y_16  ;.24_1
	
	mov  ebx,[temp_x1]
	push eax
	mov  eax,[x]
	dec  eax
	shl  eax,1
	cmp  ebx,eax
	pop  eax
	jae   .last_x_16 ;.24_1

;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB_16
	
	mov  edx,[next_pixel_x]
	xor  eax,eax
	mov  ax,[ecx+2]
	call  .mix_RGB_16
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	add  ecx,[size_x]
	xor  eax,eax
	mov  ax,[ecx]
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB_16
	
	mov  edx,[next_pixel_x]
	xor  eax,eax
	mov  ax,[ecx+2]
	call  .mix_RGB_16
	
	call  .RGB_to_EAX_16
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB_16
;----------------------------	
	call  .RGB_to_EAX1_16
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB_16
	call  .RGB_to_EAX_16
.16_1:
	ret
;---------------------------------------------------------------------
align 4
.last_y_16:
	mov  ebx,[temp_x1]
	push eax
	mov  eax,[x]
	dec  eax
	shl  eax,1
	cmp  ebx,eax
	pop  eax
	jae   .last_x_y_16 ;.16_1
	
	mov  ebx,[esp+4]
;	test ebx,ebx
;	jz   .24_1
	inc  ebx
	cmp  bx,[new_size.x1]
	jae   .last_x_y_16 ;.16_1
;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB_16
	
	mov  edx,[next_pixel_x]
	xor  eax,eax
	mov  ax,[ecx+2]
	call  .mix_RGB_16
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	mov  eax,[background_color]
	and  eax,0xffff
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB_16
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	and  eax,0xffff
	call  .mix_RGB_16
	
	call  .RGB_to_EAX_16
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB_16
;----------------------------	
	call  .RGB_to_EAX1_16
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB_16
	call  .RGB_to_EAX_16
	ret
;---------------------------------------------------------------------
align 4
.last_x_16:
	mov  ebx,[temp_y1]
	inc  ebx
	cmp  ebx,[y]
	jae   .last_x_y_16 ;.16_1
	
	mov  ebx,[esp+8]
;	test ebx,ebx
;	jz   .24_1
	inc  ebx
	cmp  bx,[new_size.y1]
	jae   .last_x_y_16 ;.16_1
;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB_16
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	and  eax,0xffff
	call  .mix_RGB_16
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	add  ecx,[size_x]
	xor  eax,eax
	mov  ax,[ecx]
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB_16
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	and  eax,0xffff
	call  .mix_RGB_16
	
	call  .RGB_to_EAX_16
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB_16
;----------------------------	
	call  .RGB_to_EAX1_16
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB_16
	call  .RGB_to_EAX_16
	ret
;---------------------------------------------------------------------
align 4
.last_x_y_16:
;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB_16
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	and  eax,0xffff
	call  .mix_RGB_16
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	mov  eax,[background_color]
	and  eax,0xffff
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB_16
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	and  eax,0xffff
	call  .mix_RGB_16
	
	call  .RGB_to_EAX_16
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB_16
;----------------------------	
	call  .RGB_to_EAX1_16
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB_16
	call  .RGB_to_EAX_16
	ret
;---------------------------------------------------------------------
align 4
.get_palette:
	shl  eax,2
	add  eax,[palette]
	mov  eax,[eax]
	ret
;---------------------------------------------------------------------
align 4
.check_filtering_8:
	xor  eax,eax
	mov  al,[ecx]
	call .get_palette
	
	mov  ebx,[temp_y1]
	inc  ebx
	cmp  ebx,[y]
	jae   .last_y_8
	
	mov  ebx,[temp_x1]
	push eax
	mov  eax,[x]
	dec  eax
	cmp  ebx,eax
	pop  eax
	jae   .last_x_8

;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	xor  eax,eax
	mov  al,[ecx+1]
	call .get_palette
	call  .mix_RGB
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	add  ecx,[size_x]
	xor  eax,eax
	mov  al,[ecx]
	call .get_palette
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	xor  eax,eax
	mov  al,[ecx+1]
	call .get_palette
	call  .mix_RGB
	
	call  .RGB_to_EAX
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB
;----------------------------	
	call  .RGB_to_EAX1
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB
	call  .RGB_to_EAX
.8_1:
	ret
;---------------------------------------------------------------------
align 4
.last_y_8:
	mov  ebx,[temp_x1]
	push eax
	mov  eax,[x]
	dec  eax
	cmp  ebx,eax
	pop  eax
	jae   .last_x_y_8 ;.8_1
	
	mov  ebx,[esp+4]
;	test ebx,ebx
;	jz   .24_1
	inc  ebx
	cmp  bx,[new_size.x1]
	jae   .last_x_y_8 ;.8_1
;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	xor  eax,eax
	mov  al,[ecx+1]
	call .get_palette
	call  .mix_RGB
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	mov  eax,[background_color]
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
	
	call  .RGB_to_EAX
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB
;----------------------------	
	call  .RGB_to_EAX1
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB
	call  .RGB_to_EAX
;---------------------------------------
	ret
;---------------------------------------------------------------------
align 4
.last_x_8:
	mov  ebx,[temp_y1]
	inc  ebx
	cmp  ebx,[y]
	jae   .last_x_y_8 ;.8_1
	
	mov  ebx,[esp+8]
;	test ebx,ebx
;	jz   .24_1
	inc  ebx
	cmp  bx,[new_size.y1]
	jae   .last_x_y_8 ;.8_1
;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	add  ecx,[size_x]
	xor  eax,eax
	mov  al,[ecx]
	call .get_palette
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]

	call  .mix_RGB
	
	call  .RGB_to_EAX
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB
;----------------------------	
	call  .RGB_to_EAX1
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB
	call  .RGB_to_EAX
	ret
;---------------------------------------------------------------------
align 4
.last_x_y_8:
;----------------------------	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]
	call  .mix_RGB
;----------------------------	
	call  .copy_RGB_to_RGB1
;----------------------------
	mov  eax,[background_color]
	
	call .clear_RGB
	mov edx,128 ;100
	sub edx,[next_pixel_x]
	call  .mix_RGB
	
	mov  edx,[next_pixel_x]
	mov  eax,[background_color]

	call  .mix_RGB
	
	call  .RGB_to_EAX
	
	call .clear_RGB
	mov  edx,[next_pixel_y]
	call  .mix_RGB
;----------------------------	
	call  .RGB_to_EAX1
;----------------------------
	mov edx,128  ;100
	sub edx,[next_pixel_y]
	call  .mix_RGB
	call  .RGB_to_EAX
	ret
;---------------------------------------------------------------------