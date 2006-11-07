;
;    MAGNIFY SCREEN
;
;    Compile with FASM for Menuet
;

use32

                org     0x0

                db      'MENUET01'              ; 8 byte id
                dd      1                       ; header version
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x1000                  ; required amount of memory
                dd      0x1000                  ; esp
                dd      0, 0                    ; no parameters, no path

include 'macros.inc'
delay   equ     20

magnify_width = 40
magnify_height = 30

START:                          ; start of execution
redraw:
        call    draw_window
still:
        call    draw_screen
wtevent:
        mov     eax, 23         ; wait here for event with timeout
        mov     ebx, delay
        int     0x40
        dec     eax
        js      still
        jz      redraw
        dec     eax
        jnz     button
; key in buffer
        mov     al, 2
        int     0x40
        jmp     wtevent
button:
; we have only one button, close
        or      eax, -1
        int     0x40

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:
        mov     eax, 12         ; function 12:tell os about windowdraw
        mov     ebx, 1          ; 1, start of draw
        int     0x40

        mov     al, 48          ; function 48 : graphics parameters
        mov     bl, 4           ; subfunction 4 : get skin height
        int     0x40

                                        ; DRAW WINDOW
        mov     ebx, 100*65536 + 8*magnify_width + 8
        lea     ecx, [eax + 100*65536 + 8*magnify_height + 3]
        mov     edx, 0x33000000         ; color of work area RRGGBB
        mov     edi, labelt             ; header
        xor     eax, eax                ; function 0 : define and draw window
        int     0x40


        mov     al, 12                  ; function 12:tell os about windowdraw
        mov     ebx, 2                  ; 2, end of draw
        int     0x40

        ret

draw_screen:
draw_magnify:
        mov     eax, 14
        int     0x40            ; get screen size
        movzx   ecx, ax
        inc     ecx
        mov     [size_y], ecx
        shr     eax, 16
        inc     eax
        mov     [size_x], eax

        mov     eax, 37
        xor     ebx, ebx
        int     0x40            ; get mouse coordinates
        mov     ecx, eax
        shr     ecx, 16         ; ecx = x
        movzx   edx, ax         ; edx = y
        inc     ecx
        mov     [m_xe], ecx
        inc     edx
        mov     [m_ye], edx
        sub     ecx, magnify_width
        sub     edx, magnify_height
        mov     [m_x], ecx
        mov     [m_y], edx
.loop_y:
.loop_x:
        xor     eax, eax        ; assume black color for invalid pixels
        test    ecx, ecx
        js      .nopix
        cmp     ecx, [size_x]
        jge     .nopix
        test    edx, edx
        js      .nopix
        cmp     edx, [size_y]
        jge     .nopix
        mov     ebx, edx
        imul    ebx, [size_x]
        add     ebx, ecx
        mov     eax, 35
        int     0x40            ; read pixel
.nopix:
        push    ecx edx
        sub     ecx, [m_x]
        sub     edx, [m_y]
        mov     ebx, ecx
        shl     ebx, 3+16
        mov     bl, 7
        mov     ecx, edx
        shl     ecx, 3+16
        mov     cl, 7
        mov     edx, eax
        mov     eax, 13
        int     0x40
        pop     edx ecx
        inc     ecx
        cmp     ecx, [m_xe]
        jnz     .loop_x
        mov     ecx, [m_x]
        inc     edx
        cmp     edx, [m_ye]
        jnz     .loop_y
        ret

; DATA AREA

labelt:
    db   'MAGNIFIER - MOVE MOUSE POINTER', 0

I_END:
align 4
m_x     dd      ?
m_y     dd      ?
m_xe    dd      ?
m_ye    dd      ?
size_x  dd      ?
size_y  dd      ?
