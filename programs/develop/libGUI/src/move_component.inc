;****************************************************
;*****************move of GUI component**************
;****************************************************
;IN
;MoveComponent(dword Parend,dword Control,dword newx,dword newy)
;newy
;newx
;Control
;Parend
;OUT
;not

align 4

move_component:

        mov ebx,[esp+4]  ;Control
        mov ecx,[esp+8]  ;new x
        mov edx,[esp+12] ;new y

        mov [ebx+24],ecx
        mov [ebx+28],edx

        ret 12
