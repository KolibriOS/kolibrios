;
; Functions for moving objects
;
; Author:   Trans
; Date:     10.03.2005
; Compiler: FASM
; Target:   Hunter - MeOS game
;

;
; In:
; Out:
init_object:
	pusha
	mov ecx,16
	mov edi,movieng_objects
	mov esi,present_objects_list
io_loc_00:
	call init_one_object
	add edi,16
	loop io_loc_00
	call set_current_objects_list
	popa
	retn


;
; In:  EDI - pointer to object structure
;      ESI - pointer to present_objects_list
; Out:
init_one_object:
	call set_object
	call set_obj_direction
	call set_obj_coord
	call set_obj_speed
	call set_obj_amplitude
	call set_obj_cost
	retn


;
;
;
set_object:
	call random_8
	and al,07
	shl eax,2
	mov ebx,[esi+eax]
	mov [edi],ebx
	retn

;
;
;
set_obj_coord:
	call random_8
	and ax,07Fh
	cmp al,0
	jz set_obj_coord
	add ax,10
	cmp ax,120
	jb soc_loc_01
	sub ax,20
soc_loc_01:
	shl eax,16
	xor ax,ax
	mov al,byte [edi+0Ah]
	cmp al,byte 0
	jz soc_loc_00
	mov ax,319
soc_loc_00:
	ror eax,16
	mov [edi+4],eax
	retn

;
;
;
set_obj_speed:
	call random_8
	and al,07h	;0Fh
	cmp al,0
	jz set_obj_speed
	mov [edi+8],al	
	retn

;
;
;
set_obj_amplitude:
	call random_8
	and al,0Fh
	mov [edi+9],al
	retn

;
;
;
set_obj_direction:
	call random_8
	and al,1
	mov [edi+0Ah],al
	retn

;
;
;
set_obj_cost:
	call random_8
	cmp al,0
	jz set_obj_cost
	mov [edi+0Bh],al
	retn

;
;
;
set_current_objects_list:
	pusha
	xor eax,eax
	mov edi,forward_list
	stosd
	stosd
	mov edi,back_list
	stosd
	stosd
	mov edi,forward_list
	call random_8
	and al,7
	cmp al,0
	jz scol_loc_00a
	mov ecx,eax
scol_loc_00:
	call random_8
	and al,7
	stosb
	loop scol_loc_00
scol_loc_00a:
	mov edi,back_list
	call random_8
	and al,07
	cmp al,0
	jz scol_loc_01a
	mov ecx,eax
scol_loc_01:
	call random_8
	and al,7
	stosb
	loop scol_loc_01
scol_loc_01a:
	popa
	retn

; 
; In:  EDI - pointer to object structure
; Out:
movie_one_object:
	pusha
	mov eax,[edi+4]
	xor ebx,ebx
	mov bl,byte [edi+8]
	ror eax,16
	cmp byte [edi+0Ah],0
	jz moo_loc_00
	sub ax,bx
	cmp ax,320
	jb moo_loc_01
	mov ax,319
	jmp moo_loc_01
moo_loc_00:
	add ax,bx
	cmp ax,320
	jb moo_loc_01
	xor ax,ax
moo_loc_01:
	rol eax,16
	mov [edi+4],eax
	popa
	retn

;
;
;
movie_objects:
	pusha
	mov esi,forward_list
	mov ebx,movieng_objects
	mov ecx,8
mo_loc_00:
	xor eax,eax
	lodsb
	cmp al,0
	jz mo_loc_01
	mov edi,ebx
	shl eax,4
	add edi,eax
	call movie_one_object
mo_loc_01:
	loop mo_loc_00
	add ebx,16*8
	mov ecx,8
mo_loc_02:
	xor eax,eax
	lodsb
	cmp al,0
	jz mo_loc_03
	mov edi,ebx
	shl eax,4
	add edi,eax
	call movie_one_object
mo_loc_03:
	loop mo_loc_02
	popa
	retn


;
; In:  EDI - pointer to object structure
; Out:
draw_one_forward_object:
	push esi
	mov esi,[edi]
	mov eax,[edi+4]
	call draw_pict
	pop esi
	retn

;
; In:  EDI - pointer to object structure
; Out:
draw_one_back_object:
	push esi
	mov esi,[edi]
	mov eax,[edi+4]
	call draw_pict_scale_div_2
	pop esi
	retn

;
;
;
draw_objects:
	pusha
	mov esi,back_list+7
	mov ebx,movieng_objects+16*8
	mov edi,ebx
	mov ecx,8
do_loc_00:
	xor eax,eax
;	lodsb
	mov al,[esi]
	cmp al,0
	jz do_loc_01
	mov edi,ebx
	shl eax,4
	add edi,eax
	call draw_one_back_object
do_loc_01:
	dec esi
	loop do_loc_00	
;	mov esi,forward_list
	mov ebx,movieng_objects
	mov ecx,8
do_loc_02:
	xor eax,eax
;	lodsb
	mov al,[esi]
	cmp al,0
	jz do_loc_03
	mov edi,ebx
	shl eax,4
	add edi,eax
	call draw_one_forward_object
do_loc_03:
	dec esi
	loop do_loc_02	
	popa
	retn

;
; In:  EDI - pointer to object structure
; Out: Carry flag Clear/Set - No/Kill object
check_kill_one_forward_object:
	pusha
	mov esi,[edi]
	lodsw
	shl eax,16
	lodsw
	mov ebx,[edi+04]
	mov edx,[mouse_coord]
	add eax,ebx
	cmp dx,bx
	jb ckofo_loc_00
	cmp dx,ax
	ja ckofo_loc_00
	shr eax,16
	shr ebx,16
	shr edx,16
	cmp dx,bx
	jb ckofo_loc_00
	cmp dx,ax
	ja ckofo_loc_00
	stc
	popa
	retn
ckofo_loc_00:
	clc
	popa
	retn

;
; In:  EDI - pointer to object structure
; Out: Carry flag Clear/Set - No/Kill object
check_kill_one_back_object:
	pusha
	mov esi,[edi]
	lodsw
	shr eax,1
	shl eax,16
	lodsw
	shr ax,1
	mov ebx,[edi+04]
	mov edx,[mouse_coord]
	add eax,ebx
	cmp dx,bx
	jb ckobo_loc_00
	cmp dx,ax
	ja ckobo_loc_00
	shr eax,16
	shr ebx,16
	shr edx,16
	cmp dx,bx
	jb ckobo_loc_00
	cmp dx,ax
	ja ckobo_loc_00
	stc
	popa
	retn
ckobo_loc_00:	
	clc
	popa
	retn

;
;
;
check_kill_object:
	pusha
	mov esi,forward_list
	mov ebx,movieng_objects
	mov ecx,8
cko_loc_00:
	xor eax,eax
	lodsb
	cmp al,0
	jz cko_loc_01
	shl eax,4
	mov edi,ebx
	add edi,eax
	call check_kill_one_forward_object
	jc cko_loc_kill
cko_loc_01:
	loop cko_loc_00
	add ebx,16*8
	mov ecx,8
cko_loc_02:
	xor eax,eax
	lodsb
	cmp al,0
	jz cko_loc_03
	shl eax,4
	mov edi,ebx
	add edi,eax
	call check_kill_one_back_object
	jc cko_loc_kill
cko_loc_03:
	loop cko_loc_02
	mov eax,[mouse_coord]
	xor ebx,ebx
	mov bl,byte [promakh_count]
	shl ebx,2
	add ebx,current_hole
	mov dword [ebx],eax
	inc byte [promakh_count]
	popa
	retn
cko_loc_kill:
	dec esi
	mov byte [esi],0
	xor eax,eax
	mov al,[edi+0Bh]
	add dword [score_count],eax
	clc
	popa
	retn


;
;
;
change_objects_speed:
	pusha
	mov esi,movieng_objects+8
	mov ecx,16
cos_loc_00:
	cmp byte [esi],0Fh
	jz cos_next
	inc byte [esi]
cos_next:
	add esi,16
	loop cos_loc_00
	popa
	retn
