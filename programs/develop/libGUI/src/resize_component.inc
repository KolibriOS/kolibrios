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

        push ebx
        push ecx
        push edx
        push ebp

        mov ebx,[esp+16+4]  ;Control
        mov ecx,[esp+16+8]  ;new size x
        mov edx,[esp+16+12] ;new size y

        mov [ebx+32],ecx
        mov [ebx+36],edx

        pop ebp
        pop edx
        pop ecx
        pop ebx

        ret 12
