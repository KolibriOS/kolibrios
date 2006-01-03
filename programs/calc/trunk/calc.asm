;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;   Calculator for MenuetOS
;;
;;   Compile with FASM for Menuet
;;


use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x1000                  ; memory for app
               dd     0x1000                  ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

include 'lang.inc'
include 'macros.inc'

START:                          ; start of execution

     call draw_window

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
    mov  eax,2
    int  0x40
    shr  eax,8
    mov  edi,asci             ;translation & emulation
    mov  ecx,35               ;of button ID's
    cld
    repne scasb
    jne  still
    sub  edi,asci
    dec  edi
    mov  esi,butid
    add  esi,edi
    lodsb
    jmp  testbut

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    shr  eax,8
  testbut:
    cmp  eax,1                   ; button id=1 ?
    jne  noclose
    mov  eax,-1                  ; close this program
    int  0x40
  noclose:

    cmp  eax,2
    jne  no_reset
    call clear_all
    jmp  still
  no_reset:

    finit

    mov    ebx,muuta1    ; Transform to fpu format
    mov    esi,18
    call   atof
    fstp   [trans1]

    mov    ebx,muuta2
    mov    esi,18
    call   atof
    fstp   [trans2]

    fld    [trans2]

    cmp  eax,30
    jne  no_sign
    cmp  [dsign],byte '-'
    jne  no_m
    mov  [dsign],byte '+'
    call print_display
    jmp  still
  no_m:
    mov  [dsign],byte '-'
    call print_display
    jmp  still
  no_sign:

    cmp  eax,3
    jne  no_display_change
    inc  [display_type]
    cmp  [display_type],2
    jbe  display_continue
    mov  [display_type],0
  display_continue:
    mov  eax,[display_type]
    mov  eax,[multipl+eax*4]
    mov  [entry_multiplier],eax
    call print_display
    jmp  still
  multipl:  dd  10,16,2

  no_display_change:


    cmp  eax,6
    jb   no_10_15
    cmp  eax,11
    jg   no_10_15
    add  eax,4
    call number_entry
    jmp  still
   no_10_15:

    cmp  eax,12
    jb   no_13
    cmp  eax,14
    jg   no_13
    sub  eax,11
    call number_entry
    jmp  still
   no_13:

    cmp  eax,12+6
    jb   no_46
    cmp  eax,14+6
    jg   no_46
    sub  eax,11+3
    call number_entry
    jmp  still
   no_46:

    cmp  eax,12+12
    jb   no_79
    cmp  eax,14+12
    jg   no_79
    sub  eax,11+6
    call number_entry
    jmp  still
   no_79:

    cmp  eax,13+18
    jne  no_0
    mov  eax,0
    call number_entry
    jmp  still
    no_0:

    cmp  eax,32
    jne  no_id
    inc  [id]
    and  [id],1
    mov  [new_dec],100000
    jmp  still
  no_id:

    cmp  eax,16
    jne  no_sin
    fld  [trans1]
    fsin
    jmp  show_result
  no_sin:

    cmp  eax,17
    jne  no_int
    fld  [trans1]
    frndint
    jmp  show_result
  no_int:

    cmp  eax,22
    jne  no_cos
    fld  [trans1]
    fcos
    jmp  show_result
  no_cos:

    cmp  eax,23
    jne  no_lg2
    fldlg2
    jmp  show_result
  no_lg2:

    cmp  eax,28
    jne  no_tan
    fld  [trans1]
    fcos
    fstp [tmp2]
    fld  [trans1]
    fsin
    fdiv [tmp2]
    jmp  show_result
  no_tan:

    cmp  eax,29
    jne  no_pi
    fldpi
    jmp  show_result
   no_pi:

    cmp  eax,34
    jne  no_sqrt
    fld  [trans1]
    fsqrt
    jmp  show_result
  no_sqrt:

    cmp  eax,15
    jne  no_add
    call calculate
    call print_display
    call new_entry
    mov  [calc],'+'
    jmp  still
  no_add:

    cmp  eax,21
    jne  no_sub
    call calculate
    call print_display
    call new_entry
    mov  [calc],'-'
    jmp  still
  no_sub:

    cmp  eax,27
    jne  no_div
    call calculate
    call print_display
    call new_entry
    mov  [calc],'/'
    jmp  still
  no_div:

    cmp  eax,33
    jne  no_mul
    call calculate
    call print_display
    mov  [calc],'*'
    call new_entry
    jmp  still
  no_mul:

    cmp    eax,35
    jne    no_calc
    call   calculate
    jmp    still
  no_calc:

    jmp  still

  show_result:

    call   ftoa
    call   print_display

    jmp    still

error:

    jmp  still


calculate:

    pusha

    cmp  [calc],' '
    je   no_calculation

    cmp  [calc],'/'
    jne  no_cdiv
    fdiv [trans1]
  no_cdiv:

    cmp  [calc],'*'
    jne  no_cmul
    fmul [trans1]
  no_cmul:

    cmp  [calc],'+'
    jne  no_cadd
    fadd [trans1]
  no_cadd:

    cmp  [calc],'-'
    jne  no_cdec
    fsub [trans1]
  no_cdec:

    call   ftoa
    call   print_display

  no_calculation:

    popa

    ret



number_entry:

    pusha

    cmp  eax,[entry_multiplier]
    jge  no_entry

    cmp  [id],1
    je   decimal_entry

    mov  ebx,[integer]
    test ebx,0xF0000000
    jnz  no_entry

    mov  ebx,eax
    mov  eax,[integer]
    mov  ecx,[entry_multiplier]
    mul  ecx
    add  eax,ebx
    mov  [integer],eax
    call print_display

    call to_muuta

    popa
    ret

  decimal_entry:

    imul eax,[new_dec]
    add  [decimal],eax

    mov  eax,[new_dec]
    xor  edx,edx
    mov  ebx,[entry_multiplier]
    div  ebx
    mov  [new_dec],eax

    call print_display

    call to_muuta

    popa

    ret

  no_entry:

    call print_display

    call to_muuta

    popa
    ret


to_muuta:

    pusha

    mov  al,[dsign]

    mov  esi,muuta0
    mov  edi,muuta1
    mov  ecx,18
    cld
    rep  movsb

    mov  [muuta1],al

    mov  edi,muuta1+10      ; INTEGER
    mov  eax,[integer]
  new_to_muuta1:
    mov  ebx,10
    xor  edx,edx
    div  ebx
    mov  [edi],dl
    add  [edi],byte 48
    dec  edi
    cmp  edi,muuta1+1
    jge  new_to_muuta1

    mov  edi,muuta1+17      ; DECIMAL
    mov  eax,[decimal]
  new_to_muuta2:
    mov  ebx,10
    xor  edx,edx
    div  ebx
    mov  [edi],dl
    add  [edi],byte 48
    dec  edi
    cmp  edi,muuta1+12
    jge  new_to_muuta2

    call print_muuta

    popa
    ret



print_muuta:

    pusha

    mov  eax,13
    mov  ebx,25*65536+125
    mov  ecx,200*65536+22
    mov  edx,0xffffff
    int  0x40

    mov  eax,4
    mov  ebx,25*65536+200
    mov  ecx,0x0
    mov  edx,muuta1
    mov  esi,18
    int  0x40

    mov  eax,4
    mov  ebx,25*65536+210
    mov  ecx,0x0
    mov  edx,muuta2
    mov  esi,18
    int  0x40

    popa
    ret



new_entry:

    pusha

    mov  esi,muuta1
    mov  edi,muuta2
    mov  ecx,18
    cld
    rep  movsb

    mov  esi,muuta0
    mov  edi,muuta1
    mov  ecx,18
    cld
    rep  movsb

    mov  [integer],0
    mov  [decimal],0
    mov  [id],0
    mov  [new_dec],100000
    mov  [sign],byte '+'

    popa
    ret

ten        dd    10.0,0
tmp        dw       1,0
sign       db       1,0
tmp2       dq     0x0,0
exp        dd     0x0,0
new_dec    dd  100000,0
id         db     0x0,0

k8         dd 10000000
k8r        dq 0

res     dd   0

trans1  dq   0
trans2  dq   0

controlWord dw 1




ftoa:     ; fpu st0 -> [integer],[decimal]

    pusha

    fst    [tmp2]

    fstcw  [controlWord]      ; set truncate integer mode
    mov    ax,[controlWord]
    mov    [tmp], ax
    or     [tmp], word 0x0C00
    fldcw  [tmp]

    ftst                      ; test if st0 is negative
    fstsw  ax
    and    ax, 4500h
    mov    [sign], 0
    cmp    ax, 0100h
    jne    no_neg
    mov    [sign],1
  no_neg:

    fld    [tmp2]
    fistp  [integer]

    fld    [tmp2]
    fisub  [integer]

    fldcw  [controlWord]

    cmp  byte [sign], 0     ; change fraction to positive
    je   no_neg2
    fchs
  no_neg2:

    mov    [res],0          ; convert 6 decimal numbers
    mov    edi,6

   newd:

    fimul  [kymppi]
    fist   [decimal]

    mov    ebx,[res]
    imul   ebx,10
    mov    [res],ebx

    mov    eax,[decimal]
    add    [res],eax

    fisub  [decimal]

    fst    [tmp2]

    ftst
    fstsw  ax
    test   ax,1
    jnz    real_done

    fld    [tmp2]

    dec    edi
    jz     real_done

    jmp    newd

  real_done:

    mov    eax,[res]
    mov    [decimal],eax

    cmp    [integer],0x80000000      ; out of fpu limits
    jne    no_error
    mov    [integer],0
    mov    [decimal],0
    call   clear_all
    mov    [calc],'E'
  no_error:

    mov    [dsign],byte '+'
    cmp    [sign],byte 0             ; convert negative result
    je     no_negative
    mov    eax,[integer]
    not    eax
    inc    eax
    mov    [integer],eax
    mov    [dsign],byte '-'
  no_negative:

    call   to_muuta

    popa
    ret



atof:

    push ax
    push di

    fldz
    mov di, 0
    cmp si, 0
    je .error           ; Jump if string has 0 length.

    mov byte [sign], 0

    cmp byte [bx], '+'   ; Take care of leading '+' or '-'.
    jne .noPlus
    inc di
    jmp .noMinus
  .noPlus:

    cmp byte [bx], '-'
    jne .noMinus
    mov byte [sign], 1       ; Number is negative.
    inc di
  .noMinus:

    cmp si, di
    je .error

    call atof_convertWholePart
    jc .error

    call atof_convertFractionalPart
    jc .error

    cmp byte [sign], 0
    je  .dontNegate
    fchs           ; Negate value
  .dontNegate:

    mov bh, 0      ; Set bh to indicate the string is a valid number.
    jmp .exit

  .error:
    mov bh, 1      ; Set error code.
    fstp st0       ; Pop top of fpu stack.

  .exit:
    pop di
    pop ax

    ret



atof_convertWholePart:

    ; Convert the whole number part (the part preceding the decimal
    ; point) by reading a digit at a time, multiplying the current
    ; value by 10, and adding the digit.

.mainLoop:

    mov al, [bx + di]
    cmp al, '.'
    je .exit

    cmp al, '0'    ; Make sure character is a digit.
    jb .error
    cmp al, '9'
    ja .error

    ; Convert single character to digit and save to memory for
    ; transfer to the FPU.

    sub al, '0'
    mov ah, 0
    mov [tmp], ax

    ; Multiply current value by 10 and add in digit.

    fmul dword [ten]
    fiadd word [tmp]

    inc di
    cmp si, di     ; Jump if end of string has been reached.
    je .exit
    jmp .mainLoop

  .error:
    stc            ; Set error (carry) flag.
    ret

  .exit:
    clc            ; Clear error (carry) flag.
    ret


atof_convertFractionalPart:

    fld1      ; Load 1 to TOS.  This will be the value of the decimal place.

  .mainLoop:

    cmp si, di     ; Jump if end of string has been reached.
    je .exit

    inc di         ; Move past the decimal point.
    cmp si, di     ; Jump if end of string has been reached.
    je .exit
    mov al, [bx + di]

    cmp al, '0'    ; Make sure character is a digit.
    jb .error
    cmp al, '9'
    ja .error

    fdiv dword [ten]     ; Next decimal place

    sub al, '0'
    mov ah, 0
    mov [tmp], ax

    ; Load digit, multiply by value for appropriate decimal place,
    ; and add to current total.

    fild  word [tmp]
    fmul  st0, st1
    faddp st2, st0

    jmp .mainLoop

  .error:

    stc            ; Set error (carry) flag.
    fstp st0       ; Pop top of fpu stack.
    ret

  .exit:

    clc            ; Clear error (carry) flag.
    fstp st0       ; Pop top of fpu stack.
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
    mov  ebx,100*65536+230         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+185         ; [y start] *65536 + [y size]
    mov  edx,0x02ffffff            ; color of work area RRGGBB,8->color gl
    mov  esi,0x805977bb            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x005977bb            ; color of frames    RRGGBB
    int  0x40
                                   ; CLOSE BUTTON
    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,(230-18)*65536+12     ; [x start] *65536 + [x size]
    mov  ecx,4*65536+12            ; [y start] *65536 + [y size]
    mov  edx,1                     ; button id
    mov  esi,0x335599              ; button color RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ddeeff            ; color of text RRGGBB
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40

    mov  ebx,24*65536+25+3
    mov  ecx,70*65536+17
    mov  edx,6
    mov  esi,0x303090;cc2211
    mov  edi,7
  newbutton:
    dec  edi
    jnz  no_new_row
    mov  edi,6
    mov  ebx,24*65536+25+3
    add  ecx,20*65536
  no_new_row:
    mov  eax,8
    int  0x40
    add  ebx,30*65536
    inc  edx
    cmp  edx,11+24
    jbe  newbutton

    mov  eax,8                     ; CLEAR ALL
    mov  ebx,25*65536+27
    mov  ecx,41*65536+15
    mov  edx,2
    int  0x40

    mov  eax,8                     ; CHANGE DISPLAY TYPE
    mov  ebx,203*65536+7
    mov  ecx,28*65536+7
    mov  edx,3
    int  0x40

    mov  ebx,25*65536+35           ; draw info text with function 4
    mov  ecx,0xffffff ; 224466
    mov  edx,text
    mov  esi,30
  newline:
    mov  eax,4
    int  0x40
    add  ebx,10
    add  edx,30
    cmp  [edx],byte 'x'
    jne  newline

    call print_display

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret



print_display:

    pusha

    mov  eax,13
    mov  ebx,59*65536+143
    mov  ecx,24*65536+44
    mov  edx,0xffffff
    int  0x40

    mov  eax,4
    mov  ebx,120*65536+28
    mov  ecx,0
    mov  edx,calc
    mov  esi,1
    int  0x40

    mov  eax,4
    mov  ebx,175*65536+28
    mov  ecx,0
    mov  edx,[display_type]
    shl  edx,2
    add  edx,display_type_text
    mov  esi,3
    int  0x40

    cmp  [display_type],0   ; display as desimal
    jne  no_display_decimal

    mov  eax,47
    mov  ebx,11*65536   ; 11 decimal digits for 32 bits
    mov  ecx,[integer]
    mov  edx,84*65536+44
    mov  esi,0x0
    int  0x40

    mov  eax,47
    mov  ebx,6*65536
    mov  ecx,[decimal]
    mov  edx,156*65536+44
    mov  esi,0x0
    int  0x40

    mov  eax,4
    mov  ebx,150*65536+44
    mov  ecx,0x0
    mov  edx,dot
    mov  esi,1
    int  0x40

    mov  eax,4
    mov  ebx,77*65536+44
    mov  ecx,0x0
    mov  edx,dsign
    mov  esi,1
    int  0x40

  no_display_decimal:

    cmp  [display_type],1
    jne  no_display_hexadecimal

    mov  eax,4
    mov  ebx,138*65536+44
    mov  ecx,0x0
    mov  edx,dsign
    mov  esi,1
    int  0x40

    mov  eax,47
    mov  ebx,1*256+8*65536   ; 8 hexadecimal digits for 32 bits
    mov  ecx,[integer]
    mov  edx,144*65536+44
    mov  esi,0x0
    int  0x40

  no_display_hexadecimal:


    cmp  [display_type],2
    jne  no_display_binary

    mov  eax,4
    mov  ebx,96*65536+44
    mov  ecx,0x0
    mov  edx,dsign
    mov  esi,1
    int  0x40

    mov  eax,47
    mov  ebx,2*256+15*65536   ; 16 binary digits for 32 bits
    mov  ecx,[integer]
    mov  edx,102*65536+44
    mov  esi,0x0
    int  0x40

  no_display_binary:

    popa
    ret


clear_all:

    pusha

    mov  [calc],' '
    mov  [integer],0
    mov  [decimal],0
    mov  [id],0
    mov  [dsign],byte '+'
    mov  esi,muuta0
    mov  edi,muuta1
    mov  ecx,18
    cld
    rep  movsb
    mov  esi,muuta0
    mov  edi,muuta2
    mov  ecx,18
    cld
    rep  movsb
    call print_muuta
    call print_display

    popa
    ret



; DATA AREA

display_type       dd   0   ; 0 = decimal, 1 = hexadecimal, 2= binary
entry_multiplier   dd  10

display_start_y    dd  0x0
display_type_text  db 'dec hex bin'

dot     db  '.'

calc    db  ' '

integer dd    0
decimal dd    0

kymppi  dd   10

dsign:
muuta1  db   '+0000000000.000000'
muuta2  db   '+0000000000.000000'
muuta0  db   '+0000000000.000000'


text:
    db '                              '
    db '  C                           '
    db '                              '
    db '                              '
    db '  A    B    C    D    E    F  '
    db '                              '
    db '  1    2    3    +   SIN  INT '
    db '                              '
    db '  4    5    6    -   COS  LG2 '
    db '                              '
    db '  7    8    9    /   TAN   P  '
    db '                              '
    db ' -/+   0    .    *   SQR   =  '
    db '                              '
    db 'x <- END MARKER, DONT DELETE  '
asci:  db 49,50,51,52,53,54,55,56,57,48,43,61,45,42,47,44,46,52,13,97,98,99,100
       db 101,102,65,66,67,68,69,70,112,80,27,182
butid: db 12,13,14,18,19,20,24,25,26,31,15,35,21,33,27,32,32,33,35,6,7,8,9,10,11
       db 6,7,8,9,10,11,29,29,2,2

labelt:
      db   'FPU CALCULATOR'
labellen:

I_END:





