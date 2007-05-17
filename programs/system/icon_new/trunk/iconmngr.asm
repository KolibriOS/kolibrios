;
;   DESKTOP ICON MANAGER
;
;   Compile with FASM for Menuet
;

use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x4000                  ; memory for app
               dd     0x4000                  ; esp
               dd     I_Param , 0x0           ; I_Param , I_Icon

include  '..\..\..\macros.inc'
include  'lang.inc'

START:                          ; start of execution

    call load_icon_list

    call check_parameters

  red:                          ; redraw
    call draw_window            ; at first, draw the window

still:

    mov  eax,10                 ; wait here for event
    mcall

    dec  eax                    ; redraw request ?
    jz   red
    dec  eax                    ; key in buffer ?
    jz   key

  button:                       ; button
    mov  al,17                  ; get id
    mcall

    shr  eax,8

    cmp  eax,1                  ; button id=1 ?
    jne  noclose
    or   eax,-1                 ; close this program
    mcall
  noclose:

    cmp  eax,11
    jb   no_str
    cmp  eax,13
    jg   no_str
    call read_string
    jmp  still
  no_str:


    cmp  eax,21                 ; apply changes
    jne  no_apply

    ; (1) save list

    mov  eax,70
    mov  ebx,finfo
    mov  dword[ebx],2
    mov  edx,78
    imul edx,dword [icons]
    mov  dword[ebx+12],edx
    mcall


    ; (2) terminate all icons

    mov  esi,1
  newread:
    inc  esi
    mov  eax,9
    mov  ebx,I_END
    mov  ecx,esi
    mcall
    cmp  esi,eax
    jg   all_terminated

    cmp  [I_END+10],dword '@ICO'
    jne  newread

    mov  eax,18
    mov  ebx,2
    mov  ecx,esi
    mcall

    mov  esi,1

    jmp  newread

   all_terminated:

   apply_changes:

    ; (3)  start icons from icon_data

    mov  edi,[icons]
    mov  esi,icon_data
  start_new:

    push edi
    push esi

    movzx eax,byte [esi]    ; x position
    cmp  eax,'E'
    jg   no_left
    sub  eax,65
    imul eax,70
    add  eax,15
    jmp  x_done
  no_left:
    sub  eax,65
    mov  ebx,9
    sub  ebx,eax
    imul ebx,70
    push ebx
    mov  eax,14
    mcall
    pop  ebx
    shr  eax,16
    sub  eax,51+15
    sub  eax,ebx
  x_done:
    add  eax,0x01010101
    mov  [icon_start_parameters],eax

    movzx eax,byte [esi+1]  ; y position
    cmp  eax,'E'
    jg   no_up
    sub  eax,65
    imul eax,80
    add  eax,12
    jmp  y_done
  no_up:
    sub  eax,65
    mov  ebx,9
    sub  ebx,eax
    imul ebx,80
    push ebx
    mov  eax,14
    mcall
    pop  ebx
    and  eax,0xffff
    sub  eax,-1+20
    sub  eax,ebx
  y_done:
    add  eax,0x01010101
    mov  [icon_start_parameters+4],eax

    mov  esi,[esp]          ; icon picture
    add  esi,3
    mov  edi,icon_start_parameters+8
    mov  ecx,30
    cld
    rep  movsb

    mov  esi,[esp]          ; icon application
    add  esi,34
    mov  edi,icon_start_parameters+8+30
    mov  ecx,30
    cld
    rep  movsb

    mov  esi,[esp]          ; icon text
    add  esi,65
    mov  edi,icon_start_parameters+8+30+30
    mov  ecx,10
    cld
    rep  movsb

    mov byte[edi],0          ; ASCII -> ASCIIZ

    mov  eax,70
    mov  ebx,finfo_start
    mcall

    pop  esi edi

    add  esi,76+2

    dec  edi
    jnz  start_new

    cmp  [I_Param],byte 0
    je   still

    or   eax,-1
    mcall

  no_apply:


    cmp  eax,22                 ; user pressed the 'add icon' button
    jne  no_add_icon

    mov  eax,13
    mov  ebx,19*65536+260
    mov  ecx,225*65536+10
    mov  edx,0xffffff
    mcall
    mov  eax,4
    mov  ebx,19*65536+225
    mov  ecx,0xc0ff0000
    mov  edx,add_text
    mov  edi,0xffffff
    mcall

    mov  eax,10
    mcall
    cmp  eax,3
    jne  still
    mov  al,17
    mcall
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
    imul edi,76+2
    add  edi,icon_data

    mov  [current_icon],edi

    mov  esi,icon_default
    mov  ecx,76+2
    cld
    rep  movsb

  no_f:

    call draw_window

    jmp  still

  no_add_icon:


    cmp  eax,23                     ; user pressed the remove icon button
    jne  no_remove_icon

    mov  eax,13
    mov  ebx,19*65536+260
    mov  ecx,225*65536+10
    mov  edx,0xffffff
    mcall
    mov  eax,4
    mov  ebx,19*65536+225
    mov  ecx,0xc0ff0000
    mov  edx,rem_text
    mov  edi,0xffffff
    mcall

    mov  eax,10
    mcall
    cmp  eax,3
    jne  no_found
    mov  al,17
    mcall
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
    mov  edi,76+2
    imul edi,[icons]
    add  edi,icon_data
  news:
    cmp  word [esi],ax
    je   foundi
    add  esi,76+2
    cmp  esi,edi
    jb   news
    jmp  no_found

  foundi:

    mov  ecx,edi
    sub  ecx,esi

    mov  edi,esi
    add  esi,76+2

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
    add  esi,76+2
    loop findl1
    jmp  still

   foundl1:

    mov  [current_icon],esi

    call print_strings

    jmp  still

  no_on_screen_button:


    jmp  still


  key:                          ; key
    mov  al,2                   ; just read it and ignore
    mcall
    jmp  still

current_icon dd icon_data


print_strings:

    pusha

    mov  eax,13              ; clear text area
    mov  ebx,95*65536+180
    mov  ecx,253*65536+40
    mov  edx,0xffffff
    mcall

    mov  eax,4               ; icon text
    mov  ebx,95*65536+253
    mov  ecx,0x000000
    mov  edx,[current_icon]
    add  edx,65
    mov  esi,10
    mcall

    ;mov  eax,4               ; icon application
    add  ebx,14
    mov  edx,[current_icon]
    add  edx,34
    mov  esi,30
    mcall

    ;mov  eax,4               ; icon file
    add  ebx,14
    mov  edx,[current_icon]
    add  edx,3
    mov  esi,30
    mcall

    popa

    ret


load_icon_list:

    ;pusha

    mov   eax,70
    mov   ebx,finfo
    mov   dword[ebx],0
    mov   dword[ebx+12],4096   ; max size of icons.dat (in current format) is 4 kb
    mcall

    mov   eax,ebx
    add   eax,2
    xor   edx,edx
    mov   ebx,76+2
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
    add   esi,76+2
    loop  ldl1

    ret


check_parameters:

    cmp   [I_Param],dword 'BOOT'
    je    chpl1
    ret

   chpl1:
    mov   eax,21
    jmp   apply_changes


positions dd 65,34,3

read_string:

    sub  eax,11
    shl  eax,2
    add  eax,positions
    mov  eax,[eax]

    mov  esi,[current_icon]
    add  esi,eax
    mov  [addr],esi

    mov  edi,[addr]
    mov  eax,'_'
    mov  ecx,30
    cld
    rep  stosb

    call print_strings

    mov  edi,[addr]
  f11:
    mov  eax,10
    mcall
    cmp  eax,2
    jz   fbu
    jmp  rs_done
  fbu:
    mov  eax,2
    mcall
    shr  eax,8
    cmp  eax,13
    je   rs_done
    cmp  eax,8
    jnz  nobsl
    cmp  edi,[addr]
    jz   f11
    dec  edi
    mov  [edi],byte ' '
    call print_strings
    jmp  f11
  nobsl:
    cmp  eax,31
    jbe  f11
    mov  [edi],al
    call print_strings

    add  edi,1
    mov  esi,[addr]
    add  esi,30
    cmp  esi,edi
    jnz  f11

   rs_done:

    mov  ecx,[addr]
    add  ecx,30
    sub  ecx,edi
    mov  eax,32
    cld
    rep  stosb

    call print_strings

    ret



;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    xor  eax,eax
    mov  ebx,210*65536+300
    mov  ecx,30*65536+390
    mov  edx,0x33ffffff
    mov  edi,title
    mcall

    mov  eax,13                    ; WINDOW AREA
    mov  ebx,15*65536+260
    mov  ecx,10*65536+200
    mov  edx,0x3366cc
    mcall

    mov  eax,38                    ; VERTICAL LINE ON WINDOW AREA
    mov  ebx,145*65536+145
    mov  ecx,10*65536+235
    mov  edx,0xffffff
    mcall

    ;mov  eax,38                    ; HOROZONTAL LINE ON WINDOW AREA
    mov  ebx,15*65536+280
    mov  ecx,110*65536+110
    mov  edx,0xffffff
    mcall

    mov  eax,8                     ; TEXT ENTER BUTTONS
    mov  ebx,15*65536+72
    mov  ecx,250*65536+13
    mov  edx,11
    mov  esi,[bcolor]
    mcall
    inc  edx
    add  ecx,14*65536
    mcall
    inc  edx
    add  ecx,14*65536
    mcall

    ;mov  eax,8                     ; APPLY AND SAVE CHANGES BUTTON
    mov  ebx,15*65536+259
    mov  ecx,304*65536+15
    mov  edx,21
    mov  esi,[bcolor]
    mcall

    ;mov  eax,8                     ; ADD ICON BUTTON
    mov  ebx,15*65536+129
    add  ecx,14*2*65536
    inc  edx
    mcall

    ;mov  eax,8                     ; REMOVE ICON BUTTON
    add  ebx,130*65536
    inc  edx
    mcall

    xor  eax,eax                    ; DRAW BUTTONS ON WINDOW AREA
    mov  ebx,15*65536+25
    mov  ecx,10*65536+19
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
    mcall
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
    mov  bx,15
    ror  ebx,16
    add  ecx,20*65536

    inc  ah
    cmp  ah,9
    jbe  newbline

    mov  ebx,24*65536+225
    mov  ecx,0xffffff
    mov  edx,text
    mov  esi,36
    mov  eax,4
  newline:
    mov  ecx,[edx]
    add  edx,4
    mcall
    add  ebx,14
    add  edx,36
    cmp  [edx],byte 'x'
    jne  newline

    call print_strings

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret


; DATA AREA

bcolor dd 0x335599

icon_table:

    times 4  db  'xxxx  xxxx'
    times 2  db  '          '
    times 3  db  'xxxx  xxxx'
    times 1  db  '          '

icons_reserved:

    times 10  db  '          '


text:
    db 0,0,0,0,         'Click on icon position to edit      '
    db 0,0,0,0,         '                                    '
    db 255,255,255,0  , 'Icon text                           '
    db 255,255,255,0  , 'Icon app                            '
    db 255,255,255,0  , 'Icon file                           '
    db 0,0,0,0,         '                                    '
    db 255,255,255,0,   '       Save and apply all changes   '
    db 0,0,0,0,         '                                    '
    db 255,255,255,0,   '     Add icon            Remove icon'
    db                  'x' ; <- End marker, dont delete


title    db 'Icon manager',0

icons dd 0

addr  dd 0
ya    dd 0

add_text    db 'Press button of unused icon position',0

rem_text    db 'Press button of used icon           ',0

finfo_start:
          dd 7
          dd 0
          dd icon_start_parameters
          dd 0
          dd 0
          db 0
          dd icon_path

icon_path db '/rd/1/@ICON',0

finfo:
          dd 0
          dd 0
          dd 0
          dd 0
          dd icon_data
          db 0
          dd icon_list

icon_list db '/rd/1/icons.dat',0

icon_start_parameters:
      db   25,1,1,1
      db   35,1,1,1
      db   '/rd/1/icons/fs.ico',0
      rb   12
      db   '/rd/1/kfar',0
      rb   20
      db   'KFAR      '

icon_default:
      db   'AA-/rd/1/icons/fs.ico            -/rd/1/kfar                    -KFAR      *',13,10

icon_data:   ; data length 76+2

rb 4096

I_Param:

I_END:
