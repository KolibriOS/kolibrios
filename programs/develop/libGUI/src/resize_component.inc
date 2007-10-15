;****************************************************
;***************Resize of GUI component**************
;****************************************************
;IN
;ResizeComponent(dword Parend,dword Control,dword newx,dword newy)
;newy
;newx
;Control
;Parend
;OUT
;not

align 4

resize_component:

        mov ebx,[esp+4]  ;Control
        mov ecx,[esp+8] ;new size x
        mov edx,[esp+12] ;new size y

        mov [ebx+32],ecx
        mov [ebx+36],edx

        ret 12
