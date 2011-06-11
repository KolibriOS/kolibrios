;*****************************************************************************
; PNG to RAW convert plugin - for zSea image viewer
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

;include	'macros.inc'
include	'../../../../macros.inc'
;---------------------------------------------------------------------
START:
	pushad
	mov	eax,dword [esp+36]
	call	check_header_1
	je	header_OK
;---------------------------------------------------------------------
no_png_file:
	xor  eax,eax
	mov [raw_area],eax
	inc  eax  ; data corrupt eax = 1
	jmp  header_OK.ret
;---------------------------------------------------------------------
check_header_1:
	mov [pointer],eax
	mov ebx,[eax]
	mov [image_file],ebx	
	cmp  [ebx],dword 0x474E5089  ; check main label p.1
	jne  @f	;no_png_file
	cmp  [ebx+4],dword 0x0A1A0A0D ; check main label p.2
	ret
@@:
	add  esp,4
	jmp  no_png_file
;---------------------------------------------------------------------
header_OK:
	mov ebx,[eax+12]  ; file size
	mov [file_size],ebx
;---------------------------------------------------------------------
	mov ebx,[eax+16]
	mov [deflate_unpack],ebx
;---------------------------------------------------------------------
	mov ebx,[pointer]
;---------------------------------------------------------------------
	xor  eax,eax
;	mov  [IDAT.pointer],eax
;	mov  [IDAT.size],eax
	mov  [deflate_start_offset],eax
	mov  eax,[image_file]
	mov  [next_Chunk],eax
	call search_IHDR
	call search_IDAT  
;	call search_IEND
;---------------------------------------------------------------------	
	mov  eax,[IHDR.pointer]
	mov  ecx,[eax] ; get width PNG
	call convert_NBO_to_PC  
	mov [IHDR_data.Width],ecx	; store width PNG
	mov [Image_Width],ecx
	mov  ecx,[eax+4] ; get height PNG
	call convert_NBO_to_PC
	mov [IHDR_data.Height],ecx  ; store height PNG
	mov [Image_Height],ecx
	mov ecx,[eax+9] ; Color type
					; Compression method
						 ; Filter method
						 ; Interlace method
	mov [IHDR_data.Color_type],ecx 
	xor ecx,ecx
	mov cl,[eax+8]	; Bit depth
	mov [IHDR_data.Bit_depth],cl
;---------------------------------------------------------------------		
	mov al,[eax+9]	; check Color type
	cmp al,byte 0	 ; Each pixel is a grayscale sample.
	je  .resolution
	cmp al,byte 2	 ; Each pixel is an R,G,B triple.
	je  .Bit_depth_2
	cmp al,byte 3	 ; Each pixel is a palette index
						  ; a PLTE chunk must appear.
	jne  .no_PLTE
	call search_PLTE
	jmp  .resolution 
.no_PLTE:
	cmp al,byte 4	 ; Each pixel is a grayscale sample,
						  ; followed by an alpha sample.
	je  .Bit_depth_4
	cmp al,byte 6	 ; Each pixel is an R,G,B triple,
						  ; followed by an alpha sample.
	jne  no_png_file
;---------------------------------------------------------------------	
.Bit_depth_6:	
	shl  ecx,1
  
.Bit_depth_4:
	shl  ecx,1
	jmp  .resolution
  
.Bit_depth_2:
	lea  ecx,[ecx*3]
  
.resolution:
;---------------------------------------------------------------------	
	cmp [IHDR_data.Compression_method], byte 0 ; check Compression method
	jne  no_png_file
  
	cmp [IHDR_data.Filter_method], byte 0 ; check Filtering method
	jne  no_png_file
  
	cmp [IHDR_data.Interlace_method], byte 0 ; No Interlaced
	je	@f
	cmp [IHDR_data.Interlace_method], byte 1 ; Interlaced
	jne  no_png_file
@@:
;---------------------------------------------------------------------  
	mov [resolution],ecx
	imul ecx,[IHDR_data.Width]
	mov  edi,ecx
	shr  ecx,3
	test edi,7
	jz  @f
	inc  ecx
@@:
	imul ecx,[IHDR_data.Height]
	cmp  [IHDR_data.Color_type],byte 3
	jne  @f
	mov  eax,[PLTE.size]
	mov  ebx,3
	xor  edx,edx
	div  ebx
	shl  eax,2	
	add  ecx,eax ; PLTE table area
	jmp  .RAW_header
;--------------------------------------
@@: 
	cmp  [IHDR_data.Color_type],byte 0
	je	@f
	cmp  [IHDR_data.Color_type],byte 4
	jne  .RAW_header
@@:
	push ecx
	mov  eax,1
	movzx ecx,byte [IHDR_data.Bit_depth]
	cmp	ecx,16
	jne  @f
	shr  ecx,1
@@:
	shl eax,cl
	shl eax,2
	pop  ecx
	add ecx,eax
;	mov ebx,[pointer]
;	mov [ebx+20],eax
;--------------------------------------
.RAW_header:
	add ecx,44 ; RAW header
;	mov ebx,[pointer]
;	mov [ebx+24],ecx
;	shl ecx,1
	mcall 68, 12
	cmp  eax,0
	jne  @f
	xor  eax,eax
	mov [raw_area],eax  ; store pointer of image area
	mov  eax,2	; not enough memory
	jmp	.ret
@@:
	mov  [raw_area],eax
;	mov ebx,[pointer]
;	mov [ebx+28],eax
;---------------------------------------------------------------------	 
	mov edi,eax
	xor eax,eax
	shr ecx,2
	cld
	rep stosd	; clear memory
;--------------------------------------------------------------------- 
; Create RAW header
;---------------------------------------------------------------------
	mov  eax,[raw_area]
	mov  [eax],dword 'RAW '
;---------------------------------------------------------------------  
	mov  ecx,[IHDR_data.Width] ; width BMP
	mov  [eax+4],ecx  ; width RAW
;---------------------------------------------------------------------
	mov  ecx,[IHDR_data.Height] ; high  BMP
	mov  [eax+8],ecx  ;high  RAW
;---------------------------------------------------------------------
	mov  ecx,[resolution] ; color resolution  BMP
	mov  [eax+12],ecx  ;color resolution  RAW 
;---------------------------------------------------------------------
	xor  ecx,ecx
	mov  cl,[IHDR_data.Bit_depth]
	mov  [eax+16],cx  ;channel color resolution  RAW
;---------------------------------------------------------------------
	mov  ecx,[IHDR_data.Color_type]
	xor  ebx,ebx
	inc  ebx
	cmp  cl,byte 0	 ; Each pixel is a grayscale sample.
	je  .1
	cmp  cl,byte 2	 ; Each pixel is an R,G,B triple.
	je  .Bit_depth_2_1
	cmp  cl,byte 3	 ; Each pixel is a palette index
						  ; a PLTE chunk must appear.
	je  .1
	cmp  cl,byte 4	 ; Each pixel is a grayscale sample,
						  ; followed by an alpha sample.
	je  .Bit_depth_4_1
	cmp  cl,byte 6
	jne  no_png_file
.Bit_depth_6_1:	
	shl  ebx,1
.Bit_depth_4_1:
	shl  ebx,1
	jmp  .1
.Bit_depth_2_1:
	lea  ebx,[ebx*3]
.1:	
	mov  [eax+18],bx  ; channels amount RAW
;---------------------------------------------------------------------
	xor  edx,edx
	cmp  cl,byte 3	 ; Each pixel is a palette index
						  ; a PLTE chunk must appear.
	je  @f
	cmp  cl,0
	je	@f
	cmp  cl,4
	jne  .no_PLTE_1
@@:
	add  edx,44
	mov  [eax+20],edx ; palette pointer (offset from file start)
;---------------------------------------------------------------------
	cmp  cl,0
	je	@f
	cmp  cl,4
	jne  .PLTE
@@:
	mov  ecx,256*4
	jmp  .PLTE_1
.PLTE:
	push eax
	mov  eax,[PLTE.size]
	xor  edx,edx
	mov  ebx,3
	div  ebx
	shl  eax,2
	mov  ecx,eax
	pop  eax
.PLTE_1:
	mov  [eax+24],ecx  ; palette area size
	jmp  @f
.no_PLTE_1:
    xor  ecx,ecx
@@:
;---------------------------------------------------------------------  
	add  ecx,dword 44
	mov  [eax+28],ecx  ; pixels pointer (offset from file start)
;	mov ebx,[pointer]
;	mov [ebx+44],ecx
;---------------------------------------------------------------------
	mov	 ecx, [IHDR_data.Width] ; width BMP
	imul ecx,[resolution]
	mov  edi,ecx
	shr  ecx,3
	test edi,7
	jz  @f
	inc  ecx
@@: 
	imul ecx,[IHDR_data.Height]  ;  high  BMP
	mov  [eax+32],ecx  ; pixels area size
;---------------------------------------------------------------------  
	xor  ecx,ecx		; Stub!!!
	mov  [eax+36],ecx  ;Transparency pointer (offset from file start)
	mov  [eax+40],ecx  ;Transparency area size

;--------------------------------------------------------------------- 
; Finish create RAW header
;--------------------------------------------------------------------- 

	mov eax,[raw_area]
	mov edi,[eax+20]  ; palette pointer (offset from file start)
	add edi,eax
	
	mov esi,[PLTE.pointer]
	mov eax,[PLTE.size]
	xor edx,edx
	mov ebx,3
	div ebx
	mov ecx,eax
	mov eax,[IHDR_data.Color_type]
	cmp  al,byte 0
	je  .grayscale_palette
	cmp  al,byte 4
	je  .grayscale_palette
	cmp  al,byte 3	 ; Each pixel is a palette index
						  ; a PLTE chunk must appear.
	jne  .no_palette
@@:
	cld
	lodsd
	dec esi
	and  eax,0xffffff

	mov bl,al
	shr eax,8
	xchg ah,bl
	shl eax,8
	mov al,bl
	
	cld
	stosd
	dec ecx
	jnz @r
	jmp .no_palette
;-------------------------------
.grayscale_palette:
;	cmp  [IHDR_data.Bit_depth], dword 1
;	jne @f
;	xor eax,eax
;	cld
;	stosd
;	dec eax
;	and eax,0xffffff
;	cld
;	stosd
;	jmp  .no_palette
@@:
	mov eax,1
	mov ecx,[resolution]
	cmp ecx,8
	jbe @f
	mov ecx,8
@@:
	shl eax,cl
	mov ecx,eax
	mov eax,256
;	mov ebx,[resolution]
	xor edx,edx
	div ecx  ;ebx
	mov edx,eax
	cmp edx,0
	jne @f
	inc edx
@@:
	xor eax,eax
;	mov eax,0xffffff
;	mov ecx,256
	
@@:	
	cld
	stosd
;	inc ah
	add ah,dl
;	inc al
	add al,dl
	shl eax,8
	mov al,ah

;	dec ah
;	dec al
;	shl eax,8
;	mov al,ah
	
	and eax,0xffffff
	dec ecx
	jnz @r

	sub edi,4	
	xor eax,eax
	dec eax
	and eax,0xffffff
	cld
	stosd
;-------------------------------	
.no_palette:

;--------------------------------------------------------------------- 
	mov ebx,[IDAT.pointer]
	mov al,[ebx]
	mov [IDAT_CMF],al  ; Compression Method and flags
	mov ah,al
	and al,1111b
	mov [IDAT_СМ],al  ; Compression method
	shr ah,4
	mov [IDAT_CINFO],ah  ; Compression info
	mov al,[ebx+1]
	mov [IDAT_FLG],al  ; FLaGs 
	mov ah,al
	and al,11111b
	mov [IDAT_FCHECK],al  
	mov al,ah
	and al,100000b
	shr al,5
	mov [IDAT_FDICT],al  ; Preset dictionary
	shr ah,6
	mov [IDAT_FLEVEL],ah  ; Compression level
	mov  al,[IDAT_СМ]
;	test al,1000b	  ; Compression method = 8 ?
	cmp  al,8
	jnz  no_png_file 
	add	[IDAT.pointer],2
	sub	[IDAT.size],2
;	xor  eax,eax
;	mov  ah,[IDAT_FLG]
;	mov  al,[IDAT_CMF]
;	imul  eax,31
;	shr	eax,16
;	cmp  [IDAT_FCHECK],al
;	jne  no_png_file  

;	 
;	cmp  [IDAT_FDICT],1
;	jne  .no_IDAT_FDICT
;	mov  ecx,[ebx+2]
;	jmp  .IDAT_DICT
;.no_IDAT_FDICT:
;	mov  eax,[IDAT.pointer]
;	add  eax,[IDAT.size]
;	mov  ecx,[eax]
;.IDAT_DICT:
;	call convert_NBO_to_PC
;	mov [IDAT_DICT],ecx
;		jmp  .ret_ok

;	jmp  no_png_file 

;	mov ecx,[file_size]
;	 mcall 68, 12
;	mov [IDAT_raw_area],eax  ; store pointer of image area
;---------------------------------------------------------------------
.start_Deflate_unpack:
	push	eax	; выделяем в стеке переменную для *pUnpackedLength
			; поскольку значение неважно, короче и быстрее всего
			; сделать это однобайтовым push <регистр>
	push	esp	; а вот и сам указатель pUnpackedLength
	push	esi	; какой-нибудь параметр
	push	deflate_callback
	call	[deflate_unpack]
	pop	ecx	; выталкиваем UnpackedLength
; как и в первом случае, eax = указатель на распакованные данные, ecx = размер
	mov [unpacked_data],eax	
	mov esi,eax
;---------------------------------------------------------------------	
	mov eax,[raw_area]
	mov edi,[eax+28]  ; pixels pointer (offset from file start)
	add edi,eax
;	mov ecx,edx
	sub ecx,[IHDR_data.Height]
	mov [first_line],byte 1
	cmp [IHDR_data.Color_type],byte 0
	je  .PLTE_and_grayscale
	cmp [IHDR_data.Color_type],byte 3
	je  .PLTE_and_grayscale
	cmp [IHDR_data.Color_type],byte 4
	je  .PLTE_and_grayscale
;---------------------------------------------------------------------
; Processing filtering RGB;	type 2 and 6, sample 8b
;---------------------------------------------------------------------
	mov eax,ecx
;	sub eax,[IHDR_data.Height]
	xor edx,edx
	mov ebx,3
	cmp [IHDR_data.Color_type],byte 6
	jne @f
	inc ebx
@@:
	cmp [IHDR_data.Bit_depth],byte 16
	jne @f
	shl ebx,1
@@:
	div ebx
	mov ecx,eax
	cmp [IHDR_data.Bit_depth],byte 16
	je  .filter_Bit_depth_16
;	jmp .ret_ok
;--------------------------------------------------------------------- 
	cmp [IHDR_data.Interlace_method], byte 0 ; Interlaced
	je  @f
	call filtering_RGB_Interlaced
	jmp .converting_MSB_to_LSB
@@:
;--------------------------------------------------------------------- 
	push edi
	call filtering_RGB
	pop  edi
;---------------------------------------------------------------------
; converting MSB to LSB
;  24b = 1B x 3 sample
;  32b = 1B x 4 sample
.converting_MSB_to_LSB:
	call .calculate_all_pixels
	mov ebx,3
	cmp [IHDR_data.Color_type],byte 6
	jne @f
	inc ebx
@@:
	sub edi,ebx
@@:
	add edi,ebx
	mov al,[edi]
	mov ah,[edi+2]
	mov [edi+2],al
	mov [edi],ah
	dec ecx
	jnz @r
	jmp .ret_ok
;--------------------------------------------------------------------- 
.calculate_all_pixels:
	mov  ecx,[Image_Height]
	imul ecx,[Image_Width]
	ret
;--------------------------------------------------------------------- 
; Processing filtering RGB;	type 2 and 6, sample 16b
;---------------------------------------------------------------------
.filter_Bit_depth_16:
	cmp [IHDR_data.Interlace_method], byte 0 ; Interlaced
	je  @f
	call filtering_RGB_16_Interlaced
	jmp .converting_MSB_to_LSB_16
@@:
;---------------------------------------------------------------------
	push edi
	call filtering_RGB_16
	pop  edi
;---------------------------------------------------------------------
; converting MSB to LSB 48 or 64b
;  48b = 2B x 3 sample
;  64b = 2B x 4 sample
.converting_MSB_to_LSB_16:
	call .calculate_all_pixels
	mov ebx,3
	cmp [IHDR_data.Color_type],byte 6
	jne @f
	inc ebx
@@:
	shl ebx,1
	
	sub edi,ebx
@@:
	add edi,ebx
	
	mov ax,[edi]
	xchg al,ah
	shl  eax,16
	mov ax,[edi+4]
	xchg al,ah
	mov [edi],ax
	shr  eax,16
	mov [edi+4],ax
	
	mov ax,[edi+2]
	xchg al,ah
	mov [edi+2],ax

;	mov ax,[edi+6]
;	xchg al,ah
;	mov [edi+6],al
	
	dec ecx
	jnz @r
	jmp .ret_ok
;---------------------------------------------------------------------
; Processing filtering Grayscale and RGB; type 2, 3 and 4, sample 8b
;---------------------------------------------------------------------	
.PLTE_and_grayscale:
	cmp [IHDR_data.Color_type],byte 4
	jne @f
	shr  ecx,1
@@:
	cmp [IHDR_data.Bit_depth],byte 16
	jne  @f
	shr  ecx,1
	jmp  .filter_grayscale_Bit_depth_16_1
@@:
;---------------------------------------------------------------------
	cmp [IHDR_data.Interlace_method], byte 0 ; Interlaced
	je  @f
	call filtering_grayscale_Interlaced
	jmp .continue_1
@@:
;---------------------------------------------------------------------
	push edi
	call filtering_grayscale
	pop  edi
;---------------------------------------------------------------------
.continue_1:
	cmp [IHDR_data.Color_type],byte 4
	jne  .ret_ok
;-------------------------------
; 8b or less	 
	mov  esi,edi
	call .calculate_all_pixels
.convert_transp_to_notransp:
	cld
	lodsw
	stosb
	dec ecx
	jnz .convert_transp_to_notransp
	
	jmp  .ret_ok
;---------------------------------------------------------------------
; Processing filtering Grayscale and RGB; type 2, 3 and 4, sample 16b
;---------------------------------------------------------------------
.filter_grayscale_Bit_depth_16_1:
	cmp [IHDR_data.Interlace_method], byte 0 ; Interlaced
	je  @f
	call filtering_grayscale_16_Interlaced
	jmp .continue_2
@@:
;---------------------------------------------------------------------
	push edi
	call filtering_grayscale_16
	pop  edi
;---------------------------------------------------------------------
.continue_2:
	cmp  [IHDR_data.Color_type],byte 4
	jne  .ret_ok
;-------------------------------
; 16b 
	mov  esi,edi 
	call .calculate_all_pixels
.convert_transp_to_notransp_1:
	cld
	lodsd
	stosw
	dec  ecx
	jnz  .convert_transp_to_notransp_1
;---------------------------------------------------------------------		
.ret_ok:
	mov	ecx,[unpacked_data]
	mcall 68, 13,
	xor  eax,eax
;---------------------------------------------------------------------	 
.ret:
	mov	ebx,[pointer]
	mov	[ebx+8],eax  ; store return code
	mov	eax,[raw_area]
	mov	[ebx+4],eax  ; store RAW pointer
	popad
	ret	4
;---------------------------------------------------------------------
include 'filter.inc'
include 'proced.inc'
include 'fl_call.inc'
include 'interlac.inc'
;---------------------------------------------------------------------
Check_Header:
	pushad
	mov	eax,dword [esp+36]
	call	check_header_1
	jne	no_png_file
	popad
	ret	4
;---------------------------------------------------------------------
Associations:
	dd Associations.end - Associations
	db 'PNG',0
.end:
	db 0
;---------------------------------------------------------------------
align 4
EXPORTS:
		dd		szStart,	START
		dd		szVersion,	0x00010002
		dd		szCheck,	Check_Header
		dd		szAssoc,	Associations
		dd		0

szStart		db 'START',0
szVersion	db 'version',0
szCheck		db 'Check_Header',0
szAssoc		db 'Associations',0

;*****************************************************************************
; Chunks names
;*****************************************************************************
; Critical chunks
IHDR_name: db 'IHDR' ; Image header
PLTE_name: db 'PLTE' ; Palette
IDAT_name: db 'IDAT' ; Image data
IEND_name: db 'IEND' ; Image trailer

; Ancillary chunks
;tRNS_name: db 'tRNS' ; Transparency

;; Color space information
;gAMA_name: db 'gAMA' ; Image gamma
;cHRM_name: db 'cHRM' ; Primary chromaticities
;sRGB_name: db 'sRGB' ; Standard RGB color space
;iCCP_name: db 'iCCP' ; Embedded ICC profile

;; Textual information
;tEXt_name: db 'tEXt' ; Textual data
;zTXt_name: db 'zTXt' ; Compressed textual data
;iTXt_name: db 'iTXt' ; International textual data

;; Miscellaneous information
;bKGD_name: db 'bKGD' ; Background color
;pHYs_name: db 'pHYs' ; Physical pixel dimensions
;sBIT_name: db 'sBIT' ; Significant bits
;sPLT_name: db 'sPLT' ; Suggested palette
;hIST_name: db 'hIST' ; Palette histogram
;tIME_name: db 'tIME' ; Imagelast-modification time
;*****************************************************************************
calculate_Interlaced_counters:
.1:	dd calculate_Interlaced_1
.2:	dd calculate_Interlaced_2
.3:	dd calculate_Interlaced_3
.4:	dd calculate_Interlaced_4
.5:	dd calculate_Interlaced_5
.6:	dd calculate_Interlaced_6
.7:	dd calculate_Interlaced_7

deflate_start_offset dd 0

pointer			dd 0
image_file		dd 0
file_size		dd 0
raw_area		dd 0
resolution		dd 0

Chunk_pointer	dd 0
next_Chunk		dd 0

deflate_unpack	dd 0

unpacked_data	dd 0
;IDAT_raw_area  dd 0
;IDAT_raw_counter dd 0

Interlaced_area	dd 0

Starting_Row:	dd 0
Starting_Col:	dd 0
Row_Increment:	dd 0
Col_Increment:	dd 0

Interlaced_step	dd 0


counter_IDAT_Chunk dd 0

;CRC32			 dd 0
;CRC32table:  rd 256
;Adler32		  dd 0

Image_Width dd 0
Image_Height dd 0

IHDR_data:
.Width					dd 0  ;+0
.Height					dd 0  ;+4
.Bit_depth:				db 0  ;+8
.Color_type:			db 0  ;+9
.Compression_method:	db 0  ;+10
.Filter_method:			db 0  ;+11
.Interlace_method:		db 0  ;+12

IDAT_CMF		db 0		  ; Compression Method and flags
  IDAT_СМ		db 0		; Compression method
  IDAT_CINFO	db 0		; Compression info
IDAT_FLG		db 0		  ; FLaGs 
  IDAT_FCHECK	db 0	; 
  IDAT_FDICT	db 0	; Preset dictionary
  IDAT_FLEVEL	db 0	; Compression level
							  ; "deflate" method (CM = 8) sets these flags:
							  ; 0 - compressor used fastest algorithm
							  ; 1 - compressor used fast algorithm
							  ; 2 - compressor used default algorithm
							  ; 3 - compressor used maximum compression, slowest algorithm  
IDAT_DICT	dd 0  ; dictionary identifier = Adler-32 checksum
BFINAL		db 0  ; set 1 only if this is the last block of the data set
BTYPE		db 0  ; specifies how the data are compressed:
				 ; 00 - no compression
				 ; 01 - compressed with fixed Huffman codes
				 ; 10 - compressed with dynamic Huffman codes
				 ; 11 - reserved (error)
				 
line_filter_type	dd 0 ; 0 None
							 ; 1 Sub
							 ; 2 Up
							 ; 3 Averag
							 ; 4 Paeth
first_line	db 0
first_pixel	db 0

previous_pixel_value:
 rb 8
 
Paeth_filter:
.a:	dw 0
.b:	dw 0
.c:	dw 0
.p	dd 0
.pa	dd 0
.pb	dd 0
.pc	dd 0
;*****************************************************************************
; Chunks pointer
;*****************************************************************************
IHDR:
.pointer	dd 0
.size		dd 0

PLTE:
.pointer	dd 0
.size		dd 0

IDAT:
.pointer	dd 0
.size		dd 0

IEND:
.pointer	dd 0
.size		dd 0

;tRNS:
;.pointer	dd 0
;.size		dd 0

;gAMA:
;.pointer	dd 0
;.size		dd 0

;cHRM:
;.pointer	dd 0
;.size		dd 0

;sRGB:
;.pointer	dd 0
;.size		dd 0

;iCCP:
;.pointer	dd 0
;.size		dd 0

;tEXt:
;.pointer	dd 0
;.size		dd 0

;zTXt:
;.pointer	dd 0
;.size		dd 0

;iTXt:
;.pointer	dd 0
;.size		dd 0

;bKGD:
;.pointer	dd 0
;.size		dd 0

;pHYs:
;.pointer	dd 0
;.size		dd 0

;sBIT:
;.pointer	dd 0
;.size		dd 0

;sPLT: 
;.pointer	dd 0
;.size		dd 0

;hIST: 
;.pointer	dd 0
;.size		dd 0

;tIME:
;.pointer	dd 0
;.size		dd 0
