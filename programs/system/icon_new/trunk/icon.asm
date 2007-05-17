;
;    ICON APPLICATION
;
;    Compile with FASM for Menuet
;
;    August 12, 2004 - 32-bit ICO format support (mike.dld)
;    March  14, 2007 - rewriten for KolibriOS (heavyiron)

use32

 org    0x0
 db     'MENUET01'              ; 8 byte id
 dd     0x01                    ; header version
 dd     START                   ; start of code
 dd     I_END                   ; size of image
 dd     0x8000                  ; memory for app
 dd     0x8000                  ; esp
 dd     I_PARAM , 0x0           ; I_Param , I_Icon

; params 4 xpos 4 ypos 30 iconfile 30 startfile 10 label
;          +0     +4      +8          +38          +68

include  '..\..\..\macros.inc'
;__DEBUG__ fix 1
;__DEBUG_LEVEL__ fix 1
;include  'debug-fdo.inc'
flipdelay = 7

START:           ; start of execution

    cmp  [I_PARAM],byte 0
    jne  nohalt
    or   eax,-1
    mcall
  nohalt:

    mov  eax,[I_PARAM+0]
    sub  eax,0x01010101
    mov  [xpos],eax
    mov  eax,[I_PARAM+4]
    sub  eax,0x01010101
    mov  [ypos],eax

    mov  esi,I_PARAM+8
    mov  edi,fname
    mov  ecx,30
    cld
    rep  movsb
    call fill_paths
    
;DEBUGF 1,"fname: '%s'\n",fname

    mov  esi,I_PARAM+8+30
    mov  edi,start_file
    mov  ecx,30
    rep  movsb
    call fill_paths

    mov  esi,I_PARAM+8+30+30
    mov  edi,labelt
    mov  ecx,10
    cld
    rep  movsb

    mov  eax,40          ; get also event background change
    mov  ebx,10101b
    mcall


    call get_bg

  red:           ; redraw
    call draw_window

still:

    mov  eax,10          ; wait here for event
    mcall

    cmp  eax,1          ; redraw request ?
    jz  red
    cmp  eax,3          ; button in buffer ?
    jz  button

  check:
    call get_bg
    call draw_icon
    mcall 5,80
    jmp  still

  button:          ; button
    mov  al,17          ; get id
    mcall

    mov  eax,70
    mov  ebx,finfo_start
    mcall

    call flip_icon

    jmp  still

flip_icon:

     mov  eax,1
     call flip
     mov  eax,2
     call flip
     mov  eax,3
     call flip
     mov  eax,4
     call flip
     mov  eax,5
     call flip
     mov  eax,4
     call flip
     mov  eax,3
     call flip
     mov  eax,2
     call flip
     mov  eax,1
     call flip
     mov  eax,0
     call flip

     ret

flip:
     mov  [iconstate],eax
     call get_bg
     call draw_icon
     mov  eax,5
     mov  ebx,flipdelay
     mcall
     ret

fill_paths:
     dec  edi
     mov  ecx,30
     std
     mov  al,' '
     repe scasb
     cld
     mov  byte[edi+2],0
     ret

draw_window:

    mov  eax,12      ; function 12:tell os about windowdraw
    mov  ebx,1      ; 1, start of draw
    mcall

       ; DRAW WINDOW
    xor  eax,eax            ; function 0 : define and draw window
    mov  ebx,[xpos-2]
    mov  ecx,[ypos-2]
    add  ebx,[yw]           ; [x start] *65536 + [x size]
    add  ecx,67             ; [y start] *65536 + [y size]
    mov  edx,0x01000000     ; color of work area RRGGBB,8->color gl
    mcall

    mov  eax,8      ; button
    mov  ebx,51
    mov  ecx,67
    mov  edx,0x60000001
    mcall

    call draw_icon

    mov  eax,12
    mov  ebx,2
    mcall

    ret

get_bg:

    mov  eax,14
    mcall
    add  eax,0x00010001
    mov  [scrxy],eax

    mov  eax,39        ; get background type
    mov  ebx,4
    mcall
    mov  [bgrdrawtype],eax

    mov  eax,39        ; get background size
    mov  ebx,1
    mcall
    mov  [bgrxy],eax

    mov  eax,70
    mov  ebx,finfo
    mcall

    mov  [itype],0
    cmp  word[I_END+256],'BM'
    je  @f
    inc  [itype]
  @@:

    mov  ebx,[yw]     
    xor  ecx,ecx            ; 10608 = 52*68*3 - bg image
    mov  esi,I_END+256+9662 ; 9662 - icon file image
    mov  edi,51*3

  newb:

    push ebx ecx

  yesbpix:

    cmp   [bgrdrawtype],dword 2
    jne   nostretch

    mov   eax,[ypos]
    add   eax,ecx
    xor   edx,edx
    movzx ebx,word [bgrxy]
    mul   ebx
    xor   edx,edx
    movzx ebx,word [scrxy]
    div   ebx
    xor   edx,edx
    movzx ebx,word [bgrxy+2]
    mul   ebx
    push  eax

    mov   eax,[xpos]
    add   eax,[esp+8]
    xor   edx,edx
    movzx ebx,word [bgrxy+2]
    mul   ebx
    xor   edx,edx
    movzx ebx,word [scrxy+2]
    div   ebx
    add   eax,[esp]
    add   esp,4

  nostretch:

    cmp   [bgrdrawtype],dword 1
    jne   notiled

    mov  eax,[ypos]
    add  eax,ecx
    xor  edx,edx
    movzx ebx,word [bgrxy]
    div  ebx
    mov  eax,edx
    movzx  ebx,word [bgrxy+2]
    xor  edx,edx
    mul  ebx
    push eax

    mov  eax,[xpos]
    add  eax,[esp+8]
    movzx ebx,word [bgrxy+2]
    xor  edx,edx
    div  ebx
    mov  eax,edx
    add  eax,[esp]
    add  esp,4

  notiled:

    lea  ecx,[eax+eax*2]

    mov  eax,39
    mov  ebx,2

    mcall

  nobpix:

    pop  ecx ebx

    mov  [esi+edi+0],al
    mov  [esi+edi+1],ah
    shr  eax,16
    mov  [esi+edi+2],al
    sub  edi,3

    dec  ebx
    jge  newb
    mov  ebx,[yw]

    add  esi,52*3
    mov  edi,51*3
    inc  ecx
    cmp  ecx,68
    jne  newb

;*****************************************************************************

    mov  esi,I_END+256+9662+10608-17*52*3+3 ;! 54+32*3*33-3
    mov  eax,[iconstate]
    mov  eax,[add_table0+eax*4]
    add  esi,eax
    mov  edi,I_END+256+62
    cmp  [itype],0
    jne  @f
    mov  edi,I_END+256+54
  @@:
    xor  ebp,ebp
    mov  [pixl],0
  newp:

    virtual at edi
      r db ?
      g db ?
      b db ?
      a db ?
    end virtual
    virtual at esi+ebp
      ar db ?
      ag db ?
      ab db ?
    end virtual

    movzx ecx,[a]

    push  ebp
    cmp   [iconstate],3
    jb   @f
    neg   ebp
  @@:

    cmp  [itype],0
    jne  @f
    mov  eax,[edi]
    and  eax,$00FFFFFF
    jnz  @f
    jmp  no_transp
  @@:

    movzx eax,[r]
    cmp   [itype],0
    je   @f
    movzx ebx,byte[ar]
    sub   eax,ebx
    imul  eax,ecx
    xor   edx,edx
    or   ebx,$0FF
    div   ebx
    movzx ebx,[ar]
    add   eax,ebx
  @@:
    mov  [esi+ebp+0],al

    movzx eax,[g]
    cmp   [itype],0
    je   @f
    movzx ebx,[ag]
    sub   eax,ebx
    imul  eax,ecx
    xor   edx,edx
    or   ebx,$0FF
    div   ebx
    movzx ebx,[ag]
    add   eax,ebx
  @@:
    mov  [esi+ebp+1],al

    movzx eax,[b]
    cmp   [itype],0
    je   @f
    movzx ebx,[ab]
    sub   eax,ebx
    imul  eax,ecx
    xor   edx,edx
    or   ebx,$0FF
    div   ebx
    movzx ebx,[ab]
    add   eax,ebx
  @@:
    mov  [esi+ebp+2],al

  no_transp:

    pop   ebp

    movzx eax,[itype]
    imul  eax,6
    add   eax,[iconstate]
    push  eax
    mov   eax,[add_table1+eax*4]
    add   edi,eax

    add  ebp,3
    pop  eax
    mov  eax,[add_table2+eax*4]
    add  [pixl],eax
    cmp  [pixl],48
    jl  newp
    xor  ebp,ebp
    mov  [pixl],0

    sub  esi,52*3
    cmp  esi,I_END+256+9662+52*4*3
    jge  newp

;*****************************************************************************

    ret

draw_picture:
    mov  eax,7
    mov  ebx,I_END+256+9662
    mov  ecx,52 shl 16 + 68
    xor  edx,edx
    mcall
    ret

draw_icon:
    call draw_picture
    call draw_text
    ret

draw_text:

    mov   eax,labelt       ;text_length
  news:
    cmp   [eax],byte 40
    jb   founde
    inc   eax
    cmp   eax,labelt+11
    jb   news
   founde:
    sub   eax,labelt

    lea   eax,[eax+eax*2]  ; eax *= char_width/2
    shl   eax,16
    mov   ebx,26*65536+58
    sub   ebx,eax
    movzx ecx,byte [I_PARAM+8+30+30+10]
    shl   ecx,16
    add   ebx,ecx

; replaced - delete commented lines below if you like that style
    mov   eax,4          ; white text

    xor   ecx,ecx
    mov   edx,labelt
    mov   esi,labellen-labelt
    add   ebx,1 shl 16      ;*65536+1
    mcall
    inc   ebx
    mcall
    add   ebx,1 shl 16
    mcall
    inc   ebx
    mcall
    sub   ebx,1 shl 16
    mcall
    sub   ebx,1 shl 16 +1
    mcall
    sub   ebx,1 shl 16 + 1
    mcall
    add   ebx,1 shl 16 - 1
    mcall
    inc   ebx
    mov   ecx,0xffffff
    mcall

    ;xor   ecx,ecx        ; black shade of text
    ;mov   edx,labelt
    ;mov   esi,labellen-labelt
    ;add   ebx,1*65536+1
    ;mcall
    ;sub   ebx,1*65536+1
    ;mov   ecx,0xffffff
    ;mcall

    ret


; DATA AREA

itype       db 0

align 4

tl          dd  2
yw          dd  51

xpos        dd  15
ypos        dd  185

bgrxy       dd  0x0
scrxy       dd  0x0
bgrdrawtype dd  0x0

iconstate   dd 0

add_table0  dd (24-6*4)*3,(24-6*2)*3,(24-6*1)*3,\
               (24+6*1)*3,(24+6*2)*3,(24+6*4)*3

add_table1  dd 3,6,12,12,6,3
            dd 4,8,16,16,8,4

add_table2  dd 1,2,4,4,2,1
            dd 1,2,4,4,2,1

finfo_start:
            dd 7
            dd 0
            dd 0
            dd 0
            dd 0
            db 0
            dd start_file

finfo:
            dd 0
            dd 0
            dd 0
            dd 9662
            dd I_END+256
            db 0
            dd fname

start_file  rb  30
fname       rb  30

labelt:
            rb  10
labellen:

pixl dd ?

;include_debug_strings
I_PARAM:

I_END:
