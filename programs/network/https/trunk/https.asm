;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                      ;
;    Tiny HTTP Server v 0.5 for KolibriOS              ;
;                                                      ;
;    License GPL / See file COPYING for details.       ;
;    Copyright 2003 Ville Turjanmaa                    ;
;                                                      ;
;    Compile with FASM for Menuet/KolibriOS            ;
;                                                      ;
;    Request /TinyStat for server statistics           ;
;    Request /TinyBoard for server message board       ;
;                                                      ;
;    Special version for KoOS by Hex && Heavyiron      ;
;                                                      ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

appname  equ  'Kolibri HTTP Server '
version  equ  '0.6'

use32

                org     0x0

                db      'MENUET01'              ; 8 byte id
                dd      0x01                    ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x400000                ; required amount of memory
                dd      0x20000
                dd      0,0                     ; reserved=no extended header

include "macros.inc"

; 0x0+       - program image
; 0x1ffff    - stack
; 0x20000+   - message board
; 0x100000+  - requested file

filel:
   dd 0
   dd 0
   dd 0
   dd 50000
   dd 0x20000
   db   '/sys/board.htm',0

files:
    dd 2
    dd 0
    dd 0
    dd 0
    dd 0x20000
    db '/sys/board.htm',0


START:                          ; start of execution

    mov  eax,70
    mov  ebx,filel
    mcall
    mov  [board_size],ebx
    cmp  eax,0
    je   board_found

    mov  [board_size],board_end-board
    mov  esi,board
    mov  edi,0x20000
    mov  ecx,[board_size]
    cld
    rep  movsb

   board_found:

    mov  eax,70
    mov  ebx,files
    mov  ecx,[board_size]
    mov  [files+12],ecx
    mcall

    mov  [status],-1
    mov  [last_status],-2
    call clear_input
red:
    call draw_window            ; at first, draw the window

still:

    call check_status
    cmp  [status],4
    je   start_transmission

    cmp  [status],0
    jne  nnn
    cmp  [server_active],1
    jne  nnn
    call ops
   nnn:

    mov  eax,5
    mov  ebx,1
    mcall

    mov  eax,11
    mcall
    call check_events

    jmp  still

last_status   dd   0x0

check_events:

    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jz   key
    cmp  eax,3                  ; button in buffer ?
    jz   button

    ret

key:                           ; Keys are not valid at this part of the
    mov  al,2                  ; loop. Just read it and ignore
    mcall
    ret

button:                         ; button

    mov  al,17          ; get id
    mcall

    cmp  ah,1           ; close
    jnz  tst2
    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    mcall
    mov  eax,-1
    mcall
  tst2:

    cmp  ah,2            ; button id=2 ?
    jnz  tst3
    ; open socket
  ops:
    mov  eax,53
    mov  ebx,5
    mov  ecx,80       ; local port # - http
    mov  edx,0      ; no remote port specified
    mov  esi,0      ; no remote ip specified
    mov  edi,0      ; PASSIVE open
    mcall
    mov  [socket], eax
    mov  [posy],1
    mov  [posx],0
    call check_for_incoming_data
    call clear_input
    call draw_data
    mov  [server_active],1
    call check_status
    ret
  tst3:
    cmp  ah,4                   ; button id=4 ?
    jnz  no4
    mov  [server_active],0
  close_socket:
    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    mcall
    mov  eax,5
    mov  ebx,2
    mcall
    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    mcall

    cmp  [server_active],1
    jne  no_re_open
    mov  eax,53
    mov  ebx,5
    mov  ecx,80     ; local port # - http
    mov  edx,0      ; no remote port specified
    mov  esi,0      ; no remote ip specified
    mov  edi,0      ; PASSIVE open
    mcall
    mov  [socket], eax
  no_re_open:

    mov  edi,input_text+256*15+1
    mov  [edi+2],dword ':  :'
    call set_time
    mov  edi,input_text+256*16+1
    mov  [edi+2],dword '.  .'
    call set_date

    mov  eax,[documents_served]
    mov  ecx,9
    mov  edi,input_text+256*15+12
    call set_value

    mov  eax,[bytes_transferred]
    mov  ecx,9
    mov  edi,input_text+256*16+12
    call set_value

    call draw_data

    mov  esp,0x1ffff
    jmp  still
  no4:

    cmp  ah,6                   ; read directory
    je   read_string

    ret


clear_input:

    mov  edi,input_text
    mov  eax,0
    mov  ecx,256*30
    cld
    rep  stosb

    ret


retries  dd 50

start_transmission:

    mov  [posy],1
    mov  [posx],0
    call clear_input
    mov  [retries],50

  wait_for_data:
    call check_for_incoming_data
    cmp  [input_text+256+1],dword 'GET '
    je   data_received
    cmp  [input_text+256+1],dword 'POST'
    je   data_received
    mov  eax,5
    mov  ebx,1
    mcall
    dec  [retries]
    jnz  wait_for_data
    jmp  no_http_request
  data_received:

    mov  eax,0x100000
    mov  ebx,0x2f0000 / 512
    call read_file

    call wait_for_empty_slot
    call send_header

    mov  [filepos],0x100000
    mov  [fileadd],700

    call check_status
    call draw_data

  newblock:

    call wait_for_empty_slot

    mov  edx,[fileadd]
    cmp  edx,[file_left]
    jbe  file_size_ok
    mov  edx,[file_left]
  file_size_ok:
    sub  [file_left],edx

    ; write to socket
    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  esi,[filepos]
    mcall

    mov  eax,esi
    add  eax,edx
    sub  eax,0x100000
    call display_progress

    mov  edx,[fileadd]
    add  [filepos],edx

    cmp  [file_left],0
    jg   newblock

  no_http_request:

    jmp  close_socket


filepos   dd  0x100000
fileadd   dd  0x1
filesize  dd  0x0
file_left dd  0x0


wait_for_empty_slot:

    pusha

  wait_more:

    mov  eax,5
    mov  ebx,1
    mcall

    mov  eax,11
    mcall
    call check_events

    mov  eax,53
    mov  ebx,255
    mov  ecx,103
    mcall

    cmp  eax,0
    je   no_wait_more

    jmp  wait_more

  no_wait_more:

    popa
    ret




display_progress:

  pusha

  mov  edi,eax

  mov  eax,13
  mov  ebx,115*65536+8*6
  mov  ecx,178*65536+10
  mov  edx,0xffffff
  mcall

  mov  eax,47
  mov  ebx,8*65536
  mov  ecx,edi
  mov  edx,115*65536+178
  mov  esi,0x000000
  mcall

  popa
  ret


send_header:

    pusha

    mov   eax,53                  ; send response and file length
    mov   ebx,7
    mov   ecx,[socket]
    mov   edx,h_len-html_header
    mov   esi,html_header
    mcall

    mov   eax,53                  ; send file type
    mov   ebx,7
    mov   ecx,[socket]
    mov   edx,[type_len]
    mov   esi,[file_type]
    mcall

    popa
    ret

fileinfo        dd 0
                dd 0
                dd 0
                dd 512
                dd 0x100000
getf            db '/sys/'
             times 50 db 0
wanted_file: times 100 db 0

getflen      dd  6

make_room:

   pusha

   mov  edx,ecx

   mov  esi,0x20000
   add  esi,[board_size]
   mov  edi,esi
   add  edi,edx
   mov  ecx,[board_size]
   sub  ecx,board1-board
   inc  ecx
   std
   rep  movsb
   cld

   popa
   ret


from_i  dd  0x0
from_len dd 0x0

message dd 0x0
message_len dd 0x0

read_file:                          ; start of execution

    mov  [fileinfo+16],eax
    shl ebx, 9
    mov  [fileinfo+12],ebx
    mov  [file_type],unk
    mov  [type_len],unkl-unk
    mov  [filename+40*2+6],dword 'UNK '

    cmp  [input_text+256+1],dword 'POST'
    je   yes_new_message

    cmp  [input_text+256+11],dword 'oard'     ; server board message
    jne  no_server_message_2

  yes_new_message:

    mov  eax,70
    mov  ebx,filel
    mcall
    mov  [board_size],ebx

    cmp  [input_text+256+1],dword 'POST'
    jne  no_new_message

    mov  edi,bsmt
    call set_time
    mov  edi,bsmd
    call set_date

    call check_for_incoming_data

    mov  esi,input_text+256   ; from
   newfroms:
    inc  esi
    cmp  esi,input_text+256*20
    je   no_server_message_2
    cmp  [esi],dword 'from'
    jne  newfroms

    add  esi,5
    mov  [from_i],esi

    mov  edx,0
   name_new_len:
    cmp  [esi+edx],byte 13
    je   name_found_len
    cmp  [esi+edx],byte '&'
    je   name_found_len
    cmp  edx,1000
    je   name_found_len
    inc  edx
    jmp  name_new_len

   name_found_len:

    mov  [from_len],edx

    mov  esi,input_text+256
   newmessages:
    inc  esi
    cmp  esi,input_text+256*20
    je   no_server_message_2
    cmp  [esi],dword 'sage'
    jne  newmessages

    add  esi,5
    mov  [message],esi

    mov  edx,0
   new_len:
    inc  edx
    cmp  [esi+edx],byte ' '
    je   found_len
    cmp  [esi+edx],byte 13
    jbe  found_len
    cmp  edx,input_text+5000
    je   found_len
    jmp  new_len
   found_len:
    mov  [message_len],edx


    mov  edx,0

   change_letters:

    cmp  [esi+edx],byte '+'
    jne  no_space
    mov  [esi+edx],byte ' '
   no_space:

    cmp  [esi+edx+1],word '0D'
    jne  no_br
    mov  [esi+edx],dword '<br>'
    mov  [esi+edx+4],word '  '
  no_br:

    cmp  [esi+edx],byte '%'
    jne  no_ascii
    movzx eax,byte [esi+edx+2]
    sub  eax,48
    cmp  eax,9
    jbe  eax_ok
    sub  eax,7
   eax_ok:
    movzx ebx,byte [esi+edx+1]
    sub  ebx,48
    cmp  ebx,9
    jbe  ebx_ok
    sub  ebx,7
   ebx_ok:
    imul ebx,16
    add  ebx,eax
    mov  [esi+edx],bl
    mov  [esi+edx+1],word ''
    add  edx,2
   no_ascii:

    inc  edx
    cmp  edx,[message_len]
    jbe  change_letters


    mov  edx,board1e-board1 + board2e-board2 + board3e-board3
    add  edx,[from_len]
    add  edx,[message_len]

    add  [board_size],edx

    mov  ecx,edx
    call make_room


    mov  esi,board1          ; first part
    mov  edi,0x20000
    add  edi,board1-board
    mov  ecx,edx
    cld
    rep  movsb

    mov  esi,[from_i]          ; name
    mov  edi,0x20000
    add  edi,board1-board
    add  edi,board1e-board1
    mov  ecx,[from_len]
    cld
    rep  movsb

    mov  esi,board2          ; middle part
    mov  edi,0x20000
    add  edi,board1-board + board1e-board1
    add  edi,[from_len]
    mov  ecx,board2e-board2
    cld
    rep  movsb

    mov  esi,[message]       ; message
    mov  edi,0x20000
    add  edi,board1-board + board1e-board1 + board2e-board2
    add  edi,[from_len]
    mov  ecx,[message_len]
    cld
    rep  movsb

    mov  esi,board3    ; end part
    mov  edi,0x20000
    add  edi,board1-board + board1e-board1 + board2e-board2
    add  edi,[from_len]
    add  edi,[message_len]
    mov  ecx,board3e-board3
    cld
    rep  movsb

    inc  [board_messages]

    mov  eax,[board_size]
    mov  [files+12],eax

    mov  eax,70
    mov  ebx,files
    mcall

  no_new_message:
    mov  esi,0x20000
    mov  edi,0x100000
    mov  ecx,[board_size]
    cld
    rep  movsb
    mov  ebx,[board_size]

    mov  [file_type],htm
    mov  [type_len],html-htm
    mov  [filename+40*2+6],dword 'HTM '

    jmp  file_loaded
  no_server_message_2:

    cmp  [input_text+256+9],dword 'ySta'     ; server message
    jne  no_server_message_1
    mov  edi,smt
    call set_time
    mov  edi,smd
    call set_date
    mov  eax,[documents_served]
    mov  ecx,9
    mov  edi,sms+21
    call set_value
    mov  eax,[bytes_transferred]
    mov  ecx,9
    mov  edi,smb+21
    call set_value
    mov  eax,[board_messages]
    mov  ecx,9
    mov  edi,smm+21
    call set_value
    mov  eax,[board_size]
    mov  ecx,9
    mov  edi,smz+21
    call set_value
    mov  esi,sm
    mov  edi,0x100000
    mov  ecx,sme-sm
    cld
    rep  movsb
    mov  ebx,sme-sm

    mov  [file_type],htm
    mov  [type_len],html-htm
    mov  [filename+40*2+6],dword 'HTM '

    jmp  file_loaded
  no_server_message_1:

    mov  esi,input_text+256+6
    cmp  [input_text+256+1],dword 'GET '
    jne  no_new_let
    mov  edi,wanted_file
    cld
  new_let:
    cmp  [esi],byte ' '
    je   no_new_let
    cmp  edi,wanted_file+30
    jge  no_new_let
    movsb
    jmp  new_let
  no_new_let:
    mov  [edi+0],dword 0
    mov  [edi+4],dword 0
    mov  [edi+8],dword 0

    cmp  esi,input_text+256+6
    jne  no_index
    mov  edi,wanted_file
    mov  [edi+0],dword  'inde'
    mov  [edi+4],dword  'x.ht'
    mov  [edi+8],byte   'm'
    mov  [edi+9],byte   0
    add  edi,9

    mov  [file_type],htm
    mov  [type_len],html-htm
    mov  [filename+40*2+6],dword 'HTM '

    jmp  html_file
  no_index:

    cmp  [edi-3],dword 'htm'+0
    je   htm_header
    cmp  [edi-3],dword 'HTM'+0
    je   htm_header
    jmp  no_htm_header
  htm_header:
    mov  [file_type],htm
    mov  [type_len],html-htm
    mov  [filename+40*2+6],dword 'HTM '
    jmp  found_file_type
  no_htm_header:

    cmp  [edi-3],dword 'png'+0
    je   png_header
    cmp  [edi-3],dword 'PNG'+0
    je   png_header
    jmp  no_png_header
  png_header:
    mov  [file_type],png
    mov  [type_len],pngl-png
    mov  [filename+40*2+6],dword 'PNG '
    jmp  found_file_type
  no_png_header:

    cmp  [edi-3],dword 'gif'+0
    je   gif_header
    cmp  [edi-3],dword 'GIF'+0
    je   gif_header
    jmp  no_gif_header
  gif_header:
    mov  [file_type],gif
    mov  [type_len],gifl-gif
    mov  [filename+40*2+6],dword 'GIF '
    jmp  found_file_type
  no_gif_header:

    cmp  [edi-3],dword 'jpg'+0
    je   jpg_header
    cmp  [edi-3],dword 'JPG'+0
    je   jpg_header
    jmp  no_jpg_header
  jpg_header:
    mov  [file_type],jpg
    mov  [type_len],jpgl-jpg
    mov  [filename+40*2+6],dword 'JPG '
    jmp  found_file_type
  no_jpg_header:

    cmp  [edi-3],dword 'asm'+0
    je   txt_header
    cmp  [edi-3],dword 'ASM'+0
    je   txt_header
    cmp  [edi-3],dword 'txt'+0
    je   txt_header
    cmp  [edi-3],dword 'TXT'+0
    je   txt_header
    jmp  no_txt_header
  txt_header:
    mov  [file_type],txt
    mov  [type_len],txtl-txt
    mov  [filename+40*2+6],dword 'TXT '
    jmp  found_file_type
  no_txt_header:

  html_file:

  found_file_type:

    mov  edi,getf
    add  edi,[getflen]
    mov  esi,wanted_file
    mov  ecx,40
    cld
    rep  movsb

    mov  esi,getf
    mov  edi,filename
    mov  ecx,35
    cld
    rep  movsb

    mov  [fileinfo+12],dword 1   ; file exists ?
    mov  eax,70
    mov  ebx,fileinfo
    mcall

    cmp  eax,0         ; file not found - message
    je   file_found
    mov  edi,et
    call set_time
    mov  edi,ed
    call set_date
    mov  esi,fnf
    mov  edi,0x100000
    mov  ecx,fnfe-fnf
    cld
    rep  movsb
    mov  ebx,fnfe-fnf

    mov  [file_type],htm
    mov  [type_len],html-htm
    mov  [filename+40*2+6],dword 'HTM '

    jmp  file_not_found

   file_found:

    mov  [fileinfo+12],dword 0x2f0000 ; read all of file
    mov  eax,70
    mov  ebx,fileinfo
    mcall

   file_not_found:
   file_loaded:

    and  ebx,0x3fffff
    mov  [filesize],ebx
    mov  [file_left],ebx

    mov  eax,ebx
    mov  edi,c_l+5
    mov  ebx,10
  newl:
    xor  edx,edx
    div  ebx
    mov  ecx,edx
    add  cl,48
    mov  [edi],cl
    dec  edi
    cmp  edi,c_l
    jge  newl

    mov  esi,c_l
    mov  edi,filename+46
    mov  ecx,7
    cld
    rep  movsb

    inc  [documents_served]
    mov  eax,[filesize]
    add  [bytes_transferred],eax

    call draw_data

    ret


set_value:

    pusha

    add  edi,ecx
    mov  ebx,10
  new_value:
    xor  edx,edx
    div  ebx
    add  dl,48
    mov  [edi],dl
    dec  edi
    loop new_value

    popa
    ret


set_time:

    pusha

    mov  eax,3
    mcall

    mov  ecx,3
  new_time_digit:
    mov  ebx,eax
    and  ebx,0xff
    shl  ebx,4
    shr  bl,4
    add  bx,48*256+48
    mov  [edi],bh
    mov  [edi+1],bl
    add  edi,3
    shr  eax,8
    loop new_time_digit

    popa
    ret



set_date:

    pusha

    mov  eax,29
    mcall

    mov  ecx,3
    add  edi,6
  new_date_digit:
    mov  ebx,eax
    and  ebx,0xff
    shl  ebx,4
    shr  bl,4
    add  bx,48*256+48
    mov  [edi],bh
    mov  [edi+1],bl
    sub  edi,3
    shr  eax,8
    loop new_date_digit

    popa
    ret



check_for_incoming_data:

    pusha

   check:

    mov  eax, 53
    mov  ebx, 2
    mov  ecx, [socket]
    mcall

    cmp  eax,0
    je   _ret_now

  new_data:

    mov  eax,53
    mov  ebx,2
    mov  ecx,[socket]
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
    cmp  [posy],20
    jbe  yok
    mov  [posy],1
   yok:

    mov  eax,[posy]
    imul eax,256
    add  eax,[posx]

    mov  [input_text+eax],bl

    jmp  new_data

  _ret:

     call draw_data

     mov  eax,5
     mov  ebx,1
     cmp  [input_text+256+1],dword 'POST'
     jne  no_ld
     mov  ebx,50
   no_ld:
     mcall

     jmp  check

  _ret_now:

    popa
    ret


posy dd 1
posx dd 0


check_status:

    pusha

    mov  eax,53
    mov  ebx,6
    mov  ecx,[socket]
    mcall

    cmp  eax,[status]
    je   c_ret
    mov  [status],eax
    add  al,48
    mov  [text+12],al
    call draw_data
   c_ret:

    popa
    ret


addr       dd  0x0
ya         dd  0x0

filename2:  times 100 db 32

read_string:

    mov  [addr],dword getf
    mov  [ya],dword 139

    mov  edi,[addr]
    mov  eax,0
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
    mov  [edi],al

    call print_text

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

    call print_text

    pop  edi
    sub  edi,[addr]
    mov  [getflen],edi

    mov  esi,getf
    mov  edi,dirp+12
    mov  ecx,28
    cld
    rep  movsb

    jmp  still


print_text:

    pusha

    mov  eax,4
    mov  edx,[addr]
    mov  ebx,97*65536
    add  ebx,[ya]
    mov  ecx,0x40000000
    mov  esi,23
    mov  edi,0xffffff
    mcall

    popa
    ret


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+480         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+215         ; [y start] *65536 + [y size]
    mov  edx,0x14ffffff            ; color of work area RRGGBB
    mov  edi,title                 ; WINDOW LABEL
    mcall

    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,(40)*65536+20         ; [x start] *65536 + [x size]
    mov  ecx,59*65536+9            ; [y start] *65536 + [y size]
    mov  edx,2                     ; button id
    mov  esi,0x66aa66              ; button color RRGGBB
    mcall

                           ; function 8 : define and draw button
    mov  ebx,(40)*65536+20         ; [x start] *65536 + [x size]
    mov  ecx,72*65536+9          ; [y start] *65536 + [y size]
    mov  edx,4                     ; button id
    mov  esi,0xaa6666              ; button color RRGGBB
    mcall

                           ; Enter directory
    mov  ebx,(25)*65536+66
    mov  ecx,135*65536+15
    mov  edx,6
    mov  esi,0x3388dd
    mcall

    mov  eax,38
    mov  ebx,240*65536+240
    mov  ecx,22*65536+210
    mov  edx,0x6699cc ; 002288
    mcall


    mov  ebx,241*65536+241
    mov  ecx,22*65536+210
    mov  edx,0x336699 ; 002288
    mcall

    call draw_data

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret


draw_data:

    pusha

    mov  ebx,25*65536+35           ; draw info text with function 4
    mov  ecx,0x000000
    mov  edx,text
    mov  esi,35
  newline:
    pusha
    cmp  ebx,25*65536+61
    je   now
    cmp  ebx,25*65536+74
    je   now
    cmp  ebx,25*65536+74+13*5
    je   now
    mov  ecx,ebx
    mov  bx,35*6
    shl  ecx,16
    mov  cx,9
    mov  edx,0xffffff
    mov  eax,13
    mcall
   now:
    popa
    mov  eax,4
    mcall
    add  ebx,13
    add  edx,40
    cmp  [edx],byte 'x'
    jnz  newline

    mov  [input_text+0],dword 'RECE'
    mov  [input_text+4],dword 'IVED'
    mov  [input_text+8],dword ':   '

    mov  ebx,255*65536+35           ; draw info text with function 4
    mov  ecx,0x000000
    mov  edx,input_text
    mov  esi,35
    mov  edi,17
  newline2:
    pusha
    mov  ecx,ebx
    mov  bx,35*6
    shl  ecx,16
    mov  cx,9
    mov  eax,13
    mov  edx,0xffffff
    mcall
    popa
    mov  eax,4
    mcall
    add  ebx,10
    add  edx,256
    dec  edi
    jnz  newline2

    popa

    ret


; DATA AREA

status  dd  0x0

text:
    db 'TCB status: x                           '
    db '                                        '
    db '       Activate server                  '
    db '       Stop server                      '
    db '                                        '
    db 'Requests: /TinyStat  -statistics        '
    db '          /TinyBoard -message board     '
    db '                                        '
dirp:
    db '   Files:   /sys/                       '
    db '                                        '
filename:
    db '                                        '
    db 'Size: -------                           '
    db 'Type: ---                               '
    db 'x' ; <- END MARKER, DONT DELETE


html_header:

     db  'HTTP/1.0 200 OK',13,10
     db  'Server: KolibriOS HTTP Server',13,10
     db  'Content-Length: '
c_l: db  '000000',13,10

h_len:

fnf:
     db  '<body>'
     db  '<pre>'
     db  "HTTP-Сервер v ",version," для KolibriOS",13,10,13,10
     db  "<H1>Error <FONT color=red>404</FONT> - File not found</H1>",13,10,13,10
     db  "Для получения статистики выполните запрос /TinyStat",13,10,13,10
et:  db  "xx:xx:xx",13,10
ed:  db  "xx.xx.xx",13,10
     db  "</pre></body>"
fnfe:


sm:
     db  '<body>'
     db  '<pre>'
     db  "HTTP-Сервер v ",version," для KolibriOS",13,10,13,10
     db  "Статистика: (после данного запроса)",13,10,13,10
sms: db  "- Документов принято: xxxxxxxxx",13,10
smb: db  "- Байт переданно    : xxxxxxxxx",13,10
     db  "- Местонахождение   : <a href=/TinyStat>Статистика</a>",13,10,13,10
     db  "Гостевая:",13,10,13,10
smm: db  "- Сообщений         : xxxxxxxxx",13,10
smz: db  "- Размер в байтах   : xxxxxxxxx",13,10
     db  "- Местонахождение   : <a href=/TinyBoard>Гостевая</a>",13,10,13,10
smt: db  "xx:xx:xx",13,10
smd: db  "xx.xx.xx",13,10
     db  '</pre></body>'
sme:

documents_served  dd  0x0
bytes_transferred dd  0x0

file_type  dd  0
type_len   dd  0

htm:   db  'Content-Type: text/html',13,10,13,10
html:
txt:   db  'Content-Type: text/plain',13,10,13,10
txtl:
png:   db  'Content-Type: image/png',13,10,13,10
pngl:
gif:   db  'Content-Type: image/gif',13,10,13,10
gifl:
jpg:   db  'Content-Type: image/jpeg',13,10,13,10
jpgl:
unk:   db  'Content-Type: unknown/unknown',13,10,13,10
unkl:


title db   appname,version,0

socket          dd  0x0
server_active   db  0x0

board:

db "<HTML><HEAD><TITLE>INTKolibriOS - /Гостевая/</TITLE></HEAD>",13,10
db "<BODY background=bgnd.gif BGCOLOR=#ffffff ALINK=black VLINK=black><br>",13,10
db "<center>",13,10
db "<TABLE CELLPADDING=10 CELLSPACING=0 BORDER=0 bgcolor=#ffffff width=600>"
db 13,10
db "<TR VALIGN=top><TD ALIGN=center bgcolor=F4F4F4>",13,10
db "<font size=4>Гостевая сервера INTKolibriOS</TD></TR></TABLE><br>",13,10
db "<TABLE CELLPADDING=14 CELLSPACING=2 BORDER=0 bgcolor=#ffffff width=600>"
db 13,10,13,10

board1:

db "<TR VALIGN=top>",13,10
db "<TD ALIGN=left width=80 bgcolor=F4F4F4><P>",13,10
db "<font size=3>",13,10
board1e:
db "Hex",13,10
board2:
db "</font>",13,10
db "<br><br><br>",13,10
db "<br><br><br><br>",13,10
bsmt:
db "15.23.45<br>",13,10
bsmd:
db "22.03.06",13,10
db "</P></TD>",13,10
db "<TD bgcolor=F4F4F4><P>",13,10
board2e:
db "Добро пожаловать в гостевую сервера INTKolibriOS! (-:<br>"
db 13,10
board3:
db "</P></TD></TR>",13,10,13,10
board3e:

boardadd:

db "</TABLE>",13,10
db "<br>",13,10
db "<TABLE CELLPADDING=14 CELLSPACING=3 BORDER=0 bgcolor=#ffffff width=600>"
db 13,10
db "<TR VALIGN=top>",13,10
db "<TD ALIGN=left bgcolor=F4F4F4><P>",13,10
db "<form method=Post Action=/TinyBoard>",13,10
db "Имя: <br><input type=text name=from size=20 MAXLENGTH=20><br>",13,10
db "Сообщение: <br><textarea cols=60 rows=6 name=message></textarea><br>",13,10
db "<input type=Submit Value='   Отправить сообщение    '></form>",13,10
db "</TD></TR>",13,10
db "</TABLE>",13,10
db "</BODY>",13,10
db "</HTML>",13,10

board_end:

board_size      dd  0x0
board_messages  dd  0x0

input_text:

I_END: