format ELF

include "proc32.inc"

section '.text' executable

public _ksys_debug_out
public debug_out_str

align 4
proc _ksys_debug_out stdcall, c:dword

  pushad

  xor ecx,ecx
  mov	cl,byte[c]
  mov	ebx,1
  mov	eax,63
  int	0x40

  popad

  ret

endp

align 4
proc debug_out_str stdcall, s:dword

	pushad
      
	mov eax,[s] ;eax=pointer to string
	next_simbol_print:

		xor ebx,ebx
		mov bl,[eax]
		test bl,bl
		jz exit_print_str

		cmp bl,10
		jne no_new_line
			mov ecx,13
			stdcall _ksys_debug_out, ecx
		no_new_line:

		stdcall _ksys_debug_out, ebx
		add eax,1

	jmp next_simbol_print

	exit_print_str:

	popad

	ret
endp
