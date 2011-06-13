;*****************************************************************************
; RAW to RAW convert plugin - for zSea image viewer
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

; Convert:
; 16b in 8b
; 1b,2b,3b in 8b

format MS COFF

public EXPORTS

section '.flat' code readable align 16

;include    'macros.inc'
include    '../../../../macros.inc'
;---------------------------------------------------------------------
START:
	pushad
	mov	eax,dword [esp+36]
	mov [pointer],eax
	mov eax,[eax+4]
	mov [image_file],eax
	mov  esi,[eax+28]
	add  esi,eax
	mov  edi,esi
	mov  ecx,[eax+32]
;	xor  ebx,ebx
;	mov  [raw_area],ebx

;	mov  ebx,[pointer]
;	movzx  eax,word [eax+18]
;	mov  [ebx+24],eax
;	jmp  .ret_ok
	
	cmp [eax+16],word 16
	je  .16b
	cmp [eax+12],dword 1
	je  .1b
	cmp [eax+12],dword 2
	je  .2b
	cmp [eax+12],dword 4
	je  .4b
;---------------------------------------------------------------------
.ret_ok:
;	mov ebx,[pointer]	
;	mov  eax,[raw_area]
;	mov  [ebx+20],eax  ; store RAW pointer
;	mov  [ebx+24],ecx

	mov  ebx,[image_file]
	cmp  [ebx+18],word 2
	jne  @f
	mov  eax,[ebx+12]
	shr  eax,1
	mov  [ebx+12],eax
@@:
	
	popad
	ret	4
;---------------------------------------------------------------------	
.less_8b:
	mov edx,[image_file]
	mov ecx,[area_size]
	add ecx,[edx+28]
	mcall 68,20
	mov  [image_file],eax

	
	mov ecx,[area_size]
	mov  eax,ecx
	shr  ecx,2
	test eax,3
	jz  @f
	inc  ecx
@@:
	mov  esi,[raw_area]
	mov  edi,[image_file]
	add  edi,[edi+28]
	cld  
	rep movsd
		
	mov  ecx,[raw_area]
	mcall 68,13
	mov eax,[image_file]
	mov ebx,[pointer]
	mov [ebx+4],eax
	popad
	ret	4
;---------------------------------------------------------------------	
.16b:
	cmp  [eax+18],word 3
	je   @f   ;.convert_16_in_8
	cmp  [eax+18],word 4
	jne  .16b_1
@@:
	xor  ebx,ebx
	mov  bx,[eax+18]
	
	xchg  eax,ecx
	xor  edx,edx
	div  ebx
	xchg  ecx,eax
	
	shr  ecx,1
	
	mov  [eax+16],word 8
	mov  ebx,[eax+12]
	shr  ebx,1
	mov  [eax+12],ebx

	mov  ebx,eax
;	jmp  .ret_ok
	
.convert_16_in_8:   ; converting 16 bit sample to 8 bit
	cld
	lodsw
	mov al,ah
	stosb
	
	lodsw
	mov al,ah
	stosb
	
	lodsw
	mov al,ah
	stosb
	
	cmp [ebx+18],word 4
	jne @f
	lodsw
	mov al,ah
	stosb
@@:   
	dec ecx
	jnz .convert_16_in_8 
	jmp .16b_end
;---------------------------------------------------------------------
.16b_1:
	cmp  [eax+18],word 1
	je   @f  ;.convert_16_in_8_1
	cmp  [eax+18],word 2
	jne  .16b_end
@@:
	shr  ecx,1
	
	mov  [eax+16],word 8
	mov  ebx,[eax+12]
	shr  ebx,1
	mov  [eax+12],ebx
	
.convert_16_in_8_1:
	cld
	lodsw
;	shr ax,8
;	mov al,ah
	stosb
	dec ecx
	jnz .convert_16_in_8_1
;---------------------------------------------------------------------
.16b_end:
	xor  eax,eax
	mov  [raw_area],eax
	jmp .ret_ok
;---------------------------------------------------------------------	
.4b:
	call .get_memory
	mov  edx,ebx
	inc  ebx
	shr  ebx,1
.4b_1:
	push ebx edi
@@:
	cld
	lodsb
	shl  eax,8
	mov  al,ah
	and  ah,0xf
	shr  al,4
	stosw
	
	dec  ebx
	jnz  @b
	pop  edi ebx
	add  edi,edx
	dec  ecx
	jnz  .4b_1
	
	jmp .less_8b  ;.ret_ok
;---------------------------------------------------------------------
.2b:
	call .get_memory
;	jmp .ret_ok
;	shr  ecx,1
	mov  edx,ebx
	mov  eax,ebx
	shr  ebx,2
	test eax,3
	jz  @f
	inc  ebx
@@:
	mov  ebp,ebx
.2b_1:
	push ebp edi
@@:
	cld
	lodsb
	
	mov  bl,al
	
	and  al,11b
	shl  ax,8
	
	mov  al,bl
	shr  al,2
	and  al,11b
	shl  eax,8
	
	mov  al,bl
	shr  al,4
	and  al,11b
	shl  eax,8
	
	mov  al,bl
	shr  al,6
	and  al,11b
	
	stosd

	dec  ebp
	jnz  @b
	pop  edi ebp
	
	add  edi,edx
	dec  ecx
	jnz  .2b_1
	
	jmp .less_8b  ;.ret_ok
;---------------------------------------------------------------------
.1b:
	call .get_memory
	mov  edx,ebx
	mov  eax,ebx
	shr  ebx,3
	test eax,7
	jz  @f
	inc  ebx
@@:
	mov  ebp,ebx
.1b_1:
	push ebp edi
@@:
	cld
	lodsb
	
	mov  bl,al
	shr  al,4
	and  al,1b
	shl  ax,8
	
	mov  al,bl
	shr  al,5
	and  al,1b
	shl  eax,8
	
	mov  al,bl
	shr  al,6
	and  al,1b
	shl  eax,8
	
	mov  al,bl
	shr  al,7
;	and  al,1b
;	shl  eax,8
	
	stosd
	
	mov  al,bl
	and  al,1b
	shl  ax,8
	
	mov  al,bl
	shr  al,1
	and  al,1b
	shl  eax,8

	mov  al,bl
	shr  al,2
	and  al,1b
	shl  eax,8
	
	mov  al,bl
	shr  al,3
	and  al,1b
	
	stosd
	
	dec  ebp
	jnz  @b
	pop  edi ebp
	
	add  edi,edx
	dec  ecx
	jnz  .1b_1
	jmp .less_8b  ;.ret_ok	
;---------------------------------------------------------------------
.get_memory:
	mov  ebx,dword 8
	mov  [eax+16],bx
	mov  [eax+12],ebx
;	mov  esi,[eax+28]
;	add  esi,eax
;	push ecx
	mov  ecx,[eax+4]
	imul ecx,[eax+8]
	push eax
	mov  [area_size],ecx
	mcall 68,12
;	pop  ecx
	mov  [raw_area],eax
	mov  edi,eax
	pop  eax
	mov  ebx,[eax+4]
	mov  ecx,[eax+8]
	ret
;---------------------------------------------------------------------
Convert24b:
	pushad
	mov	eax,dword [esp+36]
	mov [pointer],eax
	mov eax,[eax+4]
	mov [image_file],eax

	mov  esi,[eax+28]
	add  esi,eax

	mov  ebp,[eax+20]
	add  ebp,eax
	
	mov  ecx,[eax+4]
	imul ecx,[eax+8]
	push eax ecx
	lea  ecx,[ecx*3]
	mcall 68,12
	mov  [raw_area],eax
	mov  edi,eax
	pop  ecx eax

	cmp [eax+12],dword 32
	je  .32b
	cmp [eax+12],dword 16
	je  .16b
	cmp [eax+12],dword 15
	je  .15b
	cmp [eax+12],dword 8
	je  .8b
	
.ret_ok:
	mov ebx,[pointer]
	mov  eax,[raw_area]
	mov  [ebx+20],eax  ; store RAW pointer
	popad
	ret	4

;---------------------------------------------------------------------
.32b:
	cld
	lodsd
	
	stosw
	shr eax,16
	stosb
	
	dec  ecx
	jnz  .32b
	
	jmp  .ret_ok
;---------------------------------------------------------------------
.16b:
	cld
	lodsw
	
	xor ebx,ebx
	ror ax,11
	mov bl,al
	and bl,11111b
	shl bl,3
	shl ebx,8
	rol ax,6
	mov bl,al
	and bl,111111b
	shl bl,2
	shl ebx,8
	rol ax,5
	mov bl,al
	and bl,11111b
	shl bl,3
	mov eax,ebx
	
	cld
	stosw
	shr eax,16
	stosb
	
	dec  ecx
	jnz  .16b
	
	jmp  .ret_ok
;---------------------------------------------------------------------
.15b:
	cld
	lodsw
	
	xor ebx,ebx
	ror ax,10
	mov bl,al
	and bl,11111b
	shl bl,3
	shl ebx,8
	rol ax,5
	mov bl,al
	and bl,11111b
	shl bl,3
	shl ebx,8
	rol ax,5
	mov bl,al
	and bl,11111b
	shl bl,3
	mov eax,ebx
	
	cld
	stosw
	shr eax,16
	stosb
	
	dec  ecx
	jnz  .15b
	
	jmp  .ret_ok
;---------------------------------------------------------------------
.8b:
	xor  eax,eax
	cld
	lodsb
	shl  eax,2
	mov  eax,[eax+ebp]
	
	cld
	stosw
	shr eax,16
	stosb
	
	dec  ecx
	jnz  .8b
	
	jmp  .ret_ok
;---------------------------------------------------------------------
align 16
EXPORTS:
	dd      szStart,	START
	dd      szVersion,	0x00010002
	dd	szConv_24b,	Convert24b
	dd      0

szStart		db 'START',0
szVersion	db 'version',0
szConv_24b	db 'Convert24b',0
pointer		dd 0
image_file	dd 0
;delta		dd 0
;resolution	dd 0
;compression	dd 0
raw_area	dd 0
area_size	dd 0