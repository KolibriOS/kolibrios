;
;    MARIO for MENUET
;
;    Compile with fasm
;
   
use32
   
           org     0x0
   
           db      'MENUET00'           ; 8 byte id
           dd      38                   ; required os
           dd      START                ; program start
           dd      I_END                ; image size
           dd      0x100000             ; reguired amount of memory
           dd      0x00000000           ; reserved=no extended header
   
   
empty        equ    0x80000  + 0*3*40*40
build_1      equ    0x80000  + 1*3*40*40
build_2      equ    0x80000  + 2*3*40*40
build_3      equ    0x80000  + 3*3*40*40
   
mario_1      equ    0x90000  + 0*3*70*51
mario_2      equ    0x90000  + 1*3*70*51
mario_3      equ    0x90000  + 2*3*70*51
mario_4      equ    0x90000  + 3*3*70*51
mario_5      equ    0x90000  + 4*3*70*51
mario_6      equ    0x90000  + 5*3*70*51
mario_green  equ    0x90000  + 6*3*70*51
loadp        equ    0x90000  + 7*3*70*51
   
   
START:
   
   
    call load_graph
   
    mov  [marioxy],   dword 300*65536+270
    mov  [leftright], dword 1
    mov  [velocity],  dword 0xff
   
    call draw_window
   
   
sta:
   
    mov  eax,field
    mov  edx,1*65526+30
    mov  [xxyy],dword 1*65536+22
   
  stl:
   
    cmp  [eax],byte ' '
    jnz  noempty
    mov  ebx,empty
  noempty:
   
    cmp  [eax],byte '1'
    jnz  no1
    mov  ebx,build_1
  no1:
   
    cmp  [eax],byte '2'
    jnz  no2
    mov  ebx,build_2
  no2:
   
    cmp  [eax],byte '3'
    jnz  no3
    mov  ebx,build_3
  no3:
   
    pusha
   
    mov  edx,[xxyy]
   
    mov  eax,7           ; display image
    mov  ecx,40*65536+40
    int  0x40
   
    popa
   
    add  word [xxyy+2],word 40
    cmp  word [xxyy+2],word 40*12+1
    jnz  nonewline
    mov  word [xxyy+2],word 1
    add  word [xxyy+0],word 40
  nonewline:
   
    add  eax,1
    cmp  eax,field+12*9
    jz   nodraw
    jmp  stl
   
  nodraw:
   
   
;  ********* MAIN LOOP ***********
   
   
wait_for_event:
   
    call draw_mario
   
    mov  eax,11                  ; check if os wants to say something
    int  0x40
    cmp  eax,1
    jz   red
    cmp  eax,2
    jz   key
    cmp  eax,3
    jnz  wait_for_event

    jmp  button
   
  red:
   
    mov  [mariomem],dword 0x0
   
    call draw_window
    jmp  sta
   
  key:
   
    mov  [mariomem],dword 0x0
   
    mov  eax,2   ; ah <- key
    int  0x40
   
    cmp  ah,176
    jnz  key1
    mov  byte [leftright],byte 1
  key1:
   
    cmp  ah,179
    jnz  key2
    mov  byte [leftright],byte 4
  key2:
   
    cmp  ah,177
    jnz  key3
    mov  al,byte [leftright]
    mov  bl,al
    cmp  al,1
    jnz  key31
    mov  bl,2
  key31:
    cmp  al,4
    jnz  key32
    mov  bl,3
  key32:
    mov  byte [leftright],bl
  key3:
   
    cmp  ah,178
    jnz  key4
    cmp  [velocity],dword 0xff
    jnz  key4
    mov  [velocity],dword 0x110
  key4:
   
   
   
    jmp  wait_for_event
   
  button:
   
    mov  eax,0xffffffff          ; close this program
    int  0x40
   
    ; end of program
   
   
load_graph:
   
    pusha
   
    mov  eax,6        ; 6 = open file
    mov  ebx,filename
    mov  ecx,0
    mov  edx,0xFFFFFF
    mov  esi,loadp
    mov  edi,0                     ; floppy
    int  0x40
   
    ; empty
   
    mov  edi,empty
    mov  ecx,40*40
    mov  eax,0x00500f
    cld
  emptyl:
    mov  [edi],eax
    add  edi,3
    loop emptyl
   
    mov  [width],40*3
    mov  [width_move],20
    mov  [sub_esi],150*3+2
    mov  [add_edi],40*3*2
    mov  [imul_esi],20*3
    mov  [base],loadp+18*3+3*150*34+3*98-1
   
    mov  esi,0
    mov  edi,build_1
    call convert
   
    mov  esi,1
    mov  edi,build_2
    call convert
   
   
   
    ; build_3
   
    mov  edi,build_3
    mov  esi,loadp+18*3+230*3+3*300*69
    mov  ecx,40
  build_3l:
    pusha
    mov  ecx,3*40
    cld
    rep  movsb
    popa
    sub  esi,300*3
    add  edi,40*3
    loop build_3l
   
    mov  [width],50*3
    mov  [width_move],25
    mov  [sub_esi],150*3+2
    mov  [add_edi],50*3*2
    mov  [imul_esi],25*3
    mov  [base],loadp+18*3+3*150*34+68
   
    mov  esi,0
    mov  edi,mario_1
    call convert
   
    mov  esi,1
    mov  edi,mario_2
    call convert
   
    mov  esi,2
    mov  edi,mario_3
    call convert
   
    jmp  mario_mirror
   
   
convert:
   
    imul esi,[imul_esi]
    add  esi,[base] ; loadp+18*3+3*150*34+68
   
    mov  ecx,70
  mario_1l:
    pusha
    mov  ecx,[width_move]
  putpix:
    mov  eax,[esi]
    mov  [edi+00],ax
    mov  [edi+03],ax
    mov  ebx,[width]
    mov  [edi+ebx],ax
    add  ebx,3
    mov  [edi+ebx],ax
    shr  eax,16
    mov  [edi+02],al
    dec  ebx
    mov  [edi+ebx],al
    mov  [edi+05],al
    add  ebx,3
    mov  [edi+ebx],al
    add  esi,3
    add  edi,6
    loop putpix
    popa
    sub  esi,[sub_esi]
    add  edi,[add_edi]
    loop mario_1l
   
    ret
   
sub_esi  dd 0x0
add_edi  dd 0x0
imul_esi dd 0x0
base     dd 0x0
width    dd 0x0
width_move dd 0x0
   
   
   
mario_mirror:
   
   
    ; mario_green
   
    mov  edi,mario_green
    mov  ecx,70*50
  mario_greenl:
    mov  [edi],dword 0x00500f
    add  edi,3
    loop mario_greenl
   
   
   
    ; mario_4
   
    mov  edi,mario_4
    mov  esi,mario_1+49*3
    mov  ecx,70
  m4l1:
    pusha
    mov  ecx,50
  m4l2:
    mov  eax,[esi]
    mov  [edi],ax
    shr  eax,16
    mov  [edi+2],al
    add  edi,3
    sub  esi,3
    loop m4l2
    popa
    add  esi,50*3
    add  edi,50*3
    loop m4l1
   
   
    ; mario_5
   
    mov  edi,mario_5
    mov  esi,mario_2+49*3
    mov  ecx,70
  m5l1:
    pusha
    mov  ecx,50
  m5l2:
    mov  eax,[esi]
    mov  [edi],ax
    shr  eax,16
    mov  [edi+2],al
    add  edi,3
    sub  esi,3
    loop m5l2
    popa
    add  esi,50*3
    add  edi,50*3
    loop m5l1
   
   
    ; mario_6
   
    mov  edi,mario_6
    mov  esi,mario_3+49*3
    mov  ecx,70
  m6l1:
    pusha
    mov  ecx,50
  m6l2:
    mov  eax,[esi]
    mov  [edi],ax
    shr  eax,16
    mov  [edi+2],al
    add  edi,3
    sub  esi,3
    loop m6l2
    popa
    add  esi,50*3
    add  edi,50*3
    loop m6l1
   
   
    popa
   
    ret
   
   
   
advance_mario:
   
    pusha
   
    mov  eax,[marioxy]      ; wherey ?
    and  eax,65535
    add  eax,40+21
    xor  edx,edx
    mov  ebx,40
    div  ebx
    mov  [marioy],eax
   
    mov  eax,[marioxy]      ; wherex ?
    shr  eax,16
    add  eax,20
    xor  edx,edx
    mov  ebx,40
    div  ebx
    mov  [mariox],eax
   
   
   
    cmp  byte [leftright],byte 1    ; go left
    jnz  noleft
    mov  eax,[marioxy]
    xor  edx,edx
    shr  eax,16
    sub  eax,7
    mov  ebx,40
    div  ebx
    add  eax,field+7*12
    cmp  [eax],byte ' '
    jz   mgoleft
    mov  byte [leftright],byte 2
    jmp  noleft
  mgoleft:
    mov  eax,[marioxy]
    sub  eax,6*65536
    mov  [marioxy],eax
  noleft:
   
   
   
    cmp  byte [leftright],byte 4    ; go right
    jnz  noright
    mov  eax,[marioxy]
    xor  edx,edx
    shr  eax,16
    add  eax,54
    mov  ebx,40
    div  ebx
    add  eax,field+7*12
    cmp  byte [eax],byte ' '
    jz   mgoright
    mov  byte [leftright],byte 3
    jmp  noright
  mgoright:
    mov  eax,[marioxy]
    add  eax,6*65536
    mov  [marioxy],eax
  noright:
   
   
   
   
    cmp  [velocity],dword 0x100    ; jump up ?
    jbe  nojumpup
    mov  ebx,[velocity]
    sub  ebx,1
    cmp  ebx,0x100
    jnz  no0x100
    mov  [velocity],dword 0xff
    jmp  nojumpup
  no0x100:
    mov  [velocity],ebx
    sub  ebx,0x100-1
    mov  eax,[marioxy]
    sub  ax,bx
    mov  [marioxy],eax
  nojumpup:
   
    call mario_environment
   
    cmp  [velocity],dword 0x100    ; jump -> down ?
    jge  nojumpdown
    mov  eax,[marioy]
    cmp  eax,[mario_below]
    jb   yesgd
    mov  [velocity],dword 0xff
    mov  eax,[marioxy]
    mov  eax,[mario_below]
    sub  eax,1
    mov  ebx,40
     mul  ebx
     sub  eax,10
    mov  word [marioxy],ax
    jmp  nojumpdown
  yesgd:
    mov  ebx,[velocity]
    sub  ebx,1
    mov  [velocity],ebx
    mov  eax,[marioxy]
    add  ax,0xff
    sub  ax,bx
    mov  [marioxy],eax
  nojumpdown:
   
   
    popa
   
    ret
   
   
marioy dd 0x0
   
mariox dd 0x0
   
mario_below dd 5
   
   
mario_environment:
   
     pusha
   
     mov  eax,[marioy]
     mov  ebx,12
     mul  ebx
     add  eax,[mariox]
     add  eax,field
   
     cmp  [eax],byte ' '
     jz   me_ok
   
     mov  eax,[marioy]
     mov  [mario_below],eax
   
     popa
     ret
  
   me_ok:
   
     mov  [mario_below],dword 100
   
     popa
     ret
   
   
mario_delay:
   
    pusha
   
    mov  eax,5
    mov  ebx,2
    int  0x40
   
    popa
    ret
   
   
   
draw_mario:
   
    pusha
   
    mov  eax,[mariomem]
    cmp  eax,[marioxy]
    jnz  dm1
   
    call mario_delay
   
    popa
   
    ret
   
  dm1:
   
    mov  eax,[marioxy]
    mov  [mariomem],eax
   
   
    cmp  byte [leftright],byte 1   ; go left
    jnz  no_m_left
    jmp  m_left
  no_m_left:
   
    cmp  byte [leftright],byte 4   ; go right
    jnz  no_m_right
    jmp  m_right
  no_m_right:
   
    cmp  byte [leftright],byte 2   ; stand left
    jnz  no_r_still
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call advance_mario
    mov  eax,7
    mov  ebx,mario_1
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call mario_delay
    popa
    ret
  no_r_still:
   
    cmp  byte [leftright],byte 3   ; stand right
    jnz  no_l_still
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call advance_mario
    mov  eax,7
    mov  ebx,mario_4
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call mario_delay
   
    popa
    ret
  no_l_still:
   
    popa
    ret
   
   
   
   
  m_left:
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call advance_mario
    mov  eax,7
    mov  ebx,mario_1
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call mario_delay
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call advance_mario
    mov  eax,7
    mov  ebx,mario_2
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call mario_delay
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call advance_mario
    mov  eax,7
    mov  ebx,mario_3
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call mario_delay
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call advance_mario
    mov  eax,7
    mov  ebx,mario_2
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call mario_delay
   
    popa
   
    ret
   
  m_right:
   
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call advance_mario
    mov  eax,7
    mov  ebx,mario_4
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call mario_delay
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call advance_mario
    mov  eax,7
    mov  ebx,mario_5
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call mario_delay
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call advance_mario
    mov  eax,7
    mov  ebx,mario_6
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call mario_delay
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call advance_mario
    mov  eax,7
    mov  ebx,mario_5
    mov  ecx,50*65536+70
    mov  edx,[marioxy]
    int  0x40
    call mario_delay
   
    popa
   
    ret
   
   
   
   
   
draw_window:
   
    pusha
   
    mov       eax,12                    ; tell os about redraw
    mov       ebx,1
    int       0x40
   
    mov       eax,0                     ; define and draw window
    mov       ebx,80*65536+480+1
    mov       ecx,20*65536+382
    mov       edx,0x0000500f
    mov       esi,0x8066AA88
    mov       edi,0x00559977
    int       0x40
   
    mov       eax,dword 0x00000004
    mov       ebx,8*65536+8
    mov       ecx,dword 0x00cccccc
    mov       edx,text
    mov       esi,textlen-text
    int       0x40
   
    mov       eax,8
    mov       ebx,(481-19)*65536+12     ; button start x & size
    mov       ecx,5*65536+12            ; button start y & size
    mov       edx,1                     ; button number
    mov       esi,0x00229911            ; button color
    int       0x40
   
    mov       eax,12                    ; tell os about redraw end
    mov       ebx,2
    int       0x40
   
    popa
   
    ret
   
   
; DATA SECTION
   
   
filename:
    db  'MARIOALLBMP'
   
text:
    db 'SUPER MARIO - USE ARROW KEYS'
textlen:
   
xx  db 'x'
   
field:
   
    db  '1          1'
    db  '1          1'
    db  '111      111'
    db  '1          1'
    db  '1          1'
    db  '1  111111  1'
    db  '1          1'
    db  '1          1'
    db  '222222222222'
   
xxyy        dd 0x0
   
marioxy     dd 0x0
   
mariomem    dd 0x0
   
leftright   dd 0x0
   
velocity    dd 0x0
   
   
I_END:
   
   
   
   
