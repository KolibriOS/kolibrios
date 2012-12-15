use32
    org 0x0
    db  'MENUET01'
    dd  0x01,start,i_end,e_end,e_end,0,0

include '../../../../proc32.inc'
include '../../../../macros.inc'
include '../../../../dll.inc'
;include '../../../../debug.inc'
include '../../../../develop/libraries/libs-dev/libio/libio.inc'
include 'libcrash.inc'
BUFFER_SIZE	= 8192


start:
	mcall	68, 11

	stdcall dll.Load, @IMPORT
	or	eax, eax
	jnz	quit

	invoke	file.open, input_file, O_READ
	or	eax, eax
	jz	quit
	mov	[fh], eax

	invoke	file.size, input_file
	mov	[file_len], ebx

	stdcall mem.Alloc, ebx
	or	eax, eax
	jz	quit
	mov	[file_data], eax

	invoke	file.read, [fh], eax, [file_len]
	cmp	eax, -1
	je	quit
	cmp	eax, [file_len]
	jne	quit

	invoke	file.close, [fh]
	inc	eax
	jz	quit
	
	stdcall	mem.Free, [file_data]
	test	eax, eax
	jz	quit


	invoke	crash.hash, LIBCRASH_SHA512, hash, data_buffer, 0, update_data_buffer, msglen
	invoke	crash.bin2hex, hash, hex, LIBCRASH_SHA512


still:
	mcall	10
	dec	eax
	jz	redraw
	dec	eax
	jz	key

button:
	mcall	17
	shr	eax, 8

	cmp	eax, 1
	je	quit

redraw:
	mcall	12, 1
	mcall	0, <0,800>, <0,100>, 0x34000000, 0x80000000, window_title

	mcall	4, 0, 0x40ffffff, hex, 128, 0

	mcall	12, 2
	jmp	still

key:
	mcall	2
	jmp	still


quit:
	mcall	-1


proc update_data_buffer
	mcall	70, f70_buf
	mov	eax, ebx
	cmp	eax, -1
	jne	@f
	inc	eax
    @@:
	add	dword[f70_buf + 4], BUFFER_SIZE
	ret
endp


szZ window_title		,'libcrash example'

sz msg_few_args		, '2 arguments required',0x0a
sz msg_bad_hash_type	, 'invalid hash type',0x0a
sz msg_file_not_found	, 'file not found: '

input_file	db '/hd0/1/crashtest',0

f70_buf:
	funcnum	dd 0
	src	dd 0
	res1	dd 0
	count	dd BUFFER_SIZE
	dst	dd data_buffer
	res2	db 0
	fname	dd input_file

align 4
@IMPORT:

library				  \
	libio	, 'libio.obj'	, \
	libcrash, 'libcrash.obj'

import	libio				, \
	libio.init     , 'lib_init'	, \
	file.size      , 'file_size'	, \
	file.open      , 'file_open'	, \
	file.read      , 'file_read'	, \
	file.close     , 'file_close'

import	libcrash			, \
	libcrash.init  , 'lib_init'	, \
	crash.hash     , 'crash_hash'	, \
	crash.bin2hex  , 'crash_bin2hex'

i_end:
hash		rd 16
msglen		rd 1
fd		rd 1
fh		rd 1
data_length	rd 1
hex		rb 1024
data_buffer	rb BUFFER_SIZE
file_data	rd 1
file_len	rd 1

rb 0x400					;stack
e_end:

