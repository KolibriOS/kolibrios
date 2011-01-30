; Kolibri kernel packer
; (C) copyright diamond 2006, 2007
;
;   This program is free software; you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation; either version 2 of the License, or
;   (at your option) any later version.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.

; Uses LZMA compression library by Igor Pavlov
; (for more information on LZMA and 7-Zip visit http://www.7-zip.org)
; (plain-C packer is ported by diamond)

format MS COFF

extrn '_lzma_compress@16' as lzma_compress
extrn '_lzma_set_dict_size@4' as lzma_set_dict_size

section '.text' code executable readable

die_with_err:
        pop     esi
@@:
        lodsb
        test    al, al
        jz      @f
        mov     cl, al
        push    63
        pop     eax
        push    1
        pop     ebx
        int     40h
        jmp     @b
@@:
        mov     al, 63
        mov     cl, 13
        int     40h
        mov     cl, 10
        int     40h
        or      eax, -1
        int     40h

public _start
_start:
        push    70
        pop     eax
        mov     ebx, fn70_read
        int     40h
        cmp     eax, 6
        jz      read_ok
read_err:
        call    die_with_err
        db      'KerPack: cannot load kernel.mnt',0
read_ok:
        push    18
        call    lzma_set_dict_size
; find jump to 32-bit code
        mov     edi, infile - 1
@@:
        inc     edi
        cmp     dword [edi], 0E88EE08Eh         ; mov fs,ax/mov gs,ax
        jnz     @b
        cmp     dword [edi+4], 00BCD08Eh      ; mov ss,ax/mov esp,00xxxxxx
        jnz     @b
        add     edi, 11
        mov     [inptr], edi
        sub     edi, infile
        mov     [indelta], edi
        lea     eax, [ebx+0x10000]
        mov     [..loader_patch3+2], eax
        sub     ebx, edi
        mov     [insize], ebx
        call    preprocess_calltrick2
        mov     al, [cti]
        mov     [loader_patch5-1], al
        mov     eax, [ctn]
        mov     [loader_patch4+1], eax
        mov     eax, [inptr]
        add     eax, outfile - infile + loader_size - 5
        push    workmem
        push    [insize]
        push    eax
        push    [inptr]
        call    lzma_compress
        add     eax, loader_size-5
        mov     [loader_patch1+6], eax
        add     eax, [indelta]
        mov     [outsize], eax
        mov     eax, [indelta]
        mov     ecx, dword [eax + outfile + loader_size - 4]
        bswap   ecx
        mov     [loader_patch2+4], ecx
        add     eax, 0x10000
        mov     [loader_patch1+1], eax
        mov     esi, infile
        mov     edi, outfile
        mov     ecx, [indelta]
        rep     movsb
        mov     esi, loader_start
        mov     ecx, loader_size
        rep     movsb
        push    70
        pop     eax
        mov     ebx, fn70_write
        int     40h
        test    eax, eax
        jz      @f
        call    die_with_err
        db      'KerPack: cannot save kernel.mnt',0
@@:
        call    die_with_err
        db      'KerPack: all is OK',0

preprocess_calltrick2:
; input preprocessing
	mov	edi, ct1
	xor	eax, eax
	push	edi
	mov	ecx, 256/4
	rep	stosd
	pop	edi
	mov	ecx, ebx
	mov	esi, [inptr]
	mov	ebx, inbuftmp
	xchg	eax, edx
input_pre2:
	lodsb
@@:
	cmp	al, 0Fh
	jnz	ip1
	dec	ecx
	jz	input_pre_done2
	lodsb
	cmp	al, 80h
	jb	@b
	cmp	al, 90h
	jb	@f
ip1:
	sub	al, 0E8h
	cmp	al, 1
	ja	input_pre_cont2
@@:
	cmp	ecx, 5
	jb	input_pre_done2
	lodsd
	add	eax, esi
	sub	eax, [inptr]
	cmp	eax, [insize]
	jae	xxx2
	cmp	eax, 1000000h
	jae	xxx2
	sub	ecx, 4
	xchg	al, ah
	rol	eax, 16
	xchg	al, ah
	mov	[esi-4], eax
	inc	edx
	mov	[ebx], esi
	add	ebx, 4
	jmp	input_pre_cont2
xxx2:	sub	esi, 4
	movzx	eax, byte [esi]
	mov	byte [eax+edi], 1
input_pre_cont2:
	loop	input_pre2
input_pre_done2:
	mov	[ctn], edx
	xor	eax, eax
	mov	ecx, 256
	repnz	scasb
	jnz	pack_calltrick_done
	not	cl
	mov	[cti], cl
@@:
	cmp	ebx, inbuftmp
	jz	pack_calltrick_done
	sub	ebx, 4
	mov	eax, [ebx]
	mov	[eax-4], cl
	jmp	@b
pack_calltrick_done:
	ret

include 'loader_lzma.asm'

section '.data' data readable writeable
        db      'MENUET01'
        dd      1
        dd      _start
        dd      bss_start       ; i_end
        dd      bss_end         ; memory
        dd      mtstack_end     ; esp
        dd      0               ; params
        dd      0               ; icon
fn70_read:
        dd      0
        dd      0
        dd      0
        dd      200*1024
        dd      infile
filename db      '/rd/1/kernel.mnt',0
fn70_write:
        dd      2
        dd      0
        dd      0
outsize dd      ?
        dd      outfile
        db      0
        dd      filename
section '.bss' readable writeable
bss_start:
        align   4
inptr           dd      ?
indelta         dd      ?
insize          dd      ?
ct1             rb      256
ctn             dd      ?
cti             db      ?

align 4
mtstack         rb      1000h
mtstack_end:

infile          rb      200*1024
inbuftmp        rb      200*1024
outfile         rb      200*1024
workmem         rb      6A8000h
bss_end:
