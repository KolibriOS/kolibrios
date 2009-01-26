;;================================================================================================;;
;;//// libini_p.asm //// (c) mike.dld, 2006-2008 /////////////////////////////////////////////////;;
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
proc libini._.init ;//////////////////////////////////////////////////////////////////////////////;;
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
proc libini._.unget_char _f ;/////////////////////////////////////////////////////////////////////;;
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
	stdcall libini._.unload_block, [_f]
    @@: ;mov     al,[esi-1]
	pop	ecx eax
	ret
endp

;;================================================================================================;;
proc libini._.get_char _f ;///////////////////////////////////////////////////////////////////////;;
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
	stdcall libini._.preload_block, [_f]
	dec	[ecx + IniFile.cnt]
    @@: lodsb
	ret
endp

;;================================================================================================;;
proc libini._.skip_nonblanks _f ;/////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	mov	ecx, [_f]
    @@: stdcall libini._.get_char, [_f]
	cmp	al, 32
	je	@b
	cmp	al, 13
	je	@b
	cmp	al, 10
	je	@b
	cmp	al, 9
	je	@b
    @@: stdcall libini._.unget_char, [_f]
	;inc     [ecx+INIFILE.cnt]
	ret
endp

;;================================================================================================;;
proc libini._.skip_spaces _f ;////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	mov	ecx, [_f]
    @@: stdcall libini._.get_char, [_f]
	cmp	al, 32
	je	@b
	cmp	al, 9
	je	@b
    @@: stdcall libini._.unget_char, [_f]
	;inc     [ecx+INIFILE.cnt]
	ret
endp

;;================================================================================================;;
proc libini._.skip_line _f ;//////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	mov	ecx, [_f]
    @@: stdcall libini._.get_char, [_f]
	or	al, al
	jz	@f
	cmp	al, 13
	je	@f
	cmp	al, 10
	jne	@b
    @@: stdcall libini._.unget_char, [_f]
	;inc     [ecx+INIFILE.cnt]
	ret
endp

;;================================================================================================;;
proc libini._.unload_block _f ;///////////////////////////////////////////////////////////////////;;
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
	invoke	file.seek, [ebx + IniFile.fh], eax, SEEK_SET
	stdcall libini._.preload_block, ebx
	add	esi, eax ; ini.BLOCK_SIZE
	mov	[ebx + IniFile.cnt], 0
	pop	ecx ebx eax
	ret
endp

;;================================================================================================;;
proc libini._.preload_block _f ;//////////////////////////////////////////////////////////////////;;
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
proc libini._.reload_block _f ;///////////////////////////////////////////////////////////////////;;
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
	invoke	file.seek, [ebx + IniFile.fh], [ebx + IniFile.pos], SEEK_SET
	stdcall libini._.preload_block, ebx
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
proc libini._.shift_content _f, _delta ;//////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Shift file content starting from cursor position (~ delete)                                    ;;
;? Content is copied by 'delta' bytes up/down                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (fail) / 0 (ok)                                                                       ;;
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
	invoke	file.seek, ebx, eax, SEEK_SET
    @@: invoke	file.seek, ebx, [_delta], SEEK_CUR
	invoke	file.eof?, ebx
	or	eax, eax
	jnz	.done
	invoke	file.read, ebx, [buf], ini.BLOCK_SIZE
	mov	ecx, eax
	mov	eax, [_delta]
	neg	eax
	sub	eax, ecx;ini.BLOCK_SIZE
	invoke	file.seek, ebx, eax, SEEK_CUR
	invoke	file.write, ebx, [buf], ecx;ini.BLOCK_SIZE
	jmp	@b
  .done:
	mov	eax, [_delta]
	neg	eax
	invoke	file.seek, ebx, eax, SEEK_CUR
	invoke	file.seteof, ebx
;       pop     eax
;       invoke  file.seek, ebx, SEEK_SET;, eax
	stdcall libini._.reload_block, [_f]
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
    @@: invoke	file.seek, ebx, edx, SEEK_SET
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
	invoke	file.seek, ebx, edx, SEEK_SET
	invoke	file.read, ebx, [buf], ini.BLOCK_SIZE
	mov	ecx, eax
	mov	eax, [_delta]
	sub	eax, ecx
	invoke	file.seek, ebx, eax, SEEK_CUR
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
	stdcall libini._.reload_block, [_f]
	invoke	mem.free, [buf]
	pop	ecx ebx
	ret
endp

;;================================================================================================;;
proc libini._.get_value_length _f ;///////////////////////////////////////////////////////////////;;
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

	stdcall libini._.skip_line, [_f]
	invoke	file.tell, [ebx + IniFile.fh]
	sub	eax, [ebx + IniFile.cnt]
	sub	eax, edx
	mov	[esp + 4 * 3], eax

	pop	eax
	invoke	file.seek, [ebx + IniFile.fh], eax, SEEK_SET
	stdcall libini._.preload_block, [_f]
	pop	[ebx + IniFile.cnt] esi
	pop	eax edx ecx ebx
	ret
endp

;;================================================================================================;;
proc libini._.string_copy ;///////////////////////////////////////////////////////////////////////;;
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
proc libini._.find_next_section _f ;//////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	push	ebx edi

    @@: stdcall libini._.skip_nonblanks, [_f]
	cmp	al, '['
	je	@f
	or	al, al
	jz	.exit_error
	stdcall libini._.skip_line, [_f]
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
proc libini._.find_section _f, _sec_name ;////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Find section in file                                                                           ;;
;? Search is performed from the beginning of file                                                 ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (fail) / 0 (ok)                                                                       ;;
;< [_f.pos] = new cursor position (right after ']' char if eax = 0, at the end of file otherwise) ;;
;;================================================================================================;;
	push	ebx edi

	mov	ecx, [_f]
	invoke	file.seek, [ecx + IniFile.fh], 0, SEEK_SET
	stdcall libini._.preload_block, [_f]

  .next_section:
	stdcall libini._.find_next_section, [_f]
	or	eax, eax
	jnz	.exit_error

	stdcall libini._.get_char, [_f]
;       inc     esi
;       dec     [ecx + IniFile.cnt]
	stdcall libini._.skip_spaces, [_f]
	mov	edi, [_sec_name]
    @@: stdcall libini._.get_char, [_f]
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
	stdcall libini._.unget_char, [_f]
	stdcall libini._.skip_spaces, [_f]
	stdcall libini._.get_char, [_f]
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
proc libini._.find_key _f, _key_name ;////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Find key in section                                                                            ;;
;? Search is performed within current section starting from cursor position                       ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = -1 (fail) / 0 (ok)                                                                       ;;
;< [_f.pos] = new cursor position (right after '=' char if eax = 0, at the end of file or right   ;;
;<            before '[' char otherwise)                                                          ;;
;;================================================================================================;;
	push	ebx edi

  .next_value:
	mov	edi, [_key_name]
	stdcall libini._.skip_line, [_f]
	stdcall libini._.skip_nonblanks, [_f]
	or	al, al
	jz	.exit_error
	cmp	al, '['
	je	.exit_error
    @@: stdcall libini._.get_char, [_f]
	or	al, al
	jz	.exit_error
	cmp	al, '='
	je	@f
	scasb
	je	@b
	cmp	byte[edi - 1], 0
	jne	.next_value
	dec	edi
	stdcall libini._.unget_char, [_f]
	stdcall libini._.skip_spaces, [_f]
	stdcall libini._.get_char, [_f]
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
proc libini._.low.read_value _f_addr, _buffer, _buf_len ;/////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	push	edi eax
	mov	edi, [_buffer]
	stdcall libini._.skip_spaces, [_f_addr]
    @@: dec	[_buf_len]
	jz	@f
	stdcall libini._.get_char, [_f_addr]
	cmp	al, 13
	je	@f
	cmp	al, 10
	je	@f
	stosb
	or	al, al
	jnz	@b
    @@: stdcall libini._.unget_char, [_f_addr]
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
proc libini._.str_to_int ;////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> esi = string buffer address                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = binary number representation (no overflow checks made)                                   ;;
;;================================================================================================;;
	push	edx

	xor	eax, eax
	xor	edx, edx

    @@: lodsb
	cmp	al, '0'
	jb	@f
	cmp	al, '9'
	ja	@f
	add	eax, -'0'
	imul	edx, 10
	add	edx, eax
	jmp	@b

    @@: dec	esi
	mov	eax, edx
	pop	edx
	ret
endp

;;================================================================================================;;
proc libini._.int_to_str ;////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? --- TBD ---                                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> eax = number to convert                                                                        ;;
;> ecx = base                                                                                     ;;
;> edi = string buffer address                                                                    ;;
;;------------------------------------------------------------------------------------------------;;
;< --- TBD ---                                                                                    ;;
;;================================================================================================;;
	push	ecx edx

	or	eax, eax
	jns	@f
	mov	byte[edi], '-'
	inc	edi
    @@: call	.recurse
	pop	edx ecx
	ret

  .recurse:
	cmp	eax,ecx
	jb	@f
	xor	edx,edx
	div	ecx
	push	edx
	call	.recurse
	pop	eax
    @@: cmp	al,10
	sbb	al,0x69
	das
	stosb
	retn
endp
