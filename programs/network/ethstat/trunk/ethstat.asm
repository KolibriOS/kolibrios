;
;    Stack Status Monitor
;
;    Compile with FASM for Menuet
;
   
use32
 org	0x0
 db	'MENUET01'    ; header
 dd	0x01	      ; header version
 dd	START	      ; entry point
 dd	I_END	      ; image size
 dd	I_END+0x10000 ; required memory
 dd	I_END+0x10000 ; esp
 dd	0x0 , 0x0     ; I_Param , I_Path
   
include 'lang.inc'
include '..\..\..\macros.inc'
   
START:                          ; start of execution

    call draw_window            ; at first, draw the window
   
still:
    mov  eax,23                 ; wait here for event
    mov  ebx,200    ; Time out after 2s
    mcall
   
    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jz   key
    cmp  eax,3                  ; button in buffer ?
    jz   button
   
 ; read the stack status data, and write it to the screen buffer
   
 mov  eax, 53
 mov  ebx, 255
 mov  ecx, 6
 mcall
   
 mov  ebx, text + 24
 call printhex
   
 mov  eax, 53
 mov  ebx, 255
 mov  ecx, 2
 mcall
   
 mov  ebx, text + 107
 call printhex
   
 mov  eax, 53
 mov  ebx, 255
 mov  ecx, 5
 mcall
   
 mov  ebx, text + 107 + 40
 call printhex
   
 mov  eax, 53
 mov  ebx, 255
 mov  ecx, 4
 mcall
   
 mov  ebx, text + 107 + 80
 call printhex
   
 mov  eax, 53
 mov  ebx, 255
 mov  ecx, 100
 mcall
   
 mov  ebx, text + 258
 call printhex
   
 mov  eax, 53
 mov  ebx, 255
 mov  ecx, 101
 mcall
   
 mov  ebx, text + 258 + 40
 call printhex
   
 mov  eax, 53
 mov  ebx, 255
 mov  ecx, 102
 mcall
   
 mov  ebx, text + 258 + 80
 call printhex
   
 mov  eax, 53
 mov  ebx, 255
 mov  ecx, 103
 mcall
   
 mov  ebx, text + 258 + 120
 call printhex
   
red:                           ; redraw
    call draw_window
    jmp  still
   
key:                           ; Keys are not valid at this part of the
    mov  eax,2                  ; loop. Just read it and ignore
    mcall
    jmp  still
   
button:                        ; button
    mov  eax,17                 ; get id
    mcall
   
    cmp  ah,1                   ; button id=1 ?
    jnz  still
   
    or  eax,-1                 ; close this program
    mcall
   
    jmp  still
   
   
   
   
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
   
   
draw_window:
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall
   
                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,100*65536+260         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+205         ; [y start] *65536 + [y size]
    mov  edx,0x13224466            ; color of work area RRGGBB
    mov  edi,title                 ; WINDOW LABEL
    mcall
   
                                  
    ; Re-draw the screen text
    cld
    mov  eax,4
    mov  ebx,25*65536+35           ; draw info text with function 4
    mov  ecx,0xffffff
    mov  edx,text
    mov  esi,40
  newline:
    mcall
    add  ebx,16
    add  edx,40
    cmp  [edx],byte 'x'
    jnz  newline
   
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall
   
    ret
   
; Taken from PS.ASM
printhex:
; number in eax
; print to ebx
; xlat from hextable
 pusha
 mov esi, ebx
 add esi, 8
 mov ebx, hextable
 mov ecx, 8
phex_loop:
 mov edx, eax
 and eax, 15
 xlatb
 mov [esi], al
 mov eax, edx
 shr eax, 4
 dec esi
 loop phex_loop
 popa
 ret
   
   
; DATA AREA
   
text:
    db ' Ethernet card status : xxxxxxxx        '
    db '                                        '
    db ' IP packets received :     xxxxxxxx     '
    db ' ARP packets received :    xxxxxxxx     '
    db ' Dumped received packets : xxxxxxxx     '
    db '                                        '
    db ' EMPTY QUEUE    : xxxxxxxx              '
    db ' IPOUT QUEUE    : xxxxxxxx              '
    db ' IPIN  QUEUE    : xxxxxxxx              '
    db ' NET1OUT QUEUE  : xxxxxxxx              '
    db 'x <- END MARKER, DONT DELETE            '
   
   
title    db   'Stack Status',0
   
hextable db '0123456789ABCDEF'
   
   
I_END:
   
   
   
   
   
   
   
