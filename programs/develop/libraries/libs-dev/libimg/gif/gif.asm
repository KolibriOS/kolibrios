;;================================================================================================;;
;;//// gif.asm //// (c) mike.dld, 2007-2008 //////////////////////////////////////////////////////;;
;;================================================================================================;;
;;//// Partial (c) by Willow, Diamond and HidnPlayr //////////////////////////////////////////////;;
;;================================================================================================;;
;;                                                                                                ;;
;; This file is part of Common development libraries (Libs-Dev).                                  ;;
;;                                                                                                ;;
;; Libs-Dev is free software: you can redistribute it and/or modify it under the terms of the GNU ;;
;; General Public License as published by the Free Software Foundation, either version 3 of the   ;;
;; License, or (at your option) any later version.                                                ;;
;;                                                                                                ;;
;; Libs-Dev is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without  ;;
;; even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  ;;
;; General Public License for more details.                                                       ;;
;;                                                                                                ;;
;; You should have received a copy of the GNU General Public License along with Libs-Dev. If not, ;;
;; see <http://www.gnu.org/licenses/>.                                                            ;;
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
	cmp	[_length], 6
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
proc img.decode.gif _data, _length ;//////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in GIF format                  ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
  img		     dd ?
  global_color_table dd ?
endl

	push	ebx

	stdcall img.is.gif, [_data], [_length]
	or	eax, eax
	jz	.error

	mov	ebx, [_data]
;       cmp     [ebx + bmp.Header.info.Compression], bmp.BI_RGB
;       je      @f
;       mov     eax, [ebx + bmp.Header.file.Size]
;       cmp     eax, [_length]
;       jne     .error

	test	[ebx + gif.Header.lsd.Packed], gif.LSD.Packed.GlobalColorTableFlag
	jz	@f
	lea	eax, [ebx + sizeof.gif.Header]
	mov	[global_color_table], eax
	mov	cl, [ebx + gif.Header.lsd.Packed]
	and	cl, gif.LSD.Packed.SizeOfGlobalColorTableMask
	shr	cl, gif.LSD.Packed.SizeOfGlobalColorTableShift
	inc	cl
	mov	eax, 1
	shl	eax, cl
	lea	eax, [eax * 3]
	add	ebx, eax
    @@: add	ebx, sizeof.gif.Header

	mov	[img], 0

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
	mov	[img], eax
	mov	edx, eax

	mov	ecx, sizeof.gif.Image
	invoke	mem.alloc, ecx
	or	eax, eax
	jz	.error
	mov	[edx + Image.Extended], eax

	stdcall ._.process_extensions

	cmp	byte[ebx + gif.Block.Introducer], gif.Block.Introducer.ImageDescriptor
	jne	.error
	movzx	eax, [ebx + gif.ImageDescriptor.Width]
	movzx	ecx, [ebx + gif.ImageDescriptor.Height]
	stdcall img._resize_data, [img], eax, ecx
	or	eax, eax
	jz	.error

	test	[ebx + gif.ImageDescriptor.Packed], gif.ID.Packed.LocalColorTableFlag
	jz	@f
	mov	cl, [ebx + gif.ImageDescriptor.Packed]
	and	cl, gif.ID.Packed.SizeOfLocalColorTableMask
	shr	cl, gif.ID.Packed.SizeOfLocalColorTableShift
	inc	cl
	mov	eax, 1
	shl	eax, cl
	lea	ecx, [eax * sizeof.gif.RgbTriplet]
	lea	eax, [ecx + sizeof.gif.Image]
	invoke	mem.realloc, [edx + Image.Extended], eax
	or	eax, eax
	jz	.error
	mov	[edx + Image.Extended], eax
	lea	esi, [ebx + sizeof.gif.ImageDescriptor]
	lea	edi, [eax + sizeof.gif.Image]
	rep	movsb

    @@: mov	eax, [global_color_table]
	test	[ebx + gif.ImageDescriptor.Packed], gif.ID.Packed.LocalColorTableFlag
	jz	@f
	lea	eax, [ebx + sizeof.gif.ImageDescriptor]
    @@: mov	ebx, esi
	stdcall ._.process_image, eax

  .decoded:
	or	eax, eax
	jz	@f
	stdcall img.destroy, [img]
	jmp	.error
	
    @@: mov	eax, [img]
	ret

  .error:
	xor	eax, eax
	pop	ebx
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
proc img.decode.gif._.skip_data ;/////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> ebx = pointer to data blocks array                                                             ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = pointer to data right after data blocks array                                            ;;
;;================================================================================================;;
	push	ecx
	xor	ecx, ecx
    @@: mov	cl, [esi]
	or	cl, cl
	jz	@f
	lea	esi, [esi + ecx + 1]
	jmp	@b
    @@: pop	ecx
	ret
endp

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
	push	edx
	mov	esi, ebx

  .next_block:
	mov	al, [esi + gif.Block.Introducer]
	cmp	al, gif.Block.Introducer.Extension
	je	.ext_block
;       cmp     al, gif.Block.Introducer.ImageDescriptor
;       je      .exit
;       cmp     al, gif.Block.Introducer.EndOfFile
;       je      .exit
	jmp	.exit

  .ext_block:
	mov	al, [esi + gif.Extension.Label]
	cmp	al, gif.Extension.Label.PlainText
	je	.plain_text_ext
	cmp	al, gif.Extension.Label.GraphicsControl
	je	.graphics_control_ext
	cmp	al, gif.Extension.Label.Comment
	je	.comment_ext
	cmp	al, gif.Extension.Label.Application
	je	.application_ext
	jmp	.exit

  .plain_text_ext:
	add	esi, gif.PlainTextExtension.PlainTextData
	stdcall img.decode.gif._.skip_data
	jmp	.next_ext_block

  .graphics_control_ext:
	push	edi
	mov	edi, [edx + Image.Extended]
	add	edi, gif.Image.gce
	mov	ecx, sizeof.gif.GraphicsControlExtension
	rep	movsb
	pop	edi
	jmp	.next_ext_block

  .comment_ext:
	add	esi, gif.CommentExtension.CommentData
	stdcall img.decode.gif._.skip_data
	jmp	.next_ext_block

  .application_ext:
	add	esi, gif.ApplicationExtension.ApplicationData
	stdcall img.decode.gif._.skip_data
	jmp	.next_ext_block

  .next_ext_block:
	mov	al, [ebx + gif.Block.Introducer]
	cmp	al, gif.Block.Introducer.EndOfData
	jne	.exit
	inc	ebx
	jmp	.next_block

  .exit:
	mov	ebx, esi
	pop	edx
	ret
endp

;;================================================================================================;;
proc img.decode.gif._.process_image _color_table ;////////////////////////////////////////////////;;
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
;       lea     eax, [eax * 3]
	shl	eax, 2
	mov	[img_end], eax
	inc	eax
	mov	[row_end], eax
	and	[pass], 0
	mov	eax, [edx + Image.Extended]
	test	[eax + gif.Image.info.Packed], gif.ID.Packed.InterleaceFlag
	jz	@f
;       lea     ecx, [ecx * 3]
	shl	ecx, 2
	mov	[row_end], ecx

    @@: mov	esi, ebx
	mov	edi, [edx + Image.Data]

	push	edi
	movzx	ecx, byte[esi]
	inc	esi
	mov	[codesize], ecx
	inc	[codesize]
	mov	edi, [workarea]
	xor	eax, eax
	lodsb				; eax - block_count
	add	eax, esi
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
	cmp	eax, edx
	jae	.notintable
	cmp	eax, [CC]
	je	.reinit
	cmp	eax, [EOI]
	je	.end
	call	.output
  .add:
	mov	ecx, [workarea]
	mov	[ecx + edx * 4], ebx
	cmp	edx, 0x00000FFF
	jae	.cycle
	inc	edx
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
    @@: xor	eax, eax
	ret

  .error:
	cmp	[workarea], 0
	je	@f
	invoke	mem.free, [workarea]
    @@: xor	eax, eax
	inc	eax
	ret

;;------------------------------------------------------------------------------------------------;;

img.decode.gif._.process_image.get_symbol:
	mov	ecx, [compsize]
	push	ecx
	xor	eax, eax

  .shift:
	ror	byte[esi], 1
	rcr	eax,1
	dec	[bit_count]
	jnz	.loop1
	inc	esi
	cmp	esi, [block_ofs]
	jb	.noblock
	push	eax
	xor	eax, eax
	lodsb
	test	eax, eax
	jnz	.nextbl
	mov	eax, [EOI]
	sub	esi, 2
	add	esp, 8
	jmp	.exit

  .nextbl:
	add	eax, esi
	mov	[block_ofs], eax
	pop	eax

  .noblock:
	mov	[bit_count], 8

  .loop1:
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

	lea	esi, [eax * 3]
	add	esi, [_color_table]

	mov	esi, [esi]
	bswap	esi
	shr	esi, 8
	mov	[edi], esi
	add	edi, 4

	cmp	edi, [row_end]
	jb	.norowend
	mov	eax, [width]
;       lea     eax, [eax * 3]
	shl	eax, 2
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

endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


;
