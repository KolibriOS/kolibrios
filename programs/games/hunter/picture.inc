;
; Работа с картинками или их образами
;
; Author:   Trans
; Date:     08.03.2005
; Compiler: FASM
; Target:   Hunter - MeOS game
;

;
; In:  DS:ESI - указатель на образ картинки в RGB
;      EAX - X координата * 65536 + Y координата
;      //EBX - Z координата
; Out:
draw_pict:
	pusha
	mov edx,eax
	shr edx,16
	xor edi,edi
	mov di,ax
	shl di,3	; *8
	shl ax,1	; *2
	add ax,di	; = *10
	shl ax,5	; = *320
	mov di,ax
	shr eax,16
	add di,ax	; di=Y*320+X
	mov eax,edi
	shl eax,1	; = *2
	add edi,eax	; = (Y*320+X)*3
	add edi,buffer00
	lodsw
	shl eax,16
	lodsw
	ror eax,16
	xor ecx,ecx
	mov cx,320
	sub cx,dx
	xor edx,edx
	cmp cx,ax
	jae dp_loc_00
	sub ax,cx
	mov dx,ax
	shl ax,1
	add dx,ax	; invisible part for X
	mov ax,cx
dp_loc_00:
	mov cx,ax
	xor ebx,ebx
	mov bx,320
	sub bx,ax
	mov ax,bx
	shl ax,1
	add bx,ax	; additional offset for X
	shr eax,16
	xchg ecx,eax
dp_loc_01:
	push ecx
	push eax
	mov ecx,eax
dp_loc_02:
	mov eax,[esi]
	and eax,00FFFFFFh
	cmp eax,00FFFFFFh
	jnz dp_loc_03_next
	add esi,3
	add edi,3
	jmp dp_loc_04
dp_loc_03_next:
	movsb
	movsw
dp_loc_04:
	loop dp_loc_02
	add esi,edx
	add edi,ebx
	pop eax
	pop ecx
	loop dp_loc_01	
	popa
	retn

;
; In:  DS:ESI - указатель на образ картинки в RGB
;      EAX - X координата * 65536 + Y координата
;      //EBX - Z координата
; Out:
draw_pict_scale_div_2:
	pusha
	mov edi,buffer01
	lodsd
	xor ebx,ebx
	xor ebp,ebp
	mov bx,ax
	mov bp,ax
	shl ebp,1
	add ebp,ebx
	shr ax,1
	stosw
	mov bx,ax
	shr eax,17
	stosw
	xor ecx,ecx
	mov cx,ax
dpsd2_loc_00:
	push ecx
	mov ecx,ebx
dpsd2_loc_01:
	push ecx
	mov ecx,3
dpsd2_loc_02:
	xor eax,eax
	mov edx,eax
	mov al,[esi]
	mov dl,[esi+3]
	add eax,edx
	mov dl,[esi+ebp]
	add eax,edx
	mov dl,[esi+ebp+3]
	add eax,edx
	shr eax,2
	stosb
	inc esi
	loop dpsd2_loc_02
	add esi,3
	pop ecx
	loop dpsd2_loc_01
	add esi,ebp
	pop ecx
	loop dpsd2_loc_00
	popa
	pusha
	mov esi,buffer01
	call draw_pict
	popa
	retn


;
; In:
; Out:
draw_backside:
	pusha
	mov edi,buffer00
	mov ecx,50
	mov eax,00FFFFFFh
dbs_loc_00:
	push ecx
	mov ecx,320
dbs_loc_01:
	mov ebx,eax
	stosw
	shr eax,16
	stosb
	mov eax,ebx
	stosw
	shr eax,16
	stosb
	mov eax,ebx
	loop dbs_loc_01
	sub eax,00010100h
	pop ecx
	loop dbs_loc_00
	mov ecx,100
	mov eax,0000C000h
dbs_loc_02:
	push ecx
	mov ecx,320
dbs_loc_03:
	mov ebx,eax
	stosw
	shr eax,16
	stosb
	mov eax,ebx
	loop dbs_loc_03
	add eax,00010001h
	pop ecx
	loop dbs_loc_02
	popa
	retn

;
; In:
; Out:
buffer_scale_on_2:
	pusha
	mov esi,buffer00
	mov edi,buffer02
	mov ecx,200
bso2_loc_00:
	push ecx
	mov ecx,2
bso2_loc_01:
	push ecx
	push esi
	mov ecx,320
bso2_loc_02:
	mov eax,[esi]
	and eax,00FFFFFFh
	mov ebx,eax
	stosw
	shr eax,16
	stosb
	mov eax,ebx
	stosw
	shr eax,16
	stosb
	add esi,3
	loop bso2_loc_02
	pop esi
	pop ecx
	loop bso2_loc_01
	add esi,320*3
	pop ecx
	loop bso2_loc_00
	popa
	retn


;
; In:  EAX = X coordinate * 65536 + Y coordinate
;      EBX = Number
; Out:
number_print:
	pusha
	mov edi,buf_local
	mov ebp,eax
	mov eax,ebx
	or eax,eax
	jz np_loc_null
	mov ebx,100
	mov ecx,5
np_loc_00:
	xor edx,edx
	div ebx
	xchg eax,edx
	aam
	stosw
	mov eax,edx
	loop np_loc_00
	dec edi
	mov eax,ebp
	mov ecx,10
np_loc_01:
	cmp byte [edi],0
	jnz np_loc_02
	dec edi
	loop np_loc_01	
np_loc_02:
	xor ebx,ebx
	mov bl,[edi]
	shl ebx,2
	add ebx,pointer_digit_table
	mov esi,[ebx]
	call draw_pict
	add eax,10*65536+0
	dec edi
	loop np_loc_02
	popa
	retn
np_loc_null:
	mov eax,ebp
	mov esi,[pointer_digit_table]
	call draw_pict
	popa
	retn
pointer_digit_table:
	dd buf_num0, buf_num1, buf_num2, buf_num3, buf_num4
	dd buf_num5, buf_num6, buf_num7, buf_num8, buf_num9
buf_local:
	times 12 db 0


;
; In:  EAX = X*65536+Y
; Out:
draw_lives:
	pusha
	mov esi,buf_heart
	xor ecx,ecx
	mov cl,[lives_count]
	cmp cl,0
	jz dl_loc_01
dl_loc_00:
	call draw_pict
	add eax,16*65536+0
	loop dl_loc_00
dl_loc_01:
	popa
	retn

;
; In:
; Out:
draw_mushka:
	mov esi,buf_mushka
	mov eax,[mouse_coord]
	ror eax,16
	cmp ax,16
	jae dm_loc_00
	mov ax,16
dm_loc_00:
	sub ax,16
	rol eax,16
	cmp ax,16
	jae dm_loc_01
	mov ax,16
dm_loc_01:
	sub ax,16
	call draw_pict
	retn

;
; In:   eax - hole coordinate
; Out:
draw_hole:
	mov esi,buf_hole
;	mov eax,[current_hole]
	cmp eax,0
	jz dh_loc_02
	ror eax,16
	cmp ax,8
	jae dh_loc_00
	mov ax,8
dh_loc_00:
	sub ax,8
	rol eax,16
	cmp ax,8
	jae dh_loc_01
	mov ax,8
dh_loc_01:
	sub ax,8
	call draw_pict_scale_div_2
dh_loc_02:
	retn

;
;
;
draw_promakh:
	mov esi,buf_hole
	xor ecx,ecx
	mov cl,[promakh_count]
	cmp cl,0
	jz dpr_loc_exit
	mov ebx,current_hole
	mov eax,dword [ebx]  ;300*65536+180
dpr_loc_00:
	call draw_hole
;	call draw_pict_scale_div_2
;	sub ax,16
	add ebx,4
	mov eax,dword [ebx]
	loop dpr_loc_00
dpr_loc_exit:
	retn


;
; In:
; Out:
clear_buffer:
	push eax
	push ecx
	push edi
	mov edi,buffer00
	mov ecx,(320*201*3)/4
	xor eax,eax
	rep stosd
	pop edi
	pop ecx
	pop eax
	retn

;
; In:  AX - X координата [0,639]
;      BX - Y координата [0,479]
; Out:
refresh_screen:
	pusha
	popa
	retn


;
; In:
; Out:
smooth_filter:
	pusha
	mov ecx,320*200*3
	mov edi,buffer00
sf_loc_00:
	xor ax,ax
	xor dx,dx	;счетчик
	mov al,[edi]
	shl ax,2	;*4
	mov dx,ax
	xor ax,ax
	mov al,[edi-320*3]
	add dx,ax
	mov al,[edi-3]
	add dx,ax
	mov al,[edi+3]
	add dx,ax
	mov al,[edi+320*3]
	add dx,ax
	xchg dx,ax
	shr ax,3	;/8
	mov byte [edi],al
	inc edi
	loop sf_loc_00
	popa
	retn
;smooth_matrix:
;	db  0, 1, 0      /
;	db  1, 4, 1    /   8
;	db  0, 1, 0  /


