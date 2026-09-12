; Asks the I2C touchpad driver to read its settings again.
;
; The driver takes everything it can be tuned with from the [touchpad] section
; of /sys/settings/system.ini, and it reads that file once while loading. This
; tells it to read the file again, so editing a setting takes effect straight
; away instead of at the next boot.
;
; There is no window: the outcome goes to the debug board, the way the driver
; reports everything else.

format binary as ""
use32
        db      'MENUET01'
        dd      1, start, i_end, mem, mem, 0, 0

include '../../programs/macros.inc'

SRV_RELOAD_CONF = 1

start:
; 1. Find the driver's service by name.
        mcall   68, 16, srv_name
        test    eax, eax
        jz      .no_driver
        mov     [ioctl.handle], eax
; 2. Ask it to reload. No data travels either way: the driver reads the file
;    itself, so the settings file stays the only place they are written down.
        mcall   68, 17, ioctl
        test    eax, eax
        jnz     .refused
        mov     esi, msg_ok
        jmp     .say
.no_driver:
        mov     esi, msg_no_driver
        jmp     .say
.refused:
        mov     esi, msg_refused
.say:
        call    board_str
        or      eax, -1
        mcall

; Writes a zero-terminated string to the debug board.
; in: esi -> string
board_str:
        lodsb
        test    al, al
        jz      .done
        mov     cl, al
        mcall   63, 1
        jmp     board_str
.done:
        ret

srv_name        db 'I2CHID', 0

align 4
ioctl:
.handle         dd 0
.io_code        dd SRV_RELOAD_CONF
.input          dd 0
.inp_size       dd 0
.output         dd 0
.out_size       dd 0

msg_ok          db 'tpreload: touchpad settings reloaded', 13, 10, 0
msg_no_driver   db 'tpreload: the I2C touchpad driver is not loaded', 13, 10, 0
msg_refused     db 'tpreload: the driver refused the request', 13, 10, 0

i_end:
        rb      1024
mem:
