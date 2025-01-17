;
;    MARIO for MENUET
;
;    Compile with fasm
;
   
use32
   
           org     0x0

           db     'MENUET01'             ; header
           dd     1                      ; version
           dd     START                  ; program start
           dd     I_END                  ; image size
           dd     MEM                    ; size memory program
           dd     STACKTOP               ; pointer to begin stack pos
           dd     0
           dd     0
   
res_base        = loadp + 0x36 ; offset on array pixel in BMP file
res_width_px    = 150
res_height_px   = 35
res_line_size   = (res_width_px*3 + 11b) and not 11b ; 150*3 + 2


block_width     = 40
block_height    = 40
block_line_size = block_width * 3
block_img_size  = block_width * block_height * 3
block_draw_size = (block_width shl 16) + block_height ; for sysfn 7
block_count_pixel = block_width * block_height

mario_width     = 50
mario_height    = 70
mario_line_size = mario_width * 3
mario_img_size  = mario_width * mario_height * 3
mario_draw_size = (mario_width shl 16) + mario_height
mario_count_pixel = mario_width * mario_height


wnd_pos_x       = 80
wnd_pos_y       = 20

wnd_header_size = 0x16
wnd_height      = 9*block_height + wnd_header_size
wnd_width       = 12*block_width + 1


START:
   
   
    call load_graph
   
    mov  [marioxy],   dword 300*65536+270
    mov  [leftright], dword 1
    mov  [velocity],  dword 0xff
   
    call draw_window
   
   
sta:
   
    mov  eax,field
    mov  dword[xxyy], 1*65536 + wnd_header_size
   
  stl:
   
    cmp  byte[eax], ' '
    jnz  @f
    mov  ebx,empty
  @@:
   
    cmp  byte[eax], '1'
    jnz  @f
    mov  ebx,build_1
  @@:
   
    cmp  byte[eax], '2'
    jnz  @f
    mov  ebx,build_2
  @@:
   
    cmp  byte[eax], '3'
    jnz  @f
    mov  ebx,build_3
  @@:
   
    pusha
   
    mov  edx,[xxyy]
   
    mov  eax,7           ; display image
    mov  ecx, block_draw_size
    int  0x40
   
    popa
   
    add  word [xxyy+2], block_width
    cmp  word [xxyy+2], block_width*12+1
    jnz  .nonewline

    mov  word [xxyy+2], 1
    add  word [xxyy+0], block_height
  .nonewline:
   
    add  eax,1
    cmp  eax, field.end
    jnz   stl
   
;  ********* MAIN LOOP ***********
   
   
wait_for_event:
   
    call draw_mario
   
    mov  eax,11              ; check for event
    int  0x40

    cmp  eax,1
    jz   red
    cmp  eax,2
    jz   key
    cmp  eax,3   ; check pressed button
    jnz  wait_for_event
   
    mov  eax, -1 ; close this program
    int  0x40

  red:
   
    mov  [mariomem],dword 0x0
   
    call draw_window
    jmp  sta
   
  key:
   
    mov  [mariomem],dword 0x0

    mov  eax,2   ; ah <- key
    int  0x40
   
    cmp  ah,176   ; 'left'
    jnz  key1
    mov  byte [leftright],byte 1
  key1:
   
    cmp  ah,179   ; 'right'
    jnz  key2
    mov  byte [leftright],byte 4
  key2:
   
    cmp  ah,177   ; 'down'
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
   
    cmp  ah,178   ; 'up'
    jnz  key4
    cmp  [velocity],dword 0xff
    jnz  key4
    mov  [velocity],dword 0x110
  key4:
        
        ; this code using for fix defects drawing(draw in caption area)
        mov eax,71                                  ;draw caption
        mov ebx,1
        mov ecx,title
        int 0x40

    jmp sta                                         ;draw field
        
    ;jmp  wait_for_event
   
load_graph:
   
    pusha
   
    ; empty block init  - set in 0x00500f color
    mov  edi,empty
    mov  ecx, block_count_pixel
    mov  eax,0x00500f
    cld
  @@:
    mov  [edi],eax
    add  edi, 3
    loop @b
   
    mov  [width], block_line_size
    mov  [width_move], block_width/2
    mov  [height_move], block_height/2
    mov  [imul_esi],20*3
    mov  [base], res_base + 3*res_width_px * 34 +3*98-1
   
    mov  esi,0
    mov  edi,build_1
    call convert
   
    mov  esi,1
    mov  edi,build_2
    call convert
   
   
   
    ; build_3
    mov  edi,build_3
    mov  esi, res_base + 230*3+3*300*69
    mov  ecx, block_height
  build_3l:
    pusha
    mov  ecx, block_line_size
    cld
    rep  movsb
    popa
    sub  esi,300*3
    add  edi, block_line_size
    loop build_3l
   
    mov  [width], mario_line_size
    mov  [width_move], mario_width/2
    mov  [height_move], mario_height/2
    mov  [imul_esi],25*3
    mov  [base], res_base + 3*res_width_px*34 + 68
   
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
   
   
; copy in [edi], image x2
convert:
   
    imul esi,[imul_esi]
    add  esi,[base] ; loadp+18*3+3*150*34+68
   
    mov  eax, [width]
    shl  eax, 1 ; *2
    mov  ecx, [height_move]      ;?

  .new_line:
    pusha
    mov  ecx,[width_move]

  .putpix: ; set 4 pixel: edi, edi+3, edi+width, edi+width+3
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
    loop .putpix
    popa

    sub  esi,res_line_size
    add  edi, eax
    loop .new_line
   
    ret

imul_esi        dd 0
base            dd 0
width           dd 0
width_move      dd 0
height_move     dd 0
   
   
   
mario_mirror:

    ; mario_green
    mov  edi,mario_green
    mov  ecx, mario_count_pixel
  @@:
    mov  [edi],dword 0x00500f
    add  edi, 3
    loop @b
   
    ; mario_4 - reverse mario_1
   
    mov  edi, mario_4
    mov  esi, mario_1 + (mario_width - 1)*3
    mov  ecx, mario_height
  m4l1:
    pusha
    mov  ecx, mario_width
  m4l2:
    mov  eax,[esi]
    mov  [edi],ax
    shr  eax,16
    mov  [edi+2],al
    add  edi,3
    sub  esi,3
    loop m4l2
    popa

    add  esi, mario_width*3
    add  edi, mario_width*3
    loop m4l1
   
   
    ; mario_5 - reverse mario_2
   
    mov  edi, mario_5
    mov  esi, mario_2 + (mario_width - 1)*3
    mov  ecx, mario_height
  m5l1:
    pusha
    mov  ecx, mario_width
  m5l2:
    mov  eax,[esi]
    mov  [edi],ax
    shr  eax,16
    mov  [edi+2],al
    add  edi,3
    sub  esi,3
    loop m5l2
    popa

    add  esi, mario_width*3
    add  edi, mario_width*3
    loop m5l1
   
   
    ; mario_6 - reverse mario_3
   
    mov  edi, mario_6
    mov  esi, mario_3 + (mario_width - 1)*3
    mov  ecx, mario_height
  m6l1:
    pusha
    mov  ecx, mario_width
  m6l2:
    mov  eax,[esi]
    mov  [edi],ax
    shr  eax,16
    mov  [edi+2],al
    add  edi,3
    sub  esi,3
    loop m6l2
    popa

    add  esi, mario_width*3
    add  edi, mario_width*3
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
   
    mov  eax,5          ;pause
    mov  ebx,2
    int  0x40
   
    popa
    ret
   
   
   
draw_mario:
   
    pusha
   
    mov  eax,[mariomem]
    cmp  eax,[marioxy]
    jnz  @f
        
    call mario_delay

    popa
    ret

@@:
    mov  eax,[marioxy]
    mov  [mariomem],eax
   
   
    cmp  byte [leftright], 1   ; go left
    jz   m_left
   
    cmp  byte [leftright], 4   ; go right
    jz   m_right
   
    cmp  byte [leftright], 2   ; stand left
    jnz  no_r_still

    mov  eax,7
    mov  ebx,mario_green
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call advance_mario
    mov  eax,7
    mov  ebx,mario_1
    mov  ecx, mario_draw_size
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
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call advance_mario
    mov  eax,7
    mov  ebx,mario_4
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call mario_delay

  no_l_still:
   
    popa
    ret

  m_left:
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call advance_mario
    mov  eax,7
    mov  ebx,mario_1
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call mario_delay
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call advance_mario
    mov  eax,7
    mov  ebx,mario_2
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call mario_delay
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call advance_mario
    mov  eax,7
    mov  ebx,mario_3
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call mario_delay
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call advance_mario
    mov  eax,7
    mov  ebx,mario_2
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call mario_delay
   
    popa
    ret
   
  m_right:
   
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call advance_mario
    mov  eax,7
    mov  ebx,mario_4
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call mario_delay
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call advance_mario
    mov  eax,7
    mov  ebx,mario_5
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call mario_delay
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call advance_mario
    mov  eax,7
    mov  ebx,mario_6
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call mario_delay
   
    mov  eax,7
    mov  ebx,mario_green
    mov  ecx, mario_draw_size
    mov  edx,[marioxy]
    int  0x40

    call advance_mario
    mov  eax,7
    mov  ebx,mario_5
    mov  ecx, mario_draw_size
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
    mov       ebx, (wnd_pos_x shl 16) + wnd_width ;80*65536+480+1
    mov       ecx, (wnd_pos_y shl 16) + wnd_height ;20*65536+382
    mov       edx,0x5466AA88   ; 4 type window + title + no draw work space
    mov       edi,title
    int       0x40
   
    mov       eax,12                    ; tell os about redraw end
    mov       ebx,2
    int       0x40
   
    popa
   
    ret
   
   
; DATA SECTION
   
title:
    db 'SUPER MARIO - USE ARROW KEYS', 0
   
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
.end:
   
xxyy        dd 0x0
   
marioxy     dd 0x0
   
mariomem    dd 0x0
   
leftright   dd 0x0
   
velocity    dd 0x0
   
align 16
loadp:
        file 'MARIOALL.BMP'

I_END:

align 0x1000

empty:          rb block_img_size
build_1:        rb block_img_size
build_2:        rb block_img_size
build_3:        rb block_img_size

align 0x1000

mario_1:        rb mario_img_size
mario_2:        rb mario_img_size
mario_3:        rb mario_img_size
mario_4:        rb mario_img_size
mario_5:        rb mario_img_size
mario_6:        rb mario_img_size
mario_green:    rb mario_img_size


        rb 4096
STACKTOP:
MEM:
   
   
   
