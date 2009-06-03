;;================================================================================================;;
;;//// gif.asm //// (c) mike.dld, 2007-2008 //////////////////////////////////////////////////////;;
;;================================================================================================;;
;;//// Partial (c) by Willow, Diamond and HidnPlayr //////////////////////////////////////////////;;
;;================================================================================================;;
;;                                                                                                ;;
;; This file is part of Common development libraries (Libs-Dev).                                  ;;
;;                                                                                                ;;
;; Libs-Dev is free software: you can redistribute it and/or modify it under the terms of the GNU ;;
;; Lesser General Public License as published by the Free Software Foundation, either version 2.1 ;;
;; of the License, or (at your option) any later version.                                         ;;
;;                                                                                                ;;
;; Libs-Dev is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without  ;;
;; even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  ;;
;; Lesser General Public License for more details.                                                ;;
;;                                                                                                ;;
;; You should have received a copy of the GNU Lesser General Public License along with Libs-Dev.  ;;
;; If not, see <http://www.gnu.org/licenses/>.                                                    ;;
;;                                                                                                ;;
;;================================================================================================;;
;;                                                                                                ;;
;; References:                                                                                    ;;
;;   1. GIF LITE v3.0 (2004-2007)                                                                 ;;
;;      by Willow and Diamond                                                                     ;;
;;      svn://kolibrios.org/programs/media/gifview/trunk/gif_lite.inc                             ;;
;;   2. "GIF File Format Summary"                                                                 ;;
;;      from "Encyclopedia of Graphics File Formats" by O'Reilly                                  ;;
;;      http://www.fileformat.info/format/gif/                                                    ;;
;;   3. "LZW and GIF explained" (1987)                                                            ;;
;;      by Steve Blackstock, IEEE                                                                 ;;
;;      http://www.cis.udel.edu/~amer/CISC651/lzw.and.gif.explained.html                          ;;
;;   4. "Graphics Interchange Format (tm)" (June 15, 1987)                                        ;;
;;      by CompuServe Incorporated                                                                ;;
;;      http://examples.oreilly.de/english_examples/gff/CDROM/GFF/VENDSPEC/GIF/GIF87A.TXT         ;;
;;   5. "Graphics Interchange Format (sm)" (July 31, 1990)                                        ;;
;;      by CompuServe Incorporated                                                                ;;
;;      http://examples.oreilly.de/english_examples/gff/CDROM/GFF/VENDSPEC/GIF/GIF89A.TXT         ;;
;;                                                                                                ;;
;;================================================================================================;;


include 'gif.inc'

;;================================================================================================;;
proc img.is.gif _data, _length ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in GIF format)                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
	cmp	[_length], sizeof.gif.Header
	jb	.nope
	mov	eax, [_data]
	cmp	dword[eax], 'GIF8'
	jne	.nope
	cmp	word[eax + 4], '7a'
	je	.yep
	cmp	word[eax + 4], '9a'
	je	.yep

  .nope:
	xor	eax, eax
	ret

  .yep:
	xor	eax, eax
	inc	eax
	ret
endp

;;================================================================================================;;
proc img.decode.gif _data, _length, _options ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in GIF format                  ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
  max_color          dd ?
  cur_color_table_size dd ?
  transparent_color  dd ?
  background_color   dd ?
  options_bgr        dd ?
  prev_palette       dd ?
  aux_palette        dd ?
  img		     dd ?
  prev_img_data      dd ?
  aux_img_data       dd ?
  aux_img_type       dd ?
  prev_num_colors    dd ?
  main_img           dd ?
  global_color_table dd ?
  global_color_table_size dd ?
endl

img.decode.gif.main_img equ main_img
img.decode.gif.prev_img_data equ prev_img_data
img.decode.gif.transparent_color equ transparent_color
img.decode.gif.background_color equ background_color
img.decode.gif._length equ _length
img.decode.gif.prev_num_colors equ prev_num_colors
img.decode.gif.prev_palette equ prev_palette
img.decode.gif.max_color equ max_color
img.decode.gif._data equ _data
img.decode.gif.aux_img_data equ aux_img_data
img.decode.gif.aux_img_type equ aux_img_type
img.decode.gif.aux_palette equ aux_palette
img.decode.gif.options_bgr equ options_bgr
; offset of _length parameter for child functions with ebp-based frame
; child saved ebp, return address, 3 saved registers, 14 local variables
img.decode.gif._length_child equ _length + 4 + 4 + 4*3 + 4*14
img.decode.gif.max_color_child equ ebp + 4 + 4 + 4*3
img.decode.gif.cur_color_table_size_child equ ebp + 4 + 4 + 4*3 + 4

	push	ebx esi edi
	xor	eax, eax
	mov	[img], eax
	mov	[main_img], eax
	mov	[prev_img_data], eax
	mov	[aux_img_data], eax
	mov	[aux_img_type], eax
	mov	[prev_palette], eax
	mov	[aux_palette], eax
; when no previous image is available, use background fill with 1-entry palette
	inc	eax
	mov	[prev_num_colors], eax
	lea	eax, [background_color]
	mov	[prev_palette], eax
; value for bgr color in transparent images
	mov	edx, 0xFFFFFF	; white bgr if no value given
	mov	ecx, [_options]
	jecxz	@f
	cmp	[ecx + ImageDecodeOptions.UsedSize], ImageDecodeOptions.BackgroundColor + 4
	jb	@f
	mov	edx, [ecx + ImageDecodeOptions.BackgroundColor]
@@:
	mov	[options_bgr], edx
	mov	dword [eax], edx
; guard against incorrect gif files without any color tables
; "If no color table is available at
; all, the decoder is free to use a system color table or a table of its own. In
; that case, the decoder may use a color table with as many colors as its
; hardware is able to support; it is recommended that such a table have black and
; white as its first two entries, so that monochrome images can be rendered
; adequately." (c) official gif documentation
	mov	[global_color_table], gif_default_palette
	mov	[global_color_table_size], 2

; img.is.gif is called by caller (img.decode)
;	stdcall img.is.gif, [_data], [_length]
;	or	eax, eax
;	jz	.error

	mov	ebx, [_data]
	sub	[_length], sizeof.gif.Header

	mov	cl, [ebx + gif.Header.lsd.Packed]
	add	ebx, sizeof.gif.Header
; gif.LSD.Packed.GlobalColorTableFlag = 80h
;	test	cl, gif.LSD.Packed.GlobalColorTableFlag
;	jz	@f
	test	cl, cl
	jns	@f
	mov	[global_color_table], ebx
	and	cl, gif.LSD.Packed.SizeOfGlobalColorTableMask
;	shr	cl, gif.LSD.Packed.SizeOfGlobalColorTableShift	; here Shift = 0
	push	2
	pop	eax
	shl	eax, cl
	mov	[global_color_table_size], eax
	lea	eax, [eax * 3]
	sub	[_length], eax
	jbe	.error	; there must be at least 1 additional byte after color table
	movzx	ecx, byte [ebx - sizeof.gif.Header + gif.Header.lsd.BackgroundColor]
	lea	ecx, [ecx*3]
	mov	ecx, [ebx + ecx]	; eax = xxBBGGRR, convert to Kolibri color
	bswap	ecx
	shr	ecx, 8
	mov	[background_color], ecx
	add	ebx, eax
    @@:

;   @@: cmp     byte[ebx + gif.Block.Introducer], gif.Block.Introducer.Extension
;       jne     .next_image
;       cmp     byte[ebx + gif.Extension.Label], gif.Extension.Label.Comment
;       jne     .error
;       add     ebx, sizeof.gif.Extension
;       stdcall ._.skip_data
;       mov     ebx, eax
;       jmp     @b

  .next_image:
	stdcall img._.new
	or	eax, eax
	jz	.error
	mov	edx, [img]
	mov	[eax + Image.Previous], edx
	push	sizeof.gif.LogicalScreenDescriptor
	pop	ecx
	test	edx, edx
	jz	@f
	mov	[edx + Image.Next], eax
	xor	ecx, ecx
  @@:
	push	eax
	mov	[eax + Image.Type], Image.bpp8

	add	ecx, sizeof.gif.Image
	invoke	mem.alloc, ecx
	pop	edx
	or	eax, eax
	jz	.error2
	mov	[edx + Image.Extended], eax
	xor	ecx, ecx
	cmp	[img], ecx
	jnz	@f
	mov	esi, [_data]
	add	esi, gif.Header.lsd
	lea	edi, [eax + sizeof.gif.Image]
	mov	cl, sizeof.gif.LogicalScreenDescriptor
	rep	movsb
	mov	[main_img], edx
  @@:
	mov	[img], edx

	stdcall ._.process_extensions

	cmp	al, gif.Block.Introducer.ImageDescriptor
	jne	.error
	sub	[_length], sizeof.gif.ImageDescriptor
	jc	.error
	movzx	eax, [ebx + gif.ImageDescriptor.Width]
	movzx	ecx, [ebx + gif.ImageDescriptor.Height]
	push	edx
	stdcall img._.resize_data, [img], eax, ecx
	pop	edx
	or	eax, eax
	jz	.error

	mov	esi, ebx
	mov	edi, [edx + Image.Extended]
	mov	ecx, sizeof.gif.ImageDescriptor
	rep	movsb

	mov	edi, [edx + Image.Palette]
	mov	esi, [global_color_table]
	mov	ecx, [global_color_table_size]
	test	[ebx + gif.ImageDescriptor.Packed], gif.ID.Packed.LocalColorTableFlag
	jz	@f
	lea	esi, [ebx + sizeof.gif.ImageDescriptor]
	mov	cl, [ebx + gif.ImageDescriptor.Packed]
	and	cl, gif.ID.Packed.SizeOfLocalColorTableMask
; here Shift = 0
;	shr	cl, gif.ID.Packed.SizeOfLocalColorTableShift
	push	2
	pop	eax
	shl	eax, cl
	mov	ecx, eax
	lea	eax, [eax*3]
	add	ebx, eax
	sub	[_length], eax
	jbe	.error	; because we load additional byte, check is 'jbe', not 'jc'
@@:
	mov	[cur_color_table_size], ecx
	dec	[cur_color_table_size]
@@:
	lodsd
	dec	esi
	bswap	eax
	shr	eax, 8
	stosd
	loop	@b
	add	ebx, sizeof.gif.ImageDescriptor
	stdcall ._.process_image
	push	ebx
	mov	edx, [img]
	push	edx
	stdcall	._.superimpose
	pop	edx
	push	edx
	stdcall	._.dispose
	pop	edx
	mov	edx, [edx + Image.Previous]
	test	edx, edx
	jz	.nofreeprev
	mov	ebx, [edx + Image.Extended]
	cmp	[ebx + gif.Image.gce.DelayTime], 0
	jnz	.nofreeprev
	mov	esi, [prev_palette]
	cmp	esi, [edx + Image.Palette]
	jnz	@f
	mov	ecx, [prev_num_colors]
	stdcall	._.alloc_aux_palette
	test	eax, eax
	jz	.nofreeprev
	mov	[prev_palette], eax
    @@:
	mov	esi, [prev_img_data]
	cmp	esi, [edx + Image.Data]
	jnz	.noprevdata
	push	1
	pop	eax
	cmp	[edx + Image.Type], Image.bpp8
	jz	@f
	mov	al, 3
    @@:
	cmp	[aux_img_type], eax
	jb	.resetaux
	mov	edi, [aux_img_data]
	imul	eax, [edx + Image.Width]
	imul	eax, [edx + Image.Height]
	xchg	eax, ecx
	rep	movsb
	jmp	.noprevdata
    .resetaux:
	mov	[aux_img_type], eax
	mov	eax, [aux_img_data]
	test	eax, eax
	jz	@f
	invoke	mem.free, eax
    @@:
	xor	eax, eax
	xchg	eax, [edx + Image.Data]
	mov	[aux_img_data], eax
    .noprevdata:
	cmp	edx, [main_img]
	jnz	@f
	mov	eax, [edx + Image.Next]
	mov	[main_img], eax
	mov	esi, [eax + Image.Extended]
	mov	edi, [edx + Image.Extended]
	mov	[edx + Image.Extended], esi
	mov	[eax + Image.Extended], edi
	push	sizeof.gif.Image
	pop	ecx
	rep	movsb
    @@:
	stdcall	img.destroy.layer, edx
  .nofreeprev:
	pop	ebx
	test	ebx, ebx
	jz	.ret
	jmp	.next_image

  .error2:
	mov	[img], edx

  .error:
	mov	eax, [img]
	test	eax, eax
	jz	.ret
	cmp	[main_img], eax
	jnz	@f
	and	[main_img], 0
  @@:
	stdcall	img.destroy.layer, eax
  .ret:
	mov	eax, [aux_img_data]
	test	eax, eax
	jz	@f
	invoke	mem.free, eax
  @@:
	mov	eax, [aux_palette]
	test	eax, eax
	jz	@f
	invoke	mem.free, eax
  @@:
	mov	eax, [main_img]
	cmp	[eax + Image.Next], 0
	jz	@f
	or	[eax + Image.Flags], Image.IsAnimated
  @@:
	pop	edi esi ebx
	ret
endp

;;================================================================================================;;
proc img.encode.gif _img, _p_length ;/////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in GIF format                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;> _img = pointer to image                                                                        ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to encoded data                                                     ;;
;< _p_length = encoded data length                                                                ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below are private procs you should never call directly from your code                          ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


;;================================================================================================;;
proc img.decode.gif._.process_extensions ;////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> ebx = raw image data                                                                           ;;
;> edx = image data                                                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	mov	esi, ebx
	xor	eax, eax
	mov	[edx + Image.Delay], eax

  .next_block:
	dec	[img.decode.gif._length]
	js	.exit_err
	lodsb	; load gif.Block.Introducer
	cmp	al, gif.Block.Introducer.Extension
	jne	.exit

  .ext_block:
	dec	[img.decode.gif._length]
	js	.exit_err
	lodsb	; load gif.Extension.Label
	cmp	al, gif.Extension.Label.GraphicsControl
	je	.graphics_control_ext
;	cmp	al, gif.Extension.Label.PlainText
;	je	.plain_text_ext
;	cmp	al, gif.Extension.Label.Comment
;	je	.comment_ext
;	cmp	al, gif.Extension.Label.Application
;	je	.application_ext
; skip all other extensions
  .skip_ext:
	dec	[img.decode.gif._length]
	js	.exit_err
	lodsb	; load BlockSize
  .1:
	test	al, al
	jz	.next_block
	sub	[img.decode.gif._length], eax
	jc	.exit_err
	add	esi, eax
	jmp	.skip_ext

  .graphics_control_ext:
	dec	[img.decode.gif._length]
	js	.exit_err
	lodsb	; load BlockSize; must be sizeof.gif.GraphicsControlExtension
	cmp	al, sizeof.gif.GraphicsControlExtension
	jnz	.1
	sub	[img.decode.gif._length], eax
	jc	.exit_err
	push	edi
	movzx	edi, [esi + gif.GraphicsControlExtension.DelayTime]
	mov	[edx + Image.Delay], edi
	mov	edi, [edx + Image.Extended]
	add	edi, gif.Image.gce
	mov	ecx, eax
	rep	movsb
	pop	edi
	jmp	.skip_ext

  .exit_err:
	xor	eax, eax

  .exit:
	mov	ebx, esi
	ret
endp

;;================================================================================================;;
proc img.decode.gif._.process_image ;/////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> ebx = raw image data                                                                           ;;
;> edx = image data                                                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
locals
  width      dd ?
  img_start  dd ?
  img_end    dd ?
  row_end    dd ?
  pass	     dd ?
  codesize   dd ?
  compsize   dd ?
  workarea   dd ?
  block_ofs  dd ?
  bit_count  dd ?
  CC	     dd ?
  EOI	     dd ?
endl

	invoke	mem.alloc, 16 * 1024
	mov	[workarea], eax
	or	eax, eax
	jz	.error

	mov	ecx, [edx + Image.Width]
	mov	[width], ecx
	mov	eax, [edx + Image.Height]
	imul	eax, ecx
	mov	[img_end], eax
	inc	eax
	mov	[row_end], eax
	and	[pass], 0
	and	dword [img.decode.gif.max_color_child], 0
	mov	eax, [edx + Image.Extended]
	test	[eax + gif.Image.info.Packed], gif.ID.Packed.InterleaceFlag
	jz	@f
	mov	[row_end], ecx

    @@: mov	esi, ebx
	mov	edi, [edx + Image.Data]

	sub	dword [img.decode.gif._length_child], 2
	jc	.error
	movzx	ecx, byte[esi]
	inc	esi
	cmp	cl, 12
	jae	.error
	mov	[codesize], ecx
	inc	[codesize]
	xor	eax, eax
	lodsb				; eax - block_count
	sub	[img.decode.gif._length_child], eax
	jc	.error
	add	eax, esi
	push	edi
	mov	edi, [workarea]
	mov	[block_ofs], eax
	mov	[bit_count], 8
	mov	eax, 1
	shl	eax, cl
	mov	[CC], eax
	mov	ecx, eax
	inc	eax
	mov	[EOI], eax
	mov	eax, gif.Null shl 16
  .filltable:
	stosd
	inc	eax
	loop	.filltable
	pop	edi
	mov	[img_start], edi
	add	[img_end], edi
	add	[row_end], edi
  .reinit:
	mov	edx, [EOI]
	inc	edx
	push	[codesize]
	pop	[compsize]
	call	.get_symbol
	cmp	eax, [CC]
	je	.reinit
	call	.output
  .cycle:
	movzx	ebx, ax
	call	.get_symbol
	cmp	eax, [EOI]
	je	.end
	cmp	eax, edx
	ja	.error
	je	.notintable
	cmp	eax, [CC]
	je	.reinit
	call	.output
  .add:
	cmp	edx, 0x00001000
	jae	.cycle
	mov	ecx, [workarea]
	mov	[ecx + edx * 4], ebx
	inc	edx
	cmp	edx, 0x1000
	je	.noinc
	bsr	ebx, edx
	cmp	ebx, [compsize]
	jne	.noinc
	inc	[compsize]
  .noinc:
	jmp	.cycle
  .notintable:
	push	eax
	mov	eax, ebx
	call	.output
	push	ebx
	movzx	eax, bx
	call	.output
	pop	ebx eax
	jmp	.add
  .end:
	mov	edi, [img_end]
	xor	eax, eax

  .exit:
	cmp	[workarea], 0
	je	@f
	invoke	mem.free, [workarea]
    @@:
	mov	ebx, [block_ofs]
    @@:
	dec	[img.decode.gif._length_child]
	js	@f
	movzx	eax, byte [ebx]
	inc	ebx
	test	eax, eax
	jz	.ret
	sub	[img.decode.gif._length_child], eax
	jc	@f
	add	ebx, eax
	jmp	@b

  .error:
	cmp	[workarea], 0
	je	@f
	invoke	mem.free, [workarea]
    @@: xor	ebx, ebx
  .ret:
	ret

;;------------------------------------------------------------------------------------------------;;

img.decode.gif._.process_image.get_symbol:
	mov	ecx, [compsize]
	push	ecx
	xor	eax, eax

  .shift:
	dec	[bit_count]
	jns	.loop1
	inc	esi
	cmp	esi, [block_ofs]
	jb	.noblock
	push	eax
	xor	eax, eax
	sub	[img.decode.gif._length_child], 1
	jc	.error_eof
	lodsb
	test	eax, eax
	jnz	.nextbl
  .error_eof:
	add	esp, 12
	jmp	img.decode.gif._.process_image.error

  .nextbl:
	sub	[img.decode.gif._length_child], eax
	jc	.error_eof
	add	eax, esi
	mov	[block_ofs], eax
	pop	eax

  .noblock:
	mov	[bit_count], 7

  .loop1:
	ror	byte[esi], 1
	rcr	eax,1
	loop	.shift
	pop	ecx
	rol	eax, cl

  .exit:
	xor	ecx, ecx
	retn

;;------------------------------------------------------------------------------------------------;;

img.decode.gif._.process_image.output:
	push	esi eax edx
	mov	edx, [workarea]

  .next:
	pushw	[edx + eax * 4]
	mov	ax, [edx + eax * 4 + 2]
	inc	ecx
	cmp	ax, gif.Null
	jnz	.next
	shl	ebx, 16
	mov	bx, [esp]

  .loop2:
	pop	ax
	cmp	al, byte [img.decode.gif.cur_color_table_size_child]
	jbe	@f	; guard against incorrect GIFs
	mov	al, 0
    @@: cmp	al, byte [img.decode.gif.max_color_child]
	jbe	@f
	mov	[img.decode.gif.max_color_child], al
    @@:	stosb

	cmp	edi, [img_end]
	jz	.done
	cmp	edi, [row_end]
	jb	.norowend
	mov	eax, [width]
	push	eax
	sub	edi, eax
	add	eax, eax
	cmp	[pass], 3
	je	@f
	add	eax, eax
	cmp	[pass], 2
	je	@f
	add	eax, eax
    @@: add	edi, eax
	pop	eax
	cmp	edi, [img_end]
	jb	.nextrow
	mov	edi, [img_start]
	inc	[pass]
	add	edi, eax
	cmp	[pass], 3
	je	@f
	add	edi, eax
	cmp	[pass], 2
	je	@f
	add	edi, eax
	add	edi, eax
    @@:

  .nextrow:
	add	eax, edi
	mov	[row_end], eax
	xor	eax, eax

  .norowend:
	loop	.loop2
	pop	edx eax esi
	retn

  .done:
	lea	esp, [esp+(ecx-1)*2]
	pop	edx eax esi eax
	jmp	img.decode.gif._.process_image.exit

endp

;;================================================================================================;;
proc img.decode.gif._.is_logical_screen ;/////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determines whether GIF image occupies the whole logical screen                                 ;;
;;------------------------------------------------------------------------------------------------;;
;> eax = extended image data                                                                      ;;
;> ebx = main image                                                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< ZF set <=> image area equals logical screen                                                    ;;
;;================================================================================================;;
	mov	ebx, [ebx + Image.Extended]
	cmp	[eax + gif.Image.info.Left], 0
	jnz	@f
	cmp	[eax + gif.Image.info.Top], 0
	jnz	@f
	mov	cx, [eax + gif.Image.info.Width]
	cmp	cx, [ebx + sizeof.gif.Image + gif.LogicalScreenDescriptor.ScreenWidth]
	jnz	@f
	mov	cx, [eax + gif.Image.info.Height]
	cmp	cx, [ebx + sizeof.gif.Image + gif.LogicalScreenDescriptor.ScreenHeight]
@@:	retn
endp

main_img equ img.decode.gif.main_img
transparent_color equ img.decode.gif.transparent_color
background_color equ img.decode.gif.background_color
prev_num_colors equ img.decode.gif.prev_num_colors
prev_palette equ img.decode.gif.prev_palette
max_color equ img.decode.gif.max_color
prev_img_data equ img.decode.gif.prev_img_data
_data equ img.decode.gif._data
aux_img_data equ img.decode.gif.aux_img_data
aux_img_type equ img.decode.gif.aux_img_type
aux_palette equ img.decode.gif.aux_palette

;;================================================================================================;;
proc img.decode.gif._.superimpose ;///////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> edx = image data                                                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	mov	ebx, [main_img]
	mov	eax, [edx + Image.Extended]
	or	[transparent_color], -1		; no transparent color
	test	byte [eax + gif.Image.gce.Packed], 1
	jz	@f
	movzx	ecx, byte [eax + gif.Image.gce.ColorIndex]
	mov	[transparent_color], ecx
	cmp	edx, ebx
	jnz	.has_transparency
	shl	ecx, 2
	add	ecx, [edx + Image.Palette]
	push	eax
	mov	eax, [img.decode.gif.options_bgr]
	mov	dword [background_color], eax
	mov	dword [ecx], eax
	pop	eax
@@:
	call	img.decode.gif._.is_logical_screen
	jnz	.has_transparency
; image is not transparent, so keep it as is
	retn

.has_transparency:
; image has transparent areas, we must superimpose it on the previous
	mov	ecx, [prev_num_colors]
	cmp	ecx, 0x100
	ja	.superimpose_on_rgb
; create common palette
	sub	esp, 3FCh
	push	eax
	mov	edi, esp
	push	ecx
	mov	esi, [prev_palette]
	rep	movsd
	pop	ecx
	mov	esi, [edx + Image.Palette]
	xor	ebx, ebx
	mov	edi, esp
	sub	esp, 100h
.create_palette_loop:
	push	ecx
	lodsd
	cmp	ebx, [transparent_color]
	jz	.nochange
	cmp	ebx, ecx
	jae	@f
	cmp	eax, [edi+ebx*4]
	jz	.nochange
@@:
	push	edi
	repnz	scasd
	pop	edi
	jnz	.increase_palette
	sub	ecx, [esp]
	not	ecx	; cl = index of new color in current palette
	jmp	.palette_common
.increase_palette:
	mov	ecx, [esp]
	test	ch, ch
	jnz	.output_to_rgb
	inc	dword [esp]
	mov	[edi+ecx*4], eax
	jmp	.palette_common
.nochange:
	mov	ecx, ebx
.palette_common:
	mov	[ebx+esp+4], cl
	pop	ecx
	inc	ebx
	cmp	ebx, [max_color]
	jbe	.create_palette_loop
	mov	[max_color], ecx
; if image occupies only part of logical screen, allocate memory for full logical screen
	mov	ebx, [main_img]
	mov	eax, [edx + Image.Extended]
	mov	esi, [edx + Image.Data]
	call	img.decode.gif._.is_logical_screen
	jz	@f
	and	[edx + Image.Data], 0
	push	edx
	movzx	eax, [ebx + sizeof.gif.Image + gif.LogicalScreenDescriptor.ScreenHeight]
	push	eax
	movzx	eax, [ebx + sizeof.gif.Image + gif.LogicalScreenDescriptor.ScreenWidth]
	stdcall	img._.resize_data, edx, eax
	pop	edx
	test	eax, eax
	jz	.palette_nomem
@@:
; copy final palette to Image.Palette
	push	esi esi
	mov	esi, edi
	mov	edi, [edx + Image.Palette]
	mov	ecx, [max_color]
	dec	[max_color]
	rep	movsd
	mov	esi, [prev_img_data]
	mov	edi, [edx + Image.Data]
; do superimpose, [esp] -> source data, esi -> prev image data
;   (NULL if previous image is filled with background color), esp+8 -> correspondence between
;   used palette and final palette, edi -> destination data
	mov	ebx, [edx + Image.Extended]
; first Top rows are copied from [prev_img_data] or filled with bgr
	movzx	ecx, [ebx + gif.Image.info.Top]
	cmp	ecx, [edx + Image.Height]
	jb	@f
	mov	ecx, [edx + Image.Height]
@@:
	push	ecx
	imul	ecx, [edx + Image.Width]
	call	.rep_movsb_or_stosb
	pop	ecx
; convert rows
	sub	ecx, [edx + Image.Height]
	neg	ecx
	push	ecx
	cmp	cx, [ebx + gif.Image.info.Height]
	jbe	@f
	mov	cx, [ebx + gif.Image.info.Height]
@@:
	jecxz	.norows
.convert_rows:
	push	ecx
	movzx	ecx, [ebx + gif.Image.info.Left]
	cmp	ecx, [edx + Image.Width]
	jb	@f
	mov	ecx, [edx + Image.Width]
@@:
	push	ecx
	call	.rep_movsb_or_stosb
	pop	ecx
	sub	ecx, [edx + Image.Width]
	neg	ecx
	push	ecx edx
	mov	edx, [esp+16]	; source data
	cmp	cx, [ebx + gif.Image.info.Width]
	jbe	@f
	mov	cx, [ebx + gif.Image.info.Width]
@@:
	jecxz	.norowsi
.rowsloop:
	movzx	eax, byte [edx]
	inc	edx
	cmp	eax, [transparent_color]
	jz	.rows_transparent
	mov	al, [eax+esp+24]
	stosb
	call	.lodsb
	jmp	@f
.rows_transparent:
	call	.lodsb
	stosb
@@:
	loop	.rowsloop
.norowsi:
	pop	edx ecx
	sub	cx, [ebx + gif.Image.info.Width]
	jbe	@f
	call	.rep_movsb_or_stosb
@@:
	movzx	eax, [ebx + gif.Image.info.Width]
	add	[esp+8], eax
	pop	ecx
	loop	.convert_rows
.norows:
	pop	ecx
	sub	cx, [ebx + gif.Image.info.Height]
	jbe	@f
	imul	ecx, [edx + Image.Width]
	call	.rep_movsb_or_stosb
@@:
; free old image data if we have allocated new copy
	pop	esi esi
	cmp	esi, [edx + Image.Data]
	jz	@f
	invoke	mem.free, esi
@@:
; cleanup stack and return
	add	esp, 500h
	retn
.palette_nomem:
	mov	[edx + Image.Data], esi
	jmp	@b

.output_to_rgb:
	pop	ecx
	add	esp, 500h
; compose two palette-based images to one RGB image
	xor	esi, esi
	xchg	esi, [edx + Image.Data]
	push	esi
	mov	ebx, [_data]
	push	[edx + Image.Palette]
	mov	byte [edx + Image.Type], Image.bpp24
	push	edx
	movzx	eax, [ebx + gif.Header.lsd.ScreenHeight]
	push	eax
	movzx	eax, [ebx + gif.Header.lsd.ScreenWidth]
	stdcall	img._.resize_data, edx, eax
	pop	edx
	test	eax, eax
	jz	.convrgb_nomem
	push	esi
	mov	edi, [edx + Image.Data]
	mov	esi, [prev_img_data]
	mov	ebx, [edx + Image.Extended]
; first Top rows are copied from [prev_img_data] or filled with bgr
	movzx	ecx, [ebx + gif.Image.info.Top]
	cmp	ecx, [edx + Image.Height]
	jb	@f
	mov	ecx, [edx + Image.Height]
@@:
	push	ecx
	imul	ecx, [edx + Image.Width]
	call	.convrgb_prev
	pop	ecx
; convert rows
	sub	ecx, [edx + Image.Height]
	neg	ecx
	push	ecx
	cmp	cx, [ebx + gif.Image.info.Height]
	jbe	@f
	mov	cx, [ebx + gif.Image.info.Height]
@@:
	jecxz	.convrgb_norows
.convrgb_convert_rows:
	push	ecx
	movzx	ecx, [ebx + gif.Image.info.Left]
	cmp	ecx, [edx + Image.Width]
	jb	@f
	mov	ecx, [edx + Image.Width]
@@:
	push	ecx
	call	.convrgb_prev
	pop	ecx
	sub	ecx, [edx + Image.Width]
	neg	ecx
	push	ecx edx
	mov	edx, [esp+16]	; source data
	cmp	cx, [ebx + gif.Image.info.Width]
	jbe	@f
	mov	cx, [ebx + gif.Image.info.Width]
@@:
	jecxz	.convrgb_norowsi
.convrgb_rowsloop:
	movzx	eax, byte [edx]
	inc	edx
	cmp	eax, [transparent_color]
	jz	.convrgb_rows_transparent
	shl	eax, 2
	add	eax, [esp+20]	; source palette
	mov	eax, [eax]
	stosw
	shr	eax, 16
	stosb
	call	.convrgb_lodsb
	jmp	@f
.convrgb_rows_transparent:
	call	.convrgb_lodsb
	stosw
	shr	eax, 16
	stosb
@@:
	loop	.convrgb_rowsloop
.convrgb_norowsi:
	pop	edx ecx
	sub	cx, [ebx + gif.Image.info.Width]
	jbe	@f
	call	.convrgb_prev
@@:
	movzx	eax, [ebx + gif.Image.info.Width]
	add	[esp+8], eax
	pop	ecx
	loop	.convrgb_convert_rows
.convrgb_norows:
	pop	ecx
	sub	cx, [ebx + gif.Image.info.Height]
	jbe	@f
	imul	ecx, [edx + Image.Width]
	call	.convrgb_prev
@@:
; free old image data
	pop	esi esi ;esi
	invoke	mem.free;, esi
	retn
.convrgb_nomem:
	pop	esi esi
	retn

.superimpose_on_rgb:
; previous image is RGB, new image has transparent areas
	xor	esi, esi
	xchg	esi, [edx + Image.Data]
	push	esi
	mov	ebx, [_data]
	push	[edx + Image.Palette]
	mov	byte [edx + Image.Type], Image.bpp24
	push	edx
	movzx	eax, [ebx + gif.Header.lsd.ScreenHeight]
	push	eax
	movzx	eax, [ebx + gif.Header.lsd.ScreenWidth]
	stdcall	img._.resize_data, edx, eax
	pop	edx
	test	eax, eax
	jz	.rgb_nomem
	push	esi
	mov	edi, [edx + Image.Data]
	mov	esi, [prev_img_data]
	mov	ebx, [edx + Image.Extended]
; first Top rows are copied from [prev_img_data] or filled with bgr
	movzx	ecx, [ebx + gif.Image.info.Top]
	cmp	ecx, [edx + Image.Height]
	jb	@f
	mov	ecx, [edx + Image.Height]
@@:
	push	ecx
	lea	ecx, [ecx*3]
	imul	ecx, [edx + Image.Width]
	rep	movsb
	pop	ecx
; convert rows
	sub	ecx, [edx + Image.Height]
	neg	ecx
	push	ecx
	cmp	cx, [ebx + gif.Image.info.Height]
	jbe	@f
	mov	cx, [ebx + gif.Image.info.Height]
@@:
	jecxz	.rgb_norows
.rgb_convert_rows:
	push	ecx
	movzx	ecx, [ebx + gif.Image.info.Left]
	cmp	ecx, [edx + Image.Width]
	jb	@f
	mov	ecx, [edx + Image.Width]
@@:
	push	ecx
	lea	ecx, [ecx*3]
	rep	movsb
	pop	ecx
	sub	ecx, [edx + Image.Width]
	neg	ecx
	push	ecx edx
	mov	edx, [esp+16]	; source data
	cmp	cx, [ebx + gif.Image.info.Width]
	jbe	@f
	mov	cx, [ebx + gif.Image.info.Width]
@@:
	jecxz	.rgb_norowsi
.rgb_rowsloop:
	movzx	eax, byte [edx]
	inc	edx
	cmp	eax, [transparent_color]
	jz	.rgb_rows_transparent
	shl	eax, 2
	add	eax, [esp+20]	; source palette
	mov	eax, [eax]
	stosw
	shr	eax, 16
	stosb
	add	esi, 3
	jmp	@f
.rgb_rows_transparent:
	movsb
	movsb
	movsb
@@:
	loop	.rgb_rowsloop
.rgb_norowsi:
	pop	edx ecx
	sub	cx, [ebx + gif.Image.info.Width]
	jbe	@f
	lea	ecx, [ecx*3]
	rep	movsb
@@:
	movzx	eax, [ebx + gif.Image.info.Width]
	add	[esp+8], eax
	pop	ecx
	loop	.rgb_convert_rows
.rgb_norows:
	pop	ecx
	sub	cx, [ebx + gif.Image.info.Height]
	jbe	@f
	imul	ecx, [edx + Image.Width]
	lea	ecx, [ecx*3]
	rep	movsb
@@:
; free old image data
	pop	esi esi ;esi
	invoke	mem.free;, esi
	retn
.rgb_nomem:
	pop	esi esi
	retn

.lodsb:
	xor	eax, eax
	test	esi, esi
	jz	@f
	lodsb
@@:	retn

.rep_movsb_or_stosb:
	test	esi, esi
	jz	.rmos1
	rep	movsb
	jmp	.rmos2
.rmos1:	xor	eax, eax	; background index in final palette is 0 in bgr mode
	rep	stosb
.rmos2:	retn

.convrgb_prev:
	jecxz	.convrgb_noprev
	test	esi, esi
	jz	.convrgb_prev_bgr
@@:
	xor	eax, eax
	lodsb
	shl	eax, 2
	add	eax, [prev_palette]
	mov	eax, [eax]
	stosw
	shr	eax, 16
	stosb
	loop	@b
	retn
.convrgb_prev_bgr:
@@:
	mov	eax, [background_color]
	stosw
	shr	eax, 16
	stosb
	loop	@b
.convrgb_noprev:
	retn
.convrgb_lodsb:
	xor	eax, eax
	test	esi, esi
	jz	@f
	lodsb
	shl	eax, 2
	add	eax, [prev_palette]
	mov	eax, [eax]
	retn
@@:	mov	eax, [background_color]
	retn

endp

;;================================================================================================;;
proc img.decode.gif._.dispose ;///////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> edx = image data                                                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	mov	ebx, [edx + Image.Extended]
	mov	al, [ebx + gif.Image.gce.Packed]
	shr	al, 2
	and	al, 7
	cmp	al, 2
	jz	.background
	cmp	al, 3
	jz	.previous
; don't dispose - set prev_img and related vars to current image
	mov	eax, [edx + Image.Data]
	mov	[prev_img_data], eax
	cmp	[edx + Image.Type], Image.bpp8
	jnz	@f
	mov	eax, [max_color]
	inc	eax
	mov	[prev_num_colors], eax
	mov	eax, [edx + Image.Palette]
	mov	[prev_palette], eax
	retn
@@:
	or	[prev_num_colors], -1
	and	[prev_palette], 0
.previous:
	retn
.background:
	cmp	[prev_img_data], 0
	jz	.bgr_full
	mov	ebx, [main_img]
	mov	eax, [edx + Image.Extended]
	call	img.decode.gif._.is_logical_screen
	jnz	@f
.bgr_full:
	xor	eax, eax
	mov	[prev_img_data], eax
	inc	eax
	mov	[prev_num_colors], eax
	lea	eax, [background_color]
	mov	[prev_palette], eax
	retn
@@:
	cmp	[prev_num_colors], 0x100
	ja	.rgb
	mov	eax, [background_color]
	mov	edi, [prev_palette]
	mov	ecx, [prev_num_colors]
	repnz	scasd
	jz	.palette_ok
	cmp	[prev_num_colors], 0x100
	jz	.convert_rgb
	push	1
	pop	eax
	stdcall	img.decode.gif._.alloc_aux_img
	test	eax, eax
	jz	.previous
	mov	ecx, [prev_num_colors]
	mov	esi, [prev_palette]
	call	img.decode.gif._.alloc_aux_palette
	test	eax, eax
	jz	.previous
	mov	[prev_palette], eax
	mov	eax, [background_color]
	stosd
	mov	eax, [prev_num_colors]	; eax = index of background color
	inc	[prev_num_colors]
	jmp	.bpp8_common
.palette_ok:
	push	1
	pop	eax
	stdcall	img.decode.gif._.alloc_aux_img
	test	eax, eax
	jz	.previous
	sub	edi, [prev_palette]
	shr	edi, 2
	lea	eax, [edi-1]	; eax = index of background color
.bpp8_common:
	push	eax
	mov	ebx, [_data]
	mov	esi, [prev_img_data]
	mov	edi, [aux_img_data]
	mov	[prev_img_data], edi
	cmp	esi, edi
	jz	@f
	movzx	ecx, [ebx + gif.Header.lsd.ScreenWidth]
	movzx	eax, [ebx + gif.Header.lsd.ScreenHeight]
	imul	ecx, eax
	push	edi
	rep	movsb
	pop	edi
@@:
	movzx	esi, [ebx + gif.Header.lsd.ScreenHeight]
	movzx	eax, [ebx + gif.Header.lsd.ScreenWidth]
	mov	edx, [edx + Image.Extended]
	movzx	ecx, [edx + gif.Image.info.Top]
	sub	esi, ecx
	jbe	.bpp8_ret
	imul	ecx, eax
	add	edi, ecx
	cmp	si, [edx + gif.Image.info.Height]
	jb	@f
	mov	si, [edx + gif.Image.info.Height]
@@:
	movzx	ecx, [edx + gif.Image.info.Left]
	sub	eax, ecx
	jbe	.bpp8_ret
	add	edi, ecx
	cmp	ax, [edx + gif.Image.info.Width]
	jb	@f
	mov	ax, [edx + gif.Image.info.Width]
@@:
	xchg	eax, ecx
	movzx	edx, [ebx + gif.Header.lsd.ScreenWidth]
	sub	edx, ecx
	pop	eax
@@:
	push	ecx
	rep	stosb
	pop	ecx
	add	edi, edx
	dec	esi
	jnz	@b
	push	eax
.bpp8_ret:
	pop	eax
	retn
.convert_rgb:
	push	3
	pop	eax
	stdcall	img.decode.gif._.alloc_aux_img
	test	eax, eax
	jz	.previous
	or	[prev_num_colors], -1
	mov	ebx, [_data]
	mov	esi, [prev_img_data]
	mov	edi, [aux_img_data]
	mov	[prev_img_data], edi
	movzx	ecx, [ebx + gif.Header.lsd.ScreenWidth]
	movzx	eax, [ebx + gif.Header.lsd.ScreenHeight]
	imul	ecx, eax
	push	edx
	xor	edx, edx
	xchg	edx, [prev_palette]
	add	edi, ecx
	add	esi, ecx
	add	edi, ecx
	add	edi, ecx
@@:
	dec	esi
	movzx	eax, byte [esi]
	mov	eax, [eax*4+edx]
	sub	edi, 3
	mov	[edi], ax
	shr	eax, 16
	mov	[edi+2], al
	loop	@b
	pop	edx
	movzx	esi, [ebx + gif.Header.lsd.ScreenHeight]
	movzx	eax, [ebx + gif.Header.lsd.ScreenWidth]
	mov	edx, [edx + Image.Extended]
	movzx	ecx, [edx + gif.Image.info.Top]
	sub	esi, ecx
	jbe	.convert_rgb_ret
	imul	ecx, eax
	lea	ecx, [ecx*3]
	add	edi, ecx
	cmp	si, [edx + gif.Image.info.Height]
	jb	@f
	mov	si, [edx + gif.Image.info.Height]
@@:
	movzx	ecx, [edx + gif.Image.info.Left]
	sub	eax, ecx
	jbe	.convert_rgb_ret
	lea	ecx, [ecx*3]
	add	edi, ecx
	cmp	ax, [edx + gif.Image.info.Width]
	jb	@f
	mov	ax, [edx + gif.Image.info.Width]
@@:
	xchg	eax, ecx
	movzx	edx, [ebx + gif.Header.lsd.ScreenWidth]
	sub	edx, ecx
	mov	eax, [background_color]
	lea	edx, [edx*3]
.convert_rgb_loop:
	push	ecx
@@:
	stosw
	shr	eax, 16
	stosb
	loop	@b
	pop	ecx
	add	edi, edx
	dec	esi
	jnz	.convert_rgb_loop
.convert_rgb_ret:
	retn
.rgb:
	push	3
	pop	eax
	stdcall	img.decode.gif._.alloc_aux_img
	test	eax, eax
	jz	.previous
	or	[prev_num_colors], -1
	and	[prev_palette], 0
	mov	ebx, [_data]
	mov	esi, [prev_img_data]
	mov	edi, [aux_img_data]
	mov	[prev_img_data], edi
	cmp	esi, edi
	jz	@f
	movzx	ecx, [ebx + gif.Header.lsd.ScreenHeight]
	push	ecx
	movzx	eax, [ebx + gif.Header.lsd.ScreenWidth]
	imul	ecx, eax
	lea	ecx, [ecx*3]
	push	edi
	rep	movsb
	pop	edi
	pop	esi
	mov	edx, [edx + Image.Extended]
	movzx	ecx, [edx + gif.Image.info.Top]
	sub	esi, ecx
	jbe	.rgb_ret
	imul	ecx, eax
	lea	ecx, [ecx*3]
	add	edi, ecx
	cmp	si, [edx + gif.Image.info.Height]
	jb	@f
	mov	si, [edx + gif.Image.info.Height]
@@:
	movzx	ecx, [edx + gif.Image.info.Left]
	sub	eax, ecx
	jbe	.rgb_ret
	lea	ecx, [ecx*3]
	add	edi, ecx
	cmp	ax, [edx + gif.Image.info.Width]
	jb	@f
	mov	ax, [edx + gif.Image.info.Width]
@@:
	xchg	eax, ecx
	movzx	edx, [ebx + gif.Header.lsd.ScreenWidth]
	sub	edx, ecx
	mov	eax, [background_color]
	lea	edx, [edx*3]
.rgb_loop:
	push	ecx
@@:
	stosw
	shr	eax, 16
	stosb
	loop	@b
	pop	ecx
	add	edi, edx
	dec	esi
	jnz	.rgb_loop
.rgb_ret:
	retn

endp

;;================================================================================================;;
proc img.decode.gif._.alloc_aux_img ;/////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Allocate auxiliary memory for previous image                                                   ;;
;;------------------------------------------------------------------------------------------------;;
;> eax = image type: 1 = bpp8, 3 = bpp24                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = [aux_img_data]                                                                           ;;
;;================================================================================================;;
	cmp	[aux_img_type], eax
	jae	@f
	push	edx eax
	movzx	ecx, [ebx + sizeof.gif.Image + gif.LogicalScreenDescriptor.ScreenWidth]
	mul	ecx
	movzx	ecx, [ebx + sizeof.gif.Image + gif.LogicalScreenDescriptor.ScreenHeight]
	mul	ecx
	invoke	mem.realloc, [aux_img_data], eax
	pop	ecx edx
	test	eax, eax
	jz	@f
	mov	[aux_img_type], ecx
	mov	[aux_img_data], eax
@@:	retn

endp

;;================================================================================================;;
proc img.decode.gif._.alloc_aux_palette ;/////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Allocate and fill aux_palette                                                                  ;;
;;------------------------------------------------------------------------------------------------;;
;> esi -> palette, ecx = palette size                                                             ;;
;;------------------------------------------------------------------------------------------------;;
;< [aux_palette] set                                                                              ;;
;;================================================================================================;;
	mov	eax, [aux_palette]
	test	eax, eax
	jnz	@f
	push	edx ecx
	invoke	mem.alloc, 0x400
	pop	ecx edx
	test	eax, eax
	jz	.ret
	mov	[aux_palette], eax
@@:
	mov	edi, eax
	rep	movsd
.ret:
	retn

endp

restore main_img
restore transparent_color
restore background_color
restore prev_num_colors
restore prev_palette
restore max_color
restore prev_img_data
restore _data
restore aux_img_data
restore aux_img_type
restore aux_palette

;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


;
