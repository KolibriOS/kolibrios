;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 3742 $

;-----------------------------------------------------------------------------
        pusha

        mov     eax, [pg_data.pages_free]
;   1/32
        shr     eax, 5
;   round off up to 8 pages
        shr     eax, 3
        shl     eax, 3
;   translate pages in butes *4096
        shl     eax, 12
;       check a upper size of the cache, no more than 1 Mb on the physical device
        cmp     eax, 1024*1024
        jbe     @f

        mov     eax, 1024*1024
        jmp     .continue
;--------------------------------------
@@:
;       check a lower size of the cache, not less than 128 Kb on the physical device
        cmp     eax, 128*1024
        jae     .continue

        mov     eax, 128*1024
;--------------------------------------
.continue:
        push    ecx
        mov     ecx, 12
        mov     esi, cache_ide0+IDE_CACHE.size
        cld
@@:
        mov     [esi], eax
        add     esi, sizeof.IDE_CACHE
        loop    @b

        pop     ecx

        xor     eax, eax
        mov     [hdd_appl_data], 1 ;al
        mov     [cd_appl_data], 1
;--------------------------------------
        test    byte [DRIVE_DATA+1], 0x80
        je      @f

        mov     esi, cache_ide0
        call    get_cache_ide
;--------------------------------------
@@:
        test    byte [DRIVE_DATA+1], 0x20
        je      @f

        mov     esi, cache_ide1
        call    get_cache_ide
;--------------------------------------
@@:
        test    byte [DRIVE_DATA+1], 8
        je      @f

        mov     esi, cache_ide2
        call    get_cache_ide
;--------------------------------------
@@:
        test    byte [DRIVE_DATA+1], 2
        je      @f

        mov     esi, cache_ide3
        call    get_cache_ide
;--------------------------------------
@@:
        test    byte [DRIVE_DATA+6], 0x80
        je      @f

        mov     esi, cache_ide4
        call    get_cache_ide
;--------------------------------------
@@:
        test    byte [DRIVE_DATA+6], 0x20
        je      @f

        mov     esi, cache_ide5
        call    get_cache_ide
;--------------------------------------
@@:
        test    byte [DRIVE_DATA+6], 8
        je      @f

        mov     esi, cache_ide6
        call    get_cache_ide
;--------------------------------------
@@:
        test    byte [DRIVE_DATA+6], 2
        je      @f

        mov     esi, cache_ide7
        call    get_cache_ide
;--------------------------------------
@@:
        test    byte [DRIVE_DATA+11], 0x80
        je      @f

        mov     esi, cache_ide8
        call    get_cache_ide
;--------------------------------------
@@:
        test    byte [DRIVE_DATA+11], 0x20
        je      @f

        mov     esi, cache_ide9
        call    get_cache_ide
;--------------------------------------
@@:
        test    byte [DRIVE_DATA+11], 8
        je      @f

        mov     esi, cache_ide10
        call    get_cache_ide
;--------------------------------------
@@:
        test    byte [DRIVE_DATA+11], 2
        je      end_get_cache

        mov     esi, cache_ide11
        call    get_cache_ide

        jmp     end_get_cache
;-----------------------------------------------------------------------------
get_cache_ide:
        and     [esi+IDE_CACHE.search_start], 0
        and     [esi+IDE_CACHE.appl_search_start], 0

        push    ecx
;        DEBUGF  1, "K : IDE_CACHE.size %x\n", [esi+IDE_CACHE.size]
        stdcall kernel_alloc, [esi+IDE_CACHE.size]
        mov     [esi+IDE_CACHE.pointer], eax
        pop     ecx

        mov     edx, eax
        mov     eax, [esi+IDE_CACHE.size]
        shr     eax, 3
;        DEBUGF  1, "K : IDE_CACHE.system_data_size %x\n", eax
        mov     [esi+IDE_CACHE.system_data_size], eax
        mov     ebx, eax
        imul    eax, 7
;        DEBUGF  1, "K : IDE_CACHE.appl_data_size %x\n", eax
        mov     [esi+IDE_CACHE.appl_data_size], eax
        add     ebx, edx
        mov     [esi+IDE_CACHE.data_pointer], ebx

.cd:
        push    ecx
        mov     eax, [esi+IDE_CACHE.system_data_size]
        call    calculate_for_cd
        add     eax, [esi+IDE_CACHE.pointer]
        mov     [esi+IDE_CACHE.system_data], eax
        mov     [esi+IDE_CACHE.system_sad_size], ecx

        push    edi
        mov     edi, [esi+IDE_CACHE.pointer]
        call    clear_ide_cache
        pop     edi

        mov     eax, [esi+IDE_CACHE.appl_data_size]
        call    calculate_for_cd
        add     eax, [esi+IDE_CACHE.data_pointer]
        mov     [esi+IDE_CACHE.appl_data], eax
        mov     [esi+IDE_CACHE.appl_sad_size], ecx

        push    edi
        mov     edi, [esi+IDE_CACHE.data_pointer]
        call    clear_ide_cache
        pop     edi

        pop     ecx
        ret
;-----------------------------------------------------------------------------
calculate_for_cd:
        push    eax
        mov     ebx, eax
        shr     eax, 11
        shl     eax, 3
        sub     ebx, eax
        shr     ebx, 11
        mov     ecx, ebx
        shl     ebx, 11
        pop     eax
        sub     eax, ebx
        dec     ecx
        ret
;-----------------------------------------------------------------------------
clear_ide_cache:
        push    eax
        shl     ecx, 1
        xor     eax, eax
        cld
        rep stosd
        pop     eax
        ret
;-----------------------------------------------------------------------------
end_get_cache:
        popa
