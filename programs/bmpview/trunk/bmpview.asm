;
;   BMP VIEWER
;   modified by Ivan Poddubny
;
;   Compile with FASM for Menuet
;

  use32
  org    0x0
  db     'MENUET01'              ; 8 byte id
  dd     0x01                    ; header version
  dd     START                   ; start of code
  dd     I_END                   ; size of image
  dd     0x300000                ; memory for app
  dd     0xfff0                  ; esp
  dd     I_Param , 0x0           ; I_Param , I_Icon

include 'lang.inc'
include 'macros.inc'


START:                          ; start of execution

    cmp  dword [I_Param],0
    je   noparam

    cmp  dword [I_Param],'BOOT'
    jne  noboot
    call load_image
    call set_as_background
    mov  eax,15
    mov  ebx,4
    mov  ecx,2
    int  0x40
    mov  eax,15
    mov  ebx,3
    int  0x40

    or   eax,-1
    int  0x40
  noboot:

    mov edi,name_string
    mov al,0
    mov ecx,70
    rep stosb

    mov ecx,50
    mov edi,I_Param
    repne scasb
    sub edi,I_Param
    mov ecx,edi

    mov esi,I_Param
    mov edi,name_string
    cld
    rep movsb
    call load_image

  noparam:

    call draw_window            ; at first, draw the window

still:

    mov  eax,10                 ; wait here for event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

    jmp  still

  red:                          ; redraw
    call draw_window
    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    int  0x40
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jne  noclose
    mov  eax,-1                 ; close this program
    int  0x40
  noclose:

    cmp  ah,2
    jne  no_file_name
    call read_string
    jmp  still
  no_file_name:

    cmp  ah,3
    jne  no_load_image
    call load_image
    call draw_window
    jmp  still
  no_load_image:

    cmp  ah,4
    jne  no_setasbackground
    call set_as_background
    jmp  still
  no_setasbackground:

    cmp  ah,5
    jne  no_tiled
    mov  eax,15
    mov  ebx,4
    mov  ecx,1
    int  0x40
    mov  eax,15
    mov  ebx,3
    int  0x40
    jmp  still
  no_tiled:

    cmp  ah,6
    jne  no_stretch
    mov  eax,15
    mov  ebx,4
    mov  ecx,2
    int  0x40
    mov  eax,15
    mov  ebx,3
    int  0x40
    jmp  still
  no_stretch:


    jmp  still

type        dd 0x0
i_pos       dd 0x0
x_size      dd 0x1
y_size      dd 0x1
bpp         dd 0x24

temp dd 999

fileinfoblock:

    dd  0                       ; 0 = read
    dd  0                       ; first 512 block
    dd  1                       ; number of blocks to read
    dd  0x10000+1024            ; read to
    dd  35                 ; 17000 byte work area
name_string:
    db  '/HARDDISK/FIRST/MENUET/PICS/NEW.BMP',0
    times 100 db 0


read_header:

    pusha

    mov  edi,0x10000
    mov  ecx,100
    mov  eax,0
    cld
    rep  stosb

    mov  [fileinfoblock+ 8],dword 1
    mov  [fileinfoblock+12],dword 0x10000

push dword [name_string-4]
mov [name_string-4],dword 0x20000
    mov  eax,58
    mov  ebx,fileinfoblock
    int  0x40
pop dword [name_string-4]

    movzx eax,word [0x10000+0]
    mov   [type],eax
    mov   eax,[0x10000+10]
    mov   [i_pos],eax
    mov   eax,[0x10000+18]
    mov   [x_size],eax
    mov   eax,[0x10000+22]
    mov   [y_size],eax
    movzx eax,word [0x10000+28]
    mov   [bpp],eax

    popa
    ret


draw_picture_info:

    pusha

    mov  eax,13
    mov  ebx,380*65536+6*5
    mov  ecx,65*65536+40
    mov  edx,0xffffff
    int  0x40

    mov  eax,47
    mov  ebx,5*65536
    mov  ecx,[x_size]
    mov  edx,380*65536+65
    mov  esi,0x224466
    int  0x40

    mov  ecx,[y_size]
    add  edx,10
    int  0x40

    mov  ecx,[bpp]
    add  edx,20
    int  0x40

    popa
    ret



load_image:

    pusha

    call read_header

    cmp  word [type],'BM'
    je   ok_image

    cmp  [bpp],24
    je   ok_image

    mov  eax,13                  ; not found !
    mov  ebx,150*65536+50
    mov  ecx,100*65536+50
    mov  edx,0xff0000
    int  0x40

    mov  eax,5
    mov  ebx,100
    int  0x40

    jmp  retimage

  ok_image:

    call draw_picture_info

    mov  [fileinfoblock+8],dword 0x100000/512
    mov  eax,0x80000
    sub  eax,[i_pos]
    mov  [fileinfoblock+12],eax
push dword [name_string-4]
mov [name_string-4],dword 0x20000
    mov  eax,58
    mov  ebx,fileinfoblock
    int  0x40
pop dword [name_string-4]

    mov  eax,[x_size]
    imul eax,3

    mov  [x_delta],eax

    mov  ebx,[y_size]
    dec  ebx
    imul eax,ebx
    add  eax,0x80000

    mov  esi,eax
    mov  edi,0x180000
  newln:
    push esi
    push edi
    mov  ecx,[x_delta]
    cld
    rep  movsb
    pop  edi
    pop  esi

    sub  esi,[x_delta];640*3
    add  edi,[x_delta];640*3
    cmp  esi,0x80000
    jge  newln

  retimage:

    popa

    ret

x_delta dd 0x1


draw_image:

    pusha

;    mov  eax,7                     ; draw with putimage
;    mov  ebx,0x180000
;    mov  ecx,200*65536+160
;    mov  edx,14*65536+28
;    int  0x40
;    mov  eax,5
;    mov  ebx,200
;    int  0x40

    mov  ebx,0   ; show the image as 320x240 picture
    mov  ecx,0

times 6 db 0x90

  newpix:

    push ebx
    push ecx

    mov  eax,[esp]
    imul eax,[y_size]

    mov  ebx,240
    xor  edx,edx
    div  ebx

    imul eax,3
    imul eax,[x_size]

    mov  esi,eax

    mov  eax,[esp+4]
    imul eax,[x_size]

    mov  ebx,320
    xor  edx,edx
    div  ebx

    imul eax,3

    add  esi,eax
    mov  edx,[0x180000+esi]


    and  edx,0xffffff

    pop  ecx
    pop  ebx

    add  ebx,20
    add  ecx,40
    mov  eax,1
    int  0x40
    add  ebx,-20
    add  ecx,-40

    inc  ebx
    cmp  ebx,320
    jb   newpix

    xor  ebx,ebx

    inc  ecx
    cmp  ecx,240
    jb   newpix

    popa

    ret

set_as_background:

    pusha

    mov  esi,0x180000
  new_smooth:
    xor  eax,eax
    xor  ebx,ebx
    mov  al,[esi]
    mov  bl,[esi+3]
    add  eax,ebx
    mov  bl,[esi+6]
    add  eax,ebx
    mov  bl,[esi+9]
    add  eax,ebx
    shr  eax,2
    and  eax,0xff
    inc  esi
    cmp  esi,0x180000+640*480*3
    jbe  new_smooth


    mov  eax,15
    mov  ebx,1
    mov  ecx,[x_size]
    mov  edx,[y_size]
    int  0x40

    mov  eax,15
    mov  ebx,5
    mov  ecx,0x180000
    mov  edx,0
    mov  esi,640*480*3
    int  0x40

    mov  eax,15
    mov  ebx,3
    int  0x40


    popa

    ret

ya   dd 300
addr dd name_string
case_sens db 0


read_string:
pusha

mov edi,[addr]
mov eax,[addr]
mov eax,[eax-4]
mov [temp],eax

add edi,eax

call print_strings

f11:
mov eax,10
int 0x40
cmp eax,2
jne read_done
int 0x40
shr eax,8

cmp eax,13
je read_done

cmp eax,192
jne noclear

xor eax,eax
mov [temp],eax
mov edi,[addr]
mov [edi-4],eax
mov ecx,49
cld
rep stosb
mov edi,[addr]
call print_strings
jmp f11

noclear:

cmp eax,8
jnz nobsl
cmp [temp],0
jz f11
dec [temp]
mov edi,[addr]
add edi,[temp]
mov [edi],byte 0

mov eax,[addr]
dec dword [eax-4]

call print_strings
jmp f11

nobsl:
cmp [temp],50
jae read_done

cmp eax,dword 31
jbe f11
cmp [case_sens],1
je keyok
cmp eax,dword 95
jb keyok
add eax,-32
keyok:
mov edi,[addr]
add edi,[temp]
mov [edi],al

inc [temp]

mov eax,[addr]
inc dword [eax-4]
call print_strings
cmp [temp],50
jbe f11

read_done:
mov ecx,50
sub ecx,[temp]
mov edi,[addr]
add edi,[temp]
xor eax,eax
cld
rep stosb

mov [temp],999

call print_strings

popa
ret

print_strings:
pusha
mov eax,13
mov ebx,80*65536+6*45
mov ecx,[ya]
shl ecx,16
add ecx,12
mov edx,0xffffff
int 0x40

mov edx,[addr]
mov esi,[edx-4]
mov eax,4
mov ebx,80*65536+2
add ebx,[ya]
mov ecx,0
int 0x40

cmp [temp],50
ja @f

mov eax,[ya]
mov ebx,eax
shl eax,16
add eax,ebx
add eax,10
mov ecx,eax

mov eax,[temp]
imul eax,eax,6
add eax,80
mov ebx,eax
shl eax,16
add ebx,eax

mov eax,38
mov edx,0
int 0x40
@@:

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
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,20*65536+444          ; [x start] *65536 + [x size]
    mov  ecx,10*65536+333          ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff            ; color of work area RRGGBB,8->color gl
    mov  esi,0x808899ff            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x008899ff            ; color of frames    RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff            ; color of text RRGGBB
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40

    mov  eax,8
    mov  ebx,20*65536+52
    mov  ecx,295*65536+16
    mov  edx,2
    mov  esi,0x306090
    int  0x40

    add  ebx,336*65536+20
    add  ecx,5*65536
    mov  edx,3
    int  0x40

    sub  ecx,20*65536
    mov  edx,4
    int  0x40

    sub  ecx,40*65536
    inc  edx
    int  0x40

    add  ecx,20*65536
    inc  edx
    int  0x40

    mov  ebx,346*65536+45
    mov  edx,info+1
    mov  esi,15
  newinfo:
    mov  ecx,[tcolor]
    cmp  [edx-1],byte 'w'
    jne  nowhite
    mov  ecx,[btcolor]
  nowhite:
    mov  eax,4
    int  0x40
    add  ebx,10
    add  edx,16
    cmp  [edx-1],byte 'x'
    jne  newinfo

    mov  ebx,20*65536+300           ; draw info text with function 4
    mov  ecx,[btcolor]
    mov  edx,text
    mov  esi,70
  newline:
    mov  eax,4
    int  0x40
    add  ebx,10
    add  edx,esi
    cmp  [edx],byte 'x'
    jne  newline

    call print_strings

    call draw_image

    call draw_picture_info

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


; DATA AREA

tcolor   dd  0x000000
btcolor  dd  0x224466+0x808080


text:
 db '  FILE:                                                               '
 db 'x'

info:
 db '  IMAGE INFO    '
 db '                '
 db '  X:            '
 db '  Y:            '
 db '                '
 db '  BPP:          '
 db '                '
 db '                '
 db '  16M COLOURS   '
 db '  640x480 max   '
times 10 db '                '
 db 'w     TILED     '
 db '                '
 db 'w    STRETCH    '
 db '                '
 db 'w   SET AS BGR  '
 db '                '
 db 'w      LOAD     '
 db 'x'


labelt:    db   'BMP VIEWER'
labellen:

I_END:
I_Param:
