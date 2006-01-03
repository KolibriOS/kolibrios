file_handler:
 .position=0
 .size=4
 .bufer_block=8
 .operation=12
 .first_block=16
 .n_blocks=20
 .bufer=24
 .work_area=28
 .name=32
 .st_size=32+128

open: ;esi=name_string
      ;retorna eax
	pushad
	push dword [work_area]
	mov ecx,512 ;bufer
	call malloc
	push edi
	mov ecx,file_handler.st_size
	call mallocz
	pop dword[edi+file_handler.bufer]
	pop dword[edi+file_handler.work_area]
	mov [esp+28],edi
	mov ecx,100
	add edi,file_handler.name
	call movedata
	mov edi,[esp+28]
	mov byte[edi+file_handler.n_blocks],1
	mov  eax,58
	lea  ebx,[edi+file_handler.operation]
	push edi
	int  0x40
	pop edi
	test eax,eax
	jnz close.b
	mov [edi+file_handler.size],ebx
	clc
	popad
	ret

close:
	pushad
     .b:
	mov eax,[esp+28]
	mov edi,[eax+file_handler.bufer]
	call free
	mov edi,eax
	call free
	popad
	xor eax,eax
	ret


read:     ;(f,bufer,nbytes)  eax,edi,ecx ncr
	  ;retorna bytes leidos en ecx
	pushad
	mov edx,[eax+file_handler.size]
	sub edx,[eax+file_handler.position]
	cmp edx,ecx
	jnc .nb_ok
	mov ecx,edx
	mov [esp+24],edx
      .nb_ok:
	jecxz .final
	mov byte[eax+file_handler.operation],0
	test dword[eax+file_handler.position],511
	jz .l1
	call .bufer
      .l1:
	mov edx,ecx
	shr edx,9
	jz .l2
	mov [eax+file_handler.n_blocks],edx
	mov edx,[eax+file_handler.position]
	shr edx,9
	mov [eax+file_handler.first_block],edx
	xchg edi,[eax+file_handler.bufer]
	pushad
	lea ebx,[eax+file_handler.operation]
	mov eax,58
	int 40h
 cmp eax,0ffffff00h
 jnc .error
	popad
	xchg edi,[eax+file_handler.bufer]
	mov edx,ecx
	and edx,0fffffe00h
	add [eax+file_handler.position],edx
	add edi,edx
      .l2:
	and ecx,511
	jz .final
	call .bufer
      .final:
	popad
	ret

   .bufer:
	pushad
	mov ebx,[eax+file_handler.position]
	shr ebx,9
	cmp ebx,[eax+file_handler.bufer_block]
	je .l3
	mov [eax+file_handler.first_block],ebx
	mov [eax+file_handler.bufer_block],ebx
	mov dword[eax+file_handler.n_blocks],1
	lea ebx,[eax+file_handler.operation]
	mov eax,58
	int 40h
 cmp eax,0ffffff00h
 jnc .error
   .l3:
	mov eax,[esp+28]
	mov edx,[eax+file_handler.position]
	mov esi,[eax+file_handler.bufer]
	and edx,511
	add esi,edx
	neg edx
	add edx,512
	cmp ecx,edx
	jc .l4
	mov ecx,edx
    .l4:
	add [eax+file_handler.position],ecx
	sub [esp+24],ecx
	pop edi
	call movedata
	push edi
	popad
	ret
.error:
 popad
 popad
 xor ecx,ecx
 stc
 ret


ftell:  mov edx,[eax+file_handler.position]
	ret
lseek: ;eax=file edx=pos
	mov [eax+file_handler.position],edx
	ret
skip:   ;eax=file edx=bytes to skip
	add [eax+file_handler.position],edx
	ret




