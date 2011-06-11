;*****************************************************************************
; PNG to RAW convert plugin - for zSea image viewer
; Copyright (c) 2008, 2009, Marat Zakiyanov aka Mario79, aka Mario
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
;---------------------------------------------------------------------
filtering_RGB_Interlaced:
	mov edx,dword filtering_RGB
	mov ebp,dword move_Interlaced_RGB
	jmp  filtering_All
;---------------------------------------------------------------------
filtering_RGB_16_Interlaced:
	mov edx,dword filtering_RGB_16
	mov ebp,dword move_Interlaced_RGB
	jmp  filtering_All
;---------------------------------------------------------------------
filtering_grayscale_Interlaced:
	mov edx,dword filtering_grayscale
	mov ebp,dword move_Interlaced_grayscale
	jmp  filtering_All
;---------------------------------------------------------------------
filtering_grayscale_16_Interlaced:
	mov edx,dword filtering_grayscale_16
	mov ebp,dword move_Interlaced_grayscale
	jmp  filtering_All
;---------------------------------------------------------------------
pass dd 0
filtering_All:
	push edi

	mov ecx,[resolution]
	imul ecx,[IHDR_data.Width]
	mov  eax,ecx
	shr  ecx,3
	test eax,7
	jz  @f
	inc  ecx
@@:
	mov  eax,[IHDR_data.Height]
	shr  eax,1
	inc  eax
	imul ecx,eax
	mcall 68, 12
	mov [Interlaced_area],eax
;	jmp .3
	mov  eax,dword calculate_Interlaced_counters
	mov  ecx,7
@@:
	mov  [pass],ecx
	mov [Interlaced_step],eax
	push ecx
	push eax edx edi ebp eax
	call dword [eax] ;calculate_Interlaced_1
;	cmp  [pass],1
;	jbe   .2
	call edx
;.2:
	pop  eax ebp edi
	push ebp
;	cmp  [pass],1
;	jbe   .1
	call ebp
;.1:
	pop  ebp edx eax
	add  eax,4
	pop  ecx
	dec  ecx
	jnz  @r
;.3:
	mov ecx,[Interlaced_area]
	mcall 68, 13
	
	pop edi
	ret
;---------------------------------------------------------------------	
move_Interlaced_RGB:
	push edi esi
	mov  esi,[Interlaced_area]
	
	mov  ecx,[IHDR_data.Height]
	test ecx,ecx
	jz	 .end
	
	mov edx,[IHDR_data.Width]
	test edx,edx
	jz	 .end
	
	mov  ebx,[Image_Width]
	imul ebx,[resolution]
	shr  ebx,3
	
	mov  eax,[Starting_Col]
	imul eax,[resolution]
	shr  eax,3
	add  edi,eax
	
	mov  eax,[Starting_Row]
	imul eax,ebx
	add  edi,eax
	
	imul ebx,[Row_Increment]
	
	mov  ebp,[resolution]
	imul ebp,[Col_Increment]
	shr  ebp,3
	
.filter:
	mov edx,[IHDR_data.Width]
	push edi
	
.filter_x:
    cmp   [IHDR_data.Bit_depth],byte 8
	jne   @f
	call .Interlaced_RGB_8
	jmp   .1
@@:
	call .Interlaced_RGB_16
.1:
	dec edx
	jnz .filter_x
	
	pop edi
	
	add edi,ebx
	dec ecx
	jnz .filter
.end:
	pop  esi edi
	ret
;-------------------------------------------
.Interlaced_RGB_8:
	cld
	lodsw
	mov [edi],ax
	lodsb
	mov [edi+2],al
	cmp [IHDR_data.Color_type],byte 6
	jne @f
	lodsb
	mov [edi+3],al
@@:
	add edi,ebp
	ret
;-------------------------------------------
.Interlaced_RGB_16:
	cld
	lodsd
	mov [edi],eax
	lodsw
	mov [edi+4],ax
	cmp [IHDR_data.Color_type],byte 6
	jne @f
	lodsw
	mov [edi+6],al
@@:
	add edi,ebp
	ret
;---------------------------------------------------------------------	
move_Interlaced_grayscale:
;	cmp  [eax],dword calculate_Interlaced_1
;	jne   @f
;	ret
;@@:
;	cmp  [eax],dword calculate_Interlaced_2
;	jne   @f
;	ret
;@@:
;	cmp  [eax],dword calculate_Interlaced_3
;	jne   @f
;	ret
;@@:
;	cmp  [eax],dword calculate_Interlaced_4
;	jne   @f
;	ret
;@@:
;	cmp  [eax],dword calculate_Interlaced_5
;	jne   @f
;	ret
;@@:
;	cmp  [eax],dword calculate_Interlaced_6
;	jne   @f
;	ret
;@@:
;	cmp  [eax],dword calculate_Interlaced_7
;	jne   @f
;	ret
;@@:

	push edi esi
	mov  esi,[Interlaced_area]
	
	mov  ecx,[IHDR_data.Height]
	test ecx,ecx
	jz	 .end
	
	mov edx,[IHDR_data.Width]
	test edx,edx
	jz	 .end
	
	mov  ebx,[Image_Width]
	imul ebx,[resolution]
	mov eax,ebx
	shr  ebx,3
	test eax,7
	jz  @f
	inc  ebx
@@: 
; ebx = Width in bytes, need correct for resolution less 4b
	
	mov  eax,[Starting_Col]
	imul eax,[resolution]
;	mov  ebp,eax
	shr  eax,3
;	test ebp,7
;	jz  @f
;	inc  eax
;@@:
; eax = Starting Colummn in bytes
	add  edi,eax
	
	mov  eax,[Starting_Row]
	imul eax,ebx
; eax = Starting Row in bytes, no need correct!
	add  edi,eax
	
	imul ebx,[Row_Increment]
; ebx = Row Increment in bytes, no need correct!
	
	mov  eax,[IHDR_data.Width]
	imul eax,[resolution]
	mov ebp,eax
	shr  eax,3
	test ebp,7
	jz  @f
	inc  eax
@@: 
; eax = Width in bytes for Interlaced,
;		need correct for resolution less 4b

	mov  ebp,[resolution]
	imul ebp,[Col_Increment]
;	mov  eax,ebp
	shr  ebp,3
;	test eax,7
;	jz  @f
;	inc  ebp
;@@:
; ebp = Column Increment in bytes

.filter_grayscale:
	mov edx,[IHDR_data.Width]
	cmp [resolution],8
	jae .filter_grayscale_1
	
.resolution_4:
	cmp [resolution],4
	jne .resolution_2
	test edx,1
	jnz  @f
	inc  edx
@@:
	shr edx,1
	jmp .filter_grayscale_1
	
.resolution_2:
	cmp [resolution],2
	jne .resolution_1
	test edx,3
	jnz  @f
	add  edx,4
@@:
	shr edx,2
	jmp .filter_grayscale_1
	
.resolution_1:
	push eax
	mov eax,edx
	shr edx,3
	test eax,7
	jz  @f
	inc  edx
@@:
	pop eax
.filter_grayscale_1:
	push eax edi esi

.filter_x_grayscale:
    cmp   [IHDR_data.Bit_depth],byte 16
	jne   @f
	call .Interlaced_grayscale_16
	jmp   .1
@@:
    cmp   [IHDR_data.Bit_depth],byte 8
	jne   @f
	call .Interlaced_grayscale_8
	jmp   .1
@@:
    cmp   [IHDR_data.Bit_depth],byte 4
	jne   @f
	call .Interlaced_grayscale_4
	jmp   .1
@@:
    cmp   [IHDR_data.Bit_depth],byte 2
	jne   @f
	call .Interlaced_grayscale_2
	jmp   .1
@@:
	call .Interlaced_grayscale_1
	jmp   .1
.1:

	dec edx
	jnz .filter_x_grayscale
	pop  esi edi eax
	add esi,eax
	
	add edi,ebx
	dec ecx
	jnz .filter_grayscale
.end:
	pop  esi edi
	ret
;-----------------------------------
.Interlaced_grayscale_1:
	mov  eax,[Interlaced_step]
	cmp eax,calculate_Interlaced_counters.1
	je  .step_1
	cmp eax,calculate_Interlaced_counters.2
	je  .step_2
	cmp eax,calculate_Interlaced_counters.3
	je  .step_3
	cmp eax,calculate_Interlaced_counters.4
	je  .step_4
	cmp eax,calculate_Interlaced_counters.5
	je  .step_5
	cmp eax,calculate_Interlaced_counters.6
	je  .step_6	
	cld
	movsb
	ret
;--------------------	
.step_6:
	push ebx
	cld
	lodsb
	mov ah,al
	
	call @f
	
	shl ah,4
	mov al,ah

	call @f
	
	pop ebx
	ret
;--------------------
@@:
	shr al,1
	and al,64
	
	mov bl,ah
	shr bl,2
	and bl,16
	add al,bl
	
	mov bl,ah
	shr bl,3
	and bl,4
	add al,bl

	mov bl,ah
	shr bl,4
	and bl,1
	add al,bl	
	
	add al,[edi]
	stosb
	ret
;--------------------	
.step_5:
	push ebx
	cld
	lodsb

	mov ah,al
	
	call @f
	
	shl ah,4	
	mov al,ah
	
	call @f

	pop ebx
	ret
;--------------------
@@:
	and al,128
	
	mov bl,ah
	shr bl,1
	and bl,32
	add al,bl
	
	mov bl,ah
	shr bl,2
	and bl,8
	add al,bl
	
	mov bl,ah
	shr bl,3
	and bl,2
	add al,bl
	
	add al,[edi]
	stosb
	ret
;--------------------	
.step_4:
	push ebx
	cld
	lodsb

	mov ah,al
	shr al,2
	and al,32
	mov bl,ah
	shr bl,5
	call @f
	
	mov al,ah
	and al,32
	mov bl,ah
	shr bl,3
	call @f
	
	mov al,ah
	shl al,2
	and al,32
	mov bl,ah
	shr bl,1
	call @f

	mov al,ah
	shl al,4
	and al,32
	mov bl,ah
	shl bl,1
	call @f
	pop ebx
	ret
;--------------------
@@:
	and bl,2
	add al,bl
	add al,[edi]
	stosb
	ret
;--------------------
.step_3:
	push ebx
	cld
	lodsb

	mov ah,al
	and al,128
	mov bl,ah
	shr bl,3
	call @f
	
	mov al,ah
	shl al,2
	and al,128
	mov bl,ah
	shr bl,1
	call @f
	
	mov al,ah
	shl al,4
	and al,128
	mov bl,ah
	shl bl,1
	call @f

	mov al,ah
	shl al,6
	and al,128
	mov bl,ah
	shl bl,3
	call @f
	pop ebx
	ret
;--------------------	
@@:
	and bl,8
	add al,bl
	add al,[edi]
	stosb
	ret
;--------------------	
.step_2:
	cld
	lodsb

	mov ah,al
	shr al,4
	and al,8
	add al,[edi]
	stosb
	
	mov al,ah
	shr al,3
	and al,8
	add al,[edi]
	stosb
	
	mov al,ah
	shr al,2
	and al,8
	add al,[edi]
	stosb
	
	mov al,ah
	shr al,1
	and al,8
	add al,[edi]
	stosb
	
	mov al,ah
	and al,8
	add al,[edi]
	stosb
	
	mov al,ah
	shl al,1
	and al,8
	add al,[edi]
	stosb
	
	mov al,ah
	shl al,2
	and al,8
	add al,[edi]
	stosb
	
	mov al,ah
	shl al,3
	and al,8
	add al,[edi]
	stosb
	
	ret
;--------------------	
.step_1:
	cld
	lodsb
	mov ah,al
	and al,128
	stosb
	mov al,ah
	shl al,1
	and al,128
	stosb
	mov al,ah
	shl al,2
	and al,128
	stosb
	mov al,ah
	shl al,3
	and al,128
	stosb
	mov al,ah
	shl al,4
	and al,128
	stosb
	mov al,ah
	shl al,5
	and al,128
	stosb
	mov al,ah
	shl al,6
	and al,128
	stosb
	mov al,ah
	shl al,7
	and al,128
	stosb
	ret
;-----------------------------------
.Interlaced_grayscale_2:
	mov  eax,[Interlaced_step]
	cmp eax,calculate_Interlaced_counters.1
	je  .step_1_1
	cmp eax,calculate_Interlaced_counters.2
	je  .step_2_1
	cmp eax,calculate_Interlaced_counters.3
	je  .step_3_1
	cmp eax,calculate_Interlaced_counters.4
	je  .step_4_1
	cmp eax,calculate_Interlaced_counters.5
	je  .step_5_1
	cmp eax,calculate_Interlaced_counters.6
	je  .step_6_1
	cld
	movsb
	ret
	
.step_6_1:
	push ebx
	cld
	lodsb
	mov ah,al
	
	shr al,2
	and al,48
	mov bl,ah
	shr bl,4
	and bl,3
	add al,bl
	add al,[edi]
	stosb
	
	mov al,ah
	shl al,2
	and al,48
	mov bl,ah
	and bl,3
	add al,bl
	add al,[edi]
	stosb
	pop ebx
	ret
	
.step_5_1:
	push ebx
	cld
	lodsb
	mov ah,al
	
	and al,192
	mov bl,ah
	shr bl,2
	and bl,12
	add al,bl
	add al,[edi]
	stosb
	
	mov al,ah
	shl al,4
	and al,192
	mov bl,ah
	shl bl,2
	and bl,12
	add al,bl
	add al,[edi]
	stosb
	pop ebx
	ret
	
.step_4_1:
	cld
	lodsb
	mov ah,al
	
	shr al,4
	and al,12
	add al,[edi]
	stosb

	mov al,ah
	shr al,2
	and al,12
	add al,[edi]
	stosb
	
	mov al,ah
	and al,12
	add al,[edi]
	stosb
	
	mov al,ah
	shl al,2
	and al,12
	add al,[edi]
	stosb
	
	ret
	
.step_3_1:
	cld
	lodsb
	mov ah,al
	
	and al,192
	stosb

	mov al,ah
	shl al,2
	and al,192
	stosb
	
	mov al,ah
	shl al,4
	and al,192
	stosb
	
	mov al,ah
	shl al,6
	and al,192
	stosb
	
	ret
	
.step_2_1:
	cld
	lodsb
	mov ah,al
	
	and al,192
	stosb
	inc edi
	
	mov al,ah
	shl al,2
	and al,192
	stosb
	inc edi
	
	mov al,ah
	shl al,4
	and al,192
	stosb
	inc edi
	
	mov al,ah
	shl al,6
	and al,192
	stosb
	inc edi
	
	ret
	
.step_1_1:
	cld
	lodsb

	mov ah,al
	and al,192
	stosb
	inc edi
	
	mov al,ah
	shl al,2
	and al,192
	stosb
	inc edi
	
	mov al,ah
	shl al,4
	and al,192
	stosb
	inc edi
	
	mov al,ah
	shl al,6
	and al,192
	stosb
	inc edi
	
	ret
;-----------------------------------
.Interlaced_grayscale_4:
	mov al,[esi]
	mov ah,[edi]
	test ebp,ebp
	jnz  @f
	call .even_2
	jmp  .2
@@:
	test [Starting_Col],byte 1
	jz  @f
	call .even_1
	jmp  .2
@@:
	call .odd_1
.2:
	add al,ah
	mov [edi],al
	add edi,ebp

	mov al,[esi]
	mov ah,[edi]
	test ebp,ebp
	jnz  @f
	call .odd_1
	jmp  .3
@@:
	test [Starting_Col],byte 1
	jz  @f
	call .even_2
	jmp  .3
@@:
	call .odd_2
.3:
	add al,ah
	mov [edi],al
	add edi,ebp
	
	inc  esi
	test ebp,ebp
	jnz  @f
	inc edi
@@:
	ret
;---------------------
.even_1:
	shr al,4
	and ah,11110000b
	ret
;---------------------
.even_2:
	and al,1111b
	and ah,11110000b
	ret
;---------------------
.odd_1:
	and al,11110000b
	and ah,1111b
	ret
;---------------------	
.odd_2:
	shl al,4
	and ah,1111b
	ret
;-----------------------------------
.Interlaced_grayscale_8:
	cld
	lodsb
	mov [edi],al
	cmp [IHDR_data.Color_type],byte 4
	jne @f
	cld
	lodsb
	mov [edi+1],al
@@:
	add edi,ebp
	ret
;-----------------------------------
.Interlaced_grayscale_16:
	cld
	lodsw
	mov [edi],ax
	cmp [IHDR_data.Color_type],byte 4
	jne @f
	cld
	lodsw
	mov [edi+2],ax
@@:
	add edi,ebp
	ret
;---------------------------------------------------------------------	
calculatte_Interlaced_data_1:
	mov [first_line],byte 1
	mov edi,[Interlaced_area]
	mov eax,[Image_Width]
	mov ebx,dword Starting_Row
	ret
;---------------------------------------------------------------------	
calculate_Interlaced_1:
	call calculatte_Interlaced_data_1
	mov [ebx+0],dword 0 ;Starting_Row
	mov [ebx+4],dword 0 ;Starting_Col
	mov [ebx+8],dword 8 ;Row_Increment
	mov [ebx+12],dword 8 ;Col_Increment
	mov ebx,eax
	shr eax,3
	test ebx,7
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Width],eax
	
	mov eax,[Image_Height]
	mov ebx,eax
	shr eax,3
	test ebx,7
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Height],eax
	ret
;---------------------------------------------------------------------
calculate_Interlaced_2:
	call calculatte_Interlaced_data_1
	mov [ebx+0],dword 0 ;Starting_Row
	mov [ebx+4],dword 4 ;Starting_Col
	mov [ebx+8],dword 8 ;Row_Increment
	mov [ebx+12],dword 8 ;Col_Increment
	cmp eax,4
	ja  @f
	mov eax,4
@@:
	sub eax,4
	mov ebx,eax
	shr eax,3
	test ebx,7
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Width],eax
	
	mov eax,[Image_Height]
	mov ebx,eax
	shr eax,3
	test ebx,7
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Height],eax
	ret
;---------------------------------------------------------------------
calculate_Interlaced_3:
	call calculatte_Interlaced_data_1
	mov [ebx+0],dword 4 ;Starting_Row
	mov [ebx+4],dword 0 ;Starting_Col
	mov [ebx+8],dword 8 ;Row_Increment
	mov [ebx+12],dword 4 ;Col_Increment
	mov ebx,eax
	shr eax,2
	test ebx,3
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Width],eax
	
	mov eax,[Image_Height]
	cmp eax,4
	ja  @f
	mov eax,4
@@:
	sub eax,4
	mov ebx,eax
	shr eax,3
	test ebx,7
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Height],eax
	ret
;---------------------------------------------------------------------
calculate_Interlaced_4:
	call calculatte_Interlaced_data_1
	mov [ebx+0],dword 0 ;Starting_Row
	mov [ebx+4],dword 2 ;Starting_Col
	mov [ebx+8],dword 4 ;Row_Increment
	mov [ebx+12],dword 4 ;Col_Increment
	cmp eax,2
	ja  @f
	mov eax,2
@@:
	sub eax,2
	mov ebx,eax
	shr eax,2
	test ebx,3
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Width],eax

	mov eax,[Image_Height]
	mov ebx,eax
	shr eax,2
	test ebx,3
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Height],eax
	ret
;---------------------------------------------------------------------
calculate_Interlaced_5:
	call calculatte_Interlaced_data_1
	mov [ebx+0],dword 2 ;Starting_Row
	mov [ebx+4],dword 0 ;Starting_Col
	mov [ebx+8],dword 4 ;Row_Increment
	mov [ebx+12],dword 2 ;Col_Increment
	mov ebx,eax
	shr eax,1
	test ebx,1
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Width],eax
	
	mov eax,[Image_Height]
	cmp eax,2
	ja  @f
	mov eax,2
@@:
	sub eax,2
	mov ebx,eax
	shr eax,2
	test ebx,3
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Height],eax
	ret
;---------------------------------------------------------------------
calculate_Interlaced_6:
	call calculatte_Interlaced_data_1
	mov [ebx+0],dword 0 ;Starting_Row
	mov [ebx+4],dword 1 ;Starting_Col
	mov [ebx+8],dword 2 ;Row_Increment
	mov [ebx+12],dword 2 ;Col_Increment
	cmp eax,1
	ja  @f
	mov eax,1
@@:
	dec eax
	mov ebx,eax
	shr eax,1
	test ebx,1
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Width],eax
	
	mov eax,[Image_Height]
	mov ebx,eax
	shr eax,1
	test ebx,1
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Height],eax
	ret
;---------------------------------------------------------------------
calculate_Interlaced_7:
	call calculatte_Interlaced_data_1
	mov [ebx+0],dword 1 ;Starting_Row
	mov [ebx+4],dword 0 ;Starting_Col
	mov [ebx+8],dword 2 ;Row_Increment
	mov [ebx+12],dword 1 ;Col_Increment

	mov [IHDR_data.Width],eax
;	mov ebx,[raw_area]
;	mov [ebx+4],eax
	
	mov eax,[Image_Height]
	cmp eax,1
	ja  @f
	mov eax,1
@@:
	dec eax
	mov ebx,eax
	shr eax,1
	test ebx,1
	jz @f
	inc  eax
@@:
	mov [IHDR_data.Height],eax
	
;	mov ebx,[raw_area]
;	mov eax,[Image_Height]
;	mov [ebx+8],eax
	ret
;---------------------------------------------------------------------