;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                  ;
;    86DUINO GPIO DEMO APPLICATION                 ;
;                                                  ;
;      Compile with FASM                           ;
;                                                  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 
format binary as ""                     ; Binary file format without extension

use32                                   ; Tell compiler to use 32 bit instructions
 
org 0x0                                 ; the base address of code, always 0x0

; The header

db 'MENUET01'
dd 0x01
dd START
dd I_END
dd 0x100000
dd 0x7fff0
dd 0, 0
 
; The code area
 
include '../../macros.inc'
 
START:                                  ; start of execution

        mcall   68, 16, drv_name        ; load the driver
        mov     [IOCTL.handle], eax

        call    draw_window             ; draw the window
 
; After the window is drawn, it's practical to have the main loop.
; Events are distributed from here.
 
event_wait:
        mov     eax, 10                 ; function 10 : wait until event
        mcall                           ; event type is returned in eax
 
        cmp     eax, 1                  ; Event redraw request ?
        je      red                     ; Expl.: there has been activity on screen and
                                        ; parts of the applications has to be redrawn.
 
        cmp     eax, 2                  ; Event key in buffer ?
        je      key                     ; Expl.: User has pressed a key while the
                                        ; app is at the top of the window stack.
 
        cmp     eax, 3                  ; Event button in buffer ?
        je      button                  ; Expl.: User has pressed one of the
                                        ; applications buttons.
 
        jmp     event_wait
 
;  The next section reads the event and processes data.
 
red:                                    ; Redraw event handler
        call    draw_window             ; We call the window_draw function and
        jmp     event_wait              ; jump back to event_wait
 
key:                                    ; Keypress event handler
        mov     eax, 2                  ; The key is returned in ah. The key must be
        mcall                           ; read and cleared from the system queue.

        cmp     ah, 'q'
        jne     @f
        call    read_gpio0
        or      al, 1 shl 0             ; Set bit 0
        call    write_gpio0
        jmp     event_wait
  @@:
        cmp     ah, 'w'
        jne     @f
        call    read_gpio0
        and     al, not (1 shl 0)       ; Clear bit 0
        call    write_gpio0
        jmp     event_wait
  @@:
        cmp     ah, 'e'
        jne     @f
        call    read_adc0
        mov     ecx, eax
        mcall   47, 0x00040100,,25 shl 16 + 25, 0x40000000, 0x00ffffff          ; 4 digits hex number in ecx
        jmp     event_wait
  @@:
        cmp     ah, 'a'
        jne     @f
        call    read_gpio0
        or      al, 1 shl 2             ; Set bit 2
        call    write_gpio0
        jmp     event_wait
  @@:
        cmp     ah, 's'
        jne     @f
        call    read_gpio0
        and     al, not (1 shl 2)       ; Clear bit 2
        call    write_gpio0
        jmp     event_wait
  @@:
        jmp     event_wait              ; Just read the key, ignore it and jump to event_wait.
 
button:                                 ; Buttonpress event handler
        mov     eax, 17                 ; The button number defined in window_draw
        mcall                           ; is returned to ah.
 
        cmp     ah, 1                   ; button id=1 ?
        jne     noclose
        mov     eax, -1                 ; Function -1 : close this program
        mcall
 
noclose:
        jmp     event_wait              ; This is for ignored events, useful at development
 
;  *********************************************
;  ******  WINDOW DEFINITIONS AND DRAW  ********
;  *********************************************
;
;  The static window parts are drawn in this function. The window canvas can
;  be accessed later from any parts of this code (thread) for displaying
;  processes or recorded data, for example.
;
;  The static parts *must* be placed within the fn 12 , ebx = 1 and ebx = 2.
 
draw_window:
        mov     eax, 12                 ; function 12: tell os about windowdraw
        mov     ebx, 1                  ; 1, start of draw
        mcall
 
        mov     eax, 0                  ; function 0 : define and draw window
        mov     ebx, 100 * 65536 + 300  ; [x start] *65536 + [x size]
        mov     ecx, 100 * 65536 + 120  ; [y start] *65536 + [y size]
        mov     edx, 0x14ffffff         ; color of work area RRGGBB
                                        ; 0x02000000 = window type 4 (fixed size, skinned window)
        mov     esi, 0x808899ff         ; color of grab bar  RRGGBB
                                        ; 0x80000000 = color glide
        mov     edi, title
        mcall
 
        mov     ebx, 25 * 65536 + 35    ; draw info text with function 4
        mov     ecx, 0x224466
        mov     edx, text
        mov     esi, 40
        mov     eax, 4
 
  .newline:                             ; text from the DATA AREA
        mcall
        add     ebx, 10
        add     edx, 40
        cmp     byte[edx], 0
        jne     .newline
 
        mov     eax, 12                 ; function 12:tell os about windowdraw
        mov     ebx, 2                  ; 2, end of draw
        mcall
 
        ret

; Read GPIO0 port to AL register
read_gpio0:
        mov     [IOCTL.io_code], 1
        mcall   68, 17, IOCTL
        ret

; Write AL register to GPIO0 port
write_gpio0:
        mov     [IOCTL.io_code], 2
        mov     [IOCTL.input], eax
        mcall   68, 17, IOCTL
        ret

; Read ADC0
read_adc0:
        mov     [IOCTL.io_code], 3
        mcall   68, 17, IOCTL
        ret
 
;  *********************************************
;  *************   DATA AREA   *****************
;  *********************************************
;
; Data can be freely mixed with code to any parts of the image.
; Only the header information is required at the beginning of the image.
 
text    db  "This is an 86DUINO GPIO demo program    "
        db  "                                        "
        db  "press q/w to toggle GPIO 0 pin 0        "
        db  "or e to read ADC0 channel               ", 0
 
title   db  "86Duino Example application", 0

drv_name        db '86DUINO-GPIO', 0

IOCTL:
   .handle      dd ?
   .io_code     dd ?
   .input       dd ?
   .inp_size    dd ?
   .output      dd ?
   .out_size    dd ?
 
I_END: