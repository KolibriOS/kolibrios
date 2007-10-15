format ELF

section '.text' executable
include 'proc32.inc'

public memcpy
public memmove

proc memcpy stdcall, to:dword,from:dword,count:dword

	mov ecx,[count]
	test ecx,ecx
	jz no_copy_block
		
		mov esi,[from]
		mov edi,[to]
		rep movsb
	no_copy_block:

	ret
endp

proc memmove stdcall, to:dword,from:dword,count:dword

	mov ecx,[count]
	test ecx,ecx
	jz no_copy_block_
		
		mov esi,[from]
		mov edi,[to]
		rep movsb
	no_copy_block_:

	ret
endp