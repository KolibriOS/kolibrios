; standard driver stuff; version of driver model = 5
format PE DLL native 0.05

DEBUG   equ 1

MT_3B       equ 0
MT_3BScroll equ 3
MT_5BScroll equ 4

PS2_DRV_VER equ 1

section '.flat' code readable writable executable
data fixups
end data
include '../../../struct.inc'
include '../../../macros.inc'
include '../../../proc32.inc'
include '../../../peimport.inc'


entry START
proc START c, state:dword, cmdline:dword

          cmp [state], DRV_ENTRY
          je .init
          cmp [state], DRV_EXIT
          je .fini
          jmp .nothing
  .init:
; disable keyboard and mouse interrupts
; keyboard IRQ handler can interfere badly otherwise
          pushf
          cli
          mov  bl, 0x20        ; read command byte
          call kbd_cmd
          test ah,ah
          jnz  .fin
          call kbd_read
          test ah,ah
          jnz  .fin
          popf
          and  al, 0xFC        ; disable interrupts
          or   al, 0x10        ; disable keyboard
          push eax
          mov  bl, 0x60        ; write command byte
          call kbd_cmd
          pop  eax
          call kbd_write

          call detect_mouse
          test eax,eax
          jnz  .exit

          mov  [MouseType],MT_3B

          call try_mode_ID3
          test eax,eax
          jnz  .stop_try
          mov  [MouseType],MT_3BScroll

          call try_mode_ID4
          test eax,eax
          jnz  .stop_try
          mov  [MouseType],MT_5BScroll

  .stop_try:

          mov  al, 0xF4        ; enable data reporting
          call mouse_cmd

; enable keyboard and mouse interrupts
          mov  bl, 0x20        ; read command byte
          call kbd_cmd
          call kbd_read
          or   al, 3           ; enable interrupts
          and  al, not 0x10    ; enable keyboard
          push eax
          mov  bl, 0x60        ; write command byte
          call kbd_cmd
          pop  eax
          call kbd_write

          invoke AttachIntHandler, 12, irq_handler, 0
          invoke RegService, my_service, service_proc
                ret

  .fin:
          popf
          ;invoke DetachIntHandler, 12, irq_handler
          mov  bl, 0xA7        ; disable mouse interface
          call kbd_cmd
  .nothing:
          xor  eax, eax
          ret
.fini:
          mov   al, 0xF5
          call mouse_cmd
          ret

  .exit:
          mov  bl, 0xA7        ; disable mouse interface
          call kbd_cmd

; enable keyboard interrupt, leave mouse interrupt disabled
          mov  bl, 0x20        ; read command byte
          call kbd_cmd
          call kbd_read
          or   al, 1           ; enable keyboard interrupt
          and  al, not 0x10    ; enable keyboard
          push eax
          mov  bl, 0x60        ; write command byte
          call kbd_cmd
          pop  eax
          call kbd_write

          xor  eax, eax
          ret
endp

proc service_proc stdcall, ioctl:dword
    mov  edi, [ioctl]
    mov  eax, [edi+IOCTL.io_code]
    test eax, eax
    jz   .getversion
    cmp  eax,1
    jz   .gettype

  .err:
    or   eax, -1
    ret

  .ok:
    xor  eax, eax
    ret

  .getversion:
    cmp  [edi+IOCTL.out_size], 4
    jb   .err
    mov  edi, [edi+IOCTL.output]
    mov  dword [edi], PS2_DRV_VER               ; version of driver
    jmp  .ok
  .gettype:
    cmp  [edi+IOCTL.out_size], 4
    jb   .err
    mov  edi, [edi+IOCTL.output]
    mov  eax,[MouseType]
    mov  dword [edi], eax               ; mouse type
    jmp  .ok
endp

detect_mouse:

    mov  bl, 0xA8            ; enable mouse interface
    call kbd_cmd
    cmp  ah,1
    je   .fail

          mov  al, 0xFF      ; reset
    call mouse_cmd
    jc   .fail

    call mouse_read
    jc   .fail
    cmp  al, 0xAA
    jne  .fail         ; dead mouse

    ; get device ID
    call mouse_read
    jc   .fail
    cmp  al, 0x00
    jne  .fail        ; unknown device

    xor  eax,eax
    ret

  .fail:
    or   eax,-1
    ret

try_mode_ID3:
    mov  al, 0xF3    ;Set Sample Rate
    call mouse_cmd
    jc   .fail
    mov  al, 0xC8    ;200d
    call mouse_cmd
    jc   .fail
    mov  al, 0xF3    ;Set Sample Rate
    call mouse_cmd
    jc   .fail
    mov  al, 0x64    ;100d
    call mouse_cmd
    jc   .fail
    mov  al, 0xF3    ;Set Sample Rate
    call mouse_cmd
    jc   .fail
    mov  al, 0x50    ;80d
    call mouse_cmd
    jc   .fail

    mov  al, 0xF2    ;Get device id
    call mouse_cmd
    jc   .fail

    call mouse_read
    jc   .fail
    cmp  al, 0x03
    jne  .fail

    xor  eax,eax
    ret
  .fail:
    or   eax,-1
    ret

try_mode_ID4:
    mov  al, 0xF3    ;Set Sample Rate
    call mouse_cmd
    jc   .fail
    mov  al, 0xC8    ;200d
    call mouse_cmd
    jc   .fail
    mov  al, 0xF3    ;Set Sample Rate
    call mouse_cmd
    jc   .fail
    mov  al, 0xC8    ;100d
    call mouse_cmd
    jc   .fail
    mov  al, 0xF3    ;Set Sample Rate
    call mouse_cmd
    jc   .fail
    mov  al, 0x50    ;80d
    call mouse_cmd
    jc   .fail

    mov  al, 0xF2    ;Get device id
    call mouse_cmd
    jc   .fail

    call mouse_read
    jc   .fail
    cmp  al, 0x04
    jne  .fail

    xor  eax,eax
    ret

  .fail:
    or   eax,-1
    ret
    
include 'ps2m_iofuncs.inc'
include 'ps2m_irqh.inc'

my_service      db  'ps2mouse',0

;iofuncs data
mouse_cmd_byte   db 0
mouse_nr_tries   db 0
mouse_nr_resends db 0

;hid data
mouse_byte  dd 0

first_byte  db 0
second_byte db 0
third_byte  db 0
fourth_byte db 0

;main data
MouseType        dd 0

XMoving          dd 0
YMoving          dd 0
ZMoving          dd 0
ButtonState      dd 0
;timerTicks       dd 0
