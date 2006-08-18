;
;    LIFE.ASM
;
;    This program displays Conways game of life
;
;    Compile with FASM for Menuet;
;
;
;    Version 0.1    30th March 2004
;                   Mike Hibbett
;
;    Version 0.2    23th May 2004
;                   Random generation dots with start
;
;    Convert to ASCL Libary by Pavlushin Evgeni
;
;    This is an experiment to see how small a usefull application can get
;

use32
               org     0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x100000                 ; memory for app
               dd     0x100000                ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon
include 'ascl.inc'

macro setcell x,y  { mov     [esi + 512*(y)*3 + (x)*3], al }

START:

    mov     al, 0xFF
    mov     esi, I_END

    ; This is the seed pattern.

    ; Life needs a seed pattern, which is 'hardcode' at compile time
    ; The grid is 512 wide (x direction) by 512 deep (y direction)
    ; setcell take the arguments setcell x,y
    ; 0,0 is the top left corner.

    setcell 200,120
    setcell 201,120
    setcell 200,121
    setcell 199,121
    setcell 200,122

    setcell 70,120
    setcell 71,120
    setcell 70,121
    setcell 69,121
    setcell 70,122

    call    draw_window

    ;Random generation dots

    mov ecx,20000
xxx:
    push ecx
    random 30000,edi     ;up pice of screen
    mov al,0xff
    shl edi,3
;    mov [I_END+edi],al
;    random 50000,edi     ;down pice of screen
;    mov al,0xff
;    shl edi,3
    add edi,512*460 ;760
    mov [I_END+edi],al
    pop ecx
    dec ecx
    jnz xxx

still:

    timeevent 5,nokey,red,key,button    ;Wait EVENT with 5 fps
    jmp still

red:                          ; REDRAW WINDOW
    call draw_window
    jmp  still

key:                          ; KEY
    mov  eax,2                  ; get it, but ignore
    int  0x40

nokey:

    ; cycle life state

    mov  esi, I_END + 512*3

    mov     al, 0xFF

lifeloop:
    mov     ah, 0
    cmp     [esi - 3], al
    jne     t2
    inc     ah
t2:
    cmp     [esi + 3], al
    jne     t3
    inc     ah
t3:
    cmp     [esi - 512*3], al
    jne     t4
    inc     ah
t4:
    cmp     [esi + 512*3], al
    jne     t5
    inc     ah
t5:
    cmp     [esi - 512*3 - 3], al
    jne     t6
    inc     ah
t6:
    cmp     [esi - 512*3 + 3], al
    jne     t7
    inc     ah
t7:
    cmp     [esi + 512*3 - 3], al
    jne     t8
    inc     ah
t8:
    cmp     [esi + 512*3 + 3], al
    jne     tend
    inc     ah

tend:
    ; If cell is empty but has 3 neigbours, birth
    ; If cell is occupied and has 2,3 neigbours, live
    ; else die

    cmp     ah, 3
    jne     btest
    mov     [esi+1], al
    jmp     nextcell

btest:
    cmp     ah, 2
    jne     nextcell
    cmp     [esi], al
    jne     nextcell
    mov     [esi+1], al

nextcell:
    add     esi, 3
    cmp     esi, I_END + 512*512*3
    jne     lifeloop

    ; copy new generation across

    mov     ecx, 512*512*3
    mov     esi, I_END+1
    mov     edi, I_END
    rep     movsb               ; copy the data across

    mov     ecx, 512*512
    mov     esi, I_END
nc1:
    mov     [esi+2], byte 0
    add     esi, 3
    loop    nc1

    mov     ebx, I_END
    mov     ecx, 512*65536+512
    mov     edx, 5*65536+20
    mov     eax,7
    int     0x40

    jmp  still

button:                       ; BUTTON - only close supported
    close

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:
    startwd
    window 50,50,512+9,512+23,window_Skinned
    label  8,8,'Life Screen',cl_White+font_Big
    endwd
    ret

I_END:
