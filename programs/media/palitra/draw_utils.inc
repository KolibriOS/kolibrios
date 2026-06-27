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