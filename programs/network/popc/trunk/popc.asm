;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                   ;;
;;    POP CLIENT for MenuetOS                        ;;
;;    - Modified from IRC client                     ;;
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

START:                          ; start of execution

    mov  [file_start],0x100000

    mov  eax,70
    mov  ebx,filel
    int  0x40

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
    call draw_window            ; at first, draw the window

still:

    mov  eax,5
    mov  ebx,1
    int  0x40

    mov  eax,11                 ; wait here for event
    int  0x40

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
    call draw_server_data
  no_main_update:

    cmp  [server_active],0
    je   noread
    call read_incoming_data
  noread:

    call print_status

    cmp  [status],4
    je   send_request

    jmp  still


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Save the fetched mails
;;


save_file:

   pusha

   mov  ebx,files
   mov  eax,[file_start]
   sub  eax,0x100000
   mov  [ebx+12],eax

   mov  eax,70
   int  0x40

   popa

   ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Send user id/passwd/mailrq
;;


send_request:

    inc  [mcounter]

    cmp  [mcounter],1000
    jbe  no_send

    mov  eax,[ccounter]
    imul  eax,32
    add  eax,getmail
    mov  esi,eax

    inc  [ccounter]

    mov  edx,32

    cmp  [ccounter],1
    jne  no1
    mov  edx,5+2
    add  edx,[l2]
  no1:

    cmp  [ccounter],2
    jne  no2
    mov  edx,5+2
    add  edx,[l3]
  no2:

    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    int  0x40
    mov  [mcounter],0

    cmp  [esi],dword 'quit'
    je   close_fetch


  no_send:

    jmp  still


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Close connection to server
;;


close_fetch:

    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,14
    mov  esi,quitc
    int  0x40
    mov  [mcounter],0

    mov  eax,5
    mov  ebx,150
    int  0x40

    call read_incoming_data

    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    int  0x40

    mov  eax,5
    mov  ebx,2
    int  0x40

    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    int  0x40

    mov  [server_active],0

    jmp  still


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  User input processing
;;


redraw:                         ; redraw

    call draw_window
    jmp  still


key:

    mov  eax,2
    int  0x40

    jmp  still


button:                         ; button

    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,60
    jne  no_open
        mov     eax, 70
        mov     ebx, tinypad_start
        int     0x40
    jmp  still
  no_open:

    cmp  ah,1                   ; close program
    jne  noclose
    mov  eax,-1
    int  0x40
  noclose:

    cmp  ah,51
    je   read_string
    cmp  ah,52
    je   read_string
    cmp  ah,53
    je   read_string

    call socket_commands

    jmp  still


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Socket open & close
;;

socket_commands:

    cmp  ah,22       ; open socket
    jnz  tst3

    mov  [server_active],1

    mov  [mcounter],900
    mov  [ccounter],0

    mov  eax,3
    int  0x40

    mov  eax,3
    int  0x40
    mov  ecx,eax
    and  ecx,0xffff

    mov  eax,53
    mov  ebx,5
    mov  edx,110
    mov  esi,dword [ip]
    mov  edi,1
    int  0x40
    mov  [socket], eax

    ret
  tst3:


    cmp  ah,24     ; close socket
    jnz  no_24
    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    int  0x40
    mov  [header_sent],0
    mov  [mail_rp],0
    mov  [server_active],0

    ret
  no_24:

    ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Display connection status
;;

old_status dd 0x0

print_status:

    pusha

    mov  eax,53
    mov  ebx,6
    mov  ecx,[socket]
    int  0x40

    mov  [status],eax

    cmp  eax,[old_status]
    je   nopr

    mov  [old_status],eax

    push eax

    mov  eax,13
    mov  ebx,200*65536+30
    mov  ecx,160*65536+10
    mov  edx,0xffffff
    int  0x40

    pop  ecx

    cmp  [server_active],1
    jne  nopr

    mov  eax,47
    mov  ebx,3*65536
    mov  edx,200*65536+160
    mov  esi,0x000000
    int  0x40

  nopr:

    popa

    ret





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Read data from server
;;


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
    call save_file
    call analyze_data
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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Read user input for ip/user/passwd
;;


read_string:

    shr  eax,8
    sub  eax,51
    mov  ebx,eax
    imul eax,12
    add  eax,181

    mov  [len],ebx
    shl  [len],2
    add  [len],l1

    imul ebx,50
    add  ebx,input1

    mov  [addr],ebx
    mov  [ya],eax

    mov  edi,[addr]
    mov  eax,0
    mov  ecx,30
    cld
    rep  stosb

    call print_input_text

    mov  edi,[addr]

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
    cmp  edi,[addr]
    jz   f11
    sub  edi,1
    mov  [edi],byte 32
    call print_text
    jmp  f11
  nobsl:
    mov  [edi],al

    call print_input_text

    add  edi,1
    mov  esi,[addr]
    add  esi,30
    cmp  esi,edi
    jnz  f11

  read_done:

    push edi

    mov  ecx,40
    mov  eax,32
    cld
    rep  stosb

    call print_input_text

    pop  edi
    sub  edi,[addr]
    mov  eax,[len]
    mov  [eax],edi

    cmp  [len],l1
    jne  noip
    mov  esi,input1
    mov  edi,ip_text+15
    mov  ecx,16
    cld
    rep  movsb
    call ip_set
   noip:

    cmp  [len],l2
    jne  nol2
    mov  esi,input2
    mov  edi,l2_text+15
    mov  ecx,22
    cld
    rep  movsb
    mov  esi,input2
    mov  edi,getmail+5
    mov  ecx,[l2]
    cld
    rep  movsb
    mov  al,13
    stosb
    mov  al,10
    stosb
   nol2:

    cmp  [len],l3
    jne  nol3
    mov  esi,input3
    mov  edi,getmail+32+5
    mov  ecx,[l3]
    cld
    rep  movsb
    mov  al,13
    stosb
    mov  al,10
    stosb
   nol3:

    call draw_window

    jmp  still



print_input_text:

    pusha

    mov  eax,13
    mov  ebx,95*65536+23*6
    mov  ecx,[ya]
    shl  ecx,16
    mov  cx,9
    mov  edx,0xffffff
    int  0x40

    cmp  [len],l3
    je   noprt

    mov  eax,4
    mov  edx,[addr]
    mov  ebx,95*65536
    add  ebx,[ya]
    mov  ecx,0x000000
    mov  esi,23
    int  0x40

  noprt:

    popa
    ret


ip_set:

    mov   esi,input1-1
    mov   edi,ip
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
    cmp   edi,ip+3
    jbe   ip1
    ret
   no_read_ip:

    ret


analyze_data:

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
    call  draw_server_data

  cmd_len_ok:

    cmp  [command],dword '-ERR'
    je   close_fetch

    cmp  [command],word '+O'
    jne  nook
    mov  [mcounter],990
  nook:

    popa

    ret



draw_data:

    push eax

    add  eax,[text_start]
    mov  [eax],bl

    pop  eax
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
    int  0x40

    mov  ecx,-1

    cmp  eax,0
    je   no_more_data

    mov  eax, 53
    mov  ebx, 3
    mov  ecx, [socket]
    int  0x40

    mov  ecx,0

  no_more_data:

    ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Window definitions
;;



draw_window:

    pusha

    mov  eax,12
    mov  ebx,1
    int  0x40

    mov  eax,0                     ; draw window
    mov  ebx,5*65536+435
    mov  ecx,5*65536+232
    mov  edx,0x13ffffff
    mov  edi,labelt
    int  0x40

    mov  [old_status],300

    mov  eax,8                     ; button: open socket
    mov  ebx,23*65536+22
    mov  ecx,155*65536+10
    mov  edx,22
    mov  esi,0x44cc44
    int  0x40

;    mov  eax,8                     ; button: close socket
    mov  ebx,295*65536+22
    mov  ecx,155*65536+10
    mov  edx,24
    mov  esi,0xcc4444
    int  0x40

;    mov  eax,8                     ; button: text entries
    mov  ebx,243*65536+8
    mov  ecx,180*65536+8
    mov  edx,51
    mov  esi,0x4488dd
  newi:
    int  0x40
    inc  edx
    add  ecx,12*65536
    cmp  edx,53
    jbe  newi

;    mov  eax,8                     ; open inbox
    mov  ebx,295*65536+102
    mov  ecx,190*65536+14
    mov  edx,60
    mov  esi,0x5577dd
    int  0x40

    mov  eax,38                    ; line
    mov  ebx,5*65536+430
    mov  ecx,114*65536+114
    mov  edx,0x000000
    int  0x40

    mov  ebx,5*65536+133          ; info text
    mov  ecx,0x000000
    mov  edx,text
    mov  esi,70
  newline:
    mov  eax,4
    int  0x40
    add  ebx,12
    add  edx,70
    cmp  [edx],byte 'x'
    jne  newline

    mov  edx,I_END                ; text from server
    call draw_server_data

    mov  eax,12
    mov  ebx,2
    int  0x40

    popa

    ret


draw_server_data:

    pusha

    mov   eax,4
    mov   ebx,10*65536+26
    mov   ecx,8
    mov   esi,[rxs]
  dct:
    pusha
    mov   ecx,ebx
    shl   ecx,16
    mov   cl,9
    mov   eax,13
    mov   ebx,10*65536
    mov   bx,word [rxs]
    imul  bx,6
    mov   edx,0xffffff
    int   0x40
    popa
    push  ecx
    mov   eax,4
    mov   ecx,0
    int   0x40
    add   edx,[rxs]
    add   ebx,10
    pop   ecx
    loop  dct

    popa
    ret



text:

db '   Incoming mails are written to /rd/1/popc.txt                       '
db '                                                                      '
db '        Check for mail.                               Force close     '
db '                                                                      '
ip_text:
db '   Server IP : 192.168.1.200            <                             '
l2_text:
db '   User      :                          <         Open popc.txt       '
 l3_text:
db '   Password  : (not shown)              <                             '

db 'x' ; <- END MARKER, DONT DELETE

file_start      dd      0x100000

; max size is 0x100000 bytes, read to/write from 0x100000
files:
       dd  2,0,0,?,0x100000
       db  0
       dd  pr
filel:
       dd  0,0,0,0x100000,0x100000
pr db  '/rd/1/popc.txt',0

ip     db 192,168,1,200

socket  dd  0x0

posx             dd  0x0
incoming_pos     dd  0x0
incoming_string: times 128 db 0
pos          dd  0x0

text_start   dd  I_END
print        db  0x0
cmd          dd  0x0
rxs          dd  66

res:         db  0,0
command:     times  256  db 0x0

command_position  dd 0
counter           dd 0

numtext      db  '                     '
labelt       db  'POP client v ',version,0
scroll:      dd 1,8

tinypad_start:
        dd      7
        dd      0
        dd      pr
        dd      0
        dd      0
        db      '/RD/1/TINYPAD',0

getmail:
       db  'user xyz                      ',13,10
       db  'pass xyz                      ',13,10
       db  'retr 1                        ',13,10
       db  'retr 2                        ',13,10
       db  'retr 3                        ',13,10
       db  'retr 4                        ',13,10
       db  'retr 5                        ',13,10
       db  'retr 6                        ',13,10
       db  'retr 7                        ',13,10
       db  'retr 8                        ',13,10
       db  'retr 9                        ',13,10

quitc:
       db  'quit        ',13,10

mcounter dd 900
ccounter dd 0

ld   db 13,10

server_active db 0

header_sent db 0

close_connection   dd 0x0

mail_rp      dd  0

irc_data   dd  0x0
addr       dd  0x0
ya         dd  0x0
len        dd  0x0

input1:  times 50 db 32
input2:  times 50 db 32
input3:  times 50 db 32

l1 dd 0
l2 dd 3
l3 dd 3

status  dd  0x0

I_END:
