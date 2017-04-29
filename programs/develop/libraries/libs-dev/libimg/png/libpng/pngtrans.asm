
; pngtrans.asm - transforms the data in a row (used by both readers and writers)

; Last changed in libpng 1.6.24 [August 4, 2016]
; Copyright (c) 1998-2002,2004,2006-2016 Glenn Randers-Pehrson
; (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
; (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)

; This code is released under the libpng license.
; For conditions of distribution and use, see the disclaimer
; and license in png.inc


if (PNG_READ_SUPPORTED eq 1) | (PNG_WRITE_SUPPORTED eq 1)

; Turn on BGR-to-RGB mapping
;void (png_structrp png_ptr)
align 4
proc png_set_bgr uses edi, png_ptr:dword
	png_debug 1, 'in png_set_bgr'

	mov edi,[png_ptr]
	test edi,edi
	jz @f ;if (..==0) return
		or dword[edi+png_struct.transformations], PNG_BGR
	@@:
	ret
endp

; Turn on 16-bit byte swapping
;void (png_structrp png_ptr)
align 4
proc png_set_swap uses edi, png_ptr:dword
	png_debug 1, 'in png_set_swap'

	mov edi,[png_ptr]
	test edi,edi
	jz @f ;if (..==0) return

	cmp byte[edi+png_struct.bit_depth],16
	jne @f ;if (..==..)
		or dword[edi+png_struct.transformations], PNG_SWAP_BYTES
	@@:
	ret
endp

; Turn on pixel packing
;void (png_structrp png_ptr)
align 4
proc png_set_packing uses edi, png_ptr:dword
	png_debug 1, 'in png_set_packing'

	mov edi,[png_ptr]
	test edi,edi
	jz @f ;if (..==0) return

	cmp byte[edi+png_struct.bit_depth],8
	jge @f ;if (..<..)
		or dword[edi+png_struct.transformations], PNG_PACK
if PNG_WRITE_SUPPORTED eq 1
		mov byte[edi+png_struct.usr_bit_depth],8
end if
	@@:
	ret
endp

; Turn on packed pixel swapping
;void (png_structrp png_ptr)
align 4
proc png_set_packswap uses edi, png_ptr:dword
	png_debug 1, 'in png_set_packswap'

	mov edi,[png_ptr]
	test edi,edi
	jz @f ;if (..==0) return

	cmp byte[edi+png_struct.bit_depth],8
	jge @f ;if (..<..)
		or dword[edi+png_struct.transformations], PNG_PACKSWAP
	@@:
	ret
endp

;void (png_structrp png_ptr, png_color_8p true_bits)
align 4
proc png_set_shift uses ecx edi, png_ptr:dword, true_bits:dword
	png_debug 1, 'in png_set_shift'

	mov edi,[png_ptr]
	test edi,edi
	jz @f ;if (..==0) return

	or dword[edi+png_struct.transformations], PNG_SHIFT
	mov ecx,sizeof.png_color_8
	mov edi,[edi+png_struct.shift]
	mov esi,[true_bits]
	rep movsb
	@@:
	ret
endp

;int (png_structrp png_ptr)
align 4
proc png_set_interlace_handling uses edi, png_ptr:dword
	png_debug 1, 'in png_set_interlace handling'

	mov edi,[png_ptr]
	test edi,edi
	jz @f
	cmp byte[edi+png_struct.interlaced],0
	je @f ;if(..!=0 && ..!=0)
		or dword[edi+png_struct.transformations], PNG_INTERLACE
		mov eax,7
		jmp .end_f
	@@:

	xor eax,eax
	inc eax
.end_f:
	ret
endp

; Add a filler byte on read, or remove a filler or alpha byte on write.
; The filler type has changed in v0.95 to allow future 2-byte fillers
; for 48-bit input data, as well as to avoid problems with some compilers
; that don't like bytes as parameters.

;void (png_structrp png_ptr, uint_32 filler, int filler_loc)
align 4
proc png_set_filler uses eax edi, png_ptr:dword, filler:dword, filler_loc:dword
	png_debug 1, 'in png_set_filler'

	mov edi,[png_ptr]
	test edi,edi
	jz .end_f ;if (..==0) return

	; In libpng 1.6 it is possible to determine whether this is a read or write
	; operation and therefore to do more checking here for a valid call.

	mov eax,[edi+png_struct.mode]
	and eax,PNG_IS_READ_STRUCT
	jz @f ;if (..!=0)
if PNG_READ_FILLER_SUPPORTED eq 1
		; On read png_set_filler is always valid, regardless of the base PNG
		; format, because other transformations can give a format where the
		; filler code can execute (basically an 8 or 16-bit component RGB or G
		; format.)

		; NOTE: usr_channels is not used by the read code!  (This has led to
		; confusion in the past.)  The filler is only used in the read code.

		mov eax,[filler]
		mov [edi+png_struct.filler],ax
		jmp .end0
else
		cStr ,'png_set_filler not supported on read'
		stdcall png_app_error, edi, eax
		jmp .end_f
end if
	@@: ;else ;write
if PNG_WRITE_FILLER_SUPPORTED eq 1
	; On write the usr_channels parameter must be set correctly at the
	; start to record the number of channels in the app-supplied data.

;         switch (png_ptr->color_type)
;         {
;            case PNG_COLOR_TYPE_RGB:
;               png_ptr->usr_channels = 4;
;               break;

;            case PNG_COLOR_TYPE_GRAY:
;               if (png_ptr->bit_depth >= 8)
;               {
;                  png_ptr->usr_channels = 2;
;                  break;
;               }

;               else
;               {
			; There simply isn't any code in libpng to strip out bits
			; from bytes when the components are less than a byte in
			; size!

;                  png_app_error(png_ptr,
;                      "png_set_filler is invalid for"
;                      " low bit depth gray output");
;                  return;
;               }

;            default:
;               png_app_error(png_ptr,
;                   "png_set_filler: inappropriate color type");
;               return;
;         }
else
		cStr ,'png_set_filler not supported on write'
		stdcall png_app_error, edi, eax
		jmp .end_f
end if
	.end0:

	; Here on success - libpng supports the operation, set the transformation
	; and the flag to say where the filler channel is.

	or dword[edi+png_struct.transformations],PNG_FILLER

	cmp dword[filler_loc],PNG_FILLER_AFTER
	jne @f ;if (..==..)
		or dword[edi+png_struct.flags],PNG_FLAG_FILLER_AFTER
		jmp .end_f
	@@: ;else
		and dword[edi+png_struct.flags],not PNG_FLAG_FILLER_AFTER
.end_f:
	ret
endp

; Added to libpng-1.2.7
;void (png_structrp png_ptr, uint_32 filler, int filler_loc)
align 4
proc png_set_add_alpha uses eax edi, png_ptr:dword, filler:dword, filler_loc:dword
	png_debug 1, 'in png_set_add_alpha'

	mov edi,[png_ptr]
	test edi,edi
	jz .end_f ;if (..==0) return

	stdcall png_set_filler, edi, [filler], [filler_loc]
	; The above may fail to do anything.
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_FILLER
	jz .end_f ;if (..!=0)
		or dword[edi+png_struct.transformations],PNG_ADD_ALPHA
.end_f:
	ret
endp

;void (png_structrp png_ptr)
align 4
proc png_set_swap_alpha uses edi, png_ptr:dword
	png_debug 1, 'in png_set_swap_alpha'

	mov edi,[png_ptr]
	test edi,edi
	jz .end_f ;if (..==0) return
		or dword[edi+png_struct.transformations], PNG_SWAP_ALPHA
.end_f:
	ret
endp


;void (png_structrp png_ptr)
align 4
proc png_set_invert_alpha uses edi, png_ptr:dword
	png_debug 1, 'in png_set_invert_alpha'

	mov edi,[png_ptr]
	test edi,edi
	jz .end_f ;if (..==0) return
		or dword[edi+png_struct.transformations], PNG_INVERT_ALPHA
.end_f:
	ret
endp

;void (png_structrp png_ptr)
align 4
proc png_set_invert_mono uses edi, png_ptr:dword
	png_debug 1, 'in png_set_invert_mono'

	mov edi,[png_ptr]
	test edi,edi
	jz .end_f ;if (..==0) return
		or dword[edi+png_struct.transformations], PNG_INVERT_MONO
.end_f:
	ret
endp

; Invert monochrome grayscale data
;void (png_row_infop row_info, bytep row)
align 4
proc png_do_invert uses eax ebx ecx edx, row_info:dword, row:dword
	;ecx - i
	;eax - rp
	;edx - istop
	png_debug 1, 'in png_do_invert'

	mov ebx,[row_info]
	cmp byte[ebx+png_row_info.color_type],PNG_COLOR_TYPE_GRAY
	jne .end0
	mov eax,[row]
	mov edx,[ebx+png_row_info.rowbytes]
	xor ecx,ecx
	jmp @f
align 4
	.cycle0:
		inc ecx
	@@:
		cmp ecx,edx
		jae .end_f
		not byte[eax]
		inc eax
		jmp .cycle0
.end0:
	cmp byte[ebx+png_row_info.color_type],PNG_COLOR_TYPE_GRAY_ALPHA
	jne .end1
	cmp byte[ebx+png_row_info.bit_depth],8
	jne .end1
	mov eax,[row]
	mov edx,[ebx+png_row_info.rowbytes]
	xor ecx,ecx
	jmp @f
align 4
	.cycle1:
		add ecx,2
	@@:
		cmp ecx,edx
		jae .end_f
		not byte[eax]
		add eax,2
		jmp .cycle1
.end1:
if PNG_16BIT_SUPPORTED eq 1
	cmp byte[ebx+png_row_info.color_type],PNG_COLOR_TYPE_GRAY_ALPHA
	jne .end_f
	cmp byte[ebx+png_row_info.bit_depth],16
	jne .end_f
	mov eax,[row]
	mov edx,[ebx+png_row_info.rowbytes]
	xor ecx,ecx
	jmp @f
align 4
	.cycle2:
		add ecx,4
	@@:
		cmp ecx,edx
		jae .end_f
		not word[eax]
		add eax,4
		jmp .cycle2
end if
.end_f:
	ret
endp

; Swaps byte order on 16-bit depth images
;void (png_row_infop row_info, bytep row)
align 4
proc png_do_swap, row_info:dword, row:dword
	png_debug 1, 'in png_do_swap'

;   if (row_info->bit_depth == 16)
;   {
;      bytep rp = row;
;      uint_32 i;
;      uint_32 istop= row_info->width * row_info->channels;

;      for (i = 0; i < istop; i++, rp += 2)
;      {
if PNG_BUILTIN_BSWAP16_SUPPORTED eq 1
	; Feature added to libpng-1.6.11 for testing purposes, not
	; enabled by default.

;         *(uint_16*)rp = __builtin_bswap16(*(uint_16*)rp);
else
;         byte t = *rp;
;         *rp = *(rp + 1);
;         *(rp + 1) = t;
end if
;      }
;   }
	ret
endp

if (PNG_READ_PACKSWAP_SUPPORTED eq 1) | (PNG_WRITE_PACKSWAP_SUPPORTED eq 1)
align 4
onebppswaptable db 0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,\
   0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,\
   0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,\
   0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,\
   0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,\
   0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,\
   0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,\
   0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,\
   0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,\
   0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,\
   0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,\
   0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,\
   0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,\
   0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,\
   0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,\
   0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,\
   0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,\
   0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,\
   0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,\
   0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,\
   0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,\
   0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,\
   0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,\
   0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,\
   0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,\
   0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,\
   0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,\
   0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,\
   0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,\
   0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,\
   0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,\
   0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF

align 4
twobppswaptable db 0x00, 0x40, 0x80, 0xC0, 0x10, 0x50, 0x90, 0xD0,\
   0x20, 0x60, 0xA0, 0xE0, 0x30, 0x70, 0xB0, 0xF0,\
   0x04, 0x44, 0x84, 0xC4, 0x14, 0x54, 0x94, 0xD4,\
   0x24, 0x64, 0xA4, 0xE4, 0x34, 0x74, 0xB4, 0xF4,\
   0x08, 0x48, 0x88, 0xC8, 0x18, 0x58, 0x98, 0xD8,\
   0x28, 0x68, 0xA8, 0xE8, 0x38, 0x78, 0xB8, 0xF8,\
   0x0C, 0x4C, 0x8C, 0xCC, 0x1C, 0x5C, 0x9C, 0xDC,\
   0x2C, 0x6C, 0xAC, 0xEC, 0x3C, 0x7C, 0xBC, 0xFC,\
   0x01, 0x41, 0x81, 0xC1, 0x11, 0x51, 0x91, 0xD1,\
   0x21, 0x61, 0xA1, 0xE1, 0x31, 0x71, 0xB1, 0xF1,\
   0x05, 0x45, 0x85, 0xC5, 0x15, 0x55, 0x95, 0xD5,\
   0x25, 0x65, 0xA5, 0xE5, 0x35, 0x75, 0xB5, 0xF5,\
   0x09, 0x49, 0x89, 0xC9, 0x19, 0x59, 0x99, 0xD9,\
   0x29, 0x69, 0xA9, 0xE9, 0x39, 0x79, 0xB9, 0xF9,\
   0x0D, 0x4D, 0x8D, 0xCD, 0x1D, 0x5D, 0x9D, 0xDD,\
   0x2D, 0x6D, 0xAD, 0xED, 0x3D, 0x7D, 0xBD, 0xFD,\
   0x02, 0x42, 0x82, 0xC2, 0x12, 0x52, 0x92, 0xD2,\
   0x22, 0x62, 0xA2, 0xE2, 0x32, 0x72, 0xB2, 0xF2,\
   0x06, 0x46, 0x86, 0xC6, 0x16, 0x56, 0x96, 0xD6,\
   0x26, 0x66, 0xA6, 0xE6, 0x36, 0x76, 0xB6, 0xF6,\
   0x0A, 0x4A, 0x8A, 0xCA, 0x1A, 0x5A, 0x9A, 0xDA,\
   0x2A, 0x6A, 0xAA, 0xEA, 0x3A, 0x7A, 0xBA, 0xFA,\
   0x0E, 0x4E, 0x8E, 0xCE, 0x1E, 0x5E, 0x9E, 0xDE,\
   0x2E, 0x6E, 0xAE, 0xEE, 0x3E, 0x7E, 0xBE, 0xFE,\
   0x03, 0x43, 0x83, 0xC3, 0x13, 0x53, 0x93, 0xD3,\
   0x23, 0x63, 0xA3, 0xE3, 0x33, 0x73, 0xB3, 0xF3,\
   0x07, 0x47, 0x87, 0xC7, 0x17, 0x57, 0x97, 0xD7,\
   0x27, 0x67, 0xA7, 0xE7, 0x37, 0x77, 0xB7, 0xF7,\
   0x0B, 0x4B, 0x8B, 0xCB, 0x1B, 0x5B, 0x9B, 0xDB,\
   0x2B, 0x6B, 0xAB, 0xEB, 0x3B, 0x7B, 0xBB, 0xFB,\
   0x0F, 0x4F, 0x8F, 0xCF, 0x1F, 0x5F, 0x9F, 0xDF,\
   0x2F, 0x6F, 0xAF, 0xEF, 0x3F, 0x7F, 0xBF, 0xFF

align 4
fourbppswaptable db 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,\
   0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0,\
   0x01, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71,\
   0x81, 0x91, 0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1,\
   0x02, 0x12, 0x22, 0x32, 0x42, 0x52, 0x62, 0x72,\
   0x82, 0x92, 0xA2, 0xB2, 0xC2, 0xD2, 0xE2, 0xF2,\
   0x03, 0x13, 0x23, 0x33, 0x43, 0x53, 0x63, 0x73,\
   0x83, 0x93, 0xA3, 0xB3, 0xC3, 0xD3, 0xE3, 0xF3,\
   0x04, 0x14, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74,\
   0x84, 0x94, 0xA4, 0xB4, 0xC4, 0xD4, 0xE4, 0xF4,\
   0x05, 0x15, 0x25, 0x35, 0x45, 0x55, 0x65, 0x75,\
   0x85, 0x95, 0xA5, 0xB5, 0xC5, 0xD5, 0xE5, 0xF5,\
   0x06, 0x16, 0x26, 0x36, 0x46, 0x56, 0x66, 0x76,\
   0x86, 0x96, 0xA6, 0xB6, 0xC6, 0xD6, 0xE6, 0xF6,\
   0x07, 0x17, 0x27, 0x37, 0x47, 0x57, 0x67, 0x77,\
   0x87, 0x97, 0xA7, 0xB7, 0xC7, 0xD7, 0xE7, 0xF7,\
   0x08, 0x18, 0x28, 0x38, 0x48, 0x58, 0x68, 0x78,\
   0x88, 0x98, 0xA8, 0xB8, 0xC8, 0xD8, 0xE8, 0xF8,\
   0x09, 0x19, 0x29, 0x39, 0x49, 0x59, 0x69, 0x79,\
   0x89, 0x99, 0xA9, 0xB9, 0xC9, 0xD9, 0xE9, 0xF9,\
   0x0A, 0x1A, 0x2A, 0x3A, 0x4A, 0x5A, 0x6A, 0x7A,\
   0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0xFA,\
   0x0B, 0x1B, 0x2B, 0x3B, 0x4B, 0x5B, 0x6B, 0x7B,\
   0x8B, 0x9B, 0xAB, 0xBB, 0xCB, 0xDB, 0xEB, 0xFB,\
   0x0C, 0x1C, 0x2C, 0x3C, 0x4C, 0x5C, 0x6C, 0x7C,\
   0x8C, 0x9C, 0xAC, 0xBC, 0xCC, 0xDC, 0xEC, 0xFC,\
   0x0D, 0x1D, 0x2D, 0x3D, 0x4D, 0x5D, 0x6D, 0x7D,\
   0x8D, 0x9D, 0xAD, 0xBD, 0xCD, 0xDD, 0xED, 0xFD,\
   0x0E, 0x1E, 0x2E, 0x3E, 0x4E, 0x5E, 0x6E, 0x7E,\
   0x8E, 0x9E, 0xAE, 0xBE, 0xCE, 0xDE, 0xEE, 0xFE,\
   0x0F, 0x1F, 0x2F, 0x3F, 0x4F, 0x5F, 0x6F, 0x7F,\
   0x8F, 0x9F, 0xAF, 0xBF, 0xCF, 0xDF, 0xEF, 0xFF
end if ;PACKSWAP || WRITE_PACKSWAP

; Swaps pixel packing order within bytes
;void (png_row_infop row_info, bytep row)
align 4
proc png_do_packswap uses eax edx edi esi, row_info:dword, row:dword
	png_debug 1, 'in png_do_packswap'

	mov eax,[row_info]
	cmp byte[eax+png_row_info.bit_depth],8
	jge .end_f ;if (..<..)
		;edi = rp
		;esi = table

		mov edx,[eax+png_row_info.rowbytes]
		mov edi,[row]
		add edx,edi

		cmp byte[eax+png_row_info.bit_depth],1
		jne @f ;if (..==..)
			mov esi,onebppswaptable
			jmp .cycle0
		@@:
		cmp byte[eax+png_row_info.bit_depth],2
		jne @f ;else if (..==..)
			mov esi,twobppswaptable
			jmp .cycle0
		@@:
		cmp byte[eax+png_row_info.bit_depth],4
		jne .end_f ;else if (..==..)
			mov esi,fourbppswaptable
align 4
		.cycle0: ;for (..=..;..<..;..)
			cmp edi,edx
			jge .end_f
			movzx eax,byte[edi]
			mov al,byte[esi+eax]
			stosb ;*rp = table[*rp]
			jmp .cycle0
.end_f:
	ret
endp

; Remove a channel - this used to be 'png_do_strip_filler' but it used a
; somewhat weird combination of flags to determine what to do.  All the calls
; to png_do_strip_filler are changed in 1.5.2 to call this instead with the
; correct arguments.

; The routine isn't general - the channel must be the channel at the start or
; end (not in the middle) of each pixel.

;void (png_row_infop row_info, bytep row, int at_start)
align 4
proc png_do_strip_channel, row_info:dword, row:dword, at_start:dword
;   bytep sp = row; /* source pointer */
;   bytep dp = row; /* destination pointer */
;   bytep ep = row + row_info->rowbytes; /* One beyond end of row */

	; At the start sp will point to the first byte to copy and dp to where
	; it is copied to.  ep always points just beyond the end of the row, so
	; the loop simply copies (channels-1) channels until sp reaches ep.

	; at_start:        0 -- convert AG, XG, ARGB, XRGB, AAGG, XXGG, etc.
	;            nonzero -- convert GA, GX, RGBA, RGBX, GGAA, RRGGBBXX, etc.


	; GA, GX, XG cases
;   if (row_info->channels == 2)
;   {
;      if (row_info->bit_depth == 8)
;      {
;         if (at_start != 0) /* Skip initial filler */
;            ++sp;
;         else          /* Skip initial channel and, for sp, the filler */
;            sp += 2, ++dp;

	; For a 1 pixel wide image there is nothing to do
;         while (sp < ep)
;            *dp++ = *sp, sp += 2;

;         row_info->pixel_depth = 8;
;      }

;      else if (row_info->bit_depth == 16)
;      {
;         if (at_start != 0) /* Skip initial filler */
;            sp += 2;
;         else          /* Skip initial channel and, for sp, the filler */
;            sp += 4, dp += 2;

;         while (sp < ep)
;            *dp++ = *sp++, *dp++ = *sp, sp += 3;

;         row_info->pixel_depth = 16;
;      }

;      else
;         return; /* bad bit depth */

;      row_info->channels = 1;

	; Finally fix the color type if it records an alpha channel
;      if (row_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
;         row_info->color_type = PNG_COLOR_TYPE_GRAY;
;   }

	; RGBA, RGBX, XRGB cases
;   else if (row_info->channels == 4)
;   {
;      if (row_info->bit_depth == 8)
;      {
;         if (at_start != 0) /* Skip initial filler */
;            ++sp;
;         else          /* Skip initial channels and, for sp, the filler */
;            sp += 4, dp += 3;

	; Note that the loop adds 3 to dp and 4 to sp each time.
;         while (sp < ep)
;            *dp++ = *sp++, *dp++ = *sp++, *dp++ = *sp, sp += 2;

;         row_info->pixel_depth = 24;
;      }

;      else if (row_info->bit_depth == 16)
;      {
;         if (at_start != 0) /* Skip initial filler */
;            sp += 2;
;         else          /* Skip initial channels and, for sp, the filler */
;            sp += 8, dp += 6;

;         while (sp < ep)
;         {
;            /* Copy 6 bytes, skip 2 */
;            *dp++ = *sp++, *dp++ = *sp++;
;            *dp++ = *sp++, *dp++ = *sp++;
;            *dp++ = *sp++, *dp++ = *sp, sp += 3;
;         }

;         row_info->pixel_depth = 48;
;      }

;      else
;         return; /* bad bit depth */

;      row_info->channels = 3;

	; Finally fix the color type if it records an alpha channel
;      if (row_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
;         row_info->color_type = PNG_COLOR_TYPE_RGB;
;   }

;   else
;      return; /* The filler channel has gone already */

	; Fix the rowbytes value.
;   row_info->rowbytes = dp-row;
	ret
endp

; Swaps red and blue bytes within a pixel
;void (png_row_infop row_info, bytep row)
align 4
proc png_do_bgr, row_info:dword, row:dword
	png_debug 1, 'in png_do_bgr'
	;ebx - rp
	;ecx - i
	;esi - row_width
pushad
	mov edi,[row_info]
	movzx eax,byte[edi+png_row_info.color_type]
	and eax,PNG_COLOR_MASK_COLOR
	je .end_f
	mov esi,[edi+png_row_info.width]
	cmp byte[edi+png_row_info.bit_depth],8
	jne .end0
	cmp byte[edi+png_row_info.color_type],PNG_COLOR_TYPE_RGB
	jne .end1
	xor ecx,ecx
	mov ebx,[row]
	jmp @f
align 4
	.cycle0:
		inc ecx
		add ebx,3
	@@:
		cmp ecx,esi
		jae .end_f
		mov dl,[ebx]
		mov al,[ebx+2]
		mov [ebx],al
		mov [ebx+2],dl
		jmp .cycle0
.end1:
	cmp byte[edi+png_row_info.color_type],PNG_COLOR_TYPE_RGB_ALPHA
	jne .end_f
	xor ecx,ecx
	mov ebx,[row]
	jmp @f
align 4
	.cycle1:
		inc ecx
		add ebx,4
	@@:
		cmp ecx,esi
		jae .end_f
		mov dl,[ebx]
		mov al,[ebx+2]
		mov [ebx],al
		mov [ebx+2],dl
		jmp .cycle1
.end0:
if PNG_16BIT_SUPPORTED eq 1
	cmp byte[edi+png_row_info.bit_depth],16
	jne .end_f
	cmp byte[edi+png_row_info.color_type],PNG_COLOR_TYPE_RGB
	jne .end2
	xor ecx,ecx
	mov ebx,[row]
	jmp @f
align 4
	.cycle2:
		inc ecx
		add ebx,6
	@@:
		cmp ecx,esi
		jae .end_f
		mov dx,[ebx]
		mov ax,[ebx+4]
		mov [ebx],ax
		mov [ebx+4],dx
		jmp .cycle2
.end2:
	cmp byte[edi+png_row_info.color_type],PNG_COLOR_TYPE_RGB_ALPHA
	jne .end_f
	xor ecx,ecx
	mov ebx,[row]
	jmp @f
align 4
	.cycle3:
		inc ecx
		add ebx,8
	@@:
		cmp ecx,esi
		jae .end_f
		mov dx,[ebx]
		mov ax,[ebx+4]
		mov [ebx],ax
		mov [ebx+4],dx
		jmp .cycle3
end if
.end_f:
popad
	ret
endp

; Added at libpng-1.5.10
;void (png_structrp png_ptr, png_row_infop row_info)
align 4
proc png_do_check_palette_indexes, png_ptr:dword, row_info:dword
;   if (png_ptr->num_palette < (1 << row_info->bit_depth) &&
;      png_ptr->num_palette > 0) /* num_palette can be 0 in MNG files */
;   {
	; Calculations moved outside switch in an attempt to stop different
	; compiler warnings.  'padding' is in *bits* within the last byte, it is
	; an 'int' because pixel_depth becomes an 'int' in the expression below,
	; and this calculation is used because it avoids warnings that other
	; forms produced on either GCC or MSVC.

;      int padding = (-row_info->pixel_depth * row_info->width) & 7;
;      bytep rp = png_ptr->row_buf + row_info->rowbytes;

;      switch (row_info->bit_depth)
;      {
;         case 1:
;         {
;            /* in this case, all bytes must be 0 so we don't need
	; to unpack the pixels except for the rightmost one.

;            for (; rp > png_ptr->row_buf; rp--)
;            {
;              if ((*rp >> padding) != 0)
;                 png_ptr->num_palette_max = 1;
;              padding = 0;
;            }

;            break;
;         }

;         case 2:
;         {
;            for (; rp > png_ptr->row_buf; rp--)
;            {
;              int i = ((*rp >> padding) & 0x03);

;              if (i > png_ptr->num_palette_max)
;                 png_ptr->num_palette_max = i;

;              i = (((*rp >> padding) >> 2) & 0x03);

;              if (i > png_ptr->num_palette_max)
;                 png_ptr->num_palette_max = i;

;              i = (((*rp >> padding) >> 4) & 0x03);

;              if (i > png_ptr->num_palette_max)
;                 png_ptr->num_palette_max = i;

;              i = (((*rp >> padding) >> 6) & 0x03);

;              if (i > png_ptr->num_palette_max)
;                 png_ptr->num_palette_max = i;

;              padding = 0;
;            }

;            break;
;         }

;         case 4:
;         {
;            for (; rp > png_ptr->row_buf; rp--)
;            {
;              int i = ((*rp >> padding) & 0x0f);

;              if (i > png_ptr->num_palette_max)
;                 png_ptr->num_palette_max = i;

;              i = (((*rp >> padding) >> 4) & 0x0f);

;              if (i > png_ptr->num_palette_max)
;                 png_ptr->num_palette_max = i;

;              padding = 0;
;            }

;            break;
;         }

;         case 8:
;         {
;            for (; rp > png_ptr->row_buf; rp--)
;            {
;               if (*rp > png_ptr->num_palette_max)
;                  png_ptr->num_palette_max = (int) *rp;
;            }

;            break;
;         }

;         default:
;            break;
;      }
;   }
	ret
endp

;void (png_structrp png_ptr, voidp user_transform_ptr, int user_transform_depth, int user_transform_channels)
align 4
proc png_set_user_transform_info uses eax edi, png_ptr:dword, user_transform_ptr:dword, user_transform_depth:dword, user_transform_channels:dword
	png_debug 1, 'in png_set_user_transform_info'

	mov edi,[png_ptr]
	test edi,edi
	jz .end_f

if PNG_READ_USER_TRANSFORM_SUPPORTED eq 1
	mov eax,[edi+png_struct.mode]
	and eax,PNG_IS_READ_STRUCT
	jz @f
	mov eax,[edi+png_struct.flags]
	and eax,PNG_FLAG_ROW_INIT
	jz @f ;if (..!=0 && ..!=0)
		cStr ,'info change after png_start_read_image or png_read_update_info'
		stdcall png_app_error, edi, eax
		jmp .end_f
	@@:
end if

	mov eax,[user_transform_ptr]
	mov [edi+png_struct.user_transform_ptr],eax
	mov eax,[user_transform_depth]
	mov [edi+png_struct.user_transform_depth],al
	mov eax,[user_transform_channels]
	mov [edi+png_struct.user_transform_channels],al
.end_f:
	ret
endp

; This function returns a pointer to the user_transform_ptr associated with
; the user transform functions.  The application should free any memory
; associated with this pointer before png_write_destroy and png_read_destroy
; are called.

;voidp (png_structrp png_ptr)
align 4
proc png_get_user_transform_ptr, png_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
		mov eax,[eax+png_struct.user_transform_ptr]
	@@:
	ret
endp

;uint_32 (png_structrp png_ptr)
align 4
proc png_get_current_row_number, png_ptr:dword
	; See the comments in png.inc - this is the sub-image row when reading an
	; interlaced image.

	mov eax,[png_ptr]
	cmp eax,0
	je @f ;if (..!=0)
		mov eax,[eax+png_struct.row_number]
		jmp .end_f
	@@:
	mov eax,PNG_UINT_32_MAX ;help the app not to fail silently
.end_f:
	ret
endp

;byte (png_structrp png_ptr)
align 4
proc png_get_current_pass_number, png_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f ;if (..!=0)
		mov eax,[eax+png_struct.pass]
		jmp .end_f
	@@:
	mov eax,8 ;invalid
.end_f:
	ret
endp
end if
