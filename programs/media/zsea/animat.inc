;---------------------------------------------------------------------
RAW1_processing:
;    mov  ebx,[raw_pointer]
;	mov  eax,[ebx+16]
;	mov  [raw_pointer],eax
;	jmp  convert.all_ok
	xor  eax,eax
	mov  [Repeat_N],eax
	inc  eax
	mov [RAW1_flag],al

    mov  ebx,[raw_pointer]
	mov  [RAW1_pointer],ebx
	mov  eax,[ebx+4]
	mov  [Total_Repeat_N],eax
	cmp  eax,1
	jne  @f
	xor  eax,eax
	mov [RAW1_flag],al
@@:
;	mov  [test1],eax
    mov  eax,[ebx+8]
    mov  [img_width],eax
    mov  [img_width_2],eax
;	mov  [test1],eax
	mov  ecx,[ebx+12]
    mov  [img_high],ecx
    mov  [img_high_2],ecx
;	mov  [test2],ecx
	imul ecx,eax
	
;	mov  eax,ecx
;	shl  ecx,1
;	add  ecx,eax
	shl  ecx,2
	
	add  ecx,256*4
	add  ecx,44
	push  ecx

	mcall 68,12

	mov  [raw_pointer],eax
	mov  ebx,[RAW1_pointer]
	mov  ecx,[ebx+8]
	mov  [eax+4],ecx ; x
	mov  ecx,[ebx+12]
	mov  [eax+8],ecx ; y
	mov  [eax+12],dword 32 ;8
	mov  [eax+20],dword 44
	mov  [eax+24],dword 1024
	mov  [eax+28],dword 44+1024
;	mov  [eax+32],

;    mov  [img_palette],eax
	add   eax,44+1024
    mov   [soi],eax
	pop  ecx

	cmp	[Total_Repeat_N],1
	je	@f

	sub  ecx,44+1024
	mov  [RAW1_size],ecx
	mov  edi,[soi]
	mov  eax,-1
	shr  ecx,2
	cld
	rep  stosd

	mov ecx,[RAW1_size]
	mcall 68,12
	mov  [previous_frame_pointer],eax
@@:	
    mov  [img_resolution_2],8
    mov  [img_resolution],32

	call copy_next_frame

;	inc  [Repeat_N]
;	call copy_next_frame
;	inc  [Repeat_N]
;	call copy_next_frame
;	mov  [Repeat_N],0

	jmp  convert.img_resolution_ok
;---------------------------------------------------------------------
copy_next_frame:
	mov  eax,[RAW1_pointer]
	mov  ebx,[eax+8]
	mov  [RAW1_size_x],ebx

	mov  eax,[Repeat_N]
	shl  eax,4
	; eax = (start of current Repeat_N) - 16
	add  eax,[RAW1_pointer]
	mov  esi,[eax+16]

	; esi = pointer to current RAW
;	mov  ebx,[RAW1_pointer]
	mov  edi,[RAW1_size_x]  ;[ebx+8]
	mov  ebx,[eax+24]
	shl  ebx,2
	imul edi,ebx
	mov  ebx,[eax+20]
	shl  ebx,2
	add  edi,ebx
;	add edi,[raw_pointer]
;	add edi,44
	add edi,[soi]
	; edi = pointer to start for current frame
	mov ebx,[eax+28]

	test bx,bx
	jnz  @f
;	inc  ebx
	mov  bx,10
@@:
	mov [Frame_delay],bx
	shr ebx,16
	mov [Transp_index],bh
	
	mov al,bl
	shr al,2
	and al,111b
	mov [Disposal_Method],al
;	cmp al,2
;	jne  @f
;	call save_previous_frame
;@@:
	cmp al,3
	jne  @f
	call save_previous_frame
@@:
	and bl,1b
	mov [Transp_flag],bl
	
	mov ebx,[esi+4] ; X size og current frame
	mov ecx,[esi+8] ; Y size og current frame

	mov eax,[esi+20]
	add eax,esi
	mov [img_palette],eax
	
	pusha
	mov ebx,[raw_pointer]
	mov edi,[ebx+20]
	add edi,ebx
	mov esi,eax
	mov ecx,256
	xor  eax,eax
	cld
	rep movsd
	popa
	
	mov eax,[esi+28]
	add esi,eax
.y:
	push edi
	push ebx
.x:
	cld
	lodsb
	cmp [Transp_flag],0
	je @f
	cmp [Transp_index],al
	jne @f
;	cmp [Disposal_Method],2
;	jne  .not_disposal_backgr
;	mov [edi],dword -1 ; 0xff
;	inc edi
;.not_disposal_backgr:
	add edi,4
	jmp .no_store
@@:
	push ebx edi
	mov ebx,[raw_pointer]
	mov edi,[ebx+20]
	add edi,ebx
	and eax,0xff
	shl eax,2
	add edi,eax
	mov eax,[edi]
	pop edi ebx
	stosd
.no_store:
	dec  ebx
	jnz  .x

	pop  ebx
	pop  edi
	mov  eax,[RAW1_size_x]
	shl  eax,2
	add  edi,eax
	dec  ecx
	jnz .y
	inc  [Repeat_N]
	mov  eax,[Repeat_N]
;	mov  [test2],eax
	cmp  [Total_Repeat_N],eax
	ja   @f
	xor  eax,eax
	mov  [Repeat_N],eax
@@:
    mcall 26,9
	movzx ebx,word [Frame_delay]
	add eax,ebx
	mov [Animation_timer],eax
	ret
;---------------------------------------------------------------------
animation_handler:
	mcall 23,1
;	pusha
;    mcall 26,9
;	cmp   [Animation_timer],eax
;	ja   @f
;	call   .draw_image
;@@:
;	popa
    cmp  [open_file_flag],1
	je   kopen_1
	cmp  [sort_directory_flag],byte 1
	je   red_sort_directory
	cmp  [redraw_flag],byte 1
	je   redraw_window
    cmp  eax,1	; перерисовать окно ?
    je     red	; если да - на метку red
    cmp  eax,2	; нажата клавиша ?
    je     key	; если да - на key
    cmp  eax,3	; нажата кнопка ?
    je     button     ; если да - на button
    cmp   eax,6
    je    mouse
    mcall 26,9
	cmp   [Animation_timer],eax
	ja   animation_handler  ;@f
.1:
	call    .draw_image
;	mov  eax,[scaling_mode]
;	jmp   zoom_plus.2
	jmp  still
;@@:
;	mcall 23,1
;	jmp  still.1
;---------------------------------------------------------------------
.draw_image:
;	mov eax,[Repeat_N]
;	test eax,eax
;	jz  .first_frame
	mov al,[Disposal_Method]
	cmp al,2
	jne  @f
	call restore_background
	jmp  .first_frame
@@:
	cmp al,3
	jne  @f
	call restore_previous_frame
@@:
.first_frame:
	call copy_next_frame
	mov  [no_fill],1
	call drawimage
	mov  [no_fill],0
	ret
;---------------------------------------------------------------------
restore_background:
;	cmp [Transp_flag],0
;	je @f
;	pusha
;	mov  edi,[soi]
;	mov  eax,[fill_color] ;  -1
;	mov  ecx,[RAW1_size]
;	shr ecx,2
;	cld
;	rep  stosd
;	popa
;	ret
;@@:
	pusha
	mov  eax,[Repeat_N]
	test eax,eax
	jz  @f
	dec  eax
@@:
	shl  eax,4
	; eax = (start of current Repeat_N) - 16
	add  eax,[RAW1_pointer]
	mov  esi,[eax+16]
	; esi = pointer to current RAW
	mov  edi,[RAW1_size_x]  ;[ebx+8]
	mov  ebx,[eax+24]
	shl  ebx,2
	imul edi,ebx
	mov  ebx,[eax+20]
	shl  ebx,2
	add  edi,ebx
	add edi,[soi]
	; edi = pointer to start for current frame
	mov ebx,[esi+4] ; X size og current frame
	mov ecx,[esi+8] ; Y size og current frame
.y:
	push edi
	push ebx
	mov  eax,[fill_color]
.x:
	cld
	stosd

	dec  ebx
	jnz  .x

	pop  ebx
	pop  edi
	mov  eax,[RAW1_size_x]
	shl  eax,2
	add  edi,eax
	dec  ecx
	jnz .y
	
	popa
	ret
;---------------------------------------------------------------------
restore_previous_frame:
	pusha
	mov esi,[previous_frame_pointer]
	mov edi,[soi]
	mov ecx,[RAW1_size]
	shr ecx,2
	cld
	rep movsd
	popa
	ret
;---------------------------------------------------------------------
save_previous_frame:
	pusha
	mov esi,[soi]
	mov edi,[previous_frame_pointer]
	mov ecx,[RAW1_size]
	shr ecx,2
	cld
	rep movsd
	popa
	ret
;---------------------------------------------------------------------