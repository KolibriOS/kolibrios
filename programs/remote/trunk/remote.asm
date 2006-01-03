;
;    Remote processing example (remote node) - vt
;
;    Compile with FASM for Menuet
;
   
   
use32
   
                org     0x0
   
                db      'MENUET00'              ; 8 byte id
                dd      38                      ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x100000                ; required amount of memory
                dd      0x00000000              ; reserved=no extended header
   
include 'lang.inc'
include 'macros.inc'
remote_ip  db  192,168,1,26
   
   
START:                      ; start of execution
   
    mov  eax, 53                  ; open receiver socket
    mov  ebx, 0
    mov  ecx, 0x3000              ; local port
    mov  edx, 0xffff              ; remote port
    mov  esi, dword [remote_ip]   ; remote IP
    int  0x40
    mov  [socketNum],eax
    mov  [0],eax                  ; save for remote code
   
    call draw_window            ; at first, draw the window
   
still:
   
    mov  eax,23                 ; wait here for event
    mov  ebx,1
    int  0x40
   
    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jz   key
    cmp  eax,3                  ; button in buffer ?
    jz   button
   
    mov  eax,53                 ; data from cluster terminal ?
    mov  ebx,2
    mov  ecx,[socketNum]
    int  0x40
   
    cmp  eax,0
    jne  data_arrived
   
    jmp  still
   
red:
    call draw_window
    jmp  still
   
key:
    mov  eax,2
    int  0x40
    jmp  still
   
button:
   
    mov  eax,53
    mov  ebx,1
    mov  ecx,[socketNum]
    int  0x40
    mov  eax,-1
    int  0x40
   
   
data_arrived:
   
    mov  eax,5                 ; wait a second for everything to arrive
    mov  ebx,10
    int  0x40
   
    mov  edi,I_END
   
  get_data:
   
    mov  eax,53
    mov  ebx,3
    mov  ecx,[socketNum]
    int  0x40
   
    mov  [edi],bl
    inc  edi
   
    mov  eax,53
    mov  ebx,2
    mov  ecx,[socketNum]
    int  0x40
   
    cmp  eax,0
    jne  get_data
   
    add  byte [I_END+14],48
   
    mov  eax,4
    mov  ebx,10*65536+50
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,I_END
    mov  esi,23
    int  0x40
   
    add  [y],10
   
    cmp  byte [I_END+14],'1'   ; DATA PACKET ?
    jne  no_packet
    mov  esi,I_END+23
    mov  edi,[I_END+15]
    mov  ecx,[I_END+19]
    cld
    rep  movsb
    jmp  still
   no_packet:
   
    cmp  byte [I_END+14],'2'   ; EXECUTE ?
    jne  no_execute
    mov  eax,[I_END+15]
    call eax
    jmp  still
   no_execute:
   
    jmp  still
   
y   dd   0x10
   
   
   
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
   
   
draw_window:
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40
   
                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+286         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+330         ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff            ; color of work area RRGGBB
    mov  esi,0x80aabbcc            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x00aabbcc            ; color of frames    RRGGBB
    int  0x40
   
    mov  eax,8
    mov  ebx,(286-19)*65536+12
    mov  ecx,4*65536+12
    mov  edx,1
    mov  esi,0xaabbcc
;    int  0x40
   
                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff            ; color of text RRGGBB
    mov  edx,labeltext             ; pointer to text beginning
    mov  esi,lte-labeltext         ; text length
    int  0x40
   
    ; Re-draw the screen text
    cld
    mov  ebx,10*65536+30           ; draw info text with function 4
    mov  ecx,0x000000
    mov  edx,text
    mov  esi,40
  newline:
    mov  eax,4
    int  0x40
    add  ebx,16
    add  edx,40
    cmp  [edx],byte 'x'
    jnz  newline
   
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40
   
    ret
   
   
; DATA AREA
   
   
text:
    db 'THIS NODE         : 192.168.1.22        '
    db 'LISTENING TO PORT : 0x3000              '
    db 'x <- END MARKER, DONT DELETE            '
   
   
labeltext:  db  'CLUSTER REMOTE'
lte:
   
socketNum   dd  0x0
   
send_data   db  'MenuetRemote00'  ; 00  header      ; -> remote port 0x3000
            db  1                 ; 14  send
            dd  0x0               ; 15  position
            dd  0x0               ; 19  size
                                  ; 23
   
execute     db  'MenuetRemote00'  ; 00  header      ; -> remote port 0x3000
            db  2                 ; 14  execute
            dd  0x0               ; 15  position
                                  ; 19
   
I_END:
   
   
   
   
   
   
   