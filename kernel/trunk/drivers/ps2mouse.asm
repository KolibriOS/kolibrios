format MS COFF

include 'proc32.inc'
include 'imports.inc'

struc IOCTL
{  .handle      dd ?
   .io_code     dd ?
   .input       dd ?
   .inp_size    dd ?
   .output      dd ?
   .out_size    dd ?
}

virtual at 0
  IOCTL IOCTL
end virtual

public START
public version

DRV_ENTRY  equ 1
DRV_EXIT   equ -1

MT_3B       equ 0
MT_3BScroll equ 1
MT_5BScroll equ 2

PS2_DRV_VER equ 1

section '.flat' code readable align 16


proc START stdcall, state:dword

          cmp [state], DRV_ENTRY
          jne .fin
  .init:

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

          mov  bl, 0x20	       ; read command byte
          call kbd_cmd
          cmp  ah,1
          je   .exit

          call kbd_read
          cmp  ah,1
          je   .exit

          or   al, 10b
          push eax
          mov  bl, 0x60	       ; write command byte
          call kbd_cmd
          cmp  ah,1
          je   .exit

          pop  eax
          call kbd_write
          cmp  ah,1
          je   .exit

          mov  al, 0xF4	       ; enable data reporting
          call mouse_cmd

          mov  bl, 0xAE        ; enable keyboard interface
          call kbd_cmd
          
          stdcall AttachIntHandler, 12, irq_handler
          stdcall RegService, my_service, service_proc
	        ret

  .fin:
          ;stdcall DetachIntHandler, 12, irq_handler
          mov  bl, 0xA7        ; disable mouse interface
          call kbd_cmd
          xor  eax, eax
          ret

  .exit:
          mov  bl, 0xA7        ; disable mouse interface
          call kbd_cmd
          mov  bl, 0xAE        ; enable keyboard interface
          call kbd_cmd
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
    mov  dword [edi], PS2_DRV_VER		; version of driver
    jmp  .ok
  .gettype:
    cmp  [edi+IOCTL.out_size], 4
    jb   .err
    mov  edi, [edi+IOCTL.output]
    mov  eax,[MouseType]
    mov  dword [edi], eax		; mouse type
    jmp  .ok
endp

detect_mouse:

    mov  bl, 0xAD	     ; disable keyboard interface
    call kbd_cmd
    cmp  ah,1
    je   .fail

    mov  bl, 0xA8	     ; enable mouse interface
    call kbd_cmd
    cmp  ah,1
    je   .fail

	  mov  al, 0xFF	     ; reset
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

section '.data' data readable writable align 16

version		  dd  0x00050005
my_service	db  'ps2mouse',0

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
