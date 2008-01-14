format ELF

section '.text' executable

public start_

extrn main

buf_len = 0x400
max_parameters=0x20

start_:
	db 'MENUET01'	; 1. Magic number (8 bytes)
	dd 0x01		; 2. Version of executable file
	dd start__	; 3. Start address
	dd 0x0		; 4. Size of image
	dd 0x100000	; 5. Size of needed memory
	dd 0x100000	; 6. Pointer to stack
hparams	dd params	; 7. Pointer to program arguments
hpath	dd path		; 8. Pointer to program path

	start__:

	;init heap of memory
	mov eax,68
	mov ebx,11
	int 0x40

	mov ebx,path
	mov ecx,dword buf_len
	add ebx,ecx

	next_simbol_check:
	xor eax,eax
	mov al,[ebx]
	cmp al,'/'
	je simbol_fined
	dec ebx
	dec ecx
	jnz next_simbol_check

	simbol_fined:
	inc ebx

	mov [argc],dword 1
	mov edx,argv
	mov [edx],ebx	;argument number 0 - program name 

	cmp [params],byte 0
	je exit_find_params
	
	mov [argc],dword 2
	mov ebx,params
	add edx,4
	mov [edx],ebx

	next_symbol_parse:
		xor eax,eax
		mov al,[ebx]
		test al,al
		jz exit_find_params
		cmp al,' '
		je save_param
	
		inc ebx
	jmp next_symbol_parse
	save_param:

	mov [ebx],byte 0
	inc ebx
	add edx,4
	mov [edx],ebx
	inc [argc]
	
	cmp [argc],max_parameters
	jae exit_find_params

	jmp next_symbol_parse

	exit_find_params:

	push argv
	push [argc]

	call main
exit:

	xor  eax,eax
	dec  eax
	int  0x40
	dd   -1
crash:
	jmp  exit

public params as '__argv'
public path as '__path'

section '.bss'
argc     rd 1
argv     rd max_parameters
path     rb buf_len
params   rb buf_len
