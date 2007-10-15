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

remove_component:

        mov ebx,[esp+4]  ;Control
        mov ecx,[esp+8]  ;new x
        mov edx,[esp+12] ;new y

        mov [ebx+24],ecx
        mov [ebx+28],edx

        ret 12
