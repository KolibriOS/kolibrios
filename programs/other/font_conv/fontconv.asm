;;===================================================================
;; Copyright 2011 dunkaist <dunkaist@gmail.com>
;; Distributed under the terms of the GNU General Public License v3.
;; See http://www.gnu.org/licenses/gpl.txt for the full license text.
;;-------------------------------------------------------------------

CHAR1TXT_FILE_SIZE	equ	20478
CHAR2TXT_FILE_SIZE	equ	25598

CHAR1MT_FILE_SIZE	equ	2304
CHAR2MT_FILE_SIZE	equ	2560

FONT_HEIGHT		equ	9
FONT_WIDTH_MONO		equ	5
FONT_WIDTH_VAR		equ	7


use32
	org	0x0
	db	'MENUET01'
	dd	0x01, start, i_end, e_end, stacktop, 0x0, cur_dir

include '../../proc32.inc'
include '../../macros.inc'


start:
	mcall	68, 11

	cld
	mov	ecx, 4096
	mov	edi, cur_dir
	xor	al, al
	repnz	scasb
	std
	mov	al, '/'
	repnz	scasb
	add	edi, 2
	cld
	mov	[cur_dir_slash], edi
	mov	esi, _char1.txt
	mov	ecx, 10/2
	rep	movsw

monospace:
	mcall	70, func70			; get file info
	test	eax, eax
	jnz	quit

	mov	ecx, dword[file_info.Size]
	cmp	ecx, CHAR1TXT_FILE_SIZE
	jl	quit
	mov	[func70.bytes_to_read], ecx
	mcall	68, 12, 			; allocate memory for char.txt
	test	eax, eax
	jz	quit
	mov	[func70.destination], eax

	mov	[func70.func_number], 0		; read from file
	mcall	70, func70
	test	eax, eax
	jnz	freemem_quit


	xor	dl, dl
	mov	esi, [func70.destination]
	mov	edi, esi			; yes, that's a feature
	mov	bl, FONT_WIDTH_MONO
  .char:
	add	esi, 8
	call	do_symbol
	dec	dl
	jnz	.char

	mov	[func70.func_number], 2		; create/overwrite file
	mov	[func70.bytes_to_read], CHAR1MT_FILE_SIZE
	mov	edi, [cur_dir_slash]
	mov	esi, _char1.mt
	mov	ecx, 10/2
	rep	movsw
	mcall	70, func70
	test	eax, eax
	jnz	freemem_quit


varspace:
	push	[func70.destination]
	push	file_info
	pop	[func70.destination]
	mov	[func70.func_number], 5		; get file info
	mov	edi, [cur_dir_slash]
	mov	esi, _char2.txt
	mov	ecx, 10/2
	rep	movsw
	mcall	70, func70
	pop	[func70.destination]
	test	eax, eax
	jnz	freemem_quit

	mov	ecx, dword[file_info.Size]
	cmp	ecx, CHAR2TXT_FILE_SIZE
	jl	freemem_quit
	mov	[func70.bytes_to_read], ecx
	mcall	68, 20, 			; realloc memory
	test	eax, eax
	jz	freemem_quit
	mov	[func70.destination], eax

	mov	[func70.func_number], 0		; read from file
	mcall	70, func70
	test	eax, eax
	jnz	freemem_quit


	xor	dl, dl
	mov	esi, [func70.destination]
	mov	edi, esi			; yes, that's a feature
	mov	bl, FONT_WIDTH_VAR
  .char:
	add	esi, 6
	lodsb
	cmp	al, ' '			; space means default symbol width (8)
	jnz	@f
	mov	al, 8+47
    @@:	sub	al, 47
	stosb
	add	esi, 3
	call	do_symbol
	dec	dl
	jnz	.char


	mov	[func70.func_number], 2		; create/overwrite file
	mov	[func70.bytes_to_read], CHAR2MT_FILE_SIZE
	mov	edi, [cur_dir_slash]
	mov	esi, _char2.mt
	mov	ecx, 10/2
	rep	movsw
	mcall	70, func70
	test	eax, eax
	jnz	freemem_quit

freemem_quit:
	mcall	68, 13, [func70.destination]
quit:
	mcall	-1


proc do_symbol
	mov	ch, FONT_HEIGHT
  .col:	xor	ah, ah
	xor	cl, cl
  .row:	lodsb
	cmp	al, ' '
	setnz	al
	shl	al, cl
	or	ah, al
	inc	cl
	cmp	cl, bl
	jnz	.row
	add	esi, 3
	mov	al, ah
	stosb
	dec	ch
	jnz	.col
	ret
endp

_char1.txt	db 'char.txt',0
_char2.txt	db 'char2.txt',0
_char1.mt	db 'char.mt',0
_char2.mt	db 'char2.mt',0

func70:
  .func_number		dd 5		; function number
  .position		dd 0		; position in file
  .reserved		dd 0		; reserved
  .bytes_to_read	dd 0		; bytes to read
  .destination		dd file_info
  .flag			db 0		; see file name later
  .file_name		dd cur_dir	; pointer to the name of file (ASCIIZ)

i_end:
file_info	FILEINFO
cur_dir		rb 4096
cur_dir_slash	rd 1			; we save here position of the last '/' symbol
stack_here	rb 0x200
stacktop:
e_end:
