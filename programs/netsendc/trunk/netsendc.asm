;
;    NetSend(Client)
;                                
;    Автор: Hex
;    Сайт: www.mestack.narod.ru
;    
;    Описание:
;    Программа для обмена сообщениями в сети.Клиентская часть.
;
;    Compile with FASM for Menuet
;    Компилируется FASM'ом для Менуэт ОС
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
   
START:                                  ; start of execution
   
    mov  eax,53                ; open socket
    mov  ebx,0
    mov  ecx,0x4000            ; local port
    mov  edx,0x5000            ; remote port
    mov  esi,dword [remote_ip]   ; node IP
    int  0x40
   
    mov  [socketNum], eax
   
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
   
    jmp  still
   
red:
    call draw_window
    jmp  still
   
key:
    mov  eax,2
    int  0x40
    jmp  still
   
button:
    mov  eax,17
    int  0x40
   
    cmp  ah,1                  ; button id=1 ?
    jnz  noclose
    mov  eax, 53
    mov  ebx, 1
    mov  ecx, [socketNum]
    int  0x40
    mov  eax,-1
    int  0x40
  noclose:
   
    cmp  ah,2                  ; SEND CODE ?
    je   send_xcode
   

    jmp  still
   
   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                              ;;
;;           SEND CODE TO REMOTE                ;;
;;                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   
send_xcode:

  mov  esi,send_data              ; header
  mov  edi,I_END
  mov  ecx,end_message-send_data
  cld
  rep  movsb
   
  mov  eax,53                     ; SEND CODE TO REMOTE
  mov  ebx,4
  mov  ecx,[socketNum]
  mov  edx,end_message-send_data
  mov  esi,I_END
  int  0x40
   
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
    mov  ebx,100*65536+250         ; [x start] *65536 + [x size]
    mov  ecx,60*65536+150          ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff            ; color of work area RRGGBB
    mov  esi,0x80aabbcc            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x00aabbcc            ; color of frames    RRGGBB
    int  0x40
   
                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff            ; color of text RRGGBB
    mov  edx,labeltext             ; pointer to text beginning
    mov  esi,lte-labeltext         ; text length
    int  0x40
   
    mov  eax,8                     ; SEND MESSAGE
    mov  ebx,50*65536+145
    mov  ecx,47*65536+13
    mov  edx,2
    mov  esi,0x667788
    int  0x40
   
    cld
    mov  ebx,25*65536+50           ; draw info text with function 4
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
    db '        Послать сообщение               '
    db '                                        '
    db ' Локальный адрес : 192.168.0.1          '
    db ' Удалённый адрес : 192.168.0.2          '
    db 'Текст и адрес в конце исходника         '
    db 'x <- END MARKER, DONT DELETE            '
   
   
labeltext:  db  'NetSend(Client)'  ;
lte:
   
socketNum   dd  0x0
   
remote_ip  db  192,168,1,2
   
picture_position dd 0x0
   
send_data   db  'Привет,это тест!Hello,this is a test!'
end_message:

   
I_END:
   
   
   
   
   
   
   