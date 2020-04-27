;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2020. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 7825 $


align 4
proc sys_array
        cmp     ebx, 0
        je      sys_init_array
        ret
endp

align 4
proc sys_init_array
        push    ecx
        push    edx
        call    sys_index_array
        ret
endp

align 4
sys_index_array:
        enter   8, 0
        mov     eax, [ebp+8]
        and     eax, 3
        mov     [ebp-4], eax
        mov     eax, [ebp-4]
        shl     eax, 2
        add     eax, [ebp+0Ch]
        mov     [ebp-8], eax

        mov     eax, [ebp-4]
        cmp     [ebp+8], eax
        jne     .A4F4h
        push    10h
        pop     eax
        add     eax, [ebp-8]
        jmp     .A516h

.A4F4h:
        mov     esi, [ebp-8]
        cmp     dword [esi], 0
        jne     .A508h
        mov     eax, 68
        mov     ebx, 12
        mov     ecx, 32
        call    malloc
        mov     esi, [ebp-8]
        mov     [esi], eax

.A508h:
        push    dword[esi]
        mov     eax, [ebp+8]
        shr     eax, 2
        push    eax
        call    sys_index_array

.A516h:
        leave
        ret     8
