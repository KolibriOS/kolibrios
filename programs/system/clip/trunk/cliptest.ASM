; <--- description --->
; compiler:     FASM
; name:         Clipboard Client Demo
; version:      0.1
; author:       barsuk


; <--- include all MeOS stuff --->
include "lang.inc"
include "..\..\..\MACROS.INC"

TEST_SIZE = 100000	; должно делиться на 4

IPC_buffer_size = TEST_SIZE + 0x10
MAX_BUFFER = TEST_SIZE + 0x10

SEND_DELAY = 10
RECV_DELAY = 100

ATTEMPT = 5		; число попыток отправить сообщение демону, если
			; он занят

; <--- start of MenuetOS application --->
MEOS_APP_START

define DEBUG TRUE
include "bdebug.inc"

include "clip.inc"

; <--- start of code --->
CODE


;;;	This is the entry point of the program

begin:

print	"clipboard testing module loaded"

	mov	eax, 5
	mov	ebx, 100		; 5 sec
	int	0x40

	call	clipboard_init
	or	eax, eax
	jz	.err

	; put the data into clip_buffer

	mov	esi, test_line
	mov	edi, cb.data
	mov	ecx, cb.size
	rep	movsb

print	"init ok. @clip found."

	mov	esi, cb
	mov	eax, 2
	call	clipboard_write
	or	eax, eax
	jz	.err

print 	"write test ok."

; слегка очистим буфер, для надежности проверки

	mov	dword [cb.data], 0

;jmp .exit
	mov	eax, 3			; try to read bad data
	mov	esi, cb
	mov	edx, 7
	call	clipboard_read
	or	eax, eax
	jz	.bad2
	cmp	eax, -1
	jz	.err

.bad2:
print "unset data read ok 1."
	mov	eax, 88			; -"-
	mov	esi, cb
	mov	edx, 7
	call	clipboard_read
	or	eax, eax
	jz	.bad3
	cmp	eax, -1
	jz	.err

.bad3:
print "unset data read ok 2."

.read:
	mov	eax, 2
	mov	esi, cb
	mov	edx, 7
	call	clipboard_read		; read the string we've written
	or	eax, eax
	jz	.err
	cmp	eax, -1
	jz	.err

print	"read ok o_O, data:"
outs cb.data
print ""

	mov	eax, 5
	mov	ebx, 200
	int	0x40


; последняя проверка - на ресайз буфера. а именно, возьмем много данных
; и прокачаем их через буфер

	mov	eax, 68	
	mov	ebx, 11
	int	0x40

	mov	eax, 68	
	mov	ebx, 12
	mov	ecx, TEST_SIZE + 12
	int	0x40
	mov	[mega_buf], eax

	mov	dword [eax], TEST_SIZE

	lea	edx, [eax + 12]
	mov	eax, 12131415
	shr	ecx, 2
.loop:
	mov	[edx], eax
	add	edx, 4
	dec	ecx
	jnz	.loop

print "mega buffer allocated and filled"

	; now write it

	mov	esi, [mega_buf]
	mov	eax, 69
	call	clipboard_write
	or	eax, eax
	jz	.err

print "mega buffer written"
	mov	eax, 5
	mov	ebx, 100
	int	0x40

	; clear it.

	mov	edx, [mega_buf]
	add	edx, 12
	xor	eax, eax
	mov	ecx, TEST_SIZE/4	
.loopc:
	mov	[edx], eax
	add	edx, 4
	dec	ecx
	jnz	.loopc

	; now read
	
	mov	eax, 69
	mov	esi, [mega_buf]
	mov	edx, 7
	call	clipboard_read		; read the string we've written
	or	eax, eax
	jz	.err
	cmp	eax, -1
	jz	.err

print "mega buf read"

	; now compare

	mov	edx, [mega_buf]
	add	edx, 12
	mov	eax, 12131415
	mov	ecx, TEST_SIZE/4
.loopm:
	cmp	eax, [edx]
	jnz	.err
	add	edx, 4
	dec	ecx
	jnz 	.loopm

print "deleting mega buf"

	mov	eax, 69
	call	clipboard_delete
	or	eax, eax
	jz	.err

	mov	eax, 5
	mov	ebx, 100
	int	0x40

print "if read error then all is ok"

	mov	eax, 69
	mov	esi, [mega_buf]
	mov	edx, 7
	call	clipboard_read		; read the string we've written
	or	eax, eax
	jz	.reading
	jmp	.err

.reading:	

print "test complete!"

.exit:
	or	eax, -1
	int	0x40

.err:
print	"error!"
	jmp	.exit


; <--- initialised data --->
DATA

	CLIP_BUFFER	cb, 256

;	in_buf		db	256 dup(0)
;	out_buf		db 	'Cthulhu Fhtagn',0

	test_line 	db	'Cthulhu Fhtagn',0
	mega_buf	dd	0

; <--- uninitialised data --->
UDATA


MEOS_APP_END
; <--- end of MenuetOS application --->