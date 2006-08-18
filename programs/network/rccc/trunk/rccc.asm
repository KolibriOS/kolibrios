;
;    Remote Control Center(Client)
;
;    Автор: Hex
;    Сайт: www.mestack.narod.ru
;
;    Описание:
;    Программа, предназначенная для управления удалённым компьютером.Клиентская
;    часть.
;
;    Compile with FASM for Menuet
;    Компилируется FASM'ом для Менуэт ОС
;


use32
	   org	  0x0

	   db	  'MENUET01'	  ; 8 byte id
	   dd	  0x01		  ; header version
	   dd	  START 	  ; start of code
	   dd	  I_END 	  ; size of image
	   dd	  0x5000	  ; memory for app
	   dd	  0x5000	  ; esp
	   dd	  0x0 , 0x0	  ; I_Param , I_Icon

include 'lang.inc'
include 'macros.inc'

START:					; start of execution

    mov  eax,53 	       ; open socket
    mov  ebx,0
    mov  ecx,0x6000	       ; local port
    mov  edx,0x6100	       ; remote port
    mov  esi,dword [remote_ip] ; remote IP
    int  0x40

    mov  [socket], eax

    mov  eax,53 		    ; send connect code
    mov  ebx,4
    mov  ecx,[socket]
    mov  edx,1
    mov  esi,connect
    int  0x40

    call draw_window		; at first, draw the window

still:

    mov  eax,23 		; wait here for event
    mov  ebx,1
    int  0x40

    cmp  eax,1			; redraw request ?
    jz	 red
    cmp  eax,2			; key in buffer ?
    jz	 key
    cmp  eax,3			; button in buffer ?
    jz	 button

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

    cmp  ah,1		       ; button id=1 ?
    jnz  noclose
    mov  eax, 53
    mov  ebx, 1
    mov  ecx, [socket]
    int  0x40
    mov  eax,-1
    int  0x40
  noclose:

    cmp  ah,2		       ; SEND SHUTDOWN COMMAND?
    je	 send_shutdown

    cmp  ah,3		       ; SEND REBOOT COMMAND?
    je	 send_reboot

    cmp  ah,4		       ; SEND SAVEFI COMMAND?
    je	 send_savefi

    cmp  ah,5		       ; SEND SAVEHI COMMAND?
    je	 send_savehi

    cmp  ah,6		       ; SEND HOTREBOOT COMMAND?
    je	 send_hotreboot

    cmp  ah,7		       ; SEND EXIT COMMAND?
    je	 send_exit

    jmp  still


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                              ;;
;;          SEND COMMANDS TO SERVER             ;;
;;                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
send_shutdown:

  mov  eax,53			  ; SEND CODE TO REMOTE
  mov  ebx,4
  mov  ecx,[socket]
  mov  edx,1
  mov  esi,sen_shutdown
  int  0x40

  jmp  still

send_reboot:

  mov  eax,53			  ; SEND CODE TO REMOTE
  mov  ebx,4
  mov  ecx,[socket]
  mov  edx,1
  mov  esi,sen_reboot
  int  0x40

  jmp  still

send_savefi:

  mov  eax,53			  ; SEND CODE TO REMOTE
  mov  ebx,4
  mov  ecx,[socket]
  mov  edx,1
  mov  esi,sen_savefi
  int  0x40

  jmp  still

send_savehi:

  mov  eax,53			  ; SEND CODE TO REMOTE
  mov  ebx,4
  mov  ecx,[socket]
  mov  edx,1
  mov  esi,sen_savehi
  int  0x40

  jmp  still

send_hotreboot:

  mov  eax,53			  ; SEND CODE TO REMOTE
  mov  ebx,4
  mov  ecx,[socket]
  mov  edx,1
  mov  esi,sen_hotreboot
  int  0x40

  jmp  still

send_exit:

  mov  eax,53			  ; SEND CODE TO REMOTE
  mov  ebx,4
  mov  ecx,[socket]
  mov  edx,1
  mov  esi,sen_exit
  int  0x40

  jmp  still

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

  mov  eax,4
  mov  ebx,30*65536+30
  mov  ecx,0x000000
  mov  edx,I_END
  mov  esi,15
  int  0x40

  jmp  still
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    int  0x40

				   ; DRAW WINDOW
    mov  eax,0			   ; function 0 : define and draw window
    mov  ebx,100*65536+250	   ; [x start] *65536 + [x size]
    mov  ecx,60*65536+280	   ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff 	   ; color of work area RRGGBB
    mov  esi,0x80aabbcc 	   ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x00aabbcc 	   ; color of frames    RRGGBB
    int  0x40

				   ; WINDOW LABEL
    mov  eax,4			   ; function 4 : write text to window
    mov  ebx,8*65536+8		   ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff 	   ; color of text RRGGBB
    mov  edx,labeltext		   ; pointer to text beginning
    mov  esi,lte-labeltext	   ; text length
    int  0x40

    mov  eax,8			   ; CONTROL BUTTONS
    mov  ebx,25*65536+9
    mov  ecx,113*65536+9
    mov  edx,2
    mov  esi,0x667788
    int  0x40
    newbut:
    int  0x40
    add  ecx,16*65536
    inc  edx
    cmp  edx,8
    jb	 newbut

    cld
    mov  ebx,25*65536+50	   ; draw info text with function 4
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

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    int  0x40

    ret


; DATA AREA


text:
if lang eq ru
    db ' Время сервера:                         '
    db '                                        '
    db '  Меню управления сервером:             '
    db '                                        '
    db '   - Выключить                          '
    db '   - Перезагрузить                      '
    db '   - Сохранить флоппи-имедж             '
    db '   - Сохранить имедж Ж. диска           '
    db '   - Горячий рестарт ядра               '
    db '   - Закрытие серверной части           '
    db '                                        '
    db ' Локальный адрес : 192.168.0.1          '
    db ' Удалённый адрес : 192.168.0.2          '
    db 'Адрес сервера - в конце исходника       '
    db 'x' ; <- END MARKER, DONT DELETE

else
    db ' On server:                             '
    db '                                        '
    db ' Server control menu:                   '
    db '                                        '
    db '   - Shutdown                           '
    db '   - Reboot                             '
    db '   - Save ramdisk image to floppy       '
    db '   - Save ramdisk image to hard disk    '
    db '   - Kernel restart                     '
    db '   - Close server part                  '
    db '                                        '
    db ' Local  address : 192.168.0.1           '
    db ' Remote address : 192.168.0.2           '
    db 'Address of server is in end of source   '
    db 'x' ; <- END MARKER, DONT DELETE
end if

labeltext:  db	'Remote Control Center(Client)'  ;
lte:

socket	dd  0x0

remote_ip  db  192,168,0,2

sen_shutdown   db  'S'
sen_reboot   db  'R'
sen_savefi   db  'F'
sen_savehi   db  'H'
sen_hotreboot	db  'O'
sen_exit db 'E'
connect db 'C'

I_END:
