
format MS COFF

public EXPORTS

section '.flat' code readable align 16

; int version()
version:
	mov	eax, 1
	ret


; void __stdcall dos2win (const char text_in[], char text_out[])
dos2win:
	push	ecx
	push	edx

	mov	ecx, [esp+12]
	mov	edx, [esp+16]

	pushad

	mov	ebx, d2w
	mov	[charset], ebx
	call	translate8

	popad

	pop	edx
	pop	ecx
	ret	8

; void __stdcall win2dos (const char text_in[], char text_out[])
win2dos:
	push	ecx
	push	edx

	mov	ecx, [esp+12]
	mov	edx, [esp+16]

	pushad

	mov	ebx, w2d
	mov	[charset], ebx
	call	translate8

	popad

	pop	edx
	pop	ecx
	ret	8

; void __stdcall koi2dos (const char text_in[], char text_out[])
koi2dos:
	push	ecx
	push	edx

	mov	ecx, [esp+12]
	mov	edx, [esp+16]

	pushad

	mov	ebx, k2d
	mov	[charset], ebx
	call	translate8

	popad

	pop	edx
	pop	ecx
	ret	8

translate8:
	mov	byte al, [ecx]

	cmp	al, 0
	je	_translate8_end

	cmp	al, 127
	jb	_translate8_copy

	and	eax, 127
	mov	ebx, [charset]
	add	ebx, eax	

	mov	byte al, [ebx]

	_translate8_copy:
		mov	byte [edx], al

		inc	edx
		inc	ecx

		jmp	translate8

	_translate8_end:
		mov	byte [edx], 0
		ret


align 16
EXPORTS:
		dd	szVersion,	version
		dd	szDos2win,	dos2win
		dd	szWin2dos,	win2dos
		dd	szKoi2dos,	koi2dos
		dd	0,		0

szVersion	db	'version',0
szDos2win	db	'dos2win',0
szWin2dos	db	'win2dos',0
szKoi2dos	db	'koi2dos',0

d2w		db	192, 193, 194, 195, 196, 197, 198, 199 
		db	200, 201, 202, 203, 204, 205, 206, 207
		db	208, 209, 210, 211, 212, 213, 214, 215
		db	216, 217, 218, 219, 220, 221, 222, 223
		db	224, 225, 226, 227, 228, 229, 230, 231
		db	232, 233, 234, 235, 236, 237, 238, 239
		db	32,  32,  32,  32,  32,  32,  32,  32
		db	32,  32,  32,  32,  32,  32,  32,  32
		db	32,  32,  32,  32,  32,  32,  32,  32
		db	32,  32,  32,  32,  32,  32,  32,  32
		db	32,  32,  32,  32,  32,  32,  32,  32
		db	32,  32,  32,  32,  32,  32,  32,  32
		db	240, 241, 242, 243, 244, 245, 246, 247
		db	248, 249, 250, 251, 252, 253, 254, 255
		db	168, 184, 170, 186, 175, 191, 161, 162
		db	176, 32,  32,  32,  185, 164, 32,  32


w2d		db	32,  32,  44,  32,  34,  32,  32,  32
		db	94,  32,  32,  60,  32,  32,  32,  32
		db	32,  39,  39,  34,  34,  32,  45,  45 
		db	126, 32,  32,  62,  32,  32,  32,  32
		db	32,  246, 247, 74,  253, 131, 32,  32
		db	240, 32,  242, 34,  32,  32,  32,  244
		db	248, 32,  73,  105, 163, 32,  32,  250
		db	241, 252, 243, 34,  106, 83,  115, 245
		db	128, 129, 130, 131, 132, 133, 134, 135
		db	136, 137, 138, 139, 140, 141, 142, 143
		db	144, 145, 146, 147, 148, 149, 150, 151
		db	152, 153, 154, 155, 156, 157, 158, 159
		db	160, 161, 162, 163, 164, 165, 166, 167
		db	168, 169, 170, 171, 172, 173, 174, 175
		db	224, 225, 226, 227, 228, 229, 230, 231
		db	232, 233, 234, 235, 236, 237, 238, 239


k2d		db	196, 179, 218, 191, 192, 217, 195, 180
		db	194, 193, 197, 223, 220, 219, 221, 222
		db	176, 177, 178, 32,  254, 249, 251, 32 
		db	32,  32,  32,  32,  248, 32,  250, 32 
		db	205, 186, 184, 241, 214, 201, 184, 183
		db	187, 212, 211, 200, 190, 189, 188, 198
		db	199, 204, 181, 240, 182, 185, 209, 210
		db	203, 207, 208, 202, 216, 215, 206, 32 
		db	238, 160, 161, 230, 164, 165, 228, 163
		db	229, 168, 169, 170, 171, 172, 173, 174
		db	175, 239, 224, 225, 226, 227, 166, 162
		db	236, 235, 167, 232, 237, 233, 231, 234 
		db	158, 128, 129, 150, 132, 133, 148, 131
		db	149, 136, 137, 138, 139, 140, 141, 142
		db	143, 159, 144, 145, 146, 147, 134, 130
		db	156, 155, 135, 152, 157, 153, 151, 154

section '.data' data readable writable align 16

charset		rd	1