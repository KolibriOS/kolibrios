;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;**************************************************************************
;
;   [cache_ide[X]_pointer]
;   or [cache_ide[X]_data_pointer]  first entry in cache list
;
;            +0   - lba sector
;            +4   - state of cache sector
;                   0 = empty
;                   1 = used for read  ( same as in hd )
;                   2 = used for write ( differs from hd )
;
;  [cache_ide[X]_system_data]
;  or [cache_ide[x]_appl_data] - cache entries
;
;**************************************************************************

$Revision: 3742 $

align 4
find_empty_slot_CD_cache:
;-----------------------------------------------------------
; find empty or read slot, flush cache if next 10% is used by write
; output : edi = cache slot
;-----------------------------------------------------------
.search_again:
        call    cd_calculate_cache_3
.search_for_empty:
        inc     edi
        call    cd_calculate_cache_4
        jbe     .inside_cache
        mov     edi, 1
.inside_cache:
        call    cd_calculate_cache_5
        ret
;--------------------------------------------------------------------
clear_CD_cache:
        pusha
.ide0:
        xor     eax, eax
        cmp     [cdpos], 1
        jne     .ide1
        mov     [cache_ide0_search_start], eax
        mov     ecx, [cache_ide0_system_sad_size]
        mov     edi, [cache_ide0_pointer]
        call    .clear
        mov     [cache_ide0_appl_search_start], eax
        mov     ecx, [cache_ide0_appl_sad_size]
        mov     edi, [cache_ide0_data_pointer]
        jmp     .continue
.ide1:
        cmp     [cdpos], 2
        jne     .ide2
        mov     [cache_ide1_search_start], eax
        mov     ecx, [cache_ide1_system_sad_size]
        mov     edi, [cache_ide1_pointer]
        call    .clear
        mov     [cache_ide1_appl_search_start], eax
        mov     ecx, [cache_ide1_appl_sad_size]
        mov     edi, [cache_ide1_data_pointer]
        jmp     .continue
.ide2:
        cmp     [cdpos], 3
        jne     .ide3
        mov     [cache_ide2_search_start], eax
        mov     ecx, [cache_ide2_system_sad_size]
        mov     edi, [cache_ide2_pointer]
        call    .clear
        mov     [cache_ide2_appl_search_start], eax
        mov     ecx, [cache_ide2_appl_sad_size]
        mov     edi, [cache_ide2_data_pointer]
        jmp     .continue
.ide3:
        mov     [cache_ide3_search_start], eax
        mov     ecx, [cache_ide3_system_sad_size]
        mov     edi, [cache_ide3_pointer]
        call    .clear
        mov     [cache_ide3_appl_search_start], eax
        mov     ecx, [cache_ide3_appl_sad_size]
        mov     edi, [cache_ide3_data_pointer]
.continue:
        call    .clear
        popa
        ret
.clear:
        shl     ecx, 1
        cld
        rep stosd
        ret
;--------------------------------------------------------------------
align 4
cd_calculate_cache:
; 1 - IDE0 ... 4 - IDE3
.ide0:
        cmp     [cdpos], 1
        jne     .ide1
        cmp     [cd_appl_data], 0
        jne     .ide0_appl_data
        mov     ecx, [cache_ide0_system_sad_size]
        mov     esi, [cache_ide0_pointer]
        ret
.ide0_appl_data:
        mov     ecx, [cache_ide0_appl_sad_size]
        mov     esi, [cache_ide0_data_pointer]
        ret
.ide1:
        cmp     [cdpos], 2
        jne     .ide2
        cmp     [cd_appl_data], 0
        jne     .ide1_appl_data
        mov     ecx, [cache_ide1_system_sad_size]
        mov     esi, [cache_ide1_pointer]
        ret
.ide1_appl_data:
        mov     ecx, [cache_ide1_appl_sad_size]
        mov     esi, [cache_ide1_data_pointer]
        ret
.ide2:
        cmp     [cdpos], 3
        jne     .ide3
        cmp     [cd_appl_data], 0
        jne     .ide2_appl_data
        mov     ecx, [cache_ide2_system_sad_size]
        mov     esi, [cache_ide2_pointer]
        ret
.ide2_appl_data:
        mov     ecx, [cache_ide2_appl_sad_size]
        mov     esi, [cache_ide2_data_pointer]
        ret
.ide3:
        cmp     [cd_appl_data], 0
        jne     .ide3_appl_data
        mov     ecx, [cache_ide3_system_sad_size]
        mov     esi, [cache_ide3_pointer]
        ret
.ide3_appl_data:
        mov     ecx, [cache_ide3_appl_sad_size]
        mov     esi, [cache_ide3_data_pointer]
        ret
;--------------------------------------------------------------------
align 4
cd_calculate_cache_1:
; 1 - IDE0 ... 4 - IDE3
.ide0:
        cmp     [cdpos], 1
        jne     .ide1
        cmp     [cd_appl_data], 0
        jne     .ide0_appl_data
        mov     esi, [cache_ide0_pointer]
        ret
.ide0_appl_data:
        mov     esi, [cache_ide0_data_pointer]
        ret
.ide1:
        cmp     [cdpos], 2
        jne     .ide2
        cmp     [cd_appl_data], 0
        jne     .ide1_appl_data
        mov     esi, [cache_ide1_pointer]
        ret
.ide1_appl_data:
        mov     esi, [cache_ide1_data_pointer]
        ret
.ide2:
        cmp     [cdpos], 3
        jne     .ide3
        cmp     [cd_appl_data], 0
        jne     .ide2_appl_data
        mov     esi, [cache_ide2_pointer]
        ret
.ide2_appl_data:
        mov     esi, [cache_ide2_data_pointer]
        ret
.ide3:
        cmp     [cd_appl_data], 0
        jne     .ide3_appl_data
        mov     esi, [cache_ide3_pointer]
        ret
.ide3_appl_data:
        mov     esi, [cache_ide3_data_pointer]
        ret
;--------------------------------------------------------------------
align 4
cd_calculate_cache_2:
; 1 - IDE0 ... 4 - IDE3
.ide0:
        cmp     [cdpos], 1
        jne     .ide1
        cmp     [cd_appl_data], 0
        jne     .ide0_appl_data
        mov     eax, [cache_ide0_system_data]
        ret
.ide0_appl_data:
        mov     eax, [cache_ide0_appl_data]
        ret
.ide1:
        cmp     [cdpos], 2
        jne     .ide2
        cmp     [cd_appl_data], 0
        jne     .ide1_appl_data
        mov     eax, [cache_ide1_system_data]
        ret
.ide1_appl_data:
        mov     eax, [cache_ide1_appl_data]
        ret
.ide2:
        cmp     [cdpos], 3
        jne     .ide3
        cmp     [cd_appl_data], 0
        jne     .ide2_appl_data
        mov     eax, [cache_ide2_system_data]
        ret
.ide2_appl_data:
        mov     eax, [cache_ide2_appl_data]
        ret
.ide3:
        cmp     [cd_appl_data], 0
        jne     .ide3_appl_data
        mov     eax, [cache_ide3_system_data]
        ret
.ide3_appl_data:
        mov     eax, [cache_ide3_appl_data]
        ret
;--------------------------------------------------------------------
align 4
cd_calculate_cache_3:
;    mov   ecx,cache_max*10/100
;    mov   edi,[cache_search_start]

; 1 - IDE0 ... 4 - IDE3
.ide0:
        cmp     [cdpos], 1
        jne     .ide1
        cmp     [cd_appl_data], 0
        jne     .ide0_appl_data
        mov     edi, [cache_ide0_search_start]
        ret
.ide0_appl_data:
        mov     edi, [cache_ide0_appl_search_start]
        ret
.ide1:
        cmp     [cdpos], 2
        jne     .ide2
        cmp     [cd_appl_data], 0
        jne     .ide1_appl_data
        mov     edi, [cache_ide1_search_start]
        ret
.ide1_appl_data:
        mov     edi, [cache_ide1_appl_search_start]
        ret
.ide2:
        cmp     [cdpos], 3
        jne     .ide3
        cmp     [cd_appl_data], 0
        jne     .ide2_appl_data
        mov     edi, [cache_ide2_search_start]
        ret
.ide2_appl_data:
        mov     edi, [cache_ide2_appl_search_start]
        ret
.ide3:
        cmp     [cd_appl_data], 0
        jne     .ide3_appl_data
        mov     edi, [cache_ide3_search_start]
        ret
.ide3_appl_data:
        mov     edi, [cache_ide3_appl_search_start]
        ret
;--------------------------------------------------------------------
align 4
cd_calculate_cache_4:
;    cmp   edi,cache_max
; 1 - IDE0 ... 4 - IDE3
.ide0:
        cmp     [cdpos], 1
        jne     .ide1
        cmp     [cd_appl_data], 0
        jne     .ide0_appl_data
        cmp     edi, [cache_ide0_system_sad_size]
        ret
.ide0_appl_data:
        cmp     edi, [cache_ide0_appl_sad_size]
        ret
.ide1:
        cmp     [cdpos], 2
        jne     .ide2
        cmp     [cd_appl_data], 0
        jne     .ide1_appl_data
        cmp     edi, [cache_ide1_system_sad_size]
        ret
.ide1_appl_data:
        cmp     edi, [cache_ide1_appl_sad_size]
        ret
.ide2:
        cmp     [cdpos], 3
        jne     .ide3
        cmp     [cd_appl_data], 0
        jne     .ide2_appl_data
        cmp     edi, [cache_ide2_system_sad_size]
        ret
.ide2_appl_data:
        cmp     edi, [cache_ide2_appl_sad_size]
        ret
.ide3:
        cmp     [cd_appl_data], 0
        jne     .ide3_appl_data
        cmp     edi, [cache_ide3_system_sad_size]
        ret
.ide3_appl_data:
        cmp     edi, [cache_ide3_appl_sad_size]
        ret
;--------------------------------------------------------------------
align 4
cd_calculate_cache_5:
;    mov   [cache_search_start],edi
; 1 - IDE0 ... 4 - IDE3
.ide0:
        cmp     [cdpos], 1
        jne     .ide1
        cmp     [cd_appl_data], 0
        jne     .ide0_appl_data
        mov     [cache_ide0_search_start], edi
        ret
.ide0_appl_data:
        mov     [cache_ide0_appl_search_start], edi
        ret
.ide1:
        cmp     [cdpos], 2
        jne     .ide2
        cmp     [cd_appl_data], 0
        jne     .ide1_appl_data
        mov     [cache_ide1_search_start], edi
        ret
.ide1_appl_data:
        mov     [cache_ide1_appl_search_start], edi
        ret
.ide2:
        cmp     [cdpos], 3
        jne     .ide3
        cmp     [cd_appl_data], 0
        jne     .ide2_appl_data
        mov     [cache_ide2_search_start], edi
        ret
.ide2_appl_data:
        mov     [cache_ide2_appl_search_start], edi
        ret
.ide3:
        cmp     [cd_appl_data], 0
        jne     .ide3_appl_data
        mov     [cache_ide3_search_start], edi
        ret
.ide3_appl_data:
        mov     [cache_ide3_appl_search_start], edi
        ret
;--------------------------------------------------------------------
;align 4
;calculate_linear_to_real:
;    shr eax, 12
;    mov eax, [page_tabs+eax*4]
;    and eax, 0xFFFFF000
;    ret
