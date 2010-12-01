; Kolibri kernel packer
; (C) copyright diamond 2006, 2007
;
; Disassemled and corrected in 2010 specially for FASM
;            by Marat Zakiyanov aka Mario79, aka Mario
;
;	This program is free software; you can redistribute it and/or modify
;	it under the terms of the GNU General Public License as published by
;	the Free Software Foundation; either version 2 of the License, or
;	(at your option) any later version.
;
;	This program is distributed in the hope that it will be useful,
;	but WITHOUT ANY WARRANTY; without even the implied warranty of
;	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;	GNU General Public License for more details.

; Uses LZMA compression library by Igor Pavlov
; (for more information on LZMA and 7-Zip visit http://www.7-zip.org)
; (plain-C packer is ported by diamond)

;---------------------------------------------------------------------
use32
	org	0

	db 'MENUET01'
	dd 1
	dd START
	dd IM_END
	dd I_END
	dd stacktop
	dd 0	;params
	dd 0	;cur_dir_path
;---------------------------------------------------------------------
include '../../../macros.inc'
; do not touch "purge mov"!!!
purge mov ; for the correct patch of loader
; do not touch "purge mov"!!!
;*********************************************************************
die_with_err:
	pop	esi
@@:
	lodsb
	test	al,al
	jz	@f

	mov	cl,al
	mcall	63,1
	jmp	@b
;*********************************************************************
@@:
	mcall	63,,13
	mcall	,,10
	mcall	-1
;*********************************************************************
START:
	mcall	70,fn70_read
	cmp	eax,6
	jz	read_ok
;--------------------------------------
read_err:
	call	die_with_err
	db	'KerPack: cannot load kernel.mnt',0
;*********************************************************************
read_ok:
	push	18
	call	lzma_set_dict_size
; find jump to 32-bit code
	mov	edi,infile - 1
;--------------------------------------
@@:
	inc	edi
	cmp	dword [edi],0xE88EE08E	 ; mov fs,ax/mov gs,ax
	jnz	@b

	cmp	dword [edi+4],0x00BCD08E	; mov ss,ax/mov esp,00xxxxxx
	jnz	@b

	add	edi,11
	mov	[inptr],edi
	sub	edi, infile
	mov	[indelta],edi
	lea	eax,[ebx+0x10000]
	mov	dword [loader_patch3+2],eax
	sub	ebx,edi
	mov	[insize],ebx
	call	preprocess_calltrick2

	mov	al,[cti]
	mov	[loader_patch5-1],al
	mov	eax,[ctn]
	mov	[loader_patch4+1],eax
	mov	eax,[inptr]
	add	eax, outfile - infile + loader_size - 5
	push	workmem
	push	[insize]
	push	eax
	push	[inptr]
	call	lzma_compress

	add	eax, loader_size-5
	mov	[loader_patch1+6],eax
	add	eax,[indelta]
	mov	[outsize],eax
	mov	eax,[indelta]
	mov	ecx,dword [eax + outfile + loader_size - 4]
	bswap	ecx
	mov	[loader_patch2+4],ecx
	add	eax, 0x10000
	mov	[loader_patch1+1],eax
	mov	esi,infile
	mov	edi,outfile
	mov	ecx,[indelta]
	rep	movsb

	mov	esi,loader_start
	mov	ecx,loader_size
	rep	movsb

	mcall	70,fn70_write
	test	eax,eax
	jz	@f

	call	die_with_err
	db	'KerPack: cannot save kernel.mnt',0
;*********************************************************************
@@:
	call	die_with_err
	db	'KerPack: all is OK',0
;*********************************************************************
preprocess_calltrick2:
; input preprocessing
	mov	edi,ct1
	xor	eax,eax
	push	edi
	mov	ecx,256/4
	rep	stosd

	pop	edi
	mov	ecx,ebx
	mov	esi,[inptr]
	mov	ebx,inbuftmp
	xchg	eax,edx
;--------------------------------------
input_pre2:
	lodsb
;--------------------------------------
@@:
	cmp	al,0Fh
	jnz	ip1

	dec	ecx
	jz	input_pre_done2

	lodsb
	cmp	al,80h
	jb	@b

	cmp	al,90h
	jb	@f
;--------------------------------------
ip1:
	sub	al,0E8h
	cmp	al,1
	ja	input_pre_cont2
;--------------------------------------
@@:
	cmp	ecx,5
	jb	input_pre_done2

	lodsd
	add	eax,esi
	sub	eax,[inptr]
	cmp	eax,[insize]
	jae	xxx2

	cmp	eax,1000000h
	jae	xxx2

	sub	ecx,4
	xchg	al,ah
	rol	eax,16
	xchg	al,ah
	mov	[esi-4],eax
	inc	edx
	mov	[ebx],esi
	add	ebx,4
	jmp	input_pre_cont2
;*********************************************************************
xxx2:
	sub	esi,4
	movzx	eax,byte [esi]
	mov	byte [eax+edi],1
;--------------------------------------
input_pre_cont2:
	loop	input_pre2
;--------------------------------------
input_pre_done2:
	mov	[ctn],edx
	xor	eax,eax
	mov	ecx,256
	repnz	scasb
	jnz	pack_calltrick_done

	not	cl
	mov	[cti],cl
;--------------------------------------
@@:
	cmp	ebx,inbuftmp
	jz	pack_calltrick_done

	sub	ebx,4
	mov	eax,[ebx]
	mov	[eax-4],cl
	jmp	@b
;*********************************************************************
pack_calltrick_done:
	ret
;*********************************************************************
;lzma_compress:
include 'lzma_compress.inc'
;---------------------------------------------------------------------
;lzma_set_dict_size:
include 'lzma_set_dict_size.inc'
;---------------------------------------------------------------------
;
include 'loader_lzma.inc'
;*********************************************************************
fn70_read:
	dd	0
	dd	0
	dd	0
	dd	200*1024
	dd	infile
filename db	'/rd/1/kernel.mnt',0

fn70_write:
	dd	2
	dd	0
	dd	0
outsize dd	?
	dd	outfile
	db	0
	dd	filename
;---------------------------------------------------------------------
align 4
LiteralNextStates:
db 0,0,0,0,1,2,3,4,5,6,4,5
MatchNextStates:
db 7,7,7,7,7,7,7,10,10,10,10,10
RepNextStates:
db 8,8,8,8,8,8,8,11,11,11,11,11
ShortRepNextStates:
db 9,9,9,9,9,9,9,11,11,11,11,11
;---------------------------------------------------------------------
;*********************************************************************
IM_END:
;*********************************************************************
;params:
;	rb 256
;---------------------------------------------------------------------
;cur_dir_path:
;	rb 4096
;---------------------------------------------------------------------
align 4
	rb 4096
stacktop:
;---------------------------------------------------------------------
align	4
inptr		dd ?
indelta		dd ?
insize		dd ?
ct1		rb 256
ctn		dd ?
cti		db ?

infile		rb 200*1024
inbuftmp	rb 200*1024
outfile		rb 200*1024
workmem		rb 6A8000h
;---------------------------------------------------------------------
; Compress data area start
;---------------------------------------------------------------------
align 4
_lenEncoder:
	rd 8451
;-----------------------------------------------------
_prices:
	rd 4384
	rd 17
;-----------------------------------------------------
_finished:		rb 1
_writeEndMark:		rb 1
_longestMatchWasFound:	rb 1
_previousByte:		rb 1
_longestMatchLength:	rd 1
;-----------------------------------------------------
g_FastPos:
	rb 1024
;-----------------------------------------------------
_posSlotPrices:
	rd 256
;-----------------------------------------------------
_isRep0Long:
	rd 192
;-----------------------------------------------------
distances:
	rd 274
;-----------------------------------------------------
_optimumCurrentIndex:	rd 1
_additionalOffset:	rd 1
;-----------------------------------------------------
_isRepG1:
	rd 12
;-----------------------------------------------------
_isMatch:
	rd 192
;-----------------------------------------------------
_alignPriceCount:	rd 1
_numLiteralContextBits:	rd 1
;-----------------------------------------------------
_literalEncoder:
	rd 114
;-----------------------------------------------------
nowPos64:
	rd 2
;-----------------------------------------------------
_distancesPrices:
	rd 512
;-----------------------------------------------------
_repDistances:
	rd 4
;-----------------------------------------------------
_posSlotEncoder:
	rd 1028
;-----------------------------------------------------
lastPosSlotFillingPos:
	rd 2
;-----------------------------------------------------
_numFastBytes:	rd 1
_posStateMask:	rd 1
;-----------------------------------------------------
_isRepG0:
	rd 12
;-----------------------------------------------------
_repMatchLenEncoder:
	rd 8451
	rd 4384
	rd 17
;-----------------------------------------------------
_isRepG2:
	rd 12
;-----------------------------------------------------
_dictionarySize:		rd 1
_numLiteralPosStateBits:	rd 1
_distTableSize:			rd 1
_optimumEndIndex:		rd 1
;-----------------------------------------------------
;static CState state
state.State:		rb 1
state.Prev1IsChar:	rb 1
state.Prev2:		rb 2
state.PosPrev2:		rd 1
state.BackPrev2:	rd 1
state.Price:		rd 1
state.PosPrev:		rd 1
state.BackPrev:		rd 1
state.Backs:
	rd 4
;----------------------------------------------------
	rd 40950
;-----------------------------------------------------
_alignPrices:
	rd 16
;-----------------------------------------------------
_isRep:
	rd 12
;-----------------------------------------------------
_posAlignEncoder:
	rd 256
;-----------------------------------------------------
i_01:	rd 1
;-----------------------------------------------------
_state:			rb 1
_cache:			rb 1
_state.Prev2:		rb 2
_posEncoders:		rd 1
_numPrevBits:		rd 1
_numPosBits:		rd 1
_posMask:		rd 1
_posStateBits:		rd 1
_range:			rd 1
_cacheSize:		rd 1
_cyclicBufferSize:	rd 1
;-----------------------------------------------------
low:
	rd 2
;-----------------------------------------------------
Models:
	rd 512
;-----------------------------------------------------
_matchMaxLen:	rd 1
pack_pos:	rd 1
_cutValue:	rd 1
_hash:		rd 1
;-----------------------------------------------------
crc_table:
	rd 256
;-----------------------------------------------------
_buffer:	rd 1
_pos:		rd 1
_streamPos:	rd 1
pack_length:	rd 1
;---------------------------------------------------------------------
; Compress data area end
;---------------------------------------------------------------------
I_END:
;---------------------------------------------------------------------