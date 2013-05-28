;
;    CHESS CLIENT for CHESSCLUB.COM (VT)
;
;    Compile with FASM for Menuet
;

appname equ 'Chess Client for Chessclub.com '
version equ '0.2'

use32
 org	0x0
 db	'MENUET01'    ; header
 dd	0x01	      ; header version
 dd	START	      ; entry point
 dd	I_END	      ; image size
 dd	0x100000      ; required memory
 dd	0x100000      ; esp
 dd	0x0 , 0x0     ; I_Param , I_Path

include 'lang.inc'
include '..\..\..\macros.inc'

pawn_color:

     dd  0x000000
     dd  0x222222
     dd  0x444444
     dd  0xf0f0f0
     dd  0xc0c0c0
     dd  0xa0a0a0
     dd  0xa0a0a0
     dd  0x707070
     dd  0xb0b0b0
     dd  0xc0c0c0
     dd  0xd0d0d0
     dd  0xd8d8d8
     dd  0xe0e0e0
     dd  0xe8e8e8
     dd  0x00ff00
     dd  0xffffff



texts  equ  board_old+80*30

text   equ  texts+80*32*4


START:				; start of execution

    mov  esi,chess_bmp
    mov  edi,0x10000+18*3

    mov  ebx,0
    mov  ecx,0

  newp:

    xor  eax,eax
    mov  al,[esi]
    and  al,0xf0
    shr  al,4
    shl  eax,2
    mov  eax,[pawn_color+eax]
    mov  [edi+0],eax

    xor  eax,eax
    mov  al,[esi]
    and  al,0x0f
    shl  eax,2
    mov  eax,[pawn_color+eax]
    mov  [edi+3],eax

    add  edi,6
    add  esi,1

    inc  ebx
    cmp  ebx,23
    jbe  newp

    sub  edi,12

    mov  ebx,0

    inc  ecx
    cmp  ecx,279
    jb	 newp

    ; Clear the screen memory
    mov     eax, '    '
    mov     edi,text
    mov     ecx,80*30 /4
    cld
    rep     stosd


    call draw_window

still:

    call  check_for_board

    call  board_changed

    call  draw_board

    ; check connection status
    mov  eax,53
    mov  ebx,6
    mov  ecx,[socket]
    mcall

    mov     ebx, [socket_status]
    mov     [socket_status], eax

    cmp     eax, ebx
    je	    waitev

    call    display_status

waitev:
    mov  eax,23 		; wait here for event
    mov  ebx,20
    mcall

    cmp  eax,1			; redraw request ?
    je	 red
    cmp  eax,2			; key in buffer ?
    je	 key
    cmp  eax,3			; button in buffer ?
    je	 button

    ; any data from the socket?

    mov     eax, 53
    mov     ebx, 2
    mov     ecx, [socket]
    mcall
    cmp     eax, 0
    jne      read_input

    jmp  still


read_input:

    push ecx
    mov     eax, 53
    mov     ebx, 3
    mov     ecx, [socket]
    mcall
    pop  ecx

    call    handle_data

    push    ecx
    mov     eax, 53
    mov     ebx, 2
    mov     ecx, [socket]
    mcall
    pop     ecx
    cmp     eax, 0


    jne   read_input
    call draw_text
    jmp  still



check_for_board:

    pusha

     mov  esi,text-80
   news:
    add  esi,80
    cmp  esi,text+80*10
    je	 board_not_found
    cmp  [esi+12],dword '----'
    je	 cfb1
    jmp  news
   cfb1:
    cmp  [esi+16*80+12],dword '----'
    je	 cfb2
    jmp  news
  cfb2:
    cmp  [esi+2*80+12],dword '+---'
    jne  news

    cmp  [esi+4*80+12],dword '+---'
    jne  news

  board_found:

    mov  edi,chess_board
    mov  ecx,80*18
    cld
    rep  movsb

   board_not_found:

     popa

     ret


yst	dd  150
textx	equ 10
ysts	equ 410

boardx	dd 45
boardy	dd 45

boardxs dd 44
boardys dd 44

conx	equ 420
cony	equ 118

dconx	equ 420
dcony	equ 148

statusx equ 420
statusy equ 178


drsq:

     push eax ebx

     mov  ecx,ebx
     mov  ebx,eax

     mov  eax,ebx
     add  eax,ecx

     imul ebx,[boardxs]
     add  ebx,[boardx]
     shl  ebx,16
     imul ecx,[boardys]
     add  ecx,[boardy]
     shl  ecx,16

     add  ebx,[boardxs]
     add  ecx,[boardys]

     mov  edx,[sq_black]
     test eax,1
     jnz  dbl22
     mov  edx,[sq_white]
   dbl22:

     mov  eax,13
     mcall

     pop  ebx eax

     ret



draw_pawn:

;    edi,0  ; white / black
;    esi,0  ; from position 2  , 20 square
;    eax,2  ; board x
;    ebx,0  ; board y

     pusha

     call drsq

     cmp  esi,20
     jne  no_sqd

     popa
     ret

   no_sqd:

     imul eax,[boardxs]
     imul ebx,[boardys]

     add  eax,[boardx]
     add  ebx,[boardy]

     imul esi,44*45*3
     add  esi,0x10000+18*3

     mov  ecx,43

   dp0:

     pusha

     mov  ecx,44

   ldp1:

     pusha

     mov  ecx,ebx
     mov  ebx,eax

     mov  edx,[esi]
     and  edx,0xffffff
     mov  eax,1
     cmp  edx,0x00ff00
     je   nowp
     cmp  edi,1
     jne  nobl
     shr  edx,1
     and  edx,0x7f7f7f
   nobl:
     mcall
   nowp:

     popa

     add  esi,3
     add  eax,1

     dec  ecx
     jnz  ldp1

     popa

     add  ebx,1
     add  esi,3*44

     dec  ecx
     jnz  dp0

     popa

     ret


board_changed:

    pusha

    mov  eax,0
    mov  esi,chess_board
  bcl1:
    add  eax,[esi]
    add  esi,4
    cmp  esi,chess_board+19*80
    jb	 bcl1

    cmp  eax,[checksum]
    je	 bcl2
    mov  [changed],1
  bcl2:
    mov  [checksum],eax

    popa

    ret



checksum dd 0

changed db 1

draw_board:

     pusha

     cmp  [changed],1
     jne  no_change_in_board

     mov  [changed],0

     mov    eax,0
     mov    ebx,0
   scan_board:

     push   eax ebx

     mov    esi,ebx
     imul   esi,2
     imul   esi,80
     add    esi,80

     imul   eax,4
     add    eax,10

     add    esi,eax

     movzx  edx,word [chess_board+esi]
     cmp    dx,[board_old+esi]
     je     empty_slot

     mov    ecx,13
   newseek2:
     mov    edi,ecx
     imul   edi,8
     sub    edi,8
     cmp    dx,[edi+nappulat]
     je     foundnappula2
     loop   newseek2

     jmp    empty_slot

    foundnappula2:

     mov   esi,[edi+nappulat+4]
     mov   edi,0
     cmp   dl,'*'
     jne   nnbb
     mov   edi,1
   nnbb:
     mov   eax,[esp+4]
     mov   ebx,[esp]
     call  draw_pawn

    empty_slot:

     pop  ebx eax

     inc  eax
     cmp  eax,8
     jb   scan_board
     mov  eax,0
     inc  ebx
     cmp  ebx,8
     jb   scan_board

     mov  esi,chess_board
     mov  edi,board_old
     mov  ecx,80*19
     cld
     rep  movsb

     mov  eax,13
     mov  ebx,[boardx]
     sub  ebx,14
     shl  ebx,16
     add  ebx,8
     mov  ecx,[boardy]
     shl  ecx,16
     add  ecx,46*8
     mov  edx,[wcolor]
     mcall

     mov  eax,4 		   ; numbers at left
     mov  ebx,[boardx]
     sub  ebx,14
     shl  ebx,16
     add  ebx,[boardy]
     add  ebx,18
     mov  ecx,[tcolor]
     mov  edx,chess_board+80+5
     mov  esi,3
    db1:
     mcall
     add  edx,80*2
     add  ebx,[boardxs]
     cmp  edx,chess_board+80*16
     jb   db1

     mov  eax,13
     mov  ebx,[boardx]
     shl  ebx,16
     add  ebx,8*46
     mov  ecx,[boardys]
     imul ecx,8
     add  ecx,[boardy]
     add  ecx,8
     shl  ecx,16
     add  ecx,10
     mov  edx,[wcolor]
     mcall

     mov  eax,4 		   ; letters at bottom
     mov  ebx,[boardx]
     add  ebx,3
     shl  ebx,16
     mov  bx,word [boardys]
     imul bx,8
     add  ebx,[boardy]
     add  ebx,8
     mov  ecx,[tcolor]
     mov  edx,chess_board+80*17+8
     mov  esi,4
   db3:
     mcall
     mov  edi,[boardxs]
     shl  edi,16
     add  ebx,edi
     add  edx,4
     cmp  edx,chess_board+80*17+8+4*8
     jb   db3

     ; print player times

     mov  edi,74
     cmp  [chess_board+80+5],byte '1'
     jne  nowww2
     mov  edi,371
   nowww2:

     mov  eax,13
     mov  ebx,(conx)*65536+100
     mov  ecx,edi
     shl  ecx,16
     add  ecx,10
     mov  edx,[wcolor]
     mcall

     mov  eax,4
     mov  ebx,(conx)*65536
     add  ebx,edi
     mov  ecx,[tcolor]
     mov  edx,chess_board+80*7+59-1
     mov  esi,20
     mcall

     mov  edi,74
     cmp  [chess_board+80+5],byte '1'
     je   nowww
     mov  edi,371
   nowww:

     mov  eax,13
     mov  ebx,(conx)*65536+100
     mov  ecx,edi
     shl  ecx,16
     add  ecx,10
     mov  edx,[wcolor]
     mcall

     mov  eax,4
     mov  ebx,(conx)*65536
     add  ebx,edi
     mov  ecx,[tcolor]
     mov  edx,chess_board+80*9+59-1
     mov  esi,20
     mcall

     ; move #

     mov  eax,13
     mov  ebx,conx*65536+120
     mov  ecx,200*65536+10
     mov  edx,[wcolor]
     mcall

     mov  eax,4
     mov  ebx,conx*65536
     add  ebx,200
     mov  ecx,[tcolor]
     mov  edx,chess_board+80*1+46
     mov  esi,30
     mcall

   no_change_in_board:

     popa

     ret


handle_data:
    ; Telnet servers will want to negotiate options about our terminal window
    ; just reject them all.
    ; Telnet options start with the byte 0xff and are 3 bytes long.

    mov     al, [telnetstate]
    cmp     al, 0
    je	    state0
    cmp     al, 1
    je	    state1
    cmp     al, 2
    je	    state2
    jmp     hd001

state0:
    cmp     bl, 255
    jne     hd001
    mov     al, 1
    mov     [telnetstate], al
    ret

state1:
    mov     al, 2
    mov     [telnetstate], al
    ret

state2:
    mov     al, 0
    mov     [telnetstate], al
    mov     [telnetrep+2], bl

    mov     edx, 3
    mov     eax,53
    mov     ebx,7
    mov     ecx,[socket]
    mov     esi, telnetrep
    mcall
    ret

hd001:
    cmp  bl,13				; BEGINNING OF LINE
    jne  nobol
    mov  ecx,[pos]
    add  ecx,1
  boll1:
    sub  ecx,1
    mov  eax,ecx
    xor  edx,edx
    mov  ebx,80
    div  ebx
    cmp  edx,0
    jne  boll1
    mov  [pos],ecx

    call check_for_board

    jmp  newdata
  nobol:

    cmp  bl,10				  ; LINE DOWN
    jne  nolf
   addx1:
    add  [pos],dword 1
    mov  eax,[pos]
    xor  edx,edx
    mov  ecx,80
    div  ecx
    cmp  edx,0
    jnz  addx1
    mov  eax,[pos]
    jmp  cm1
  nolf:

     cmp  bl,9				   ; TAB
     jne  notab
    add  [pos],dword 8
    jmp  newdata
  notab:

    cmp  bl,8				 ; BACKSPACE
    jne  nobasp
    mov  eax,[pos]
    dec  eax
    mov  [pos],eax
    mov  [eax+text],byte 32
    mov  [eax+text+60*80],byte 0
    jmp  newdata
   nobasp:

    cmp  bl,15				 ; CHARACTER
    jbe  newdata
    mov  eax,[pos]
    mov  [eax+text],bl
    mov  eax,[pos]
    add  eax,1
  cm1:
    mov  ebx,[scroll+4]
    imul ebx,80
    cmp  eax,ebx
    jb	 noeaxz
    mov  esi,text+80
    mov  edi,text
    mov  ecx,ebx
    cld
    rep  movsb
    mov  eax,ebx
    sub  eax,80
  noeaxz:
    mov  [pos],eax
  newdata:
    ret


  red:				; REDRAW WINDOW
    call draw_window
    jmp  still

  key:				; KEY
    mov  eax,2			; send to modem
    mcall

    mov     ebx, [socket_status]
    cmp     ebx, 4		; connection open?
    jne     still		; no, so ignore key

    shr  eax,8
    cmp  eax,178		; ARROW KEYS
    jne  noaup
    mov  al,'A'
    call arrow
    jmp  still
  noaup:
    cmp  eax,177
    jne  noadown
    mov  al,'B'
    call arrow
    jmp  still
  noadown:
    cmp  eax,179
    jne  noaright
    mov  al,'C'
    call arrow
    jmp  still
  noaright:
    cmp  eax,176
    jne  noaleft
    mov  al,'D'
    call arrow
    jmp  still
  noaleft:
  modem_out:

    call    to_modem

    jmp  still

  button:			; BUTTON
    mov  eax,17
    mcall
    cmp  ah,1			; CLOSE PROGRAM
    jne  noclose

    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    mcall

     mov  eax,-1
     mcall
  noclose:

    cmp     ah, 4		; connect
    jne     notcon

    mov     eax, [socket_status]
    cmp     eax, 4
    je	   still
    call    connect

    jmp     still

notcon:
    cmp     ah,5		; disconnect
    jne     notdiscon

    call    disconnect
    jmp  still

 notdiscon:

    jmp     still

arrow:

    push eax
    mov  al,27
    call to_modem
    mov  al,'['
    call to_modem
    pop  eax
    call to_modem

    ret


to_modem:
    pusha
    push    ax
    mov     [tx_buff], al
    mov     edx, 1
    cmp     al, 13
    jne     tm_000
    mov     edx, 2
tm_000:
    mov     eax,53
    mov     ebx,7
    mov     ecx,[socket]
    mov     esi, tx_buff
    mcall
    pop     bx
    mov     al, [echo]
    cmp     al, 0
    je	    tm_001

    push    bx
    call    handle_data
    pop     bx

    cmp     bl, 13
    jne     tm_002

    mov     bl, 10
    call    handle_data

tm_002:
    call    draw_text

tm_001:
    popa
    ret



disconnect:
    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    mcall
    ret



connect:
    pusha

 mov	 ecx, 1000  ; local port starting at 1000

getlp:
 inc	 ecx
 push ecx
 mov	 eax, 53
 mov	 ebx, 9
 mcall
 pop	 ecx
 cmp	 eax, 0   ; is this local port in use?
 jz  getlp	; yes - so try next

    mov     eax,53
    mov     ebx,5
    mov     dl, [ip_address + 3]
    shl     edx, 8
    mov     dl, [ip_address + 2]
    shl     edx, 8
    mov     dl, [ip_address + 1]
    shl     edx, 8
    mov     dl, [ip_address]
    mov     esi, edx
    movzx   edx, word [port]	  ; telnet port id
    mov     edi,1      ; active open
    mcall
    mov     [socket], eax

    popa

    ret



;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    pusha

    mov  eax,12
    mov  ebx,1
    mcall

    mov  eax,14
    mcall

    mov  ebx,eax
    mov  ecx,eax

    shr  ebx,16
    and  ebx,0xffff
    and  ecx,0xffff

    shr  ebx,1
    shr  ecx,1

    sub  ebx,275
    sub  ecx,235

    shl  ebx,16
    shl  ecx,16

    mov  eax,0 		    ; DRAW WINDOW
    mov  bx,550
    mov  cx,470
    mov  edx,[wcolor]
    add  edx,0x14000000
    mov  edi,title
    mcall

    call display_status

    mov  eax,8			   ; BUTTON 4: Connect
    mov  ebx,conx*65536+80
    mov  ecx,cony*65536+15
     mov  esi,[wbutton]
     mov  edx,4
    mcall
    mov  eax,4			   ; Button text
    mov  ebx,(conx+4)*65536+cony+4
    mov  ecx,0xffffff
    mov  edx,cont
    mov  esi,conlen-cont
    mcall


    mov  eax,8			   ; BUTTON 5: disconnect
    mov  ebx,dconx*65536+80
    mov  ecx,dcony*65536+15
    mov  edx,5
     mov  esi,[wbutton]
     mcall
    mov  eax,4			   ; Button text
    mov  ebx,(dconx+4)*65536+dcony+4
    mov  ecx,0x00ffffff
    mov  edx,dist
    mov  esi,dislen-dist
    mcall


    xor  eax,eax
    mov  edi,text+80*30
    mov  ecx,80*30 /4
    cld
    rep  stosd

    call draw_text

    mov  [changed],1

    mov  edi,board_old
    mov  ecx,80*19
    mov  al,0
    cld
    rep  stosb

    mov  eax,4
    mov  ebx,conx*65536+52
    mov  ecx,[tcolor]
    mov  edx,quick_start
    mov  esi,30

  prqs:

    mcall
    add  ebx,10
    add  edx,30
    cmp  [edx],byte 'x'
    jne  prqs

    mov  eax,12
    mov  ebx,2
    mcall

    popa

    ret


display_status:

    pusha

    ; draw status bar
    mov  eax, 13
    mov  ebx, statusx*65536+80
    mov  ecx, statusy*65536 + 16
    mov  edx, [wcolor]
    mcall

    mov  esi,contlen-contt	    ; display connected status
    mov  edx, contt
    mov  eax, [socket_status]
    cmp  eax, 4 		 ; 4 is connected
    je	 pcon
    mov  esi,discontlen-discontt
    mov  edx, discontt
  pcon:
    mov  eax,4			   ; status text
    mov  ebx,statusx*65536+statusy+2
     mov  ecx,[tcolor]
     mcall

    popa
    ret


nappulat:

    dd '*P  ',5
    dd '*K  ',3
    dd '*Q  ',4
    dd '*R  ',0
    dd '*N  ',1
    dd '*B  ',2

    dd '    ',20

    dd 'P   ',5
    dd 'K   ',3
    dd 'Q   ',4
    dd 'R   ',0
    dd 'N   ',1
    dd 'B   ',2


row   dd  0x0
col   dd  0x0



draw_text:

    mov  esi,text+80*24
    mov  edi,texts+80*3

  dtl1:

    cmp  [esi],dword 'logi'
    je	 add_text
    cmp  [esi],dword 'aics'
    je	 add_text
    cmp  [esi],dword 'You '
    je	 add_text
    cmp  [esi],dword 'Your'
    je	 add_text
    cmp  [esi],dword 'Game'
    je	 add_text
    cmp  [esi],dword 'Ille'
    je	 add_text
    cmp  [esi],dword 'No s'
    je	 add_text

    sub  esi,80
    cmp  esi,text
    jge  dtl1

  dtl2:

    mov  eax,13
    mov  ebx,10*65536+532
    mov  ecx,420*65536+40
     mov  edx,[wtcom]
     mcall

    mov  eax,4
    mov  ebx,10*65536+420
     mov  ecx,[wtxt]
     mov  edx,texts
    mov  esi,80

  dtl3:

    mcall
    add  edx,80
    add  ebx,10
    cmp  edx,texts+4*80
    jb	 dtl3

    ret

  add_text:

    pusha

    cld
    mov  ecx,80
    rep  movsb

    popa


    sub  esi,80
    sub  edi,80

    cmp  edi,texts
    jb	 dtl2

    jmp  dtl1


read_string:

    mov  edi,string
    mov  eax,'_'
    mov  ecx,[string_length]
    inc     ecx
    cld
    rep  stosb
    call print_text

    mov  edi,string
  f11:
    mov  eax,10
    mcall
    cmp  eax,2
    jne  read_done
    mov  eax,2
    mcall
    shr  eax,8
    cmp  eax,13
    je	 read_done
    cmp  eax,8
    jnz  nobsl
    cmp  edi,string
    jz	 f11
    sub  edi,1
    mov  [edi],byte '_'
    call print_text
    jmp  f11
  nobsl:
    cmp  eax,dword 31
    jbe  f11
    cmp  eax,dword 95
    jb	 keyok
    sub  eax,32
  keyok:
    mov  [edi],al
    call print_text

    inc  edi
    mov  esi,string
    add  esi,[string_length]
    cmp  esi,edi
    jnz  f11

  read_done:

    call print_text

    ret


print_text:

    pusha

    mov  eax,13
    mov  ebx,[string_x]
    shl  ebx,16
    add  ebx,[string_length]
    imul bx,6
    mov  ecx,[string_y]
    shl  ecx,16
    mov  cx,8
    mov  edx,[wcolor]
    mcall

    mov  eax,4
    mov  ebx,[string_x]
    shl  ebx,16
    add  ebx,[string_y]
    mov  ecx,[tcolor]
    mov  edx,string
    mov  esi,[string_length]
    mcall

    popa
    ret




; DATA AREA

telnetrep	db 0xff,0xfc,0x00
telnetstate	db 0

string_length  dd    16
string_x       dd    200
string_y       dd    60

string	       db    '________________'

tx_buff 	db  0, 10
ip_address	db  204,178,125,65
port		dw  5051 ;  0,0
echo		db  1
socket		dd  0x0
socket_status	dd  0x0
pos		dd  80 * 22
scroll		dd  1
		dd  24

wbutton 	dd  0x336688

wtcom		dd  0x336688 ; 0x666666
wtxt		dd  0xffffff

wcolor		dd  0xe0e0e0
tcolor		dd  0x000000

sq_black	dd  0x336688 ; 666666
sq_white	dd  0xffffff

title		db  appname,version,0

setipt		db  '               .   .   .'
setiplen:
setportt	db  '     '
setportlen:
cont		db  'Connect'
conlen:
dist		db  'Disconnect'
dislen:
contt		db  'Connected'
contlen:
discontt	db  'Disconnected'
discontlen:
echot	     db  'Echo On'
echolen:
echoot	      db  'Echo Off'
echoolen:

quick_start:

    db	'( OPPONENT )                  '

   times 16  db  '                             1'

    db	'Quick start:                  '
    db	'                              '
    db	'1 Connect                     '
    db	'2 login: "guest"              '
    db	'3 aics% "seek 10 0"           '
    db	'  (for a player)              '
    db	'  (wait)                      '
    db	'4 Play eg. "e7e5"             '
    db	'  or  "d2d4"                  '
    db	'5 aics% "resign"              '
    db	'  (quit game)                 '
    db	'6 Disconnect                  '

  times 5  db  '                              '

    db	'( YOU )                       '

    db	'x'


chess_board:

    times  80	 db 0

 db '     8    *R  *N  *B  *Q  *K  *B  *N  *R'
 db '                                        '

     times  80	db 0

 db '     7    *P  *P  *P  *P  *P  *P  *P  *P'
 db '                                        '

     times  80	db 0

 db '     6                                  '
 db '                                        '

     times  80	db 0

 db '     5                                  '
 db '                                        '

     times  80	db 0

 db '     4                                  '
 db '                                        '

     times  80	db 0

 db '     3                                  '
 db '                                        '

     times  80	db 0

 db '     2    P   P   P   P   P   P   P   P '
 db '                                        '

    times  80	   db 0

 db '     1    R   N   B   Q   K   B   N   R '
 db '                                        '

    times  80	   db 0

 db '          a   b   c   d   e   f   g   h '
 db '                                        '


    times  80*20 db 0

chess_bmp:
        file    'chess.bmp':22*3+4+24*2

board_old:


I_END:
