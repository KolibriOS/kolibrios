;
;    CPU USAGE for MenuetOS  -  Compile with FASM 1.30+
;

use32

              org    0x0
              db     'MENUET00'              ; 8 byte id
              dd     38                      ; required os version
              dd     START                   ; program start
              dd     I_END                   ; program image size
              dd     0x1000                  ; reguired amount of memory
              dd     0x1000
              dd     0x00000000              ; reserved=no extended header

include 'lang.inc'
include 'macros.inc'
START:

    call set_variables

    call draw_window

    mov  edi,0

still:

    mov  eax,23
    mov  ebx,10
    int  0x40

    cmp  eax,1
    je   red
    cmp  eax,2
    je   key
    cmp  eax,3
    je   button

    inc  edi
    cmp  edi,10
    jb   still

    mov  edi,0
    call draw_usage

    jmp  still

  red:
    call draw_window
    jmp  still

  key:
    mov  eax,2
    int  0x40
    jmp  still

  button:
    mov  eax,17
    int  0x40
    cmp  al,byte 0
    jnz  still
    mov  eax,-1
    int  0x40


set_variables:

    pusha

    mov  ecx,190
    mov  edi,pros
    mov  eax,100
    cld
    rep  stosb

    popa
    ret


draw_window:

    pusha

    mov  eax,12                    ; tell os about redraw
    mov  ebx,1
    int  0x40

    mov  eax,0                     ; define and draw window
    mov  ebx,50*65536+207
    mov  ecx,50*65536+127
    mov  edx,0x03000000
    mov  esi,0x806688aa

    mov  edi,0x0088ccee
    int  0x40

    mov  eax,4      ; 'CPU USAGE'
    mov  ebx,8*65536+8
    mov  ecx,dword 0x00ffffff
    mov  edx,text
    mov  esi,textlen-text
    int  0x40

    mov  eax,12                    ; tell os about redraw end
    mov  ebx,2
    int  0x40

    popa
    ret



draw_usage:

    pusha                          ; CPU usage

    cld
    mov  eax,18 ; TSC / SEC
    mov  ebx,5
    int  0x40
    shr  eax,20
    push eax

    mov  eax,18 ; IDLE / SEC
    mov  ebx,4
    int  0x40
    shr  eax,20
    xor  edx,edx
    mov  ebx,100
    mul  ebx

    xor  edx,edx
    pop  ebx
    add  ebx,1
    div  ebx
    push eax

    mov  esi,pros+1
    mov  edi,pros
    mov  ecx,195
    cld
    rep  movsb

    pop  eax
    mov  [pros+99],al

    mov  eax,13
    mov  ebx,5*65536+1
    mov  esi,pros
    mov  edi,pros+99

  newpros:

    add  esi,1

    xor  eax,eax   ; up
    mov  al,[esi]
    add  eax,1
    mov  ecx,22*65536
    mov  cx,ax
    mov  eax,13
    mov  edx,0x0
    int  0x40

    pusha          ; down
    xor  eax,eax
    mov  al,[esi]
    mov  ecx,22
    add  ecx,eax
    shl  ecx,16
    mov  cx,101
    sub  cx,ax
    mov  eax,13
    mov  edx,0xdddddd
    int  0x40
    popa

    add  ebx,2*65536

    cmp  esi,edi
    jb   newpros

    popa
    ret


; DATA AREA

text:     db   'CPU LOAD HISTORY  '
textlen:

pros:

I_END:



