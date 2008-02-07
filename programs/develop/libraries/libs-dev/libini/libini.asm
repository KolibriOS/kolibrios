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

section '.flat' code readable align 16

include 'libini_p.inc'

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

	invoke	dll.load, @IMPORT
	or	eax, eax
	jz	.ok

	xor	eax, eax
	inc	eax
	ret

  .ok:	xor	eax,eax
	ret
endp

;;================================================================================================;;
proc ini._.unget_char _f ;////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	;push    ecx
	;mov     ecx,[f]
	;inc     [ecx+INIFILE.cnt]
	;dec     esi
	;pop     ecx
	;ret
	push	eax ecx
	mov	ecx, [_f]
	inc	[ecx + IniFile.cnt]
	dec	esi
	mov	eax, [ecx + IniFile.bsize]
	cmp	[ecx + IniFile.cnt], eax
	jle	@f
	stdcall ini._.unload_block, [_f]
    @@: ;mov     al,[esi-1]
	pop	ecx eax
	ret
endp

;;================================================================================================;;
proc ini._.get_char _f ;//////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	mov	ecx, [_f]
	dec	[ecx + IniFile.cnt]
	jns	@f
	stdcall ini._.preload_block, [_f]
	dec	[ecx + IniFile.cnt]
    @@: lodsb
	ret
endp

;;================================================================================================;;
proc ini._.skip_nonblanks _f ;////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	mov	ecx, [_f]
    @@: stdcall ini._.get_char, [_f]
	cmp	al, 32
	je	@b
	cmp	al, 13
	je	@b
	cmp	al, 10
	je	@b
	cmp	al, 9
	je	@b
    @@: stdcall ini._.unget_char, [_f]
	;inc     [ecx+INIFILE.cnt]
	ret
endp

;;================================================================================================;;
proc ini._.skip_spaces _f ;///////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	mov	ecx, [_f]
    @@: stdcall ini._.get_char, [_f]
	cmp	al, 32
	je	@b
	cmp	al, 9
	je	@b
    @@: stdcall ini._.unget_char, [_f]
	;inc     [ecx+INIFILE.cnt]
	ret
endp

;;================================================================================================;;
proc ini._.skip_line _f ;/////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	mov	ecx, [_f]
    @@: stdcall ini._.get_char, [_f]
	or	al, al
	jz	@f
	cmp	al, 13
	je	@f
	cmp	al, 10
	jne	@b
    @@: stdcall ini._.unget_char, [_f]
	;inc     [ecx+INIFILE.cnt]
	ret
endp

;;================================================================================================;;
proc ini._.unload_block _f ;//////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	push	eax ebx ecx
	mov	ebx, [_f]
	mov	eax, [ebx + IniFile.pos]
	add	eax, -ini.BLOCK_SIZE
	invoke	file.seek, [ebx + IniFile.fh], SEEK_SET, eax
	stdcall ini._.preload_block, ebx
	add	esi, eax ; ini.BLOCK_SIZE
	mov	[ebx + IniFile.cnt], 0
	pop	ecx ebx eax
	ret
endp

;;================================================================================================;;
proc ini._.preload_block _f ;/////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	push	eax ebx ecx
	mov	ebx, [_f]
    @@: mov	esi, [ebx + IniFile.buf]
	push	edi
	mov	edi, esi
	mov	ecx, ini.BLOCK_SIZE / 4
	xor	eax, eax
	rep	stosd
	pop	edi
	invoke	file.tell, [ebx + IniFile.fh]
	mov	[ebx + IniFile.pos], eax
	invoke	file.read, [ebx + IniFile.fh], esi, ini.BLOCK_SIZE
	mov	esi,[ebx + IniFile.buf]
	cmp	eax,ini.BLOCK_SIZE
	jl	@f
	;dec     eax
    @@: mov	[ebx + IniFile.cnt], eax;ini.BLOCK_SIZE-1
	mov	[ebx + IniFile.bsize], eax
	pop	ecx ebx eax
	ret
endp

;;================================================================================================;;
proc ini._.reload_block _f ;//////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	push	eax ebx ecx
	mov	ebx, [_f]
	push	[ebx + IniFile.bsize]
	push	esi [ebx + IniFile.cnt]
	invoke	file.seek, [ebx + IniFile.fh], SEEK_SET, [ebx + IniFile.pos]
	stdcall ini._.preload_block, ebx
	pop	[ebx + IniFile.cnt] esi
	pop	eax
	sub	eax,[ebx + IniFile.bsize]
	sub	[ebx + IniFile.cnt], eax
	pop	ecx ebx eax
	ret
endp

; f_info - contains current file block number
; esi    - position in block from where to shift
; ecx    - number of bytes to shift by

;;================================================================================================;;
proc ini._.shift_content _f, _delta ;/////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Shift file content starting from cursor position (~ delete)                                    ;;
;? Content is copied by 'delta' bytes up/down                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (ok) / -1 (fail)                                                                       ;;
;;================================================================================================;;
locals
  buf dd ?
endl

	xor	eax, eax
	cmp	[_delta], 0
	je	.skip

	push	ebx ecx
	invoke	mem.alloc, ini.BLOCK_SIZE
	or	eax, eax
	jz	.fail
	mov	[buf], eax

	cmp	[_delta], 0
	jl	.down

	mov	ebx, [_f]
	mov	ecx, [ebx + IniFile.cnt]
	mov	ebx, [ebx + IniFile.fh]
	invoke	file.tell, ebx
;       push    eax
	sub	eax, ecx
;       dec     eax
	invoke	file.seek, ebx, SEEK_SET, eax
    @@: invoke	file.seek, ebx, SEEK_CUR, [_delta]
	invoke	file.eof?, ebx
	or	eax, eax
	jnz	.done
	invoke	file.read, ebx, [buf], ini.BLOCK_SIZE
	mov	ecx, eax
	mov	eax, [_delta]
	neg	eax
	sub	eax,ecx;ini.BLOCK_SIZE
	invoke	file.seek,ebx,SEEK_CUR,eax
	invoke	file.write,ebx,[buf],ecx;ini.BLOCK_SIZE
	jmp	@b
  .done:
	mov	eax, [_delta]
	neg	eax
	invoke	file.seek, ebx, SEEK_CUR, eax
	invoke	file.seteof, ebx
;       pop     eax
;       invoke  file.seek, ebx, SEEK_SET;, eax
	stdcall ini._.reload_block, [_f]
	invoke	mem.free, [buf]
	pop	ecx ebx
  .skip:
	ret
  .fail:
	or	eax, -1
	pop	ecx ebx
	ret

  .down:
	neg	[_delta]

	mov	ebx, [_f]
	mov	ecx, [ebx + IniFile.cnt]
	mov	ebx, [ebx + IniFile.fh]
	invoke	file.tell, ebx
;       push    eax
	sub	eax, ecx
	lea	edx, [eax - 1]
	push	edx
    @@: invoke	file.seek, ebx, SEEK_SET, edx
	invoke	file.eof?, ebx
	or	eax, eax
	jnz	@f
	add	edx, ini.BLOCK_SIZE
	jmp	@b
    @@: cmp	edx, [esp]
	je	.skip.2
	add	edx, -ini.BLOCK_SIZE
	cmp	edx, [esp]
	jl	@f
	invoke	file.seek, ebx, SEEK_SET, edx
	invoke	file.read, ebx, [buf], ini.BLOCK_SIZE
	mov	ecx, eax
	mov	eax, [_delta]
	sub	eax, ecx
	invoke	file.seek, ebx, SEEK_CUR, eax
	invoke	file.write, ebx, [buf], ecx
	jmp	@b
    @@:
  .skip.2:
	add	esp, 4
;       mov     eax,[delta]
;       neg     eax
;       invoke  file.seek,ebx,SEEK_CUR,eax
;       pop     eax
;       invoke  file.seek,ebx,SEEK_SET;,eax
	stdcall ini._.reload_block, [_f]
	invoke	mem.free, [buf]
	pop	ecx ebx
	ret
endp

;;================================================================================================;;
proc ini._.get_value_length _f ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	push	ebx ecx edx eax
	mov	ebx, [_f]
	invoke	file.tell, [ebx + IniFile.fh]
	push	esi [ebx + IniFile.cnt] [ebx + IniFile.pos]
	sub	eax, [ebx + IniFile.cnt]
	mov	edx, eax

	stdcall ini._.skip_line, [_f]
	invoke	file.tell, [ebx + IniFile.fh]
	sub	eax, [ebx + IniFile.cnt]
	sub	eax, edx
	mov	[esp + 4 * 3], eax

;       pop     eax
	invoke	file.seek, [ebx + IniFile.fh], SEEK_SET;, eax
	stdcall ini._.preload_block, [_f]
	pop	[ebx + IniFile.cnt] esi
	pop	eax edx ecx ebx
	ret
endp

;;================================================================================================;;
proc ini._.string_copy ;//////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
    @@: lodsb
	or	al, al
	jz	@f
	stosb
	jmp	@b
    @@: ret
endp

;;================================================================================================;;
proc ini._.find_next_section _f ;/////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	push	ebx edi

    @@: stdcall ini._.skip_nonblanks, [_f]
	cmp	al, '['
	je	@f
	or	al, al
	jz	.exit_error
	stdcall ini._.skip_line, [_f]
	or	al, al
	jz	.exit_error
	jmp	@b
    @@:
	pop	edi ebx
	xor	eax, eax
	ret

  .exit_error:
	pop	edi ebx
	or	eax, -1
	ret
endp

;;================================================================================================;;
proc ini._.find_section _f, _sec_name ;///////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Find section in file                                                                           ;;
;? Search is performed from the beginning of file                                                 ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (ok) / -1 (fail)                                                                       ;;
;< [f.pos] = new cursor position (right after ']' char if eax = 0, at the end of file otherwise)  ;;
;;================================================================================================;;
	push	ebx edi

	mov	ecx, [_f]
	invoke	file.seek, [ecx + IniFile.fh], SEEK_SET, 0
	stdcall ini._.preload_block, [_f]

  .next_section:
	stdcall ini._.find_next_section, [_f]
	or	eax, eax
	jnz	.exit_error

	stdcall ini._.get_char, [_f]
;       inc     esi
;       dec     [ecx + IniFile.cnt]
	stdcall ini._.skip_spaces, [_f]
	mov	edi, [_sec_name]
    @@: stdcall ini._.get_char, [_f]
	cmp	al, ']'
	je	@f
	or	al, al
	jz	.exit_error
	cmp	al, 13
	je	.next_section
	cmp	al, 10
	je	.next_section
	scasb
	je	@b
	cmp	byte[edi - 1], 0
	jne	.next_section
	dec	edi
	stdcall ini._.unget_char, [_f]
	stdcall ini._.skip_spaces, [_f]
	stdcall ini._.get_char, [_f]
	cmp	al, ']'
	jne	.next_section
    @@:
	cmp	byte[edi], 0
	jne	.next_section
	pop	edi ebx
	xor	eax, eax
	ret

  .exit_error:
	pop	edi ebx
	or	eax, -1
	ret
endp

;;================================================================================================;;
proc ini._.find_key _f, _key_name ;///////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Find key in section                                                                            ;;
;? Search is performed within current section starting from cursor position                       ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (ok) / -1 (fail)                                                                       ;;
;< [f.pos] = new cursor position (right after '=' char if eax = 0, at the end of file or right    ;;
;<           before '[' char otherwise)                                                           ;;
;;================================================================================================;;
	push	ebx edi

  .next_value:
	mov	edi, [_key_name]
	stdcall ini._.skip_line, [_f]
	stdcall ini._.skip_nonblanks, [_f]
	or	al, al
	jz	.exit_error
	cmp	al, '['
	je	.exit_error
    @@: stdcall ini._.get_char, [_f]
	or	al, al
	jz	.exit_error
	cmp	al, '='
	je	@f
	scasb
	je	@b
	cmp	byte[edi - 1], 0
	jne	.next_value
	dec	edi
	stdcall ini._.unget_char, [_f]
	stdcall ini._.skip_spaces, [_f]
	stdcall ini._.get_char, [_f]
	cmp	al, '='
	je	@f
	jmp	.next_value
    @@:
	cmp	byte[edi], 0
	jne	.next_value

	pop	edi ebx
	xor	eax, eax
	ret

  .exit_error:
	pop	edi ebx
	or	eax, -1
	ret
endp

;;================================================================================================;;
proc ini._.low.read_value _f_addr, _buffer, _buf_len ;////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	push	edi eax
	mov	edi, [_buffer]
	stdcall ini._.skip_spaces, [_f_addr]
    @@: dec	[_buf_len]
	jz	@f
	stdcall ini._.get_char, [_f_addr]
	cmp	al, 13
	je	@f
	cmp	al, 10
	je	@f
	stosb
	or	al, al
	jnz	@b
    @@: stdcall ini._.unget_char, [_f_addr]
	mov	byte[edi], 0
	dec	edi
    @@: cmp	byte[edi], 32
	ja	@f
	mov	byte[edi], 0
	dec	edi
	cmp	edi, [_buffer]
	jae	@b
    @@: pop	eax edi
	ret
endp

;;================================================================================================;;
proc ini.enum_sections _f_name, _callback ;///////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> _callback = callback function address: func(f_name, sec_name)                                  ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
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

	invoke	file.seek, [f.fh], SEEK_SET, 0
	stdcall ini._.preload_block, [f_addr]

  .next_section:
	stdcall ini._.find_next_section, [f_addr]
	or	eax, eax
	jnz	.exit_error

	stdcall ini._.get_char, [f_addr]
	stdcall ini._.skip_spaces, [f_addr]
;       inc     esi
;       dec     [f.cnt]
	mov	edi, [sec_buf]
    @@: stdcall ini._.get_char, [f_addr]
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
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> _callback = callback function address: func(f_name, sec_name, key_name, key_value)             ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
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
	stdcall ini._.find_section, ebx, [_sec_name]
	or	eax, eax
	jnz	.exit_error

  .next_key:
	stdcall ini._.skip_line, [f_addr]
	stdcall ini._.skip_nonblanks, [f_addr]
	or	al, al
	jz	.exit_error
	cmp	al, '['
	je	.exit_error
	mov	edi, [key_buf]
    @@: stdcall ini._.get_char, [f_addr]
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
    @@: stdcall ini._.low.read_value, [f_addr], [val_buf], ini.MAX_VALUE_LEN
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
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
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
	stdcall ini._.find_section, ebx, [_sec_name]
	or	eax, eax
	jnz	.exit_error

	stdcall ini._.find_key, ebx, [_key_name]
	or	eax, eax
	jnz	.exit_error

	stdcall ini._.low.read_value, [f_addr], [_buffer], [_buf_len]
;       mov     edi, [_buffer]
;   @@: dec     [_buf_len]
;       jz      @f
;       stdcall ini.aux.get_char, [f_addr]
;       or      al, al
;       jz      @f
;       cmp     al, 13
;       je      @f
;       cmp     al, 10
;       je      @f
;       stosb
;       jmp     @b
    @@: invoke	file.close, [f.fh]
	invoke	mem.free, [f.buf]
	xor	eax, eax
	stosb
	pop	edi esi ebx
	ret

  .exit_error:
	invoke	file.close, [f.fh]
	invoke	mem.free, [f.buf]
	mov	esi, [_def_val]
	mov	edi, [_buffer]
    @@: lodsb
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
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
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

	stdcall ini._.find_section, ebx, [_sec_name]
	or	eax, eax
	jnz	.create_section

	stdcall ini._.find_key, ebx, [_key_name]
	or	eax, eax
	jnz	.create_key

  .modify_key:

	stdcall ini._.get_value_length, [f_addr]
	sub	eax, [_buf_len]
	stdcall ini._.shift_content, [f_addr], eax

  .modify_key.ex:
	invoke	file.tell, [f.fh]
	sub	eax, [f.cnt]
;       dec     eax
	invoke	file.seek, [f.fh], SEEK_SET, eax
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
	call	ini._.string_copy
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
	stdcall ini._.shift_content, [f_addr], eax

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
	call	ini._.string_copy
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
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
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
	stdcall ini._.find_section, ebx, [_sec_name]
	or	eax, eax
	jnz	.exit_error

	stdcall ini._.find_key, ebx, [_key_name]
	or	eax, eax
	jnz	.exit_error

	stdcall ini._.skip_nonblanks, [f]
	xor	eax, eax
	xor	ebx, ebx
	xor	edx, edx
	stdcall ini._.get_char, [f_addr]
	cmp	al, '-'
	jne	.lp1
	inc	bh
    @@: stdcall ini._.get_char, [f_addr]
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
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
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
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Imported functions section                                                                     ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;


align 16
@IMPORT:

library \
	libfile , 'libio.obj'

import	libfile 		    , \
	file.size   , 'file.size'   , \ ; f_name
	file.open   , 'file.open'   , \ ; f_name  f_mode
	file.read   , 'file.read'   , \ ; f_descr buffer   buf_len
	file.write  , 'file.write'  , \ ; f_descr buffer   buf_len
	file.seek   , 'file.seek'   , \ ; f_descr f_origin f_where
	file.eof?   , 'file.eof?'   , \ ; f_descr
	file.seteof , 'file.seteof' , \ ; f_descr
	file.tell   , 'file.tell'   , \ ; f_descr
	file.close  , 'file.close'	; f_descr


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
	lib_init	  , 'lib_init'		, \
	0x00040005	  , 'version'		, \
	ini.enum_sections , 'ini.enum_sections' , \
	ini.enum_keys	  , 'ini.enum_keys'	, \
	ini.get_str	  , 'ini.get_str'	, \
	ini.get_int	  , 'ini.get_int'	, \
	ini.set_str	  , 'ini.set_str'	, \
	ini.set_int	  , 'ini.set_int'
