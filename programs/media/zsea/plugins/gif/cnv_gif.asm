;*****************************************************************************
; GIF to RAW1 convert plugin - for zSea image viewer
; Copyright (c) 2009, Evgeny Grechnikov aka Diamond
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
; THIS SOFTWARE IS PROVIDED BY Evgeny Grechnikov ''AS IS'' AND ANY
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
; Based on gif_lite.inc (c) Ivuskin Andrey aka Willow and Diamond 2004-2007
;*****************************************************************************
; Some small changes (c) 2011 Marat Zakiyanov aka Mario79, aka Mario
;*****************************************************************************

format MS COFF

public EXPORTS

section '.flat' code readable align 16

START:
	pushad
	mov	eax,dword [esp+36]
	mov	esi, [eax]	; esi -> GIF data
	mov	ebp, [eax+12]	; ebp = file size
	xor	ebx, ebx	; ebx -> list of images, not allocated yet
	call	check_header_1
	jz	ReadGIF
ReadGIF.end:
; general exit from the function
	xor	eax, eax
        cmp     ebx, eax
        jz      .bad
        cmp     dword [ebx+4], eax
        jnz     ReadGIF.ret
        mov     ecx, ebx
        push    68
        pop     eax
        push    13
        pop     ebx
        int     40h
        xor     ebx, ebx
.bad:
	inc	eax	; bad image
ReadGIF.ret:
	mov	ecx, [esp+28]
	mov	[ecx+4], ebx	; save RAW data ptr
	mov	[ecx+8], eax	; save result
	popad
	ret	4
ReadGIF.animated.ret:
        mov     ebx, [ReadGIF.gifList]
        jmp     ReadGIF.ret
_null fix 0x1000
ReadGIF:
; allocate one page for list of images
        mov     ecx, 0x1000
        push    68
        pop     eax
        push    12
        pop     ebx
        int     40h
        xchg    eax, ebx
        test    ebx, ebx
        jnz     @f
        mov     al, 2   ; no memory
        jmp     .ret
@@:
    mov  dword[ebx],'RAW1'
    xor  eax,eax
    mov  [.globalColor],eax
    mov  [.globalColorSize],eax
    mov  [.curImageIndex],eax
    sub  ebp,0xd
    jb   .end
    movzx eax,word[esi+6]
    mov  [ebx+8],eax
    movzx eax,word[esi+8]
    mov  [ebx+12],eax
    mov  cl,[esi+0xa]
    add  esi,0xd
    test cl,cl
    jns  .nextblock
    mov  [.globalColor],esi
    push ebx
    call .Gif_skipmap
    mov  [.globalColorSize],ebx
    pop  ebx
    jb   .end
  .nextblock:
    dec  ebp
    js   .end
    cmp  byte[esi],0x21
    jne  .noextblock
    inc  esi
    cmp  byte[esi],0xf9 ; Graphic Control Ext
    jne  .no_gc
    sub  ebp,7
    jc   .end
    mov  ecx,[ebx+4]
    shl  ecx,4
    add  ecx,ebx
    mov  eax,[esi+3]
    mov  [ecx+16+12],ax
;    test byte[esi+2],1
;    setnz byte[ecx+16+14]
	mov al,[esi+2]
	mov [ecx+16+14], al
	
    mov  al,[esi+5]
    mov  [ecx+16+15],al
    add  esi,7
    jmp  .nextblock
  .no_gc:
    inc  esi
	xor eax,eax
  .block_skip:
    dec  ebp
    js   .end
    lodsb
    add  esi,eax
    sub  ebp,eax
    jc   .end2
    test eax,eax
    jnz  .block_skip
    jmp  .nextblock
  .noextblock:
    cmp  byte[esi],0x2c    ; image beginning
    jne  .end
    inc  esi
    sub  ebp,11
    jc   .end2
    movzx ecx,word[esi+4]  ; ecx = width
    jecxz .end2
    mov  [.width],ecx
    movzx eax,word[esi+6]  ; eax = height
    test eax,eax
    jz   .end2
    push eax ecx
    imul ecx,eax
    cmp  ecx,4000000h
    jb   @f
    pop  ecx eax
.end2:
    jmp  .end
@@:
    push ebx
    push ecx
    add  ecx,44+256*4
    push 68
    pop  eax
    push 12
    pop  ebx
    int  0x40
    pop  ecx
    pop  ebx
    test eax,eax
    jnz  @f
    pop  ecx ecx
    jmp  .end2
@@:
    xchg eax,edi
    inc  dword[ebx+4]
    mov  [edi+32],ecx      ; size of pixels area
    mov  byte[edi+20],44   ; pointer to palette
    mov  byte[edi+24+1],4  ; size of palette=256*4
    mov  dword[edi+28],44+256*4 ; pointer to RAW data
    pop  ecx eax
    mov  dword[edi], 'RAW ' ; signature
    mov  dword[edi+4],ecx  ; width
    mov  dword[edi+8],eax  ; height
    mov  byte[edi+12],8    ; total pixel size
    mov  byte[edi+16],8    ; 8 bits per component
    mov  byte[edi+18],1    ; number of components
    mov  eax,[ebx+4]
    shl  eax,4
    add  eax,ebx
    mov  [eax],edi
    movzx ecx,word[esi]
    mov  [eax+4],ecx
    movzx ecx,word[esi+2]
    mov  [eax+8],ecx
    mov  eax,[edi+32]
    mov  [.img_end],eax
    inc  eax
    mov  [.row_end],eax
    and  [.pass],0
    test byte[esi+8],40h
    jz   @f
    mov  ecx,[edi+4]
    mov  [.row_end],ecx
@@:
    mov  cl,[esi+8]
    add  esi,9
    add  edi,44
    push edi
    test cl,cl
    js   .uselocal
    push esi
    mov  esi,[.globalColor]
    mov  ecx,[.globalColorSize]
    call .swap_palette
    pop  esi
    jmp  .setPal
  .uselocal:
    push ebx
    call .Gif_skipmap
    jnc  @f
    pop  ebx
    pop  edi
    jmp  .end
@@:
    sub  esi,ebx
    mov  ecx,ebx
    pop  ebx
    call .swap_palette
  .setPal:
    movzx ecx,byte[esi]
    inc  ecx
    mov  [.codesize],ecx
    dec  ecx
    inc  esi
    mov  edi,.gif_workarea
    xor  eax,eax
    lodsb               ; eax - block_count
    add  eax,esi
    mov  [.block_ofs],eax
    mov  [.bit_count],8
    mov  eax,1
    shl  eax,cl
    mov  [.CC],eax
    mov  ecx,eax
    inc  eax
    mov  [.EOI],eax
    mov  eax, _null shl 16
  .filltable:
    stosd
    inc  eax
    loop .filltable
    pop  edi
    add  edi,256*4
    mov  [.img_start],edi
    add  [.img_end],edi
    add  [.row_end],edi
    mov  [.ebx],ebx
  .reinit:
    mov  edx,[.EOI]
    inc  edx
    push [.codesize]
    pop  [.compsize]
    call .Gif_get_sym
    cmp  eax,[.CC]
    je   .reinit
    call .Gif_output
  .cycle:
    movzx ebx,ax
    call .Gif_get_sym
    cmp  eax,edx
    jae  .notintable
    cmp  eax,[.CC]
    je   .reinit
    cmp  eax,[.EOI]
    je   .unpend
    call .Gif_output
  .add:
    mov  dword [.gif_workarea+edx*4],ebx
    cmp  edx,0xFFF
    jae  .cycle
    inc  edx
    bsr  ebx,edx
    cmp  ebx,[.compsize]
    jne  .noinc
    inc  [.compsize]
  .noinc:
    jmp  .cycle
  .notintable:
    push eax
    mov  eax,ebx
    call .Gif_output
    push ebx
    movzx eax,bx
    call .Gif_output
    pop  ebx eax
    jmp  .add
.unpend:
    mov  ebx,[.ebx]
    add  ebp,esi
    mov  esi,[.block_ofs]
    sub  ebp,esi
    jc   .end2
    xor  eax,eax
@@:
    dec  ebp
    js   .end2
    lodsb
    test eax,eax
    jz   @f
    sub  ebp,eax
    jc   .end2
    add  esi,eax
    jmp  @b
@@:
    test ebp,ebp
    jz   .end2
    cmp  byte[esi],0x3b
    jz   .end2
; next image
    mov  ecx,[ebx+4]
    cmp  cl,0xFF
    jnz  .noresize
    mov  edx,ebx
    inc  ecx
    inc  ecx
    shl  ecx,4
    push 68
    pop  eax
    push 20
    pop  ebx
    int  40h
    test eax,eax
    jnz  @f
    mov  ebx,edx
    jmp  .end2
 @@:
    xchg ebx,eax
 .noresize:
    jmp  .nextblock

.Gif_skipmap:
; in: ecx - image descriptor, esi - pointer to colormap
; out: edi - pointer to area after colormap

    and  ecx,111b	; color map size
    mov  ebx,3*2
    shl  ebx,cl
    add  esi,ebx
    sub  ebp,ebx
    ret

.Gif_get_sym:
    mov  ecx,[.compsize]
    push ecx
    xor  eax,eax
  .shift:
    ror  byte[esi],1
    rcr  eax,1
    dec  [.bit_count]
    jnz  .loop1
    inc  esi
    cmp  esi,[.block_ofs]
    jb   .noblock
    push eax
    xor  eax,eax
    dec  ebp
    js   .dataend
    lodsb
    test eax,eax
    jnz  .nextbl
    mov  eax,[.EOI]
    sub  esi,2
    add  esp,8
    jmp  .exx
  .nextbl:
    add  eax,esi
    mov  [.block_ofs],eax
    pop  eax
  .noblock:
    mov  [.bit_count],8
  .loop1:
    loop .shift
    pop  ecx
    rol  eax,cl
  .exx:
    xor  ecx,ecx
    ret

.dataend:
	pop	eax eax
	mov	ebx, [.ebx]
	jmp	.end2

.Gif_output:
    push esi eax edx
    mov  edx,.gif_workarea
  .next:
    push word[edx+eax*4]
    mov  ax,word[edx+eax*4+2]
    inc  ecx
    cmp  ax,_null
    jnz  .next
    shl  ebx,16
    mov  bx,[esp]
  .loop2:
    pop  ax

        stosb

    cmp  edi,[.row_end]
    jb   .norowend
    mov  eax,[.width]
    push eax
    sub  edi,eax
    add  eax,eax
    cmp  [.pass],3
    jz   @f
    add  eax,eax
    cmp  [.pass],2
    jz   @f
    add  eax,eax
@@:
    add  edi,eax
    pop  eax
    cmp  edi,[.img_end]
    jb   .nextrow
    mov  edi,[.img_start]
    inc  [.pass]
    add  edi,eax
    cmp  [.pass],3
    jz   @f
    add  edi,eax
    cmp  [.pass],2
    jz   @f
    add  edi,eax
    add  edi,eax
@@:
.nextrow:
    add  eax,edi
    mov  [.row_end],eax
    xor  eax,eax
.norowend:

    loop .loop2
    pop  edx eax esi
    ret

.swap_palette:
    xor  eax,eax
@@:
    lodsb
    mov  ah,al
    lodsb
    shl  eax,8
    lodsb
    stosd
    sub  ecx,3
    jnz  @b
    ret
;---------------------------------------------------------------------
check_header_1:
	and	dword [eax+8], 0
	cmp	dword [eax+12], 6
	jb	.err
	push	eax
	mov	eax, [eax]
	cmp	dword [eax], 'GIF8'
	jnz	.errpop
	cmp	byte [eax+5], 'a'
	jnz	.errpop
	cmp	byte [eax+4], '7'
	jz	@f
	cmp	byte [eax+4], '9'
	jnz	.errpop
@@:
	pop	eax
	ret
.errpop:
	pop	eax
.err:
	inc	dword [eax+8]
	ret
;---------------------------------------------------------------------
check_header:
	pushad
	mov	eax,dword [esp+36]
	call	check_header_1
	popad
	ret	4
;---------------------------------------------------------------------
Associations:
dd  Associations.end - Associations
db 'GIF',0
.end:
db 0
;---------------------------------------------------------------------
align 4
EXPORTS:
	dd szStart, START
	dd szVersion, 0x00010002
	dd szCheck, check_header
	dd szAssoc, Associations
	dd 0

szStart		db 'START',0
szVersion	db 'version',0
szCheck		db 'Check_Header',0
szAssoc		db 'Associations',0

section '.data' data readable writable align 16
    ReadGIF.globalColor rd 1
    ReadGIF.globalColorSize rd 1
    ReadGIF.cur_info rd 1        ; image table pointer
    ReadGIF.codesize rd 1
    ReadGIF.compsize rd 1
    ReadGIF.bit_count rd 1
    ReadGIF.CC rd 1
    ReadGIF.EOI rd 1
    ReadGIF.block_ofs rd 1
    ReadGIF.row_end rd 1
    ReadGIF.img_end rd 1
    ReadGIF.img_start rd 1
    ReadGIF.pass rd 1
    ReadGIF.width rd 1
    ReadGIF.ebx rd 1
    ReadGIF.gifList rd 1
    ReadGIF.curImageIndex rd 1
    ReadGIF.gif_workarea rb 16*1024
