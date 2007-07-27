;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


MouseSearch_PS2:
	jmp	MouseSearch_PS2_begin

mouse_error equ MouseSearch_PS2_begin.error

 kb_cmd_c:
	call	kb_cmd
	jmp	check_kbd

 kb_write_c:
	call	kb_write
	jmp	check_kbd

 kb_read_c:
	call	kb_read
	;jmp	check_kbd

 check_kbd:
	cmp	ah, 1
	je	mouse_error
	ret

uglobal
  mouse_cmd_byte   db 0
  mouse_nr_tries   db 0
  mouse_nr_resends db 0

  mouse_error_esp  dd 0
endg


 mouse_cmd:
	mov	[mouse_cmd_byte], al
	mov	[mouse_nr_resends], 5
   .resend:
	mov	bl, 0xd4
	call	kb_cmd_c
	mov	al, [mouse_cmd_byte]
	call	kb_write_c

	call	mouse_read

	cmp	al, 0xFA	; ack
	jne	.noack
	ret
   .noack:
	cmp	al, 0xFE	; resend
	jne	.noresend
	dec	[mouse_nr_resends]
	jnz	.resend
   .noresend:
	jmp	mouse_error


 mouse_read:
	mov	[mouse_nr_tries], 100
   .repeat:
	call	kb_read
	cmp	ah, 1
	jne	.fin
	mov	esi, 10
	call	delay_ms
	dec	[mouse_nr_tries]
	jnz	.repeat
	jmp	mouse_error
   .fin:
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MouseSearch_PS2_begin:
	pushad

	mov	[mouse_error_esp], esp

        mov     bl, 0xAD	; disable keyboard interface
        call    kb_cmd_c

        mov     bl, 0xA8	; enable mouse interface
        call    kb_cmd_c

	mov	al, 0xFF	; reset
	call	mouse_cmd

	; now the mouse is in Reset Mode
	; get the Basic Assurance Test completion code
	call	mouse_read
	cmp	al, 0xAA
	jne	.error		; dead mouse

	; get device ID
	call	mouse_read
	cmp	al, 0x00
	jne	.error		; unknown device

	; reset completed successfully

	; enable mouse interrupt - IRQ12
	mov	bl, 0x20	; read command byte
	call	kb_cmd_c
	call	kb_read_c
	or	al, 10b
	push	eax
	mov	bl, 0x60	; write command byte
	call	kb_cmd_c
	pop	eax
	call	kb_write_c

	mov	al, 0xF4	; enable data reporting
	call	mouse_cmd

	mov	[ps2_mouse_detected], 1
	mov     bl, 0xAE	; enable keyboard interface
	call	kb_cmd

	mov	esi, boot_setmouse_type
	call	boot_log

	jmp	.finish


.error:
	mov	esp, [mouse_error_esp]    ; clear stack frame
	mov	[ps2_mouse_detected], 0
	mov	bl, 0xA7 ; disable mouse interface
	call	kb_cmd
  	mov	bl, 0xAE ; enable keyboard interface
  	call	kb_cmd

.finish:
	popad
