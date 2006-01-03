;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;    ETHERNET SETUP
;;
   
use32
   
                  org    0x0
                  db     'MENUET00'              ; 8 byte id
                  dd     38                      ; required os
                  dd     START                   ; program start
                  dd     I_END                   ; program image size
                  dd     0x100000                ; required amount of memory
                  dd     0x00000000              ; reserved=no extended header
   
include 'lang.inc'
include 'macros.inc'
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
    mov  eax,-1                 ; close this program
    int  0x40
  noclose:
   
    cmp  ah,2
    jne  no_details
    mov  eax,19
    mov  ebx,file1
    mov  ecx,file2
    int  0x40
    jmp  still
  no_details:
   
    jmp  still
   
   
   
; WINDOW DEFINITIONS AND DRAW
   
   
draw_window:
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40
   
                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx, 50*65536+370         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+230         ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff            ; color of work area RRGGBB
    mov  esi,0x80557799            ; color of grab bar  RRGGBB,8->color glide
    mov  edi,esi                   ; color of frames    RRGGBB
    and  edi,0xffffff
    int  0x40
   
                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff            ; color of text RRGGBB
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40
                                   ; BUTTON
    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,202*65536+135         ; [x start] *65536 + [x size]
    mov  ecx,190*65536+16          ; [y start] *65536 + [y size]
    mov  edx,2                     ; button id
    mov  esi,edi                   ; button color RRGGBB
    int  0x40
   
    mov  ebx,6*65536+35           ; draw info text with function 4
    mov  ecx,0;xffffff
    mov  edx,text
    mov  esi,60
  newline:
    mov  eax,4
    int  0x40
    add  ebx,10
    add  edx,60
    cmp  [edx],byte 'x'
    jnz  newline
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40
   
    ret
   
   
; DATA AREA
   
   
text:
    db '  ETHERNET CONNECTION BETWEEN MENUET AND A TFTP SERVER      '
    db '                                                            '
    db '  1) CURRENT ETHERNET CODE IS FOR RTL 8029 AND i8255x       '
    db '     BASED PCI CARDS                                        '
    db '  2) START STACK CONFIG FROM NET MENU AND PRESS THE         '
    db '     READ BUTTON, ACTIVATE PACKET DRIVER, SET DESIRED       '
    db '     IP ADDRESS & APPLY                                     '
    db '  3) SET THE SERVERS IP ADDRESS TO EG. 192.168.1.24         '
    db '                                                            '
    db '  THE MENUET MACHINE SHOULD NOW BE ABLE TO RESPOND TO A     '
    db '  PING FROM THE SERVER, TRANSFER FILES USING TFTP AND USE   '
    db '  IRC CLIENT. SEE MENUET PAGES FOR MORE TCP/IP APPLICATIONS.'
    db '                                                            '
    db '  MOST LINUX DISTRIBUTIONS HAVE A TFTP SERVER INCLUDED      '
    db '  FOR MS YOU CAN DOWNLOAD TFTPD FROM TFTPD32.JOUNIN.NET     '
    db '                                                            '
    db '                                  DETAILED DESCRIPTION      '
    db 'x'
   
file1: db 'TINYPAD     '
file2: db 'STACK.TXT',0
   
labelt:
    db   'NETWORK INFO'
labellen:
   
I_END:
   
   
   
   
   