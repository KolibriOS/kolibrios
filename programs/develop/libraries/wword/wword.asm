
format MS COFF

public EXPORTS

section '.flat' code readable writable align 16


version:
	mov	eax, 10
	ret

; int __stdcall convert (const char filename[], char buffer[], int mode)
convert:

	mov	eax, [esp+4]
	mov	dword [filename], eax
	mov	eax, [esp+8]
	mov	dword [buffer], eax
	mov	eax, [esp+12]
	mov	dword [mode], eax

	pushad

	mov	eax, 68	
	mov	ebx, 11
	int	0x40

	mov	dword [file_in],    5
	mov	dword [file_in+4],  0
	mov	dword [file_in+8],  0
	mov	dword [file_in+12], 0
	mov	dword [file_in+16], bdvk
	mov	byte  [file_in+20], 0
	mov	eax, [filename]
	mov	dword [file_in+21], eax

	mov	eax, 70
	mov	ebx, file_in
	int	0x40

	mov	dword eax, [bdvk+32]
	mov	dword [buf_len], eax

	mov	eax, 68
	mov	ebx, 12
	mov	ecx, [buf_len]
	int	0x40

	mov	[buf], eax

	cmp	eax, 0
	jne	_mem_ok

	popad
	mov	eax, -1
	ret	12

_mem_ok:
	mov	dword [file_in],    0
	mov	dword [file_in+4],  0x600
	mov	dword [file_in+8],  0
	mov	eax, [buf_len]
	sub	eax, 0x600
	mov	[buf_len], eax
	mov	dword [file_in+12], eax
	mov	eax, [buf]
	mov	dword [file_in+16], eax
	mov	byte  [file_in+20], 0
	mov	eax, [filename]
	mov	dword [file_in+21], eax

	mov	eax, 70
	mov	ebx, file_in
	int	0x40

	call	translate

	mov	eax, 68
	mov	ebx, 13
	mov	ecx, [buf]
	int	0x40

	popad

	mov	eax, [result]
	ret	12

translate:
	pushad

	xor	ebx, ebx
	mov	[result], 0

	mov	ecx, [buf]	; ¢ÂÆ§≠Æ© °„‰•‡ 
	mov	edx, [buffer]	; ¢ÎÂÆ§≠Æ© °„‰•‡

_translate_loop:


	cmp	ebx, [buf_len]
	je	_translate_end

	mov	byte al, [ecx]
	inc	ecx
	inc	ebx
	mov	byte ah, [ecx]

	cmp	ah, 0
	jne	_translate_next1 

	cmp	al, 0
	je	_translate_end

	cmp	al, 13
	jne	_translate_no13

	mov	byte [edx], 0x0d
	inc	edx
	mov	byte [edx], 0x0a
	inc	edx
	mov	byte [edx], 32
	inc	edx
	mov	byte [edx], 32
	inc	edx
	mov	byte [edx], 32
	inc	edx
	mov	al, 32
	jmp	_translate_copy

_translate_no13:
	
	cmp	al, 0x1f
	jg	_translate_eng_1

	inc	ecx
	inc	ebx
	jmp	_translate_loop	

_translate_eng_1:

	cmp	al, 0x7f
	jle	_translate_eng_2

	inc	ecx
	inc	ebx
	jmp	_translate_loop	

_translate_eng_2:
	jmp	_translate_copy

_translate_next1:

	cmp	ah, 4
	jne	_translate_next2

	cmp	al, 0x2f
	jb	_translate_rus_s

	and	eax, 0xff
	add	eax, rus_big
	sub	eax, 0x10

	mov	al, [eax]
	
	jmp	_translate_copy	

_translate_rus_s:
	and	eax, 0xff
	add	eax, rus_small
	sub	eax, 0x30

	mov	al, [eax]
	
	jmp	_translate_copy	


_translate_next2:
	cmp	ah, 0x20
	jne	_translate_next3

_translate_next3:

_translate_copy:
	mov	byte [edx], al

	inc	ecx
	inc	edx
	inc	ebx
	inc	[result]

	jmp	_translate_loop

_translate_end:
	popad
	ret



align 16
EXPORTS:
		dd	szVersion,	version
		dd	szConvert,	convert
	        dd      0,		0

szVersion	db	'version',0
szConvert	db	'convert',0

buf		dd	0
buf_len		dd	0
result		dd	0

mode		dd	0
buffer		dd	0
filename	dd	0

rus_big		db 	'ÄÅÇÉÑÖÜáàâäãåçéèêëíìîïñóòôöõúùûü'
rus_small	db	'†°¢£§•¶ß®©™´¨≠ÆØ‡·‚„‰ÂÊÁËÈÍÎÏÌÓÔ'

section '.data' data readable writable align 16

file_in		rd	1
		rd	1
		rd	1
		rd	1
		rd	1
		rb	1
		rd	1

bdvk		rd	1
		rb	1
		rb	3
		rb	4
		rb	4
		rb	4
		rb	4
		rb	4
		rb	4
		rd	1
		rd	1

