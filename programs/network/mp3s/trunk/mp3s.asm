;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                             ;
;    Tiny MP3 Shoutcast Server v0.1 (vt)      ;
;                                             ;
;    Compile with FASM for Menuet             ;
;                                             ;
;    Listening to port 8008                   ;
;    Connect with eg: 192.168.1.22:8008       ;
;                                             ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

version  equ  '0.3'

  use32
  org     0x0
  db      'MENUET01'    ; 8 byte id
  dd      0x01          ; header version
  dd      START         ; program start
  dd      I_END         ; program image size
  dd      0x80000       ; memory usage
  dd      0x20000       ; stack
  dd      0,0

include 'lang.inc'
include '..\..\..\macros.inc'

; 0x0+      program image
; 0x1ffff   stack
; 0x20000   work area for file read
; 0x40000+  file send buffer ( 100 kb )


START:                          ; start of execution

    mov  [status],0
    call clear_input
    call draw_window            ; at first, draw the window

still:

    mov  eax,23                 ; wait here for event
    mov  ebx,2
    mcall

    call check_events

    call check_connection_status

    cmp  [status],4
    je   start_transmission

    jmp  still


check_events:

    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jz   key
    cmp  eax,3                  ; button in buffer ?
    jz   button

    ret

red:                           ; redraw
    call draw_window
    ret

key:
    mov  eax,2                 ; Just read it and ignore
    mcall
    ret

button:                         ; button

    mov  eax,17                 ; get id
    mcall

    cmp  ah,1                   ; close
    jne  no_close
    mov  eax,-1
    mcall
  no_close:

    cmp  ah,2                   ; button id=2 ?
    jnz  tst3
    ; open socket
    mov  eax,53
    mov  ebx,5
    mov  ecx,8008   ; local port # - http
    mov  edx,0      ; no remote port specified
    mov  esi,0      ; no remote ip specified
    mov  edi,0      ; PASSIVE open
    mcall
    mov  [socket], eax
    mov  [posy],1
    mov  [posx],0
    mov  [read_on],1
    call check_for_incoming_data
    call draw_window
    ret
  tst3:

    cmp  ah,4
    je   close_socket
    cmp  ah,6
    je   close_socket
    jmp  no_socket_close
  close_socket:
    mov  edx,eax
    ; Close socket
    mov  eax, 53
    mov  ebx, 8
    mov  ecx, [socket]
    mcall
    mov  esp,0x1fff0
    cmp  dh,6
    je   read_string
    jmp  still
  no_socket_close:

    cmp  ah,9
    jne  no_bps_add
    add  [bps],8*1000
    call draw_window
    ret
  no_bps_add:

    cmp  ah,8
    jne  no_bps_sub
    sub  [bps],8*1000
    call draw_window
    ret
  no_bps_sub:


    ret


clear_input:

    mov  edi,input_text
    mov  eax,0
    mov  ecx,60*40
    cld
    rep  stosb

    ret


read_string:

    mov  [addr],dword filename
    mov  [ya],dword 95

    mov  edi,[addr]
    mov  eax,32
    mov  ecx,30
    cld
    rep  stosb

    call print_text

    mov  edi,[addr]

  f11:
    mov  eax,10
    mcall
    cmp  eax,2
    jne  read_done
    mov  eax,2
    mcall
    shr  eax,8
    cmp  eax,13
    je   read_done
    cmp  eax,8
    jnz  nobsl
    cmp  edi,[addr]
    jz   f11
    sub  edi,1
    mov  [edi],byte 32
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

    add  edi,1
    mov  esi,[addr]
    add  esi,30
    cmp  esi,edi
    jnz  f11

  read_done:

    mov  ecx,40
    mov  eax,0
    cld
    rep  movsb

    call print_text

    jmp  still


print_text:

    pusha

    mov  eax,13
    mov  ebx,56*65536+30*6
    mov  ecx,[ya]
    shl  ecx,16
    mov  cx,8
    mov  edx,0xffffff
    mcall

    mov  eax,4
    mov  edx,[addr]
    mov  ebx,56*65536
    add  ebx,[ya]
    mov  ecx,0x000000
    mov  esi,30
    mcall

    popa
    ret


wait_for  dd 0x0

transmission_start  dd  0x0
sentbytes           dd  0x0

start_transmission:

    call clear_input

    mov  eax,5
    mov  ebx,50
    mcall

    call check_for_incoming_data
    call draw_window

    call send_header

    mov  [fileinfo+4],dword 0   ; start from beginning
    mov  [read_to],0x40000
    mov  [playpos],0x40000

    mov  ecx,1024 / 512

  new_buffer:

    mov  eax,[read_to]
    mov  ebx,1
    call read_file

    loop new_buffer


  newpart:

    call check_connection_status
    call draw_window

    mov  eax,26
    mov  ebx,9
    mcall
    mov  [transmission_start],eax
    mov  [sentbytes],0

  newblock:

    mov  eax,[read_to]
    mov  ebx,2
    call read_file

  wait_more:

    mov  eax,26
    mov  ebx,9
    mcall

    cmp  eax,[wait_for]
    jge  nomw

    mov  eax,5
    mov  ebx,1
    mcall

    jmp  wait_more

  nomw:

    add  eax,2
    mov  [wait_for],eax

    mov  eax,11
    mcall
    call check_events

    mov  eax,53
    mov  ebx,255
    mov  ecx,103
    mcall

    cmp  eax,0
    jne  wait_more

    ; write to socket
    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,[playadd]
    mov  esi,[playpos]
    mcall

    add  [sentbytes],edx

    mov  esi,[playpos]
    add  esi,[playadd]
    mov  edi,0x40000
    mov  ecx,110000 / 4
    cld
    rep  movsd

    mov  eax,[playadd]
    sub  [read_to],eax

    call check_for_incoming_data
    call show_progress
    call check_rate

    mov  eax, 53
    mov  ebx, 6
    mov  ecx, [socket]
    mcall
    cmp  eax,4
    jne  end_stream

    cmp  [read_to],0x40000
    jge  newblock

  end_stream:

    ; Close socket

    mov  eax, 53
    mov  ebx, 8
    mov  ecx, [socket]
    mcall

    mov  eax,5
    mov  ebx,5
    mcall

    ; Open socket

    mov  eax,53
    mov  ebx,5
    mov  ecx,8008   ; local port # - http
    mov  edx,0      ; no remote port specified
    mov  esi,0      ; no remote ip specified
    mov  edi,0      ; PASSIVE open
    mcall
    mov  [socket], eax
    mov  [posy],1
    mov  [posx],0
    mov  [read_on],0

    call draw_window

    jmp  still


check_rate:

    pusha

    mov  eax,[bps]
    xor  edx,edx
    mov  ebx,8*100
    div  ebx
    shl  eax,1
    mov  [playadd],eax

    mov  eax,26
    mov  ebx,9
    mcall

    sub  eax,[transmission_start]
    shr  eax,1

    imul eax,[playadd]

    mov  edx,0x00dd00

    cmp  [sentbytes],eax
    jge  sendok

    sub  eax,20000
    cmp  [sentbytes],eax        ; a long buffer underrun correction
    jge  no_buffer_overrun      ; actually leads to overrun
    mov  [sentbytes],eax
  no_buffer_overrun:

    add  [playadd],150
    mov  edx,0xdd0000

  sendok:

    mov  eax,13
    mov  ebx,320*65536+10
    mov  ecx,105*65536+10
    mcall

    mov  eax,47
    mov  ebx,4*65536
    mov  ecx,[playadd]
    mov  edx,322*65536+106
    mov  esi,0x000000
;    mcall

    popa

    ret


show_progress:

    pusha

    mov  eax,13
    mov  ebx,236*65536+10*6
    mov  ecx,107*65536+8
    mov  edx,0xffffff
    mcall

    mov  ecx,[fileinfo+4]
    imul ecx,512

    mov  eax,47               ; file read
    mov  ebx,9*65536
    mov  edx,236*65536+107
    mov  esi,0x000000
    mcall

    popa
    ret


playpos  dd  0x100000
playadd  dd  256000 / 8 / 100


send_header:

    pusha

    mov   [playpos],0x40000

    mov   esi,fileinfo+5*4
    mov   edi,transname
    mov   ecx,30
    cld
    rep   movsb

    mov   eax, 53
    mov   ebx, 7
    mov   ecx, [socket]
    mov   edx, headere-headers
    mov   esi, headers
    mcall

    popa
    ret


read_file:

    cmp  [read_to],0x40000+2000
    jg   cache_ok
    mov  [read_on],1
  cache_ok:

    cmp  [read_to],0x40000+95500
    jg   no_read_1

    mov  [fileinfo+12],eax
    mov  [fileinfo+8],ebx

    mov  eax,58
    mov  ebx,fileinfo
    mcall

    cmp  eax,0
    jne  no_read_1

    mov  eax,[fileinfo+8]
    add  [fileinfo+4],eax

    add  [read_to],512*2

    ret

  no_read_1:

    mov  [read_on],0

    ret



check_for_incoming_data:

    pusha

    mov  eax, 53
    mov  ebx, 2
    mov  ecx, [socket]
    mcall

    cmp  eax,0
    je   _ret_now

  new_data:

    mov  eax, 53
    mov  ebx, 2
    mov  ecx, [socket]
    mcall

    cmp  eax,0
    je   _ret

    mov  eax,53
    mov  ebx,3
    mov  ecx,[socket]
    mcall

    cmp  bl,10
    jne  no_lf
    inc  [posy]
    mov  [posx],0
    jmp  new_data
  no_lf:

    cmp  bl,20
    jb   new_data

    inc  [posx]
    cmp  [posx],60
    jbe  xok
    inc  [posy]
    mov  [posx],0
  xok:

    cmp  [posy],12
    jbe  yok
    mov  [posy],1
  yok:

    mov  eax,[posy]
    imul eax,60
    add  eax,[posx]

    mov  [input_text+eax],bl

    jmp  new_data

  _ret:

;    call draw_window

  _ret_now:

    popa
    ret



check_connection_status:

    pusha

    mov  eax, 53
    mov  ebx, 6
    mov  ecx, [socket]
    mcall

    cmp  eax,[status]
    je   .ccs_ret
    mov  [status],eax
    add  eax,48
    mov  [text+20],al
    call draw_window
   .ccs_ret:

    popa
    ret




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    pusha

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

    mov  eax,0                     ; Draw Window
    mov  ebx,50*65536+410
    mov  ecx,100*65536+141
    mov  edx,0x13ffffff
    mov  edi,title
    mcall

    mov  eax,8                     ; Start server
    mov  ebx,(25)*65536+21
    mov  ecx,57*65536+10
    mov  edx,2
    mov  esi,0x409040
    mcall                      ; Stop server
;    mov  eax,8
    mov  ebx,(25)*65536+21
    mov  ecx,69*65536+10
    mov  edx,4
    mov  esi,0x904040
    mcall

    mov  esi,0x3366d0

;    mov  eax,8                     ; Enter filename
    mov  ebx,(25)*65536+21
    mov  ecx,93*65536+10
    mov  edx,6
    mcall
;    mov  eax,8                     ; Decrease transfer rate
    mov  ebx,(25)*65536+10
    mov  ecx,105*65536+10
;    mov  edx,8
    mcall
;    mov  eax,8                     ; Increase transfer rate
    mov  ebx,(36)*65536+10
    mov  ecx,105*65536+10
    mov  edx,9
    mcall

    mov  ebx,10*65536+35           ; draw info text
    mov  ecx,0x00000000
    mov  edx,text
    mov  esi,40
  newline:
    mov  eax,4
    mcall
    add  ebx,12
    add  edx,40
    cmp  [edx],byte 'x'
    jnz  newline

    mov  eax,4                     ; Filename
    mov  ebx,56*65536+95
    mov  ecx,0x000000
    mov  edx,filename
    mov  esi,30
    mcall

    mov  eax,[bps]
    xor  edx,edx
    mov  ebx,1000
    div  ebx
    mov  ecx,eax

    mov  eax,47
    mov  ebx,3*65536
    mov  edx,58*65536+107
    mov  esi,0x00000000
    mcall

    mov  [input_text+0],dword 'RECE'
    mov  [input_text+4],dword 'IVED'
    mov  [input_text+8],dword ':   '

    mov  ebx,230*65536+35           ; draw info text
    mov  ecx,0x00000000
    mov  edx,input_text
    mov  esi,28
    mov  edi,7
   newline2:
    mov  eax,4
    mcall
    add  ebx,10
    add  edx,60
    dec  edi
    jnz  newline2

    mov  eax,38
    mov  ebx,210*65536+210
    mov  ecx,22*65536+136
    mov  edx,0x6699cc ; 002288
    mcall

    mov  eax,38
    mov  ebx,211*65536+211
    mov  ecx,22*65536+136
    mov  edx,0x336699 ; 002288
    mcall

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    popa

    ret



; DATA AREA

text:
   db '        TCB status: 0                   '
   db '                                        '
   db '        Activate - port 8008            '
   db '        Stop server                     '
   db '                                        '
   db '    >                                   '
   db '   < >      Kbps                        '
   db 'x';  <- END MARKER, DONT DELETE

headers:

  db   'ICY 200 OK',13,10
  db   'icy-notice1:This stream requires Winamp or xmms',13,10
  db   'icy-url:http://www.menuetos.org',13,10
  db   'icy-pub: 1',13,10
  db   'icy-name: Menuet Mp3 Shoutcast Radio ',version,' - '
 transname:
  db   '                              ',13,10,13,10

headere:

title  db   'MP3 shoutcast server ',version,0

socket   dd  0
status   dd  0

posy     dd  1
posx     dd  0

read_on  db  1
read_to  dd  0

addr     dd  0
ya       dd  0

bps      dd  128*1000

fileinfo:  dd  0,0,0,0,0x20000
filename:  db  '/RD/1/MENUET.MP3',0
times 50   db  0

input_text:

I_END:
