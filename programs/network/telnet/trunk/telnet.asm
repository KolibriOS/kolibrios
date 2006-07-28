;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;    TERMINAL
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
                                                ; esp = 0x7FFF0
                dd      0x00000000              ; reserved=no extended header


include 'lang.inc'
include 'macros.inc'

START:                          ; start of execution

    ; Clear the screen memory
    mov     eax, '    '
    mov     edi,text
    mov     ecx,80*30 /4
    cld
    rep     stosd


    call draw_window


still:
    ; check connection status
    mov  eax,53
    mov  ebx,6
    mov  ecx,[socket]
    int  0x40

    mov     ebx, [socket_status]
    mov     [socket_status], eax

    cmp     eax, ebx
    je      waitev

    call    draw_window

waitev:
    mov  eax,23                 ; wait here for event
    mov  ebx,20
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

    ; any data from the socket?

    mov     eax, 53
    mov     ebx, 2
    mov     ecx, [socket]
    int     0x40
    cmp     eax, 0
    jne      read_input

    jmp  still


read_input:

    push ecx
    mov     eax, 53
    mov     ebx, 3
    mov     ecx, [socket]
    int     0x40
    pop  ecx

    call    handle_data

    push    ecx
    mov     eax, 53
    mov     ebx, 2
    mov     ecx, [socket]
    int     0x40
    pop     ecx
    cmp     eax, 0


    jne   read_input
    call draw_text
    jmp  still



handle_data:
    ; Telnet servers will want to negotiate options about our terminal window
    ; just reject them all.
    ; Telnet options start with the byte 0xff and are 3 bytes long.

    mov     al, [telnetstate]
    cmp     al, 0
    je      state0
    cmp     al, 1
    je      state1
    cmp     al, 2
    je      state2
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
    int     0x40
    ret

hd001:
    cmp  bl,13                          ; BEGINNING OF LINE
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
    jmp  newdata
  nobol:

    cmp  bl,10                            ; LINE DOWN
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

    cmp  bl,8                            ; BACKSPACE
    jne  nobasp
    mov  eax,[pos]
    dec  eax
    mov  [pos],eax
    mov  [eax+text],byte 32
    mov  [eax+text+60*80],byte 0
    jmp  newdata
   nobasp:

    cmp  bl,15                           ; CHARACTER
    jbe  newdata
    mov  eax,[pos]
    mov  [eax+text],bl
    mov  eax,[pos]
    add  eax,1
  cm1:
    mov  ebx,[scroll+4]
    imul ebx,80
    cmp  eax,ebx
    jb   noeaxz
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


  red:                          ; REDRAW WINDOW
    call draw_window
    jmp  still

  key:                          ; KEY
    mov  eax,2                  ; send to modem
    int  0x40

    mov     ebx, [socket_status]
    cmp     ebx, 4              ; connection open?
    jne     still               ; no, so ignore key

    shr  eax,8
    cmp  eax,178                ; ARROW KEYS
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

  button:                       ; BUTTON
    mov  eax,17
    int  0x40
    cmp  ah,1                   ; CLOSE PROGRAM
    jne  noclose

    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    int  0x40

     mov  eax,-1
     int  0x40
  noclose:
    cmp     ah, 2               ; Set IP
    jne     notip

    mov  [string_x], dword 78
    mov  [string_y], dword 276
    mov  [string_length], dword 15
    call read_string
    mov   esi,string-1
    mov   edi,ip_address
    xor   eax,eax
   ip1:
    inc   esi
    cmp   [esi],byte '0'
    jb    ip2
    cmp   [esi],byte '9'
    jg    ip2
    imul  eax,10
    movzx ebx,byte [esi]
    sub   ebx,48
    add   eax,ebx
    jmp   ip1
   ip2:
    mov   [edi],al
    xor   eax,eax
    inc   edi
    cmp   edi,ip_address+3
    jbe   ip1
    call draw_window


    jmp     still

notip:
    cmp     ah, 3               ; set port
    jne     notport

    mov  [string_x], dword 215
    mov  [string_y], dword 276
    mov  [string_length], dword 4
    call read_string
    mov   esi,string-1
    mov   edi,port
    xor   eax,eax
   ip11:
    inc   esi
    cmp   [esi],byte '0'
    jb    ip21
    cmp   [esi],byte '9'
    jg    ip21
    imul  eax,10
    movzx ebx,byte [esi]
    sub   ebx,48
    add   eax,ebx
    jmp   ip11
   ip21:
    mov   [edi],al
    inc   edi
    mov   [edi],ah
    call draw_window


    jmp     still

notport:
    cmp     ah, 4               ; connect
    jne     notcon

    mov     eax, [socket_status]
    cmp     eax, 4
    je     still
    call    connect

    jmp     still

notcon:
    cmp     ah,5                ; disconnect
    jne     notdiscon

    call    disconnect
    jmp  still

notdiscon:                      ; Echo Toggle
    cmp     ah, 6
    jne     still

    mov     al, [echo]
    not     al
    mov     [echo], al

    call    draw_window
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
    int  0x40
    pop     bx
    mov     al, [echo]
    cmp     al, 0
    je      tm_001

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
    int  0x40
    ret



connect:
    pusha

 mov     ecx, 1000  ; local port starting at 1000

getlp:
 inc     ecx
 push ecx
 mov     eax, 53
 mov     ebx, 9
 int     0x40
 pop     ecx
 cmp     eax, 0   ; is this local port in use?
 jz  getlp      ; yes - so try next

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
    movzx   edx, word [port]      ; telnet port id
    mov     edi,1      ; active open
    int     0x40
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
    int  0x40

    mov  eax,0                     ; DRAW WINDOW
    mov  ebx,100*65536+491 + 8 +15
    mov  ecx,100*65536+270 + 20     ; 20 for status bar
    mov  edx,[wcolor]
    add  edx,0x02000000
    mov  esi,0x80557799
    mov  edi,0x00557799
    int  0x40

    mov  eax,4                     ; WINDOW LABEL
    mov  ebx,8*65536+8
    mov  ecx,0x00ffffff
    mov  edx,labelt
    mov  esi,labellen-labelt
    int  0x40


    mov  eax,8                     ; CLOSE BUTTON
     mov  ebx,(491 + 20 -19)*65536+12

    mov  ecx,5*65536+12
    mov  edx,1
    mov  esi,0x557799
    int  0x40

    ; draw status bar
    mov     eax, 13
    mov     ebx, 4*65536+484 + 8 +15
    mov     ecx, 270*65536 + 3
    mov     edx, 0x00557799
    int     0x40

    mov  eax,8                     ; BUTTON 2: SET IP
    mov  ebx,4*65536+70
    mov  ecx,273*65536+12
    mov     esi, 0x00557799
    mov  edx,2
    int  0x40

    mov  eax,4                     ; Button text
    mov  ebx,6*65536+276
    mov  ecx,0x00ffffff
    mov  edx,setipt
    mov  esi,setiplen-setipt
    int  0x40


    mov  edi,ip_address             ; display IP address
    mov  edx,78*65536+276
    mov  esi,0x00ffffff
    mov  ebx,3*65536
  ipdisplay:
    mov  eax,47
    movzx ecx,byte [edi]
    int  0x40
    add  edx,6*4*65536
    inc  edi
    cmp  edi,ip_address+4
    jb   ipdisplay

    mov  eax,8                     ; BUTTON 3: SET PORT
    mov  ebx,173*65536+38
    mov  ecx,273*65536+12
    mov  edx,3
    mov     esi, 0x00557799
    int  0x40

    mov  eax,4                     ; Button text
    mov  ebx,178*65536+276
    mov  ecx,0x00ffffff
    mov  edx,setportt
    mov  esi,setportlen-setportt
    int  0x40


    mov  edx,216*65536+276           ; display port
    mov  esi,0x00ffffff
    mov  ebx,4*65536
    mov  eax,47
    movzx  ecx,word [port]
    int  0x40

    mov  eax,8                     ; BUTTON 4: Connect
    mov  ebx,250*65536+50
    mov  ecx,273*65536+12
    mov     esi, 0x00557799
    mov  edx,4
    int     0x40

    mov  eax,4                     ; Button text
    mov  ebx,255*65536+276
    mov  ecx,0x00ffffff
    mov  edx,cont
    mov  esi,conlen-cont
    int  0x40


    mov  eax,8                     ; BUTTON 5: disconnect
    mov  ebx,303*65536+70
    mov  ecx,273*65536+12
    mov  edx,5
    mov     esi, 0x00557799
    int     0x40


    mov  eax,4                     ; Button text
    mov  ebx,307*65536+276
    mov  ecx,0x00ffffff
    mov  edx,dist
    mov  esi,dislen-dist
    int  0x40


    mov  esi,contlen-contt          ; display connected status
    mov     edx, contt
    mov     eax, [socket_status]
    cmp     eax, 4                  ; 4 is connected
    je      pcon
    mov     esi,discontlen-discontt
    mov     edx, discontt
pcon:

    mov  eax,4                     ; status text
    mov  ebx,380*65536+276
    mov  ecx,0x00ffffff
    int  0x40


    mov  eax,8                     ; BUTTON 6: echo
    mov  ebx,460*65536+50
    mov  ecx,273*65536+12
    mov  edx,6
    mov     esi, 0x00557799
    int     0x40

    mov  edx,echot
    mov  esi,echolen-echot
    mov     al, [echo]
    cmp     al, 0
    jne     peo
    mov  edx,echoot
    mov  esi,echoolen-echoot

peo:
    mov  eax,4                     ; Button text
    mov  ebx,463*65536+276
    mov  ecx,0x00ffffff
    int  0x40


    xor  eax,eax
    mov  edi,text+80*30
    mov  ecx,80*30 /4
    cld
    rep  stosd

    call draw_text

    mov  eax,12
    mov  ebx,2
    int  0x40

    popa

    ret


draw_text:

    pusha

    mov  esi,text
    mov  eax,0
    mov  ebx,0
  newletter:
    mov  cl,[esi]
    cmp  cl,[esi+30*80]
    jne  yesletter
    jmp  noletter
  yesletter:
    mov  [esi+30*80],cl

    ; erase character

    pusha
    mov     edx, 0                  ; bg colour
    mov     ecx, ebx
    add     ecx, 26
    shl     ecx, 16
    mov     cx, 9
    mov     ebx, eax
    add     ebx, 6
    shl     ebx, 16
    mov     bx, 6
    mov     eax, 13
    int     0x40
    popa

    ; draw character

    pusha
    mov     ecx, 0x00ffffff
    push bx
    mov  ebx,eax
    add  ebx,6
    shl  ebx,16
    pop  bx
    add  bx,26
    mov  eax,4
    mov  edx,esi
    mov  esi,1
    int  0x40
    popa

  noletter:

    add  esi,1
    add  eax,6
    cmp  eax,80*6
    jb   newletter
    mov  eax,0
    add  ebx,10
    cmp  ebx,24*10
    jb   newletter

    popa
    ret


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
    int  0x40
    cmp  eax,2
    jne  read_done
    mov  eax,2
    int  0x40
    shr  eax,8
    cmp  eax,13
    je   read_done
    cmp  eax,8
    jnz  nobsl
    cmp  edi,string
    jz   f11
    sub  edi,1
    mov  [edi],byte '_'
    call print_text
    jmp  f11
  nobsl:
    cmp  eax,dword 31
    jbe  f11
    cmp  eax,dword 95
    jb   keyok
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
    mov  edx,0x00000000
    int  0x40

    mov  eax,4
    mov  ebx,[string_x]
    shl  ebx,16
    add  ebx,[string_y]
    mov  ecx,0x00ffffff
    mov  edx,string
    mov  esi,[string_length]
    int  0x40

    popa
    ret




; DATA AREA

telnetrep       db 0xff,0xfc,0x00
telnetstate     db 0

string_length  dd    16
string_x       dd    200
string_y       dd    60

string         db    '________________'

tx_buff         db  0, 10
ip_address      db  001,002,003,004
port            db  0,0
echo            db  0
socket          dd  0x0
socket_status   dd  0x0
pos             dd  80 * 1
scroll          dd  1
                dd  24
wcolor          dd  0x000000
labelt          db  'Telnet v0.1'
labellen:
setipt          db  'IP Address:    .   .   .'
setiplen:
setportt        db  'Port:'
setportlen:
cont            db  'Connect'
conlen:
dist            db  'Disconnect'
dislen:
contt           db  'Connected'
contlen:
discontt        db  'Disconnected'
discontlen:
echot        db  'Echo On'
echolen:
echoot        db  'Echo Off'
echoolen:



text:
I_END:
