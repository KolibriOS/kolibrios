include '..\..\..\..\macros.inc'
use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      mem
        dd      mem
        dd      0
        dd      0

start:
        push    66
        pop     eax
        push    4
        pop     ebx
        mov     cl, 45h         ; NumLock scancode
        xor     edx, edx
        mcall
        mov     al, 40          ; старшие биты уже обнулены
        mov     bl, 2           ; старшие биты уже обнулены
        mcall
event:
        push    10
        pop     eax
        mcall
; у нас может быть только одно событие - нажата клавиша
        mov     al, 2
        mcall
        cmp     al, 2
        jnz     event
; у нас есть только одна горячая клавиша
        push    70
        pop     eax
        mov     ebx, fileinfo
        mcall
        jmp     event

fileinfo:
        dd      7
        dd      0
        dd      0
        dd      0
        dd      0
        db      '/rd/1/calc',0

i_end:
        align   16
        rb      16
mem:
