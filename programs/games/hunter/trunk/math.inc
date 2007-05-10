;
; Mathematical functions
;
; Author:   Trans
; Date:     08.03.2005
; Compiler: FASM
; Target:   Hunter - MeOS game
;

rinit	dw 0

; Randomize of 8 bits
; In:
; Out: AL - random value
random_8:
	push ebx
	push ecx
	xor eax,eax
	mov ax,[rinit]
	cmp ax,0
	jnz rand_loc_00
	mov eax,26
	mov ebx,9
	int 40h
	mov [rinit],ax
	shl eax,16
	shr eax,16
rand_loc_00:
	mov ecx,8
rand_loc_01:
	mov bx,ax
	and bx,002Dh
	xor bh,bl
	clc
	jpe rand_loc_01a
	stc
rand_loc_01a:
	rcr ax,1
	loop rand_loc_01
	mov [rinit],ax
	xor ah,ah
	pop ecx
	pop ebx
	retn
