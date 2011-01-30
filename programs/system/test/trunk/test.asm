;
;    PROTECTION TEST
;

use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x10000                 ; memory for app
               dd     0xfff0                  ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

include '..\..\macros.inc'

START:                          ; start of execution

    call draw_window            ; at first, draw the window

still:

    mov  eax,10                 ; wait here for event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jz   key
    cmp  eax,3                  ; button in buffer ?
    jz   button

    jmp  still

  red:                          ; redraw
    call draw_window

    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    int  0x40

    jmp  still

  button:                       ; button
    mov  eax,17
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jnz  noclose
    mov  eax,0xffffffff         ; close this program
    int  0x40
  noclose:

    cmp  ah,2
    jnz  notest2
    cli
  notest2:

    cmp  ah,3
    jnz  notest3
    sti
  notest3:

    cmp  ah,4
    jnz  notest4
     mov  [0x10000],byte 1
   notest4:

    cmp  ah,5
    jnz  notest5
    jmp  dword 0x10000
  notest5:

    cmp  ah,6
    jnz  notest6
    mov  esp,0
    push eax
  notest6:

    cmp  ah,7
    jnz  notest7
    in   al,0x60
  notest7:

    cmp  ah,8
    jnz  notest8
    out  0x60,al
  notest8:




    jmp  still


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+300         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+240         ; [y start] *65536 + [y size]
    mov  edx,0x02ffffff            ; color of work area RRGGBB
    mov  esi,0x80597799            ; color of grab bar  RRGGBB,8->color glide
    mov  edi,0x00597799            ; color of frames    RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff            ; color of text RRGGBB
    mov  edx,tlabel                 ; pointer to text beginning
    mov  esi,labellen-tlabel        ; text length
    int  0x40

                                   ; CLOSE BUTTON
    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,(300-19)*65536+12     ; [x start] *65536 + [x size]
    mov  ecx,5*65536+12            ; [y start] *65536 + [y size]
    mov  edx,1                     ; button id
    mov  esi,0x5977bb              ; button color RRGGBB
    int  0x40


    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,25*65536+9            ; [x start] *65536 + [x size]
    mov  ecx,74*65536+9            ; [y start] *65536 + [y size]
    mov  edx,2                     ; button id
    mov  esi,0x5977bb              ; button color RRGGBB
  newb:
    int  0x40
    add  ecx,20*65536
    inc  edx
    cmp  edx,9
    jb   newb

    cld
    mov  ebx,25*65536+36           ; draw info text with function 4
    mov  ecx,0x000000
    mov  edx,text
    mov  esi,40
  newline:
    mov  eax,4
    int  0x40
    add  ebx,10
    add  edx,40
    cmp  [edx],byte 'x'
    jnz  newline


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


; DATA AREA


text:

    db 'APPLICATION USES 0x10000 BYTES OF MEMORY'
    db '                                        '
    db 'OPEN DEBUG BOARD FOR PARAMETERS         '
    db '                                        '
    db '     CLI                                '
    db '                                        '
    db '     STI                                '
    db '                                        '
    db '     MOV [0x10000],BYTE 1               '
    db '                                        '
    db '     JMP DWORD 0x10000                  '
    db '                                        '
    db '     MOV ESP,0 & PUSH EAX               '
    db '                                        '
    db '     IN  Al,0x60                        '
    db '                                        '
    db '     OUT 0x60,AL                        '
    db 'x                                       '



tlabel:
    db   'MENUET PROTECTION TEST'
labellen:

I_END:
