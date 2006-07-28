;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   MENU / DIALOG EXAMPLE
;
;   Compile with FASM for Menuet
;

use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x10000                  ; memory for app
               dd     0x10000                  ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

include 'lang.inc'
include 'macros.inc'
include 'dialogs1.inc'

menu_history dd 0x0

START:                          ; start of execution

     call draw_window_main

still:                          ; wait here for event

    mov  eax,23
    mov  ebx,2
    int  0x40

    cmp  eax,1                  ; process events
    je   red
    cmp  eax,2
    je   key
    cmp  eax,3
    je   button

    call check_mouse            ; DIALOG CHECK

    mov  eax,[menu_action]
    cmp  eax,[menu_history]
    je   nodisplay

    mov  [menu_history],eax

    mov  eax,13
    mov  ebx,220*65536+6*4
    mov  ecx,70*65536+8
    mov  edx,0xffffff
    int  0x40

    mov  eax,4                  ; show menu selections
    mov  ebx,220*65536+70
    mov  ecx,0x000000
    mov  edx,menu_action
    mov  esi,4
    int  0x40

  nodisplay:

    cmp  word [menu_action],word 'AD' ; user requests close
    jne  no_menu_close
    mov  eax,-1
    int  0x40
  no_menu_close:

    jmp  still

  red:                          ; redraw
    call draw_window_main
    jmp  still

  key:
    mov  eax,2                  ; key in buffer
    int  0x40
    jmp  still

  button:                       ; button in buffer
    mov  eax,17
    int  0x40

    cmp  ah,1                   ; close application
    jne  noclose
    mov  eax,-1
    int  0x40
  noclose:

    cmp  ah,2
    jne  no_alert_box           ; ALERT BOX
    mov  eax,170                ; window width
    mov  ebx,alert_text         ; asciiz string
    mov  ecx,1                  ; OK button
    call alert_box              ; function call
    jmp  still
  no_alert_box:

    cmp  ah,3
    jne  no_choose_box          ; CHOOSE BOX
    mov  eax,220                ; window width
    mov  ebx,choose_text        ; asciiz string
    mov  ecx,2                  ; YES/NO buttons
    call alert_box              ; function call
    jmp  still
  no_choose_box:


    jmp  still




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window_main:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

    mov  eax,0                     ; open window
    mov  ebx,100*65536+300
    mov  ecx,100*65536+120
    mov  edx,0x02ffffff
    mov  esi,0x805080d0
    mov  edi,0x005080d0
    int  0x40

    call draw_menu                 ; DRAW MENU

    mov  eax,4                     ; window label
    mov  ebx,8*65536+8
    mov  ecx,0x10ddeeff
    mov  edx,labelt
    mov  esi,labellen-labelt
    int  0x40

    mov  eax,8                     ; close button
    mov  ebx,(300-17)*65536+10
    mov  ecx,5*65536+10
    mov  edx,1
    mov  esi,0x4466bb
    int  0x40

    mov  eax,8                     ; button : OPEN ALERT BOX
    mov  ebx,25*65536+150
    mov  ecx,61*65536+14
    mov  edx,2
    mov  esi,0x4466aa
    int  0x40

    mov  eax,8                     ; button : OPEN CHOOSE BOX
    mov  ebx,25*65536+150
    mov  ecx,81*65536+14
    mov  edx,3
    mov  esi,0x4466aa
    int  0x40

    mov  ebx,20*65536+55           ; draw info text with function 4
    mov  ecx,0xffffff
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

text:
    db '                                        '
    db '   OPEN ALERT BOX                       '
    db '                                        '
    db '   OPEN CHOOSE BOX                      '

    db 'x <- END MARKER, DONT DELETE            '

labelt:
     db   'EXAMPLE APPLICATION'
labellen:

alert_text   db  '    File not found !',0
choose_text  db  '    Save file before exit ? ',0

I_END:
