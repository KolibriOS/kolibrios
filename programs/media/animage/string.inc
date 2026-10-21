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

;output:
; eax = strlen
align 4
proc str_len, str1:dword
	mov eax,[str1]
	@@:
		cmp byte[eax],0
		je @f
		inc eax
		jmp @b
	@@:
	sub eax,[str1]
	ret
endp