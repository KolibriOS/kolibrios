;*****************************************************************************
; Rotate RAW image plugin - for zSea image viewer
; Copyright (c) 2009 - 2011, Marat Zakiyanov aka Mario79, aka Mario
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
; Rotate 32b, 24b, 16b, 8b

format MS COFF

public EXPORTS

section '.flat' code readable align 16

;include	'macros.inc'
include	'../../../../macros.inc'
;---------------------------------------------------------------------
START:
	pushad
	mov	ebx,dword [esp+40]
	mov	eax,dword [esp+36]
	mov [pointer],eax
; ebx - direction
; 1 - clockwise, 2 - counter clockwise
; 3 - Left&Right, 4 - Up&Down
	mov [direction],ebx
	mov eax,[eax+4]
	mov [image_file],eax

    mov  esi,[eax+28]
    add  esi,eax
;	mov  ecx,[eax+32]
;	xor  ebx,ebx
;	mov  [raw_area],ebx
	mov ebx,[eax+12]
	cmp  ebx,32
	jne  @f
	mov  ebp,dword START.32
	jmp  .1
@@:
	cmp  ebx,24
	jne  @f
	mov  ebp,dword START.24
	jmp  .1
@@:
	cmp  ebx,16
	jne  @f
	mov  ebp,dword START.16
	jmp  .1
@@:
	cmp  ebx,15
	jne  @f
	inc  ebx
	mov  ebp,dword START.16
	jmp  .1
@@:
	cmp  ebx,8
	jne  @f
	mov  ebp,dword START.8
@@:
.1:
	shr  ebx,3
	mov  [bytes_to_pixel],ebx
	mov  ebx,[eax+4]
	imul ebx,[bytes_to_pixel]
	mov  [size_x],ebx
	mov  ebx,[eax+8]
	imul ebx,[bytes_to_pixel]
	mov  [size_y],ebx

	call .get_memory
	
	cmp  [direction],1
	jne  @f
	call .clockwise
	jmp .end
@@:
	cmp  [direction],2
	jne  @f
	call .counter_clockwise
	jmp .end
@@:
	cmp  [direction],3
	jne  @f
	call .Left_Right
	jmp .end
@@:
	cmp  [direction],4
	jne  .exit
	call .Up_Down
.end:
	xchg esi,edi
	mov ecx,[image_file]
	mov eax,[ecx+4]
	imul eax,[bytes_to_pixel]
	imul eax,[ecx+8]
	
	mov ecx,eax
	cld
	rep movsb
	
	mov   ecx,[raw_area]
	mcall 68,13
;---------------------------------------------------------------------
.ret_ok:
	cmp  [direction],1
	jne  @f
	call  .XY_data_exchange
	jmp  .exit
@@:
	cmp  [direction],2
	jne  .exit
	call  .XY_data_exchange
.exit:
	popad
	ret	8
;---------------------------------------------------------------------
.XY_data_exchange:
	mov ecx,[image_file]
	mov eax,[ecx+4]
	mov ebx,[ecx+8]
	mov [ecx+8],eax
	mov [ecx+4],ebx
	ret
;---------------------------------------------------------------------
.clockwise:
	push edi esi
	
	add  edi,[size_y]
	sub  edi,[bytes_to_pixel]
.y:
	push edi
	push ebx
.x:
	call ebp
	add  edi,[size_y]
	dec  ebx
	jnz  .x
	
	pop ebx
	pop  edi

	sub  edi,[bytes_to_pixel]
	dec ecx
	jnz .y
	
	pop esi edi
	ret
;---------------------------------------------------------------------
.counter_clockwise:
	push edi esi
	
	mov  eax,[eax+4]
	dec  eax
	imul eax,[size_y]
	add  edi,eax
.y1:
	push edi
	push ebx
.x1:
	call ebp
	sub  edi,[size_y]
	dec  ebx
	jnz  .x1
	
	pop  ebx
	pop  edi

	add  edi,[bytes_to_pixel]
	dec  ecx
	jnz .y1
	
	pop  esi edi
	ret
;---------------------------------------------------------------------
.Left_Right:
	push edi esi
	add  edi,[size_x]
.y2:
	push edi
	push ebx
.x2:
	sub  edi,[bytes_to_pixel]
	call ebp
	dec  ebx
	jnz  .x2
	
	pop  ebx
	pop  edi

	add  edi,[size_x]
	dec  ecx
	jnz .y2
	
	pop  esi edi
	ret
;---------------------------------------------------------------------
.Up_Down:
	push edi esi
	
	mov  eax,[eax+8]
	dec  eax
	imul eax,[size_x]
	add  edi,eax
.y3:
	push edi
	push ebx
.x3:
	call ebp
	add  edi,[bytes_to_pixel]
	dec  ebx
	jnz  .x3
	
	pop  ebx
	pop  edi

	sub  edi,[size_x]
	dec  ecx
	jnz .y3
	
	pop  esi edi
	ret
;---------------------------------------------------------------------
.32:
	cld 
	lodsd
	mov  [edi],eax

	ret
;---------------------------------------------------------------------
.24:
	cld 
	lodsw
	mov  [edi],ax
	lodsb
	mov  [edi+2],al
	ret
;---------------------------------------------------------------------
.16:
	cld 
	lodsw
	mov  [edi],ax
	ret
;---------------------------------------------------------------------
.8:
	cld 
	lodsb
	mov  [edi],al
	ret
;---------------------------------------------------------------------
.get_memory:
	mov  ecx,[eax+4]
	imul ecx,[eax+8]
	imul ecx,[bytes_to_pixel]
	push eax
	mcall 68,12
	mov  [raw_area],eax
	mov  edi,eax
	pop  eax
	mov  ebx,[eax+4]
	mov  ecx,[eax+8]
	ret
;---------------------------------------------------------------------
align 16
EXPORTS:
	dd      szStart,	START
	dd      szVersion,	0x00010001
	dd      0

szStart		db 'START',0
szVersion	db 'version',0

pointer		dd 0
image_file	dd 0
direction	dd 0
size_x		dd 0
size_y		dd 0
bytes_to_pixel dd 0
;delta		dd 0
;resolution	dd 0
;compression	dd 0
raw_area	dd 0