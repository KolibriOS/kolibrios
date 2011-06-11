;---------------------------------------------------------------------
convert_plugin_invoke:
	mov	eax,[plugin_0_name]
	call	prepare_convert
	push	eax
	call	[Convert_plugin_0.Start]
	call	check_convert
;-------------------------------    
	mov	eax,[plugin_1_name]
	call	prepare_convert
	push	eax
	call	[Convert_plugin_1.Start]
	call	check_convert
;-------------------------------
	mov	eax,[plugin_2_name]
	call	prepare_convert
	push	eax
	call	[Convert_plugin_2.Start]
	call	check_convert
;-------------------------------
	mov	eax,[plugin_3_name]
	call	prepare_convert
	push	eax
	call	[Convert_plugin_3.Start]
	call	check_convert
;-------------------------------
	mov	eax,[plugin_4_name]
	call	prepare_convert
	push	eax
	call	[Convert_plugin_4.Start]
	call	check_convert
;-------------------------------
	mov	eax,[plugin_5_name]
	call	prepare_convert
	push	eax
	call	[Convert_plugin_5.Start]
	call	check_convert
;-------------------------------
	mov	eax,[plugin_6_name]
	call	prepare_convert
	push	eax
	call	[Convert_plugin_6.Start]
	call	check_convert
;-------------------------------
	mov	eax,[plugin_7_name]
	call	prepare_convert
	push	eax
	call	[Convert_plugin_7.Start]
	call	check_convert
;-------------------------------
	mov	eax,[plugin_8_name]
	call	prepare_convert
	push	eax
	call	[Convert_plugin_8.Start]
	mov	eax,[return_code]
	call	check_convert
;-------------------------------
	mov	eax,[plugin_9_name]
	call	prepare_convert
	push	eax
	call	[Convert_plugin_9.Start]
;-------------------------------
.end:
	mov	ecx,[image_file]
	mcall	68, 13,
	ret
;---------------------------------------------------------------------
prepare_convert:
	cmp	eax,dword 'NoKe'
	je	.1  ;@f
	xor	eax,eax
	mov	[return_code],eax
	mov	eax,image_file
	ret
.1:
	add	esp,4
	jmp	convert_plugin_invoke.end
;---------------------------------------------------------------------
check_convert:
	mov   eax,[return_code]
	test  eax,eax
	jz   prepare_convert.1
    cmp   eax,dword 2
    je   prepare_convert.1
	ret
;---------------------------------------------------------------------	
convert:
	xor  eax,eax
	cmp  [error_fs],eax
	jnz   .error

	call  convert_plugin_invoke

	cmp   [return_code],dword 0
	je   .all_ok
	xor  eax,eax
;-------------------------------
.error_1:
	mov  [img_size],eax
	mov  [img_width],eax
	mov  [img_width_2],eax
	mov  [img_high],eax
	mov  [img_high_2],eax
	mov  [img_resolution],eax
	mov  [img_resolution_2],eax
	mov  [img_palette],eax 
	mov  [soi],eax
	mov  [file_name],eax
	mov  [Total_Repeat_N],eax
	mov  [RAW1_pointer],eax
	mov  [RAW1_flag],al
	
	mov  [scaling_mode],dword 100
	call error_window
	ret
;-------------------------------
.error:
	xor  eax,eax
	mov  [return_code],eax
	jmp .error_1
;-------------------------------
.all_ok:
    mov  ebx,[raw_pointer]
	cmp  [ebx],dword 'RAW1'
	je   RAW1_processing
	xor  eax,eax
	mov  [RAW1_flag],al
	mov  [RAW1_pointer],eax
	inc  eax
	mov  [Total_Repeat_N],eax
	mov  eax,[ebx+4]
	mov  [img_width],eax
	mov  [img_width_2],eax
	mov  eax,[ebx+8]
	mov  [img_high],eax
	mov  [img_high_2],eax
	mov  eax,[ebx+12]
	mov  [img_resolution_2],eax
;	mov  eax,[ebx+20]
;	add  eax,ebx
;	mov  [img_palette],eax    
;	mov  eax,[ebx+28]
;	add  eax,ebx
;	mov  [soi],eax
;-------------------------------
;	xor  eax,eax
;	mov  [raw_pointer_2],eax
	mov eax,image_file
;	call [plugin_convert]
	call [convert_Start]

	mov  ebx,[raw_pointer]
	mov  eax,[ebx+12]
;	cmp  [ebx+18],word 2
;	jne  @f
;	shr  eax,1
;@@:
	cmp  eax,15
	jne  @f
	inc  eax
@@:	
	mov  [img_resolution],eax
	
	mov  eax,[ebx+20]
	add  eax,ebx
	mov  [img_palette],eax 
	
	mov  eax,[ebx+28]
	add  eax,ebx
	mov  [soi],eax

;    mov  eax,[raw_pointer_2]
;    test eax,eax
;	jz  @f
;    mov  [soi],eax
;@@:

;-------------------------------
;    mov  eax,[img_resolution]
;    cmp  eax,32
;    ja   @f
;    cmp  [ebx+18],word 2
;    je   @f
;    cmp  [ebx+18],word 1
;    jne  .img_resolution_ok
;.grayscale_16:
;    cmp  eax,16
;    jne  .img_resolution_ok 
;    shr  eax,1
;    mov  [img_resolution],eax
;    jmp  .img_resolution_ok    
;@@:
;    shr  eax,1
;    cmp  eax,16
;    jne  @f
;    shr  eax,1
;@@:
;    mov  [img_resolution],eax
   
.img_resolution_ok:
	xor  eax,eax
	mov  [scroll_bar_data_vertical.position],eax
	mov  [scroll_bar_data_horizontal.position],eax
    call  get_window_param
    test  [window_status],1b
    jnz   .end
	cmp  [scaling_mode],dword 100
	jne   .end
    mov  ebx,[img_high]
    mov  eax,[img_width]
	test	ebx, ebx
	jns	@f
	neg	ebx
@@:
    add  eax,9
    cmp  eax,770
    jae  @f
    mov  eax,770
@@:
    add  ebx,54
    cmp  ebx,150
    jae  @f
    mov  ebx,150
@@:
    mov  [wnd_width],eax
    mov  [wnd_height],ebx
	
	mcall 14
	
	mov  ebx,eax
	and  eax,0xffff
	shr  ebx,16
	inc  eax
	inc  ebx
	cmp  eax,[wnd_height]
	jae  @f
	xor  eax,eax
	mov  ax,[scroll_bar_data_vertical.size_x]
	add  [wnd_width],eax
@@:	
	cmp  ebx,[wnd_width]
	jae  @f
	xor  eax,eax
	mov  ax,[scroll_bar_data_horizontal.size_y]
	add  [wnd_height],eax
@@:	
    mov  eax,[wnd_width]
    mov  ebx,[wnd_height]
	
    test [bWasDraw],1
    jz	 @f
    mov  esi,ebx
    mov  edx,eax
    mcall 67,-1,-1
@@:
.end:
    ret
;---------------------------------------------------------------------