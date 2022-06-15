bits 32

section .text

%define which	[ebp+36]	; int which
%define line	[ebp+40]	; int line
%define where	[ebp+44]	; unsigned char *where

%define vram	[ebp+24]	; unsigned char *vram
%define reg	[ebp+28]	; unsigned char reg[0x20]
%define highpal [ebp+32]	; unsigned int *highpal

;%define cache_align times ($$-$) & 3 nop	; Align to 4-byte boundary
;%define cache_align times ($$-$) & 7 nop	; Align to 8-byte boundary
%define cache_align times ($$-$) & 31 nop	; Align to 32-byte boundary

global asm_tiles_init
global drawtile1_solid
global drawtile1
global drawtile2_solid
global drawtile2
global drawtile3_solid
global drawtile3
global drawtile4_solid
global drawtile4

; Neat utility macro
%macro triple_xor 2
	xor %1, %2	; Triple XOR for a neat register exchange  ;)
	xor %2, %1
	xor %1, %2
%endmacro

%macro blit_pixel1 1-*		; 8bpp blitting, solid
	mov eax, ebx
	and eax, %1
    %if %0 > 1
        shr eax, byte %2
    %endif
	or eax, [esi]
	mov byte [edi], al
	inc edi
%endmacro

%macro blit_pixel1_trans 1-*	; 8bpp blitting, transparent
	mov eax, ebx
	and eax, %1
	jz %%trans
    %if %0 > 1
        shr eax, byte %2
    %endif
	or eax, [esi]
	mov byte [edi], al
    %%trans:
	inc edi
%endmacro

%macro blit_pixel2 1-*		; 16bpp blitting, solid
	mov eax, ebx
	and eax, %1
    %if %0 > 1
        shr eax, byte %2
    %endif
	lea edx, [esi+eax*4]
	mov eax, [edx]
	mov word [edi], ax
	add edi, byte 2
%endmacro

%macro blit_pixel2_trans 1-*	; 16bpp blitting, transparent
	mov eax, ebx
	and eax, %1
	jz %%trans
    %if %0 > 1
        shr eax, byte %2
    %endif
	lea edx, [esi+eax*4]
	mov eax, [edx]
	mov word [edi], ax
    %%trans:
	add edi, byte 2
%endmacro

%macro blit_pixel3 1-*		; 24bpp blitting, solid
	mov eax, ebx
	and eax, %1
    %if %0 > 1
        shr eax, byte %2
    %endif
	lea edx, [esi+eax*4+1]
	mov ax, word [edx]
	mov word [edi], ax
	add edi, 2
	dec edx
	mov al, byte [edx]
	mov byte [edi], al
	inc edi
%endmacro

%macro blit_pixel3_trans 1-*	; 24bpp blitting, transparent
	mov eax, ebx
	and eax, %1
	jz %%trans
    %if %0 > 1
        shr eax, byte %2
    %endif
	lea edx, [esi+eax*4+1]
	mov ax, word [edx]
	mov word [edi], ax
	add edi, 2
	dec edx
	mov al, byte [edx]
	mov byte [edi], al
	inc edi
	jmp %%next
    %%trans:
	add edi, byte 3
    %%next:
%endmacro

%macro blit_pixel4 1-*		; 32bpp blitting, solid
	mov eax, ebx
	and eax, %1
    %if %0 > 1
        shr eax, byte %2
    %endif
	lea edx, [esi+eax*4]
	mov eax, [edx]
	mov [edi], eax
	add edi, byte 4
%endmacro

%macro blit_pixel4_trans 1-*	; 32bpp blitting, transparent
	mov eax, ebx
	and eax, %1
	jz %%trans
    %if %0 > 1
        shr eax, byte %2
    %endif
	lea edx, [esi+eax*4]
	mov eax, [edx]
	mov [edi], eax
    %%trans:
	add edi, byte 4
%endmacro

; ----------------------------------------
; int _asm_tiles_init
;   (unsigned char *vram, unsigned char *reg, unsigned char *highpal)
; ----------------------------------------

	cache_align

asm_tiles_init:

	push eax
	push ebx
	push edx
	push esp
	push ebp
	mov ebp, esp

	mov eax, vram
	mov ebx, reg
	mov edx, highpal
	mov [__vram], eax
	mov [__reg], ebx
	mov [__highpal], edx

	pop ebp
	pop esp
	pop edx
	pop ebx
	pop eax

	ret

	cache_align

; ----------------------------------------
; int _drawtile1_solid
;   (int which, int line, unsigned char *where)
; ----------------------------------------

	cache_align

drawtile1_solid:

	pushad
	mov ebp, esp

.setup:

.get_pal:	

	mov ebx, which
	mov esi, [__highpal]
	mov eax, ebx
	shr eax, byte 7
	and eax, 0xc0
	add esi, eax
	mov edi, [__reg]
	push esi

.check_y_flip:

	mov eax, ebx
	xor ecx, ecx
	mov edx, line
	test eax, 0x1000

	jz .check_interlace

.y_flipped:

	xor edx, byte 7

	cache_align

.check_interlace:

	mov esi, [__reg]
	mov cl, [esi+12]
	mov esi, [__vram]
	and eax, 0x7ff
	test cl, byte 0x2

	jz .no_interlace

.interlace:

	lea edx, [edx*8]
	shl eax, 6
	jmp .check_x_flip

	cache_align

.no_interlace:

	lea edx, [edx*4]
	shl eax, 5

	cache_align

.check_x_flip:

	add eax, edx
	mov edi, where
	lea esi, [esi+eax]
	mov ebx, [esi]
	pop esi
	mov eax, which
	test eax, 0x800

	jz near .x_not_flipped

.x_flipped:

	blit_pixel1 0x0f000000, 24	; pixel 8
	blit_pixel1 0xf0000000, 28	; ..... 7
	blit_pixel1 0x000f0000, 16	; ..... 6
	blit_pixel1 0x00f00000, 20	; ..... 5
	blit_pixel1 0x00000f00, 8	; ..... 4
	blit_pixel1 0x0000f000, 12	; ..... 3
	blit_pixel1 0x0000000f		; ..... 2
	blit_pixel1 0x000000f0, 4	; ..... 1

	jmp .cleanup

	cache_align

.x_not_flipped:

	blit_pixel1 0x000000f0, 4	; pixel 1
	blit_pixel1 0x0000000f		; ..... 2
	blit_pixel1 0x0000f000, 12	; ..... 3
	blit_pixel1 0x00000f00, 8	; ..... 4
	blit_pixel1 0x00f00000, 20	; ..... 5
	blit_pixel1 0x000f0000, 16	; ..... 6
	blit_pixel1 0xf0000000, 28	; ..... 7
	blit_pixel1 0x0f000000, 24	; ..... 8

	cache_align

.cleanup:

	popad

	ret

	cache_align

; ----------------------------------------

drawtile1:

	pushad
	mov ebp, esp

.setup:

.get_pal:	

	mov ebx, which
	mov esi, [__highpal]
	mov eax, ebx
	shr eax, byte 7
	and eax, 0xc0
	add esi, eax
	push esi

.check_y_flip:

	mov eax, ebx
	xor ecx, ecx
	mov edx, line
	test eax, 0x1000

	jz .check_interlace

.y_flipped:

	xor edx, byte 7

	cache_align

.check_interlace:

	mov esi, [__reg]
	mov cl, [esi+12]
	mov esi, [__vram]
	and eax, 0x7ff
	test cl, byte 0x2

	jz .no_interlace

.interlace:

	lea edx, [edx*8]
	shl eax, 6
	jmp .check_x_flip

	cache_align

.no_interlace:

	lea edx, [edx*4]
	shl eax, 5

	cache_align

.check_x_flip:

	add eax, edx
	mov edi, where
	lea esi, [esi+eax]
	mov ebx, [esi]
	pop esi
	test ebx, ebx

	jz near .cleanup		; Don't waste time if the tile is blank!

	mov eax, which
	test eax, 0x800

	jz near .x_not_flipped

.x_flipped:

	blit_pixel1_trans 0x0f000000, 24	; pixel 8
	blit_pixel1_trans 0xf0000000, 28	; ..... 7
	blit_pixel1_trans 0x000f0000, 16	; ..... 6
	blit_pixel1_trans 0x00f00000, 20	; ..... 5
	blit_pixel1_trans 0x00000f00, 8		; ..... 4
	blit_pixel1_trans 0x0000f000, 12	; ..... 3
	blit_pixel1_trans 0x0000000f		; ..... 2
	blit_pixel1_trans 0x000000f0, 4		; ..... 1

	jmp .cleanup

	cache_align

.x_not_flipped:

	blit_pixel1_trans 0x000000f0, 4		; pixel 1
	blit_pixel1_trans 0x0000000f		; ..... 2
	blit_pixel1_trans 0x0000f000, 12	; ..... 3
	blit_pixel1_trans 0x00000f00, 8		; ..... 4
	blit_pixel1_trans 0x00f00000, 20	; ..... 5
	blit_pixel1_trans 0x000f0000, 16	; ..... 6
	blit_pixel1_trans 0xf0000000, 28	; ..... 7
	blit_pixel1_trans 0x0f000000, 24	; ..... 8

	cache_align

.cleanup:

	popad

	ret

	cache_align

; ----------------------------------------

	cache_align

drawtile2_solid:

	pushad
	mov ebp, esp

.setup:

.get_pal:	

	mov ebx, which
	mov esi, [__highpal]
	mov ecx, esi
	mov eax, ebx
	shr eax, byte 7
	and eax, 0xc0
	add esi, eax
; -
	mov edi, [__reg]
	mov edx, [edi + 7]
	push dword [esi]
	and edx, 0x3f
	mov eax, [ecx + edx*4]
	mov [esi], eax
; -
	push esi

.check_y_flip:

	mov eax, ebx
	xor ecx, ecx
	mov edx, line
	test eax, 0x1000

	jz .check_interlace

.y_flipped:

	xor edx, byte 7

	cache_align

.check_interlace:

	mov esi, [__reg]
	mov cl, [esi+12]
	mov esi, [__vram]
	and eax, 0x7ff
	test cl, byte 0x2

	jz .no_interlace

.interlace:

	lea edx, [edx*8]
	shl eax, 6
	jmp .check_x_flip

	cache_align

.no_interlace:

	lea edx, [edx*4]
	shl eax, 5

	cache_align

.check_x_flip:

	add eax, edx
	mov edi, where
	lea esi, [esi+eax]
	mov ebx, [esi]
	pop esi
	mov eax, which
	test eax, 0x800

	jz near .x_not_flipped

.x_flipped:

	blit_pixel2 0x0f000000, 24	; pixel 8
	blit_pixel2 0xf0000000, 28	; ..... 7
	blit_pixel2 0x000f0000, 16	; ..... 6
	blit_pixel2 0x00f00000, 20	; ..... 5
	blit_pixel2 0x00000f00, 8	; ..... 4
	blit_pixel2 0x0000f000, 12	; ..... 3
	blit_pixel2 0x0000000f		; ..... 2
	blit_pixel2 0x000000f0, 4	; ..... 1

	jmp .cleanup

	cache_align

.x_not_flipped:

	blit_pixel2 0x000000f0, 4	; pixel 1
	blit_pixel2 0x0000000f		; ..... 2
	blit_pixel2 0x0000f000, 12	; ..... 3
	blit_pixel2 0x00000f00, 8	; ..... 4
	blit_pixel2 0x00f00000, 20	; ..... 5
	blit_pixel2 0x000f0000, 16	; ..... 6
	blit_pixel2 0xf0000000, 28	; ..... 7
	blit_pixel2 0x0f000000, 24	; ..... 8

	cache_align

.cleanup:
	pop dword [esi]
	popad
	ret

	cache_align

; ----------------------------------------

	cache_align

drawtile2:

	pushad
	mov ebp, esp

.get_pal:	

	mov ebx, which
	mov esi, [__highpal]
	mov eax, ebx	
	shr eax, 7
	and eax, 0xc0
	add esi, eax
	push esi

.check_y_flip:

	mov eax, ebx
	xor ecx, ecx
	mov edx, line
	test eax, 0x1000

	jz .check_interlace

.y_flipped:

	xor edx, byte 7

	cache_align

.check_interlace:

	mov esi, [__reg]
	mov cl, [esi+12]
	mov esi, [__vram]
	and eax, 0x7ff
	test cl, byte 0x2

	jz .no_interlace

.interlace:

	lea edx, [edx*8]
	shl eax, 6

	jmp .check_x_flip

	cache_align

.no_interlace:
	lea edx, [edx*4]
	shl eax, 5

	cache_align

.check_x_flip:

	add eax, edx
	mov edi, where
	lea esi, [esi+eax]
	mov ebx, [esi]
	pop esi
	test ebx, ebx

	jz near .cleanup		; Don't waste time if the tile is blank!

	mov eax, which
	test eax, 0x800

	jz near .x_not_flipped

.x_flipped:

	blit_pixel2_trans 0x0f000000, 24	; pixel 8
	blit_pixel2_trans 0xf0000000, 28	; ..... 7
	blit_pixel2_trans 0x000f0000, 16	; ..... 6
	blit_pixel2_trans 0x00f00000, 20	; ..... 5
	blit_pixel2_trans 0x00000f00, 8		; ..... 4
	blit_pixel2_trans 0x0000f000, 12	; ..... 3
	blit_pixel2_trans 0x0000000f		; ..... 2
	blit_pixel2_trans 0x000000f0, 4		; ..... 1

	jmp .cleanup

	cache_align

.x_not_flipped:

	blit_pixel2_trans 0x000000f0, 4		; pixel 1
	blit_pixel2_trans 0x0000000f		; ..... 2
	blit_pixel2_trans 0x0000f000, 12	; ..... 3
	blit_pixel2_trans 0x00000f00, 8		; ..... 4
	blit_pixel2_trans 0x00f00000, 20	; ..... 5
	blit_pixel2_trans 0x000f0000, 16	; ..... 6
	blit_pixel2_trans 0xf0000000, 28	; ..... 7
	blit_pixel2_trans 0x0f000000, 24	; ..... 8

	cache_align

.cleanup:
	popad
	ret

	cache_align

; ----------------------------------------

drawtile3_solid:

	pushad
	mov ebp, esp

.setup:

.get_pal:	

	mov ebx, which
	mov esi, [__highpal]
	mov ecx, esi
	mov eax, ebx
	shr eax, byte 7
	and eax, 0xc0
	add esi, eax
; -
	mov edi, [__reg]
	mov edx, [edi + 7]
	push dword [esi]
	and edx, 0x3f
	mov eax, [ecx + edx*4]
	mov [esi], eax
; -
	push esi

.check_y_flip:

	mov eax, ebx
	xor ecx, ecx
	mov edx, line
	test eax, 0x1000

	jz .check_interlace

.y_flipped:

	xor edx, byte 7

	cache_align

.check_interlace:

	mov esi, [__reg]
	mov cl, [esi+12]
	mov esi, [__vram]
	and eax, 0x7ff
	test cl, byte 0x2

	jz .no_interlace

.interlace:

	lea edx, [edx*8]
	shl eax, 6
	jmp .check_x_flip

	cache_align

.no_interlace:

	lea edx, [edx*4]
	shl eax, 5

	cache_align

.check_x_flip:

	add eax, edx
	mov edi, where
	lea esi, [esi+eax]
	mov ebx, [esi]
	pop esi
	mov eax, which
	test eax, 0x800

	jz near .x_not_flipped

.x_flipped:

	blit_pixel3 0x0f000000, 24	; pixel 8
	blit_pixel3 0xf0000000, 28	; ..... 7
	blit_pixel3 0x000f0000, 16	; ..... 6
	blit_pixel3 0x00f00000, 20	; ..... 5
	blit_pixel3 0x00000f00, 8	; ..... 4
	blit_pixel3 0x0000f000, 12	; ..... 3
	blit_pixel3 0x0000000f		; ..... 2
	blit_pixel3 0x000000f0, 4	; ..... 1

	jmp .cleanup

	cache_align

.x_not_flipped:

	blit_pixel3 0x000000f0, 4	; pixel 1
	blit_pixel3 0x0000000f		; ..... 2
	blit_pixel3 0x0000f000, 12	; ..... 3
	blit_pixel3 0x00000f00, 8	; ..... 4
	blit_pixel3 0x00f00000, 20	; ..... 5
	blit_pixel3 0x000f0000, 16	; ..... 6
	blit_pixel3 0xf0000000, 28	; ..... 7
	blit_pixel3 0x0f000000, 24	; ..... 8

	cache_align

.cleanup:

	pop dword [esi]
	popad
	ret

	cache_align

; ----------------------------------------

drawtile3:

	pushad
	mov ebp, esp

.setup:

.get_pal:	

	mov ebx, which
	mov esi, [__highpal]
	mov eax, ebx
	shr eax, byte 7
	and eax, 0xc0
	add esi, eax
	push esi

.check_y_flip:

	mov eax, ebx
	xor ecx, ecx
	mov edx, line
	test eax, 0x1000

	jz .check_interlace

.y_flipped:

	xor edx, byte 7

	cache_align

.check_interlace:

	mov esi, [__reg]
	mov cl, [esi+12]
	mov esi, [__vram]
	and eax, 0x7ff
	test cl, byte 0x2

	jz .no_interlace

.interlace:

	lea edx, [edx*8]
	shl eax, 6
	jmp .check_x_flip

	cache_align

.no_interlace:

	lea edx, [edx*4]
	shl eax, 5

	cache_align

.check_x_flip:

	add eax, edx
	mov edi, where
	lea esi, [esi+eax]
	mov ebx, [esi]
	pop esi
	test ebx, ebx

	jz near .cleanup		; Don't waste time if the tile is blank!

	mov eax, which
	test eax, 0x800

	jz near .x_not_flipped

.x_flipped:

	blit_pixel3_trans 0x0f000000, 24	; pixel 8
	blit_pixel3_trans 0xf0000000, 28	; ..... 7
	blit_pixel3_trans 0x000f0000, 16	; ..... 6
	blit_pixel3_trans 0x00f00000, 20	; ..... 5
	blit_pixel3_trans 0x00000f00, 8		; ..... 4
	blit_pixel3_trans 0x0000f000, 12	; ..... 3
	blit_pixel3_trans 0x0000000f		; ..... 2
	blit_pixel3_trans 0x000000f0, 4		; ..... 1

	jmp .cleanup

	cache_align

.x_not_flipped:

	blit_pixel3_trans 0x000000f0, 4		; pixel 1
	blit_pixel3_trans 0x0000000f		; ..... 2
	blit_pixel3_trans 0x0000f000, 12	; ..... 3
	blit_pixel3_trans 0x00000f00, 8		; ..... 4
	blit_pixel3_trans 0x00f00000, 20	; ..... 5
	blit_pixel3_trans 0x000f0000, 16	; ..... 6
	blit_pixel3_trans 0xf0000000, 28	; ..... 7
	blit_pixel3_trans 0x0f000000, 24	; ..... 8

	cache_align

.cleanup:

	popad

	ret

	cache_align

; ----------------------------------------

drawtile4_solid:

	pushad
	mov ebp, esp

.setup:

.get_pal:	

	mov ebx, which
	mov esi, [__highpal]
	mov ecx, esi
	mov eax, ebx
	shr eax, byte 7
	and eax, 0xc0
	add esi, eax
; -
	mov edi, [__reg]
	mov edx, [edi + 7]
	push dword [esi]
	and edx, 0x3f
	mov eax, [ecx + edx*4]
	mov [esi], eax
; -
	push esi

.check_y_flip:

	mov eax, ebx
	xor ecx, ecx
	mov edx, line
	test eax, 0x1000

	jz .check_interlace

.y_flipped:

	xor edx, byte 7

	cache_align

.check_interlace:

	mov esi, [__reg]
	mov cl, [esi+12]
	mov esi, [__vram]
	and eax, 0x7ff
	test cl, byte 0x2

	jz .no_interlace

.interlace:

	lea edx, [edx*8]
	shl eax, 6
	jmp .check_x_flip

	cache_align

.no_interlace:

	lea edx, [edx*4]
	shl eax, 5

	cache_align

.check_x_flip:

	add eax, edx
	mov edi, where
	lea esi, [esi+eax]
	mov ebx, [esi]
	pop esi
	mov eax, which
	test eax, 0x800

	jz near .x_not_flipped

.x_flipped:

	blit_pixel4 0x0f000000, 24	; pixel 8
	blit_pixel4 0xf0000000, 28	; ..... 7
	blit_pixel4 0x000f0000, 16	; ..... 6
	blit_pixel4 0x00f00000, 20	; ..... 5
	blit_pixel4 0x00000f00, 8	; ..... 4
	blit_pixel4 0x0000f000, 12	; ..... 3
	blit_pixel4 0x0000000f		; ..... 2
	blit_pixel4 0x000000f0, 4	; ..... 1

	jmp .cleanup

	cache_align

.x_not_flipped:

	blit_pixel4 0x000000f0, 4	; pixel 1
	blit_pixel4 0x0000000f		; ..... 2
	blit_pixel4 0x0000f000, 12	; ..... 3
	blit_pixel4 0x00000f00, 8	; ..... 4
	blit_pixel4 0x00f00000, 20	; ..... 5
	blit_pixel4 0x000f0000, 16	; ..... 6
	blit_pixel4 0xf0000000, 28	; ..... 7
	blit_pixel4 0x0f000000, 24	; ..... 8

	cache_align

.cleanup:

	pop dword [esi]
	popad
	ret

	cache_align

; ----------------------------------------

drawtile4:

	pushad
	mov ebp, esp

.setup:

.get_pal:	

	mov ebx, which
	mov esi, [__highpal]
	mov eax, ebx
	shr eax, byte 7
	and eax, 0xc0
	add esi, eax
	push esi

.check_y_flip:

	mov eax, ebx
	xor ecx, ecx
	mov edx, line
	test eax, 0x1000

	jz .check_interlace

.y_flipped:

	xor edx, byte 7

	cache_align

.check_interlace:

	mov esi, [__reg]
	mov cl, [esi+12]
	mov esi, [__vram]
	and eax, 0x7ff
	test cl, byte 0x2

	jz .no_interlace

.interlace:

	lea edx, [edx*8]
	shl eax, 6
	jmp .check_x_flip

	cache_align

.no_interlace:

	lea edx, [edx*4]
	shl eax, 5

	cache_align

.check_x_flip:

	add eax, edx
	mov edi, where
	lea esi, [esi+eax]
	mov ebx, [esi]
	pop esi
	test ebx, ebx

	jz near .cleanup		; Don't waste time if the tile is blank!

	mov eax, which
	test eax, 0x800

	jz near .x_not_flipped

.x_flipped:

	blit_pixel4_trans 0x0f000000, 24	; pixel 8
	blit_pixel4_trans 0xf0000000, 28	; ..... 7
	blit_pixel4_trans 0x000f0000, 16	; ..... 6
	blit_pixel4_trans 0x00f00000, 20	; ..... 5
	blit_pixel4_trans 0x00000f00, 8		; ..... 4
	blit_pixel4_trans 0x0000f000, 12	; ..... 3
	blit_pixel4_trans 0x0000000f		; ..... 2
	blit_pixel4_trans 0x000000f0, 4		; ..... 1

	jmp .cleanup

	cache_align

.x_not_flipped:

	blit_pixel4_trans 0x000000f0, 4		; pixel 1
	blit_pixel4_trans 0x0000000f		; ..... 2
	blit_pixel4_trans 0x0000f000, 12	; ..... 3
	blit_pixel4_trans 0x00000f00, 8		; ..... 4
	blit_pixel4_trans 0x00f00000, 20	; ..... 5
	blit_pixel4_trans 0x000f0000, 16	; ..... 6
	blit_pixel4_trans 0xf0000000, 28	; ..... 7
	blit_pixel4_trans 0x0f000000, 24	; ..... 8

	cache_align

.cleanup:

	popad

	ret

	cache_align

section .data

	__vram		dd 0
	__reg		dd 0
	__highpal	dd 0

; ----------------------------------------

%ifdef NASM_STACK_NOEXEC
section .note.GNU-stack noalloc noexec nowrite progbits
%endif
