;-----------------------------------------------------------
;-------find simvole in string------------------------------
;-----------------------------------------------------------
find_symvol:
	;eax=string
	;ebx=symvol
	mov esi,eax
	next_symvol:
	xor ecx,ecx
	mov cl,[eax]
	cmp cl,bl
	je symvol_fined
	inc eax
	jmp next_symvol

	symvol_fined:
	sub eax,esi

	ret
