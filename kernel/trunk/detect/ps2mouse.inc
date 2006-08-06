; for information on PS2 mouse/keyboard programming
; refer to http://www.computer-engineering.org/ps2mouse/
; and http://www.computer-engineering.org/ps2keyboard/
; respectively

MouseSearch_PS2:
        jmp .begin

.kb_cmd_c:
        call    kb_cmd
.check:
        cmp     ah, 1
        je      @f
        ret
@@:
        add     esp, 4  ; return address
        jmp     .DataInputError

.kb_write_c:
        call    kb_write
        jmp     .check

.kb_read_c:
        call    kb_read
        jmp     .check

;--------------------------------------------
.begin:
        pushad

        mov     bl, 0xAD        ; disable keyboard interface
        call    .kb_cmd_c

        mov     bl, 0xa8        ; enable mouse interface
        call    .kb_cmd_c

        mov     bl, 0xd4        ; to mouse
        call    .kb_cmd_c
        mov     al, 0xFF        ; reset
        call    .kb_write_c
        call    .kb_read_c
        cmp     al, 0xFA        ; ack
        jne     .no_ack

        ; now the mouse is in Reset Mode
        ; get the Basic Assurance Test completion code
        call    .kb_read_c
        cmp     al, 0xAA
        jne     .dead_mouse
        ; get device ID
        call    .kb_read_c
        cmp     al, 0x00
        jne     .unknown_device

        ; reset completed successfully

        ; enable mouse interrupt - IRQ12
        mov     bl, 0x20        ; read command byte
        call    .kb_cmd_c
        call    .kb_read_c
        or      al, 10b         ; set mouse IRQ bit
        push    eax
        mov     bl, 0x60        ; write command byte
        call    .kb_cmd_c
        pop     eax
        call    .kb_write_c

        mov     bl, 0xd4        ; to mouse
        call    .kb_cmd_c
        mov     al, 0xf4        ; enable data reporting
        call    .kb_write_c
        call    .kb_read_c
        cmp     al, 0xFA
        jne     .no_ack

;       jmp     @f
;.set_sample_rate:
;       push    eax
;       mov     bl, 0xd4        ; to mouse
;       call    .kb_cmd_c
;       mov     al, 0xF3        ; set sample rate
;       call    .kb_write_c
;       call    .kb_read_c      ; ack
;       pop     eax
;       call    .kb_write_c
;       call    .kb_read_c      ; ack
; @@:
;       mov     eax, 200
;       call    .set_sample_rate
;       mov     eax, 100
;       call    .set_sample_rate
;       mov     eax, 80
;       call    .set_sample_rate
;       mov     bl, 0xd4
;       call    .kb_cmd_c
;       mov     al, 0xF2        ; read device type
;       call    .kb_write_c
;       call    .kb_read_c      ; ack
;       call    .kb_read_c      ; mouse ID
;       cmp     al, 0x03
;       jne     .no_scroll
;       mov     [ps2_mouse_scroll], 1
;       ; it'll send 4-byte packets instead of 3-byte ones
;       ; the last byte of a packet represents z-axis movement
; .no_scroll:

        mov     [ps2_mouse_detected], 1
        mov     bl, 0xAE        ; enable keyboard interface
        call    .kb_cmd_c

        mov     esi, boot_setmouse_type
        call    boot_log

        jmp     .finish


.DataInputError:
.no_ack:
.dead_mouse:
.unknown_device:
        mov     [ps2_mouse_detected],0
        mov     bl, 0xA7 ; disable mouse interface
        call    kb_cmd
        mov     bl, 0xAE ; enable keyboard interface
        call    kb_cmd

.finish:
        popad
