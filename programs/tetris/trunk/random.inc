;--------------------------------------------------------------

random: 	mov eax,[generator]
		add eax,-43ab45b5h
		ror eax,1
		xor eax,32c4324fh
		ror eax,1
		mov [generator],eax
		; --- IVAN ---
		push ebx
		mov eax,22
		mov ebx,9
		int 0x40
		pop ebx
		xor eax,0xdeadbeef
		add eax,[generator]
		; --- IVAN ---
		ret