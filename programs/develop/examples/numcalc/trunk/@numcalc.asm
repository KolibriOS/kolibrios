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
        int     40h
        mov     al, 40          ; старшие биты уже обнулены
        mov     bl, 2           ; старшие биты уже обнулены
        int     40h
event:
        push    10
        pop     eax
        int     40h
; у нас может быть только одно событие - нажата клавиша
        mov     al, 2
        int     40h
        cmp     al, 2
        jnz     event
; у нас есть только одна горячая клавиша
        push    70
        pop     eax
        mov     ebx, fileinfo
        int     40h
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
