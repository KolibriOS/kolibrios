;ebx - pointer to string
GetLengthString:

      xor ecx,ecx

      next_simvol:
	inc ecx
	xor eax,eax
	mov al,[ebx]
	test al,al
	jz null
	inc ebx
       jmp next_simvol

      null:
      mov eax,ecx
      dec eax

      ret