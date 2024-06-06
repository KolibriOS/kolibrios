;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; Memory management for USB structures.
; Protocol layer uses the common kernel heap malloc/free.
; Hardware layer has special requirements:
; * memory blocks should be properly aligned
; * memory blocks should not cross page boundary
; Hardware layer allocates fixed-size blocks.
; Thus, hardware layer uses the system slab allocator.

; Helper procedure: translate physical address in ecx
; of some transfer descriptor to linear address.
; in: eax = address of first page
proc usb_td_to_virt
; Traverse all pages used for transfer descriptors, looking for the one
; with physical address as in ecx.
@@:
        test    eax, eax
        jz      .zero
        push    eax
        call    get_pg_addr
        sub     eax, ecx
        jz      .found
        cmp     eax, -0x1000
        ja      .found
        pop     eax
        mov     eax, [eax+0x1000-4]
        jmp     @b
.found:
; When found, combine page address from eax with page offset from ecx.
        pop     eax
        and     ecx, 0xFFF
        add     eax, ecx
.zero:
        ret
endp
