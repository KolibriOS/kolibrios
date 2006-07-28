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
  dd     0x2000               ; memory for app
  dd     0x1000                ; esp
  dd     0x0 , 0x0              ; I_Param , I_Icon

include 'lang.inc'
include 'macros.inc'


START:                          ; start of execution

    call draw_window            ; at first, draw the window

still:

    mov  eax,10                 ; wait here for event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

    jmp  still

  red:                          ; redraw
    call draw_window
    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    int  0x40
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jne  noclose
    mov  eax,-1                 ; close this program (thread)
    int  0x40
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
    int  0x40

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
    int  0x40

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,10*65536+290         ; [x start] *65536 + [x size]
    mov  ecx,10*65536+130         ; [y start] *65536 + [y size]
    mov  esi,[thread_stack]
    sub  esi,0x1000
    shr  esi,4
    shl  esi,16
    add  ebx,esi
    add  ecx,esi
    mov  edx,0x03ffffff            ; color of work area RRGGBB,8->color gl
    mov  esi,0x808899ff            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x008899ff            ; color of frames    RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff            ; color of text RRGGBB
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40

    mov  eax,8                     ; NEW THREAD BUTTON
    mov  ebx,25*65536+128
    mov  ecx,88*65536+20
    mov  edx,2
    mov  esi,0x90b0d0 ;0x5577cc
    int  0x40

    mov  ebx,25*65536+35           ; draw info text with function 4
    mov  ecx,0x224466
    mov  edx,text
    mov  esi,40
  newline:
    mov  eax,4
    int  0x40
    add  ebx,10
    add  edx,40
    cmp  [edx],byte 'x'
    jne  newline


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


; DATA AREA

if lang eq ru
  text:
      db '’€ ƒ€ŒŒ€ ‘‡„€…’ ’Šˆ, ‡€“‘Š€Ÿ  '
      db '„ˆ ˆ ’’ †… Š„ Œƒ €‡. €Œ “†  '
      db '’‹œŠ ‡€’ˆ’œ‘Ÿ  ’„…‹œŒ ‘’Š…  '
      db '„‹Ÿ Š€†„ƒ ’Š€.                     '
      db '€ŒŸ’œ „‹Ÿ ‚‘…• ’Š‚ ™€Ÿ.          '
      db '                                        '
      db ' ‘‡„€’œ ‚›‰ ’Š                    '

      db 'x' ; <- END MARKER, DONT DELETE

  labelt:
       db   'ˆŒ… ˆ‘‹œ‡‚€ˆŸ 51®© ”“Š–ˆˆ'
  labellen:
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

  labelt:
       db   'THREAD EXAMPLE'
  labellen:
end if
I_END:
