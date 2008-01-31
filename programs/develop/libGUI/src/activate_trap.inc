;*******************************************************
;*Function activate a trap for the specialized message.*
;*******************************************************

;IN
;void ActivateTrapForSpecializedMessage(dword *control)

activate_trap_for_specialized_message:

        push eax
        push ebx

        mov ebx,[esp+8+4]
        xor eax,eax
        mov al,[ebx+control_header_size]
        or  al,activate_trap
        mov [ebx+control_header_size],al

        pop ebx
        pop eax

        ret 4