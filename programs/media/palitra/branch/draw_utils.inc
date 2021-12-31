; DrawRectangle3D not work
proc DrawRectangle3D, _x:word, _y:word, _w:word, _h:word, _color1:dword, _color2:dword
    mov ebx,dword[_w]
    add ebx,dword[_x] ;тут точно какая-то дичь... нужно переписать :)
    shl ebx,16
    mov bx,1
    mov ecx,dword[_h]
    inc cx
    mov edx,[_color2]
    mcall SF_DRAW_RECT ; x+w,y,1,h+1,color2
    rol ebx,16
    sub bx,[_w]
    ror ebx,16
    dec cx
    mov edx,[_color1]
    mcall ;SF_DRAW_RECT x,y,1,h,color1
    mov bx,[_h]
    xchg bx,cx
    mcall ;SF_DRAW_RECT x,y,w,1,color1
    rol ecx,16
    add cx,[_h]
    ror ecx,16
    mov edx,[_color2]
    mcall ;SF_DRAW_RECT x,y+h,w,1,color2
        ret
endp



proc DrawDeepRectangle, _x:word, _y:word, _w:word, _h:word, color1:dword, color2:dword
pusha

movzx ebx, [_x]
shl ebx, 16
or ebx, dword[_x]
add ebx, dword[_w]

movzx ecx, [_y]
shl ecx, 16
add ecx, dword[_y]

mcall SF_DRAW_LINE, , ,[color1] ; x+w,y,1,h+1,color2

sub ebx, dword[_w]
add ecx, dword[_h]
int 0x40

mov edx, dword[_w]
shl edx, 16
or edx, dword[_w]
add ebx, edx
mcall SF_DRAW_LINE, , ,[color2]



mov edx, dword[_w]
shl edx, 16
sub ebx, edx

mov edx, dword[_h]
shl edx, 16
add ecx, edx

mcall SF_DRAW_LINE, , ,[color2]


popa
        ret
endp