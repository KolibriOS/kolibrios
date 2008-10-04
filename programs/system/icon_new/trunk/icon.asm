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

flipdelay = 5

START:           ; start of execution

    cmp   [I_PARAM],byte 0
    jne   nohalt
    mcall -1
  
 nohalt:
    mov   eax,[I_PARAM+0]
    sub   eax,0x01010101
    mov   [xpos],eax
    mov   eax,[I_PARAM+4]
    sub   eax,0x01010101
    mov   [ypos],eax

    mov   esi,I_PARAM+8
    mov   edi,fname
    mov   ecx,30
    cld
    rep   movsb
    call  fill_paths
    
;DEBUGF 1,"fname: '%s'\n",fname

    mov   esi,I_PARAM+8+30
    mov   edi,start_file
    mov   ecx,30
    cld
    rep   movsb
    call  fill_paths

    mov   esi,I_PARAM+8+30+30
    mov   edi,labelt
    mov   ecx,10
    cld
    rep   movsb

    mcall 40,110001b

    mcall 37,4,hand,1
    mov   ecx,eax
    mcall 37,5

red:
    call  get_bg
    call  draw_window

still:

    mcall  10          ; wait here for event

    cmp   eax,1          ; redraw request ?
    jz    red
    cmp   eax,6
    jz    mouse

    call  get_bg
    call  draw_icon
    mcall 5,80
    jmp   still


  mouse:

    mcall 9,process,-1
    xor   ebx,ebx
    mov   bx,[process.window_stack_position]
    cmp   eax,ebx              ;check is this process is active
    jne   still

    cmp   [mouse_pressed],1
    je    @f

    mcall 37,2
    test  eax,1
    jz    still

    mov   [icon_moved],0

    mcall 37,0
    mov   ebx,eax
    shr   eax,16             ;   eax= abs x
    and   ebx,0xffff         ;   ebx= abs y

    mov   [xmouse_old],eax   ; saving mouse coordinates
    mov   [ymouse_old],ebx

    cmp   eax,[process.box.left]    ; check is mouse in icon area
    jl    still
    sub   eax,[process.box.left]
    cmp   eax,[process.box.width]
    ja    still

    cmp   ebx,[process.box.top]
    jl    still
    sub   ebx,[process.box.top]
    cmp   ebx,[process.box.height]
    ja    still

    mov   [xmouse_rel],eax   ; saving relative coordinates
    mov   [ymouse_rel],ebx

    mov   [mouse_pressed],1

    jmp   still

  @@:
    mcall 37,2
    test  eax,1
    jnz   @F

    mov   [mouse_pressed],0

  @@:
    mcall 37,0
    mov   ecx,eax
    shr   eax,16             ;   eax= abs x
    and   ecx,0xffff         ;   ecx= abs y
    push  eax ecx

    cmp   [icon_moved],1
    je    move

    add   eax,2
    cmp   eax,[xmouse_old]
    jle   move
    sub   eax,4
    cmp   eax,[xmouse_old]
    jae   move

    add   ecx,2
    cmp   ecx,[ymouse_old]
    jle   move
    sub   ecx,4
    cmp   ecx,[ymouse_old]
    jae   move

    cmp   [mouse_pressed],1
    je    still

    mcall 70,finfo_start
    call  flip_icon
    jmp   still

 move:
    mov   [icon_moved],1
    pop   ecx ebx
    sub   ebx,[xmouse_rel]   ;   ebx=new_x
    sub   ecx,[ymouse_rel]   ;   ecx=new_y
    mov   [xpos],ebx
    mov   [ypos],ecx

    mcall 67,,,-1,-1    ;   move

    jmp   still

fill_paths:
     dec   edi
     mov   ecx,30
     std
     mov   al,' '
     repe  scasb
     cld
     mov   byte[edi+2],0
     ret

flip_icon:

     mov   eax,1
     call  flip
     inc   eax
     call  flip
     inc   eax
     call  flip
     inc   eax
     call  flip
     inc   eax
     call  flip
     dec   eax
     call  flip
     dec   eax
     call  flip
     dec   eax
     call  flip
     dec   eax
     call  flip
     xor   eax,eax
     call  flip

     ret

flip:
     push  eax
     mov   [iconstate],eax
     call  get_bg
     call  draw_icon
     mcall 5,flipdelay
     pop   eax
     ret


draw_window:

     mcall 12,1             ; function 12,1 - tell os about start of draw window

     xor   eax,eax          ; function 0 : define and draw window
     mov   ebx,[xpos-2]
     add   ebx,51           ; [x start] *65536 + [x size]
     mov   ecx,[ypos-2]
     add   ecx,67           ; [y start] *65536 + [y size]
     mov   edx,0x01000000
     mov   esi,0x01000000
     mcall

     call  draw_icon

     mcall 12,2             ; function 12,2 - tell os about end of draw window

     ret

get_bg:

    mcall 61,1
    mov  [scrxy],eax

    mcall 39,4             ; get background type
    mov  [bgrdrawtype],eax

    mcall 39,1             ; get background size
    mov  [bgrxy],eax

    mcall 70,finfo

    mov  [itype],0
    cmp  word[I_END+256],'BM'
    je  @f
    inc  [itype]
  @@:

    mov  ebx,51     
    xor  ecx,ecx            ; 10608 = 52*68*3 - bg image
    mov  esi,I_END+256+9662 ; 9662 - icon file image
    mov  edi,51*3

  newb:

    push ebx ecx

  yesbpix:

    cmp   [bgrdrawtype],2
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

    cmp   [bgrdrawtype],1
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
    mcall 39,2

  nobpix:

    pop  ecx ebx

    mov  [esi+edi+0],al
    mov  [esi+edi+1],ah
    shr  eax,16
    mov  [esi+edi+2],al
    sub  edi,3

    dec  ebx
    jge  newb
    mov  ebx,51

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
    mcall 7,I_END+256+9662,52 shl 16+68,0
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

; black shade of text
; uncomment lines below if you like that style
    
    xor   ecx,ecx
    mov   edx,labelt
    mov   esi,labellen-labelt
    add   ebx,1 shl 16
    mcall 4
    inc   ebx
    mcall
    add   ebx,1 shl 16
    mcall
    inc   ebx
    mcall
    sub   ebx,1 shl 16
    mcall
    sub   ebx,1*65536+1
    mcall
    sub   ebx,1*65536+1
    mcall
    add   ebx,1*65536-1
    mcall
    inc   ebx
    or    ecx,0xffffff
    mcall

    ;xor   ecx,ecx        
    ;mov   edx,labelt
    ;mov   esi,labellen-labelt
    ;mcall 4
    ;sub   ebx,1*65536+1
    ;or    ecx,0xffffff
    ;mcall

    ret


; DATA AREA

itype       db 0

tl          dd  2


bgrxy       dd  0x0
scrxy       dd  0x0
bgrdrawtype dd  0x0

hand file 'hand.cur'

icon_moved  dd 0

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

xpos        dd  ?
ypos        dd  ?

mouse_pressed  dd ?

xmouse_rel  dd ?
ymouse_rel  dd ?

xmouse_old  dd ?
ymouse_old  dd ?
processes   dd ?
pid         dd ?

process     process_information

;include_debug_strings

I_PARAM:

I_END: