;------------------------------------------------------------------------------
;///// SORTING ////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

r_sort db 1
l_sort db 1

align 4
 compare_table:
  dd compare_by_name
  dd compare_by_extension

; AL = panel
__func mfar_sort
        pusha
        cmp     al,0
        jne    ._00
        mov     eax,[d_lcnt]
        mov     edi,lstart
        movzx   edx,[l_sort]
        jmp     @f
   ._00:
        mov     eax,[d_rcnt]
        mov     edi,rstart
        movzx   edx,[r_sort]
    @@:
        mov     edx,[edx*4+compare_table]
        cmp     eax,2
        jbe    .exit
        dec     eax
        shl     eax,4
        add     eax,edi
        cmp     word[edi],'..'
        jne     @f
        add     edi,16
    @@:
        push    edi
        mov     esi,edi
        add     edi,16
   .next_file:
        call    edx
        add     edi,16
        cmp     edi,eax
        jbe    .next_file

        mov     ebx,12
    @@:
        mov     ecx,[ebx+eax]
        xchg    [ebx+esi],ecx
        mov     [ebx+eax],ecx
        sub     ebx,4
        jge     @b

        mov     esi,[esp]
        mov     edi,esi
        add     edi,16

        sub     eax,16
        cmp     eax,edi
        jge    .next_file
        pop     edi

 .exit:
        popad
        ret
endf

;------------------------------------------------------------------------------

; ESI = pointer to 1st file info
; EDI = pointer to 2nd file info
__func compare_by_name
        mov     ecx,11
        push    edi esi
        repe    cmpsb
        pop     esi edi
        pushfd
        test    byte[esi+11],FA_FOLDER
        jnz    ._00
        test    byte[edi+11],FA_FOLDER
        jz     ._01
        jmp    ._03
   ._00:
        test    byte[edi+11],FA_FOLDER
        jz     ._02
   ._01:
        popfd
        jg     ._04
        mov     esi,edi
        ret
   ._02:
        popfd
        mov     esi,edi
        ret
   ._03:
        popfd
   ._04:
        ret
endf

;------------------------------------------------------------------------------

; ESI = pointer to 1st file info
; EDI = pointer to 2nd file info
__func compare_by_extension
        push    edi esi
        mov     ecx,3
        add     esi,8
        add     edi,8
        repe    cmpsb
        jne    ._00
        mov     esi,[esp]
        mov     edi,[esp+4]
        mov     ecx,8
        repe    cmpsb
   ._00:
        pop     esi edi
        pushfd
        test    byte[esi+11],FA_FOLDER
        jnz    ._01
        test    byte[edi+11],FA_FOLDER
        jz     ._02
        jmp    ._04
   ._01:
        test    byte[edi+11],FA_FOLDER
        jz     ._03
   ._02:
        popfd
        jg     ._05
        mov     esi,edi
        ret
   ._03:
        popfd
        mov     esi,edi
        ret
   ._04:
        popfd
   ._05:
        ret
endf
