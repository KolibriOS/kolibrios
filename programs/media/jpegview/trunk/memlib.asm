
movedata:
	push eax
	xor eax,eax
	sub eax,edi
	and eax,3
	xchg ecx,eax
	sub eax,ecx
	jle .l1
	rep movsb
	mov ecx,eax
	shr ecx,2
	rep movsd
	and eax,3
   .l1: add ecx,eax
	rep  movsb
	pop eax
	ret

mallocz:
	call malloc
	pushad
	add ecx,3
	xor eax,eax
	shr ecx,2
	rep stosd
	popad
	ret

     mresize1: popad
		xor edi,edi
		stc
     mresize2: ret
mresize: ; puntero en di ncr retorna nuevo puntero en di
       test edi,edi
       jz malloc
       cmp ecx,[edi-4]
       je mresize2
       call free
malloc:
	mov edi,ecx
	jecxz mresize2
	pushad
	mov esi,iniciomemoria+4
	lea ebx,[ecx+3]
	and ebx,-4 ;redondeo a 4
    .l1: mov edi,esi
	add esi,[esi]
	jc mresize1
	lodsd
	cmp eax,ebx
	jc .l1
	cmp esi,[iniciomemoria+8]
	jc .l2
	jne mresize1
	lea edx,[ebx+esi+4]
	cmp edx,[iniciomemoria+12]
	jnc mresize1
	mov [iniciomemoria+8],edx
    .l2: pop dword [esi-4]
	push esi
	sub eax,ebx
	je .l3
	sub eax,4
	mov [esi+ebx],eax
	jz .l3
	;fragmentar
	add ebx,4
	add [edi],ebx
	mov eax,[esi]
	sub eax,ebx
	mov [esi+ebx],eax
	popad
	ret
    .l3: lodsd
	add eax,4
	add [edi],eax
	popad
	ret

realloc: test edi,edi
	jz malloc
	jecxz free
       pushad
       pop esi
       mov eax,[edi-4]
       call malloc
       push edi
       cmp ecx,eax
       jc .l1
       mov ecx,eax
    .l1: push esi
	call movedata
       pop edi
       call free
       popad
   .l2: ret
free:           ;puntero en di
		;no se comprueban los punteros
		;retorna di=0 , ncr
       test edi,edi
       jz realloc.l2
       pushad
       pop edi
       mov ebp,[edi-4]
       dec ebp
       and ebp,-4  ;redondeo a 4,dx=dx-4
       xor edx,edx
       push edx
       mov edx,iniciomemoria+4
       mov esi,edx
       ;buscar puntero libre anterior
    .l1: mov ebx,esi
       lodsd
       add esi,eax
       cmp esi,edi
       jc .l1
	;enlazar
       mov ecx,esi
       sub ecx,edi
       sub eax,ecx
       sub ecx,4
       mov [ebx],eax
	;fusionar con el anterior
       cmp eax,[ebx-4]
       jne .l2
       cmp ebx,edx
       je .l2 ;no fusionar con el primero
       mov edi,ebx
       add eax,4
       add ecx,eax
       add ebp,eax
    .l2: mov ebx,ebp ;fusionar con bloques de tama¤o 0
    .l3: add ebx,4
       test dword [edi+ebx],-1
       jz .l3
       cmp ebx,ecx
       jne .l4
	;fusionar con el siguiente
	add ebx,[esi-4]
	add ecx,[esi]
	add ebx,4
	add ecx,4
	cmp esi,[edx+4]
	jne .l4
	mov [edx+4],edi
     .l4: mov [edi-4],ebx
	mov [edi],ecx
	popad
	ret

add_mem: ;edi,ecx ;el ultimo bloque libre debe ser >8 bytes para poder fragmentarlo
	cmp ecx,64
	jc .l1
	add ecx,edi
	add edi,3
	and edi,-4
	and ecx,-4
	mov eax,ecx
	sub ecx,edi   ;redondeo
	xchg eax,[iniciomemoria+12]
	cmp edi,eax
	jna .l1
	lea esi,[edi+4]
	mov edx,esi
	xchg esi,[iniciomemoria+8]
	neg edx
	mov [edi],edx
	mov [edi+4],edx
	lea edx,[edi-4]
	sub edi,esi
	mov [esi],edi
	sub eax,4
	sub eax,esi
	mov [esi-4],eax
	add esi,eax
	sub edx,esi
	mov [esi],edx
   .l1: ret

check_mem:  ;busqueda de errores en la memoria
	   ;retorna edx nbloques o 0 si error,ecx memoria libre
	   ;ncr: ebp,ebx,eax
	mov edi,iniciomemoria
	mov esi,edi
	xor edx,edx
	mov ecx,[edi]
	neg ecx ;el primer bloque no cuenta
    .l1: add ecx,[edi]
	add edi,4
	add edi,[edi]
   .l2: inc edx
	add esi,[esi]
	jc .l4
	add esi,7
	jc .l3
	and esi,-4
	cmp esi,edi
	jc .l2
	je .l1
	jmp .l4
    .l3: test edi,edi
       jnz .l4
       add ecx,[iniciomemoria+12]
       ret
     .l4: xor edx,edx
      stc
      ret


