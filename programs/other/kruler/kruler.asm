; KRuler - a screen ruler
; rgimad 2021
; author of idea - ConLenov

; header:
use32              
        org     0
        db      'MENUET01'  ; magic
        dd      1           ; header version
        dd      START       ; entry point
        dd      I_END       ; program size
        dd      MEM         ; memory size
        dd      STACKTOP    ; stack top addr
        dd      0           ; buf for args
        dd      0           ; reversed

WND_START_X     = 10
WND_START_Y     = 40
WND_WIDTH       = 360
WND_HEIGHT      = 68
 
include "../../macros.inc" 
 
START:

; event loop:
event_loop:
        mcall   10            ; wait for event
 
        cmp     eax, 1        ; redraw event
        je      on_redraw

        cmp     eax,3         ; btn
        je      on_button         
 
        jmp     event_loop
 

on_button:
        mcall   17            ; 17 - get key code
        cmp     ah, 1         ; if key with code 1 is not pressed then continue
        jne     event_loop
        mcall   -1 ; else exit
 
; define and draw window
on_redraw:
 
        mcall   12, 1       ; begin redraw
        ; mcall   48, 3, sc,sizeof.system_colors
 
        mov     edx, 0xFEF977          ; background color
        or      edx, 0x34000000        ; window type
        mcall   0, <WND_START_X, WND_WIDTH>, <WND_START_Y, WND_HEIGHT>, , , wnd_title

        mov     esi, 5
.while1:
        cmp     esi, 355
        jae     .while1_end

        mov     eax, 38
        mov     ebx, esi
        shl     ebx, 16
        add     ebx, esi
        xor     ecx, ecx
        shl     ecx, 16
        add     ecx, 30 ; stripe height
        xor     edx, edx
        int     0x40

        add     esi, 10
        jmp     .while1
.while1_end:

mov     esi, 10
.while2:
        cmp     esi, 350
        jae     .while2_end

        mov     eax, 38
        mov     ebx, esi
        shl     ebx, 16
        add     ebx, esi
        xor     ecx, ecx
        shl     ecx, 16
        add     ecx, 20 ; stripe height
        xor     edx, edx
        int     0x40

        add     esi, 10
        jmp     .while2
.while2_end:
 
        mcall   12, 2                  ; end draw
        jmp     event_loop
 
; data:

sc              system_colors
wnd_title       db 'KRuler v0.0.1',0

; labels:
 
I_END:
  rb 4096               ; for stack

align 16
STACKTOP:               ; stack top label, stack grows downwards
                        
MEM:                    ; end