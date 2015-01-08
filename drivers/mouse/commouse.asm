;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;; Includes source code by Kulakov Vladimir Gennadievich.       ;;
;; Modified by Mario79 and Rus.                                 ;;
;; 02.12.2009 <Lrz>                                             ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 2

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'

; Serial data packet format:
;            D6    D5    D4    D3    D2    D1    D0
; 1st byte    1    LB    RB    Y7    Y6    X7    X6
; 2nd byte    0    X5    X4    X3    X2    X1    X0
; 3rd byte    0    Y5    Y4    Y3    Y2    Y1    Y0

; optional: (logitech extension protocol)
; 4th byte    0    MB     0     0     0     0     0

struct com_mouse_data

        port            dw ?
        offset          db ?
        data            rb 3

ends

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc START c, reason:dword, cmdline:dword

        cmp     [reason], DRV_ENTRY
        jne     .fail

        DEBUGF  2,"Loading serial mouse driver\n"

        stdcall init_mouse, 0x3f8, 4
        stdcall init_mouse, 0x2f8, 3
        stdcall init_mouse, 0x3e8, 4
        stdcall init_mouse, 0x2e8, 3

        invoke  RegService, my_service, service_proc
        ret

  .fail:
        xor     eax, eax
        ret

endp


proc service_proc stdcall, ioctl:dword

        mov     ebx, [ioctl]
        mov     eax, [ebx + IOCTL.io_code]
        cmp     eax, 0 ;SRV_GETVERSION
        jne     .fail

        mov     eax, [ebx + IOCTL.output]
        cmp     [ebx + IOCTL.out_size], 4
        jne     .fail
        mov     [eax], dword API_VERSION
        xor     eax, eax
        ret

  .fail:
        or      eax, -1
        ret
endp


proc init_mouse stdcall port, irq

        DEBUGF  1, "Trying to init serial mouse on port 0x%x\n", [port]

        xor     ebx, ebx        ; reserve port area
        mov     ecx, [port]
        lea     edx, [ecx + 7]
        push    ebp
        invoke  ReservePortArea
        pop     ebp
        test    eax, eax
        jnz     .fail

        DEBUGF  1, "Reserved port area\n"

        mov     bx, word[port]

        ; Set the speed to 1200 baud
        mov     dx, bx
        add     dx, 3
        in      al, dx
        or      al, 80h         ; set DLAB bit
        out     dx, al

        mov     dx, bx
        mov     al, 60h         ; 1200 baud
        out     dx, al
        inc     dx
        mov     al, 0
        out     dx, al

        ; Use 7 bit words, 1 stop bit, no parity control, reset DLAB bit
        mov     dx, bx
        add     dx, 3
        mov     al, 00000010b
        out     dx, al

        ; Disable interrupts
        mov     dx, bx
        inc     dx
        mov     al, 0
        out     dx, al

; Check if a MS type serial mouse is connected

; Disable power and mouse interrupts
        mov     dx, bx
        add     dx, 4           ; modem control register
        mov     al, 0           ; reset DTR, RTS, and AUX2
        out     dx, al

; Wait 5 ticks (0.2s)
        mov     esi, 200
        invoke  Sleep

; Power on the mouse
        mov     al, 1
        out     dx, al

; Wait 5 ticks (0.2s)
        mov     esi, 200
        invoke  Sleep

; Clear data register
        mov     dx, bx
        in      al, dx

; Power on the mouse
        add     dx, 4
        mov     al, 1011b       ; set DTR, DTS and AUX2
        out     dx, al

        mov     ecx, 0x1FFFF
; Poll port
  .loop:
        dec     ecx
        jz      .fail

; Check if identification byte is available
        mov     dx, bx
        add     dx, 5
        in      al, dx
        test    al, 1           ; data ready?
        jz      .loop

; Read data byte
        mov     dx, bx
        in      al, dx
        cmp     al, 'M'
        jne     .free

        DEBUGF  2, "Serial mouse detected on port 0x%x\n", [port]

; Create data struct

        invoke  Kmalloc, sizeof.com_mouse_data
        test    eax, eax
        jz      .fail

        DEBUGF  1, "Structure 0x%x allocated\n", eax

        mov     bx, word[port]
        mov     [eax + com_mouse_data.port], bx
        mov     [eax + com_mouse_data.offset], 0

; Attach int handler

        invoke  AttachIntHandler, [irq], irq_handler, eax
        test    eax, eax
        jz      .fail

        DEBUGF  1, "Attached int handler\n"

; Enable interrupts
        mov     dx, word[port]
        inc     dx
        mov     al, 1
        out     dx, al

        xor     eax, eax
        ret

  .free:
        DEBUGF  1, "Freeing port area\n"
        xor     ebx, ebx
        inc     ebx             ; free port area
        mov     ecx, [port]
        lea     edx, [ecx + 7]
        push    ebp
        invoke  ReservePortArea
        pop     ebp

  .fail:
        DEBUGF  1, "Failed\n"
        or      eax, -1
        ret

endp



irq_handler:

        push    esi
        mov     esi, [esp+2*4]

  .read_loop:
        mov     dx, [esi + com_mouse_data.port]
        add     dx, 5
        in      al, dx
        test    al, 1           ; data ready?
        jz      .end
; read data
        sub     dx, 5
        in      al, dx
        and     al, 01111111b   ; clear MSB (use 7 bit words)
        test    al, 01000000b   ; First byte indicator set?
        jnz     .FirstByte

; Check which data byte we are reading
        cmp     [esi + com_mouse_data.offset], 1
        jb      .SecondByte
        je      .ThirdByte
        ja      .FourthByte

; read first data byte
  .FirstByte:
        mov     [esi + com_mouse_data.data+0], al
        mov     [esi + com_mouse_data.offset], 0
        jmp     .read_loop

; read second data byte
  .SecondByte:
        mov     [esi + com_mouse_data.data+1], al
        inc     [esi + com_mouse_data.offset]
        jmp     .read_loop

; read third data byte
  .ThirdByte:
        mov     [esi + com_mouse_data.data+2], al
        inc     [esi + com_mouse_data.offset]

; Data packet is complete, parse it and set mouse data

        ; Left and Right Buttons
        mov     al, [esi + com_mouse_data.data+0]
        mov     ah, al
        shr     al, 3           ; right mouse button
        and     al, 10b         ;
        shr     ah, 5           ; left mouse button
        and     ah, 1b          ;
        or      al, ah
        and     [BTN_DOWN], not 11b
        or      byte[BTN_DOWN], al

        ; X coordinate
        mov     al, [esi + com_mouse_data.data+0]
        shl     al, 6
        or      al, [esi + com_mouse_data.data+1]
        movsx   eax, al
        mov     [MOUSE_X], eax

        ; Y coordinate
        mov     al, [esi + com_mouse_data.data+0]
        and     al, 00001100b
        shl     al, 4
        or      al, [esi + com_mouse_data.data+2]
        movsx   eax, al
        neg     eax
        mov     [MOUSE_Y], eax

        invoke  SetMouseData, [BTN_DOWN], [MOUSE_X], [MOUSE_Y], 0, 0

        pop     esi
        mov     al, 1
        ret

  .FourthByte:
        cmp     [esi + com_mouse_data.offset], 2
        jne     .end
        inc     [esi + com_mouse_data.offset]

        and     [BTN_DOWN], not 100b
        test    al, 00100000b
        jz      @f
        or      byte[BTN_DOWN], 100b
  @@:
        invoke  SetMouseData, [BTN_DOWN], 0, 0, 0, 0

  .end:
        pop     esi
        mov     al, 1
        ret




; End of code

data fixups
end data

include '../peimport.inc'

my_service   db 'commouse',0    ; max 16 chars include zero

include_debug_strings

MOUSE_X      dd ?
MOUSE_Y      dd ?
BTN_DOWN     dd ?