;********************************
;*                              *
;*     DESKTOP ICON MANAGER     *
;*                              *
;*  Compile with flat assembler *
;*                              *
;********************************
;  22.02.05 was modified for work with new multi-thread ICON.
;******************************************************************************
  use32
  org    0x0
  db     'MENUET01'     ; 8 byte id
  dd     0x01           ; header version
  dd     START          ; start of code
  dd     I_END          ; size of image
  dd     0x2000        ; memory for app
  dd     0x2000        ; esp
  dd     I_Param , 0x0  ; I_Param , I_Icon
include  'lang.inc'
include  'macros.inc'
;******************************************************************************

START:                       ; start of execution

    call load_icon_list

    call check_parameters

 red:
    call draw_window         ; at first, draw the window

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
    jne  noclose
    mov  eax,-1              ; close this program
    int  0x40
  noclose:

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

    mov  eax,33
    mov  ebx,icon_lst
    mov  ecx,icon_data
    mov  edx,52
    imul edx,dword [icons]
    mov  esi,0
    int  0x40


    ; (2) terminate all icons

    mov  esi,1
   newread:
    inc  esi
    mov  eax,9
    mov  ebx,I_END
    mov  ecx,esi
    int  0x40
    cmp  esi,eax
    jg   all_terminated

    cmp  [I_END+10],dword 'ICON'
    jne  newread
    cmp  [I_END+14],dword '    '
    jne  newread

    mov  eax,18
    mov  ebx,2
    mov  ecx,esi
    int  0x40

;    mov  eax,5
;    mov  ebx,5
;    int  0x40

    mov  esi,1

    jmp  newread

   all_terminated:

   apply_changes:

    mov  eax,19
    mov  ebx,icon_name
    mov  ecx,icon_start_parameters
    int  0x40

;    mov  eax,5
;    mov  ebx,3
;    int  0x40

    jmp   still

;    mov  eax,-1
;    int  0x40

  no_apply:


    cmp  eax,22                 ; user pressed the 'add icon' button
    jne  no_add_icon

    mov  eax,13
    mov  ebx,24*65536+270
    mov  ecx,250*65536+10
    mov  edx,0xffffff
    int  0x40
    mov  eax,4
    mov  ebx,24*65536+250
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
    imul edi,52
    add  edi,icon_data

    mov  [current_icon],edi

    mov  esi,icon_default
    mov  ecx,52
    cld
    rep  movsb

  no_f:

    call draw_window

    jmp  still

  no_add_icon:


    cmp  eax,23                     ; user pressed the remove icon button
    jne  no_remove_icon

    mov  eax,13
    mov  ebx,24*65536+270
    mov  ecx,250*65536+10
    mov  edx,0xffffff
    int  0x40
    mov  eax,4
    mov  ebx,24*65536+250
    mov  ecx,0xff0000
    mov  edx,rem_text
    mov  esi,rem_text_len-rem_text
    int  0x40

    mov  eax,10
    int  0x40
    cmp  eax,3
    jne  no_found
    mov  eax,17
    int  0x40
    shr  eax,8
    cmp  eax,40
    jb   no_found
    sub  eax,40

    xor  edx,edx
    mov  ebx,16
    div  ebx
    imul eax,10
    add  eax,edx

    mov  ebx,eax
    add  ebx,icons_reserved
    cmp  [ebx],byte 'x'
    jne  no_found
    mov  [ebx],byte ' '

    xor  edx,edx
    mov  ebx,10
    div  ebx
    shl  eax,8
    mov  al,dl

    add  eax,65*256+65

    mov  esi,icon_data
    mov  edi,52
    imul edi,[icons]
    add  edi,icon_data
  news:
    cmp  word [esi],ax
    je   foundi
    add  esi,52
    cmp  esi,edi
    jb   news
    jmp  no_found

  foundi:

    mov  ecx,edi
    sub  ecx,esi

    mov  edi,esi
    add  esi,52

    cld
    rep  movsb

    dec  [icons]

    mov  eax,icon_data
    mov  [current_icon],eax

  no_found:

    call draw_window

    jmp  still



  no_remove_icon:


    cmp  eax,40                 ; user pressed button for icon position
    jb   no_on_screen_button

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
    add  esi,50+2
    loop findl1
    jmp  still

   foundl1:

    mov  [current_icon],esi

    call print_strings

    jmp  still

  no_on_screen_button:


    jmp  still


current_icon dd icon_data


print_strings:

    pusha

    mov  eax,13              ; clear text area
    mov  ebx,100*65536+100
    mov  ecx,278*65536+40
    mov  edx,0xffffff
    int  0x40

    mov  eax,4               ; icon text
    mov  ebx,100*65536+278
    mov  ecx,0x000000
    mov  edx,[current_icon]
    add  edx,33
    mov  esi,12
    int  0x40

    mov  eax,4               ; icon application
    add  ebx,14
    mov  edx,[current_icon]
    add  edx,19
    mov  esi,12
    int  0x40

    mov  eax,4               ; icon bmp
    add  ebx,14
    mov  edx,[current_icon]
    add  edx,5
    mov  esi,12
    int  0x40

    popa

    ret


icon_lst db 'ICON    LST'

load_icon_list:


    pusha

    mov   eax,6
    mov   ebx,icon_lst
    mov   ecx,0
    mov   edx,-1
    mov   esi,icon_data
    int   0x40

    add   eax,10
    xor   edx,edx
    mov   ebx,52
    div   ebx
    mov   [icons],eax

    mov   edi,icons_reserved   ; clear reserved area
    mov   eax,32
    mov   ecx,10*10
    cld
    rep   stosb

    mov   ecx,[icons]          ; set used icons to reserved area
    mov   esi,icon_data
    cld
  ldl1:
    movzx ebx,byte [esi+1]
    sub   ebx,65
    imul  ebx,10
    movzx eax,byte [esi]
    add   ebx,eax
    sub   ebx,65
    add   ebx,icons_reserved
    mov   [ebx],byte 'x'
    add   esi,50+2
    loop  ldl1

    popa

    ret


check_parameters:

    cmp   [I_Param],dword 'BOOT'
    je    chpl1
    ret
   chpl1:

    mov   eax,21
    jmp   apply_changes


positions dd 33,19,5

read_string:

    pusha

    sub  eax,11
    shl  eax,2
    add  eax,positions
    mov  eax,[eax]

    mov  esi,[current_icon]
    add  esi,eax
    mov  [addr],esi

    mov  edi,[addr]
    mov  eax,'_'
    mov  ecx,12
    cld
    rep  stosb

    call print_strings

    mov  edi,[addr]
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
    cmp  eax,97
    jb   keyok
    sub  eax,32
  keyok:
    mov  [edi],al
    call print_strings

    add  edi,1
    mov  esi,[addr]
    add  esi,12
    cmp  esi,edi
    jnz  f11

   rs_done:

    mov  ecx,[addr]
    add  ecx,12
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
    mov  ecx,30*65536+390
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
    mov  ecx,275*65536+13
    mov  edx,11
    mov  esi,[bcolor]
    int  0x40
    inc  edx
    add  ecx,14*65536
    mov  eax,8
    int  0x40
    inc  edx
    add  ecx,14*65536
    mov  eax,8
    int  0x40

    mov  eax,8                     ; APPLY AND SAVE CHANGES BUTTON
    mov  ebx,20*65536+259
    mov  ecx,329*65536+15
    mov  edx,21
    mov  esi,[bcolor]
    int  0x40

    mov  eax,8                     ; ADD ICON BUTTON
    mov  ebx,20*65536+129
    add  ecx,14*2*65536
    inc  edx
    int  0x40

    mov  eax,8                     ; REMOVE ICON BUTTON
    add  ebx,130*65536
    inc  edx
    int  0x40

    mov  eax,0                     ; DRAW BUTTONS ON WINDOW AREA
    mov  ebx,20*65536+25
    mov  ecx,35*65536+19
    mov  edi,icon_table
    mov  edx,40
   newbline:

    cmp  [edi],byte 'x'
    jne  no_button

    mov  esi,0x5577cc
    cmp  [edi+100],byte 'x'
    jne  nores
    mov  esi,0xcc5555
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
    cmp  ah,9
    jbe  newbline

    mov  ebx,24*65536+250
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

    call print_strings

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


; DATA AREA


str1   db   '                   '
str2   db   '                   '

bcolor dd 0x335599

icon_table:

    times 4  db  'xxxx  xxxx'
    times 2  db  '          '
    times 1  db  '          '
    times 2  db  'xxxx  xxxx'
    times 1  db  '          '

icons_reserved:

    times 10 db  '          '

if lang eq ru
  text:
      db 0,0,0,0,         '€†’… € ‡–  „‹ …„€’‚€   '
      db 0,0,0,0,         '                                               '
      db 255,255,255,0,   '   ’…‘’                                       '
      db 255,255,255,0,   ' ƒ€€                                     '
      db 255,255,255,0,   ' ‚ ”€‰‹                                      '
      db 0,0,0,0,         '                                               '
      db 255,255,255,0,   '                 …’                     '
      db 0,0,0,0,         '                                               '
      db 255,255,255,0,   '      „€‚’              “„€‹’            '
      db                  'x' ; <- END MARKER, DONT DELETE

  labelt:
       db ' αβΰ®©ª  ΰ ΅®η¥£® αβ®« '
  labellen:
else
  text:
      db 0,0,0,0,         'CLICK ON THE UNUSED ICON POSITION TO EDIT      '
      db 0,0,0,0,         '                                               '
      db 255,255,255,0,   '   ’…‘’                                       '
      db 255,255,255,0,   ' ƒ€€                                     '
      db 255,255,255,0,   ' ‚ ”€‰‹                                      '
      db 0,0,0,0,         '                                               '
      db 255,255,255,0,   '                 …’                     '
      db 0,0,0,0,         '                                               '
      db 255,255,255,0,   '      „€‚’              “„€‹’            '
      db                  'x' ; <- END MARKER, DONT DELETE

  labelt:
       db ' αβΰ®©ª  ΰ ΅®η¥£® αβ®« '
  labellen:
end if

icons dd 0

addr  dd 0
ya    dd 0

add_text db '€†’… € ‡– …‘‹‡“…‰ '
add_text_len:

rem_text db '€†’… € ‡– ‘‹‡“…‰ '
rem_text_len:

icon_name:
      db 'ICON       '

icon_start_parameters:
      db   25,1,1,1
      db   35,1,1,1
      db   'WRITE   BMP'
      db   'EDITOR     '
      db   'EDITOR  ',0,0

icon_default:
      db   'AA - TIP.BMP     - SETUP       - SETUP           *',13,10

I_Param:

 icon_data = I_END+256

I_END:
