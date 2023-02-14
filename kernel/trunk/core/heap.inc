;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2022. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


struct  MEM_BLOCK
        list            LHEAD
        next_block      dd ? ;+8
        prev_block      dd ? ;+4
        base            dd ? ;+16
        size            dd ? ;+20
        flags           dd ? ;+24
        handle          dd ? ;+28
ends

MEM_BLOCK_RESERVED  = 0x02      ; Will be allocated on first access (lazy allocation)
MEM_BLOCK_FREE      = 0x04
MEM_BLOCK_USED      = 0x08
MEM_BLOCK_DONT_FREE = 0x10

macro calc_index op
{          shr op, 12
        dec     op
        cmp     op, 63
        jna     @f
        mov     op, 63
@@:
}

align 4
md:
.add_to_used:
        mov     eax, [esi + MEM_BLOCK.base]
        mov     ebx, [esi + MEM_BLOCK.base]
        shr     ebx, 6
        add     eax, ebx
        shr     ebx, 6
        add     eax, ebx
        shr     eax, 12
        and     eax, 63
        inc     [mem_hash_cnt + eax*4]

        lea     ecx, [mem_used_list + eax*8]
        list_add esi, ecx
        mov     [esi + MEM_BLOCK.flags], MEM_BLOCK_USED
        mov     eax, [esi + MEM_BLOCK.size]
        sub     [heap_free], eax
        ret
align 4
.find_used:
        mov     ecx, eax
        mov     ebx, eax
        shr     ebx, 6
        add     ecx, ebx
        shr     ebx, 6
        add     ecx, ebx
        shr     ecx, 12
        and     ecx, 63

        lea     ebx, [mem_used_list + ecx*8]
        mov     esi, ebx
.next:
        mov     esi, [esi + MEM_BLOCK.list.next]
        cmp     esi, ebx
        je      .fail

        cmp     eax, [esi + MEM_BLOCK.base]
        jne     .next

        ret
.fail:
        xor     esi, esi
        ret

align 4
.del_from_used:
        call    .find_used
        test    esi, esi
        jz      .done

        cmp     [esi + MEM_BLOCK.flags], MEM_BLOCK_USED
        jne     .fatal

        dec     [mem_hash_cnt + ecx*4]
        list_del esi
.done:
        ret
.fatal:                            ;FIXME panic here
        xor     esi, esi
        ret

;Initial heap state
;
; + heap_size               terminator        MEM_BLOCK_USED
; + 4096*MEM_BLOCK.sizeof   free space        MEM_BLOCK_FREE
;HEAP_BASE                heap_descriptors  MEM_BLOCK_USED
;

align 4
proc init_kernel_heap

        mov     ecx, 64
        mov     edi, mem_block_list
  @@:
        mov     eax, edi
        stosd
        stosd
        loop    @B

        mov     ecx, 64
        mov     edi, mem_used_list
  @@:
        mov     eax, edi
        stosd
        stosd
        loop    @B

        stdcall alloc_pages, dword 32

        or      eax, PG_SWR
        mov     ebx, HEAP_BASE
        mov     ecx, 32
        call    commit_pages

        mov     edi, HEAP_BASE                     ;descriptors
        mov     ebx, HEAP_BASE + sizeof.MEM_BLOCK      ;free space
        mov     ecx, HEAP_BASE + sizeof.MEM_BLOCK*2    ;terminator

        xor     eax, eax
        mov     [edi + MEM_BLOCK.next_block], ebx
        mov     [edi + MEM_BLOCK.prev_block], eax
        mov     [edi + MEM_BLOCK.list.next], eax
        mov     [edi + MEM_BLOCK.list.prev], eax
        mov     [edi + MEM_BLOCK.base], HEAP_BASE
        mov     [edi + MEM_BLOCK.size], PAGE_SIZE * sizeof.MEM_BLOCK
        mov     [edi + MEM_BLOCK.flags], MEM_BLOCK_USED

        mov     [ecx + MEM_BLOCK.next_block], eax
        mov     [ecx + MEM_BLOCK.prev_block], ebx
        mov     [ecx + MEM_BLOCK.list.next], eax
        mov     [ecx + MEM_BLOCK.list.prev], eax
        mov     [ecx + MEM_BLOCK.base], eax
        mov     [ecx + MEM_BLOCK.size], eax
        mov     [ecx + MEM_BLOCK.flags], MEM_BLOCK_USED

        mov     [ebx + MEM_BLOCK.next_block], ecx
        mov     [ebx + MEM_BLOCK.prev_block], edi
        mov     [ebx + MEM_BLOCK.base], HEAP_BASE + PAGE_SIZE * sizeof.MEM_BLOCK

        mov     ecx, [pg_data.kernel_pages]
        shl     ecx, 12
        sub     ecx, HEAP_BASE-OS_BASE + PAGE_SIZE * sizeof.MEM_BLOCK
        mov     [heap_size], ecx
        mov     [heap_free], ecx
        mov     [ebx + MEM_BLOCK.size], ecx
        mov     [ebx + MEM_BLOCK.flags], MEM_BLOCK_FREE

        mov     [mem_block_mask], eax
        mov     [mem_block_mask + 4], 0x80000000

        mov     ecx, mem_block_list + 63*8
        list_add ebx, ecx

        mov     ecx, PAGE_SIZE -3-1
        mov     eax, HEAP_BASE + sizeof.MEM_BLOCK*4

        mov     [next_memblock], HEAP_BASE + sizeof.MEM_BLOCK *3
 @@:
        mov     [eax - sizeof.MEM_BLOCK], eax
        add     eax, sizeof.MEM_BLOCK
        loop    @B

        mov     dword[eax - sizeof.MEM_BLOCK], 0

        mov     ecx, heap_mutex
        call    mutex_init
        mov     [heap_blocks], 4094
        mov     [free_blocks], 4093
        ret
endp

; param
;  eax= required size
;
; retval
;  edi= memory block descriptor
;  ebx= descriptor index

align 4
get_small_block:
        mov     ecx, eax
        shr     ecx, 12
        dec     ecx
        cmp     ecx, 63
        jle     .get_index
        mov     ecx, 63
  .get_index:
        lea     esi, [mem_block_mask]
        xor     ebx, ebx
        or      edx, -1

        cmp     ecx, 32
        jb      .bit_test

        sub     ecx, 32
        add     ebx, 32
        add     esi, 4
  .bit_test:
        shl     edx, cl
        and     edx, [esi]
  .find:
        bsf     edi, edx
        jz      .high_mask
        add     ebx, edi
        lea     ecx, [mem_block_list + ebx*8]
        mov     edi, ecx
  .next:
        mov     edi, [edi + MEM_BLOCK.list.next]
        cmp     edi, ecx
        je      .err
        cmp     eax, [edi + MEM_BLOCK.size]
        ja      .next
        ret
  .err:
        xor     edi, edi
        ret

  .high_mask:
        add     esi, 4
        cmp     esi, mem_block_mask + 8
        jae     .err
        add     ebx, 32
        mov     edx, [esi]
        jmp     .find


align 4
free_mem_block:

        mov     ebx, [next_memblock]
        mov     [eax], ebx
        mov     [next_memblock], eax

        xor     ebx, ebx
        mov     dword[eax + 4], ebx
        mov     dword[eax + 8], ebx
        mov     dword[eax + 12], ebx
        mov     dword[eax + 16], ebx
;           mov dword[eax + 20], 0     ;don't clear block size
        mov     dword[eax + 24], ebx
        mov     dword[eax + 28], ebx

        inc     [free_blocks]

        ret

align 4
proc alloc_kernel_space stdcall, size:dword
           local block_ind:DWORD

        push    ebx
        push    esi
        push    edi

        mov     eax, [size]
        add     eax, PAGE_SIZE-1
        and     eax, -PAGE_SIZE
        mov     [size], eax

        cmp     eax, [heap_free]
        ja      .error

        spin_lock_irqsave heap_mutex

        mov     eax, [size]

        call    get_small_block ; eax
        test    edi, edi
        jz      .error_unlock

        cmp     [edi + MEM_BLOCK.flags], MEM_BLOCK_FREE
        jne     .error_unlock

        mov     [block_ind], ebx  ;index of allocated block

        mov     eax, [edi + MEM_BLOCK.size]
        cmp     eax, [size]
        je      .m_eq_size

        mov     esi, [next_memblock]    ;new memory block
        test    esi, esi
        jz      .error_unlock

        dec     [free_blocks]
        mov     eax, [esi]
        mov     [next_memblock], eax

        mov     [esi + MEM_BLOCK.next_block], edi
        mov     eax, [edi + MEM_BLOCK.prev_block]
        mov     [esi + MEM_BLOCK.prev_block], eax
        mov     [edi + MEM_BLOCK.prev_block], esi
        mov     [esi + MEM_BLOCK.list.next], 0
        mov     [esi + MEM_BLOCK.list.prev], 0
        mov     [eax + MEM_BLOCK.next_block], esi

        mov     ebx, [edi + MEM_BLOCK.base]
        mov     [esi + MEM_BLOCK.base], ebx
        mov     edx, [size]
        mov     [esi + MEM_BLOCK.size], edx
        add     [edi + MEM_BLOCK.base], edx
        sub     [edi + MEM_BLOCK.size], edx

        mov     eax, [edi + MEM_BLOCK.size]
        calc_index eax
        cmp     eax, [block_ind]
        je      .add_used

        list_del edi

        mov     ecx, [block_ind]
        lea     edx, [mem_block_list + ecx*8]
        cmp     edx, [edx]
        jnz     @f
        btr     [mem_block_mask], ecx
@@:
        bts     [mem_block_mask], eax
        lea     edx, [mem_block_list + eax*8]  ;edx= list head
        list_add edi, edx
.add_used:

        call    md.add_to_used

        spin_unlock_irqrestore heap_mutex
        mov     eax, [esi + MEM_BLOCK.base]
        pop     edi
        pop     esi
        pop     ebx
        ret

.m_eq_size:
        list_del edi
        lea     edx, [mem_block_list + ebx*8]
        cmp     edx, [edx]
        jnz     @f
        btr     [mem_block_mask], ebx
@@:
        mov     esi, edi
        jmp     .add_used

.error_unlock:
        spin_unlock_irqrestore heap_mutex
.error:
        xor     eax, eax
        pop     edi
        pop     esi
        pop     ebx
        ret
endp

align 4
proc free_kernel_space stdcall uses ebx ecx edx esi edi, base:dword

        spin_lock_irqsave heap_mutex

        mov     eax, [base]

        call    md.del_from_used
        test    esi, esi
        jz      .fail

        mov     eax, [esi + MEM_BLOCK.size]
        add     [heap_free], eax

        mov     edi, [esi + MEM_BLOCK.next_block]
        cmp     [edi + MEM_BLOCK.flags], MEM_BLOCK_FREE
        jne     .prev

        list_del edi

        mov     edx, [edi + MEM_BLOCK.next_block]
        mov     [esi + MEM_BLOCK.next_block], edx
        mov     [edx + MEM_BLOCK.prev_block], esi
        mov     ecx, [edi + MEM_BLOCK.size]
        add     [esi + MEM_BLOCK.size], ecx

        calc_index ecx

        lea     edx, [mem_block_list + ecx*8]
        cmp     edx, [edx]
        jne     @F
        btr     [mem_block_mask], ecx
@@:
        mov     eax, edi
        call    free_mem_block
.prev:
        mov     edi, [esi + MEM_BLOCK.prev_block]
        cmp     [edi + MEM_BLOCK.flags], MEM_BLOCK_FREE
        jne     .insert

        mov     edx, [esi + MEM_BLOCK.next_block]
        mov     [edi + MEM_BLOCK.next_block], edx
        mov     [edx + MEM_BLOCK.prev_block], edi

        mov     eax, esi
        call    free_mem_block

        mov     ecx, [edi + MEM_BLOCK.size]
        mov     eax, [esi + MEM_BLOCK.size]
        add     eax, ecx
        mov     [edi + MEM_BLOCK.size], eax

        calc_index eax                     ;new index
        calc_index ecx                     ;old index
        cmp     eax, ecx
        je      .m_eq

        push    ecx
        list_del edi
        pop     ecx

        lea     edx, [mem_block_list + ecx*8]
        cmp     edx, [edx]
        jne     .add_block
        btr     [mem_block_mask], ecx

.add_block:
        bts     [mem_block_mask], eax
        lea     edx, [mem_block_list + eax*8]
        list_add edi, edx
.m_eq:
        spin_unlock_irqrestore heap_mutex
        xor     eax, eax
        not     eax
        ret
.insert:
        mov     [esi + MEM_BLOCK.flags], MEM_BLOCK_FREE
        mov     eax, [esi + MEM_BLOCK.size]
        calc_index eax
        mov     edi, esi
        jmp     .add_block

.fail:
        spin_unlock_irqrestore heap_mutex
        xor     eax, eax
        ret
endp

align 4
proc kernel_alloc stdcall, size:dword
        locals
          lin_addr    dd ?
          pages_count dd ?
        endl

        push    ebx
        push    edi

        mov     eax, [size]
        add     eax, PAGE_SIZE-1
        and     eax, -PAGE_SIZE;
        mov     [size], eax
        and     eax, eax
        jz      .err
        mov     ebx, eax
        shr     ebx, 12
        mov     [pages_count], ebx

        stdcall alloc_kernel_space, eax
        mov     [lin_addr], eax
        mov     ebx, [pages_count]
        test    eax, eax
        jz      .err

        mov     edx, eax

        shr     ebx, 3
        jz      .tail

        shl     ebx, 3
        stdcall alloc_pages, ebx
        test    eax, eax
        jz      .err

        mov     ecx, ebx
        or      eax, PG_GLOBAL + PG_SWR
        mov     ebx, [lin_addr]
        call    commit_pages

        mov     edx, ebx                    ; this dirty hack
.tail:
        mov     ebx, [pages_count]
        and     ebx, 7
        jz      .end
@@:
        call    alloc_page
        test    eax, eax
        jz      .err

        stdcall map_page, edx, eax, dword (PG_GLOBAL + PG_SWR)
        add     edx, PAGE_SIZE
        dec     ebx
        jnz     @B
.end:
        mov     eax, [lin_addr]
        pop     edi
        pop     ebx
        ret
.err:
        xor     eax, eax
        pop     edi
        pop     ebx
        ret
endp

align 4
proc kernel_free stdcall, base:dword

        push    ebx esi

        spin_lock_irqsave heap_mutex

        mov     eax, [base]
        call    md.find_used

        cmp     [esi + MEM_BLOCK.flags], MEM_BLOCK_USED
        jne     .fail

        spin_unlock_irqrestore heap_mutex

        mov     eax, [esi + MEM_BLOCK.base]
        mov     ecx, [esi + MEM_BLOCK.size]
        shr     ecx, 12
        call    release_pages   ;eax, ecx
        stdcall free_kernel_space, [base]
        pop     esi ebx
        ret
.fail:
        spin_unlock_irqrestore heap_mutex
        xor     eax, eax
        pop     esi ebx
        ret
endp

;;;;;;;;;;;;;;      USER HEAP     ;;;;;;;;;;;;;;;;;

HEAP_TOP  = 0x80000000

align 4
proc init_heap

        mov     ebx, [current_process]
        mov     eax, [ebx + PROC.heap_top]
        test    eax, eax
        jz      @F
        sub     eax, [ebx + PROC.heap_base]
        sub     eax, PAGE_SIZE
        ret
@@:
        lea     ecx, [ebx + PROC.heap_lock]
        call    mutex_init

        mov     esi, [ebx + PROC.mem_used]
        add     esi, PAGE_SIZE-1
        and     esi, -PAGE_SIZE
        mov     [ebx + PROC.mem_used], esi
        mov     eax, HEAP_TOP
        mov     [ebx + PROC.heap_base], esi
        mov     [ebx + PROC.heap_top], eax

        sub     eax, esi
        shr     esi, 10
        mov     ecx, eax
        sub     eax, PAGE_SIZE
        or      ecx, MEM_BLOCK_FREE
        mov     [page_tabs + esi], ecx
        ret
endp

align 4
proc user_alloc stdcall, alloc_size:dword

        push    ebx esi edi

        mov     ebx, [current_process]
        lea     ecx, [ebx + PROC.heap_lock]
        call    mutex_lock

        mov     ecx, [alloc_size]
        add     ecx, (4095 + PAGE_SIZE)
        and     ecx, -PAGE_SIZE
        mov     esi, [ebx + PROC.heap_base]
        mov     edi, [ebx + PROC.heap_top]
  .scan:
        cmp     esi, edi
        jae     .m_exit

        mov     ebx, esi
        shr     ebx, 12
        mov     eax, [page_tabs + ebx*4]
        test    al, MEM_BLOCK_FREE
        jz      .test_used
        and     eax, -PAGE_SIZE
        cmp     eax, ecx   ;alloc_size
        jb      .m_next
        jz      @f

        lea     edx, [esi + ecx]
        sub     eax, ecx
        or      al, MEM_BLOCK_FREE
        shr     edx, 12
        mov     [page_tabs + edx*4], eax
  @@:
        or      ecx, MEM_BLOCK_USED
        mov     [page_tabs + ebx*4], ecx
        shr     ecx, 12
        inc     ebx
        dec     ecx
        jz      .no
  @@:
        mov     dword [page_tabs + ebx*4], MEM_BLOCK_RESERVED
        inc     ebx
        dec     ecx
        jnz     @B
  .no:

        mov     edx, [current_process]
        mov     ebx, [alloc_size]
        add     ebx, PAGE_SIZE-1
        and     ebx, -PAGE_SIZE
        add     [edx + PROC.mem_used], ebx

        lea     ecx, [edx + PROC.heap_lock]
        call    mutex_unlock

        lea     eax, [esi + PAGE_SIZE]

        pop     edi
        pop     esi
        pop     ebx
        ret
.test_used:
        test    al, MEM_BLOCK_USED
        jz      .m_exit

        and     eax, -PAGE_SIZE
.m_next:
        add     esi, eax
        jmp     .scan
.m_exit:
        mov     ecx, [current_process]
        lea     ecx, [ecx + PROC.heap_lock]
        call    mutex_unlock

        xor     eax, eax
        pop     edi
        pop     esi
        pop     ebx
        ret
endp

align 4
proc user_alloc_at stdcall, address:dword, alloc_size:dword

        push    ebx
        push    esi
        push    edi

        mov     ebx, [current_process]
        lea     ecx, [ebx + PROC.heap_lock]
        call    mutex_lock

        mov     edx, [address]
        and     edx, -PAGE_SIZE
        mov     [address], edx
        sub     edx, PAGE_SIZE
        jb      .error
        mov     esi, [ebx + PROC.heap_base]
        mov     edi, [ebx + PROC.heap_top]
        cmp     edx, esi
        jb      .error
.scan:
        cmp     esi, edi
        jae     .error
        mov     ebx, esi
        shr     ebx, 12
        mov     eax, [page_tabs + ebx*4]
        mov     ecx, eax
        and     ecx, -PAGE_SIZE
        add     ecx, esi
        cmp     edx, ecx
        jb      .found
        mov     esi, ecx
        jmp     .scan
.error:
        mov     ecx, [current_process]
        lea     ecx, [ecx + PROC.heap_lock]
        call    mutex_unlock

        xor     eax, eax
        pop     edi
        pop     esi
        pop     ebx
        ret
.found:
        test    al, MEM_BLOCK_FREE
        jz      .error
        mov     eax, ecx
        sub     eax, edx
        sub     eax, PAGE_SIZE
        cmp     eax, [alloc_size]
        jb      .error

; Here we have 1 big free block which includes requested area.
; In general, 3 other blocks must be created instead:
; free at [esi, edx);
; busy at [edx, edx + 0x1000 + ALIGN_UP(alloc_size,0x1000));
; free at [edx + 0x1000 + ALIGN_UP(alloc_size,0x1000), ecx)
; First or third block (or both) may be absent.
        mov     eax, edx
        sub     eax, esi
        jz      .nofirst
        or      al, MEM_BLOCK_FREE
        mov     [page_tabs + ebx*4], eax
  .nofirst:
        mov     eax, [alloc_size]
        add     eax, 0x1FFF
        and     eax, -PAGE_SIZE
        mov     ebx, edx
        add     edx, eax
        shr     ebx, 12
        or      al, MEM_BLOCK_USED
        mov     [page_tabs + ebx*4], eax
        shr     eax, 12
        dec     eax
        jz      .second_nofill
        inc     ebx
  .fill:
        mov     dword [page_tabs + ebx*4], MEM_BLOCK_RESERVED
        inc     ebx
        dec     eax
        jnz     .fill

  .second_nofill:
        sub     ecx, edx
        jz      .nothird
        or      cl, MEM_BLOCK_FREE
        mov     [page_tabs + ebx*4], ecx

  .nothird:
        mov     edx, [current_process]
        mov     ebx, [alloc_size]
        add     ebx, PAGE_SIZE-1
        and     ebx, -PAGE_SIZE
        add     [edx + PROC.mem_used], ebx

        lea     ecx, [edx + PROC.heap_lock]
        call    mutex_unlock

        mov     eax, [address]

        pop     edi
        pop     esi
        pop     ebx
        ret
endp

align 4
proc user_free stdcall, base:dword

        push    esi

        mov     esi, [base]
        test    esi, esi
        jz      .fail

        push    ebx

        mov     ebx, [current_process]
        lea     ecx, [ebx + PROC.heap_lock]
        call    mutex_lock

        xor     ebx, ebx
        shr     esi, 12
        mov     eax, [page_tabs + (esi-1)*4]
        test    al, MEM_BLOCK_USED
        jz      .cantfree
        test    al, MEM_BLOCK_DONT_FREE
        jnz     .cantfree

        and     eax, -PAGE_SIZE
        mov     ecx, eax
        or      al, MEM_BLOCK_FREE
        mov     [page_tabs + (esi-1)*4], eax
        sub     ecx, PAGE_SIZE
        mov     ebx, ecx
        shr     ecx, 12
        jz      .released
  .release:
        xor     eax, eax
        xchg    eax, [page_tabs + esi*4]
        test    al, 1
        jz      @F
        test    eax, PG_SHARED
        jnz     @F
        call    free_page
        mov     eax, esi
        shl     eax, 12
        invlpg  [eax]
  @@:
        inc     esi
        dec     ecx
        jnz     .release

 .released:
        push    edi

        mov     edx, [current_process]
        lea     ecx, [edx + PROC.heap_lock]
        mov     esi, dword [edx + PROC.heap_base]
        mov     edi, dword [edx + PROC.heap_top]
        sub     ebx, [edx + PROC.mem_used]
        neg     ebx
        mov     [edx + PROC.mem_used], ebx
        call    user_normalize
        pop     edi
 .exit:
        call    mutex_unlock

        xor     eax, eax
        inc     eax
        pop     ebx
        pop     esi
        ret

  .cantfree:
        mov     ecx, [current_process]
        lea     ecx, [ecx + PROC.heap_lock]
        jmp     .exit
  .fail:
        xor     eax, eax
        pop     esi
        ret
endp


align 4
proc user_unmap stdcall, base:dword, offset:dword, size:dword

        push    ebx

        mov     ebx, [base]             ; must be valid pointer
        test    ebx, ebx
        jz      .error

        mov     edx, [offset]           ; check offset
        add     edx, ebx                ; must be below 2Gb app limit
        js      .error

        shr     ebx, 12                 ; chek block attributes
        lea     ebx, [page_tabs + ebx*4]
        mov     eax, [ebx - 4]          ; block attributes
        test    al, MEM_BLOCK_USED
        jz      .error
        test    al, MEM_BLOCK_DONT_FREE
        jnz     .error

        shr     edx, 12
        lea     edx, [page_tabs + edx*4]  ; unmap offset

        mov     ecx, [size]
        add     ecx, PAGE_SIZE-1
        shr     ecx, 12                 ; unmap size in pages

        shr     eax, 12                 ; block size  +  1 page
        lea     ebx, [ebx + eax*4-4]      ; block end ptr
        lea     eax, [edx + ecx*4]        ; unmap end ptr

        cmp     eax, ebx                ; check for overflow
        ja      .error

        mov     ebx, [offset]
        and     ebx, -PAGE_SIZE         ; is it required ?
        add     ebx, [base]

  .unmap:
        mov     eax, [edx]              ; get page addres
        test    al, 1                   ; page mapped ?
        jz      @F
        test    eax, PG_SHARED          ; page shared ?
        jnz     @F
        mov     dword[edx], MEM_BLOCK_RESERVED
                                        ; mark page as reserved
        invlpg  [ebx]                   ; when we start using
        call    free_page               ; empty c-o-w page instead this ?
  @@:
        add     ebx, PAGE_SIZE
        add     edx, 4
        dec     ecx
        jnz     .unmap

        pop     ebx
        or      al, 1                   ; return non zero on success
        ret
.error:
        pop     ebx
        xor     eax, eax                ; something wrong
        ret
endp

align 4
user_normalize:
; in: esi=heap_base, edi=heap_top
; out: eax=0 <=> OK
; destroys: ebx,edx,esi,edi
        shr     esi, 12
        shr     edi, 12
@@:
        mov     eax, [page_tabs + esi*4]
        test    al, MEM_BLOCK_USED
        jz      .test_free
        shr     eax, 12
        add     esi, eax
        jmp     @B
.test_free:
        test    al, MEM_BLOCK_FREE
        jz      .err
        mov     edx, eax
        shr     edx, 12
        add     edx, esi
        cmp     edx, edi
        jae     .exit

        mov     ebx, [page_tabs + edx*4]
        test    bl, MEM_BLOCK_USED
        jz      .next_free

        shr     ebx, 12
        add     edx, ebx
        mov     esi, edx
        jmp     @B
.next_free:
        test    bl, MEM_BLOCK_FREE
        jz      .err
        and     dword[page_tabs + edx*4], 0
        add     eax, ebx
        and     eax, -PAGE_SIZE
        or      eax, MEM_BLOCK_FREE
        mov     [page_tabs + esi*4], eax
        jmp     @B
.exit:
        xor     eax, eax
        inc     eax
        ret
.err:
        xor     eax, eax
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

        push    eax
        mov     ecx, [current_process]
        lea     ecx, [ecx + PROC.heap_lock]
        call    mutex_lock
        pop     eax

        lea     ecx, [eax - PAGE_SIZE]
        shr     ecx, 12
        mov     edx, [page_tabs + ecx*4]
        test    dl, MEM_BLOCK_USED
        jnz     @f
; attempt to realloc invalid pointer
.ret0:
        mov     ecx, [current_process]
        lea     ecx, [ecx + PROC.heap_lock]
        call    mutex_unlock

        pop     edx ecx
        xor     eax, eax
        ret
@@:
        test    dl, MEM_BLOCK_DONT_FREE
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
.loop:
        cmp     edx, ebx
        jz      .release_done
        dec     edx
        xor     eax, eax
        xchg    eax, [page_tabs + edx*4]
        test    al, 1
        jz      .loop
        call    free_page
        mov     eax, edx
        shl     eax, 12
        invlpg  [eax]
        jmp     .loop
.release_done:
        sub     ebx, ecx
        cmp     ebx, 1
        jnz     .nofreeall
        mov     eax, [page_tabs + ecx*4]
        and     eax, -PAGE_SIZE
        mov     edx, [current_process]
        mov     ebx, [edx + PROC.mem_used]
        sub     ebx, eax
        add     ebx, PAGE_SIZE
        or      al, MEM_BLOCK_FREE
        mov     [page_tabs + ecx*4], eax
        push    esi edi
        mov     esi, [edx + PROC.heap_base]
        mov     edi, [edx + PROC.heap_top]
        mov     [edx + PROC.mem_used], ebx
        call    user_normalize
        pop     edi esi
        jmp     .ret0   ; all freed
.nofreeall:
        sub     edx, ecx
        shl     ebx, 12
        or      ebx, MEM_BLOCK_USED
        xchg    [page_tabs + ecx*4], ebx
        shr     ebx, 12
        sub     ebx, edx
        push    ebx ecx edx
        mov     edx, [current_process]
        shl     ebx, 12
        sub     ebx, [edx + PROC.mem_used]
        neg     ebx
        mov     [edx + PROC.mem_used], ebx
        pop     edx ecx ebx
        lea     eax, [ecx + 1]
        shl     eax, 12
        push    eax
        add     ecx, edx
        lea     edx, [ecx + ebx]
        shl     ebx, 12
        jz      .ret
        push    esi
        mov     esi, [current_process]
        mov     esi, [esi + PROC.heap_top]
        shr     esi, 12
@@:
        cmp     edx, esi
        jae     .merge_done
        mov     eax, [page_tabs + edx*4]
        test    al, MEM_BLOCK_USED
        jnz     .merge_done
        and     dword [page_tabs + edx*4], 0
        shr     eax, 12
        add     edx, eax
        shl     eax, 12
        add     ebx, eax
        jmp     @b
.merge_done:
        pop     esi
        or      ebx, MEM_BLOCK_FREE
        mov     [page_tabs + ecx*4], ebx
.ret:
        mov     ecx, [current_process]
        lea     ecx, [ecx + PROC.heap_lock]
        call    mutex_unlock
        pop     eax edx ecx
        ret

.realloc_add:
; get some additional memory
        mov     eax, [current_process]
        mov     eax, [eax + PROC.heap_top]
        shr     eax, 12
        cmp     edx, eax
        jae     .cant_inplace
        mov     eax, [page_tabs + edx*4]
        test    al, MEM_BLOCK_FREE
        jz      .cant_inplace
        shr     eax, 12
        add     eax, edx
        sub     eax, ebx
        jb      .cant_inplace
        jz      @f
        shl     eax, 12
        or      al, MEM_BLOCK_FREE
        mov     [page_tabs + ebx*4], eax
@@:
        mov     eax, ebx
        sub     eax, ecx
        shl     eax, 12
        or      al, MEM_BLOCK_USED
        mov     [page_tabs + ecx*4], eax
        lea     eax, [ecx + 1]
        shl     eax, 12
        push    eax
        push    edi
        lea     edi, [page_tabs + edx*4]
        mov     eax, 2
        sub     ebx, edx
        mov     ecx, ebx
        cld
        rep stosd
        pop     edi
        mov     edx, [current_process]
        shl     ebx, 12
        add     [edx + PROC.mem_used], ebx

        mov     ecx, [current_process]
        lea     ecx, [ecx + PROC.heap_lock]
        call    mutex_unlock
        pop     eax edx ecx
        ret

.cant_inplace:
        push    esi edi
        mov     eax, [current_process]
        mov     esi, [eax + PROC.heap_base]
        mov     edi, [eax + PROC.heap_top]
        shr     esi, 12
        shr     edi, 12
        sub     ebx, ecx
.find_place:
        cmp     esi, edi
        jae     .place_not_found
        mov     eax, [page_tabs + esi*4]
        test    al, MEM_BLOCK_FREE
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
        or      al, MEM_BLOCK_FREE
        mov     [page_tabs + esi*4], eax
        pop     esi
@@:
        mov     eax, ebx
        shl     eax, 12
        or      al, MEM_BLOCK_USED
        mov     [page_tabs + esi*4], eax
        inc     esi
        mov     eax, esi
        shl     eax, 12
        push    eax
        mov     eax, [page_tabs + ecx*4]
        and     eax, -PAGE_SIZE
        or      al, MEM_BLOCK_FREE
        sub     edx, ecx
        mov     [page_tabs + ecx*4], eax
        inc     ecx
        dec     ebx
        dec     edx
        jz      .no
@@:
        xor     eax, eax
        xchg    eax, [page_tabs + ecx*4]
        mov     [page_tabs + esi*4], eax
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
        mov     edx, [current_process]
        shl     ebx, 12
        add     [edx + PROC.mem_used], ebx
        pop     ebx
@@:
        mov     dword [page_tabs + esi*4], MEM_BLOCK_RESERVED
        inc     esi
        dec     ebx
        jnz     @b

        mov     ecx, [current_process]
        lea     ecx, [ecx + PROC.heap_lock]
        call    mutex_unlock
        pop     eax edi esi edx ecx
        ret



;;;;;;;;;;;;;;      SHARED MEMORY     ;;;;;;;;;;;;;;;;;


; param
;  eax= shm_map object

align 4
destroy_smap:

        pushfd
        cli

        push    esi
        push    edi

        mov     edi, eax
        mov     esi, [eax + SMAP.parent]
        test    esi, esi
        jz      .done

        lock dec [esi + SMEM.refcount]
        jnz     .done

        mov     ecx, [esi + SMEM.bk]
        mov     edx, [esi + SMEM.fd]

        mov     [ecx + SMEM.fd], edx
        mov     [edx + SMEM.bk], ecx

        stdcall kernel_free, [esi + SMEM.base]
        mov     eax, esi
        call    free
.done:
        mov     eax, edi
        call    destroy_kernel_object

        pop     edi
        pop     esi
        popfd

        ret

E_NOTFOUND      =  5
E_ACCESS        = 10
E_NOMEM         = 30
E_PARAM         = 33

SHM_READ        = 0
SHM_WRITE       = 1

SHM_ACCESS_MASK = 3

SHM_OPEN        = 0 shl 2
SHM_OPEN_ALWAYS = 1 shl 2
SHM_CREATE      = 2 shl 2

SHM_OPEN_MASK   = 3 shl 2

align 4
proc shmem_open stdcall name:dword, size:dword, access:dword
        locals
           action         dd ?
           owner_access   dd ?
           mapped         dd ?
        endl

        push    ebx
        push    esi
        push    edi

        mov     [mapped], 0
        mov     [owner_access], 0

        pushfd                         ;mutex required
        cli

        mov     eax, [access]
        and     eax, SHM_OPEN_MASK
        mov     [action], eax

        mov     ebx, [name]
        test    ebx, ebx
        mov     edx, E_PARAM
        jz      .fail

        mov     esi, [shmem_list.fd]
align 4
@@:
        cmp     esi, shmem_list
        je      .not_found

        lea     edx, [esi + SMEM.name]; link , base, size
        stdcall strncmp, edx, ebx, 32
        test    eax, eax
        je      .found

        mov     esi, [esi + SMEM.fd]
        jmp     @B

.not_found:
        mov     eax, [action]

        cmp     eax, SHM_OPEN
        mov     edx, E_NOTFOUND
        je      .fail

        cmp     eax, SHM_CREATE
        mov     edx, E_PARAM
        je      .create_shm

        cmp     eax, SHM_OPEN_ALWAYS
        jne     .fail

.create_shm:

        mov     ecx, [size]
        test    ecx, ecx
        jz      .fail

        add     ecx, PAGE_SIZE-1
        and     ecx, -PAGE_SIZE
        mov     [size], ecx

        mov     eax, sizeof.SMEM
        call    malloc
        test    eax, eax
        mov     esi, eax
        mov     edx, E_NOMEM
        jz      .fail

        stdcall kernel_alloc, [size]
        test    eax, eax
        mov     [mapped], eax
        mov     edx, E_NOMEM
        jz      .cleanup

        mov     ecx, [size]
        mov     edx, [access]
        and     edx, SHM_ACCESS_MASK

        mov     [esi + SMEM.base], eax
        mov     [esi + SMEM.size], ecx
        mov     [esi + SMEM.access], edx
        mov     [esi + SMEM.refcount], 0
        mov     [esi + SMEM.name + 28], 0

        lea     eax, [esi + SMEM.name]
        stdcall strncpy, eax, [name], 31

        mov     eax, [shmem_list.fd]
        mov     [esi + SMEM.bk], shmem_list
        mov     [esi + SMEM.fd], eax

        mov     [eax + SMEM.bk], esi
        mov     [shmem_list.fd], esi

        mov     [action], SHM_OPEN
        mov     [owner_access], SHM_WRITE

.found:
        mov     eax, [action]

        cmp     eax, SHM_CREATE
        mov     edx, E_ACCESS
        je      .exit

        cmp     eax, SHM_OPEN
        mov     edx, E_PARAM
        je      .create_map

        cmp     eax, SHM_OPEN_ALWAYS
        jne     .fail

.create_map:

        mov     eax, [access]
        and     eax, SHM_ACCESS_MASK
        cmp     eax, [esi + SMEM.access]
        mov     [access], eax
        mov     edx, E_ACCESS
        ja      .fail

        mov     ebx, [current_slot]
        mov     ebx, [ebx + APPDATA.tid]
        mov     eax, sizeof.SMAP

        call    create_kernel_object
        test    eax, eax
        mov     edi, eax
        mov     edx, E_NOMEM
        jz      .fail

        inc     [esi + SMEM.refcount]

        mov     [edi + SMAP.magic], 'SMAP'
        mov     [edi + SMAP.destroy], destroy_smap
        mov     [edi + SMAP.parent], esi
        mov     [edi + SMAP.base], 0

        stdcall user_alloc, [esi + SMEM.size]
        test    eax, eax
        mov     [mapped], eax
        mov     edx, E_NOMEM
        jz      .cleanup2

        mov     [edi + SMAP.base], eax

        mov     ecx, [esi + SMEM.size]
        mov     [size], ecx

        shr     ecx, 12
        shr     eax, 10

        mov     esi, [esi + SMEM.base]
        shr     esi, 10
        lea     edi, [page_tabs + eax]
        add     esi, page_tabs

        mov     edx, [access]
        or      edx, [owner_access]
        shl     edx, 1
        or      edx, PG_SHARED + PG_UR
@@:
        lodsd
        and     eax, -PAGE_SIZE
        or      eax, edx
        stosd
        loop    @B

        xor     edx, edx

        cmp     [owner_access], 0
        jne     .fail
.exit:
        mov     edx, [size]
.fail:
        mov     eax, [mapped]

        popfd
        pop     edi
        pop     esi
        pop     ebx
        ret
.cleanup:
        mov     [size], edx
        mov     eax, esi
        call    free
        jmp     .exit

.cleanup2:
        mov     [size], edx
        mov     eax, edi
        call    destroy_smap
        jmp     .exit
endp

align 4
proc shmem_close stdcall, name:dword

        mov     eax, [name]
        test    eax, eax
        jz      .fail

        push    ebx ; Added
        push    esi
        push    edi
        pushfd
        cli

        mov     esi, [current_slot]
        add     esi, APP_OBJ_OFFSET
        mov     ebx, esi ; Fixed endless loop bug with not existing name (part 1)
.next:
        mov     eax, [esi + APPOBJ.fd]
        test    eax, eax
        jz      @F

        cmp     eax, ebx ;esi ; Fixed endless loop bug with not existing name (part 2)
        je      @F            ; Small optimization
        mov     esi, eax
        ;je      @F

        cmp     [eax + SMAP.magic], 'SMAP'
        jne     .next

        mov     edi, [eax + SMAP.parent]
        test    edi, edi
        jz      .next

        lea     edi, [edi + SMEM.name]
        stdcall strncmp, [name], edi, 32
        test    eax, eax
        jne     .next

        stdcall user_free, [esi + SMAP.base]

        mov     eax, esi
        call    [esi + APPOBJ.destroy]
@@:
        popfd
        pop     edi
        pop     esi
        pop     ebx ; Added
.fail:
        ret
endp



proc    user_ring stdcall, size:dword

locals
        virt_ptr        dd ?
        phys_ptr        dd ?
        num_pages       dd ?
endl

; Size must be an exact multiple of pagesize
        mov     eax, [size]
        test    eax, PAGE_SIZE-1
        jnz     .exit

; We must have at least one complete page
        shr     eax, 12
        jz      .exit
        mov     [num_pages], eax

; Allocate double the virtual memory
        mov     eax, [size]
        shl     eax, 1
        jz      .exit
        stdcall user_alloc, eax
        test    eax, eax
        jz      .exit
        mov     [virt_ptr], eax

; Now allocate physical memory
        stdcall alloc_pages, [num_pages]
        test    eax, eax
        jz      .exit_free_virt
        mov     [phys_ptr], eax

; Map first half of virtual memory to physical memory
        push    ecx esi edi
        mov     ecx, [num_pages]
        mov     esi, [virt_ptr]
        mov     edi, [phys_ptr]
  .loop1:
        stdcall map_page, esi, edi, PG_UWR
        add     esi, PAGE_SIZE
        add     edi, PAGE_SIZE
        dec     ecx
        jnz     .loop1

; Map second half of virtual memory to same physical memory
        mov     ecx, [num_pages]
        mov     edi, [phys_ptr]
  .loop2:
        stdcall map_page, esi, edi, PG_UWR
        add     esi, PAGE_SIZE
        add     edi, PAGE_SIZE
        dec     ecx
        jnz     .loop2
        pop     edi esi ecx

        mov     eax, [virt_ptr]
        ret

  .exit_free_virt:
        stdcall user_free, [virt_ptr]

  .exit:
        xor     eax, eax
        ret

endp