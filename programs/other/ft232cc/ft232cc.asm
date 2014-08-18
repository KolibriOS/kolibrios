;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  FT232 SyncBB mode demonstration for KolibriOS               ;;
;;                                                              ;;
;;   Written by gtament@gmail.com                               ;;
;;                                                              ;;
;;         GNU GENERAL PUBLIC LICENSE                           ;;
;;          Version 2, June 1991                                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""
use32
org 0x0

; The header

db 'MENUET01'
dd 0x01
dd INIT
dd I_END
dd 0x100000
dd 0x7fff0
dd 0, 0

; The code area

include '../../macros.inc'
include '../../debug.inc'

struct IOCTL
handle			dd	?
io_code 		dd	?
input			dd	?
inp_size		dd	?
output			dd	?
out_size		dd	?
ends

BUTTONXSIZE = 60
BUTTONYSIZE = 60
BUTTONXSTART= 20
BUTTONYSTART= 55
BUTTONXSPACE= 10
BUTTONCOUNT = 8
ONCOLOR     = 0x0018c015
OFFCOLOR    = 0x00db2521

;Flags
PIN_CONF_FLAG = 0

INIT:
	mov	    [btn_state], 0xFF
	mov	    [btn_io], 0xFF

	mcall	68, 11
	call	draw_window
	mov	    edi, drv_name
	mcall	68, 16, edi
    cmp     eax, 0
    jnz     @f
    debug_print 'Error while loading driver\n'
  @@:
    mov     [IOCTLs+IOCTL.handle], eax
    mov     eax, 8
    mov     [IOCTLs+IOCTL.inp_size], eax
    mov     [IOCTLs+IOCTL.out_size], eax
    mov 	eax, out_buf
    mov 	[IOCTLs+IOCTL.output], eax
    mov 	eax, in_buf
    mov 	[IOCTLs+IOCTL.input], eax

    call    drivercomm.getlist
    test    eax, eax
    jnz     @f
    mcall   4, 5 shl 16 + 25, 1 shl 31 + 0xFF shl 16, noftdi_msg
  @@:
    call    drivercomm.getlock
    mov     eax, [pid]
    cmp     eax, [out_buf]
    jz	    @f
    mcall   4, 5 shl 16 + 25, 1 shl 31 + 0xFF shl 16, devlkd_msg
    jmp     event_wait
  @@:
    mcall   4, 5 shl 16 + 25, 1 shl 31, welcio_msg
    mov     [flags], 1 shl PIN_CONF_FLAG
    call    draw_window.no_redraw
    jmp     event_wait
  .init_cont:
    call    draw_window
    mcall   4, 5 shl 16 + 25, 1 shl 31 + ONCOLOR, devrdy_msg

event_wait:
	mov	    eax, 10
	mcall

	cmp	    eax, 1			; Event redraw request ?
	je	    redraw

	cmp	    eax, 2			; Event key in buffer ?
	je	    key

	cmp	    eax, 3			; Event button in buffer ?
	je	    button

	jmp	    event_wait

redraw:
	call	draw_window
	jmp	    event_wait

key:
	mcall	2
    cmp     ah, '8'
    jg	    event_wait
    cmp     ah, '1'
    jl	    event_wait
    sub     ah, 0x2F
    jmp     button.noclose

button:
	mov	    eax,17
	mcall

	cmp	    ah, 1
	jg	    .noclose
    call    drivercomm.unlock
	mov	    eax, -1
	mcall

 .noclose:
	cmp	    ah, 10
	jge	    .toggleb
	mov	    ebx, [flags]
	and 	ebx, 1 shl PIN_CONF_FLAG
	jnz	    event_wait
	movzx	edx, ah
	dec	    ah
	dec	    ah
	mov	    cl, ah
	mov	    ah ,1
	shl	    ah, cl
	mov	    al, [btn_io]
	and	    al, ah
	jz	    .input
	xor	    byte[btn_state], ah
  .input:
    call    draw_window.oloop_entry
    call    drivercomm.write
    call    drivercomm.read
    ;call    drivercomm.pins
	jmp	    event_wait

  .toggleb:
    cmp     ah, 19
    jg	    .pinconf
    movzx	edx, ah
	mov	    cl, ah
	sub	    cl, 10
	mov	    ah, 1
	shl	    ah, cl
	xor	    byte[btn_io], ah
	call	draw_tbutton
	jmp	    event_wait

  .pinconf:
    cmp     ah, 20
    mov     eax, [flags]
    and     eax, 1 shl PIN_CONF_FLAG
    jz	    .no_conf
    call    drivercomm.bitmode
    call    drivercomm.baud
  .no_conf:
    xor     [flags], 1 shl PIN_CONF_FLAG
    jmp     INIT.init_cont

drivercomm:				;Driver communication procs
  .baud:
    mov     [IOCTLs+IOCTL.io_code], 10
    mov     edi, in_buf
    mov     eax, [pid]
    mov     [edi], eax
    mov     eax, [dev]
    mov     [edi+4], eax
    mov     [edi+8], dword 9600
    mcall   68, 17, IOCTLs
    ret

  .write:
    mov     [IOCTLs+IOCTL.io_code], 8
    mov     edi, in_buf
    mov     eax, [pid]
    mov     [edi], eax
    mov     eax, [dev]
    mov     [edi+4], eax
    mov     [edi+8], dword 1
    mov     al, [btn_state]
    mov     ecx, 4
    add     edi, 12
    rep     stosb
    mcall   68, 17, IOCTLs
    ret

  .read:
    mov     [IOCTLs+IOCTL.io_code], 9
    mov     edi, in_buf
    mov     eax, [pid]
    mov     [edi], eax
    mov     eax, [dev]
    mov     [edi+4], eax
    mov     [edi+8], dword 3
    mcall   68, 17, IOCTLs
    test    eax, eax
    jz	    .read_ok
    debug_print 'Error'
    newline
 .read_ok:
    mov     al, byte[out_buf+2]
    mov     [btn_state], al
    ret

  .getlock:
    mcall	9, thread_inf, 0
    mov     eax, dword[thread_inf+30]
    mov     [pid], eax
    mov     edi, in_buf
    mov     [edi], eax
    mov     eax, 2
    mov     [IOCTLs+IOCTL.io_code], eax
    mov     eax, [dev]
    mov     [edi+4], eax
    mov     eax, 8
    mov     [IOCTLs+IOCTL.inp_size], eax
    mcall   68, 17, IOCTLs
    ret

  .getlist:
    mov     eax, 1
    mov     [IOCTLs+IOCTL.io_code], eax
    mcall   68, 17, IOCTLs
    mov     edi, out_buf
    mov     eax, [edi+12]
    mov     [dev], eax
    mov     eax, [edi]
    ret

  .bitmode:
	mov		[IOCTLs+IOCTL.io_code], 11
    mov     edi, in_buf
	mov	    eax, [pid]
	mov	    [edi], eax
	mov	    eax, [dev]
	mov	    [edi+4], eax
	xor	    eax, eax
	mov	    ah, [btn_io]
	mov	    al, 0x01
	mov	    [edi+8], eax
    mcall   68, 17, IOCTLs
    ret

  .pins:
    mov [IOCTLs+IOCTL.io_code], 22
    mov edi, in_buf
    mov eax, [pid]
    mov [edi], eax
    mov eax, [dev]
    mov [edi+4], eax
    mcall 68, 17, IOCTLs
    mov al, byte[out_buf]
    mov [btn_state], al
    ret

  .unlock:
    mov     eax, [pid]
    mov     edi, in_buf
    mov     [edi], eax
    mov     eax, 3
    mov     [IOCTLs+IOCTL.io_code], eax
    mov     eax, [dev]
    mov     [edi+4], eax
    mov     eax, 8
    mov     [IOCTLs+IOCTL.inp_size], eax
    mcall   68, 17, IOCTLs
    ret

;  *********************************************
;  ******  WINDOW DEFINITIONS AND DRAW  ********
;  *********************************************

draw_window:
	mov	    eax, 12
	mov	    ebx, 1
	mcall

	mov	    eax, 0
	mov	    ebx, 100 * 65536 + (BUTTONXSIZE+BUTTONXSPACE)*BUTTONCOUNT+BUTTONXSTART*2
	mov	    ecx, 100 * 65536 + 120
	mov	    edx, 0x14E1E1E1
	mov	    esi, 0x808899ff
	mov	    edi, title
	mcall

  .no_redraw:
	mcall	    8, ((BUTTONXSIZE+BUTTONXSPACE)*BUTTONCOUNT-30) shl 16 + 60, 25 shl 16 + 12, 20, 0x00E1E1E1
	mov	    eax, [flags]
	and	    eax, 1 shl PIN_CONF_FLAG
	jz	    .no_toggles
	mcall	    4, ((BUTTONXSIZE+BUTTONXSPACE)*BUTTONCOUNT-27) shl 16 + 28, 1 shl 31, fnsh_btn
	mov	    edx, BUTTONCOUNT+9
  .tloop:
    push    edx
    call    draw_tbutton
    pop     edx
    dec     edx
    cmp     edx, 9
    jnz     .tloop
    jmp     @f
  .no_toggles:
	mcall 4, ((BUTTONXSIZE+BUTTONXSPACE)*BUTTONCOUNT-27) shl 16 + 28, 1 shl 31, conf_btn
  @@:
  .oloop_entry:
	mov	edx, BUTTONCOUNT+1
  .oloop:
    push    edx
    call    draw_obutton
    pop     edx
    dec     edx
    cmp     edx, 1
    jnz     .oloop

	mov	    eax, 12
	mov	    ebx, 2
	mcall
	ret

redraw_obutton:
	mov	    eax, 8
    push    edx
	or	    edx, 0x80 shl 24
	mcall
	pop	    edx
draw_obutton:
	mov	    ecx, edx
	dec	    ecx
	dec	    ecx
	xor	    ebx, ebx
	mov	    bl, [btn_state]
	mov	    bh, 1
	shl	    bh, cl
	and	    bl, bh
	jz	    .off
	mov	    esi, ONCOLOR
	jmp	    .aftercolor
  .off:
	mov	    esi, OFFCOLOR
  .aftercolor:
	mov	    bl, [btn_io]
	and	    bl, bh
	jne	.output
	or	    edx, 1 shl 29
  .output:
	push	ecx
	imul	bx, cx, word(BUTTONXSPACE+BUTTONXSIZE)
	add	    bx, BUTTONXSTART
	imul	ebx, 65536
	add	    ebx, BUTTONXSIZE
	mov	    ecx, BUTTONYSTART*65536+BUTTONYSIZE
	mcall	8
	pop	    ecx
	xor	    ebx, ebx
	mov	    bl, [btn_io]
	mov	    bh, 1
	shl	    bh, cl
	and	    bl, bh
	jnz	    .text
	ret
  .text:
	mov	    bl, [btn_state]
	and	    bl, bh
	jz	    .off_text
	mov	    edx, on_text
	jmp	    .aftertext
  .off_text:
	mov	    edx, off_text
  .aftertext:
	imul	bx, cx, word(BUTTONXSPACE+BUTTONXSIZE)
	add	    bx, (BUTTONXSTART + (BUTTONXSIZE/2)-5)
	shl	    ebx, 16
	add	    ebx, BUTTONYSTART + (BUTTONYSIZE/2)
	mcall	4,,1 shl 31
	ret

draw_tbutton:
	mov	    ecx, edx
	sub	    ecx, 10
	push	edx ecx
	or	    edx, 1 shl 31
	mcall	8
	xor	    edi, edi
	imul	di, cx, word(BUTTONXSPACE+BUTTONXSIZE)
	push	edi
	shl	    edi, 16
	mov	    ebx, (BUTTONXSTART)*65536+BUTTONYSTART-12
	add	    ebx, edi
	mcall	4,, 1 shl 31, i_text
	mov	    ebx, (BUTTONXSTART+5)*65536+BUTTONXSIZE-5*2
	add	    ebx, edi
	mcall	13,, (BUTTONYSTART-13)*65536+9, 0xFFFFFF
	mov	    ebx, (BUTTONXSTART+BUTTONXSIZE-4)*65536+BUTTONYSTART-12
	add	    ebx, edi
	mcall	4,, 1 shl 31, o_text
	pop	    edi ecx edx
	mov	    ebx, edi
	add	    ebx, (BUTTONXSTART+7)
	mov	    al, [btn_io]
	mov	    ah, 1
	shl	    ah, cl
	and	    al, ah
	jz	    .input
	add	    ebx, (BUTTONXSIZE-14)/2
  .input:
	shl	    ebx, 16
	add	    ebx, (BUTTONXSIZE-14)/2
	mcall	8,,(BUTTONYSTART-12)*65536+6,, 0x00576B8C
	ret

;  *********************************************
;  *************   DATA AREA   *****************
;  *********************************************

btn_state   db ?
btn_io	    db ?
dev		dd ?
pid		dd ?
flags	    dd ?
drv_name    db 'usbother', 0
noftdi_msg  db 'No FTDI connected', 0
devrdy_msg  db 'First FTDI is ready', 0
devlkd_msg  db 'First FTDI is locked', 0
welcio_msg  db 'Please, choose pin state', 0
conf_btn    db 'Configure', 0
fnsh_btn    db 'Finish', 0
off_text    db 'OFF', 0
on_text     db 'ON', 0
i_text	    db 'I', 0
o_text	    db 'O', 0
out_buf     rd 10
in_buf	    rd 10
IOCTLs	    rd 6
thread_inf  rb 1024
title	    db "FT232 Control Center", 0

I_END: