;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 4850 $


;------------------------------------------------------------------------------
align 4
sys_clipboard:
        xor     eax, eax
        dec     eax
; check availability of main list
        cmp     [clipboard_main_list], eax
        je      .exit_1 ; main list area not found

        test    ebx, ebx  ; 0 - Get the number of slots in the clipboard
        jnz     .1
; get the number of slots
        mov     eax, [clipboard_slots]
        jmp     .exit_1
;------------------------------------------------------------------------------
align 4
.1:
        dec     ebx  ; 1 - Read the data from the clipboard
        jnz     .2
; verify the existence of slot
        cmp     ecx, [clipboard_slots]
        jae     .exit_2
; get a pointer to the data of slot
        shl     ecx, 2
        add     ecx, [clipboard_main_list]
        mov     esi, [ecx]
        mov     ecx, [esi]
; allocate memory for application for copy the data of slots
        push    ecx
        stdcall user_alloc, ecx
        pop     ecx
; copying data of slots
        mov     edi, eax
        cld
        rep movsb
        jmp     .exit_1
;------------------------------------------------------------------------------
align 4
.2:
        dec     ebx  ; 2 - Write the data to the clipboard
        jnz     .3
; check the lock
        mov     ebx, clipboard_write_lock
        xor     eax, eax
        cmp     [ebx], eax
        jne     .exit_2
; lock last slot
        inc     eax
        mov     [ebx], eax
; check the overflow pointer of slots
        cmp     [clipboard_slots], 1024
        jae     .exit_3
; get memory for new slot
        push    ebx ecx edx
        stdcall kernel_alloc, ecx
        pop     edx ecx ebx
        test    eax, eax
        jz      .exit_3
; create a new slot
        mov     edi, eax
        mov     eax, [clipboard_slots]
        shl     eax, 2
        add     eax, [clipboard_main_list]
        mov     [eax], edi
; copy the data into the slot
        mov     esi, edx
        mov     eax, ecx
        cld
        stosd  ; store size of slot
        sub     ecx, 4
        add     esi, 4
        rep movsb ; store slot data
; increase the counter of slots
        inc     [clipboard_slots]
; unlock last slot
        xor     eax, eax
        mov     [ebx], eax
        jmp     .exit_1
;------------------------------------------------------------------------------
align 4
.3:
        dec     ebx  ; 3 - Delete the last slot in the clipboard
        jnz     .4
; check the availability of slots
        mov     eax, [clipboard_slots]
        test    eax, eax
        jz      .exit_2
; check the lock
        mov     ebx, clipboard_write_lock
        xor     eax, eax
        cmp     [ebx], eax
        jne     .exit_2
; lock last slot
        inc     eax
        mov     [ebx], eax
; decrease the counter of slots
        mov     eax, clipboard_slots
        dec     dword [eax]
; free of kernel memory allocated for the slot
        mov     eax, [eax]
        shl     eax, 2
        add     eax, [clipboard_main_list]
        mov     eax, [eax]
        push    ebx
        stdcall kernel_free, eax
        pop     ebx
; unlock last slot
        xor     eax, eax
        mov     [ebx], eax
        jmp     .exit_1
;------------------------------------------------------------------------------
align 4
.4:
        dec     ebx  ; 4 - Emergency discharge of clipboard
        jnz     .exit
; check the lock
        mov     ebx, clipboard_write_lock
        xor     eax, eax
        cmp     [ebx], eax
        je      .exit_2

; there should be a procedure for checking the integrity of the slots 
; and I will do so in the future

; unlock last slot
        mov     [ebx], eax
        jmp     .exit
;------------------------------------------------------------------------------
align 4
.exit_3:
; unlock last slot
        xor     eax, eax
        mov     [ebx], eax
.exit_2:
        xor     eax, eax
        inc     eax     ; error
.exit_1:
        mov     [esp + 32], eax
.exit:
        ret
;------------------------------------------------------------------------------
uglobal
align 4
clipboard_slots dd ?
clipboard_main_list dd ?
clipboard_write_lock dd ?
endg
;------------------------------------------------------------------------------
