;;================================================================================================;;
;;//// libini.asm //// (c) mike.dld, 2006-2008 ///////////////////////////////////////////////////;;
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
;; 2009-03-08 (mike.dld)                                                                          ;;
;;   bug-fixes:                                                                                   ;;
;;     - moved buffer bound check in libini._.low.read_value up (reported by Insolor)             ;;
;;   new features:                                                                                ;;
;;     - comments support (char is ini.COMMENT_CHAR, defaults to ';')                             ;;
;;       inline comments are not supported                                                        ;;
;; 2008-12-29 (mike.dld)                                                                          ;;
;;   bug-fixes:                                                                                   ;;
;;     - unnecessary 'stosb' in ini.get_str was causing problems                                  ;;
;;   new features:                                                                                ;;
;;     - new functions: ini.get_color and ini.set_color                                           ;;
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
purge section,mov,add,sub

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
	mov	byte[edi], '['
	inc	edi
	call	libini._.string_copy
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

	stdcall libini._.skip_spaces, [f_addr]
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
proc ini.get_shortcut _f_name, _sec_name, _key_name, _def_val, _modifiers ;///////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Read shortcut key                                                                              ;;
;;------------------------------------------------------------------------------------------------;;
;> _f_name = ini filename <asciiz>                                                                ;;
;> _sec_name = section name <asciiz>                                                              ;;
;> _key_name = key name <asciiz>                                                                  ;;
;> _def_val = default value to return if no key, section or file found <dword>                    ;;
;> _modifiers = pointer to dword variable which receives modifiers state as in 66.4 <dword*>      ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = [_def_val] (error) / shortcut key value as scancode <int>                                ;;
;< [[_modifiers]] = unchanged (error) / modifiers state for this shortcut <int>                   ;;
;;================================================================================================;;
locals
  buf rb 64
endl

	push	ebx esi edi

	lea	esi, [buf]
	stdcall ini.get_str, [_f_name], [_sec_name], [_key_name], esi, 64, 0
	cmp	byte[esi],0
	je	.exit_error

	xor	ebx, ebx	; ebx holds the value of modifiers
.loop:
; test for end
	xor	eax, eax
	cmp	byte [esi], al
	jz	.exit_ok	; exit with scancode zero
; skip all '+'s
	cmp	byte [esi], '+'
	jnz	@f
	inc	esi
	jmp	.loop
@@:
; test for names
	mov	edi, .names_table
	xor	edx, edx
.names_loop:
	movzx	ecx, byte [edi]
	inc	edi
	push	esi
@@:
	lodsb
	or	al, 20h
	scasb
	loopz	@b
	jz	.name_found
	pop	esi
	lea	edi, [edi+ecx+4]
	inc	edx
	cmp	byte [edi], 0
	jnz	.names_loop
; special test: functional keys F<number>
	cmp	byte [esi], 'f'
	jz	@f
	cmp	byte [esi], 'F'
	jnz	.no_fx
@@:
	mov	edi, esi
	inc	esi
	call	libini._.str_to_int
	test	eax, eax
	jz	.fx
	mov	esi, edi
.no_fx:
; name not found, that must be usual key
	movzx	eax, byte [esi]
	stdcall	libini._.ascii_to_scan, eax
	test	eax, eax
	jz	.exit_error
; all is ok
.exit_ok:
	mov	ecx, [_modifiers]
	test	ecx, ecx
	jz	@f
	mov	[ecx], ebx

@@:

	pop	edi esi ebx
	ret

.exit_error:
	mov	eax, [_def_val]
	pop	edi esi ebx
	ret
; handler for Fx
; eax = number
.fx:
	cmp	eax, 10
	ja	@f
	add	eax, 3Bh-1
	jmp	.exit_ok
@@:
	add	eax, 57h-11
	jmp	.exit_ok
; handlers for names
.name_found:
	pop	eax	; ignore saved esi
	call	dword [edi]
	cmp	edx, .num_modifiers
	jae	.exit_ok
	jmp	.loop
; modifiers
; syntax of value for each modifier:
; 0 = none, 1 = exactly one of L+R, 2 = both L+R, 3 = L, 4 = R
; Logic for switching: LShift+RShift=LShift+Shift=Shift+Shift, LShift+LShift=LShift
; generic modifier: 0->1->2->2, 3->2, 4->2
; left modifier: 0->3->3, 1->2->2, 4->2
; right modifier: 0->4->4, 1->2->2, 3->2
; Shift corresponds to first hex digit, Ctrl - second, Alt - third
macro shortcut_handle_modifiers name,reg,shift
{
local .set2,.set3,.set4
.#name#_handler:	; generic modifier
	test	reg, 0xF
	jnz	.set2
if shift
	or	reg, 1 shl shift
else
	inc	reg
end if
	retn
.set2:
	and	reg, not (0xF shl shift)
	or	reg, 2 shl shift
	retn
.l#name#_handler:
	mov	al, reg
	and	al, 0xF shl shift
	jz	.set3
	cmp	al, 3 shl shift
	jnz	.set2
	retn
.set3:
	add	reg, 3 shl shift
	retn
.r#name#_handler:
	mov	al, reg
	and	al, 0xF shl shift
	jz	.set4
	cmp	al, 4 shl shift
	jnz	.set2
	retn
.set4:
	add	reg, 4 shl shift
	retn
}
shortcut_handle_modifiers shift,bl,0
shortcut_handle_modifiers ctrl,bl,4
shortcut_handle_modifiers alt,bh,0
; names of keys
.name_handler:
	movzx	eax, byte [.names_scancodes+edx-.num_modifiers]
	retn
endp

; note: comparison ignores case, so this table keeps lowercase names
; macro does this
macro shortcut_name_with_handler name,handler
{
local .start, .end
	db	.end - .start
.start:
	db	name
.end:
repeat .end - .start
	load .a byte from .start + % - 1
	store byte .a or 0x20 at .start + % - 1
end repeat
	dd	handler
}
macro shortcut_name [name]
{
	shortcut_name_with_handler name, .name_handler
}
; all names here must be in english
; ... or modify lowercasing in macro and in comparison
.names_table:
; generic modifiers
	shortcut_name_with_handler 'Ctrl', .ctrl_handler
	shortcut_name_with_handler 'Alt', .alt_handler
	shortcut_name_with_handler 'Shift', .shift_handler
; concrete modifiers
	shortcut_name_with_handler 'LCtrl', .lctrl_handler
	shortcut_name_with_handler 'RCtrl', .rctrl_handler
	shortcut_name_with_handler 'LAlt', .lalt_handler
	shortcut_name_with_handler 'RAlt', .ralt_handler
	shortcut_name_with_handler 'LShift', .lshift_handler
	shortcut_name_with_handler 'RShift', .rshift_handler
.num_modifiers = 9
; symbolic names of keys
	shortcut_name 'Home', 'End', 'PgUp', 'PgDn', 'Ins', 'Insert', 'Del', 'Delete'
	shortcut_name 'Tab', 'Plus', 'Esc', 'Enter', 'Backspace', 'Space', 'Left', 'Right'
	shortcut_name 'Up', 'Down'
; end of table
	db	0
ini.get_shortcut.names_scancodes:
; scancodes for 'Home' ... 'Down'
	db	47h, 4Fh, 49h, 51h, 52h, 52h, 53h, 53h
	db	0Fh, 4Eh, 01h, 1Ch, 0Eh, 39h, 4Bh, 4Dh
	db	48h, 50h

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
	file.size   , 'file_size'   , \
	file.open   , 'file_open'   , \
	file.read   , 'file_read'   , \
	file.write  , 'file_write'  , \
	file.seek   , 'file_seek'   , \
	file.eof?   , 'file_iseof'  , \
	file.seteof , 'file_seteof' , \
	file.tell   , 'file_tell'   , \
	file.close  , 'file_close'


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
	0x00080009	  , 'version'		, \
	ini.enum_sections , 'ini_enum_sections' , \
	ini.enum_keys	  , 'ini_enum_keys'	, \
	ini.get_str	  , 'ini_get_str'	, \
	ini.get_int	  , 'ini_get_int'	, \
	ini.get_color	  , 'ini_get_color'	, \
	ini.set_str	  , 'ini_set_str'	, \
	ini.set_int	  , 'ini_set_int'	, \
	ini.set_color	  , 'ini_set_color'	, \
	ini.get_shortcut  , 'ini_get_shortcut'
