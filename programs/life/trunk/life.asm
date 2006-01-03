;
;    LIFE.ASM
;
;    This program displays Conways game of life
;
;    Compile with FASM v1.49 for DOS;
;
;    Version 0.1a   20th May   2004
;                   Ivan Poddubny
;
;    Version 0.1    30th March 2004
;                   Mike Hibbett
;
;    This is an experiment to see how small a usefull application can get

include "lang.inc"
include "macros.inc"

  use32
  org     0x0

  db     'MENUET01'              ; 8 byte id
  dd     0x01                    ; header version
  dd     START                   ; start of code
  dd     I_END                   ; size of image
  dd     0xc1000                  ; memory for app
  dd     0xc1000                  ; esp
  dd     0x0 , 0x0               ; I_Param , I_Icon

;include "DEBUG.INC"
macro setcell x,y  { mov     [esi + 512*(y)*3 + (x)*3], al }

START:

    mov     al, 0xFF
    mov     esi, I_END

    ; This is the seed pattern.

    ; Life needs a seed pattern, which is 'hardcode' at compile time
    ; The grid is 512 wide (x direction) by 512 deep (y direction)
    ; setcell take the arguments setcell x,y
    ; 0,0 is the top left corner.

;    setcell 200,120
;    setcell 201,120
;    setcell 200,121
;    setcell 199,121
;    setcell 200,122

;    setcell 70,120
;    setcell 71,120
;    setcell 70,121
;    setcell 69,121
;    setcell 70,122

    mov     eax, 40
    mov     ebx, 100101b
    int     0x40

    call    draw_window

still:

;    mov  eax, 23                 ; wait here for event
;    mov  ebx, 5
;    int  0x40
    mov  eax, 11
    int  0x40

    test eax, eax
    je   nokey
    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,3                  ; button in buffer ?
    je   button
    cmp  eax,6
    je   mouse

    jmp  still


  mouse:
    mov  eax, 37
    mov  ebx, 2
    int  0x40
    test eax, eax
    jz   still

    mov  eax, 37
    mov  ebx, 1
    int  0x40
    sub  eax, 5*65536+20
    mov  ebx, eax
    shr  eax, 16
    and  ebx, 0xffff

; WRITE COORDINATES
;   dpd  eax
;   dps  "  "
;   dpd  ebx
;   dps  <10,13>

    cmp  ax, 0
    js   still
    cmp  bx, 0
    js   still

    shl  ebx, 9
    add  ebx, eax
    imul ebx, 3
    add  ebx, I_END
    mov  [ebx], dword 0xFFFFFFFF
    jmp  draw

  red:                          ; REDRAW WINDOW
    call draw_window
    jmp  still


nokey:
    ; cycle life state

mov eax,5
mov ebx,5
int 0x40

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
draw:
    mov     ebx, I_END
    mov     ecx, 512*65536+512
    mov     edx, 5*65536+22
    mov     eax,7
    int     0x40

    jmp  still

button:                       ; BUTTON - only close supported
    or   eax,-1
    int  0x40




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:
    mov  eax,12
    mov  ebx,1
    int  0x40

    mov  eax,0                     ; open window
    mov  ebx,50*65536+512+9
    mov  ecx,50*65536+512+22+4
    mov  edx,0x03000000
    int  0x40

    mov  eax,4                     ; WINDOW LABEL
    mov  ebx,8*65536+8
    mov  ecx,0x10ffffff
    mov  edx,header
    mov  esi,header.size
    int  0x40

    mov     eax,12                    ; function 12:tell os about windowdraw
    mov     ebx,2                     ; 2, end of draw
    int     0x40

    ret



; DATA AREA

header          db  'Life'
   .size = $ - header

I_END:
