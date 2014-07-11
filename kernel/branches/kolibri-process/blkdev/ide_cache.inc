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

$Revision: 4700 $

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
        DEBUGF  1, 'K : clear_CD_cache\n'
        pusha

        mov     esi, [cdpos]
        dec     esi
        imul    esi, sizeof.IDE_CACHE
        add     esi, cache_ide0

        xor     eax, eax

        mov     [esi+IDE_CACHE.search_start], eax
        mov     ecx, [esi+IDE_CACHE.system_sad_size]
        mov     edi, [esi+IDE_CACHE.pointer]
        call    .clear

        mov     [esi+IDE_CACHE.appl_search_start], eax
        mov     ecx, [esi+IDE_CACHE.appl_sad_size]
        mov     edi, [esi+IDE_CACHE.data_pointer]
        call    .clear

        popa
        ret
;--------------------------------------
.clear:
        shl     ecx, 1
        cld
        rep stosd
        ret
;--------------------------------------------------------------------
align 4
cd_calculate_cache:
; 1 - IDE0 ... 12 - IDE11
        push    eax

        mov     eax, [cdpos]
        dec     eax
        imul    eax, sizeof.IDE_CACHE
        add     eax, cache_ide0

        cmp     [cd_appl_data], 0
        jne     @f

        mov     ecx, [eax+IDE_CACHE.system_sad_size]
        mov     esi, [eax+IDE_CACHE.pointer]
        pop     eax
        ret
;--------------------------------------
@@:
        mov     ecx, [eax+IDE_CACHE.appl_sad_size]
        mov     esi, [eax+IDE_CACHE.data_pointer]
        pop     eax
        ret
;--------------------------------------------------------------------
align 4
cd_calculate_cache_1:
; 1 - IDE0 ... 12 - IDE11
        push    eax

        mov     eax, [cdpos]
        dec     eax
        imul    eax, sizeof.IDE_CACHE
        add     eax, cache_ide0

        cmp     [cd_appl_data], 0
        jne     @f

        mov     esi, [eax+IDE_CACHE.pointer]
        pop     eax
        ret
;--------------------------------------
@@:
        mov     esi, [eax+IDE_CACHE.data_pointer]
        pop     eax
        ret
;--------------------------------------------------------------------
align 4
cd_calculate_cache_2:
; 1 - IDE0 ... 12 - IDE11
        mov     eax, [cdpos]
        dec     eax
        imul    eax, sizeof.IDE_CACHE
        add     eax, cache_ide0

        cmp     [cd_appl_data], 0
        jne     @f

        mov     eax, [eax+IDE_CACHE.system_data]
        ret
;--------------------------------------
@@:
        mov     eax, [eax+IDE_CACHE.appl_data]
        ret
;--------------------------------------------------------------------
align 4
cd_calculate_cache_3:
; 1 - IDE0 ... 12 - IDE11
        push    eax

        mov     eax, [cdpos]
        dec     eax
        imul    eax, sizeof.IDE_CACHE
        add     eax, cache_ide0

        cmp     [cd_appl_data], 0
        jne     @f

        mov     edi, [eax+IDE_CACHE.search_start]
        pop     eax
        ret
;--------------------------------------
@@:
        mov     edi, [eax+IDE_CACHE.appl_search_start]
        pop     eax
        ret
;--------------------------------------------------------------------
align 4
cd_calculate_cache_4:
; 1 - IDE0 ... 12 - IDE11
        push    eax

        mov     eax, [cdpos]
        dec     eax
        imul    eax, sizeof.IDE_CACHE
        add     eax, cache_ide0

        cmp     [cd_appl_data], 0
        jne     @f

        cmp     edi, [eax+IDE_CACHE.system_sad_size]
        pop     eax
        ret
;--------------------------------------
@@:
        cmp     edi, [eax+IDE_CACHE.appl_sad_size]
        pop     eax
        ret
;--------------------------------------------------------------------
align 4
cd_calculate_cache_5:
; 1 - IDE0 ... 12 - IDE11
        push    eax

        mov     eax, [cdpos]
        dec     eax
        imul    eax, sizeof.IDE_CACHE
        add     eax, cache_ide0

        cmp     [cd_appl_data], 0
        jne     @f

        mov     [eax+IDE_CACHE.search_start], edi
        pop     eax
        ret
;--------------------------------------
@@:
        mov     [eax+IDE_CACHE.appl_search_start], edi
        pop     eax
        ret
;--------------------------------------------------------------------
