;
;   THREAD EXAMPLE
;
;   Compile with FASM for Menuet
;

  use32
  org    0x0

  db     'MENUET01'             ; 8 byte id
  dd     0x01                   ; header version
  dd     START                  ; start of code
  dd     I_END                  ; size of image
  dd     0x2000                ; memory for app
  dd     0x2000                ; esp
  dd     0x0 , 0x0              ; I_Param , I_Icon

include 'lang.inc'
include '..\..\..\..\macros.inc'


START:                          ; start of execution

red:                            ; redraw
    call draw_window            ; at first, draw the window

still:

    mov  eax,10                 ; wait here for event
    mcall

    dec  eax                    ; redraw request ?
    je   red
    dec  eax                    ; key in buffer ?
    jne  button

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    mcall
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    mcall

    cmp  ah,1                   ; button id=1 ?
    jne  noclose
    or   eax,-1                 ; close this program (thread)
    mcall
  noclose:

    cmp  ah,2
    jne  no_thread

    cmp  [thread_stack],0x1f00
    jge  no_thread

    add  [thread_stack],0x100
    mov  eax,51
    mov  ebx,1
    mov  ecx,START
    mov  edx,[thread_stack]
    mcall

    jmp  still

  no_thread:

    jmp  still


thread_stack dd 0x1000


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,10*65536+290         ; [x start] *65536 + [x size]
    mov  ecx,10*65536+130         ; [y start] *65536 + [y size]
    mov  esi,[thread_stack]
    sub  esi,0x1000
    shr  esi,4
    shl  esi,16
    add  ebx,esi
    add  ecx,esi
    mov  edx,0x33ffffff            ; color of work area RRGGBB,8->color gl
    mov  edi,title               ; WINDOW LABEL
    mcall

                                   
    mov  eax,8                     ; NEW THREAD BUTTON
    mov  ebx,20*65536+128
    mov  ecx,63*65536+20
    mov  edx,2
    mov  esi,0x90b0d0 ;0x5577cc
    mcall

    mov  eax,4
    mov  ebx,20*65536+10           ; draw info text with function 4
    mov  ecx,0x224466
    mov  edx,text
    mov  esi,40
  newline:
    mcall
    add  ebx,10
    add  edx,40
    cmp  [edx],byte 'x'
    jne  newline


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret


; DATA AREA

if lang eq ru
  text:
      db 'Эта программа создает потоки, запуская  '
      db 'один и тот же код много раз. Нам нужно  '
      db 'только позаботиться об отдельном стэке  '
      db 'для каждого потока.                     '
      db 'Память для всех потоков общая.          '
      db '                                        '
      db ' СОЗДАТЬ НОВЫЙ ПОТОК                    '
      db 'x' ; <- END MARKER, DONT DELETE

  title   db   'Пример использования потоков',0

else
  text:
      db 'THIS EXAMPLE CREATES THREADS BY RUNNING '
      db 'THE SAME CODE MULTIPLE TIMES. ALL WE    '
      db 'NEED IS A NEW STACK FOR EACH THREAD.    '
      db 'ALL THREADS SHARE THE SAME MEMORY.      '
      db '                                        '
      db '                                        '
      db '  CREATE NEW THREAD                     '
      db 'x' ; <- END MARKER, DONT DELETE

  title   db   'THREAD EXAMPLE',0

end if
I_END:
