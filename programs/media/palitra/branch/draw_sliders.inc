    ;.................................................................................................
    ; Отрисовка слайдеров
    ;.................................................................................................
    ; Функция 13 - нарисовать прямоугольник

     mov ebx, 18*65536+18
     mov ecx, 61*65536+61

     push dword 0xFC5663
     push dword 0xFA0919
     push dword 0xE20516
     push dword 0xFA0919

     call Draw_base

     add ebx, 20*65536+20

     push dword 0x33F742
     push dword 0x0ADD18
     push dword 0x08CE19
     push dword 0x08CE19

     call Draw_base

     add ebx, 20*65536+20

     push dword 0x88A6FF
     push dword 0x3366FF
     push dword 0x114DFF
     push dword 0x114DFF

     call Draw_base

     add ebx, 20*65536+20

     push dword 0xDADADA
     push dword 0xB9B9B9
     push dword 0xA9A9A9
     push dword 0xA9A9A9

     call Draw_base

    ;.................................................................................................
    ; Отрисовка ползунков
    ;.................................................................................................
     mov ebx, 16*65536+16
     mov ecx, 61*65536+61
     movzx   eax,[cred]
     call draw_slider

     mov ebx, (16+25*1)*65536+(16+25*1)
     mov ecx, 61*65536+61
     movzx   eax,[cgreen]
     call draw_slider

     mov ebx, (16+25*2)*65536+(16+25*2)
     mov ecx, 61*65536+61
     movzx   eax,[cblue]
     call draw_slider

     mov ebx, (16+25*3)*65536+(16+25*3)
     mov ecx, 61*65536+61
     movzx   eax,[calpha]
     call draw_slider


    ret                                   ; Возвращаем управление
;end_draw_result

Draw_base:
push ebp
mov ebp, esp

add ebx, 1*65536+4
mcall SF_DRAW_LINE, , , 0
add ecx, 128*65536+128
int     0x40

sub ebx, 1*65536+4
sub ecx, (128-1)*65536+1
int     0x40

add ebx, 1*65536+1
mov edx, dword [ebp+20]
int     0x40

add ebx, 1*65536+1
mov edx, dword [ebp+16]
int     0x40

add ebx, 1*65536+1
mov edx, dword [ebp+12]
int     0x40

add ebx, 1*65536+1
mov edx, dword [ebp+8]
int     0x40

add ebx, 1*65536+1
mov edx, 0
int     0x40

sub ecx, 1*65536+(128-1)

pop ebp
ret 4*4



draw_slider:
shr eax, 1
mov edx, eax
shl edx, 16
or eax, edx
sub ecx, eax
add ecx, 127*65536+127

add ecx, 1*65536+2
mcall SF_DRAW_LINE, , , 0
add ebx, 9*65536+9
int     0x40

sub ebx, 8*65536+1
sub ecx, 1*65536+2
int     0x40

mov edx, 0xE1E1E1
add ecx, 1*65536+1
int     0x40
add ecx, 1*65536+1
int     0x40

mov edx, 0
add ecx, 1*65536+1
int     0x40

shr ebx, 16
shr ecx, 16
dec ecx
mcall SF_PUT_PIXEL, , , 0xF3F3F3
int     0x40

ret

;end_slider