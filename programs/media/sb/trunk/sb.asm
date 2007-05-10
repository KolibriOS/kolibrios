;
;    SOUND BLASTER
;
;    Compile with FASM for Menuet
;

include 'lang.inc'
include '..\..\..\macros.inc'

use32

                org     0x0

                db      'MENUET01'              ; 8 byte id
                dd      0x01
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x80000                 ; required amount of memory
                dd      0xfff0                  ; stack position
                dd      0,0


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

vol_data             dd 0x40000
playposition         dd 0x20000
next_tic             dd 0x0
onoff                dd 0x0
pause_between_songs  dd 100
delay                dd 100
repeat_song          db 0
mono_stereo          dd 1


; 0x10000 data from hd
; 0x20000 play position for sb
; 0x40000 volume indicator data


START:                          ; start of execution

    call draw_window            ; at first, draw the window

    mov  eax,26
    mov  ebx,9
    mcall
    mov  [next_tic],eax

still:

    mov  [delay],145
    cmp  [mono_stereo],2
    jne  no_stereo
    mov  [delay],73
  no_stereo:

    mov  eax,55
    mov  ebx,2
    mov  ecx,0
    mov  edx,[mono_stereo]
    mcall

    mov  eax,23                 ; wait here for event
    mov  ebx,1
    mcall

    cmp  eax,0
    jne  do_task

still2:

    cmp  [onoff],0
    je   still

    mov  eax,26
    mov  ebx,9
    mcall
    mov  ebx,[next_tic]
    cmp  eax,ebx
    jge  play_wave_block

    mov  edi,[next_tic]
    sub  edi,eax
    mov  eax,[delay]
    sub  eax,edi
    mov  edi,eax
    call draw_volume

    jmp  still

  do_task:

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

    cmp  eax,16+4
    jne  no_irman
    mov  eax,42
    mov  ebx,4
    mcall
    dec  [ir_count]
    cmp  bl,140
    jne  no_first
    mov  [ir_count],3
  no_first:
    cmp  [ir_count],0
    jne  no_irman
    call give_wav
    cmp  bl,215
    je   play
    mov  ah,4
;;    cmp  bl,55
;;    je   rev
    mov  ah,3
;;;    cmp  bl,183
;;;    je   rev
    cmp  bl,127
    jne  no_block_dec
    add  [block],65536/512*10
  no_block_dec:
    cmp  bl,191
    jne  no_block_inc
    sub  [block],65536/512*10
  no_block_inc:
    jmp  still
   no_irman:

    jmp  still

  play_wave_block:

    mov  eax,55    ; load wave
    mov  ebx,0
    mov  ecx,[playposition]
    mcall

    mov  eax,55    ; play wave
    mov  ebx,1
    mcall

    mov  eax,26
    mov  ebx,9
    mcall
    add  eax,[delay]
    mov  [next_tic],eax

    call draw_wave
    call read_wav

    jmp  still

  red:                          ; redraw
    call draw_window
    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    mcall
    jmp  still2

  button:                       ; button
    mov  eax,17                 ; get id
    mcall

    cmp  ah,6
    jne  no_ir
    call enable_ir
    jmp  still
  no_ir:

    cmp  ah,7
    jne  no_file_name
    call read_string
    jmp  still
  no_file_name:

    cmp  ah,2                   ; button id=2
    jne  noplay
  play:
    mov  eax,[onoff]
    not  eax
    and  eax,1
    mov  [onoff],eax
    mov  [playposition],0x20000
    mov  [block],2
    call read_header
    call read_wav
;    mov  [next_tic],0
    jmp  still

  noplay:

    cmp  ah,3
    jb   no_rev
    cmp  ah,4
    jg   no_rev
    sub  ah,3
    shr  eax,8
    imul eax,4000
    sub  eax,2000
    add  [block],eax
    cmp  [block],0x0f000000
    jb   block_ok
    mov  [block],2
  block_ok:
    call display_progress
    jmp  still2
  no_rev:

    cmp  ah,5                   ; repeat song ?
    jne  no_repeat
    mov  al,[repeat_song]
    inc  al
    and  eax,1
    mov  [repeat_song],al
    shl  eax,2
    mov  eax,[repeat_text+eax]
    mov  [text+40*5+31],eax
    call draw_window
    jmp  still
    repeat_text: db 'OFF ON  '
  no_repeat:


    cmp  ah,1
    jne  noclose
    cmp  [infrared_enabled],1
    jne  no_dis
    mov  eax,45
    mov  ebx,1
    mov  ecx,4
    mcall
    mov  eax,46
    mov  ebx,1
    mov  ecx,0x3f0
    mov  edx,0x3ff
    mcall
  no_dis:
    mov  eax,-1                 ; close this program
    mcall
  noclose:

    jmp  still2


give_wav:

    pusha

    mov  eax,55
    mov  ebx,0
    mov  ecx,0x20000
    mcall

    popa
    ret

ir_count db 0x0


drop_rate dd 100

draw_volume:

; edi = tic to show

    ret

    pusha

    add  edi,[vol_data]

    mov  eax,[drop_rate]
    cmp  eax,2
    jb   no_drop
    sub  eax,2
  no_drop:
    mov  [drop_rate],eax

    movzx eax,byte [edi]
    cmp  eax,[drop_rate]
    jge  drop_ok
    mov  eax,[drop_rate]
    mov  [edi],al
    jmp  fixed_drop
  drop_ok:
    mov  [drop_rate],eax
  fixed_drop:

    mov  eax,13
    mov  ebx,320*65536+20
    mov  ecx,50*65536+1
    movzx edx,byte [edi]
    shr   edx,1
    mov  esi,128
    sub  esi,edx
    add  ecx,esi
    mov  edx,0x00ff00
    mcall

    mov  eax,13
    mov  ebx,320*65536+20
    movzx edx,byte [edi]
    shr  edx,1
    mov  ecx,edx
    add  ecx,(50+128)*65536+1
    shl  edx,16
    sub  ecx,edx
    mov  edx,0xff0000
    mcall

    popa

    ret


read_header:

    pusha

    mov  dword [file_info+4],0               ; block to read
    mov  dword [file_info+8],1               ; blocks to read
    mov  dword [file_info+12],0x10000+1024   ; return data pointer
    mov  dword [file_info+16],0x60000        ; work area for os

    mov  eax,58
    mov  ebx,file_info
    mcall

    movzx eax,byte [0x10000+1024+12+10]
    mov   [channels],eax
    movzx eax,byte [0x10000+1024+12+20]
    mov   [bytes_per_sample],eax

    cmp   [0x10000+1024],dword 'RIFF'
    jne   unknownformat
    cmp   [0x10000+1024+8],dword 'WAVE'
    jne   unknownformat

    mov [addb],128
    cmp [channels],1
    je  addb_ok
    mov [addb],256
  addb_ok:
    cmp [bytes_per_sample],1
    je  addb_ok2
    mov [addb],256
  addb_ok2:

    mov [bmul],256
    cmp [addb],256
    je  bmok
    mov [bmul],512
  bmok:

    cmp [bytes_per_sample],4
    jne no16s
    mov [addb],512   ;mono_stereo
    mov ecx,[mono_stereo]
    shr ecx,1
    shr [addb],cl
    mov [bmul],128   ;mono_stereo
    shl [bmul],cl
  no16s:

    popa

    ret

unknownformat:

    mov  [onoff],0

    call display_progress

    mov  eax,13
    mov  ebx,190*65536+10
    mov  ecx,104*65536+10
    mov  edx,0xff0000
    mcall
    pusha

    mov  eax,5
    mov  ebx,[pause_between_songs]
    mcall

    popa
    mov  eax,13
    mov  edx,0x000000
    mcall

    popa

    ret



channels           dd 0x0  ; 1=mono, 2 stereo
bytes_per_sample   dd 0x0  ; 1=8 2=2*8/16 4=16

buffer dd 0x20000

block  dd 0x2
addb   dd 256    ;  128 = mono 8 bit , 256 = stereo 8 bit/16 bit mono
bmul   dd 0x0    ;  512 = mono 8 bit , 256 = stereo 8 bit/16 bit mono

file_size dd 100

current_play dd wavfile+40*0


read_wav:

    pusha

  new_file:

    mov  edx,[block]

  newread:

    mov  dword [file_info+4],edx             ; block to read
    mov  dword [file_info+8],1               ; blocks to read
    mov  dword [file_info+12],0x10000+1024   ; return data pointer
    mov  dword [file_info+16],0x60000        ; work area for os

    mov  eax,58
    mov  ebx,file_info
    mcall


    pusha
    mov  eax,11
    mcall
    cmp  eax,1
    jne  no_wd
    call draw_window
  no_wd:
    popa

    pusha
    mov  eax,38
    mov  ebx,1*65536+128
    mov  ecx,71*65536+71
    add  ebx,25*65536+25
    mov  edx,0x555555
;    mcall
    mov  eax,38
    mov  ebx,[esp+32-12]
    and  ebx,65536/512 -1
    or   ebx,1*65536
    add  ebx,25*65536+25
    mov  ecx,71*65536+71
    mov  edx,0x999999
;    mcall
    popa

    cmp  eax,0
    je   conp

    movzx eax,byte [repeat_song]
    inc   eax
    and   eax,1
    imul  eax,40
    mov  [current_play],wavfile
  play_ok:

    mov  [onoff],1
    mov  [playposition],0x20000
    mov  [block],20

    mov  eax,5
    mov  ebx,[pause_between_songs]
    add  ebx,[delay]
    mcall

    call read_header

    cmp  [onoff],0
    je   noplay2
    cmp  [repeat_song],0
    je   noplay2

    call display_progress

    jmp  new_file

  noplay2:

    mov  [onoff],0
    mov  [block],2
    call display_progress

    popa
    ret
  conp:

    mov  [file_size],ebx

    mov  esi,0x10000+1024   ; 8 bit stereo & 16 bit mono
    mov  edi,edx
    sub  edi,[block]
    imul edi,[bmul]
    add  edi,[buffer]
    mov  ecx,512

  movedata:

    mov  al,[esi+1]

    cmp  [bytes_per_sample],4 ; for 16 bit stereo
    jne  no_16_stereo
    mov  al,[esi+1]
    add  al,128
  no_16_stereo:

    cmp  [bytes_per_sample],1 ; for 16 bit mono
    je   no_16_mono
    cmp  [channels],2
    je   no_16_mono
    mov  al,[esi+1]
    add  al,128
  no_16_mono:

    mov  [edi],al
    mov  eax,[bytes_per_sample]
    cmp  [mono_stereo],1
    je   bps1
    mov  eax,[bytes_per_sample]
    push ecx
    mov  ecx,[mono_stereo]
    dec  ecx
    shr  eax,cl
    pop  ecx
  bps1:
    add  esi,eax ; 2;[bytes_per_sample] ; / mono_stereo
    add  edi,1
    loop movedata

    add  edx,1
    mov  ecx,[block]
    add  ecx,[addb]
    cmp  edx,ecx
    jbe  newread

    mov  ecx,[addb]
    add  [block],ecx

    call display_progress

  rewr:

    popa

    call set_vol_data

    ret


set_vol_data:

;    ret

    pusha

    mov  eax,65536
    xor  edx,edx
    mov  ebx,[delay]
    div  ebx
    push eax

    mov  esi,[playposition]
    mov  edi,[vol_data]
    mov  ecx,[delay]

  svd:

    mov   eax,0
    mov   edx,100
  svd3:
    movzx ebx,byte [esi]
    cmp   ebx,128
    jge   svd2
    mov   ebx,0
   svd2:
    sub   ebx,128
    shl   ebx,1

    cmp   ebx,ebp
    jbe   svd4
    mov   edx,ebx
   svd4:

    inc   esi
    inc   eax
    cmp   eax,[esp]
    jb    svd3

    mov  [edi],dl
    inc  edi
    loop svd

    pop  eax
    popa

    ret


addr  dd   0x0
ya    dd   0x0


read_string:

     mov  [onoff],0

    mov  [addr],wavfile
    mov  [ya],30

    mov  edi,[addr]
    mov  al,'_'
    mov  ecx,32
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

    add  edi,1
    mov  esi,[addr]
    add  esi,32
    cmp  esi,edi
    jnz  f11

  read_done:

    mov  ecx,[addr]
    add  ecx,38
    sub  ecx,edi
    mov  eax,0
    cld
    rep  stosb

    call print_text

    ret


print_text:

display_progress:

    pusha

    mov  eax,13                ; gray progress bar
    mov  ebx,25*65536+215
    mov  ecx,61*65536+8
    mov  edx,[border]
    mcall

    cmp  [onoff],1
    je   yes_playing
    mov  [block],0
    mov  [file_size],100*512
  yes_playing:
    mov  eax,[block]           ; yellow progress bar
    imul eax,214
    xor  edx,edx
    mov  ebx,[file_size]
    shr  ebx,9
    or   ebx,1
    div  ebx
    mov  ebx,eax
    and  ebx,0xff
    mov  eax,13
    add  ebx,25*65536
    mov  ecx,61*65536+1
    mov  edx,[drawp]
   newbar:
    mcall
    add  edx,0x101010
    add  ecx,1*65536
    cmp  ecx,65*65536
    jb   newbar
   newbar2:
    mcall
    sub  edx,0x101010
    add  ecx,1*65536
    cmp  ecx,69*65536
    jb   newbar2


    mov   eax,[block]
    imul  eax,214-30
    xor   edx,edx
    mov   ebx,[file_size]
    shr   ebx,9
    or    ebx,1
    div   ebx
    mov   ebx,eax
    shl   ebx,16
    add   ebx,25*65536+30
    mov   ecx,61*65536+9
    mov   edx,0xeeeeff
    mov   eax,13
    mov   edi,5
  newb:
;    mcall
    add   ebx,1*65536-2
    add   ecx,1*65536-2
    sub   edx,0x332211;3366aa
    dec   edi
    jnz   newb


  noyellow:

    mov  esi,[current_play]
    mov  edi,now_playing
    mov  ecx,40
    cld
    rep  movsb

    mov  eax,13
    mov  ebx,42*65536+33*6
    mov  ecx,114*65536+11
    mov  edx,0x000000
    mcall

    mov  eax,4
    mov  ebx,42*65536+117
    mov  ecx,[textc]
    mov  edx,now_playing
    mov  esi,38
    mcall

    popa

    ret


shape_window:

    ret

    pusha

    mov  eax,50
    mov  ebx,0
    mov  ecx,shape_reference
    mcall

    mov  eax,50
    mov  ebx,1
    mov  ecx,4
    mcall

    popa

    ret


shape_reference:

times 1  db  1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0
times 9  db  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1

infrared_enabled db 0x0


enable_ir:

    pusha

    mov  eax,46
    mov  ebx,0
    mov  ecx,0x3f0
    mov  edx,0x3ff
    mcall

    mov  eax,45
    mov  ebx,0
    mov  ecx,4
    mcall

    mov  eax,40
    mov  ebx,10000b shl 16 + 111b
    mcall

    mov  [infrared_enabled],1

    popa

    ret



;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    pusha

    inc  [next_tic]

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall
                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+320         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+140         ; [y start] *65536 + [y size]
    mov  edx,[bgr]
    or   edx,0x13000000            ; color of work area RRGGBB,8->color gl
    mov  edi,title                 ; WINDOW LABEL
    mcall

                                   
    mov  eax,8                     ; START/STOP  - id 2
    mov  ebx,24*65536+77
    mov  ecx,80*65536+16
    mov  edx,2
    mov  esi,[border]
    mcall

    inc  edx                       ; << / >>     - id 3 , 4
    add  ebx,86*65536-57
    mov  eax,8
    mcall
    inc  edx
    add  ebx,24*65536
    mov  eax,8
    mcall

    mov  eax,8                      ; REPEAT
    add  ebx,29*65536+54
    inc  edx
    mcall

    mov  eax,8                      ; enable infrared
    add  ebx,98*65536-33
    add  ecx,10*65536+10
    inc  edx
    mcall

    pusha
    mov  eax,8
    mov  ebx,25*65536+9
    mov  ecx,115*65536+9
    inc  edx
    mcall
    popa

    mov  eax,4
    shr  ecx,16
    mov  bx,cx
    add  ebx,2*65536+4
    mov  ecx,0xffffff
    mov  edx,infrared_text
    mov  esi,10
    mcall
    add  ebx,11
    add  edx,10
    mov  eax,4
    mcall

    mov  ebx,25*65536+35           ; draw info text with function 4
    mov  ecx,[textc]
    mov  edx,text
    mov  esi,40
  newline:
    mov  eax,4
    mcall
    add  ebx,10
    add  edx,40
    cmp  [edx],byte 'x'
    jne  newline

    call display_progress

    call draw_wave

    mov  eax,12
    mov  ebx,2
    mcall

    popa
    ret




draw_wave:

;    ret

    pusha

    mov   eax,13
    mov   ebx,260*65536+43
    mov   ecx,42*65536+32
    mov   edx,[border]
    mcall

    mov   esi,[playposition]
    mov   ebx,260
   npix:
    mov   eax,1
    inc   ebx
    movzx ecx,byte [esi]
    shr   ecx,3
    add   ecx,42
    mov   edx,[drawc];0x2255aa
    mcall

    add   esi,2

    cmp   ebx,300
    jbe   npix

    popa

    ret


; DATA AREA

infrared_text: db  'IRMAN     INFRAR      '


textc   dd  0xffffff
bgr     dd  0x00000000
drawc   dd  0x2255aa
drawp   dd  0x8011aa
border  dd  0x5577aa

text:
    db 'Define SB, HD & partition with setup    '
    db 'If clipping change "delay" in source    '
    db '                                        '
    db '                                        '
    db '                                        '
    db ' START/STOP    <<  >>   REPEAT:OFF      '
    db 'x <- END MARKER, DONT DELETE            '
now_playing:
    db '                                        '
    db 'xx                                      '

file_info:

    dd  0   ; read
    dd  0
    dd  0
    dd  0
    dd  0

wavfile:
    db '/HD/1/MENUET/MUSIC/FILE7.WAV',0
    db '                                        '


title      db   ' WAVE PLAYER  :  8b Mono - 16b Stereo',0

I_END:
