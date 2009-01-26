;;================================================================================================;;
;;//// libio_p.asm //// (c) mike.dld, 2006-2008 //////////////////////////////////////////////////;;
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

mem.alloc   dd ?
mem.free    dd ?
mem.realloc dd ?
dll.load    dd ?

;;================================================================================================;;
proc libio._.init ;///////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Library entry point (called after library load)                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> eax = memory allocation routine <mem.alloc*>                                                   ;;
;> ebx = memory freeing routine <mem.free*>                                                       ;;
;> ecx = memory reallocation routine <mem.realloc*>                                               ;;
;> edx = library loading routine <dll.load*>                                                      ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 1 (fail) / 0 (ok) (library initialization result)                                        ;;
;;================================================================================================;;
	mov	[mem.alloc], eax
	mov	[mem.free], ebx
	mov	[mem.realloc], ecx
	mov	[dll.load], edx
	xor	eax, eax
	ret
endp

;;================================================================================================;;
proc libio._.match_wildcard _str, _wcard ;////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Match string against wildcard                                                                  ;;
;? Based on http://user.cs.tu-berlin.de/~schintke/references/wildcards/                           ;;
;? 1997-2001 (c) Florian Schintke                                                                 ;;
;;------------------------------------------------------------------------------------------------;;
;> _str = string (filename in most cases) <asciiz>                                                ;;
;> _wcard = mask expressed using wilcards (?, *, [..]) <asciiz>                                   ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true (match result)                                                              ;;
;;================================================================================================;;
	push	ecx edx esi edi
	mov	dl, 1 ; fit
	mov	esi, [_wcard]
	mov	edi, [_str]
  .loop_wildcard:
	mov	al, [esi]
	or	al, al
	jz	.loop_wildcard_exit
	or	dl, dl
	jz	.loop_wildcard_exit
	cmp	byte[edi], 0
	je	.loop_wildcard_exit

	cmp	al, '['
	je	.process_set
	cmp	al, '?'
	je	.process_question
	cmp	al, '*'
	je	.process_asterisk

	xor	dl, dl
	cmp	[edi], al
	jne	@f
	inc	dl
    @@: inc	edi

  .loop_wildcard_next:
	inc	esi
	jmp	.loop_wildcard


  .process_set:
	inc	esi
	xor	dl, dl ; fit
	xor	dh, dh ; negation
	mov	cl, 1  ; at_beginning
	cmp	byte[esi], '^'
	jne	.loop_set_wildcard
	inc	dh
	inc	esi

    .loop_set_wildcard:
	mov	al, [esi]
	cmp	al, ']'
	jne	@f
	or	cl, cl
	jz	.loop_set_wildcard_exit
    @@: or	dl, dl
	jnz	.loop_set_wildcard_fit
	cmp	al, '-'
	jne	.loop_set_wildcard_not_range
	mov	ch, [esi - 1]
	cmp	[esi + 1], ch
	jbe	.loop_set_wildcard_not_range
	cmp	byte[esi + 1], ']'
	je	.loop_set_wildcard_not_range
	or	cl, cl
	jnz	.loop_set_wildcard_not_range
	cmp	[edi], ch
	jb	.loop_set_wildcard_fit
	mov	ch, [esi + 1]
	cmp	[edi], ch
	ja	.loop_set_wildcard_fit
	mov	dl, 1
	inc	esi
	jmp	.loop_set_wildcard_fit

    .loop_set_wildcard_not_range:
	cmp	[edi], al
	jne	.loop_set_wildcard_fit
	mov	dl, 1

    .loop_set_wildcard_fit:
	inc	esi
	xor	cl, cl
	jmp	.loop_set_wildcard

    .loop_set_wildcard_exit:
	or	dh, dh
	jz	@f
	xor	dl, 1
    @@: or	dl, dl
	jz	@f
	inc	edi
    @@:
	jmp	.loop_wildcard_next

  .process_question:
	inc	edi
	jmp	.loop_wildcard_next

  .process_asterisk:
	mov	dl, 1
	inc	esi

    .loop_asterisk_del_shit:
	lodsb
	cmp	byte[edi], 0
	je	.loop_asterisk_del_shit_exit
	cmp	al, '?'
	jne	@f
	inc	edi
	jmp	.loop_asterisk_del_shit
    @@: cmp	al, '*'
	je	.loop_asterisk_del_shit

    .loop_asterisk_del_shit_exit:

    @@: cmp	al, '*'
	jne	@f
	lodsb
	jmp	@b
    @@:
	dec	esi
	cmp	byte[edi], 0
	jne	.process_asterisk_skip_exit
	xor	dl, dl
	or	al, al
	jnz	@f
	inc	dl
    @@: dec	esi
	jmp	.loop_wildcard_next

    .process_asterisk_skip_exit:
	stdcall libio._.match_wildcard, edi, esi
	or	eax, eax
	jnz	.process_asterisk_not_match

    .loop_asterisk_match:
	inc	edi

    .loop_asterisk_char_match:
	mov	al, [esi]
	cmp	[edi], al
	je	.loop_asterisk_char_match_exit
	cmp	byte[esi], '['
	je	.loop_asterisk_char_match_exit
	cmp	byte[edi], 0
	je	.loop_asterisk_char_match_exit
	inc	edi
	jmp	.loop_asterisk_char_match

    .loop_asterisk_char_match_exit:
	cmp	byte[edi], 0
	je	@f
	stdcall libio._.match_wildcard, edi, esi
	or	eax, eax
	jnz	.loop_asterisk_match_exit
	jmp	.loop_asterisk_match
    @@:
	xor	dl, dl

    .loop_asterisk_match_exit:

    .process_asterisk_not_match:
	cmp	byte[esi], 0
	jne	@f
	cmp	byte[edi], 0
	jne	@f
	mov	dl, 1
    @@:
	dec	esi
	jmp	.loop_wildcard_next

  .loop_wildcard_exit:
	or	dl, dl
	jz	.exit
    @@: cmp	byte[esi], '*'
	jne	.exit
	inc	esi
	jmp	@b

  .exit:
	cmp	byte[esi], 0
	je	@f
	xor	dl, dl
    @@: cmp	byte[edi], 0
	je	@f
	xor	dl, dl
    @@:
	movzx	eax, dl

	pop	edi esi edx ecx
	ret
endp

;;================================================================================================;;
proc libio._.find_matching_file _ffb ;////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Find file with matching attributes (`FindFileBlock.Options.Attributes`) and mask               ;;
;? (`FindFileBlock.Options.Mask`) starting from Nth (`FindFileBlock.InfoBlock.Position`) file in  ;;
;? directory (`FindFileBlock.InfoBlock.FileName`)                                                 ;;
;;------------------------------------------------------------------------------------------------;;
;> _ffb = find file block <FindFileBlock*>                                                        ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / matched file info pointer <FileInfo*>                                        ;;
;;================================================================================================;;
	push	ebx edx
	mov	edx, [_ffb]
  .loop_find:
	lea	ebx, [edx + FindFileBlock.InfoBlock]
	mcall	70
	or	eax, eax
	jnz	.loop_find_error
	mov	eax, [edx + FindFileBlock.Info.Attributes]
	and	eax, [edx + FindFileBlock.Options.Attributes]
	jz	.loop_find_next
	lea	eax, [edx + FindFileBlock.Info.FileName]
	stdcall libio._.match_wildcard, eax, [edx + FindFileBlock.Options.Mask]
	or	eax, eax
	jnz	.loop_find_exit

  .loop_find_next:
	inc	[edx + FindFileBlock.InfoBlock.Position]
	jmp	.loop_find

  .loop_find_error:
	xor	eax, eax
	pop	edx ebx
	ret

  .loop_find_exit:
	lea	eax, [edx + FindFileBlock.Info]
	pop	edx ebx
	ret
endp
