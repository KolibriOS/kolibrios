;*******************************************************
;*Function activate a trap for the specialized message.*
;*******************************************************

;IN
;void ActivateTrapForSpecializedMessage(dword *control)

activate_trap_for_specialized_message:

        mov ebx,[esp+4]
        xor eax,eax
        mov al,[ebx+control_header_size]
        or  al,activate_trap
        mov [ebx+control_header_size],al

        ret 4