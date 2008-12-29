;;================================================================================================;;
;;//// libini.asm //// (c) mike.dld, 2006-2008 ///////////////////////////////////////////////////;;
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
;; 2008-12-29 (mike.dld)                                                                          ;;
;;   bug-fixes:                                                                                   ;;
;;     - unnecessary 'stosb' in ini.get_str was causing problems                                  ;;
;; 2008-08-06 (mike.dld)                                                                          ;;
;;   changes:                                                                                     ;;
;;     - split private procs into libini_p.asm, added comments                                    ;;
;; 2008-02-07 (mike.dld)                                                                          ;;
;;   changes:                                                                                     ;;
;;     - renamed all *.aux.* to *._.* to match overall libraries design                           ;;
;; 2007-09-26 (mike.dld)                                                                          ;;
;;   bug-fixes:                                                                                   ;;
;;     - value was not correctly trimmed (reported by diamond)                                    ;;
;; 2007-08-01 (mike.dld)                                                                          ;;
;;   bug-fixes:                                                                                   ;;
;;     - serious defect in ini.set_str causing displaced write operations                         ;;
;;       (reported by diamond)                                                                    ;;
;;     - another serious defect in ini.enum_keys introduced with refactoring                      ;;
;;   changes:                                                                                     ;;
;;     - callback for enum_keys now takes additional parameter - key value                        ;;
;;     - handling trailing spaces in section/key/value                                            ;;
;; 2007-05-19 (mike.dld)                                                                          ;;
;;   bug-fixes:                                                                                   ;;
;;     - last char still wasn't read correctly                                                    ;;
;;     - digits of number were reversed when using ini.set_int                                    ;;
;;     - now using 'ini.aux.unget_char' instead of dangerous 'dec esi'                            ;;
;;   changes:                                                                                     ;;
;;     - all non-public functions now start with ini.aux.*                                        ;;
;;     - added ini.enum_sections and ini.enum_keys                                                ;;
;;     - removed ini.query_sec (use ini.enum_* instead)                                           ;;
;;                                                                                                ;;
;;================================================================================================;;

format MS COFF

public @EXPORT as 'EXPORTS'

include '../../../../proc32.inc'
include '../../../../macros.inc'
include '../libio/libio.inc'
purge section ; mov,add,sub

include 'libini_p.inc'

section '.flat' code readable align 16

include 'libini_p.asm'

;;================================================================================================;;
proc ini.enum_sections _f_name, _callback ;///////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Enumerate sections, calling callback function for each of them                                 ;;
;;------------------------------------------------------------------------------------------------;;
;> _f_name = ini filename <asciiz>                                                                ;;
;> _callback = callback function address: func(f_name, sec_name), where                           ;;
;>   f_name = ini filename (as passed to the function) <asciiz>                                   ;;
;>   sec_name = section name found <asciiz>                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / 0                                                                           ;;
;;================================================================================================;;
locals
  f	  IniFile
  f_addr  dd ?
  sec_buf dd ?
endl

	push	ebx esi edi

	cld

	invoke	mem.alloc, ini.MAX_NAME_LEN
	or	eax, eax
	jz	.exit_error.2
	mov	[sec_buf], eax

	xor	eax, eax
	mov	[f.fh], eax
	mov	[f.buf], eax
	invoke	file.open, [_f_name], O_READ
	cmp	eax, 32
	jb	.exit_error
	mov	[f.fh], eax
	invoke	mem.alloc, ini.MEM_SIZE
	or	eax, eax
	jz	.exit_error
	mov	[f.buf], eax
	lea	ebx, [f]
	mov	[f_addr], ebx

	invoke	file.seek, [f.fh], 0, SEEK_SET
	stdcall libini._.preload_block, [f_addr]

  .next_section:
	stdcall libini._.find_next_section, [f_addr]
	or	eax, eax
	jnz	.exit_error

	stdcall libini._.get_char, [f_addr]
	stdcall libini._.skip_spaces, [f_addr]
;       inc     esi
;       dec     [f.cnt]
	mov	edi, [sec_buf]
    @@: stdcall libini._.get_char, [f_addr]
	cmp	al, ']'
	je	@f
	or	al, al
	jz	.exit_ok
	cmp	al, 13
	je	.next_section
	cmp	al, 10
	je	.next_section
	stosb
	jmp	@b
    @@: xor	al, al
	stosb
	add	edi, -2
    @@: cmp	byte[edi], 32
	ja	@f
	mov	byte[edi], 0
	dec	edi
	jmp	@b
    @@:
	pushad
	mov	eax, [f_addr]
	stdcall [_callback], [_f_name], [sec_buf]
	or	eax, eax
	popad
	jnz	.next_section

  .exit_ok:
	invoke	file.close, [f.fh]
	invoke	mem.free, [f.buf]
	invoke	mem.free, [sec_buf]
	xor	eax, eax
	pop	edi esi ebx
	ret

  .exit_error:
	invoke	file.close, [f.fh]
	invoke	mem.free, [f.buf]
	invoke	mem.free, [sec_buf]
  .exit_error.2:
	or	eax, -1
	pop	edi esi ebx
	ret
endp

;;================================================================================================;;
proc ini.enum_keys _f_name, _sec_name, _callback ;////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Enumerate keys within a section, calling callback function for each of them                    ;;
;;------------------------------------------------------------------------------------------------;;
;> _f_name = ini filename <asciiz>                                                                ;;
;> _sec_name = section name <asciiz>                                                              ;;
;> _callback = callback function address: func(f_name, sec_name, key_name, key_value), where      ;;
;>   f_name = ini filename (as passed to the function) <asciiz>                                   ;;
;>   sec_name = section name (as passed to the function) <asciiz>                                 ;;
;>   key_name = key name found <asciiz>                                                           ;;
;>   key_value = value of key found <asciiz>                                                      ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / 0                                                                           ;;
;;================================================================================================;;
locals
  f	  IniFile
  f_addr  dd ?
  key_buf dd ?
  val_buf dd ?
endl

	push	ebx esi edi

	cld

	invoke	mem.alloc, ini.MAX_NAME_LEN
	or	eax, eax
	jz	.exit_error.3
	mov	[key_buf], eax
	invoke	mem.alloc, ini.MAX_VALUE_LEN
	or	eax, eax
	jz	.exit_error.2
	mov	[val_buf], eax

	xor	eax, eax
	mov	[f.fh], eax
	mov	[f.buf], eax
	invoke	file.open, [_f_name], O_READ
	cmp	eax, 32
	jb	.exit_error
	mov	[f.fh], eax
	invoke	mem.alloc, ini.MEM_SIZE
	or	eax, eax
	jz	.exit_error
	mov	[f.buf], eax
	lea	ebx, [f]
	mov	[f_addr], ebx
	stdcall libini._.find_section, ebx, [_sec_name]
	or	eax, eax
	jnz	.exit_error

  .next_key:
	stdcall libini._.skip_line, [f_addr]
	stdcall libini._.skip_nonblanks, [f_addr]
	or	al, al
	jz	.exit_error
	cmp	al, '['
	je	.exit_error
	mov	edi, [key_buf]
    @@: stdcall libini._.get_char, [f_addr]
	or	al, al
	jz	.exit_error
	cmp	al, '='
	je	@f
	stosb
	jmp	@b
    @@:
	xor	al, al
	stosb
	add	edi, -2
    @@: cmp	byte[edi], 32
	ja	@f
	mov	byte[edi], 0
	dec	edi
	jmp	@b
    @@: stdcall libini._.low.read_value, [f_addr], [val_buf], ini.MAX_VALUE_LEN
	pushad
	stdcall [_callback], [_f_name], [_sec_name], [key_buf], [val_buf]
	or	eax, eax
	popad
	jnz	.next_key

    @@: invoke	file.close, [f.fh]
	invoke	mem.free, [f.buf]
	xor	eax, eax
	stosb
	pop	edi esi ebx
	ret

  .exit_error:
	invoke	file.close, [f.fh]
	invoke	mem.free, [f.buf]
	invoke	mem.free, [val_buf]
  .exit_error.2:
	invoke	mem.free, [key_buf]
  .exit_error.3:
	or	eax, -1
	pop	edi esi ebx
	ret
endp

;;================================================================================================;;
proc ini.get_str _f_name, _sec_name, _key_name, _buffer, _buf_len, _def_val ;/////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Read string                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> _f_name = ini filename <asciiz>                                                                ;;
;> _sec_name = section name <asciiz>                                                              ;;
;> _key_name = key name <asciiz>                                                                  ;;
;> _buffer = destination buffer address <byte*>                                                   ;;
;> _buf_len = buffer size (maximum bytes to read) <dword>                                         ;;
;> _def_val = default value to return if no key, section or file found <asciiz>                   ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / 0                                                                           ;;
;< [_buffer] = [_def_val] (error) / found key value <asciiz>                                      ;;
;;================================================================================================;;
locals
  f	 IniFile
  f_addr dd ?
endl

	push	ebx esi edi

	xor	eax, eax
	mov	[f.fh], eax
	mov	[f.buf], eax
	invoke	file.open, [_f_name], O_READ
	cmp	eax, 32
	jb	.exit_error
	mov	[f.fh], eax
	invoke	mem.alloc, ini.MEM_SIZE
	or	eax, eax
	jz	.exit_error
	mov	[f.buf], eax
	lea	ebx, [f]
	mov	[f_addr], ebx
	stdcall libini._.find_section, ebx, [_sec_name]
	or	eax, eax
	jnz	.exit_error

	stdcall libini._.find_key, ebx, [_key_name]
	or	eax, eax
	jnz	.exit_error

	stdcall libini._.low.read_value, [f_addr], [_buffer], [_buf_len]
    @@: invoke	file.close, [f.fh]
	invoke	mem.free, [f.buf]
	xor	eax, eax
	pop	edi esi ebx
	ret

  .exit_error:
	invoke	file.close, [f.fh]
	invoke	mem.free, [f.buf]
	mov	edi, [_buffer]
	mov	esi, [_def_val]
	xor	al, al
	or	esi, esi
	jz	.exit_error.2
    @@: lodsb
  .exit_error.2:
	stosb
	or	al, al
	jnz	@b
	or	eax, -1
	pop	edi esi ebx
	ret
endp

;;================================================================================================;;
proc ini.set_str _f_name, _sec_name, _key_name, _buffer, _buf_len ;///////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Write string                                                                                   ;;
;;------------------------------------------------------------------------------------------------;;
;> _f_name = ini filename <asciiz>                                                                ;;
;> _sec_name = section name <asciiz>                                                              ;;
;> _key_name = key name <asciiz>                                                                  ;;
;> _buffer = source buffer address <byte*>                                                        ;;
;> _buf_len = buffer size (bytes to write) <dword>                                                ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / 0                                                                           ;;
;;================================================================================================;;
locals
  f	 IniFile
  f_addr dd ?
endl

	push	ebx esi edi

	xor	eax, eax
	mov	[f.fh], eax
	mov	[f.buf], eax
	invoke	file.open, [_f_name], O_READ + O_WRITE + O_CREATE
	cmp	eax, 32
	jb	.exit_error
	mov	[f.fh], eax
	invoke	mem.alloc, ini.MEM_SIZE
	or	eax, eax
	jz	.exit_error
	mov	[f.buf], eax
	lea	ebx, [f]
	mov	[f_addr], ebx

	stdcall libini._.find_section, ebx, [_sec_name]
	or	eax, eax
	jnz	.create_section

	stdcall libini._.find_key, ebx, [_key_name]
	or	eax, eax
	jnz	.create_key

  .modify_key:
	stdcall libini._.get_value_length, [f_addr]
	sub	eax, [_buf_len]
	stdcall libini._.shift_content, [f_addr], eax

  .modify_key.ex:
	invoke	file.tell, [f.fh]
	sub	eax, [f.cnt]
;       dec     eax
	invoke	file.seek, [f.fh], eax, SEEK_SET
	invoke	file.write, [f.fh], [_buffer], [_buf_len]

	pop	edi esi ebx
	xor	eax, eax
	ret

  .create_key:
	mov	edi, [f.buf]
	add	edi, ini.BLOCK_SIZE
	push	edi

  .create_key.ex:
;       mov     word[edi], 0x0A0D
;       add     edi,2
	mov	esi, [_key_name]
	call	libini._.string_copy
	mov	byte[edi], '='
	inc	edi
	mov	esi, [_buffer]
	mov	ecx, [_buf_len]
	rep	movsb
	mov	word[edi], 0x0A0D
	add	edi, 2
	mov	eax, edi

	pop	edi
	sub	eax, edi
	mov	[_buffer], edi
	mov	[_buf_len], eax
	neg	eax
	stdcall libini._.shift_content, [f_addr], eax

	jmp	.modify_key.ex

  .create_section:
	mov	edi, [f.buf]
	add	edi, ini.BLOCK_SIZE
	push	edi

	mov	esi, [_sec_name]
;       mov     dword[edi], 0x0A0D + ('[' shl 16)
;       add     edi, 3
	mov	byte[edi], '['
	inc	edi
	call	libini._.string_copy
;       mov     byte[edi], ']'
;       inc     edi
	mov	dword[edi], ']' + (0x0A0D shl 8)
	add	edi, 3

	jmp	.create_key.ex

  .exit_error:
	pop	edi esi ebx
	or	eax, -1
	ret
endp

;;================================================================================================;;
proc ini.get_int _f_name, _sec_name, _key_name, _def_val ;////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Read integer                                                                                   ;;
;;------------------------------------------------------------------------------------------------;;
;> _f_name = ini filename <asciiz>                                                                ;;
;> _sec_name = section name <asciiz>                                                              ;;
;> _key_name = key name <asciiz>                                                                  ;;
;> _def_val = default value to return if no key, section or file found <dword>                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = [_def_val] (error) / found key value <dword>                                             ;;
;;================================================================================================;;
locals
  f	 IniFile
  f_addr dd ?
endl

	push	ebx esi edi

	xor	eax, eax
	mov	[f.fh], eax
	mov	[f.buf], eax
	invoke	file.open, [_f_name], O_READ
	cmp	eax, 32
	jb	.exit_error
	mov	[f.fh], eax
	invoke	mem.alloc, ini.MEM_SIZE
	or	eax, eax
	jz	.exit_error
	mov	[f.buf], eax
	lea	ebx, [f]
	mov	[f_addr], ebx
	stdcall libini._.find_section, ebx, [_sec_name]
	or	eax, eax
	jnz	.exit_error

	stdcall libini._.find_key, ebx, [_key_name]
	or	eax, eax
	jnz	.exit_error

	stdcall libini._.skip_nonblanks, [f_addr]
	xor	eax, eax
	xor	ebx, ebx
	xor	edx, edx
	stdcall libini._.get_char, [f_addr]
	cmp	al, '-'
	jne	.lp1
	inc	bh
    @@: stdcall libini._.get_char, [f_addr]
  .lp1: cmp	al, '0'
	jb	@f
	cmp	al, '9'
	ja	@f
	inc	bl
	add	eax, -'0'
	imul	edx, 10
	add	edx, eax
	jmp	@b
    @@:
	or	bl, bl
	jz	.exit_error
	or	bh, bh
	jz	@f
	neg	edx
    @@: invoke	file.close, [f.fh]
	invoke	mem.free, [f.buf]
	mov	eax, edx
	pop	edi esi ebx
	ret

  .exit_error:
	invoke	file.close, [f.fh]
	invoke	mem.free, [f.buf]
	mov	eax, [_def_val]
	pop	edi esi ebx
	ret
endp

;;================================================================================================;;
proc ini.set_int _f_name, _sec_name, _key_name, _val ;////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Write integer                                                                                  ;;
;;------------------------------------------------------------------------------------------------;;
;> _f_name = ini filename <asciiz>                                                                ;;
;> _sec_name = section name <asciiz>                                                              ;;
;> _key_name = key name <asciiz>                                                                  ;;
;> _val = value <dword>                                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / 0                                                                           ;;
;;================================================================================================;;
locals
  buf rb 16
endl

	push	ecx edx edi

	lea	edi, [buf]
	add	edi, 15
	mov	eax, [_val]
	or	eax, eax
	jns	@f
	mov	byte[edi], '-'
	neg	eax
	inc	edi
    @@: mov	ecx, 10
    @@: xor	edx, edx
	idiv	ecx
	add	dl, '0'
	mov	[edi], dl
	dec	edi
	or	eax, eax
	jnz	@b
	lea	eax, [buf]
	add	eax, 15
	sub	eax, edi
	inc	edi

	stdcall ini.set_str, [_f_name], [_sec_name], [_key_name], edi, eax

	pop	edi edx ecx
	ret
endp

;;================================================================================================;;
proc ini.get_color _f_name, _sec_name, _key_name, _def_val ;//////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Read color                                                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;> _f_name = ini filename <asciiz>                                                                ;;
;> _sec_name = section name <asciiz>                                                              ;;
;> _key_name = key name <asciiz>                                                                  ;;
;> _def_val = default value to return if no key, section or file found <dword>                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = [_def_val] (error) / found key value <dword>                                             ;;
;;================================================================================================;;
locals
  buf rb 14
endl

	push	ebx esi edi

	lea	esi, [buf]
	stdcall ini.get_str, [_f_name], [_sec_name], [_key_name], esi, 14, 0
	cmp	byte[esi],0
	je	.exit_error

	xor	ebx, ebx
	stdcall libini._.str_to_int
	movzx	ebx, al
	shl	ebx, 16
	lodsb
	cmp	al, ','
	jne	@f
	stdcall libini._.str_to_int
	mov	bh, al
	lodsb
	cmp	al, ','
	jne	@f
	stdcall libini._.str_to_int
	mov	bl, al

    @@: mov	eax, ebx

	pop	edi esi ebx
	ret

  .exit_error:
	mov	eax, [_def_val]
	pop	edi esi ebx
	ret
endp

;;================================================================================================;;
proc ini.set_color _f_name, _sec_name, _key_name, _val ;//////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Write color                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> _f_name = ini filename <asciiz>                                                                ;;
;> _sec_name = section name <asciiz>                                                              ;;
;> _key_name = key name <asciiz>                                                                  ;;
;> _val = value <dword>                                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (error) / 0                                                                           ;;
;;================================================================================================;;
locals
  buf rb 16
endl

	push	ecx edx edi

	lea	edi, [buf]
	mov	ecx, 10
	mov	ebx, [_val]
	mov	eax, ebx
	shr	eax, 16
	and	eax, 0x0ff
	stdcall libini._.int_to_str
	mov	byte[edi], ','
	inc	edi
	movzx	eax, bh
	stdcall libini._.int_to_str
	mov	byte[edi], ','
	inc	edi
	movzx	eax, bl
	stdcall libini._.int_to_str

	lea	eax, [buf]
	sub	edi, eax

	stdcall ini.set_str, [_f_name], [_sec_name], [_key_name], eax, edi

	pop	edi edx ecx
	ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Imported functions section                                                                     ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


align 16
@IMPORT:

library \
	libio , 'libio.obj'

import	libio			    , \
	file.size   , 'file.size'   , \
	file.open   , 'file.open'   , \
	file.read   , 'file.read'   , \
	file.write  , 'file.write'  , \
	file.seek   , 'file.seek'   , \
	file.eof?   , 'file.eof?'   , \
	file.seteof , 'file.seteof' , \
	file.tell   , 'file.tell'   , \
	file.close  , 'file.close'


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Exported functions section                                                                     ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


align 16
@EXPORT:

export						  \
	libini._.init	  , 'lib_init'		, \
	0x00040005	  , 'version'		, \
	ini.enum_sections , 'ini.enum_sections' , \
	ini.enum_keys	  , 'ini.enum_keys'	, \
	ini.get_str	  , 'ini.get_str'	, \
	ini.get_int	  , 'ini.get_int'	, \
	ini.get_color	  , 'ini.get_color'	, \
	ini.set_str	  , 'ini.set_str'	, \
	ini.set_int	  , 'ini.set_int'	, \
	ini.set_color	  , 'ini.set_color'
