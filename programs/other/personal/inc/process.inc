process_slot:
    ;..........................................................................
    ; Получаем позицию окна потока в оконном стэке
    ;..........................................................................
    mov     eax,9
    mov     ebx,buffer
    mov     ecx,-1
    int     0x40
    xor     ecx,ecx
    mov     cx,word [ebx+4]
    cmp     eax,ecx
    je      @f
    mov     [win_slot],0
    ret
  @@:
    mov     [win_slot],1
    ret

    ;mov     [win_slot],eax
    ;mov     cx,word [ebx+4]
    ;mov     [win_stack],eax

    ;mov     eax,18
    ;mov     ebx,7
    ;int     0x40
    ;mov     [win_slot],eax
    ret
;end_process_info
