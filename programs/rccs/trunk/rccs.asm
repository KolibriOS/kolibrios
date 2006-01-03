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
include 'macros.inc'
remote_ip  db  192,168,0,1
   
   
START:                      ; start of execution
   
    mov  eax, 53                  ; open receiver socket
    mov  ebx, 0
    mov  ecx, 0x6100              ; local port
    mov  edx, 0x6000              ; remote port
    mov  esi, dword [remote_ip]   ; remote IP
    int  0x40
    mov  [socket],eax
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
    mov  ecx,[socket]
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
    mov  ecx,[socket]
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
    mov  ecx,[socket]
    int  0x40
   
    mov  [edi],bl
    inc  edi
   
    mov  eax,53
    mov  ebx,2
    mov  ecx,[socket]
    int  0x40
   
    cmp  eax,0
    jne  get_data
   
    cmp  byte [I_END],'C'   ;Connect ?
    jne  no_con
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_con
    mov  esi,29
    int  0x40
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
    mov  esi,26
    int  0x40
    add  [y],10

    mov  eax,18
    mov  ebx,9
    mov  ecx,2
    int  0x40

    jmp  still

no_shut:
    cmp  byte [I_END],'R'   ; Reboot ?
    jne  no_reb
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_reb
    mov  esi,20
    int  0x40
    add  [y],10

    mov  eax,18
    mov  ebx,9
    mov  ecx,3
    int  0x40
    jmp  still

no_reb:
    cmp  byte [I_END],'F'   ; Save image on floppi ?
    jne  no_savefi
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_savefi
    mov  esi,29
    int  0x40
    add  [y],10

    mov  eax,18
    mov  ebx,9
    mov  ecx,1
    int  0x40
    jmp  still

no_savefi:
    cmp  byte [I_END],'H'   ; Save image on hard disk ?
    jne  no_savehi
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_savehi
    mov  esi,29
    int  0x40
    add  [y],10

    mov  eax,18
    mov  ebx,6
    mov  ecx,2
    int  0x40

    jmp  still

no_savehi:
    cmp  byte [I_END],'O'   ; Hot reboot ?
    jne  no_hotreb
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_hotreb
    mov  esi,28
    int  0x40
    add  [y],10

    mov  eax,18
    mov  ebx,9
    mov  ecx,4
    int  0x40
    jmp  still

no_hotreb:
    cmp  byte [I_END],'E'   ; Unload server ?
    jne  no_com
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_exit
    mov  esi,28
    int  0x40
    add  [y],10
    
    call button
    jmp  still

no_com:
    mov  eax,4
    mov  ebx,10*65536+60
    add  ebx,[y]
    mov  ecx,0x000000
    mov  edx,inp_com
    mov  esi,22
    int  0x40
    add  [y],10

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
    db 'Данный адрес        : 192.168.0.2       '
    db 'Прослушиваемый порт : 0x6100            '
    db 'Состояние:                              '
    db 'x <- END MARKER, DONT DELETE            '
   
   
labeltext:  db  'Remote Control Center(Server)'
lte:
   
socket   dd  0x0
y   dd   0x10
sysclock dd 0x0

inp_con db 'Внимание, подключился клиент!'
inp_shut db 'Идёт отключение системы...'
inp_reb db 'Идёт перезагрузка...'
inp_savefi db 'Сохраняем имедж на дискету...'
inp_savehi db 'Сохраняем имедж на Ж. диск...'
inp_hotreb db 'Идёт горячий рестарт ядра...'
inp_exit db 'Выход из программы...'
inp_com db 'Неопознанная комманда!'
I_END:
   
   
   
   
   
   
   