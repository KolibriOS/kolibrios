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
filtering_RGB:
	mov  ecx,[IHDR_data.Height]
	test ecx,ecx
	jz	 .filter_end
.filter:
	mov edx,[IHDR_data.Width]
	test edx,edx
	jz	 .filter_end
	xor eax,eax
	mov [previous_pixel_value],eax
	mov [first_pixel],byte 1
	mov al,[esi]
	mov [line_filter_type],eax
	inc esi
	
.filter_x:

	mov bh,[previous_pixel_value]
	call filtering_sample_8
	mov [previous_pixel_value],bl
	
	mov bh,[previous_pixel_value+1]
	call filtering_sample_8
	mov [previous_pixel_value+1],bl

	mov bh,[previous_pixel_value+2]
	call filtering_sample_8 
	mov [previous_pixel_value+2],bl
	
	cmp [IHDR_data.Color_type],byte 6
	jne @f

	mov bh,[previous_pixel_value+3]
	call filtering_sample_8 
	mov [previous_pixel_value+3],bl
@@:
	mov [first_pixel],byte 0
	dec edx
	jnz .filter_x
	mov [first_line],byte 0
	dec ecx
	jnz .filter
.filter_end:
	ret
;------------------------------- 
filtering_sample_8:
	cld
	lodsb
	mov bl,al
	call filtering
	mov al,bl
	cld
	stosb
	ret
;---------------------------------------------------------------------
filtering_RGB_16:
	mov  ecx,[IHDR_data.Height]
	test ecx,ecx
	jz	 .filter_end_16
.filter_16:
	mov edx,[IHDR_data.Width]
	test edx,edx
	jz	 .filter_end_16
	xor eax,eax
	mov [previous_pixel_value],eax
	mov [previous_pixel_value+4],eax
	mov [first_pixel],byte 1
	mov al,[esi]
	mov [line_filter_type],eax
	inc esi
.filter_x_16:
	mov bx,[previous_pixel_value]
	call filtering_sample_16
	mov [previous_pixel_value],bx
		
	mov bx,[previous_pixel_value+2]
	call filtering_sample_16
	mov [previous_pixel_value+2],bx
		
	mov bx,[previous_pixel_value+4]
	call filtering_sample_16
	mov [previous_pixel_value+4],bx
		
	cmp [IHDR_data.Color_type],byte 6
	jne @f

	mov bx,[previous_pixel_value+6]
	call filtering_sample_16
	mov [previous_pixel_value+6],bx
@@:
	mov [first_pixel],byte 0
	dec edx
	jnz .filter_x_16
	mov [first_line],byte 0
	dec ecx
	jnz .filter_16
.filter_end_16:
	ret
;-------------------------------
filtering_sample_16:
	shl ebx,16
	cld
	lodsw
	mov  bx,ax
	call filtering
	mov  ax,bx
	cld
	stosw
	ret
;---------------------------------------------------------------------
filtering_grayscale:
	mov  ecx,[IHDR_data.Height]
	test ecx,ecx
	jz	 .filter_end_grayscale
	
	mov edx,[IHDR_data.Width]
	test edx,edx
	jz	 .filter_end_grayscale
	cmp [IHDR_data.Color_type],byte 4
	je  @f
	imul edx,[resolution]
	mov eax,edx
	shr edx,3
	test eax,7
	jz  @f
	inc  edx
@@: 

.filter_grayscale:
	push edx
	xor eax,eax
	mov [previous_pixel_value],eax
	mov [first_pixel],byte 1
	mov al,[esi]
	mov [line_filter_type],eax
	inc esi

.filter_x_grayscale:

	mov bh,[previous_pixel_value]
	call filtering_sample_8
	mov [previous_pixel_value],bl

	cmp [IHDR_data.Color_type],byte 4
	jne @f
	
	mov bh,[previous_pixel_value+1]
	call filtering_sample_8
	mov [previous_pixel_value+1],bl
@@:
	mov [first_pixel],byte 0
	dec edx
	jnz .filter_x_grayscale
	pop edx
	
	mov [first_line],byte 0
	dec ecx
	jnz .filter_grayscale
.filter_end_grayscale:
	ret
;---------------------------------------------------------------------
filtering_grayscale_16:
	mov  ecx,[IHDR_data.Height]
	test ecx,ecx
	jz	 .filter_end_grayscale_16
	
.filter_grayscale_Bit_depth_16:

	mov edx,[IHDR_data.Width]
	test edx,edx
	jz	 .filter_end_grayscale_16
	xor eax,eax
	mov [previous_pixel_value],eax
	mov [first_pixel],byte 1
	mov al,[esi]
	mov [line_filter_type],eax
	inc esi
.filter_x_grayscale_16:
	mov bx,[previous_pixel_value]
	call filtering_sample_16
	mov [previous_pixel_value],bx

	cmp [IHDR_data.Color_type],byte 4
	jne @f
	mov bx,[previous_pixel_value+2]
	call filtering_sample_16
	mov [previous_pixel_value+2],bx

@@:
	mov [first_pixel],byte 0
	dec edx
	jnz .filter_x_grayscale_16
	mov [first_line],byte 0
	dec ecx
	jnz .filter_grayscale_Bit_depth_16
.filter_end_grayscale_16:
	ret
;---------------------------------------------------------------------
