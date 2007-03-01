; <--- description --->
; compiler:     FASM 1.67
; name:         Basic window example for KolibriOS
; version:      1.02
; last update:  1/03/2007
; written by:   Ivan Poddubny
; e-mail:       ivan-yar@bk.ru
;modified by: Heavyiron

; <--- include all MeOS stuff --->
include "lang.inc"
include "macros.inc"


; <--- start of MenuetOS application --->
MEOS_APP_START


; <--- start of code --->
CODE

   mov  eax,48                      ; get system colors
   mov  ebx,3
   mov  ecx,sc
   mov  edx,sizeof.system_colors
   int  0x40

  redraw:                                ; redraw event handler
   call    draw_window            ; at first create and draw the window

  wait_event:                      ; main cycle
    mov     eax, 10
    int     0x40

    dec   eax                    ;   if event = 1
    jz      redraw               ;   jump to redraw handler
    dec   eax                    ;   else if event = 2
    jz      key                    ;   jump to key handler


  button:                         ; button event handler
    mov     al, 17               ;   get button identifier
    int     0x40

    cmp     ah, 1
    jne     wait_event        ;   return if button id != 1

    or      eax, -1               ;   exit application
    int     0x40

  key:                              ; key event handler
    mov     al, 2                 ;   get key code
    int     0x40

    jmp     wait_event

  draw_window:
    mov     eax, 12                ; start drawing
    mov     ebx, 1
    int     0x40

    xor       eax, eax                      ; create and draw the window
    mov     ebx, 100*65536+200 ; (window_cx)*65536+(window_sx)
    mov     ecx, 100*65536+100  ; (window_cy)*65536+(window_sy)
    mov     edx, [sc.work]              ; work area color 
    or         edx, 0x33000000        ; & window type 3
    mov     edi, header                    ; window header
    int        0x40

    mov     eax, 12                ; finish drawing
    mov     ebx, 2
    int     0x40

  ret

; <--- initialised data --->
DATA

if lang eq ru
header db 'Шаблон программы',0
else if lang eq fr
header db 'La programme poncive',0
else
header db 'Template program',0
end if

; <--- uninitialised data --->
UDATA
sc   system_colors

MEOS_APP_END
; <--- end of MenuetOS application --->