file_handler:
 .operation=0
 .position=4
 .reserved=8
 .n_bytes=12
 .bufer=16
 .name=20
 .st_size=20+1024

open: ;esi=name_string
      ;retorna eax
	pushad
	mov ecx,file_handler.st_size
	call mallocz
	mov [esp+28],edi
	push edi
	mov ecx,1024
	add edi,file_handler.name
	call movedata
	pop edi
; test if file exists
	lea ebx,[edi+file_handler.operation]
	mov byte[ebx],5
	mov dword[ebx+16],fileattr
	mov eax,70
	int 0x40
	cmp eax,2
	jz .virtual
	test eax,eax
	jnz close.b
@@:
	clc
	popad
	ret
.virtual:
        mov     byte [fileattr], 0x10
        jmp     @b

close:
	pushad
     .b:
	mov edi,[esp+28]
	call free
	popad
	xor eax,eax
	ret


read:     ;(f,bufer,nbytes)  eax,edi,ecx ncr
	  ;retorna bytes leidos en ecx
	pushad
	lea     ebx, [eax+file_handler.operation]
	mov     byte [ebx], 0
	mov     [ebx+12], ecx
	mov     [ebx+16], edi
	mov     eax, 70
	int     0x40
	cmp     ebx, -1
	sbb     ebx, -1
	mov     eax, [esp+28]
	add     [eax+file_handler.position], ebx
	mov     [esp+24], ebx
	popad
	ret

ftell:  mov edx,[eax+file_handler.position]
	ret
lseek: ;eax=file edx=pos
	mov [eax+file_handler.position],edx
	ret
skip:   ;eax=file edx=bytes to skip
	add [eax+file_handler.position],edx
	ret




