;*****************************************************************************
; Scaling RAW image plugin - for zSea image viewer
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
; Scaling 32b, 24b, 16b, 8b

format MS COFF

public EXPORTS

section '.flat' code readable align 16

;include	'macros.inc'
include	'../../../../macros.inc'
;---------------------------------------------------------------------
START:
	pushad
	mov [pointer],eax
	test bx,bx
	jnz  @f
	inc  bx
@@:
	ror ebx,16
	test bx,bx
	jnz  @f
	inc  bx
@@:
	rol ebx,16
	mov [new_size],ebx
	mov [start_coordinates],ecx
	mov [scaling_mode],edx
	mov [filtering],esi
	mov [background_color],edi
	mov eax,[eax+4]
	mov [image_file],eax

	mov  esi,[eax+28]
    add  esi,eax
	
	mov  ebx,[eax+20]
	add  ebx,eax
	mov  [palette],ebx
	
	mov ebx,[eax+12]
	mov [resolution],ebx
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
	
	mov  ebx,[eax+8]
	mov  [y],ebx
	mov  ebx,[eax+4]
	mov  [x],ebx
	imul ebx,[bytes_to_pixel]
	mov  [size_x],ebx
	
	mov  eax,100
	shl  eax,12
	mov  ebx,[scaling_mode]
	test ebx,ebx
	jnz  @f
	inc  ebx
@@:
	xor  edx,edx
	div  ebx
	mov  [scaling_delta],eax
	
	call .get_memory
	cmp  [scaling_mode],0
	jne  @f
	call .scaling
	jmp  .ret_ok
@@:
	call .scaling_2	
	
;---------------------------------------------------------------------
.ret_ok:
	mcall 68,13,[area_for_x]
	mov ebx,[pointer]
	mov  eax,[raw_area]
	mov  [ebx+20],eax  ; store RAW pointer
;	movzx eax,word [new_size.x1]
;	mov [ebx+24],esi  ;eax
;	movzx eax,word [new_size.y1]
;	mov [ebx+28],eax
;	mov eax,[size_x]
;	mov [ebx+32],eax
;	mov eax,[bytes_to_pixel]
;	mov [ebx+36],eax
;	mov eax,[x]
;	mov [ebx+40],eax
;	mov eax,[y]
;	mov [ebx+44],eax
.exit:
	popad
	ret
;---------------------------------------------------------------------
align 4
.scaling:
	xor ecx,ecx
.y:
	xor ebx,ebx
;-------------------------
.x:
	call ebp
	inc  ebx
	cmp  bx,[new_size.x1]
	jb   .x
;-------------------------
	inc  ecx
	cmp  cx,[new_size.y1]
	jb   .y
	ret
;---------------------------------------------------------------------
align 4
.scaling_2:
	xor  eax,eax
	mov  ax,[start_coordinates.y]
	imul eax,[size_x]
	add  esi,eax
	xor  eax,eax
	mov  ax,[start_coordinates.x]
	imul eax,[bytes_to_pixel]
	add  esi,eax
	
	xor  eax,eax
	dec  eax
	mov [temp_y],eax
	
	xor ecx,ecx
align 4
.y_2:
	xor ebx,ebx
;-------------------------
align 4
.x_2:
	call ebp
	inc  ebx
	cmp  bx,[new_size.x1]
	jb   .x_2
;-------------------------
	inc  ecx
	cmp  cx,[new_size.y1]
	jb   .y_2
	ret
;---------------------------------------------------------------------
align 4
.32:
	push ecx ebx
	call .calculate_pixel
	mov  eax,[ecx]
	call .check_filtering_32
	pop  ebx ecx
	cld 
	stosd
	ret
;---------------------------------------------------------------------
align 4
.24:
	push ecx ebx
	call .calculate_pixel
	mov  eax,[ecx]	
	call .check_filtering_24
	cld
	stosw
	shr  eax,16	
	pop  ebx ecx
	cld
	stosb
	ret
;---------------------------------------------------------------------
align 4
.16:
	push ecx ebx
	call .calculate_pixel
	xor  eax,eax
	mov  ax,[ecx]
	call .check_filtering_16
	pop  ebx ecx
	cld 
	stosw
	ret
;---------------------------------------------------------------------
align 4
.8:
	push ecx ebx
	call .calculate_pixel
	cmp  [filtering],0
	jne   @f
	mov  al,[ecx]
	pop  ebx ecx
	cld
	stosb
	ret
@@:
	call .check_filtering_8
	cld
	stosw
	shr  eax,16	
	pop  ebx ecx
	cld
	stosb
	ret	
	
;---------------------------------------------------------------------
align 4
.calculate_pixel:
	test ecx,ecx
	jz   .offset_x
;.offset_y:
	mov  eax,ecx
	
	mov  ecx,[temp_y]
	cmp  eax,ecx
	jne  .new_y
	mov  eax,[temp_y_offset]
	mov  ecx,eax
	jmp  .offset_x
;--------------------------------
align 4
.new_y:
	mov  [temp_y],eax

	mov  ebx,[scaling_mode]
	test ebx,ebx
	jz  @f
	imul eax,[scaling_delta]
;--------------------------------
	push ebx
	mov  ebx,eax
	shr  eax,12
	and  ebx,0xFFF
	shl  ebx,7  ;multiply 128
	shr  ebx,12
	mov  [next_pixel_y],ebx
	pop  ebx
;--------------------------------
	jmp  .ex_1
align 4
@@:
;--------------------------------
	imul eax,dword [y]
	mov bx,word [new_size.y1]
;--------------------------------
align 4
.y_div:
	test ebx,ebx
	jnz  @f
	inc  ebx
align 4
@@:
	xor  edx,edx
	div  ebx
;--------------------------------
	push eax
	mov  eax,edx
	shl  eax,7  ;multiply 128
	xor  edx,edx
	div  ebx
	mov  [next_pixel_y],eax
	pop  eax
;--------------------------------
align 4
.ex_1:
	mov  [temp_y1],eax
	imul eax,[size_x]

	mov  [temp_y_offset],eax
	mov  ecx,eax
align 4
.offset_x:
	test ebx,ebx
	jz   .finish
	mov  eax,[esp+4]   ;ebx
	
	mov edx,[esp+8]
	test edx,edx
	jz  .continue
	shl eax,3
	add eax,[area_for_x]
	mov edx,[eax+4]
	mov [next_pixel_x],edx
	mov eax,[eax]
	jmp .ex_3
;--------------------------------
align 4
.continue:
	mov  ebx,[scaling_mode]
	test ebx,ebx
	jz  @f
	imul eax,[scaling_delta]
;--------------------------------
	mov  ebx,eax
	shr  eax,12
	and  ebx,0xFFF
	shl  ebx,7  ;multiply 128
	shr  ebx,12
	mov  [next_pixel_x],ebx
;--------------------------------
	jmp  .ex_2
;--------------------------------
align 4
@@:
	imul eax,dword [x]
	mov  bx,word [new_size.x1]
;--------------------------------
align 4
.x_div:
	test ebx,ebx
	jnz  @f
	inc  ebx
align 4
@@:
	xor  edx,edx
	div  ebx
;--------------------------------
	push eax
	mov  eax,edx
	shl  eax,7  ;multiply 128
	xor  edx,edx
	div  ebx
	mov  [next_pixel_x],eax
	pop  eax
;--------------------------------
align 4
.ex_2:
	mov  edx,[bytes_to_pixel]
	mov  ebx,eax
	xor  eax,eax
align 4
@@:
	add eax,ebx
	dec  edx
	jnz   @r
	
	mov ebx,[esp+4]
	shl ebx,3
	add ebx,[area_for_x]
	mov [ebx],eax
	mov edx,[next_pixel_x]
	mov [ebx+4],edx
align 4
.ex_3:
	mov [temp_x1],eax
	add  ecx,eax
align 4
.finish:
	add  ecx,esi
	ret
;---------------------------------------------------------------------
align 4
.get_memory:

	xor  ecx,ecx
	mov  cx,[new_size.x1]
	shl  ecx,3
	mcall 68,12
	mov  [area_for_x],eax
	
	xor   ecx,ecx
	mov   ebx,[new_size]
	mov   cx,bx
	shr   ebx,16
	imul ecx,ebx  ;[eax+8]
	mov  eax,[bytes_to_pixel]
	cmp  eax,1
	jne  @f
	mov  eax,3
@@:
	imul ecx,eax
	mcall 68,12
	mov  [raw_area],eax
	mov  edi,eax
	ret
;---------------------------------------------------------------------
include 'b_filter.inc'
;---------------------------------------------------------------------
align 4
EXPORTS:
	dd      szStart,	START
	dd      szVersion,	0x00010001
	dd      0

szStart		db 'START',0
szVersion	db 'version',0

align 4
pointer		dd 0
image_file	dd 0
new_size:
.y1:			dw 0
.x1:			dw 0

x:  dd 0
y:  dd 0

size_x		dd 0
bytes_to_pixel dd 0

start_coordinates:
.y	dw 0
.x	dw 0

scaling_mode	dd 0
raw_area	dd 0
scaling_delta dd 0

area_for_x dd 0

temp_y dd 0
temp_y_offset dd 0

resolution dd 0

filtering dd 0

next_pixel_y dd 0
next_pixel_x dd 0

temp_y1 dd 0
temp_x1 dd 0

B_sample dd 0
G_sample dd 0
R_sample dd 0

B_sample_1 dd 0
G_sample_1 dd 0
R_sample_1 dd 0

palette dd 0

background_color dd 0