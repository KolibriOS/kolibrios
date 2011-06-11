;*****************************************************************************
; BMP to RAW convert plugin - for zSea image viewer
; Copyright (c) 2008-2011, Marat Zakiyanov aka Mario79, aka Mario
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;	 * Redistributions of source code must retain the above copyright
;	   notice, this list of conditions and the following disclaimer.
;	 * Redistributions in binary form must reproduce the above copyright
;	   notice, this list of conditions and the following disclaimer in the
;	   documentation and/or other materials provided with the distribution.
;	 * Neither the name of the <organization> nor the
;	   names of its contributors may be used to endorse or promote products
;	   derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Marat Zakiyanov ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*****************************************************************************

format MS COFF

public EXPORTS

section '.flat' code readable align 16

;include 'macros.inc'
include '../../../../macros.inc'
;---------------------------------------------------------------------
START:
	pushad
	mov	eax,dword [esp+36]
	call	.check_header
	je	@f
.no_bmp_file:
	xor	eax,eax
	mov	[raw_area],eax
	inc	eax	; data corrupt eax = 1
	jmp	.ret

.check_header:
	mov	[pointer],eax
	mov	eax,[eax]
	mov	[image_file],eax
	cmp	[eax],word 'BM'
	ret
;---------------------------------------------------------------------
@@:
	mov	[MinusHeight],byte 0
	mov	edx,[eax+22]
	test	edx,0x80000000
	jz	@f
	neg	edx
	mov	[eax+22],edx
	mov	[MinusHeight],byte 1
@@:
	xor	edx,edx
	mov	dx,[eax+28]	; color resolution 1,4,8,16,24,32 bits
;	mov	[ebx+16],edx	; store resolution BMP
	mov	[resolution],edx
	mov	ecx,[eax+30]
	mov	[compression],ecx
;---------------------------------------------------------------------	
	mov	ecx,[eax+18]	; width BMP
	imul	ecx,edx	;[resolution]
	mov	edi,ecx
	shr	ecx,3
	test	edi,7
	jz	@f
	inc	ecx
@@:	
	imul	ecx,[eax+22]	; size of pixels data area
;---------------------------------------------------------------------	
	cmp	edx,16
	jae	.get_memory	; no palette
	
	mov	eax,4
	xchg	cl,dl
	shl	eax,cl
	xchg	cl,dl
	add	ecx,eax		; palette size
;---------------------------------------------------------------------	
.get_memory:
	add	ecx,44		; header
	mcall	68,12
	cmp	eax,0
	jne	@f
	xor	eax,eax
	mov	[raw_area],eax
	mov	eax,2		; not enough memory
	jmp	.ret
@@:
	mov	[raw_area],eax
;---------------------------------------------------------------------	
	mov	edi,eax
	xor	eax,eax
	shr	ecx,2
	cld
	rep	stosd		; clear memory
;---------------------------------------------------------------------
; Create RAW header
;---------------------------------------------------------------------
	mov	eax,[raw_area]
	mov	[eax],dword 'RAW '
	mov	ebx,[image_file]
;---------------------------------------------------------------------	
	mov	ecx,[ebx+18]	; width BMP
	mov	[eax+4],ecx	; width RAW
;---------------------------------------------------------------------
	mov	ecx,[ebx+22]	; high BMP
	mov	[eax+8],ecx	; high RAW
;---------------------------------------------------------------------
	mov	ecx,[resolution]	; color resolution BMP
	mov	[eax+12],ecx	; color resolution RAW	
;---------------------------------------------------------------------
	mov	ecx,[resolution]
	cmp	ecx,16
	jb	.1
	push	eax
	mov	eax,ecx
	mov	ecx,3
	xor	edx,edx
	div	ecx
	cmp	eax,8
	jbe	@f
	mov	eax,8
@@:
	mov	ecx,eax
	pop	eax
.1:
	mov	[eax+16],cx	; channel color resolution RAW
;---------------------------------------------------------------------
	mov	ecx,[resolution]
	cmp	ecx,16
	jae	@f
	mov	[eax+18],word 1	; channels amount RAW
	jmp	.2
@@:
	mov	[eax+18],word 3	; channels amount RAW
.2:
;---------------------------------------------------------------------
	xor	edx,edx
	cmp	ecx,16
	jae	@f
	add	edx,44
@@:
	mov	[eax+20],edx	; palette pointer (offset from file start)
;---------------------------------------------------------------------
	mov	ecx,[resolution]
	cmp	ecx,16
	jae	@f
	mov	ecx,[resolution]
	push	eax
	mov	eax,1
	shl	eax,cl
	mov	ecx,eax
	pop	eax
	shl	ecx,2
	jmp	.3
@@:
	xor	ecx,ecx
.3:
	mov	[eax+24],ecx	; palette area size
;---------------------------------------------------------------------	
	add	ecx,dword 44
	mov	[eax+28],ecx	; pixels pointer (offset from file start)
;---------------------------------------------------------------------
	mov	ecx,[ebx+18]	; width BMP
	imul	ecx,[resolution]
	mov	edi,ecx
	shr	ecx,3
	test	edi,7
	jz	@f
	inc	ecx
@@:	
	imul	ecx,[ebx+22]	; high BMP
	mov	[eax+32],ecx	; pixels area size
;---------------------------------------------------------------------	
	xor	ecx,ecx		; Stub!!!
	mov	[eax+36],ecx	; Transparency pointer	(offset	from	file	start)
	mov	[eax+40],ecx	; Transparency area	size

;---------------------------------------------------------------------	
; Finish create RAW header
;---------------------------------------------------------------------	
;	mov	ebx,[pointer]
;	mov	[ebx+4],eax	; store [soi] pointer of image area
;	mov	ecx,eax
.convert:
	cmp	[resolution],16
	jae	.no_palette
;	mov	[ebx+24],eax
;	mov	edi,eax
	mov	edi,[raw_area]
	mov	edi,[edi+20]	; palette pointer (offset from file start)
	add	edi,[raw_area]
	mov	esi,[image_file]
	add	esi,54
	mov	ecx,[resolution]
	mov	eax,1
	shl	eax,cl
	mov	ecx,eax
	rep	movsd

;	mov	[ebx+4],edi	; store [soi] pointer of image area
.no_palette:
	mov	ecx,[raw_area]
	mov	ecx,[ecx+28]	; pixels pointer (offset from file start)
	add	ecx,[raw_area]
;---------------------------------------------------------------------
	mov	eax,[image_file]
	mov	ebp,[eax+18]	; width BMP
;	mov	[ebx+8],ebp	; store width
	imul	ebp,[resolution]
	mov	edi,ebp
	shr	ebp,3		; ebp = size of output scanline
	test	edi,7
	jz	@f
	inc	ebp
@@:	
	mov	eax,[eax+22]	; high BMP
;	mov	[ebx+12],eax	; store high
	dec	eax
	mul	ebp
	add	eax,ecx
	
	mov	edi,ecx
	mov	bl,[MinusHeight]
	test	bl,bl
	jnz	@f
	mov	edi,eax		; edi points to last scanline
@@:
	mov	esi,[image_file]
	add	esi,[esi+10]	; start of pixels data
	mov	ebx,[image_file]
	mov	edx,[ebx+22]	; high BMP
	add	ebx,54
	lea	eax,[ebp*2]
	mov	[delta],eax
	test	edx,edx
	jz	.ret
	jns	@f
	neg	edx
	and	[delta], 0
	mov	edi,ecx
@@:
;---------------------------------------------------------------------
	cmp	[compression],3
	je	.BI_BITFIELDS	; @f
	cmp	[compression],2
	je	RLE4
	cmp	[compression],1
	je	RLE8
	cmp	[compression],0
	je	@f
	jmp	.no_bmp_file
;---------------------------------------------------------------------
@@:
	cmp	[resolution],16
	jne	.continue
	mov	ebx,[raw_area]
	mov	[ebx+12],dword 15
	jmp	.continue
;---------------------------------------------------------------------	
.BI_BITFIELDS:
	cmp	[resolution],32
	je	.32
	cmp	[resolution],16
	jne	.continue
	mov	ebx,[raw_area]
	mov	eax,[image_file]
	cmp	[eax+54],dword 0x7C00
	jne	@f
	mov	[ebx+12],dword 15
	jmp	.continue
;---------------------------------------------------------------------
@@:
	cmp	[eax+54],dword 0xF800
	jne	@f
	mov	[ebx+12],dword 16
	jmp	.continue
;---------------------------------------------------------------------
@@:
	cmp	[eax+54],dword 0xF00
	jne	.no_bmp_file	; @f
	mov	[ebx+12],dword 15
	
@@:
	mov	ebx,ebp
	neg	ebx
	and	ebx,3
	mov	[aligner],ebx
	shr	ebp,1
	mov	cl,[MinusHeight]
	test	cl,cl
	jnz	.start_16_1
align	4
.start_16:
	mov	ecx,ebp
	call	.process_16b_x4r4g4b4
	sub	edi,[delta]
	add	esi,[aligner]
	dec	edx
	jnz	.start_16
	jmp	.ret_ok
;---------------------------------------------------------------------
.process_16b_x4r4g4b4:
	cld
@@:
	lodsw
	xor	ebx,ebx
	mov	bx,ax
	xor	eax,eax
	ror	ebx,8
	and	bl,0xf
	mov	al,bl
	shl	eax,6
	rol	ebx,4
	and	bx,0xf
	shl	bl,1
	add	ax,bx
	shl	eax,5
	rol	ebx,4
	and	bx,0xf
	shl	bl,1
	add	ax,bx
	stosw
	dec	ecx
	jnz	@r
	ret
;---------------------------------------------------------------------
.start_16_1:	;negative value of Height
	mov	ecx,ebp
	call	.process_16b_x4r4g4b4
	add	esi,[aligner]
	dec	edx
	jnz	.start_16_1
	jmp	.ret_ok
;---------------------------------------------------------------------
.32:
	mov	eax,[image_file]
	cmp	[eax+54],dword 0xFF000000
	jne	.no_bmp_file
	shr	ebp,2

	mov	al,[MinusHeight]
	test	al,al
	jnz	.start_32_1
align	4
.start_32:
	mov	ecx,ebp
@@:
	cld
	lodsd
	shr	eax,8
	stosd
	dec	ecx
	jnz	@r
	sub	edi,[delta]
	dec	edx
	jnz	.start_32
	jmp	.ret_ok
;---------------------------------------------------------------------
.start_32_1:	;negative value of Height
	mov	ecx,ebp
	imul	ecx,edx
@@:
	cld
	lodsd
	shr	eax,8
	stosd
	dec	ecx
	jnz	@r
	jmp	.ret_ok
;---------------------------------------------------------------------
.continue:
	mov	eax,ebp
	neg	eax
	and	eax,3
	mov	cl,[MinusHeight]
	test	cl,cl
	jz	.start_24
	cld
align	4
@@:	;negative value of Height
	mov	ecx,ebp
	rep	movsb
	add	esi,eax
	dec	edx
	jnz	@r
	jmp	.ret_ok
;---------------------------------------------------------------------	
align	4
.start_24:
@@:
	mov	ecx,ebp
	rep	movsb
	sub	edi,[delta]
	add	esi,eax
	dec	edx
	jnz	.start_24	; @r
.ret_ok:
	mov	eax,0		; convert OK
.ret:
	mov	ebx,[pointer]
	mov	[ebx+8],eax	; store return code
	mov	eax,[raw_area]
	mov	[ebx+4],eax	; store RAW pointer
	popad
	ret	4
;---------------------------------------------------------------------
RLE4:
	cmp	[resolution],4
	jne	START.no_bmp_file
	xor	ebx,ebx
	xor	edx,edx
align	4
.start:
	cld
	lodsb
	cmp	al,0
	jnz	.Encoded_Mode
	cld
	lodsb
	cmp	al,0
	jz	.end_line
	cmp	al,1
	jz	START.ret_ok	; .end_bitmap
	cmp	al,2
	jz	.Delta
	
.Absolute_Mode:
	xor	ecx,ecx
	mov	cl,al
	add	ebx,ecx
	cmp	dl,0
	je	@f
	mov	dh,[esi]
	shr	dh,4
	mov	dl,[edi-1]
	and	dl,11110000b
	add	dl,dh
	mov	[edi-1],dl
	dec	ecx
	xor	edx,edx
	mov	dh,1
@@:
	test	cl,1b
	jz	@f
	inc	ecx
	inc	edx
@@:
	shr	ecx,1
	cmp	dh,1
	je	.1
	mov	eax,ecx
	cld
	rep	movsb
	test	eax,1b
	jz	@f
	inc	esi
@@:	
	jmp	.start
;---------------------------------------------------------------------
.1:
	mov	dh,cl
@@:
	cld
	lodsb
	mov	ah,[esi]
	shl	al,4
	shr	ah,4
	add	al,ah
	cld
	stosb
	dec	ecx
	jnz	@r
	test	dh,1b
	jz	@f
	inc	esi
@@:	
	xor	dh,dh
	jmp	.start
;---------------------------------------------------------------------
.Encoded_Mode:
	xor	ecx,ecx
	mov	cl,al
	add	ebx,ecx
	cld
	lodsb
	cmp	dl,0
	je	@f
	rol	al,4
	mov	dh,al
	and	dh,00001111b
	mov	dl,[edi-1]
	and	dl,11110000b
	add	dl,dh
	mov	[edi-1],dl
	dec	ecx
	xor	edx,edx
@@:
	test	cl,1b
	jz	@f
	inc	ecx
	inc	edx
@@:
	shr	ecx,1
	cld
	rep	stosb
	cmp	dl,0
	je	@f
	mov	al,[edi-1]
	and	al,11110000b
	mov	[edi-1],al
@@:
	jmp	.start	
;---------------------------------------------------------------------
.Delta:
	xor	eax,eax
	cld
	lodsb
	add	ebx,eax
	cmp	dl,0
	je	@f
	dec	eax
	xor	edx,edx
@@:
	test	al,1b
	jz	@f
	inc	eax
	inc	edx
@@:	
	shr	eax,1
	add	edi,eax
	cld
	lodsb
	imul	eax,ebp
	sub	edi,eax
	jmp	.start
;---------------------------------------------------------------------
.end_line:
	mov	eax,ebp
	test	ebx,1b
	jz	@f
	inc	ebx
@@:
	shr	ebx,1
	sub	eax,ebx
	add	edi,eax
	sub	edi,[delta]
	xor	ebx,ebx
	xor	edx,edx
	jmp	.start
;---------------------------------------------------------------------
RLE8:
	cmp	[resolution],8
	jne	START.no_bmp_file
	xor	ebx,ebx
align	4
.start:
	cld
	lodsb
	cmp	al,0
	jnz	.Encoded_Mode
	cld
	lodsb
	cmp	al,0
	jz	.end_line
	cmp	al,1
	jz	START.ret_ok	; .end_bitmap
	cmp	al,2
	jz	.Delta
	
.Absolute_Mode:
	xor	ecx,ecx
	mov	cl,al
	add	ebx,ecx
	mov	eax,ecx
	cld
	rep	movsb
	test	eax,1b
	jz	@f
	inc	esi
@@:	
	jmp	.start
;---------------------------------------------------------------------
.Encoded_Mode:
	xor	ecx,ecx
	mov	cl,al
	add	ebx,ecx
	cld
	lodsb
	rep	stosb
	jmp	.start
;---------------------------------------------------------------------
.Delta:
	xor	eax,eax
	cld
	lodsb
	add	edi,eax
	add	ebx,eax
	cld
	lodsb
	imul	eax,ebp
	sub	edi,eax
	jmp	.start
;---------------------------------------------------------------------
.end_line:
	mov	eax,ebp
	sub	eax,ebx
	add	edi,eax
	xor	ebx,ebx
	sub	edi,[delta]
	jmp	.start
;---------------------------------------------------------------------
Check_Header:
	pushad
	mov	eax,dword [esp+36]
	call	START.check_header
	jne	START.no_bmp_file
	popad
	ret	4
;---------------------------------------------------------------------
Associations:
	dd Associations.end - Associations
	db 'BMP',0
.end:
	db 0
;---------------------------------------------------------------------
align	16
EXPORTS:
	dd szStart, START
	dd szVersion, 0x00010002
	dd szCheck, Check_Header
	dd szAssoc, Associations
	dd 0

pointer	dd 0
image_file	dd 0
delta		dd 0
aligner	dd 0
resolution	dd 0
compression	dd 0
raw_area	dd 0
MinusHeight	db 0
	
szStart	db 'START',0
szVersion	db 'version',0
szCheck	db 'Check_Header',0
szAssoc	db 'Associations',0
;---------------------------------------------------------------------