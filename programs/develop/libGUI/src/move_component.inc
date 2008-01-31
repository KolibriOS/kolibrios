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

        push ebx
        push ecx
        push edx
        push ebp

        mov ebx,[esp+16+4]  ;Control
        mov ecx,[esp+16+8]  ;new x
        mov edx,[esp+16+12] ;new y

        mov [ebx+24],ecx
        mov [ebx+28],edx

        pop ebp
        pop edx
        pop ecx
        pop ebx

        ret 12
