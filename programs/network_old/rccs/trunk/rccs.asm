;
;    Remote Control Center(Server)
;                                
;    Автор: Hex
;    Сайт: www.mestack.narod.ru
;    
;    Описание:
;    Программа, предназначенная для управления удалённым компьютером.Серверная 
;    часть.
;
;    Compile with FASM for Menuet
;    Компилируется FASM'ом для Менуэт ОС
;
   
use32
   
                org     0x0
   
 	        db     'MENUET01'      ; 8 byte id
  	        dd     0x01            ; header version
 	        dd     START           ; start of code
  	        dd     I_END           ; size of image
  	        dd     0x5000          ; memory for app
  	        dd     0x5000          ; esp
  	        dd     0x0 , 0x0       ; I_Param , I_Icon
   
include 'lang.inc'
include '..\..\..\macros.inc'
remote_ip  db  192,168,0,1
   
   
START:                      ; start of execution
   
    mov  eax, 53                  ; open receiver socket
    mov  ebx, 0
    mov  ecx, 0x6100              ; local port
    mov  edx, 0x6000              ; remote port
    mov  esi, dword [remote_ip]   ; remote IP
    mcall
    mov  [socket],eax
    mov  [0],eax                  ; save for remote code

red:   
    call draw_window            ; at first, draw the window
   
still:
   
    mov  eax,23                 ; wait here for event
    mov  ebx,1
    mcall
   
    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jz   key
    cmp  eax,3                  ; button in buffer ?
    jz   button
   
    mov  eax,53                 ; data from cluster terminal ?
    mov  ebx,2
    mov  ecx,[socket]
    mcall
   
    cmp  eax,0
    jne  data_arrived
   
    jmp  still
   
key:
    mov  eax,2
    mcall
    jmp  still
   
button:
   
    mov  eax,53
    mov  ebx,1
    mov  ecx,[socket]
    mcall
    or  eax,-1
    mcall
   
   
data_arrived:
   
    mov  eax,5                 ; wait a second for everything to arrive
    mov  ebx,10
    mcall
   
    mov  edi,I_END
   
  get_data:
   
    mov  eax,53
    mov  ebx,3
    mov  ecx,[socket]
    mcall
   
    mov  [edi],bl
    inc  edi
   
    mov  eax,53
    mov  ebx,2
    mov  ecx,[socket]
    mcall
   
    cmp  eax,0
    jne  get_data
   
    cmp  byte [I_END],'C'   ;Connect ?
    jne  no_con
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_con
    mov  esi,inp_con.len
    mcall
    add  [y],10

    jmp  still

no_con:
    cmp  byte [I_END],'S'   ; Shutdown ?
    jne  no_shut
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_shut
    mov  esi,inp_shut.len
    mcall
    add  [y],10

    mov  eax,18
    mov  ebx,9
    mov  ecx,2
    mcall

    jmp  still

no_shut:
    cmp  byte [I_END],'R'   ; Reboot ?
    jne  no_reb
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_reb
    mov  esi,inp_reb.len
    mcall
    add  [y],10

    mov  eax,18
    mov  ebx,9
    mov  ecx,3
    mcall
    jmp  still

no_reb:
    cmp  byte [I_END],'F'   ; Save image on floppi ?
    jne  no_savefi
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_savefi
    mov  esi,inp_savefi.len
    mcall
    add  [y],10

    mov  eax,18
    mov  ebx,9
    mov  ecx,1
    mcall
    jmp  still

no_savefi:
    cmp  byte [I_END],'H'   ; Save image on hard disk ?
    jne  no_savehi
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_savehi
    mov  esi,inp_savehi.len
    mcall
    add  [y],10

    mov  eax,18
    mov  ebx,6
    mov  ecx,2
    mcall

    jmp  still

no_savehi:
    cmp  byte [I_END],'O'   ; Hot reboot ?
    jne  no_hotreb
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_hotreb
    mov  esi,inp_hotreb.len
    mcall
    add  [y],10

    mov  eax,18
    mov  ebx,9
    mov  ecx,4
    mcall
    jmp  still

no_hotreb:
    cmp  byte [I_END],'E'   ; Unload server ?
    jne  no_com
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_exit
    mov  esi,inp_exit.len
    mcall
    add  [y],10
    
    call button
    jmp  still

no_com:
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_com
    mov  esi,inp_com.len
    mcall
    add  [y],10

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
    mov  ebx,100*65536+300         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+330         ; [y start] *65536 + [y size]
    mov  edx,0x14ffffff            ; color of work area RRGGBB
    mov  edi,title                 ; WINDOW LABEL
    mcall
   
   
    ; Re-draw the screen text
    cld
    mov  eax,4
    mov  ebx,10*65536+30           ; draw info text with function 4
    mov  ecx,0x000000
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
   
   
; DATA AREA
   
   
text:
if lang eq ru
    db 'Данный адрес        : 192.168.0.2       '
    db 'Прослушиваемый порт : 0x6100            '
    db 'Состояние:                              '
    db 'x' ; <- END MARKER, DONT DELETE
else
    db 'This address        : 192.168.0.2       '
    db 'Used port           : 0x6100            '
    db 'Status:                                 '
    db 'x' ; <- END MARKER, DONT DELETE
end if
   
title  db  'Remote Control Center(Server)',0
   
socket   dd  0x0
y   dd   0x10
sysclock dd 0x0

if lang eq ru
inp_con db 'Внимание, подключился клиент!'
.len = $-inp_con
inp_shut db 'Идёт отключение системы...'
.len = $-inp_shut
inp_reb db 'Идёт перезагрузка...'
.len = $-inp_reb
inp_savefi db 'Сохраняем имедж на дискету...'
.len = $-inp_savefi
inp_savehi db 'Сохраняем имедж на Ж. диск...'
.len = $-inp_savehi
inp_hotreb db 'Идёт горячий рестарт ядра...'
.len = $-inp_hotreb
inp_exit db 'Выход из программы...'
.len = $-inp_exit
inp_com db 'Неопознанная комманда!'
.len = $-inp_com
else
inp_con db 'Note, client has been connected!'
.len = $-inp_con
inp_shut db 'Turn off in progress...'
.len = $-inp_shut
inp_reb db 'Reboot in progress...'
.len = $-inp_reb
inp_savefi db 'Saving image to floppy...'
.len = $-inp_savefi
inp_savehi db 'Saving image to hard disk...'
.len = $-inp_savehi
inp_hotreb db 'Kernel restart in progress...'
.len = $-inp_hotreb
inp_exit db 'Exiting from program...'
.len = $-inp_exit
inp_com db 'Unknown command!'
.len = $-inp_com
end if
I_END:
