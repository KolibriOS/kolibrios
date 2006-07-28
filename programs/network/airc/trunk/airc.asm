;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                   ;;
;;    IRC CLIENT for MenuetOS                        ;;
;;                                                   ;;
;;    License: GPL / See file COPYING for details    ;;
;;    Copyright 2004 (c) Ville Turjanmaa             ;;
;;                                                   ;;
;;    Compile with FASM for Menuet                   ;;
;;                                                   ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

version equ '0.3'

use32

                org     0x0

                db      'MENUET01'              ; 8 byte id
                dd      0x01                    ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x100000                ; required amount of memory
                dd      0x100000
                dd      0,0
include "lang.inc"
include "macros.inc"

irc_server_ip   db      192,168,1,1

user_nick       dd      4                                 ; length
                db      'airc                   '         ; string

user_real_name  dd      8                                 ; length
                db      'Joe User               '         ; string


START:                          ; start of execution

    mov  eax,40
    mov  ebx,10000111b
    int  0x40

    mov  edi,I_END
    mov  ecx,60*120
    mov  eax,32
    cld
    rep  stosb

    mov  eax,[rxs]
    imul eax,11
    mov  [pos],eax

    mov  ebp,0
    mov  edx,I_END
    call draw_window            ; at first, draw the window

still:

    inc  [cursor_on_off]

    mov  eax,5
    mov  ebx,1
    int  0x40

    mov  eax,11                 ; wait here for event
    int  0x40

    call print_status

    cmp  eax,1                  ; redraw
    je   redraw
    cmp  eax,2                  ; key
    je   main_window_key
    cmp  eax,3                  ; button
    je   button

    cmp  [I_END+120*60],byte 1
    jne  no_main_update
    mov  [I_END+120*60],byte 0
    mov  edx,I_END
    call draw_channel_text
  no_main_update:

    call read_incoming_data

    call send_data_to_server

    test [cursor_on_off],0x3f
    jnz  nopri
    inc  [blink]
    call blink_cursor
    call print_channel_list
  nopri:

    jmp  still


redraw:                         ; redraw

    call draw_window
    jmp  still


button:                         ; button

    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; close program
    jne  noclose
    mov  eax,-1
    int  0x40
  noclose:

    call socket_commands

    jmp  still


print_status:

    pusha

    mov  eax,53
    mov  ebx,6
    mov  ecx,[socket]
    int  0x40

    mov  [status],eax

    cmp  [old_status],eax
    je   nopr

    mov  [old_status],eax

    push eax

    mov  eax,13
    mov  ebx,450*65536+30
    mov  ecx,231*65536+10
    mov  edx,0xffffff
    int  0x40

    pop  ecx
    mov  eax,47
    mov  ebx,2*65536
    mov  edx,450*65536+231
    mov  esi,0x000000
    int  0x40

  nopr:

    popa

    ret

status dd 0
old_status dd 0


socket_commands:

    cmp  ah,22       ; open socket
    jnz  tst3
    mov  eax,3
    int  0x40
    mov  ecx,eax
    mov  eax,53
    mov  ebx,5
    mov  edx,6667
    mov  esi,dword [irc_server_ip]
    mov  edi,1
    int  0x40
    mov  [socket], eax
    ret
  tst3:


    cmp  ah,23        ; write userinfo
    jnz  tst4

    mov  eax,53  ; user
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,string0l-string0
    mov  esi,string0
    int  0x40

    mov  eax,53  ;
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,[user_real_name]
    mov  esi,user_real_name+4
    int  0x40

    mov  eax,53  ;
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,2
    mov  esi,line_feed
    int  0x40


    mov  eax,5
    mov  ebx,10
    int  0x40

    mov  eax,53  ; nick
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,string1l-string1
    mov  esi,string1
    int  0x40

    mov  eax,53  ;
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,[user_nick]
    mov  esi,user_nick+4
    int  0x40

    mov  eax,53  ;
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,2
    mov  esi,line_feed
    int  0x40


    ret

  line_feed:  db  13,10

  tst4:


    cmp  ah,24     ; close socket
    jnz  no_24
    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    int  0x40
    ret
  no_24:


    ret


main_window_key:

    mov  eax,2
    int  0x40

    shr  eax,8

    cmp  eax,8
    jne  no_bks2
    cmp  [xpos],0
    je   still
    dec  [xpos]
    call print_entry
    jmp  still
   no_bks2:

    cmp  eax,20
    jbe  no_character2
    mov  ebx,[xpos]
    mov  [send_string+ebx],al
    inc  [xpos]
    cmp  [xpos],80
    jb   noxposdec
    mov  [xpos],79
  noxposdec:
    call print_entry
    jmp  still
  no_character2:

    cmp  eax,13
    jne  no_send
    cmp  [xpos],0
    je   no_send2
    cmp  [send_string],byte '/'   ; server command
    jne  no_send2
    mov  [send_to_server],1
    jmp  still
  no_send2:

    jmp  still


print_channel_list:

    pusha

    mov  eax,13
    mov  ebx,415*65536+6*13
    mov  ecx,27*65536+12*10
    mov  edx,0xffffff
    int  0x40

    mov  eax,4
    mov  ebx,415*65536+27
    mov  ecx,[index_list_1]
    mov  edx,channel_list+32
  newch:
    movzx esi,byte [edx+31]
    and  esi,0x1f
    int  0x40
    add  edx,32
    add  ebx,12
    cmp  edx,channel_list+32*10
    jbe  newch

  no_channel_list:

    popa

    ret


print_user_list:

    pusha

  newtry:

    mov  edx,ebp
    imul edx,120*80
    add  edx,120*60+8+I_END
    cmp  [edx],byte 1
    je   nonp

    mov  edx,ebp
    imul edx,120*80
    add  edx,120*70+I_END
    mov  edi,edx

    mov  eax,[edx-8]
    mov  ebx,[edx-4]
    add  ebx,edx
    sub  ebx,3
    inc  eax
    dec  edx
  newnss:
    inc  edx
    dec  eax
    jz   startuu
  asdf:
    cmp  [edx],word '  '
    jne  nodouble
    inc  edx
  nodouble:
    cmp  [edx],byte ' '
    je   newnss
    inc  edx
    cmp  edx,ebx
    jbe  asdf
    dec  dword [edi-8]

    popa
    ret

  startuu:

    cmp  [edx],byte ' '
    jne  startpr
    inc  edx
  startpr:

    pusha
    mov  eax,13
    mov  ebx,415*65536+6*13
    mov  ecx,27*65536+12*10
    mov  edx,0xffffff
    int  0x40
    popa

    mov  eax,4
    mov  ebx,415*65536+27

    mov  ebp,0
  newuser:

    mov  esi,0
  newusers:
    cmp  [edx+esi],byte ' '
    je   do_print
    inc  esi
    cmp  esi,20
    jbe  newusers
  do_print:

    mov  ecx,[index_list_1]
    cmp  [edx],byte '@'
    jne  no_op
    mov  ecx,[index_list_2]
  no_op:

    int  0x40

    inc  ebp
    cmp  ebp,10
    je   nonp

    add  ebx,12

    add  edx,esi

    inc  edx
    cmp  [edx],byte ' '
    jne  newuser
    inc  edx
    jmp  newuser

  nonp:

    popa

    ret


start_user_list_at dd 0x0




send_data_to_server:

    pusha

    cmp  [send_to_server],1
    jne  sdts_ret

    mov  eax,[xpos]
    mov  [send_string+eax+0],byte 13
    mov  [send_string+eax+1],byte 10

    mov  eax,[rxs]
    imul eax,11
    mov  [pos],eax
    mov  eax,[send_to_channel]
    imul eax,120*80
    add  eax,I_END
    mov  [text_start],eax

    cmp  [send_string],byte '/'   ; server command
    je   server_command

    mov  bl,13
    call print_character
    mov  bl,10
    call print_character
    mov  bl,'<'
    call print_character

    mov  esi,user_nick+4
    mov  ecx,[user_nick]
  newnp:
    mov  bl,[esi]
    call print_character
    inc  esi
    loop newnp

    mov  bl,'>'
    call print_character
    mov  bl,' '
    call print_character

    mov  ecx,[xpos]
    mov  esi,send_string
  newcw:
    mov  bl,[esi]
    call print_character
    inc  esi
    loop newcw

    mov  eax,dword [send_to_channel]
    shl  eax,5
    add  eax,channel_list
    mov  esi,eax

    mov  edi,send_string_header+8
    movzx ecx,byte [eax+31]
    cld
    rep  movsb

    mov  [edi],word ' :'

    mov   esi, send_string_header
    mov   edx,10
    movzx ebx,byte [eax+31]
    add   edx,ebx

    mov  eax, 53      ; write channel
    mov  ebx, 7
    mov  ecx, [socket]
    int  0x40

    mov  esi,send_string
    mov  edx,[xpos]
    inc  edx

    mov  eax, 53      ; write message
    mov  ebx, 7
    mov  ecx, [socket]
    int  0x40

    jmp  send_done

  server_command:

    cmp  [send_string+1],dword 'anic'
    jne  no_set_nick

    mov  ecx,[xpos]
    sub  ecx,7
    mov  [user_nick],ecx

    mov  esi,send_string+7
    mov  edi,user_nick+4
    cld
    rep  movsb

    pusha
    mov  edi,text+70*1+15
    mov  eax,32
    mov  ecx,15
    cld
    rep  stosb
    popa

    mov  esi,user_nick+4
    mov  edi,text+70*1+15
    mov  ecx,[user_nick]
    cld
    rep  movsb

    call draw_window

    mov  [xpos],0
    mov  [send_to_server],0

    popa
    ret

  no_set_nick:

    cmp  [send_string+1],dword 'area'
    jne  no_set_real_name

    mov  ecx,[xpos]
    sub  ecx,7
    mov  [user_real_name],ecx

    mov  esi,send_string+7
    mov  edi,user_real_name+4
    cld
    rep  movsb

    pusha
    mov  edi,text+70*0+15
    mov  eax,32
    mov  ecx,15
    cld
    rep  stosb
    popa

    mov  esi,user_real_name+4
    mov  edi,text+70*0+15
    mov  ecx,[xpos]
    sub  ecx,7
    cld
    rep  movsb

    call draw_window

    mov  [xpos],0
    mov  [send_to_server],0

    popa
    ret

  no_set_real_name:

    cmp  [send_string+1],dword 'aser'
    jne  no_set_server

    pusha
    mov   edi,irc_server_ip
    mov   esi,send_string+7
    mov   eax,0
    mov   edx,[xpos]
    add   edx,send_string-1
  newsip:
    cmp   [esi],byte '.'
    je    sipn
    cmp   esi,edx
    jg    sipn
    movzx ebx,byte [esi]
    inc   esi
    imul  eax,10
    sub   ebx,48
    add   eax,ebx
    jmp   newsip
  sipn:
    mov   [edi],al
    xor   eax,eax
    inc   esi
    cmp   esi,send_string+30
    jg    sipnn
    inc   edi
    cmp   edi,irc_server_ip+3
    jbe   newsip
  sipnn:
    popa

    mov  ecx,[xpos]
    sub  ecx,7

    pusha
    mov  edi,text+70*2+15
    mov  eax,32
    mov  ecx,15
    cld
    rep  stosb
    popa

    mov  esi,send_string+7
    mov  edi,text+70*2+15
    cld
    rep  movsb

    call draw_window

    mov  [xpos],0
    mov  [send_to_server],0

    popa
    ret

   no_set_server:




    cmp  [send_string+1],dword 'quer'
    jne  no_query_create

    mov  edi,I_END+120*80
    mov  eax,1 ; create channel window - search for empty slot
   newse2:
    mov  ebx,eax
    shl  ebx,5
    cmp  dword [channel_list+ebx],dword '    '
    je   free_found2
    add  edi,120*80
    inc  eax
    cmp  eax,[max_windows]
    jb   newse2

  free_found2:

    mov  edx,send_string+7

    mov  ecx,[xpos]
    sub  ecx,7
    mov  [channel_list+ebx+31],cl

    call create_channel_name

    push edi
    push eax
    mov  [edi+120*60+8],byte 1 ; query window
    mov  eax,32
    mov  ecx,120*60
    cld
    rep  stosb
    pop  eax
    pop  edi

    ; eax has the free position
    mov  [thread_screen],edi
    call create_channel_window

    mov  [xpos],0
    mov  [send_to_server],0

    popa
    ret

  no_query_create:


    mov  esi, send_string+1
    mov  edx, [xpos]
    add  edx,1

    mov  eax, 53      ; write server command
    mov  ebx, 7
    mov  ecx, [socket]
    int  0x40

  send_done:

    mov  [xpos],0
    mov  [send_to_server],0

    cmp  [send_string+1],dword 'quit'
    jne  no_quit_server
    mov  eax,5
    mov  ebx,200
    int  0x40

    mov  eax, 53      ; close socket
    mov  ebx, 8
    mov  ecx, [socket]
    int  0x40

    mov  ecx,[max_windows]
    mov  edi,I_END
  newclose:
    mov  [edi+120*60+4],byte  1
    add  edi,120*80
    loop newclose

    popa
    ret

  no_quit_server:

  sdts_ret:

    popa
    ret



read_incoming_data:

    pusha

  read_new_byte:

    call read_incoming_byte
    cmp  ecx,-1
    je   no_data_in_buffer

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
    cmp  eax,512
    jge  still

    mov  [eax+command-2],bl
    inc  [cmd]

    jmp  read_new_byte

  no_data_in_buffer:

    popa

    ret


create_channel_name:

    pusha

  search_first_letter:
    cmp  [edx],byte ' '
    jne  first_letter_found
    inc  edx
    jmp  search_first_letter
  first_letter_found:

    mov  esi,edx
    mov  edi,channel_list
    add  edi,ebx
    mov  ecx,30
    xor  eax,eax
  newcase:
    mov  al,[esi]
    cmp  eax,'a'
    jb   nocdec
    cmp  eax,'z'
    jg   nocdec
    sub  al,97-65
  nocdec:
    mov  [edi],al
    inc  esi
    inc  edi
    loop newcase

    popa

    ret


create_channel_window:

    pusha

    mov  [cursor_on_off],0

    mov  [thread_nro],eax

    mov  eax,51
    mov  ebx,1
    mov  ecx,channel_thread
    mov  edx,[thread_stack]
    int  0x40

    mov  eax,5
    mov  ebx,10
    int  0x40

    add  [thread_stack],0x4000
    add  [thread_screen],120*80

    popa

    ret


print_entry:

    pusha

    mov  eax,13
    mov  ebx,8*65536+6*80
    mov  ecx,151*65536+13
    mov  edx,0xffffff
    int  0x40

    mov  eax,4
    mov  ebx,8*65536+154
    mov  ecx,0x000000
    mov  edx,send_string
    mov  esi,[xpos]
    int  0x40

    popa

    ret

blink dd 0x0

blink_cursor:

    pusha

    mov  eax,9
    mov  ebx,0xe0000
    mov  ecx,-1
    int  0x40

    mov  edx,[blink]
    and  edx,1
    sub  edx,1
    and  edx,0xffffff
;    mov  edx,0

    cmp  ax,word [0xe0000+4]
    jne  no_blink

    call print_entry

    mov  ebx,[xpos]
    imul ebx,6
    add  ebx,8
    mov  cx,bx
    shl  ebx,16
    mov  bx,cx
    mov  ecx,151*65536+163
    mov  eax,38
    int  0x40

    popa

    ret

  no_blink:

    mov  eax,13
    mov  ebx,8*65536+6*60
    mov  ecx,151*65536+13
    mov  edx,0xffffff
    int  0x40

    popa

    ret





set_channel:

    pusha

    ; UPPER / LOWER CASE CHECK

    mov  esi,eax
    mov  edi,channel_temp
    mov  ecx,40
    xor  eax,eax
  newcase2:
    mov  al,[esi]
    cmp  eax,'#'
    jb   newcase_over2
    cmp  eax,'a'
    jb   nocdec2
    cmp  eax,'z'
    jg   nocdec2
    sub  al,97-65
  nocdec2:
    mov  [edi],al
    inc  esi
    inc  edi
    loop newcase2
  newcase_over2:
    sub  edi,channel_temp
    mov  [channel_temp_length],edi

    mov  eax,channel_temp

    mov  [text_start],I_END+120*80
    mov  ebx,channel_list+32
    mov  eax,[eax]

    mov  edx,[channel_temp_length]

  stcl1:
    cmp  dl,[ebx+31]
    jne  notfound

    pusha
    xor  eax,eax
    xor  edx,edx
    mov  ecx,0
  stc4:
    mov  dl,[ebx+ecx]
    mov  al,[channel_temp+ecx]
    cmp  eax,edx
    jne  notfound2
    inc  ecx
    cmp  ecx,[channel_temp_length]
    jb   stc4
    popa

    jmp  found

  notfound2:
    popa

  notfound:
    add  [text_start],120*80
    add  ebx,32
    cmp  ebx,channel_list+19*32
    jb   stcl1

    mov  [text_start],I_END

  found:

    popa

    ret


channel_temp:         times   100   db   0
channel_temp_length   dd      0x0



print_nick:

    pusha

    mov  eax,command+1
    mov  dl,'!'
    call print_text

    popa
    ret


analyze_command:

    pusha

    mov  [text_start],I_END
    mov  ecx,[rxs]
    imul ecx,11
    mov  [pos],ecx

    mov  bl,13
;  call print_character
    mov  bl,10
;  call print_character

    mov  ecx,[cmd]
    sub  ecx,2
    mov  esi,command+0
  newcmdc:
    mov  bl,[esi]
;  call print_character
    inc  esi
    loop newcmdc

    mov   edx,I_END
;  call  draw_channel_text

    cmp  [cmd],20
    jge  cmd_len_ok

    mov  [cmd],0

    popa
    ret


  cmd_len_ok:

    cmp  [command],dword 'PING'  ; ping response
    jne  no_ping_responce

    call print_command_to_main

    mov  [command],dword 'PONG'

    call print_command_to_main

    mov  eax,4
    mov  ebx,100*65536+3
    mov  ecx,0xffffff
    mov  edx,command
    mov  esi,[cmd]
    mov  [command+esi-1],word '**'
;    int  0x40

    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,[cmd]
    sub  edx,2
    and  edx,255
    mov  esi,command
    int  0x40

    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,2
    mov  esi,linef
    int  0x40

    popa
    ret

  linef  db  13,10

  no_ping_responce:

    mov  eax,[rxs]
    imul eax,11
    mov  [pos],eax

    mov  [command],byte '<'

    mov  eax,command
    mov  ecx,100
   new_blank:
    cmp  [eax],byte ' '
    je   bl_found
    inc  eax
    loop new_blank
    mov  eax,50
  bl_found:

    inc  eax
    mov  [command_position],eax

    mov  esi,eax
    mov  edi,irc_command
    mov  ecx,8
    cld
    rep  movsb


    cmp  [irc_command],'PRIV'  ; message to channel
    jne  no_privmsg

    ; compare nick

    mov  eax,[command_position]
    add  eax,8
    call compare_to_nick
    cmp  [cresult],0
    jne  no_query_msg
    mov  eax,command+1
  no_query_msg:
    call set_channel

    mov  ecx,100 ; [cmd]
    mov  eax,command+10
  acl3:
    cmp  [eax],byte ':'
    je   acl4
    inc  eax
    loop acl3
    mov  eax,10
  acl4:
    inc  eax

    cmp  [eax+1],dword 'ACTI'
    jne  no_action
    push eax
    mov  eax,action_header_short
    mov  dl,0
    call print_text
    mov  eax,command+1
    mov  dl,'!'
    call print_text
    mov  bl,' '
    call print_character
    pop  eax
    add  eax,8
    mov  dl,0
    call print_text
    popa
    ret

  no_action:

    push eax
    mov  bl,10
    call print_character
    mov  eax,command
    mov  dl,'!'
    call print_text
    mov  bl,'>'
    call print_character
    mov  bl,' '
    call print_character
    pop  eax

    mov  dl,0
    call print_text

    popa
    ret

  no_privmsg:


    cmp  [irc_command],'PART'    ; channel leave
    jne  no_part

    ; compare nick

    mov  eax,command+1
    call compare_to_nick
    cmp  [cresult],0
    jne  no_close_window

    mov  eax,[command_position]
    add  eax,5
    call set_channel

    mov  eax,[text_start]
    mov  [eax+120*60+4],byte 1

    popa
    ret

  no_close_window:

    mov  eax,[command_position]
    add  eax,5
    call set_channel

    mov  eax,action_header_red
    mov  dl,0
    call print_text
    mov  eax,command+1
    mov  dl,'!'
    mov  cl,' '
    call print_text
    mov  eax,has_left_channel
    mov  dl,0
    call print_text
    mov  eax,[command_position]
    add  eax,5
    mov  dl,' '
    call print_text

    popa
    ret

  no_part:


    cmp  [irc_command],'JOIN'    ; channel join
    jne  no_join

    ; compare nick

    mov  eax,command+1
    call compare_to_nick
    cmp  [cresult],0
    jne  no_new_window

    mov  edi,I_END+120*80
    mov  eax,1 ; create channel window - search for empty slot
   newse:
    mov  ebx,eax
    shl  ebx,5
    cmp  dword [channel_list+ebx],dword '    '
    je   free_found
    add  edi,120*80
    inc  eax
    cmp  eax,[max_windows]
    jb   newse

  free_found:

    mov  edx,[command_position]
    add  edx,6

    push eax
    push edx
    mov  ecx,0
   finde:
    inc  ecx
    inc  edx
    movzx eax,byte [edx]
    cmp  eax,'#'
    jge  finde
    mov  [channel_list+ebx+31],cl
    pop  edx
    pop  eax

    call create_channel_name

    push edi
    push eax
    mov  [edi+120*60+8],byte 0 ; channel window
    mov  eax,32
    mov  ecx,120*60
    cld
    rep  stosb
    pop  eax
    pop  edi

    ; eax has the free position
    mov  [thread_screen],edi
    call create_channel_window

  no_new_window:

    mov  eax,[command_position]
    add  eax,6
    call set_channel

    mov  eax,action_header_blue
    mov  dl,0
    call print_text
    mov  eax,command+1
    mov  dl,'!'
    mov  cl,' '
    call print_text

    mov  eax,joins_channel
    mov  dl,0
    call print_text

    mov  eax,[command_position]
    add  eax,6
    mov  dl,0
    call print_text

    popa
    ret

  no_join:


    cmp  [irc_command],'NICK'      ; nick change
    jne  no_nick_change

    mov  [text_start],I_END
    add  [text_start],120*80

 new_all_channels3:

    mov  eax,action_header_short
    mov  dl,0
    call print_text
    mov  eax,command+1
    mov  dl,'!'
    call print_text
    mov  eax,is_now_known_as
    mov  dl,0
    call print_text
    mov  eax,[command_position]
    add  eax,6
    mov  dl,0
    call print_text

    add  [text_start],120*80
    cmp  [text_start],I_END+120*80*20
    jb   new_all_channels3

    popa
    ret

  no_nick_change:


     cmp  [irc_command],'KICK'      ; kick
     jne  no_kick

    mov  [text_start],I_END
    add  [text_start],120*80

    mov  eax,[command_position]
    add  eax,5
    call set_channel

; new_all_channels4:

    mov  eax,action_header_short
    mov  dl,0
    call print_text
    mov  eax,command+1
    mov  dl,'!'
    call print_text
     mov  eax,kicked
     mov  dl,0
    call print_text
    mov  eax,[command_position]
    add  eax,5
    mov  dl,0
    call print_text

;    add  [text_start],120*80
;    cmp  [text_start],I_END+120*80*20
;    jb   new_all_channels4

    popa
    ret

  no_kick:




    cmp  [irc_command],'QUIT'    ; irc quit
    jne  no_quit

    mov  [text_start],I_END
    add  [text_start],120*80

 new_all_channels2:

    mov  eax,action_header_red
    mov  dl,0
    call print_text
    mov  eax,command+1
    mov  dl,'!'
    call print_text
    mov  eax,has_quit_irc
    mov  dl,0
    call print_text

    add  [text_start],120*80
    cmp  [text_start],I_END+120*80*20
    jb   new_all_channels2

    popa
    ret

  no_quit:


    cmp  [irc_command],dword 'MODE'  ; channel mode change
    jne  no_mode

    mov  [text_start],I_END
    add  [text_start],120*80

    mov  eax,[command_position]
    add  eax,5
    call set_channel

 new_all_channels:

    mov  eax,action_header_short
    mov  dl,0
    call print_text

    call print_nick

    mov  eax,sets_mode
    mov  dl,0
    call print_text

    mov  eax,[command_position]
    add  eax,5
    mov  dl,0
    call print_text

;    add  [text_start],120*80
;    cmp  [text_start],I_END+120*80*20
;    jb   new_all_channels

    popa
    ret

  no_mode:


    cmp  [irc_command],dword '353 '  ; channel user names
    jne  no_user_list

    mov  eax,[command_position]
   finde2:
    inc  eax
    cmp  [eax],byte '#'
    jne  finde2
    call set_channel

   finde3:
    inc  eax
    cmp  [eax],byte ':'
    jne  finde3

    pusha
    cmp  [user_list_pos],0
    jne  no_clear_user_list
    mov  edi,[text_start]
    add  edi,120*70
    mov  [edi-8],dword 0
    mov  [edi-4],dword 0
    mov  eax,32
    mov  ecx,1200
    cld
    rep  stosb
  no_clear_user_list:
    popa

    push eax

    mov  esi,eax
    inc  esi
    mov  edi,[text_start]
    add  edi,120*70
    add  edi,[user_list_pos]
    mov  edx,edi
    mov  ecx,command
    add  ecx,[cmd]
    sub  ecx,[esp]
    sub  ecx,3
    and  ecx,0xfff
    cld
    rep  movsb

    pop  eax
    mov  ebx,command
    add  ebx,[cmd]
    sub  ebx,eax
    sub  ebx,2
    mov  [edx+ebx-1],dword '    '

    add  [user_list_pos],ebx

    mov  eax,[user_list_pos]
    mov  ebx,[text_start]
    add  ebx,120*70
    mov  [ebx-4],eax

    popa
    ret

  user_list_pos dd 0x0

  no_user_list:


    cmp  [irc_command],dword '366 '  ; channel user names end
    jne  no_user_list_end

    mov  [user_list_pos],0

    popa
    ret

  no_user_list_end:

    mov  [command],byte '-'
    call print_command_to_main

    popa

    ret


cresult db 0

compare_to_nick:

; input  : eax = start of compare
; output : [cresult] = 0 if match, [cresult]=1 if no match


    pusha

    mov  esi,eax
    mov  edi,0

  new_nick_compare:

    mov  bl,byte [esi]
    mov  cl,byte [user_nick+4+edi]

    cmp  bl,cl
    jne  nonickm

    add  esi,1
    add  edi,1

    cmp  edi,[user_nick]
    jb   new_nick_compare

    movzx eax,byte [esi]
    cmp  eax,40
    jge  nonickm

    popa
    mov  [cresult],0
    ret

  nonickm:

    popa
    mov  [cresult],1
    ret





print_command_to_main:

    pusha

    mov  [text_start],I_END
    mov  ecx,[rxs]
    imul ecx,11
    mov  [pos],ecx

    mov  bl,13
    call print_character
    mov  bl,10
    call print_character

    mov  ecx,[cmd]
    sub  ecx,2
    mov  esi,command
   newcmdc2:
    mov  bl,[esi]
    call print_character
    inc  esi
    loop newcmdc2

    mov   edx,I_END
    call  draw_channel_text

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



draw_data:

    pusha

    and  ebx,0xff

    cmp  bl,0xe4   ; finnish a
    jne  noe4
    mov  bl,0xc1
  noe4:
    cmp  bl,0xc4   ; ?
    jne  noc4
    mov  bl,0xc9
  noc4:

    cmp  ebx,229   ; swedish a
    jne  no_swedish_a
    mov  bl,192
  no_swedish_a:

    add  eax,[text_start]
    mov  [eax],bl

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



draw_window:

    pusha

    mov  eax,12
    mov  ebx,1
    int  0x40

    mov  [old_status],300

    mov  eax,0                     ; draw window
    mov  ebx,5*65536+499
    mov  ecx,5*65536+345
    mov  edx,[wcolor]
    add  edx,0x03ffffff
    mov  esi,0x80555599
    mov  edi,0x00ffffff
    int  0x40

    mov  eax,4                     ; label
    mov  ebx,9*65536+8
    mov  ecx,0x10ffffff
    mov  edx,labelt
    mov  esi,labellen-labelt
    int  0x40

    mov  eax,8                     ; button: open socket
    mov  ebx,43*65536+22
    mov  ecx,229*65536+10
    mov  edx,22
    mov  esi,[main_button]
    int  0x40

    mov  eax,8                     ; button: send userinfo
    mov  ebx,180*65536+22
    mov  ecx,229*65536+10
    mov  edx,23
    int  0x40

    mov  eax,8                     ; button: close socket
    mov  ebx,317*65536+22
    mov  ecx,229*65536+10
    mov  edx,24
    int  0x40

    mov  eax,38                    ; line
    mov  ebx,5*65536+494
    mov  ecx,148*65536+148
    mov  edx,[main_line]
    int  0x40
    add  ecx,1*65536+1
;    mov  edx,0x5555cc
;    int  0x40

    mov  eax,38                    ; line
    mov  ebx,5*65536+494
    mov  ecx,166*65536+166
    int  0x40
    add  ecx,1*65536+1
;    mov  edx,0x5555cc
;    int  0x40

    mov  eax,38                    ; line
    mov  ebx,410*65536+410
    mov  ecx,22*65536+148
    int  0x40
    add  ebx,1*65536+1
;    mov  edx,0x5555cc
;    int  0x40

    mov  ebx,25*65536+183          ; info text
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
    call draw_channel_text

    mov  eax,12
    mov  ebx,2
    int  0x40

    popa

    ret

main_line    dd 0x000000
main_button  dd 0x6565cc


text:

db '   Real name : Joe User        - change with eg /areal Jill User      '
db '   Nick      : AIRC            - change with eg /anick Jill           '
db '   Server    : 192.168.1.1     - change with eg /aserv 192.168.1.24   '
db '                                                                      '
db '        1) Open socket         2) Send userinfo       Close socket    '
db '                                                                      '
db '   Commands after established connection:                             '
db '                                                                      '
db '   /join #ChannelName         - eg /join #menuet                      '
db '   /part #ChannelName         - eg /part #linux                       '
db '   /query Nickname            - eg /query Mary                        '
db '   /quit                      - Quit server and Close socket          '

db 'x <- END MARKER, DONT DELETE            '




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;                        CHANNEL THREADS
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



channel_thread:

    mov   ebp,[thread_nro]
    mov   eax,ebp
    shl   eax,14
    add   eax,0x80000
    mov   esp,eax

    mov   edi,ebp        ; clear thread memory
    imul  edi,120*80
    add   edi,I_END
    mov   ecx,120*80
    mov   eax,32
    cld
;    rep   stosb

    mov   edx,[thread_screen]

    call  thread_draw_window

  w_t:

    mov  esi,ebp
    imul esi,120*80
    add  esi,I_END
    cmp  [esi+120*60+4],byte 1
    jne  no_channel_leave
    mov  [esi+120*60+4],byte 0
    mov  edi,ebp
    shl  edi,5
    mov  dword [channel_list+edi],dword '    '
    mov  byte  [channel_list+edi+31],byte 1
    mov  eax,-1
    int  0x40
  no_channel_leave:

    call  check_mouse

    mov   eax,23
    mov   ebx,1
    int   0x40

    cmp   eax,1
    jne   no_draw_window
    call  thread_draw_window
    call  draw_channel_text
    call  print_user_list
  no_draw_window:

    cmp   eax,2
    je    thread_key

    cmp   eax,3
    jne   no_end
    mov   eax,17
    int   0x40
    mov   eax,ebp
    imul  eax,120*80
    add   eax,I_END
    cmp   [eax+120*60+8],byte 0 ; channel window
    je    not_close
    mov   eax,ebp
    shl   eax,5
    add   eax,channel_list
    mov   [eax],dword '    '
    mov   [eax+31],byte 1
    mov   eax,-1
    int   0x40
  not_close:
    mov   [text_start],eax
    mov   eax,nocl
  newcc:
    mov   bl,[eax]
    call  print_character
    inc   eax
    cmp   [eax],byte 0
    jne   newcc
    call  draw_channel_text
    jmp   w_t
   nocl:   db  13,10,'To exit channel, use PART or QUIT command.',0
   no_end:

    cmp   [edx+120*60],byte 1
    jne   no_update
    mov   [edx+120*60],byte 0
    call  draw_channel_text
  no_update:

    test [cursor_on_off],0x3f
    jnz   nopri2

    call  blink_cursor
    call  print_user_list

  nopri2:

    jmp   w_t



check_mouse:

    pusha

    mov  eax,37
    mov  ebx,1
    int  0x40

    mov  ebx,eax
    shr  eax,16
    and  ebx,0xffff

    cmp  eax,420
    jb   no_mouse
    cmp  eax,494
    jg   no_mouse

    cmp  ebx,145
    jg   no_mouse
    cmp  ebx,23
    jb   no_mouse


    cmp  ebx,100
    jb   no_plus
    mov  eax,ebp
    imul eax,120*80
    add  eax,120*70+I_END
    inc  dword [eax-8]
    call print_user_list
    mov  eax,5
    mov  ebx,8
    int  0x40
    jmp  no_mouse
  no_plus:

    cmp  ebx,80
    jg   no_mouse
    mov  eax,ebp
    imul eax,120*80
    add  eax,120*70+I_END
    cmp  dword [eax-8],dword 0
    je   no_mouse
    dec  dword [eax-8]
    call print_user_list
    mov  eax,5
    mov  ebx,8
    int  0x40

  no_minus:

  no_mouse:

    popa

    ret




thread_key:

    mov  eax,2
    int  0x40

    shr  eax,8

    cmp  eax,8
    jne  no_bks
    cmp  [xpos],0
    je   w_t
    dec  [xpos]
    call print_entry
    jmp  w_t
   no_bks:

    cmp  eax,20
    jbe  no_character
    mov  ebx,[xpos]
    mov  [send_string+ebx],al
    inc  [xpos]
    cmp  [xpos],80
    jb   xpok
    mov  [xpos],79
  xpok:
    call print_entry
    jmp  w_t
  no_character:

    cmp  eax,13
    jne  no_send
    cmp  [xpos],0
    je   no_send
    mov  dword [send_to_channel],ebp
    mov  [send_to_server],1
  wait_for_sending:
    mov  eax,5
    mov  ebx,1
    int  0x40
    cmp  [send_to_server],1
    je   wait_for_sending
    call draw_channel_text
    call print_entry
    jmp  w_t
  no_send:

    jmp  w_t






draw_channel_text:

    pusha

    mov   eax,4
    mov   ebx,10*65536+26
    mov   ecx,12
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
    int   0x40
    popa
    push  ecx
    mov   eax,4
    mov   ecx,0
    cmp   [edx],word '* '
    jne   no_red
    mov   ecx,0x0000ff
   no_red:
    cmp   [edx],word '**'
    jne   no_light_blue
    cmp   [edx+2],byte '*'
    jne   no_light_blue
    mov   ecx,0x0000ff
  no_light_blue:
    cmp   [edx],byte '#'
    jne   no_blue
    mov   ecx,0x0000ff
  no_blue:
    int   0x40
    add   edx,[rxs]
    add   ebx,10
    pop   ecx
    loop  dct

    popa
    ret





thread_draw_window:

    pusha

    mov  eax,12
    mov  ebx,1
    int  0x40

    mov  ebx,ebp                   ; draw window
    shl  ebx,16+4
    mov  eax,0
    mov  ecx,ebx
    mov  bx,499
    mov  cx,170

;    mov  edx,ebp                   ; draw window
;    imul edx,120*80
;    add  edx,I_END+120*60+8
;    movzx edx,byte [edx]
;    imul edx,88
;    sub  bx,dx

    mov  edx,[wcolor]
    add  edx,0x03ffffff
    mov  esi,0x80555599
    mov  edi,0x00ffffff

    int  0x40

    mov  eax,ebp                   ; label
    add  eax,48
    mov  [labelc+14],al
    mov  eax,ebp
    shl  eax,5
    add  eax,channel_list
    mov  esi,eax
    mov  edi,labelc+17
    movzx ecx,byte [eax+31]
    cld
    rep   movsb

    mov  esi,17                    ; print label
    movzx ebx,byte [eax+31]
    add  esi,ebx
    mov  eax,4
    mov  ebx,9*65536+8
    mov  ecx,0x00ffffff
    mov  edx,labelc
    int  0x40

    mov  eax,38                    ; line
    mov  ebx,5*65536+494
    mov  ecx,148*65536+148
    mov  edx,[channel_line_sun]
    int  0x40
    add  ecx,1*65536+1
    mov  edx,[channel_line_shadow]
    int  0x40


    mov  eax,38                    ; line
    mov  ebx,410*65536+410
    mov  ecx,22*65536+148
    mov  edx,[channel_line_sun]
    int  0x40
    add  ebx,1*65536+1
    mov  edx,[channel_line_shadow]
    int  0x40

    mov  eax,12
    mov  ebx,2
    int  0x40

    popa

    ret



; DATA AREA

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

channel_line_sun    dd 0x9999ff
channel_line_shadow dd 0x666699

cursor_on_off  dd  0x0

max_windows    dd  20

thread_stack   dd  0x9fff0
thread_nro     dd 1
thread_screen  dd I_END+120*80*1

action_header_blue  db  10,'*** ',0
action_header_red   db  10,'*** ',0

action_header_short db  10,'* ',0

has_left_channel db  ' left channel ',0
joins_channel    db  ' joined channel ',0
is_now_known_as  db  ' is now known as ',0
has_quit_irc     db  ' has quit irc',0
sets_mode        db  ' sets mode ',0
kicked           db  ' kicked from ',0

index_list_1     dd  0x0000bb
index_list_2     dd  0x0000ff

posx             dd  0x0
incoming_pos     dd  0x0
incoming_string: times 128 db 0

pos          dd  0x0

text_start   dd  I_END
irc_data     dd  0x0
print        db  0x0
cmd          dd  0x0
rxs          dd  66

res:         db  0,0
command:     times  600  db 0x0

nick         dd  0,0,0
irc_command  dd  0,0

command_position  dd 0x0
counter           dd  0
send_to_server    db 0

channel_list:     times 32*20 db 32
send_to_channel   dd 0x0

send_string_header:  db     'privmsg #eax :'
                     times  100  db  0x0

send_string:         times  100  db  0x0
xpos         dd  0

string0:     db  'USER guest ser1 ser2 :'
string0l:
string1:     db  'nick '
string1l:

attribute   dd  0
scroll      dd  1
            dd  12

numtext     db  '                     '

wcolor      dd  0x000000

labelc      db  'AIRC - WINDOW X: #xxx                 '
labelt      db  'IRC client ',version
labellen:

;;
;;   Channel data at I_END
;;
;;   120*80 * channel window (1+)
;;
;;      At         Size
;;
;;      00      ,  120*60   window text 120 characters per row
;;  120*60      ,  1        text is updated
;;  120*60+4    ,  1        close yourself
;;  120*60+8    ,  1        0 = channel window  :  1 = private chat
;;  120*61      ,  256      channel name
;;  120*61+254  ,  254      channel entry text from user
;;  120*61+255  ,  1        length of entry text
;;  120*69+248  ,  4        display names from n:th name
;;  120*69+252  ,  4        length of names string
;;  120*70      ,  1200     names separated with space
;;
I_END: ;;
