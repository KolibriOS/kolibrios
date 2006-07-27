;********************************
;*                              *
;*     DESKTOP ICON MANAGER     *
;*                              *
;*  Compile with flat assembler *
;*                              *
;********************************
;  22.02.05 was modified for work with new multi-thread ICON.
;******************************************************************************
RAW_SIZE equ 350000
ICON_SIZE equ 32*32*3
GIF_SIZE equ 45000
REC_SIZE equ 80
ICON_LST equ '/RD/1/ICON2.LST'
;ICON_APP equ '/hd/1/me/icon2';
ICON_APP equ '/RD/1/ICON2'
;ICON_STRIP equ '/HD/1/ME/ICONSTRP.GIF'
ICON_STRIP equ '/RD/1/ICONSTRP.GIF'

  use32
  org    0x0
  db     'MENUET01'     ; 8 byte id
  dd     0x01           ; header version
  dd     START          ; start of code
  dd     I_END          ; size of image
  dd     icon_data+0x30000        ; memory for app
  dd     icon_data+0x30000        ; esp
  dd     I_Param , 0x0  ; I_Param , I_Icon
include  'macros.inc'
include  'lang.inc'
COLOR_ORDER equ MENUETOS
include  'gif_lite.inc'
;include  'debug.inc'
purge newline
;******************************************************************************
START:                       ; start of execution
    mcall 58,finfo
    cmp   ebx,GIF_SIZE
    ja    close
    mov   esi,gif_file
    mov   edi,strip_file
    mov   eax,icon_data
    call  ReadGIF
    movzx eax,word[strip_file+10]
    shr  eax,5
    mov  [icon_count],eax
    call load_ic
  boot_str:
    cmp   [I_Param],dword 'BOOT'
    je   load_icon_list2
      call  load_icon_list
 red:
    call draw_window         ; at first, draw the window
    mov  esi,[current_icon]
    jmp  band
still:

    mov  eax,10              ; wait here for event
    int  0x40

    cmp  eax,1               ; redraw request ?
    je   red
    cmp  eax,2               ; key in buffer ?
    je   key
    cmp  eax,3               ; button in buffer ?
    je   button

    jmp  still

  key:                       ; key
    mov  eax,2               ; just read it and ignore
    int  0x40
    jmp  still

  button:                    ; button
    mov  eax,17              ; get id
    int  0x40

    shr  eax,8

    cmp  eax,1               ; button id=1 ?
    je   close
  noclose:
      mov  esi,[current_icon]
      add  esi,12
    mov  ebx,[cur_band];eax
    cmp  eax,31
    jne  .no_back
    add  ebx,8
    mov  eax,[icon_count]
    cmp  eax,ebx
    jae  .drwic2
    xor  ebx,ebx
    jmp  .drwic2
  .no_back:
    cmp  eax,30
    jne  .no_side
    test ebx,ebx
    jnz  .dec
    mov  ebx,[icon_count]
         and  ebx,0xfffffff8
    add  ebx,8
  .dec:
    sub  ebx,8
  .drwic2:
    mov  [cur_band],ebx
  .drwic1:
    call draw_icon
    jmp  still
    .no_side:
       cmp  eax,32
       jne  .no_ico
       push ebx
       mcall 37,1
       pop  ebx
          shr  eax,16
       sub  eax,33-19
          mov  edi,34
          xor  edx,edx
          div  edi
    lea  ecx,[ebx+eax]
    cmp  ecx,[icon_count]
    jae  still
          mov  [sel_icon1],eax
          mov  ecx,eax
    add  eax,ebx
    call itoa
          jmp  .drwic1
  .no_ico:
    cmp  eax,11
    jb   no_str
    cmp  eax,13
    jg   no_str
    call read_string
    jmp  still
  no_str:


    cmp  eax,21              ; apply changes
    jne  no_apply

    ; (1) save list

    mov  ebx,finfo
    mov  dword[ebx],1
    mov  edx,REC_SIZE
    imul edx,dword [icons]
    mov  [ebx+8],edx
    mov  esi,iconlst
    call lst_path
    mcall 58

    ; (2) terminate all icons
    mov  eax,9
    mov  ebx,I_END
    mov  ecx,-1
    int  0x40
    mov  edi,[ebx+30]
     newread2:
    mov  esi,1
   newread:
    inc  esi
    mov  eax,9
    mov  ebx,I_END
    mov  ecx,esi
    int  0x40
    cmp  edi,[ebx+30]
    je   newread
    cmp  esi,eax
    jg   all_terminated

    cmp  [I_END+10],dword 'ICON'
    jne  newread
    mov  eax,51
    cmp  eax,[I_END+42]
    jne  newread
    cmp  eax,[I_END+46]
    jne  newread

    mov  eax,18
    mov  ebx,2
    mov  ecx,esi
    int  0x40

    jmp  newread2

finfo:
        dd 0
        dd 0
        dd GIF_SIZE/512
        dd gif_file
        dd icon_data
  .path:
        db ICON_STRIP,0
        rb 31-($-.path)

   all_terminated:

   apply_changes:

    mov  ebx,finfo
    mov  dword[ebx],16
    mov  dword[ebx+8],boot_str+6
    mov  esi,iconname
    call lst_path
    mcall 58
    jmp   still

  no_apply:

    cmp  eax,22                 ; user pressed the 'add icon' button
    jne  no_add_icon

    mov  eax,13
    mov  ebx,24*65536+270
    mov  ecx,(250+8*14)*65536+8
    mov  edx,0xffffff
    int  0x40
    mov  eax,4
    mov  ebx,24*65536+250+8*14
    mov  ecx,0xff0000
    mov  edx,add_text
    mov  esi,add_text_len-add_text
    int  0x40

    mov  eax,10
    int  0x40
    cmp  eax,3
    jne  still
    mov  eax,17
    int  0x40
    shr  eax,8
    cmp  eax,40
    jb   no_f
    mov  edi,eax
    sub  eax,40

    xor  edx,edx  ; bcd -> 10
    mov  ebx,16
    div  ebx
    imul eax,10
    add  eax,edx

    mov  ebx,eax
    add  ebx,icons_reserved
    cmp  [ebx],byte 'x'
    je   no_f
    mov  [ebx],byte 'x'

    mov  [cur_btn],edi
    xor  edx,edx
    mov  ebx,10
    div  ebx
    add  eax,65
    add  edx,65
    mov  [icon_default+0],dl
    mov  [icon_default+1],al

    inc  dword [icons]
    mov  edi,[icons]
    dec  edi
    imul edi,REC_SIZE
    add  edi,icon_data

    mov  [current_icon],edi

    mov  esi,icon_default
    mov  ecx,REC_SIZE
    cld
    rep  movsb
    mov  esi,[current_icon]
    jmp  band
  no_f:

    call draw_btns;draw_window

    jmp  still

  no_add_icon:


    cmp  eax,23                     ; user pressed the remove icon button
    jne  no_remove_icon

    mov  eax,13
    mov  ebx,24*65536+270
    mov  ecx,(250+8*14)*65536+8
    mov  edx,0xffffff
    int  0x40
    mov  eax,4
    mov  ebx,24*65536+250+8*14
    mov  ecx,0xff0000
    mov  edx,rem_text
    mov  esi,rem_text_len-rem_text
    int  0x40

    mov  eax,10
    int  0x40
    cmp  eax,3
    jne  no_f;ound
    mov  eax,17
    int  0x40
    shr  eax,8
    cmp  eax,40
    jb   red;no_f;ound
    sub  eax,40

    xor  edx,edx
    mov  ebx,16
    div  ebx
    imul eax,10
    add  eax,edx

    mov  ebx,eax
    add  ebx,icons_reserved
    cmp  [ebx],byte 'x'
    jne  red
    mov  [ebx],byte ' '

    xor  edx,edx
    mov  ebx,10
    div  ebx
    shl  eax,8
    mov  al,dl

    add  eax,65*256+65

    mov  esi,icon_data
    mov  edi,REC_SIZE
    imul edi,[icons]
    add  edi,icon_data
  news:
    cmp  word [esi],ax
    je   foundi
    add  esi,REC_SIZE
    cmp  esi,edi
    jb   news
    jmp  red

  foundi:

    mov  ecx,edi
    sub  ecx,esi

    mov  edi,esi
    add  esi,REC_SIZE

    cld
    rep  movsb

    dec  [icons]

    mov  eax,icon_data
    mov  [current_icon],eax
    movzx ebx,word[eax]
    sub  bx,'AA'
    shl  bl,4
    shr  ebx,4
    add  ebx,40
    mov  [cur_btn],ebx

    jmp  red

  no_remove_icon:

    cmp  eax,40                 ; user pressed button for icon position
    jb   no_on_screen_button
    mov  edi,eax
    sub  eax,40
    mov  edx,eax
    shl  eax,4
    and  edx,0xf
    mov  dh,ah
    add  edx,65*256+65

    mov  esi,icon_data
    mov  ecx,[icons]
    cld
   findl1:
    cmp  dx,[esi]
    je   foundl1
    add  esi,REC_SIZE
    loop findl1
    jmp  still

   foundl1:

    mov  [current_icon],esi
    mov  [cur_btn],edi
   band:
    add  esi,12
    call atoi
    and  eax,0xfffff8
    mov  [cur_band],eax
    call draw_btns

    jmp  still

  no_on_screen_button:


    jmp  still


current_icon dd icon_data


print_strings:

    pusha

    mov  eax,13              ; clear text area
    mov  ebx,100*65536+180
    mov  ecx,(278+12)*65536+40
    mov  edx,0xffffff
    int  0x40

          xor  edi,edi
    mov  eax,4               ; icon text
    mov  ebx,100*65536+278+14
    mov  ecx,3
  .ll:
    push ecx
    mov  ecx,0x000000
    mov  edx,[current_icon]
    add  edx,[positions+edi*4]
    movzx esi,byte[str_lens+edi]
    inc  edi
    int  0x40
    add  ebx,14
    pop  ecx
    loop .ll

    popa
    ret

iconlst db ICON_LST,0

load_icon_list:

    mov   edi,icons_reserved   ; clear reserved area
    mov   eax,32
    mov   ecx,10*9
    cld
    rep   stosb

    mov   ecx,[icons]          ; set used icons to reserved area
    mov   esi,icon_data
  ldl1:
    movzx ebx,byte [esi+1]
    sub   ebx,65
    imul  ebx,10
    movzx eax,byte [esi]
    add   ebx,eax
    sub   ebx,65
    add   ebx,icons_reserved
    mov   [ebx],byte 'x'
    add   esi,REC_SIZE
    loop  ldl1
    ret

lst_path:
    mov   ecx,30
    mov   edi,finfo.path
    rep   movsb
    ret

load_ic:
    mov   ebx,finfo
    mov   dword[ebx+8],(48*REC_SIZE)shr 9+1
    mov   dword[ebx+12],icon_data
    mov   dword[ebx+16],gif_file
    mov   esi,iconlst
    call  lst_path
    mcall 58
    lea   eax,[ebx+10]
    xor   edx,edx
    mov   ebx,REC_SIZE
    div   ebx
    mov   [icons],eax
        ret


positions dd 3,16,47
str_lens db 8,30,30

read_string:
    pusha
    sub  eax,11
    movzx ecx,byte[str_lens+eax]
    mov  [cur_str],ecx
    mov  eax,[positions+eax*4]

    mov  edi,[current_icon]
    add  edi,eax
    mov  [addr],edi

          add  edi,ecx

  .l1:
    dec  edi
    cmp  byte[edi],' '
    jne  .found
    mov  byte[edi],'_'
    loop .l1
    dec  edi
  .found:
    inc  edi
    push  edi
    call print_strings

    pop  edi
  f11:
    mov  eax,10
    int  0x40
    cmp  eax,2
    jz   fbu
    jmp  rs_done
  fbu:
    mov  eax,2
    int  0x40
    shr  eax,8
    cmp  eax,13
    je   rs_done
    cmp  eax,8
    jnz  nobsl
    cmp  edi,[addr]
    jz   f11
    dec  edi
    mov  [edi],byte '_'
    call print_strings
    jmp  f11
  nobsl:
    cmp  eax,31
    jbe  f11
    mov  [edi],al
    call print_strings

    inc  edi
    mov  esi,[addr]
    add  esi,[cur_str]
    cmp  esi,edi
    jnz  f11

   rs_done:

    mov  ecx,[addr]
    add  ecx,[cur_str]
    sub  ecx,edi
    mov  eax,32
    cld
    rep  stosb
    call print_strings
    popa
    ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    mov  eax,0
    mov  ebx,210*65536+300
    mov  ecx,30*65536+390-14
    mov  edx,0x03ffffff
    mov  esi,0x808899ff
    mov  edi,0x008899ff
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4
    mov  ebx,8*65536+8
    mov  ecx,0x10ffffff
    mov  edx,labelt
    mov  esi,labellen-labelt
    int  0x40

    mov  eax,13                    ; WINDOW AREA
    mov  ebx,20*65536+260
    mov  ecx,35*65536+200
    mov  edx,0x3366cc
    int  0x40

    mov  eax,38                    ; VERTICAL LINE ON WINDOW AREA
    mov  ebx,150*65536+150
    mov  ecx,35*65536+235
    mov  edx,0xffffff
    int  0x40

    mov  eax,38                    ; HOROZONTAL LINE ON WINDOW AREA
    mov  ebx,20*65536+280
    mov  ecx,135*65536+135
    mov  edx,0xffffff
    int  0x40

    mov  eax,8                     ; TEXT ENTER BUTTONS
    mov  ebx,20*65536+72
    mov  ecx,(275+1+14)*65536+13-2
    mov  edx,11
    mov  esi,[bcolor]
    int  0x40
    inc  edx
    add  ecx,14*65536
    int  0x40
    inc  edx
    add  ecx,14*65536
    int  0x40

;    mov  eax,8                     ; APPLY AND SAVE CHANGES BUTTON
    mov  ebx,20*65536+259
    mov  ecx,(329+2)*65536+15-4
    mov  edx,21
    mov  esi,[bcolor]
    int  0x40

;    mov  eax,8                     ; ADD ICON BUTTON
    mov  ebx,20*65536+129-2
    add  ecx,14*65536
    inc  edx
    int  0x40

;    mov  eax,8                     ; REMOVE ICON BUTTON
    add  ebx,(130+2)*65536
    inc  edx
    int  0x40

    mcall ,<20-14,8>,<260-23,32>,30+1 shl 30    ; IMAGE BUTTON
    inc  edx
    add  ebx,(36*7+26) shl 16
    mcall
    add  edx,1+1 shl 29
    mov  ebx,(33-19) shl 16+(34*8)
    mcall
    mcall 4,<23-15,273-24>,0,arrows,1
    add  ebx,(36*7+27)shl 16
    add  edx,2
    mcall
    dec  edx
    mcall ,<120,250>
    lea  edx,[ebx+8 shl 16]
    mov  ecx,[icon_count]
    mcall 47,0x30000,,,0

;;
    mov  ebx,24*65536+250+14+14+14
    mov  ecx,0xffffff
    mov  edx,text
    mov  esi,47
  newline:
    mov  ecx,[edx]
    add  edx,4
    mov  eax,4
    int  0x40
    add  ebx,14
    add  edx,47
    cmp  [edx],byte 'x'
    jne  newline
draw_btns:
;;
    mov  eax,0                     ; DRAW BUTTONS ON WINDOW AREA
    mov  ebx,20*65536+25
    mov  ecx,35*65536+19
    mov  edi,icon_table
    mov  edx,40
   newbline:

    cmp  [edi],byte 'x'
    jne  no_button

    mov  esi,0x5577cc
    cmp  [edi+90],byte 'x'
    jne  nores
    mov  esi,0xcc5555
    cmp  edx,[cur_btn]
    jne  nores
    mov  esi,0xe7e05a
  nores:

    push eax
    mov  eax,8
    int  0x40
    pop  eax

  no_button:

    add  ebx,26*65536

    inc  edi
    inc  edx

    inc  al
    cmp  al,9
    jbe  newbline
    mov  al,0

    add  edx,6

    ror  ebx,16
    mov  bx,20
    ror  ebx,16
    add  ecx,20*65536

    inc  ah
    cmp  ah,8;9
    jbe  newbline
    call print_strings
    call draw_icon
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret

draw_icon:
    mcall 13,<33-20,34*8+2>,<260-24,37+15-2>,0xffffff
    mov  esi,[current_icon]
    add  esi,12
    call atoi
    push eax
    cmp  eax,[cur_band]
    jb   .nou
    sub  eax,[cur_band]
    cmp  eax,7
    ja   .nou
    imul eax,34 shl 16
    lea  ebx,[eax+(33-19) shl 16]
    mov  bx,34
    mcall 13,,<236+35,3>,0xff0000
    mov  eax,[esp]
  .nou:
    mov  eax,[cur_band]
    and  eax,0xfffffff8
    push eax
    imul eax,ICON_SIZE
    lea  ebx,[strip_file+12+eax]
    mov  ecx,8
    mov  edx,(33-18) shl 16+238
  .nxt:
    push ecx
    mcall 7,,<32,32>
    pop  ecx
    add  ebx,ICON_SIZE
    add  edx,34 shl 16
    loop .nxt

    mcall 4,<45,280-2>,0,rep_text,rep_text_len-rep_text
    lea  edx,[ebx+(8*5)shl 16]
    pop  ecx
    mcall 47,0x30000,,,0xff
    add  ecx,7
    add  edx,(3*8+4)shl 16
    mcall
    mov  ecx,[icon_count]
    add  edx,(5*8+4)shl 16
    mcall
    pop  ecx
    add  edx,(10*8+4)shl 16
    mcall ,,,,0xff0000
    ret

; DATA AREA


bcolor dd 0x335599

icon_table:

    times 4  db  'xxxx  xxxx'
    times 2  db  '          '
    times 1  db  '          '
    times 2  db  'xxxx  xxxx'
;    times 1  db  '          '

icons_reserved:
    times 9  db  '          '

if lang eq ru
  text:
      db 255,255,255,0,   '   íÖäëí                                       '
      db 255,255,255,0,   ' èêéÉêÄååÄ                                     '
      db 255,255,255,0,   ' èÄêÄåÖíêõ                                     '
      db 255,255,255,0,   '                 èêàåÖçàíú                     '
      db 255,255,255,0,   '      ÑéÅÄÇàíú              ìÑÄãàíú            '
      db 0,0,0,0,         'çÄÜåàíÖ çÄ èéáàñàû àäéçäà Ñãü êÖÑÄäíàêéÇÄçàü   '
      db                  'x' ; <- END MARKER, DONT DELETE

  labelt:
       db 'ç†·‚‡Æ©™† ‡†°ÆÁ•£Æ ·‚Æ´†'
  labellen:
else
  text:
      db 255,255,255,0,   '   TITLE                                       '
      db 255,255,255,0,   '  APP NAME                                     '
      db 255,255,255,0,   ' PARAMETERS                                    '
      db 255,255,255,0,   '                APPLY CHANGES                  '
      db 255,255,255,0,   '      ADD ICON              REMOVE ICON        '
      db 0,0,0,0,         'çÄÜåàíÖ çÄ èéáàñàû àäéçäà Ñãü êÖÑÄäíàêéÇÄçàü   '
      db                  'x' ; <- END MARKER, DONT DELETE

  labelt:
       db 'Icon Manager'
  labellen:
end if

;ya    dd 0

add_text db 'çÄÜåàíÖ çÄ èéáàñàû çÖàëèéãúáìÖåéâ àäéçäà'
add_text_len:

rem_text db 'çÄÜåàíÖ çÄ èéáàñàû àëèéãúáìÖåéâ àäéçäà'
rem_text_len:
arrows db '</>'
iconname:
      db ICON_APP,0

icon_default:
   db   'AA-SYSXTREE-000-/RD/1/SYSXTREE                '
   db   '-                              *'
   db   13,10

rep_text:
if lang eq ru
     db 'áçÄóäà    -     àá    , ÇõÅêÄç #'
else
     db 'ICONS     -     OF    , SELECTED'
end if

rep_text_len:

;//////////////////////////
get_bg_info:
    mov  eax,39
    mov  ebx,4
    int  0x40
    mov  [bgrdrawtype],eax

    mov  eax,39     ; get background size
    mov  ebx,1
    int  0x40
    mov  [bgrxy],eax

    mov  ebx,eax
    shr  eax,16
    and  ebx,0xffff
    mov  [bgrx],eax
    mov  [bgry],ebx
    ret

calc_icon_pos:
    movzx eax,byte [ebp-20]    ; x position
    sub  eax,'A'        ;eax - number of letter
    cmp  eax,4
    jg     no_left
    shl  eax,6 ;imul eax,64
    add  eax,16
    movzx ebx,[warea.left]
    add  eax,ebx
    jmp  x_done
  no_left:
    sub  eax,9
    sal  eax,6 ;imul eax,64
    sub  eax,16+52-1
    movzx ebx,[warea.right]
    add  eax,ebx
  x_done:
;    mov  [xpos],eax
    mov  [ebp-12],eax

    movzx eax,byte [ebp-20+1]  ; y position
    sub  eax,'A'        ; eax - number of letter
    cmp  eax,4
    jg     no_up
    shl  eax,6            ;imul eax,80
    add  eax,16
    movzx ebx,[warea.top]
    add  eax,ebx
    jmp  y_done
  no_up:
    sub  eax,9
    shl  eax,6            ;imul eax,80
    sub  eax,16-1
    movzx ebx,[warea.bottom]
    add  eax,ebx
  y_done:
;    mov  [ypos],eax
    mov  [ebp-8],eax
    ret

;START2:
load_icon_list2:
        mov  eax,finfo
        mov  dword[eax],16
        mov  dword[eax+8],param_str
    call  get_bg_info

        mcall   48,5
        mov     [warea.by_x],eax
        mov     [warea.by_y],ebx

        mov     eax,14
        int     0x40
        add     eax,0x00010001
        mov     [scrxy],eax

apply_changes2:

    mov  edi,[icons]
    mov  esi,icon_data
    mov  ebp,0x5000 ; threads stack starting point

  start_new:
    mov eax,[esi]
    mov [ebp-20],eax
    call calc_icon_pos

    mov  eax,51
    mov  ebx,1
    mov  ecx,thread
;    mov  edx,[thread_stack]
    mov  edx,ebp
;    sub  edx,4
;    mov  [edx],esi
    mov  dword[ebp-4],esi
    int  0x40
;    add  [thread_stack],0x100
    add  ebp,0x100

    mov  eax,5
    mov  ebx,1
wait_thread_start:         ;wait until thread draw itself first time
    cmp  [create_thread_event],bl
    jz     wait_thread_end
    int  0x40
    jmp  wait_thread_start
wait_thread_end:
    dec  [create_thread_event]     ;reset event


    add  esi,REC_SIZE
    dec  edi
    jnz  start_new
  close:
    or     eax,-1
    int  0x40

thread:
;   pop  ebp ;ebp - address of our icon
    sub  esp,12
    mov  ebp,esp
    sub  esp,16
    call draw_window2
    mov  [create_thread_event],1
    mov  eax,40
    mov  ebx,010101b
    int  0x40

still2:

    mov  eax,10
    int  0x40

    cmp  eax,1
    je     red2
    cmp  eax,3
    je     button2
    cmp  eax,5
    jne  still2

    call  get_bg_info
    mov   eax,5
    mov   ebx,1
    call  draw_icon2

    jmp  still2

  red2:
        mcall   14
        add     eax,0x00010001
        mov     [scrxy],eax
        mcall   48,5
        mov     [warea.by_x],eax
        mov     [warea.by_y],ebx
        add     ebp,+12
        call    calc_icon_pos
        add     ebp,-12
        mcall   9,I_END,-1
        mov     eax,[I_END+process_information.x_start]
        cmp     eax,[ebp+0]
        jne     @f
        mov     eax,[I_END+process_information.y_start]
        cmp     eax,[ebp+4]
        je      .lp1
    @@: call    get_bg_info
        mcall   67,[ebp+0],[ebp+4],51,51

  .lp1: call    draw_window2
        jmp     still2

  key2:
    mov  eax,2
    int  0x40

    jmp  still2

  button2:
    mov  eax,17
    int  0x40

;    mcall 55,eax, , ,klick_music

    mov  esi,[ebp+8]
          mov  ebx,1
          mov  edi,finfo.path
          call fill_paths
          inc  ebx
       mov  edi,param_str
    mov  dword[finfo+8],param_str
          call fill_paths
          cmp  byte[edi],0
    jne  .no0
    and  dword[finfo+8],0
  .no0:
;    lea  ebx,[ebp+19]
    mov  ebx,finfo
    mov  eax,58
    int  0x40
;    dph  eax
    cmp  eax,1024
    jae  still2
    mcall 55,eax, , ,klick_music
    jmp  still2

klick_music db 0x85,0x60,0x85,0x70,0x85,0x65,0

fill_paths:
        push esi edi
;        dps  '>'
        movzx ecx,byte[str_lens+ebx]
        add  esi,[positions+ebx*4]
        push esi
;  mov  edx,esi
        add  esi,ecx

    .l1:
        dec  esi
        cmp  byte[esi],' '
        jnz   .found
        loop .l1
  pop  esi
  jmp  .noms
    .found:
        lea  ecx,[esi+1]
        pop  esi
        sub  ecx,esi
        rep  movsb
 .noms:
        and  byte[edi],0
;        call debug_outstr
;        dps  <'<',13,10>
        pop  edi esi
        ret

atoi:
        push esi
        xor  eax,eax
        xor  ebx,ebx
  .nxt:
    lodsb
    cmp  al,'0'
    jb   .done
    cmp  al,'9'
    ja   .done
    sub  eax,'0'
    imul ebx,10
    add  ebx,eax
    jmp  .nxt
  .done:
      pop  esi
      mov  eax,ebx
        ret

itoa:
;        mov  esi,[current_icon]
        add  esi,2
    mov ebx,10
    mov ecx,3
  .l0:
    xor edx,edx
    div ebx
    add dl,'0'
    mov [esi],dl
    dec esi
    loop .l0
;    and byte[esi],0
        ret

draw_picture:
    mov  [image],0x3000
    mov  edi,[ebp+8]
    lea  esi,[edi+12]
    call atoi
          cmp  eax,[icon_count]
          ja  toponly.ex
          imul eax,(32*3*32)
          lea  edi,[eax+strip_file+12]
    xor  ebx,ebx
    xor  ecx,ecx
    mov  esi,edi;strip_file+12+(32*3*32)*2

    mov  [pixpos],0
  newb:
    push ebx
    push ecx

    cmp  ebx,10
    jb     yesbpix
    cmp  ebx,42
    jge  yesbpix
    cmp  ecx,31;2
    jg     yesbpix

    push esi
    mov  esi,edi
    add  esi,[pixpos]

no_correction_pixpos:
    add  [pixpos],3
    mov  eax,[esi]
    and  eax,0xffffff

    pop  esi

    cmp eax,0
    je    yesbpix
    cmp eax,0xfffcff ;f5f5f5
    je    yesbpix
    jmp nobpix

  yesbpix:

  stretch:
    cmp   [bgrdrawtype],dword 2
    jne   nostretch
;    mov   eax,[ypos]
    mov   eax,[ebp+4]
    add   eax,ecx
    imul  eax,[bgry]
    cdq
    movzx ebx,word [scrxy]
    div   ebx
    imul  eax,[bgrx]
    push  eax
;    mov   eax,[xpos]
    mov   eax,[ebp+0]
    add   eax,[esp+8]
    imul  eax,[bgrx]
    cdq
    movzx ebx,word [scrxy+2]
    div   ebx
    add   eax,[esp]
    add   esp,4

    jmp   notiled

  nostretch:

    cmp   [bgrdrawtype],dword 1
    jne   notiled
;    mov   eax,[ypos]
    mov   eax,[ebp+4]
    add   eax,ecx
    cdq
    movzx ebx,word [bgrxy]
    div   ebx
    mov   eax,edx
    imul  eax,[bgrx]
    push  eax
;    mov   eax,[xpos]
    mov   eax,[ebp+0]
    add   eax,[esp+8]
    movzx ebx,word [bgrxy+2]
    cdq
    div   ebx
    mov   eax,edx
    add   eax,[esp]
    add   esp,4

  notiled:

    lea  ecx,[eax+eax*2]
    mov  eax,39
    mov  ebx,2
    int  0x40

  nobpix:

    pop  ecx
    pop  ebx

    mov  edx,eax
    mov  eax,[image]
    mov  [eax],edx
    mov  [eax],dl
    inc  eax
    ror  edx,8
    mov  [eax],dl
    inc  eax
    ror  edx,8
    mov  [eax],dl
    inc  eax
    mov  [image],eax
    inc  ebx
    mov  eax,[yw]
    inc  eax
    cmp  ebx,eax
    jnz  newb
    xor  ebx,ebx

    inc  ecx

    mov  eax,[ya]
    add  [pixpos],eax

    cmp  [top],1
    jne  notop
    cmp  ecx,38
    je     toponly

  notop:

    cmp  ecx,52
    jnz  newb

  toponly:

    mov  eax,7
    mov  ebx,0x3000
    mov  ecx,52 shl 16 + 52
    xor  edx,edx
    int  0x40
  .ex:
    mov  [load_pic],0
    ret

draw_text:

    mov  esi,[ebp+8]
    add  esi,3
    push edi
    mov  edi,labelt
    mov  ecx,8
    cld
    rep  movsb
    pop  edi
    mov   eax,labelt
  news2:
    cmp   [eax],byte 33
    jb      founde
    inc   eax
    cmp   eax,labelt+8;11
    jb      news2
   founde:
    sub   eax,labelt
    mov   [tl],eax

    mov   eax,[tl]
    lea   eax,[eax+eax*2]  ; eax *= char_width/2
    shl   eax,16

    mov   ebx,27*65536+42
    sub   ebx,eax

    mov   eax,4
    xor   ecx,ecx         ; black shade of text
    mov   edx,labelt
    mov   esi,[tl]
    add   ebx,1 shl 16      ;*65536+1
    int   0x40
    inc   ebx
    int   0x40
    add   ebx,1 shl 16
    int   0x40
    inc   ebx
    int   0x40
    sub   ebx,1 shl 16
    int   0x40
    dec   ebx
    sub   ebx,1 shl 16
    int   0x40
    sub   ebx,1 shl 16
    dec   ebx
    int   0x40
    dec   ebx
    add   ebx,1 shl 16
    int   0x40
    inc   ebx
    mov   ecx,0xffffff

    int   0x40
    mov   [draw_pic],0
    ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window2:

    mov  eax,12            ; function 12:tell os about windowdraw
    mov  ebx,1               ; 1, start of draw
    int  0x40

                   ; DRAW WINDOW
    xor  eax,eax             ; function 0 : define and draw window
;    mov  ebx,[xpos-2]
    mov  ebx,[ebp+0-2]
;    mov  ecx,[ypos-2]
    mov  ecx,[ebp+4-2]
    add  ebx,[yw]           ; [x start] *65536 + [x size]
    add  ecx,51            ; [y start] *65536 + [y size]
    mov  edx,0x01000000        ; color of work area RRGGBB,8->color gl
    int  0x40

    mov  eax,8      ; button
    mov  ebx,51
    mov  ecx,50
    mov  edx,1+20000000 ; or 0x40000000
    int  0x40

    mov  eax,5
    mov  ebx,1
draw_icon2:
    xchg [load_pic],bl
    test bl,bl
    je     draw_icon_end
    int  0x40
    jmp  draw_icon2
draw_icon_end:

    mov  eax,5
    mov  ebx,1
draw_icon_2:
    xchg [draw_pic],bl
    test bl,bl
    je     draw_icon_end_2
    int  0x40
    jmp  draw_icon_2
draw_icon_end_2:

    mov  eax,9
    mov  ebx,process_table
    mov  ecx,-1
    int  0x40

    call draw_picture
    call draw_text

    mov  eax,12
    mov  ebx,2
    int  0x40

    ret

tl        dd      8
yw        dd     51
ya        dd      0
cur_btn   dd 40

;xpos       dd   15
;ypos       dd  185
draw_pic    db      0
load_pic    db      0
create_thread_event db 0


image          dd  0x3000
;thread_stack  dd  0x5000

;icons dd 0


I_Param:

 icon_data = I_END+0x1400
 process_table = I_END+0x2400

;I_END:

bgrx dd ?
bgry dd ?
param_str rb 31

;//////////////////////////

bgrxy        dd    ?
warea:
 .by_x:
  .right  dw ?
  .left   dw ?
 .by_y:
  .bottom dw ?
  .top    dw ?
scrxy        dd    ?
bgrdrawtype  dd    ?

pixpos dd    ?
top      dd ?
icons dd ?
addr  dd ?
cur_str    dd ?
cur_band   dd ?
sel_icon1  rd 1
icon_count rd 1
gif_file  rb  GIF_SIZE
strip_file rb RAW_SIZE
;I_Param:

; icon_data = I_END+256

I_END:
