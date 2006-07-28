;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;   NNTP CLIENT v 0.1
;;
;;   (C) Ville Turjanmaa
;;

version equ '0.1'

include "lang.inc"
include "macros.inc"

  use32
  org    0x0

  db     'MENUET01'              ; 8 byte id
  dd     0x01                    ; header version
  dd     START                   ; start of code
  dd     I_END                   ; size of image
  dd     0x80000                 ; memory for app
  dd     0x80000                 ; esp
  dd     0x0 , 0x0               ; I_Param , I_Icon


connect_state  db  0,0,'Disconnected'
               db  1,3,'Trying..    '
               db  4,4,'Connected   '
               db  5,9,'Closing..   '

prev_state     dd  -1

space          dd  0x0

text_start     dd  0x0

text_current   dd  0x0

status     dd  0x0

server_ip  db  192,168,0,96

socket     dd  0x0

xpos       dd  0x0
ypos       dd  0x0

;;

group      db  'GROUP alt.test',13,10
           db  '                              '

grouplen   dd  16

stat       db  'STAT                          '

statlen    dd  0x0

article    db  'ARTICLE',13,10
articlelen:

;;

quit       db  'QUIT',13,10
quitlen:

xwait      dd  0x0
ywait      dd  0x0

article_n  dd  0x0
article_l  dd  0x0

article_start dd 0x0
article_last dd 0x0
article_all  dd 0x0

article_fetch dd 0x0

xpost      dd  0x0

edisave    dd  0x0


connection_status:

    pusha

    mov  eax,53
    mov  ebx,6
    mov  ecx,[socket]
    int  0x40

    cmp  eax,[prev_state]
    je   no_cos

    mov  [prev_state],eax

    mov  eax,13
    mov  ebx,435*65536+12*6
    mov  ecx,42*65536+10
    mov  edx,0xffffff
    int  0x40

    mov  ecx,-14
    mov  eax,[prev_state]

  next_test:

    add  ecx,14

    cmp  ecx,14*4
    je   no_cos

    cmp  al,[connect_state+ecx+0]
    jb   next_test
    cmp  al,[connect_state+ecx+1]
    jg   next_test

    mov  edx,ecx
    add  edx,2
    add  edx,connect_state

    mov  eax,4
    mov  ebx,435*65536+42
    mov  ecx,0x000000
    mov  esi,12
    int  0x40

  no_cos:

    popa

    ret



text_input:

    pusha
    mov  ecx,25
    mov  eax,32
    cld
    rep  stosb
    popa

    mov  [edisave],edi

  ti0:

    mov  [edi],byte ' '
    call draw_entries

    mov  eax,10
    int  0x40

    cmp  eax,2
    jne  no_more_text

    mov  eax,2
    int  0x40

    cmp   ah,8
    jne   no_bg
    cmp   edi,[edisave]
    je    ti0
    dec   edi
    jmp   ti0
   no_bg:

    cmp  ah,13
    je   no_more_text

    mov  [edi],ah
    inc  edi

    call draw_entries

    jmp  ti0

  no_more_text:

    mov  [xpost],edi

    ret


convert_text_to_ip:

    pusha

    mov   edi,server_ip
    mov   esi,text+10
    mov   eax,0
    mov   edx,[xpost]
  newsip:
    cmp   [esi],byte '.'
    je    sipn
    cmp   esi,edx
    jge   sipn
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
    cmp   esi,text+50
    jg    sipnn
    inc   edi
    cmp   edi,server_ip+3
    jbe   newsip
  sipnn:

    popa

    ret


send_group:

    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,[grouplen]
    mov  esi,group
    int  0x40
    mov  [status],3
    call clear_text
    call save_coordinates

    ret


convert_number_to_text:

    pusha

    mov  eax,[esi]
    mov  ecx,0
  newch:
    inc  ecx
    xor  edx,edx
    mov  ebx,10
    div  ebx
    cmp  eax,0
    jne  newch

    add  edi,ecx
    dec  edi
    mov  [article_l],ecx

    mov  eax,[esi]
  newdiv:
    xor  edx,edx
    mov  ebx,10
    div  ebx
    add  edx,48
    mov  [edi],dl
    dec  edi
    loop newdiv

    popa

    ret


convert_text_to_number:

    pusha

    mov   edx,0
  newdigit:
    movzx eax,byte [esi]
    cmp   eax,'0'
    jb    cend
    cmp   eax,'9'
    jg    cend
    imul  edx,10
    add   edx,eax
    sub   edx,48
    inc   esi
    jmp   newdigit
  cend:
    mov   [edi],edx
    popa

    ret


clear_text:

    mov  [text_start],0
    mov  [xpos],0
    mov  [ypos],0
    mov  [xwait],0
    mov  [ywait],0
    mov  edi,nntp_text
    mov  ecx,0x50000
    mov  eax,32
    cld
    rep  stosb
    ret


state_machine_write:


    cmp  [status],2
    jne  no_22
    call send_group
    ret
  no_22:

    cmp  [status],4
    jne  no_4
    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,[statlen] ; -stat
    mov  esi,stat
    int  0x40
    mov  [status],5
    call save_coordinates
    ret
  no_4:

    cmp  [status],6
    jne  no_6
    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,articlelen-article
    mov  esi,article
    int  0x40
    mov  [status],7
    call save_coordinates
    ret
  no_6:


    ret

save_coordinates:

    mov  eax,[xpos]
    mov  ebx,[ypos]
    mov  [xwait],eax
    mov  [ywait],ebx

    ret


state_machine_read:

    cmp  [status],1
    jne  no_1
    mov  eax,'200 '
    call wait_for_string
    ret
  no_1:

    cmp  [status],3   ;; responce to group
    jne  no_3
    mov  eax,'211 '
    call wait_for_string
    ret
  no_3:

    cmp  [status],5   ;; responce to stat
    jne  no_5
    mov  eax,'223 '
    call wait_for_string
    ret
  no_5:

    ;; after 'article' request - no wait

    cmp  [status],9
    jne  no_9
    mov  eax,'222 '
    call wait_for_string
    ret
  no_9:



    ret



wait_for_string:

    mov  ecx,[ywait]
    imul ecx,80
    add  ecx,[xwait]

    mov  ecx,[nntp_text+ecx]

    cmp  eax,ecx
    jne  no_match

    cmp  [status],3
    jne  no_stat_ret

    mov  esi,[ywait]
    imul esi,80
    add  esi,[xwait]

  new32s:
    inc  esi
    movzx eax,byte [esi+nntp_text]
    cmp  eax,47
    jge  new32s
  new32s2:
    inc  esi
    movzx eax,byte [esi+nntp_text]
    cmp  eax,47
    jge  new32s2
    inc  esi
    add  esi,nntp_text
;    mov  [esi-1],byte '.'

    mov  edi,article_n
    call convert_text_to_number
    mov  eax,[article_n]
    mov  [article_start],eax

  new32s3:
    inc  esi
    movzx eax,byte [esi]
    cmp  eax,47
    jge  new32s3
    inc  esi

    mov  edi,article_last
    call convert_text_to_number

    mov  eax,[text_current]
    add  [article_n],eax

    mov  esi,article_n
    mov  edi,nntp_text+71
    call convert_number_to_text

    mov  esi,article_n
    mov  edi,stat+5
    call convert_number_to_text

    mov  eax,[article_l]
    mov  [stat+5+eax],byte 13
    mov  [stat+6+eax],byte 10
    add  eax,5+2
    mov  [statlen],eax

    pusha
    mov  edi,text+10+66*2
    mov  ecx,25
    mov  eax,32
    cld
    rep  stosb
    mov  esi,text_current
    mov  edi,text+10+66*2
    call convert_number_to_text
    mov  eax,32
    mov  ecx,20
    mov  edi,text+10+66*3
    cld
    rep  stosb
    mov  eax,[article_last]
    sub  eax,[article_start]
    mov  [article_all],eax
    mov  esi,article_all
    mov  edi,text+10+66*3
    call convert_number_to_text
    call draw_entries
    popa

    call draw_text

  no_stat_ret:

    inc  [status]

    mov  eax,5
    mov  ebx,10
    int  0x40

    call check_for_incoming_data

  no_match:

    ret



START:                          ; start of execution

     mov  eax,40
     mov  ebx,10000111b
     int  0x40

     call clear_text

     call draw_window

still:

    call state_machine_write
    call state_machine_read

    mov  eax,23                 ; wait here for event
    mov  ebx,5
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

    call check_for_incoming_data

    call connection_status

    jmp  still

  red:                          ; redraw
    call draw_window
    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    int  0x40

    cmp  ah,' '
    jne  no_space
    mov  eax,[space]
    dec  eax
    add  [text_start],eax
    call draw_text
    jmp  still
  no_space:

    cmp  ah,177
    jne  no_plus
    inc  [text_start]
    call draw_text
    jmp  still
  no_plus:

    cmp  ah,178
    jne  no_minus
    cmp  [text_start],0
    je   no_minus
    dec  [text_start]
    call draw_text
    jmp  still
  no_minus:

    cmp  ah,179
    jne  no_next
    inc  [text_current]
    call send_group
    jmp  still
  no_next:

    cmp  ah,176
    jne  no_prev
    cmp  [text_current],0
    je   still
    dec  [text_current]
    call send_group
    jmp  still
  no_prev:

    jmp  still


  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    shr  eax,8

    cmp  eax,11
    jne  no_11
    mov  edi,text+10
    call text_input
    call convert_text_to_ip
    jmp  still
   no_11:

    cmp  eax,12
    jne  no_12
    mov  edi,text+66+10
    call text_input
    mov  esi,text+66+10
    mov  edi,group+6
    mov  ecx,[xpost]
    sub  ecx,text+66+10
    mov  eax,ecx
    cld
    rep  movsb
    mov  [group+6+eax],byte 13
    mov  [group+7+eax],byte 10
    add  eax,6+2
    mov  [grouplen],eax
    mov  [text_current],0
    jmp  still
   no_12:

    cmp  eax,13
    jne  no_13
    mov  edi,text+10+66*2
    call text_input
    mov  esi,text+10+66*2
    mov  edi,text_current
    call convert_text_to_number
    call send_group
    jmp  still
   no_13:


    cmp  eax,14
    jne  no_start
    call clear_text
    mov  eax,3
    int  0x40
    mov  ecx,eax
    mov  eax,53
    mov  ebx,5
    mov  edx,119
    mov  esi,dword [server_ip]
    mov  edi,1
    int  0x40
    mov  [socket],eax
    mov  [status],1
    jmp  still
  no_start:

    cmp  eax,15
    jne  no_end
    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,quitlen-quit
    mov  esi,quit
    int  0x40
    mov  eax,5
    mov  ebx,10
    int  0x40
    call check_for_incoming_data
    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    int  0x40
    mov  eax,5
    mov  ebx,5
    int  0x40
    mov  eax,53
    mov  ebx,8
    mov  ecx,[socket]
    int  0x40
    mov  [status],0
    jmp  still
  no_end:

    cmp  eax,1                   ; button id=1 ?
    jne  noclose
    mov  eax,-1                 ; close this program
    int  0x40
  noclose:

    jmp  still


check_for_incoming_data:

    cmp  [status],0
    jne  go_on
    ret
  go_on:

    mov  eax,53
    mov  ebx,2
    mov  ecx,[socket]
    int  0x40

    cmp  eax,0
    je   ch_ret

    mov  eax,53
    mov  ebx,3
    mov  ecx,[socket]
    int  0x40

    and  ebx,0xff

    cmp  ebx,13
    jb   no_print

    cmp  bl,13
    jne  char
    mov  [xpos],0
    inc  [ypos]
    jmp  no_print
  char:

    cmp  ebx,128
    jbe  char_ok
    mov  ebx,'?'
  char_ok:

    mov  ecx,[ypos]
    imul ecx,80
    add  ecx,[xpos]
    mov  [nntp_text+ecx],bl
    cmp  [xpos],78
    jg   noxinc
    inc  [xpos]
  noxinc:

  no_print:

    mov  eax,53
    mov  ebx,2
    mov  ecx,[socket]
    int  0x40

    cmp  eax,0
    jne  check_for_incoming_data

    call draw_text

  ch_ret:

    ret



;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    pusha

    mov  [prev_state],-1

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+520         ; [x start] *65536 + [x size]
    mov  ecx,5*65536+470           ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff            ; color of work area RRGGBB,8->color gl
    mov  esi,0x805080d0            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x005080d0            ; color of frames    RRGGBB
    int  0x40

    mov  eax,38
    mov  ebx,5*65536+515
    mov  ecx,101*65536+101
    mov  edx,0x99bbff
    int  0x40
    mov  eax,38
    mov  ebx,5*65536+515
    mov  ecx,102*65536+102
    mov  edx,0x3366aa
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff            ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40

    mov  eax,8
    mov  ebx,238*65536+8
    mov  ecx,30*65536+8
    mov  edx,11
    mov  esi,0x88aadd
    int  0x40
    mov  eax,8
    mov  ebx,238*65536+8
    mov  ecx,41*65536+8
    mov  edx,12
    int  0x40
    mov  eax,8
    mov  ebx,238*65536+8
    mov  ecx,52*65536+8
    mov  edx,13
    int  0x40

    mov  eax,8
    mov  ebx,265*65536+75
    mov  ecx,39*65536+13
    mov  edx,14
    int  0x40
    mov  eax,8
    mov  ebx,351*65536+75
    mov  ecx,39*65536+13
    mov  edx,15
    int  0x40

    call draw_entries

    call draw_text

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    popa

    ret


draw_entries:

    pusha

    mov  eax,13
    mov  ebx,30*65536+200
    mov  ecx,30*65536+44
    mov  edx,0xffffff
    int  0x40

    mov  ebx,30*65536+31           ; draw info text with function 4
    mov  ecx,0x000000
    mov  edx,text
    mov  esi,66
    mov  edi,6

  newline2:

    mov  eax,4
    int  0x40
    add  ebx,11
    add  edx,66
    dec  edi
    jnz  newline2

    popa

    ret


draw_text:

    pusha

    mov  eax,9
    mov  ebx,0x70000
    mov  ecx,-1
    int  0x40

    mov  eax,[0x70000+46]
    cmp  eax,150
    jbe  dtret

    sub  eax,111
    mov  ebx,10
    xor  edx,edx
    div  ebx
    mov  edi,eax
    dec  edi

    mov  [space],edi

    mov  ebx,20*65536+111           ; draw info text with function 4
    mov  ecx,0x000000
    mov  edx,[text_start]
    imul edx,80
    add  edx,nntp_text
    mov  esi,80
  newline:

    pusha
    mov  ecx,ebx
    shl  ecx,16
    mov  eax,13
    mov  ebx,20*65536+80*6
    mov  cx,10
    mov  edx,0xffffff
    int  0x40
    popa

    mov  eax,4
    int  0x40
    add  ebx,10
    add  edx,80
    dec  edi
    jnz  newline

  dtret:

    popa

    ret


; DATA AREA

text:
 db 'NNTP IP : 192.168.0.96             <                              '
 db 'Group   : alt.test                 <      Connect      Disconnect '
 db 'Article : 0                        <                              '
 db 'Art.max : ?                                                       '
 db '                                                                  '
 db 'Arrow left/rigth: fetch prev/next - Arrow up/down & space: scroll '

textl:


labelt:
     db   'NNTP client v',version
labellen:

nntp_text:

     db 'a'

I_END:   ;;;
