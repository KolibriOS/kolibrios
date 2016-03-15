;-----------------------------------------------------------
;-------find simvole in string------------------------------
;-----------------------------------------------------------

;input:
;eax=string
;ebx=symvol
find_symvol:
	mov esi,eax
	next_symvol:
	cmp byte[eax],bl
	je symvol_fined
	inc eax
	jmp next_symvol

	symvol_fined:
	sub eax,esi

	ret
