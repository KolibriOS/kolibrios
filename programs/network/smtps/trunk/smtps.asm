;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                   ;;
;;    SMTP server for MenuetOS                       ;;
;;                                                   ;;
;;    License: GPL / See file COPYING for details    ;;
;;    Copyright 2002 (c) Ville Turjanmaa             ;;
;;                                                   ;;
;;    Compile with FASM for Menuet                   ;;
;;                                                   ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

version equ '0.1'

use32

                org     0x0

                db      'MENUET01'              ; 8 byte id
                dd      0x01                    ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x200000                ; required amount of memory
                dd      0xffff0
                dd      0,0

include '..\..\..\macros.inc'

save_file:

;   cmp  [file_start],0x100000+10
;   jbe  nosub
;   sub  [file_start],8
;  nosub:

   mov  eax,[file_start]
   sub  eax,0x100000
   mov  ebx,files
   mov  [ebx+12],eax

   mov  eax,70
   mcall

   ret


START:                          ; start of execution

    mov  [file_start],0x100000

    mov  eax,70
    mov  ebx,filel
    mcall

    test eax,eax
    jz   @f
    cmp  eax,6
    jnz  notfound
@@:
    add  [file_start],ebx
  notfound:


    mov  edi,I_END
    mov  ecx,60*120
    mov  al,32
    cld
    rep  stosb

    mov  eax,[rxs]
    imul eax,11
    mov  [pos],eax

    mov  ebp,0
    mov  edx,I_END

redraw:
    call draw_window            ; at first, draw the window

still:

    inc  [cursor_on_off]

    mov  eax,5
    mov  ebx,1
    mcall

    mov  eax,11                 ; wait here for event
    mcall

    cmp  eax,1                  ; redraw
    je   redraw
    cmp  eax,2                  ; key
    je   key
    cmp  eax,3                  ; button
    je   button

    cmp  [I_END+120*60],byte 1
    jne  no_main_update
    mov  [I_END+120*60],byte 0
    mov  edx,I_END
    call draw_channel_text
  no_main_update:

    cmp  [server_active],0
    je   noread
    cmp  [status],4
    jne  noread
    call read_incoming_data
    inc  [close_connection]
    cmp  [close_connection],15*100
    jbe  noread

    call yq

  noread:

    call print_status

    cmp  [status],4
    je   check_header

    jmp  still


check_header:

    cmp [header_sent],1
    je  still

    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,6
    mov  esi,r220
    mcall
    mov  [header_sent],1

    jmp  still


button:                         ; button

    mov  eax,17                 ; get id
    mcall

    cmp  ah,1                   ; close program
    jne  noclose
    or   eax,-1
    mcall
  noclose:

    call socket_commands

    jmp  still


old_status dd 0x0

print_status:

    pusha

    mov  eax,53
    mov  ebx,6
    mov  ecx,[socket]
    mcall

    mov  [status],eax

    cmp  eax,[old_status]
    je   no_print

    mov  [old_status],eax

    push eax

    mov  eax,13
    mov  ebx,360*65536+30
    mov  ecx,151*65536+10
    mov  edx,0xffffff
    mcall

    pop  ecx
    mov  eax,47
    mov  ebx,3*65536
    mov  edx,360*65536+151
    mov  esi,0x000000

    cmp  [server_active],0
    je   no_print

    mcall

  no_print:

    popa

    ret


socket_commands:

    cmp  ah,22       ; open socket
    jnz  tst3
    mov  eax,3
    mcall

    mov  [server_active],1

    mov  eax,53
    mov  ebx,5
    mov  ecx,25     ; local port # - http
    mov  edx,0      ; no remote port specified
    mov  esi,0      ; no remote ip specified
    mov  edi,0      ; PASSIVE open
    mcall
    mov  [socket], eax

    ret
  tst3:


    cmp  ah,24     ; close socket
    jnz  no_24
    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    mcall
    mov  [header_sent],0
    mov  [mail_rp],0
    mov  [server_active],0

    ret
  no_24:


    ret



key:

    mov  eax,2
    mcall

    jmp  still



read_incoming_data:

    pusha

  read_new_byte:

    call read_incoming_byte
    cmp  ecx,-1
    je   no_data_in_buffer

    mov  eax,[file_start]
    mov  [eax],bl
    inc  [file_start]

    cmp  bl,10
    jne  no_start_command
    mov  [cmd],1
  no_start_command:

    cmp  bl,13
    jne  no_end_command
    mov  eax,[cmd]
    mov  [eax+command-2],byte 0
    call analyze_command
    mov  edi,command
    mov  ecx,250
    mov  eax,0
    cld
    rep  stosb
    mov  [cmd],0
  no_end_command:

    mov  eax,[cmd]
    cmp  eax,250
    jge  still

    mov  [eax+command-2],bl
    inc  [cmd]

    jmp  read_new_byte

  no_data_in_buffer:

    popa

    ret





analyze_command:

    pusha

    mov  [text_start],I_END
    mov  ecx,[rxs]
    imul ecx,11
    mov  [pos],ecx

    mov  bl,13
    call print_character
    mov  bl,10
    call print_character

    cmp  [cmd],2
    jbe  nott
    mov  ecx,[cmd]
    sub  ecx,2
    mov  esi,command+0
  newcmdc:
    mov  bl,[esi]
    call print_character
    inc  esi
    loop newcmdc

   nott:

    mov   edx,I_END
    call  draw_channel_text

  cmd_len_ok:

    cmp  [command],dword 'data'
    je   datacom
    cmp  [command],dword 'DATA'
    je   datacom
    cmp  [command],dword 'Data'
    je   datacom
    jmp  nodatacom
  datacom:
    inc  [mail_rp]
    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,6
    mov  esi,r354
    mcall
    mov  [cmd],0
    popa
    ret

  nodatacom:

    cmp  [mail_rp],0
    jne  nomrp0
    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,6
    mov  esi,r250
    mcall
    mov  [cmd],0
    popa
    ret
  nomrp0:



    cmp  [command],dword 'QUIT'
    je   yesquit
    cmp  [command],dword 'Quit'
    je   yesquit
    cmp  [command],dword 'quit'
    je   yesquit
    jmp  noquit
  yq:
     pusha

  yesquit:

    mov  [close_connection],0

    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,6
    mov  esi,r221
    mcall
    mov  [cmd],0

    mov  eax,5
    mov  ebx,5
    mcall

    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    mcall

    mov  eax,5
    mov  ebx,5
    mcall

    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    mcall

    mov  [header_sent],0
    mov  [mail_rp],0

    call save_file

    mov  eax,5
    mov  ebx,20
    mcall

    mov  eax,53
    mov  ebx,5
    mov  ecx,25     ; local port # - http
    mov  edx,0      ; no remote port specified
    mov  esi,0      ; no remote ip specified
    mov  edi,0      ; PASSIVE open
    mcall
    mov  [socket], eax

    popa
    ret
  noquit:



    cmp  [command],byte '.'
    jne  nodot
    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,6
    mov  esi,r250
    mcall
    mov  [cmd],0
    popa
    ret
  nodot:

    popa
    ret


r250  db  '250 ',13,10
r221  db  '221 ',13,10
r220  db  '220 ',13,10
r354  db  '354 ',13,10



draw_data:

    pusha

    add  eax,[text_start]
    mov  [eax],bl

    popa
    ret




print_text:

    pusha

    mov  ecx,command-2
    add  ecx,[cmd]

  ptr2:
    mov  bl,[eax]
    cmp  bl,dl
    je   ptr_ret
    cmp  bl,0
    je   ptr_ret
    call print_character
    inc  eax
    cmp  eax,ecx
    jbe  ptr2

  ptr_ret:

    mov  eax,[text_start]
    mov  [eax+120*60],byte 1

    popa
    ret



print_character:

    pusha

    cmp  bl,13     ; line beginning
    jne  nobol
    mov  ecx,[pos]
    add  ecx,1
  boll1:
    sub  ecx,1
    mov  eax,ecx
    xor  edx,edx
    mov  ebx,[rxs]
    div  ebx
    cmp  edx,0
    jne  boll1
    mov  [pos],ecx
    jmp  newdata
  nobol:

    cmp  bl,10     ; line down
    jne  nolf
   addx1:
    add  [pos],dword 1
    mov  eax,[pos]
    xor  edx,edx
    mov  ecx,[rxs]
    div  ecx
    cmp  edx,0
    jnz  addx1
    mov  eax,[pos]
    jmp  cm1
  nolf:
  no_lf_ret:


    cmp  bl,15    ; character
    jbe  newdata

    mov  eax,[irc_data]
    shl  eax,8
    mov  al,bl
    mov  [irc_data],eax

    mov  eax,[pos]
    call draw_data

    mov  eax,[pos]
    add  eax,1
  cm1:
    mov  ebx,[scroll+4]
    imul ebx,[rxs]
    cmp  eax,ebx
    jb   noeaxz

    mov  esi,[text_start]
    add  esi,[rxs]

    mov  edi,[text_start]
    mov  ecx,ebx
    cld
    rep  movsb

    mov  esi,[text_start]
    mov  ecx,[rxs]
    imul ecx,61
    add  esi,ecx

    mov  edi,[text_start]
    mov  ecx,[rxs]
    imul ecx,60
    add  edi,ecx
    mov  ecx,ebx
    cld
    rep  movsb

    mov  eax,ebx
    sub  eax,[rxs]
  noeaxz:
    mov  [pos],eax

  newdata:

    mov  eax,[text_start]
    mov  [eax+120*60],byte 1

    popa
    ret



read_incoming_byte:

    mov  eax, 53
    mov  ebx, 2
    mov  ecx, [socket]
    mcall

    mov  ecx,-1

    cmp  eax,0
    je   no_more_data

    mov  eax, 53
    mov  ebx, 3
    mov  ecx, [socket]
    mcall

    mov  ecx,0

  no_more_data:

    ret



draw_window:

    pusha

    mov  eax,12
    mov  ebx,1
    mcall

    mov  [old_status],300

    mov  eax,0                     ; draw window
    mov  ebx,5*65536+400
    mov  ecx,5*65536+200
    mov  edx,0x13ffffff
    mov  edi,title
    mcall

    mov  eax,8                     ; button: open socket
    mov  ebx,23*65536+22
    mov  ecx,169*65536+10
    mov  edx,22
    mov  esi,0x55aa55
    mcall

 ;   mov  eax,8                     ; button: close socket
    mov  ebx,265*65536+22
    mov  edx,24
    mov  esi,0xaa5555
    mcall

    mov  eax,38                    ; line
    mov  ebx,5*65536+395
    mov  ecx,108*65536+108
    mov  edx,0x000000
    mcall

    mov  eax,4
    mov  ebx,5*65536+123          ; info text
    mov  ecx,0x000000
    mov  edx,text
    mov  esi,70
  newline:
    mcall
    add  ebx,12
    add  edx,70
    cmp  [edx],byte 'x'
    jne  newline

    mov  edx,I_END                ; text from server
    call draw_channel_text

    mov  eax,12
    mov  ebx,2
    mcall

    popa

    ret





draw_channel_text:

    pusha

    mov   eax,4
    mov   ebx,10*65536+26
    mov   ecx,[scroll+4]
    mov   esi,[rxs]
  dct:
    pusha
    mov   cx,bx
    shl   ecx,16
    mov   cx,9
    mov   eax,13
    mov   ebx,10*65536
    mov   bx,word [rxs]
    imul  bx,6
    mov   edx,0xffffff
    mcall
    popa
    push  ecx
    mov   eax,4
    mov   ecx,0
    cmp   [edx],word '* '
    jne   no_red
    mov   ecx,0xff0000
   no_red:
    cmp   [edx],word '**'
    jne   no_light_blue
    cmp   [edx+2],byte '*'
    jne   no_light_blue
    mov   ecx,0x0000ff
  no_light_blue:
    cmp   [edx],byte '#'
    jne   no_blue
    mov   ecx,0x00ff00
  no_blue:
    mcall
    add   edx,[rxs]
    add   ebx,10
    pop   ecx
    loop  dct

    popa
    ret



text:

db '   Incoming mails are written to /rd/1/smtps.txt                      '
db '   The file can be fetched with TinyServer and a Html-browser.        '
db '   Timeout is set to 15 seconds.                                      '
db '                                                                      '
db '        Open SMTP server port 25                Close SMTP            '
db 'x' ; <- END MARKER, DONT DELETE


irc_server_ip   db      192,168,1,1

file_start      dd      0x100000

files:
       dd  2,0,0,?,0x100000
       db  '/rd/1/smtps.txt',0
filel:
       dd  0,0,0,0x100000,0x100000
       db  '/rd/1/smtps.txt',0


server_active dd 0

status  dd  0x0
header_sent db 0

channel_temp:         times   100   db   0
channel_temp_length   dd      0x0

close_connection   dd 0x0

mail_rp      dd  0

socket  dd  0x0

bgc  dd  0x000000
     dd  0x000000
     dd  0x00ff00
     dd  0x0000ff
     dd  0x005500
     dd  0xff00ff
     dd  0x00ffff
     dd  0x770077

tc   dd  0xffffff
     dd  0xff00ff
     dd  0xffffff
     dd  0xffffff
     dd  0xffffff
     dd  0xffffff
     dd  0xffffff
     dd  0xffffff

cursor_on_off  dd  0x0

max_windows    dd  20

thread_stack   dd  0x9fff0
thread_nro     dd 1
thread_screen  dd I_END+120*80*1

action_header_blue  db  10,'*** ',0
action_header_red   db  10,'*** ',0

action_header_short db  10,'* ',0

posx             dd  0x0
incoming_pos     dd  0x0
incoming_string: times 128 db 0

pos          dd  0x0

text_start   dd  I_END
irc_data     dd  0x0
print        db  0x0
cmd          dd  0x0
rxs          dd  56

res:         db  0,0
command:     times  256  db 0x0

nick         dd  0,0,0
irc_command  dd  0,0

command_position  dd 0x0
counter           dd  0
send_to_server    db 0

channel_list:     times 32*20 db 32
send_to_channel   dd 0x0

send_string:         times  100  db  0x0

xpos        dd  0
attribute   dd  0
scroll      dd  1
            dd  8

numtext     db  '                     '

title       db  'Tiny SMTP email server v ',version,0

I_END:
