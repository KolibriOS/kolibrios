;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$



MEM_LIST_OFFSET equ  8
FREE_BLOCK      equ  4
USED_BLOCK      equ  8
DONT_FREE_BLOCK equ  10h



;;;;;;;;;;;;;;      USER     ;;;;;;;;;;;;;;;;;

HEAP_TOP  equ 0x5FC00000

align 4
proc init_heap

           mov ebx,[current_slot]
           mov eax, [ebx+APPDATA.heap_top]
           test eax, eax
           jz @F
           sub eax,[ebx+APPDATA.heap_base]
           sub eax, 4096
           ret
@@:
           mov esi, [ebx+APPDATA.mem_size]
           add esi, 4095
           and esi, not 4095
           mov [ebx+APPDATA.mem_size], esi
           mov eax, HEAP_TOP
           mov [ebx+APPDATA.heap_base], esi
           mov [ebx+APPDATA.heap_top], eax

           sub eax, esi
           shr esi, 10
           mov ecx, eax
           sub eax, 4096
           or ecx, FREE_BLOCK
           mov [page_tabs+esi], ecx
           ret
endp

align 4
_UserAlloc:
proc user_alloc stdcall, alloc_size:dword

           push ebx
           push esi
           push edi

           mov ecx, [alloc_size]
           add ecx, (4095+4096)
           and ecx, not 4095

           mov ebx, [current_slot]
           mov esi, dword [ebx+APPDATA.heap_base]  ; heap_base
           mov edi, dword [ebx+APPDATA.heap_top]   ; heap_top
l_0:
           cmp esi, edi
           jae m_exit

           mov ebx, esi
           shr ebx, 12
           mov eax, [page_tabs+ebx*4]
           test al, FREE_BLOCK
           jz test_used
           and eax, 0xFFFFF000
           cmp eax, ecx    ;alloc_size
           jb  m_next
	   jz  @f

           lea edx, [esi+ecx]
           sub eax, ecx
           or al, FREE_BLOCK
           shr edx, 12
           mov [page_tabs+edx*4], eax
@@:
           or ecx, USED_BLOCK
           mov [page_tabs+ebx*4], ecx
           shr ecx, 12
           inc ebx
           dec ecx
           jz  .no
@@:
           mov dword [page_tabs+ebx*4], 2
           inc ebx
           dec ecx
           jnz @B
.no:

           mov     edx, [current_slot]
           mov     ebx, [alloc_size]
           add     ebx, 0xFFF
           and     ebx, not 0xFFF
           add     ebx, [edx+APPDATA.mem_size]
           call    update_mem_size

           lea eax, [esi+4096]

           pop edi
           pop esi
           pop ebx
           ret
test_used:
           test al, USED_BLOCK
           jz m_exit

           and eax, 0xFFFFF000
m_next:
           add esi, eax
           jmp l_0
m_exit:
           xor eax, eax
           pop edi
           pop esi
           pop ebx
           ret
endp

align 4
_UserFree:
proc user_free stdcall, base:dword

           push esi

           mov esi, [base]
           test esi, esi
           jz .exit

           push ebx

           xor ebx, ebx
           shr esi, 12
           mov eax, [page_tabs+(esi-1)*4]
           test al, USED_BLOCK
           jz .cantfree

           test al, DONT_FREE_BLOCK
           jnz .cantfree

           push edi

           and eax, not 4095
           mov edi, eax
           or al, FREE_BLOCK
           mov [page_tabs+(esi-1)*4], eax
           sub edi, 4096
           mov ebx, edi
           shr edi, 12
           jz .released

.release:
           xor ecx, ecx
           xchg ecx, [page_tabs+esi*4]
           test cl, 1
           jz @F

           call @core_free@4
           mov eax, esi
           shl eax, 12
           invlpg [eax]
@@:
           inc esi
           dec edi
           jnz .release
.released:
           mov edx, [current_slot]
           mov esi, dword [edx+APPDATA.heap_base]
           mov edi, dword [edx+APPDATA.heap_top]
           sub ebx, [edx+APPDATA.mem_size]
           neg ebx
           call update_mem_size
           call user_normalize
           pop edi
           pop ebx
           pop esi
           ret
.exit:
           xor eax, eax
           inc eax
           pop esi
           ret
.cantfree:
           xor eax, eax
           pop ebx
           pop esi
           ret
endp

user_normalize:
; in: esi=heap_base, edi=heap_top
; out: eax=0 <=> OK
; destroys: ebx,edx,esi,edi
           shr esi, 12
           shr edi, 12
@@:
           mov eax, [page_tabs+esi*4]
           test al, USED_BLOCK
           jz .test_free
           shr eax, 12
           add esi, eax
           jmp @B
.test_free:
           test al, FREE_BLOCK
           jz .err
           mov edx, eax
           shr edx, 12
           add edx, esi
           cmp edx, edi
           jae .exit

           mov ebx, [page_tabs+edx*4]
           test bl, USED_BLOCK
           jz .next_free

           shr ebx, 12
           add edx, ebx
           mov esi, edx
           jmp @B
.next_free:
           test bl, FREE_BLOCK
           jz .err
           and dword [page_tabs+edx*4], 0
           add eax, ebx
           and eax, not 4095
           or eax, FREE_BLOCK
           mov [page_tabs+esi*4], eax
           jmp @B
.exit:
           xor eax, eax
           inc eax
           ret
.err:
           xor eax, eax
           ret

user_realloc:
; in: eax = pointer, ebx = new size
; out: eax = new pointer or NULL
        test    eax, eax
        jnz     @f
; realloc(NULL,sz) - same as malloc(sz)
        push    ebx
        call    user_alloc
        ret
@@:
        push    ecx edx
        lea     ecx, [eax - 0x1000]
        shr     ecx, 12
        mov     edx, [page_tabs+ecx*4]
        test    dl, USED_BLOCK
        jnz     @f
; attempt to realloc invalid pointer
.ret0:
        pop     edx ecx
        xor     eax, eax
        ret
@@:
        test    dl, DONT_FREE_BLOCK
        jnz     .ret0
        add     ebx, 0x1FFF
        shr     edx, 12
        shr     ebx, 12
; edx = allocated size, ebx = new size
        add     edx, ecx
        add     ebx, ecx
        cmp     edx, ebx
        jb      .realloc_add
; release part of allocated memory

        push ecx
.loop:
        cmp     edx, ebx
        jz      .release_done
        dec     edx
        xor     ecx, ecx
        xchg    ecx, [page_tabs+edx*4]
        test    al, 1
        jz      .loop

        push edx
        call    @core_free@4
        pop edx
        mov     eax, edx
        shl     eax, 12
        invlpg  [eax]
        jmp     .loop
.release_done:

        pop ecx

        sub     ebx, ecx
        cmp     ebx, 1
        jnz     .nofreeall
        mov     eax, [page_tabs+ecx*4]
        and     eax, not 0xFFF
        mov     edx, [current_slot]
        mov     ebx, [APPDATA.mem_size+edx]
        sub     ebx, eax
        add     ebx, 0x1000
        or      al, FREE_BLOCK
        mov     [page_tabs+ecx*4], eax
        push    esi edi
        mov     esi, [APPDATA.heap_base+edx]
        mov     edi, [APPDATA.heap_top+edx]
        call    update_mem_size
        call    user_normalize
        pop     edi esi
        jmp     .ret0   ; all freed
.nofreeall:
        sub     edx, ecx
        shl     ebx, 12
        or      ebx, USED_BLOCK
        xchg    [page_tabs+ecx*4], ebx
        shr     ebx, 12
        sub     ebx, edx
        push    ebx ecx edx
        mov     edx, [current_slot]
        shl     ebx, 12
        sub     ebx, [APPDATA.mem_size+edx]
        neg     ebx
        call    update_mem_size
        pop     edx ecx ebx
        lea     eax, [ecx+1]
        shl     eax, 12
        push    eax
        add     ecx, edx
        lea     edx, [ecx+ebx]
        shl     ebx, 12
        jz      .ret
        push    esi
        mov     esi, [current_slot]
        mov     esi, [APPDATA.heap_top+esi]
        shr     esi, 12
@@:
        cmp     edx, esi
        jae     .merge_done
        mov     eax, [page_tabs+edx*4]
        test    al, USED_BLOCK
        jnz     .merge_done
        and     dword [page_tabs+edx*4], 0
        shr     eax, 12
        add     edx, eax
        shl     eax, 12
        add     ebx, eax
        jmp     @b
.merge_done:
        pop     esi
        or      ebx, FREE_BLOCK
        mov     [page_tabs+ecx*4], ebx
.ret:
        pop     eax edx ecx
        ret
.realloc_add:
; get some additional memory
        mov     eax, [current_slot]
        mov     eax, [APPDATA.heap_top+eax]
        shr     eax, 12
        cmp     edx, eax
        jae     .cant_inplace
        mov     eax, [page_tabs+edx*4]
        test    al, FREE_BLOCK
        jz      .cant_inplace
        shr     eax, 12
        add     eax, edx
        sub     eax, ebx
        jb      .cant_inplace
        jz      @f
        shl     eax, 12
        or      al, FREE_BLOCK
        mov     [page_tabs+ebx*4], eax
@@:
        mov     eax, ebx
        sub     eax, ecx
        shl     eax, 12
        or      al, USED_BLOCK
        mov     [page_tabs+ecx*4], eax
        lea     eax, [ecx+1]
        shl     eax, 12
        push    eax
        push    edi
        lea     edi, [page_tabs+edx*4]
        mov     eax, 2
        sub     ebx, edx
        mov     ecx, ebx
        cld
        rep     stosd
        pop     edi
        mov     edx, [current_slot]
        shl     ebx, 12
        add     ebx, [APPDATA.mem_size+edx]
        call    update_mem_size
        pop     eax edx ecx
        ret
.cant_inplace:
        push    esi edi
        mov     eax, [current_slot]
        mov     esi, [APPDATA.heap_base+eax]
        mov     edi, [APPDATA.heap_top+eax]
        shr     esi, 12
        shr     edi, 12
        sub     ebx, ecx
.find_place:
        cmp     esi, edi
        jae     .place_not_found
        mov     eax, [page_tabs+esi*4]
        test    al, FREE_BLOCK
        jz      .next_place
        shr     eax, 12
        cmp     eax, ebx
        jae     .place_found
        add     esi, eax
        jmp     .find_place
.next_place:
        shr     eax, 12
        add     esi, eax
        jmp     .find_place
.place_not_found:
        pop     edi esi
        jmp     .ret0
.place_found:
        sub     eax, ebx
        jz      @f
        push    esi
        add     esi, ebx
        shl     eax, 12
        or      al, FREE_BLOCK
        mov     [page_tabs+esi*4], eax
        pop     esi
@@:
        mov     eax, ebx
        shl     eax, 12
        or      al, USED_BLOCK
        mov     [page_tabs+esi*4], eax
        inc     esi
        mov     eax, esi
        shl     eax, 12
        push    eax
        mov     eax, [page_tabs+ecx*4]
        and     eax, not 0xFFF
        or      al, FREE_BLOCK
        sub     edx, ecx
        mov     [page_tabs+ecx*4], eax
        inc     ecx
        dec     ebx
        dec     edx
        jz      .no
@@:
        xor     eax, eax
        xchg    eax, [page_tabs+ecx*4]
        mov     [page_tabs+esi*4], eax
	mov     eax, ecx
	shl     eax, 12
	invlpg  [eax]
        inc     esi
        inc     ecx
        dec     ebx
        dec     edx
        jnz     @b
.no:
        push    ebx
        mov     edx, [current_slot]
        shl     ebx, 12
        add     ebx, [APPDATA.mem_size+edx]
        call    update_mem_size
        pop     ebx
@@:
        mov     dword [page_tabs+esi*4], 2
        inc     esi
        dec     ebx
        jnz     @b
        pop     eax edi esi edx ecx
        ret

if 0
align 4
proc alloc_dll
           pushf
           cli
           bsf eax, [dll_map]
           jnz .find
           popf
           xor eax, eax
           ret
.find:
           btr [dll_map], eax
           popf
           shl eax, 5
           add eax, dll_tab
           ret
endp

align 4
proc alloc_service
           pushf
           cli
           bsf eax, [srv_map]
           jnz .find
           popf
           xor eax, eax
           ret
.find:
           btr [srv_map], eax
           popf
           shl eax,0x02
           lea eax,[srv_tab+eax+eax*8]   ;srv_tab+eax*36
           ret
endp

end if
