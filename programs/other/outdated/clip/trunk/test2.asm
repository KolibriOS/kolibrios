; <--- description --->
; compiler:     FASM
; name:         Clipboard test with GUI.
; version:      0.1
; author:       barsuk
; comments: 	uses edit_box by Maxxxx32,<Lrz> (old static version)


; <--- include all MeOS stuff --->
include "lang.inc"
include "..\..\..\macros.inc"

; <--- start of MenuetOS application --->
MEOS_APP_START

SEND_DELAY = 10
RECV_DELAY = 100
ATTEMPT = 5


define DEBUG TRUE
include "bdebug.inc"

include "editbox.inc"

include "clip.inc"

use_edit_box

; <--- start of code --->
CODE

    call    clipboard_init

    call    draw_window            ; at first create and draw the window

wait_event:                      ; main cycle

	mov	edi, input_box
	call	edit_box.mouse

    mov     eax, 10
    int     0x40

    cmp     eax, 1                 ;   if event == 1
    je      redraw                 ;     jump to redraw handler
    cmp     eax, 2                 ;   else if event == 2
    je      key                    ;     jump to key handler
    cmp     eax, 3                 ;   else if event == 3
    je      button                 ;     jump to button handler

    jmp     wait_event             ;   else return to the start of main cycle


redraw:                          ; redraw event handler
    call    draw_window
    jmp     wait_event


key:                             ; key event handler
	mov     eax, 2                 ;   get key code
	int     0x40

	cmp	ah, '0'
	jb	no_digit
	cmp	ah, '9'
	ja	no_digit
	sub	ah, '0'
	mov	byte [format_id], ah
	call	draw_window
	jmp	wait_event
no_digit:

	mov	edi, input_box
	call	edit_box.key

	jmp     wait_event


  button:                          ; button event handler
    mov     eax, 17                ;   get button identifier
    int     0x40

    cmp     ah, 1
    jz      exit

	cmp	ah, 5
	jz	copy
	cmp	ah, 6
	jz	paste
	jmp	wait_event
copy:
	mov	esi, input_text
	mov	edi, buffer.data
	mov	ecx, [buffer.size]
	rep	movsb			; copy text to buffer

	mov	edi, input_box
	mov	edx, [edi + 38]		; ed_size
	xchg	edx, [buffer.size]
	movzx	eax, byte [format_id]
	mov	esi, buffer
	call	clipboard_write
	xchg	edx, [buffer.size]
	jmp	wait_event

paste:
	mov	esi, buffer
	movzx	eax, byte [format_id]
	mov	edx, 7
	call	clipboard_read

	or	eax, eax
	jz	wait_event

	cmp	eax, -1
	jz	wait_event

	or	edx, edx
	jz	wait_event		; это все ошибки

	mov	eax, input_box
;print "input box"
;dph1	[eax]
;dph1	[eax+4]
;dph1	[eax+8]
;dph1	[eax+12]
;dph1	[eax+16]
;dph1	[eax+20]
;dph1	[eax+24]

;;;;jmp	wait_event

	mov	word [input_box + 36], ed_focus	; flags

	mov	ecx, [input_box + 38]	; size
print "paste read ecx=size"
	jecxz	.noloop
	mov	edi, input_box
	mov	ah, 8		; backspace
.loop:
	call	edit_box.key
	dec	ecx
	jnz	.loop
.noloop:
	mov	esi, buffer.data
	mov	ecx, edx
pregs
.loop2:
	mov	ah, [esi]
	cmp	ah, 0
	jz	.done
	call	edit_box.key	; я бы себе руки отрезал за такое
				; но что делать, если иначе не получается?
	inc	esi
	dec	ecx
	jnz	.loop2

.done:
print "rest of data ecx"
pregs
	call	draw_window
	jmp	wait_event

exit:
    or      eax, -1                ;   exit application
    int     0x40


  draw_window:
    mov     eax, 12                ; start drawing
    mov     ebx, 1
    int     0x40

    mov     eax, 0                 ; create and draw the window
    mov     ebx, 100*65536+400     ;   (window_cx)*65536+(window_sx)
    mov     ecx, 100*65536+250     ;   (window_cy)*65536+(window_sy)
    mov     edx, 0x33ffffff        ;   work area color & window type 3
    mov	    edi, head
    int     0x40

	mov	eax, 4
	mov	ebx, 10 shl 16 + 10
	mov	ecx, 0x80000000
	mov	edx, text1
	int	0x40
	mov	eax, 4
	mov	ebx, 10 shl 16 + 20
	mov	edx, text2
	int	0x40
	mov	eax, 47
	mov	ebx, 0x00020000
	movzx	ecx, byte [format_id]
	mov	edx, 200 shl 16 + 20
	mov	esi, 0
	int	0x40

	mov	eax, 8
	mov	ebx, 10 shl 16 + 60
	mov	ecx, 40 shl 16 + 15
	mov	edx, 5
	mov	esi, 0xd72189
	int	0x40
	mov	eax, 4
	mov	ebx, 12 shl 16 + 42
	mov	ecx, 0x80000000
	mov	edx, button1
	int	0x40

	mov	eax, 8
	mov	ebx, 80 shl 16 + 60
	mov	ecx, 40 shl 16 + 15
	mov	edx, 6
	mov	esi, 0xd72189
	int	0x40
	mov	eax, 4
	mov	ebx, 82 shl 16 + 42
	mov	ecx, 0x80000000
	mov	edx, button2
	int	0x40

	mov	edi, input_box
	call	edit_box.draw

;	mov	eax, 4
;	mov	ebx, 10 shl 16 + 90
;	mov	ecx, 0
;	mov	edx, input_text
;	mov	esi, [input_box + 38]
;	int	0x40
;	mov	eax, esi
;print "eax = edit etxt len"
;pregs



    mov     eax, 12                ; finish drawing
    mov     ebx, 2
    int     0x40

  ret



; <--- initialised data --->
DATA

	format_id	db	1

;	buffer		db	256 dup(0)

	CLIP_BUFFER	buffer, 256

	input_text	db	256 dup(0)
	input_box	edit_box 100,10,70,0xffffff,0,0x00aaaaaa,0,255,input_text

;	input_box	edit_box 100,10,70,0xffffff,0,0xaaaaaa,0,\
;			0xaaaaaa,0,255,input_text,ed_always_focus

	head	db	"Пример работы с буфером обмена",0
	text1	db	"Нажимайте цифры 0-9 для смены id формата данных",0
	text2	db	"Сейчас выбран id формата данных: ",0

	button1	db	"Копировать",0
	button2	db	"Вставить",0

; <--- uninitialised data --->
UDATA


MEOS_APP_END
; <--- end of MenuetOS application --->
