;;   Calculator for MenuetOS (c) Ville Turjanmaa
;;  
;;   Compile with FASM
;;   
;;   Pavel Rymovski (Heavyiron) - version for KolibriOS
;;
;; What's new:
;;   Calc 1.1
;;           1) changed design
;;           2) new procedure of draw window (10 decimal digits, 23 binary, "+" not displayed now)
;;           3) window with skin
;;   Calc 1.2
;;           1)added some useful functions, such as arcsin, arccos, arctg, 1/x, x^2
;;   Calc 1.31
;;           1)optimised program
;;           2)new type of window (you need kernel 114 revision or higher)
;;   Calc 1.32
;;           1)fixed arccos


appname equ 'Calc '
version    equ '1.32'

use32
               org    0x0
               db    'MENUET01'            ; 8 byte id
               dd     0x01                      ; header version
               dd     START                    ; start of code
               dd     I_END                    ; size of image
               dd     0x1000                  ; memory for app
               dd     0x1000                  ; esp
               dd     0x0,0x0                 ; I_Param , I_Icon

include '..\..\..\macros.inc'

START:

 red:
    call draw_window

 still:  
    push 10 
    pop eax 
    mcall

    dec eax
    jz red
    dec eax 
    jz key 
 
  button:
    mov  al,17      ; получить идентификатор нажатой кнопки
    mcall
    shr  eax,8
    jmp  testbut

  key:         
    mov  al,2       ; получить ASCII-код нажатой клавиши
    mcall
    shr  eax,8
    mov  edi,asci   ; перевод ASCII в идентификатор кнопки
    mov  ecx,18
    cld
    repne scasb
    jne  still
    sub  edi,asci
    dec  edi
    mov  esi,butid
    add  esi,edi
    lodsb
     
  testbut:
    cmp  eax,1      ; кнопка 1 - закрытие программы
    jne  noclose
    or   eax,-1
    mcall 
 
  noclose:
    cmp  eax,2
    jne  no_reset
    call clear_all
    jmp  still
  
  no_reset:
    finit
    mov  ebx,muuta1  ; Перевод в формат FPU
    mov  esi,18
    call atof
    fstp [trans1]
    mov  ebx,muuta2
    mov  esi,18
    call atof
    fst  [trans2]
    
    cmp  eax,33
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

  no_display_change:
    cmp  eax,6
    jb   no_a_f
    cmp  eax,11
    jg   no_a_f
    add  eax,4
    call number_entry
    jmp  still
   
   no_a_f:
    cmp  eax,12
    jb   no_13
    cmp  eax,14
    jg   no_13
    sub  eax,11
    call number_entry
    jmp  still
   
   no_13:
    cmp  eax,19
    jb   no_46
    cmp  eax,21
    jg   no_46
    sub  eax,15
    call number_entry
    jmp  still
   
   no_46:
    cmp  eax,26
    jb   no_79
    cmp  eax,28
    jg   no_79
    sub  eax,19
    call number_entry
    jmp  still
   
   no_79:
    cmp  eax,34
    jne  no_0
    xor  eax,eax
    call number_entry
    jmp  still
   
   no_0:
    cmp  eax,35
    jne  no_id
    inc  [id]
    and  [id],1
    mov  [new_dec],100000
    jmp  still
  
  no_id:
    cmp  eax,17
    jne  no_sin
    fld  [trans1]
    fsin
    jmp  show_result
  
  no_sin:
    cmp  eax,18
    jne  no_asin
    fld  [trans1]
    fld  st0
    fmul st,st1
    fld1
    fsubrp  st1,st0
    fsqrt
    fpatan
    jmp  show_result
  
  no_asin:
    cmp  eax,16
    jne  no_int
    fld  [trans1]
    frndint
    jmp  show_result
  
  no_int:
    cmp  eax,23
    jne  no_1x
    fld1
    fdiv [trans1]
    jmp  show_result
    
  no_1x:  
    cmp  eax,24
    jne  no_cos
    fld  [trans1]
    fcos
    jmp  show_result
  
  no_cos:
    cmp  eax,25
    jne  no_acos
    fld  [trans1]
    fld st0
    fmul st,st1
    fld1
    fsubrp st1,st0
    fsqrt
    fxch st1
    fpatan
    jmp  show_result
  
  no_acos:   
    cmp  eax,30
    jne  no_x2
    fld  [trans1]
    fmul st,st0
    jmp  show_result
    
  no_x2:  
    cmp  eax,31
    jne  no_tan
    fld  [trans1]
    fptan
    fstp st2
    jmp  show_result
  
  no_tan:
    cmp  eax,32
    jne  no_atan
    fld  [trans1]
    fld1
    fpatan
    jmp  show_result
   
   no_atan:
    cmp  eax,38
    jne  no_pi
    fldpi
    jmp  show_result
   
   no_pi:
    cmp  eax,37
    jne  no_sqrt
    fld  [trans1]
    fsqrt
    jmp  show_result
  
  no_sqrt:
    cmp  eax,15
    jne  no_add
    call calculate
    call new_entry
    mov  [calc],'+'
    jmp  still
  
  no_add:
    cmp  eax,22
    jne  no_sub
    call calculate
    call new_entry
    mov  [calc],'-'
    jmp  still
  
  no_sub:
    cmp  eax,29
    jne  no_div
    call calculate
    call new_entry
    mov  [calc],'/'
    jmp  still
  
  no_div:
    cmp  eax,36
    jne  no_mul
    call calculate
    mov  [calc],'*'
    call new_entry
    jmp  still
  
  no_mul:
    cmp    eax,39
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
   
  no_calculation:
    call   print_display
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
    mov  edi,muuta1+10     ; целое
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
    mov  edi,muuta1+17     ; дробное
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


ftoa:                         ; fpu st0 -> [integer],[decimal]
    pusha
    fst    [tmp2]
    fstcw  [controlWord]      ; set truncate integer mode
    mov    ax,[controlWord]
    mov    [tmp], ax
    or     [tmp], word 0x0c00
    fldcw  [tmp]
    ftst                      ; test if st0 is negative
    fstsw  ax
    and    ax, 0x4500
    mov    [sign], 0
    cmp    ax, 0x0100
    jne    no_neg
    mov    [sign],1
  
  no_neg:
    fld    [tmp2]
    cmp    byte [sign], 0     ; change fraction to positive
    je     no_neg2
    fchs
 
  no_neg2:
    fadd   [smallValueForRounding]
    fist   [integer]
    fisub  [integer]
    mov    [res],0     ; convert 6 decimal numbers
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
    jz	   real_done
    jmp    newd

  real_done:
    fldcw  [controlWord]
    mov    eax,[res]
    mov    [decimal],eax
    cmp    [integer],0x80000000
    jne    no_error
    call   clear_all
    mov    [calc],'E'
  
  no_error:
    mov    [dsign],byte '+'
    cmp    [sign],byte 0      ; convert negative result
    je     no_negative
;    mov    eax,[integer]
;    not    eax
;    inc    eax
;    mov    [integer],eax
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
    je .error            ; Jump if string has 0 length.
    mov byte [sign], 0
    cmp byte [bx], '+'   ; Take care of leading '+' or '-'.
    jne .noPlus
    inc di
    jmp .noMinus
  
  .noPlus:
    cmp byte [bx], '-'
    jne .noMinus
    mov byte [sign], 1   ; Number is negative.
    inc di
  
  .noMinus:
    cmp si, di
    je .error
    call atof_convertWholePart
    jc .error
    call atof_convertFractionalPart
    jc .error
    cmp byte [sign], 0
    je .dontNegate
    fchs    ; Negate value
 
  .dontNegate:
    mov bh, 0    ; Set bh to indicate the string is a valid number.
    jmp .exit

  .error:
    mov bh, 1    ; Set error code.
   ; fstp st0    ; Pop top of fpu stack.

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
    cmp al, '0'       ; Make sure character is a digit.
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
    cmp si, di         ; Jump if end of string has been reached.
    je .exit
    jmp .mainLoop

  .error:
    stc                ; Set error (carry) flag.
    ret

  .exit:
    clc                ; Clear error (carry) flag.
    ret


atof_convertFractionalPart:
    fld1               ; Load 1 to TOS.  This will be the value of the decimal place.

  .mainLoop:
    cmp si, di         ; Jump if end of string has been reached.
    je .exit
    inc di             ; Move past the decimal point.
    cmp si, di         ; Jump if end of string has been reached.
    je .exit
    mov al, [bx + di]
    cmp al, '0'        ; Make sure character is a digit.
    jb .error
    cmp al, '9'
    ja .error
    fdiv dword [ten]   ; Next decimal place
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
    stc           ; Set error (carry) flag.
    fstp st0    ; Pop top of fpu stack.
    ret

  .exit:
    clc              ; Clear error (carry) flag.
    fstp st0    ; Pop top of fpu stack.
    ret

;   *********************************************
;   ******* WINDOW DEFINITIONS AND DRAW *********
;   *********************************************

draw_window:
    
    mov  eax,12
    mov  ebx,1
    mcall
                                   
    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    mcall

    mov  eax,48
    mov  ebx,4
    int  0x40
    mov  ecx, eax
    xor  eax,eax                     
    mov  ebx,200 shl 16+256        
    add  ecx,200 shl 16+158
    mov  edx,[sc.work]
    or   edx,0x34000000
    mov  edi,title
    mcall

    mov  eax,8
    mov  ebx,19 shl 16+28
    mov  ecx,49 shl 16+18
    mov  edx,6
    mov  esi,[sc.work_button]
    mov  edi,7
  newbutton:
    dec  edi
    jnz  no_new_row
    mov  edi,7
    mov  ebx,19 shl 16+28
    add  ecx,20 shl 16
  no_new_row:
    mcall
    add  ebx,30 shl 16
    inc  edx
    cmp  edx,39
    jbe  newbutton
  
    mcall  ,199 shl 16+28,49 shl 16+18,2               ; 'C'
    mcall  ,220 shl 16+8,7 shl 16+8,3                     ; 'dec-bin-hex'

    mov  eax,4
    mov  ebx,27 shl 16+55
    mov  ecx,[sc.work_button_text]
    mov  edx,text
    mov  esi,33
  newline:
    mcall
    add  ebx,20
    add  edx,33
    cmp  [edx],byte 'x'
    jne  newline
    
    call print_display
    
    mov  eax,12
    mov  ebx,2
    mcall

    ret

print_display:
    pusha
    mcall 13,18 shl 16+210,19 shl 16+13,0xffffff

    mov  eax,4
    mov  ebx,135 shl 16+7
    mov  ecx,[sc.work_text]
    or   ecx,0x40000000
    mov  edx,calc
    mov  esi,1
    mov  edi,[sc.work]
    mcall

    mov  ebx,198 shl 16+8
    mov  edx,[display_type]
    shl  edx,2
    add  edx,display_type_text
    mov  esi,3
    mov  edi,[sc.work]
    mcall
   
    cmp  [dsign],byte '+'
    je   positive
    mov  ebx,23 shl 16+22
    mov  ecx,0x0
    mov  edx,dsign
    mov  esi,1
    mcall  
 
positive:  
    cmp  [display_type],0
    jne  no_display_decimal
    cmp  [decimal],0
    je   whole

    mov  ebx,180 shl 16+22
    mov  ecx,0x0
    mov  edx,dot
    mov  esi,1
    mcall
    
    mov  eax,47
    mov  ebx,10 shl 16
    mov  ecx,[integer]
    mov  edx,120 shl 16+22
    mov  esi,0x0
    mcall     
    
    mov  ebx,6 shl 16
    mov  ecx,[decimal]
    mov  edx,187 shl 16+22     
    mov  esi,0x0
    mcall 

    popa
    ret
    
whole:
    mov  ebx,220 shl 16+22
    mov  ecx,0x0
    mov  edx,dot
    mov  esi,1
    mcall

    cmp  [integer],0
    je  null

    mov  eax,47
    mov  ebx,10 shl 16
    mov  ecx,[integer]
    mov  edx,160 shl 16+22
    mov  esi,0x0
    mcall

    popa
    ret
              
  no_display_decimal:
    cmp  [display_type],1
    jne  no_display_hexadecimal
    cmp  [integer],0
    je  null
    
    mov  eax,47
    mov  ebx,256+8 shl 16
    mov  ecx,[integer]
    mov  edx,173 shl 16+22
    mov  esi,0x0
    mcall

    popa
    ret
 
  no_display_hexadecimal:
    cmp  [integer],0
    je  null

    mov  eax,47
    mov  ebx,2*256+32 shl 16
    mov  ecx,[integer]
    mov  edx,32 shl 16+22
    mov  esi,0x0
    mcall

    popa
    ret
  
  null:
    mov  eax,47
    mov  ebx,1 shl 16
    mov  ecx,0
    mov  edx,214 shl 16+22
    mov  esi,0x0
    mcall

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
    call print_display
    popa
    ret


;data

title db appname,version,0

display_type       dd  0    ; 0 = decimal, 1 = hexadecimal, 2= binary
entry_multiplier   dd  10
display_type_text  db  'dec hex bin'

dot           db  '.'
calc          db  ' '
integer       dd  0
decimal       dd  0
kymppi        dd  10
ten           dd  10.0,0
tmp           dw  1,0
sign          db  1,0
tmp2          dq  0x0,0
exp           dd  0x0,0
new_dec       dd  100000,0
id            db  0x0,0
res           dd  0
trans1        dq  0
trans2        dq  0
controlWord   dw  1
smallValueForRounding dq 0.0000005 ; 1/2 from last significant digit
multipl:      dd  10,16,2

dsign:
muuta1        db  '+0000000000.000000'
muuta2        db  '+0000000000.000000'
muuta0        db  '+0000000000.000000'

text:
    db ' A    B    C    D    E    F    C '
    db ' 1    2    3    +   Int  Sin Asin'
    db ' 4    5    6    -   1/x  Cos Acos'
    db ' 7    8    9    /   x^2  Tan Atan'
    db '+/-   0    .    *   Sqr  Pi    = '
    db 'x'

asci:  db 49,50,51,52,53,54,55,56,57,48,43,61,45,42,47,44,46,27
butid: db 12,13,14,19,20,21,26,27,28,34,15,39,22,36,29,35,35,1

I_END:

sc     system_colors
