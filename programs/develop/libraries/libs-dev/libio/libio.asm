;;================================================================================================;;
;;//// libio.asm //// (c) mike.dld, 2006-2008 ////////////////////////////////////////////////////;;
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
;; 2007-12-10 (mike.dld)                                                                          ;;
;;   changes:                                                                                     ;;
;;     - almost fully incompatible with previous version since return values were changed.        ;;
;;       now they are more C-like                                                                 ;;
;;   notes:                                                                                       ;;
;;     - `file.err` is not yet available                                                          ;;
;; 2007-09-26 (mike.dld)                                                                          ;;
;;   changes:                                                                                     ;;
;;     - modified `file.size` a bit (according to changes in FileInfo struct)                     ;;
;;     - added `file.find_first`, `file.find_next`, `file.find_close`                             ;;
;;   notes:                                                                                       ;;
;;     - `file.aux.match_wildcard` is exported only for testing purposes, don't                   ;;
;;       use it since it may be removed or renamed in next versions                               ;;
;;                                                                                                ;;
;;================================================================================================;;


format MS COFF

public @EXPORT as 'EXPORTS'

include '../../../../proc32.inc'
include '../../../../macros.inc'
purge section;mov,add,sub

include 'libio.inc'
include 'libio_p.inc'

section '.flat' code readable align 16

mem.alloc   dd ?
mem.free    dd ?
mem.realloc dd ?
dll.load    dd ?

;;================================================================================================;;
proc lib_init ;///////////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Library entry point (called after library load)                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> eax = pointer to memory allocation routine                                                     ;;
;> ebx = pointer to memory freeing routine                                                        ;;
;> ecx = pointer to memory reallocation routine                                                   ;;
;> edx = pointer to library loading routine                                                       ;;
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
proc file.aux.match_wildcard _str, _wcard ;///////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Match string against wildcard                                                                  ;;
;? Based on http://user.cs.tu-berlin.de/~schintke/references/wildcards/                           ;;
;? 1997-2001 (c) Florian Schintke                                                                 ;;
;;------------------------------------------------------------------------------------------------;;
;> _str = pointer to string (filename in most cases)                                              ;;
;> _wcard = pointer to string (mask expressed using wilcards (?, *, [..]))                        ;;
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
	stdcall file.aux.match_wildcard, edi, esi
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
	stdcall file.aux.match_wildcard, edi, esi
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
proc file.aux.find_matching_file _ffb ;///////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Find file with matching attributes (`FindFileBlock.Options.Attributes`) and mask               ;;
;? (`FindFileBlock.Options.Mask`) starting from Nth (`FindFileBlock.InfoBlock.Position`) file in  ;;
;? directory (`FindFileBlock.InfoBlock.FileName`)                                                 ;;
;;------------------------------------------------------------------------------------------------;;
;> _ffb = pointer to FindFileBlock                                                                ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / pointer to `FileInfo` with matched file data                                 ;;
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
	stdcall file.aux.match_wildcard, eax, [edx + FindFileBlock.Options.Mask]
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

;;================================================================================================;;
proc file.find_first _dir, _mask, _attr ;/////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Find first file with matching attributes and mask in specified directory                       ;;
;;------------------------------------------------------------------------------------------------;;
;> _dir = pointer to string (directory path to search in)                                         ;;
;> _mask = pointer to string (file mask, with use of wildcards)                                   ;;
;> _attr = file attributes mask (combination of FA_* constants)                                   ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / pointer to `FileInfo` with matched file data (acts as find descriptor)       ;;
;;================================================================================================;;
	push	ebx edx

	invoke	mem.alloc, sizeof.FindFileBlock
	or	eax, eax
	jz	.exit.error
	mov	edx, eax
	mov	ebx, [_attr]
	mov	[edx + FindFileBlock.Options.Attributes], ebx
	mov	ebx, [_mask]
	mov	[edx + FindFileBlock.Options.Mask], ebx

	lea	ebx, [edx + FindFileBlock.InfoBlock]
	mov	[ebx + FileInfoBlock.Function], F70_READ_D
	mov	[ebx + FileInfoBlock.Count], 1
	lea	eax, [edx + FindFileBlock.Header]
	mov	[ebx + FileInfoBlock.Buffer], eax
	mov	eax, [_dir]
	mov	[ebx + FileInfoBlock.FileName], eax

	stdcall file.aux.find_matching_file, edx
	pop	edx ebx
	ret

  .exit.error:
	xor	eax, eax
	pop	edx ebx
	ret
endp

;;================================================================================================;;
proc file.find_next _findd ;//////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Find next file matching criteria                                                               ;;
;;------------------------------------------------------------------------------------------------;;
;> _findd = find descriptor (see `file.find_first`)                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / pointer to `FileInfo` with matched file data (acts as find descriptor)       ;;
;;================================================================================================;;
	mov	eax, [_findd]
	add	eax, -sizeof.FileInfoHeader
	inc	[eax + FindFileBlock.InfoBlock.Position]
	stdcall file.aux.find_matching_file, eax
	ret
endp

;;================================================================================================;;
proc file.find_close _findd ;/////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Close find descriptor and free memory                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;> _findd = find descriptor (see `file.find_first`)                                               ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = result of memory freeing routine                                                         ;;
;;================================================================================================;;
	mov	eax, [_findd]
	add	eax, -sizeof.FileInfoHeader
	invoke	mem.free, eax
	ret
endp

;;================================================================================================;;
proc file.size _name ;////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Get file size                                                                                  ;;
;;------------------------------------------------------------------------------------------------;;
;> _name = path to file (full or relative)                                                        ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / file size (in bytes, up to 2G)                                              ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
locals
  loc_info FileInfoBlock
endl

	lea	ebx, [loc_info]
	invoke	mem.alloc, 40
	push	eax
	mov	[ebx + FileInfoBlock.Function], F70_GETATTR_FD
	mov	[ebx + FileInfoBlock.Buffer], eax
	mov	byte[ebx + FileInfoBlock.FileName - 1], 0
	mov	eax, [_name]
	mov	[ebx + FileInfoBlock.FileName], eax
	mcall	70
	pop	ebx
	push	eax
	mov	eax, ebx
	mov	ebx, [ebx + FileInfo.FileSizeLow]
	invoke	mem.free, eax
	pop	eax
	ret
endp

;;================================================================================================;;
proc file.open _name, _mode ;/////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Open file                                                                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _name = path to file (full or relative)                                                        ;;
;> _mode = mode to open file in (combination of O_* constants)                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) / file descriptor                                                              ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
locals
  loc_info FileInfoBlock
  loc_buf  rb 40
endl

	push	ebx esi edi

	xor	ebx, ebx
	invoke	mem.alloc, sizeof.InternalFileInfo
	or	eax, eax
	jz	.exit_error
	mov	ebx, eax
	push	[_mode]
	pop	[ebx + InternalFileInfo.Mode]
	mov	[ebx + InternalFileInfo.Position], 0
	lea	edi, [ebx + InternalFileInfo.FileName]
	mov	esi, [_name]
	mov	ecx, 260 / 4
	cld
	rep	movsd

  .get_info:
	push	ebx
	mov	[loc_info.Function], F70_GETATTR_FD
	lea	eax, [loc_buf]
	mov	[loc_info.Buffer], eax
	mov	byte[loc_info.FileName - 1], 0
	mov	eax, [_name]
	mov	[loc_info.FileName], eax
	lea	ebx, [loc_info]
	mcall	70
	pop	ebx
	or	eax, eax
	jz	@f
	cmp	eax, 6
	jne	.exit_error.ex
    @@:
	mov	eax, ebx
	pop	edi esi ebx
	ret

  .exit_error.ex:
	test	[_mode], O_CREATE
	jz	.exit_error
	push	ebx
	mov	[loc_info.Function], F70_CREATE_F
	xor	eax, eax
	mov	[loc_info.Position], eax
	mov	[loc_info.Flags], eax
	mov	[loc_info.Count], eax
	lea	ebx, [loc_info]
	mcall	70
	pop	ebx
	or	eax, eax
	jz	.get_info

  .exit_error:
	invoke	mem.free, ebx
	xor	eax, eax
	pop	edi esi ebx
	ret
endp

;;================================================================================================;;
proc file.read _filed, _buf, _buflen ;////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Read data from file                                                                            ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`)                                                     ;;
;> _buf = pointer to buffer to put read data to                                                   ;;
;> _buflen = buffer size (number of bytes to be read from file)                                   ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / number of bytes read                                                        ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
locals
  loc_info FileInfoBlock
endl

	push	ebx esi edi

	mov	ebx, [_filed]
	test	[ebx + InternalFileInfo.Mode], O_READ
	jz	.exit_error

	xor	eax, eax
	mov	[loc_info.Function], F70_READ_F
	mov	[loc_info.Flags], eax
	mov	byte[loc_info.FileName - 1], al
	push	[ebx+InternalFileInfo.Position] [_buflen] [_buf]
	pop	[loc_info.Buffer] [loc_info.Count] [loc_info.Position]
	lea	eax, [ebx + InternalFileInfo.FileName]
	mov	[loc_info.FileName], eax
	lea	ebx, [loc_info]
	mcall	70
	or	eax, eax
	jz	@f
	cmp	eax, 6
	jne	.exit_error
    @@:
	mov	eax, ebx
	mov	ebx, [_filed]
	add	[ebx + InternalFileInfo.Position], eax
	pop	edi esi ebx
	ret

  .exit_error:
	or	eax, -1
	pop	edi esi ebx
	ret
endp

;;================================================================================================;;
proc file.write _filed, _buf, _buflen ;///////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Write data to file                                                                             ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`)                                                     ;;
;> _buf = pointer to buffer to get write data from                                                ;;
;> _buflen = buffer size (number of bytes to be written to file)                                  ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / number of bytes written                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
locals
  loc_info FileInfoBlock
endl

	push	ebx esi edi

	mov	ebx, [_filed]
	test	[ebx + InternalFileInfo.Mode], O_WRITE
	jz	.exit_error

	stdcall file.eof?, [_filed]
	or	eax, eax
	js	.exit_error
	jz	@f
	stdcall file.truncate, [_filed]
    @@:
	mov	[loc_info.Function], F70_WRITE_F
	xor	eax, eax
	mov	[loc_info.Flags], eax
	mov	byte[loc_info.FileName - 1], al
	push	[ebx + InternalFileInfo.Position] [_buflen] [_buf]
	pop	[loc_info.Buffer] [loc_info.Count] [loc_info.Position]
	lea	eax, [ebx + InternalFileInfo.FileName]
	mov	[loc_info.FileName], eax
	lea	ebx, [loc_info]
	mcall	70
	or	eax, eax
	jnz	.exit_error
    @@:
	mov	eax, ebx
	mov	ebx, [_filed]
	add	[ebx + InternalFileInfo.Position],eax
	pop	edi esi ebx
	ret

  .exit_error:
	or	eax, -1
	pop	edi esi ebx
	ret
endp

;;================================================================================================;;
proc file.seek _filed, _where, _origin ;//////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Set file pointer position                                                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`)                                                     ;;
;> _where = position in file (in bytes) counted from specified origin                             ;;
;> _origin = origin from where to set the position (one of SEEK_* constants)                      ;;
;>   SEEK_SET - from beginning of file                                                            ;;
;>   SEEK_CUR - from current pointer position                                                     ;;
;>   SEEK_END - from end of file                                                                  ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / 0                                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
	push	ebx ecx edx

	mov	ecx, [_filed]
	lea	eax, [ecx + InternalFileInfo.FileName]
	stdcall file.size, eax
	or	eax, eax
	jnz	.exit_error
	mov	edx, [_where]
	cmp	[_origin], SEEK_SET
	jne	.n_set
	mov	[ecx + InternalFileInfo.Position], edx
	jmp	.exit_ok

  .n_set:
	cmp	[_origin], SEEK_CUR
	jne	.n_cur
	add	[ecx + InternalFileInfo.Position], edx
	jmp	.exit_ok

  .n_cur:
	cmp	[_origin], SEEK_END
	jne	.exit_error
	neg	edx
	add	edx, ebx
	mov	[ecx + InternalFileInfo.Position], edx

  .exit_ok:

	cmp	[ecx + InternalFileInfo.Position], 0
	jge	@f
	mov	[ecx + InternalFileInfo.Position], 0
    @@:
;       cmp     ebx, [ecx+InternalFileInfo.Position]
;       jae     @f
;       mov     [ecx + InternalFileInfo.Position], ebx
;   @@:
	xor	eax, eax
	pop	edx ecx ebx
	ret

  .exit_error:
	or	eax, -1
	pop	edx ecx ebx
	ret
endp

;;================================================================================================;;
proc file.eof? _filed ;///////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if file pointer is at the end of file                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`)                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
	push	ebx ecx

	mov	ecx, [_filed]
	lea	eax, [ecx + InternalFileInfo.FileName]
	stdcall file.size, eax
	or	eax, eax
	jnz	.exit_error

	xor	eax, eax
	cmp	[ecx + InternalFileInfo.Position], ebx
	jb	@f
	inc	eax
    @@: pop	ecx ebx
	ret

  .exit_error:
	or	eax, -1
	pop	ecx ebx
	ret
endp

;;================================================================================================;;
proc file.truncate _filed ;///////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Truncate file size to current file pointer position                                            ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`)                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / 0                                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
locals
  loc_info FileInfoBlock
endl

	push	ebx esi edi

	mov	ebx, [_filed]
	test	[ebx + InternalFileInfo.Mode], O_WRITE
	jz	.exit_error

	mov	[loc_info.Function], F70_SETSIZE_F
	mov	eax, [ebx + InternalFileInfo.Position]
	mov	[loc_info.Position], eax
	xor	eax, eax
	mov	[loc_info.Flags], eax
	mov	[loc_info.Count], eax
	mov	[loc_info.Buffer], eax
	mov	byte[loc_info.FileName - 1], al
	lea	eax, [ebx + InternalFileInfo.FileName]
	mov	[loc_info.FileName], eax
	lea	ebx, [loc_info]
	mcall	70
	cmp	eax, 2
	je	.exit_error
	cmp	eax, 8
	je	.exit_error
    @@: xor	eax, eax
	pop	edi esi ebx
	ret

  .exit_error:
	or	eax, -1
	pop	edi esi ebx
	ret
endp

file.seteof equ file.truncate

;;================================================================================================;;
proc file.tell _filed ;///////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Get current file pointer position                                                              ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`)                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / file pointer position                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
	mov	eax, [_filed]
	mov	eax, [eax + InternalFileInfo.Position]
	ret
endp

;;================================================================================================;;
proc file.close _filed ;//////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Close file                                                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;> _filed = file descriptor (see `file.open`)                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / file pointer position                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;# call `file.err` to obtain extended error information                                           ;;
;;================================================================================================;;
	mov	eax, [_filed]
	mov	[eax + InternalFileInfo.Mode], 0
	mov	[eax + InternalFileInfo.FileName], 0
	invoke	mem.free, eax
	xor	eax, eax
	ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Exported functions section                                                                     ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


align 16
@EXPORT:

export					      \
	lib_init	, 'lib_init'	    , \
	0x00030003	, 'version'	    , \
	file.find_first , 'file.find_first' , \
	file.find_next	, 'file.find_next'  , \
	file.find_close , 'file.find_close' , \
	file.size	, 'file.size'	    , \
	file.open	, 'file.open'	    , \
	file.read	, 'file.read'	    , \
	file.write	, 'file.write'	    , \
	file.seek	, 'file.seek'	    , \
	file.tell	, 'file.tell'	    , \
	file.eof?	, 'file.eof?'	    , \
	file.seteof	, 'file.seteof'     , \
	file.truncate	, 'file.truncate'   , \
	file.close	, 'file.close'
