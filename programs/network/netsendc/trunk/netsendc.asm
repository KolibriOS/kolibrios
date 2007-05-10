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
   
                db      'MENUET01'              ; 8 byte id
                dd      1                       ; header version
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      mem                     ; required amount of memory
                dd      mem                     ; stack pointer
                dd      0, 0                    ; param, icon
   
include 'lang.inc'
include '..\..\..\macros.inc'
   
START:                                  ; start of execution
   
    mov  eax,53                ; open socket
    mov  ebx,0
    mov  ecx,0x4000            ; local port
    mov  edx,0x5000            ; remote port
    mov  esi,dword [remote_ip]   ; node IP
    mcall
   
    mov  [socketNum], eax

red: 
    call draw_window            ; at first, draw the window
   
still:
   
    mov  eax,10                 ; wait here for event
    mcall

    dec  eax
    jz   red
    dec  eax
    jnz  button

key:
    mov  al,2
    mcall
    jmp  still
   
button:
    mov  al,17
    mcall
   
    dec  ah                    ; button id=1 ?
    jnz  noclose
    mov  eax, 53
    mov  ebx, 1
    mov  ecx, [socketNum]
    mcall
    or  eax,-1
    mcall
  noclose:
; it was not close button, so it must be send code button

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                              ;;
;;           SEND CODE TO REMOTE                ;;
;;                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   
send_xcode:

  mov  eax,53                     ; SEND CODE TO REMOTE
  mov  ebx,4
  mov  ecx,[socketNum]
  mov  edx,end_message-send_data
  mov  esi,send_data
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
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+250         ; [x start] *65536 + [x size]
    mov  ecx,60*65536+150          ; [y start] *65536 + [y size]
    mov  edx,0x13ffffff            ; color of work area RRGGBB
    mov  edi,title                 ; WINDOW LABEL
    mcall
   
   
    mov  eax,8                     ; SEND MESSAGE
    mov  ebx,50*65536+145
    mov  ecx,47*65536+13
    mov  edx,2
    mov  esi,0x667788
    mcall
   
    mov  eax,4
    mov  ebx,25*65536+50           ; draw info text with function 4
    mov  ecx,0x000000
    mov  edx,text
    mov  esi,40
  newline:
    mcall
    add  ebx,16
    add  edx,esi
    cmp  [edx],byte 'x'
    jnz  newline
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall
   
    ret
   
   
; DATA AREA
   
if lang eq ru   
text:
    db '        Послать сообщение               '
    db '                                        '
    db ' Локальный адрес : 192.168.0.1          '
    db ' Удалённый адрес : 192.168.0.2          '
    db 'Текст и адрес в конце исходника         '
    db 'x' ; <- END MARKER, DONT DELETE
else
text:
    db '          Send message                  '
    db '                                        '
    db ' Local  address : 192.168.0.1           '
    db ' Remote address : 192.168.0.2           '
    db 'Text and address in end of source       '
    db 'x' ; <- END MARKER, DONT DELETE
end if   
   
title  db  'NetSend(Client)',0
   
remote_ip  db  192,168,1,2
   
send_data   db  'Привет,это тест!Hello,this is a test!'
end_message:

   
I_END:
align 4
socketNum dd ?

rb 32 ; this is for stack

mem:

