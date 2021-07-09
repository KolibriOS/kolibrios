;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2020. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


align 4
proc alloc_page

        pushfd
        cli
        push    ebx

        cmp     [pg_data.pages_free], 1
        jle     .out_of_memory


        mov     ebx, [page_start]
        mov     ecx, [page_end]
.l1:
        bsf     eax, [ebx];
        jnz     .found
        add     ebx, 4
        cmp     ebx, ecx
        jb      .l1
        pop     ebx
        popfd
        xor     eax, eax
        ret
.found:

        dec     [pg_data.pages_free]
        jz      .out_of_memory

        btr     [ebx], eax
        mov     [page_start], ebx
        sub     ebx, sys_pgmap
        lea     eax, [eax+ebx*8]
        shl     eax, 12
;       dec [pg_data.pages_free]
        pop     ebx
        popfd
        ret

.out_of_memory:
        mov     [pg_data.pages_free], 1
        xor     eax, eax
        pop     ebx
        popfd
        ret

endp

align 4
proc alloc_pages stdcall, count:dword
        pushfd
        push    ebx
        push    edi
        cli
        mov     eax, [count]
        add     eax, 7
        shr     eax, 3
        mov     [count], eax

        mov     ebx, [pg_data.pages_free]
        sub     ebx, 9
        js      .out_of_memory
        shr     ebx, 3
        cmp     eax, ebx
        jg      .out_of_memory

        mov     ecx, [page_start]
        mov     ebx, [page_end]
.find:
        mov     edx, [count]
        mov     edi, ecx
  .match:
        cmp     byte [ecx], 0xFF
        jne     .next
        dec     edx
        jz      .ok
        inc     ecx
        cmp     ecx, ebx
        jb      .match
  .out_of_memory:
  .fail:
        xor     eax, eax
        pop     edi
        pop     ebx
        popfd
        ret
  .next:
        inc     ecx
        cmp     ecx, ebx
        jb      .find
        pop     edi
        pop     ebx
        popfd
        xor     eax, eax
        ret
  .ok:
        sub     ecx, edi
        inc     ecx
        push    esi
        mov     esi, edi
        xor     eax, eax
        rep stosb
        sub     esi, sys_pgmap
        shl     esi, 3+12
        mov     eax, esi
        mov     ebx, [count]
        shl     ebx, 3
        sub     [pg_data.pages_free], ebx
        pop     esi
        pop     edi
        pop     ebx
        popfd
        ret
endp

align 4
;proc map_page stdcall,lin_addr:dword,phis_addr:dword,flags:dword
map_page:
        push    ebx
        mov     eax, [esp+12]              ; phis_addr
        or      eax, [esp+16]              ; flags
        and     eax, [pte_valid_mask]
        mov     ebx, [esp+8]               ; lin_addr
        shr     ebx, 12
        mov     [page_tabs+ebx*4], eax
        mov     eax, [esp+8]               ; lin_addr
        pop     ebx
        invlpg  [eax]
        ret     12

align 4
map_space:    ;not implemented


        ret


align 4
proc free_page
;arg:  eax  page address
        pushfd
        cli
        shr     eax, 12                       ;page index
        bts     dword [sys_pgmap], eax        ;that's all!
        cmc
        adc     [pg_data.pages_free], 0
        shr     eax, 3
        and     eax, not 3                    ;dword offset from page_map
        add     eax, sys_pgmap
        cmp     [page_start], eax
        ja      @f
        popfd
        ret
@@:
        mov     [page_start], eax
        popfd
        ret
endp

align 4
proc map_io_mem stdcall, base:dword, size:dword, flags:dword

        push    ebx
        push    edi
        mov     eax, [size]
        add     eax, [base]
        add     eax, 4095
        and     eax, -4096
        mov     ecx, [base]
        and     ecx, -4096
        sub     eax, ecx
        mov     [size], eax

        stdcall alloc_kernel_space, eax
        test    eax, eax
        jz      .fail
        push    eax

        mov     edi, 0x1000
        mov     ebx, eax
        mov     ecx, [size]
        mov     edx, [base]
        shr     eax, 12
        shr     ecx, 12
        or      edx, [flags]
        and     edx, [pte_valid_mask]
@@:
        mov     [page_tabs+eax*4], edx
        invlpg  [ebx]
        inc     eax
        add     ebx, edi
        add     edx, edi
        loop    @B

        pop     eax
        mov     edx, [base]
        and     edx, 4095
        add     eax, edx
.fail:
        pop     edi
        pop     ebx
        ret
endp

; param
;  eax= page base + page flags
;  ebx= linear address
;  ecx= count

align 4
commit_pages:
        test    ecx, ecx
        jz      .fail

        push    edi
        push    eax
        push    ecx
        mov     ecx, pg_data.mutex
        call    mutex_lock
        pop     ecx
        pop     eax

        and     eax, [pte_valid_mask ]
        mov     edi, ebx
        shr     edi, 12
        lea     edi, [page_tabs+edi*4]
@@:
        stosd
        invlpg  [ebx]
        add     eax, 0x1000
        add     ebx, 0x1000
        loop    @B

        pop     edi

        mov     ecx, pg_data.mutex
        call    mutex_unlock
.fail:
        ret


; param
;  eax= base
;  ecx= count

align 4
release_pages:

        push    ebp
        push    esi
        push    edi
        push    ebx

        mov     esi, eax
        mov     edi, eax

        shr     esi, 12
        lea     esi, [page_tabs+esi*4]

        push    ecx
        mov     ecx, pg_data.mutex
        call    mutex_lock
        pop     ecx

        mov     ebp, [pg_data.pages_free]
        mov     ebx, [page_start]
        mov     edx, sys_pgmap
@@:
        xor     eax, eax
        xchg    eax, [esi]
        invlpg  [edi]

        test    eax, 1
        jz      .next

        shr     eax, 12
        bts     [edx], eax
        cmc
        adc     ebp, 0
        shr     eax, 3
        and     eax, -4
        add     eax, edx
        cmp     eax, ebx
        jae     .next

        mov     ebx, eax
.next:
        add     edi, 0x1000
        add     esi, 4
        loop    @B

        mov     [pg_data.pages_free], ebp
        mov     ecx, pg_data.mutex
        call    mutex_unlock

        pop     ebx
        pop     edi
        pop     esi
        pop     ebp
        ret

; param
;  eax= base
;  ecx= count

align 4
unmap_pages:

        push    edi

        mov     edi, eax
        mov     edx, eax

        shr     edi, 10
        add     edi, page_tabs

        xor     eax, eax
@@:
        stosd
        invlpg  [edx]
        add     edx, 0x1000
        loop    @b

        pop     edi
        ret


align 4
proc map_page_table stdcall, lin_addr:dword, phis_addr:dword
        push    ebx
        mov     ebx, [lin_addr]
        shr     ebx, 22
        mov     eax, [phis_addr]
        and     eax, not 0xFFF
        or      eax, PG_UWR
        mov     dword [master_tab+ebx*4], eax
        mov     eax, [lin_addr]
        shr     eax, 10
        add     eax, page_tabs
        invlpg  [eax]
        pop     ebx
        ret
endp

uglobal
sb16_buffer_allocated db 0
endg

; Allocates [.size] bytes so that the target memory block
; is inside one 64K page for 24-bit DMA controller,
; that is, somewhere between 00xx0000h and 00xxFFFFh.
proc alloc_dma24
; Implementation note.
; The only user of that function is SB16 driver,
; so just return a statically allocated buffer.
virtual at esp
                dd      ? ; return address
.size           dd      ?
end virtual
        cmp     [sb16_buffer_allocated], 0
        jnz     .fail
        inc     [sb16_buffer_allocated]
        mov     eax, SB16Buffer
        ret     4
.fail:
        xor     eax, eax
        ret     4
endp

; Allocates a physical page for master page table
; that duplicates first Mb of OS_BASE at address 0;
; used for starting APs and for shutting down,
; where it is important to execute code in trivial-mapped pages.
; Returns eax = allocated physical page.
proc create_trampoline_pgmap
; The only non-trivial moment:
; we need a linear address to fill information,
; but we don't need it outside of this function,
; so we're returning physical address.
; Therefore, allocate memory with kernel_alloc,
; this will allocate physical page and a linear address somewhere,
; and deallocate only linear address with free_kernel_space.
        stdcall kernel_alloc, 0x1000
        mov     edi, eax
        mov     esi, master_tab
        mov     ecx, 1024
        rep movsd
        mov     ecx, [master_tab+(OS_BASE shr 20)]
        mov     [eax], ecx
        mov     edi, eax
        call    get_pg_addr
        push    eax
        stdcall free_kernel_space, edi
        pop     eax
        ret
endp

align 4
proc new_mem_resize stdcall, new_size:dword

        push    ebx
        push    esi
        push    edi

        mov     edx, [current_slot]
        mov     ebx, [edx+APPDATA.process]

        cmp     [ebx+PROC.heap_base], 0
        jne     .exit

        mov     edi, [new_size]
        add     edi, 4095
        and     edi, not 4095
        mov     [new_size], edi

        mov     esi, [ebx+PROC.mem_used]
        add     esi, 4095
        and     esi, not 4095

        cmp     edi, esi
        ja      .expand
        je      .exit

        mov     ebx, edi
        shr     edi, 12
        shr     esi, 12

        mov     ecx, pg_data.mutex
        call    mutex_lock
@@:
        mov     eax, [app_page_tabs+edi*4]
        test    eax, 1
        jz      .next

        mov     dword [app_page_tabs+edi*4], 0
        invlpg  [ebx]
        call    free_page

.next:
        inc     edi
        add     ebx, 0x1000
        cmp     edi, esi
        jb      @B

        mov     ecx, pg_data.mutex
        call    mutex_unlock

.update_size:
        mov     edx, [current_slot]
        mov     ebx, [new_size]
        mov     edx, [edx+APPDATA.process]
        mov     [edx+PROC.mem_used], ebx
.exit:
        pop     edi
        pop     esi
        pop     ebx
        xor     eax, eax
        ret

.expand:

        mov     ecx, pg_data.mutex
        call    mutex_lock

        xchg    esi, edi

        push    esi                   ;new size
        push    edi                   ;old size

        add     edi, 0x3FFFFF
        and     edi, not(0x3FFFFF)
        add     esi, 0x3FFFFF
        and     esi, not(0x3FFFFF)

        cmp     edi, esi
        jae     .grow
 @@:
        call    alloc_page
        test    eax, eax
        jz      .exit_fail

        stdcall map_page_table, edi, eax

        push    edi
        shr     edi, 10
        add     edi, page_tabs
        mov     ecx, 1024
        xor     eax, eax
        cld
        rep stosd
        pop     edi

        add     edi, 0x00400000
        cmp     edi, esi
        jb      @B
.grow:
        pop     edi                   ;old size
        pop     ecx                   ;new size

        shr     edi, 10
        shr     ecx, 10
        sub     ecx, edi
        shr     ecx, 2                ;pages count
        mov     eax, 2

        add     edi, app_page_tabs
        rep stosd

        mov     ecx, pg_data.mutex
        call    mutex_unlock

        jmp     .update_size

.exit_fail:
        mov     ecx, pg_data.mutex
        call    mutex_unlock

        add     esp, 8
        pop     edi
        pop     esi
        pop     ebx
        xor     eax, eax
        inc     eax
        ret
endp


; param
;  eax= linear address
;
; retval
;  eax= physical page address

align 4
get_pg_addr:
        sub     eax, OS_BASE
        cmp     eax, 0x400000
        jb      @f
        shr     eax, 12
        mov     eax, [page_tabs+(eax+(OS_BASE shr 12))*4]
@@:
        and     eax, 0xFFFFF000
        ret


align 4
; Now it is called from core/sys32::exc_c (see stack frame there)
proc page_fault_handler

    .err_addr   equ ebp-4

        push    ebx               ;save exception number (#PF)
        mov     ebp, esp
        mov     ebx, cr2
        push    ebx               ;that is locals: .err_addr = cr2
        inc     [pg_data.pages_faults]

        mov     esi, [pf_err_code]

        cmp     ebx, OS_BASE      ;ebx == .err_addr
        jb      .user_space       ;page in application memory

        cmp     ebx, page_tabs
        jb      .kernel_space     ;page in kernel memory

        xor     eax, eax
        cmp     ebx, kernel_tabs
        jb      .alloc;.app_tabs  ;page tables of application ;
                                  ;simply create one
.core_tabs:
.fail:  ;simply return to caller
        mov     esp, ebp
        pop     ebx               ;restore exception number (#PF)
        ret
.fail_maybe_unlock:
        test    esi, esi
        jz      .fail
.fail_unlock:
        mov     ecx, [current_process]
        add     ecx, PROC.heap_lock
        call    mutex_unlock
        jmp     .fail

.user_space:
; PF entry in IDT is interrupt gate, so up to this moment
; atomicity was guaranteed by cleared IF.
; It is inpractical to guard all modifications in the page table by cli/sti,
; so enable interrupts and acquire the address space lock.
; Unfortunately, that enables the scenario when the fault of current thread
; is resolved by another thread when the current thread waits in mutex_lock,
; so watch out: in lock-protected section we can find out that
; there is no error already.
        sti
        mov     ecx, [current_process]
        add     ecx, PROC.heap_lock
        call    mutex_lock
        test    esi, PG_READ
        jnz     .err_access       ;Page is present
                                  ;Access error ?

        shr     ebx, 12
        mov     ecx, ebx
        shr     ecx, 10
        test    dword [master_tab+ecx*4], PG_READ
        jz      .fail_unlock      ;page table is not created
                                  ;incorrect address in program

        mov     eax, [page_tabs+ebx*4]
        test    eax, PG_READ
        jnz     .exit_unlock      ; already resolved by a parallel thread
        test    eax, LAZY_ALLOC_PAGE
        jz      .fail_unlock      ;address is not reserved for use, error
        test    eax, LAZY_ALLOC_UNREADABLE
        jnz     .fail_unlock
.alloc:
        mov     esi, eax
        call    alloc_zero_page
        test    eax, eax
        jz      .fail_maybe_unlock

        mov     edx, PG_UWR
        test    esi, LAZY_ALLOC_UNWRITABLE
        jz      @f
        mov     edx, PG_UR
@@:
        stdcall map_page, [.err_addr], eax, edx
        mov     ecx, [current_process]
        add     [ecx+PROC.mem_used], 0x1000
.exit_unlock:
        mov     ecx, [current_process]
        add     ecx, PROC.heap_lock
        call    mutex_unlock
.exit:  ;iret with repeat fault instruction
        add     esp, 12;clear in stack: locals(.err_addr) + #PF + ret_to_caller
        restore_ring3_context
        iretd

.err_access:
; access denied? this may be a result of copy-on-write protection
; Check whether the problem has already been resolved
; while we were waiting in mutex_lock.
        mov     eax, ebx
        shr     eax, 12
        mov     eax, [page_tabs+eax*4]
        test    eax, PG_READ
        jz      .fail_unlock ; someone has free'd the page
        test    eax, PG_USER
        jz      .fail_unlock ; page is mprotect'ed without PROT_READ
        test    eax, PG_WRITE
        jnz     .exit_unlock ; someone has enabled write
        test    eax, PG_SHARED
        jz      .fail_unlock ; only shared pages can be copy-on-write
; check list of mapped data
        and     ebx, not 0xFFF
        call    find_smap_by_address
        test    esi, esi
        jz      .fail_unlock
; ignore children of SMEM, only children of PEDESCR can have copy-on-write data
        cmp     [esi+SMAP.type], SMAP_TYPE_PE
        jnz     .fail_unlock
        shr     edi, 12
; lock page array in PEDESCR
        mov     esi, [esi+SMAP.parent]
        lea     ecx, [esi+PEDESCR.page_array_lock]
        push    eax
        call    mutex_lock
        pop     eax
; check whether the page is shared
; PG_SHARED flag could be set by lock_and_map_page
        xor     eax, [esi+sizeof.PEDESCR+edi*4]
        test    eax, not 0xFFF
        jnz     .fail_unlock2
; check whether write is allowed by section attributes
        mov     eax, [esi+sizeof.PEDESCR+edi*4]
        test    eax, IMAGE_SCN_MEM_WRITE shr 20
        jz      .fail_unlock2
; if we're faulting on the page which was originally shareable writable,
; it means that someone has disabled writing with mprotect; fail
        test    eax, IMAGE_SCN_MEM_SHARED shr 20
        jnz     .fail_unlock2
        stdcall pe_copy_on_write, PG_UWR
        jc	.fail_unlock2
.exit_unlock2:
        lea     ecx, [esi+PEDESCR.page_array_lock]
        call    mutex_unlock
        jmp     .exit_unlock
.fail_unlock2:
        lea     ecx, [esi+PEDESCR.page_array_lock]
        call    mutex_unlock
        jmp     .fail_unlock

.kernel_space:
        test    esi, PG_READ
        jz      .fail   ;page does not present

        test    esi, 12 ;U/S (+below)
        jnz     .fail   ;application requested kernel memory
                        
       ;test    esi, 8
       ;jnz     .fail   ;the reserved bit is set in page tables
			;added in P4/Xeon

;an attempt to write to a protected kernel page

        cmp     ebx, tss._io_map_0
        jb      .fail

        cmp     ebx, tss._io_map_0+8192
        jae     .fail

; io permission map
; copy-on-write protection

        call    alloc_page
        test    eax, eax
        jz      .fail

        push    eax
        stdcall map_page, [.err_addr], eax, PG_SWR
        pop     eax
        mov     edi, [.err_addr]
        and     edi, -4096
        lea     esi, [edi+(not tss._io_map_0)+1]; -tss._io_map_0

        mov     ebx, esi
        shr     ebx, 12
        mov     edx, [current_slot]
        or      eax, PG_SWR
        mov     [edx+APPDATA.io_map+ebx*4], eax

        add     esi, [default_io_map]
        mov     ecx, 4096/4
       ;cld     ;caller is duty for this
        rep movsd
        jmp     .exit
endp

; Sometimes we can just allocate a page and let the caller fill it.
; Sometimes we need a zero-filled page, but we can zero it at the target.
; Sometimes we need a zero-filled page before mapping to the target.
; This function is for the last case.
; out: eax = physical page
; destroys: nothing
proc alloc_zero_page
	call	alloc_page
	test	eax, eax
	jz	.nothing
	spin_lock_irqsave zero_page_spinlock
	push	ecx edx edi eax
	mov	edi, [zero_page_tab]
	stdcall	map_page, edi, [esp+4], PG_SWR
	pushd	0 [esp+4] edi ; for map_page
	mov	ecx, 0x1000/4
	xor	eax, eax
	rep stosd
	call	map_page
	pop	eax edi edx ecx
	spin_unlock_irqrestore zero_page_spinlock
.nothing:
	ret
endp

; in: ebx = address
; out if SMAP exists for this address: esi -> SMAP, edi = ebx - SMAP.base
; out if SMAP does not exist: esi = 0
proc find_smap_by_address
        mov     edx, [current_process]
        add     edx, PROC.smap_list
        mov     esi, [edx+SMAP.fd]
.scan:
        cmp     esi, edx
        jz      .fail
        mov     edi, ebx
        sub     edi, [esi+SMAP.base]
        cmp     edi, [esi+SMAP.size]
        jb      .exit
        mov     esi, [esi+SMAP.fd]
        jmp     .scan
.fail:
        xor     esi, esi
.exit:
        ret
endp

; Someone is about to write to copy-on-write page inside mapped PE.
; Provide a page that can be written to.
; in: esi -> PEDESCR
; in: edi = page number inside PE
; in: eax = [esi+sizeof.PEDESCR+edi*4]
; in: ebx = address in process, must be page-aligned
; in: [esp+4] = access rights for the new page
; out: CF=0 - ok, CF=1 - error, no memory
proc pe_copy_on_write
; 1. Decrement reference counter unless it is 0xFF.
        mov     edx, eax
        and     edx, 0xFF
        cmp     edx, 0xFF
        jz      @f
        dec     eax
@@:
; 2. If reference counter is zero now, transfer ownership from PEDESCR to the process.
        test    eax, 0xFF
        jnz     .alloc_copy
        mov     dword [esi+sizeof.PEDESCR+edi*4], 0
        and     eax, not 0xFFF
.remap:
        stdcall map_page, ebx, eax, [esp+4]
        clc
        ret     4
.alloc_copy:
; 3. Otherwise, store updated reference counter to PEDESCR,
; allocate new page, map it as rw and copy data.
        mov     [esi+sizeof.PEDESCR+edi*4], eax
        stdcall kernel_alloc, 0x1000
        test    eax, eax
        jz      .error
        push    esi
        mov     esi, ebx
        mov     edi, eax
        mov     ecx, 0x1000/4
        rep movsd
        mov     esi, eax
        call    get_pg_addr
        push    eax
        stdcall free_kernel_space, esi
        pop     eax
        pop     esi
        jmp     .remap
.error:
        stc
        ret     4
endp

PROT_READ = 1
PROT_WRITE = 2
PROT_EXEC = 4
proc mprotect stdcall uses ebx esi edi, address:dword, size:dword, access:dword
locals
retval		dd	-1
smap_ptr	dd	0
endl
	mov	ecx, [current_process]
	add	ecx, PROC.heap_lock
	call	mutex_lock
	test	[access], not (PROT_READ+PROT_WRITE+PROT_EXEC)
	jnz	.error
	cmp	[size], 0
	jz	.error
	mov	eax, [address]
	add	[size], eax
	and	eax, not 0xFFF
.addrloop:
	mov	[address], eax
	mov	ecx, eax
	cmp	eax, OS_BASE
	jae	.error
	shr     eax, 22
	test    byte [master_tab+eax*4], PG_READ
	jz      .error
	shr	ecx, 12
	mov	eax, [page_tabs+ecx*4]
	test	al, PG_READ
	jnz	.page_present
	test	al, LAZY_ALLOC_PAGE
	jz	.error
	cmp	[retval], -1
	jnz	.skip_query
	inc	[retval]
	test	al, LAZY_ALLOC_UNREADABLE
	jnz	@f
	or	[retval], PROT_READ+PROT_EXEC
@@:
	test	al, LAZY_ALLOC_UNWRITABLE
	jnz	@f
	or	[retval], PROT_WRITE
@@:
.skip_query:
	and	al, not (LAZY_ALLOC_UNREADABLE+LAZY_ALLOC_UNWRITABLE)
	test	[access], PROT_READ
	jnz	@f
	or	al, LAZY_ALLOC_UNREADABLE
@@:
	test	[access], PROT_WRITE
	jnz	@f
	or	al, LAZY_ALLOC_UNWRITABLE
@@:
	mov	[page_tabs+ecx*4], eax
	jmp	.nextpage
.page_present:
	test	eax, PG_SHARED
	jnz	.page_shared
.normal_page:
	cmp	[retval], -1
	jnz	.skip_query2
	inc	[retval]
	test	al, PG_USER
	jz	@f
	or	[retval], PROT_READ+PROT_EXEC
@@:
	test	al, PG_WRITE
	jz	@f
	or	[retval], PROT_WRITE
@@:
.skip_query2:
	and	al, not (PG_USER+PG_WRITE)
	test	[access], PROT_READ
	jz	@f
	or	al, PG_USER
@@:
	test	[access], PROT_WRITE
	jz	@f
	or	al, PG_WRITE
@@:
	mov	[page_tabs+ecx*4], eax
	mov	eax, [address]
	invlpg	[eax]
	jmp	.nextpage
.page_shared:
	mov	esi, [smap_ptr]
	test	esi, esi
	jz	.find_smap
	mov	edx, [address]
	sub	edx, [esi+SMAP.base]
	cmp	edx, [esi+SMAP.size]
	jb	.found_smap
.find_smap:
	mov	ebx, [address]
	call	find_smap_by_address
	mov	[smap_ptr], esi
	test	esi, esi
	jz	.normal_page
.found_smap:
	cmp	[esi+SMAP.type], SMAP_TYPE_PE
	jnz	.error
	shr	edi, 12
	mov	esi, [esi+SMAP.parent]
	lea	ecx, [esi+PEDESCR.page_array_lock]
	push	eax
	call	mutex_lock
	pop	eax
	xor	eax, [esi+sizeof.PEDESCR+edi*4]
	test	eax, not 0xFFF
	jnz	.normal_page_unlock
	mov	eax, [esi+sizeof.PEDESCR+edi*4]
	test	eax, IMAGE_SCN_MEM_SHARED shr 20
	jnz	.normal_page_unlock
	cmp	[retval], -1
	jnz	.skip_query3
	mov	edx, [address]
	shr	edx, 12
	inc	[retval]
	test	byte [page_tabs+edx*4], PG_USER
	jz	@f
	or	[retval], PROT_READ+PROT_EXEC
@@:
	test	eax, IMAGE_SCN_MEM_WRITE shr 20
	jz	@f
	or	[retval], PROT_WRITE
@@:
.skip_query3:
	test	[access], PROT_WRITE
	jz	.no_write
	push	PG_SWR
	test	[access], PROT_READ
	jz	@f
	pop	edx
	push	PG_UWR
@@:
	call	pe_copy_on_write
	lea	ecx, [esi+PEDESCR.page_array_lock]
	call	mutex_unlock
	jmp	.nextpage
.normal_page_unlock:
	lea	ecx, [esi+PEDESCR.page_array_lock]
	call	mutex_unlock
	mov	ecx, [address]
	shr	ecx, 12
	mov	eax, [page_tabs+ecx*4]
	jmp	.normal_page
.no_write:
	lea	ecx, [esi+PEDESCR.page_array_lock]
	call	mutex_unlock
	mov	ecx, [address]
	shr	ecx, 12
	mov	eax, [page_tabs+ecx*4]
	jmp	.skip_query2
.nextpage:
	mov	eax, [address]
	add	eax, 0x1000
	cmp	eax, [size]
	jb	.addrloop
.exit:
	mov	ecx, [current_process]
	add	ecx, PROC.heap_lock
	call	mutex_unlock
	mov	eax, [retval]
	ret
.error:
	or	[retval], -1
	jmp	.exit
endp

; returns number of mapped bytes
proc map_memEx stdcall uses ebx esi edi, lin_addr:dword,slot:dword,\
                      ofs:dword,buf_size:dword,req_access:dword,where:dword
        locals
             count   dd ?
             process dd ?
        endl

        mov     [count], 0
        cmp     [buf_size], 0
        jz      .exit

        mov     eax, [slot]
        shl     eax, 8
        mov     ecx, [SLOT_BASE+eax+APPDATA.process]
        test    ecx, ecx
        jz      .exit

        mov     [process], ecx
        add     ecx, PROC.heap_lock
        call    mutex_lock
        mov     eax, [process]
        mov     ebx, [ofs]
        shr     ebx, 22
        mov     eax, [eax+PROC.pdt_0+ebx*4]                 ;get page table
        mov     esi, [where]
        and     eax, 0xFFFFF000
        jz      .unlock_exit
        stdcall map_page, esi, eax, PG_SWR
@@:
        mov     edi, [lin_addr]
        and     edi, 0xFFFFF000
        mov     ecx, [buf_size]
        add     ecx, 4095
        shr     ecx, 12

        mov     edx, [ofs]
        shr     edx, 12
        and     edx, 0x3FF
.map:
        stdcall lock_and_map_page, [slot], [req_access], [ofs]
        jnc     .unlock_exit
        add     [count], PAGE_SIZE
        add     [ofs], PAGE_SIZE
        dec     ecx
        jz      .unlock_exit

        add     edi, PAGE_SIZE
        inc     edx
        cmp     edx, 1024
        jnz     .map

        inc     ebx
        mov     eax, [process]
        mov     eax, [eax+PROC.pdt_0+ebx*4]
        and     eax, 0xFFFFF000
        jz      .unlock_exit

        stdcall map_page, esi, eax, PG_SWR
        xor     edx, edx
        jmp     .map
.unlock_exit:
        mov     ecx, [process]
        add     ecx, PROC.heap_lock
        call    mutex_unlock
.exit:
        mov     eax, [count]
        ret
endp

proc unmap_memEx stdcall uses ebx esi edi, lin_addr:dword,slot:dword,\
                      ofs:dword,mapped_size:dword,pagedir:dword
        locals
             process dd ?
        endl

        cmp     [mapped_size], 0
        jz      .exit

        mov     eax, [slot]
        shl     eax, 8
        mov     ecx, [SLOT_BASE+eax+APPDATA.process]
        mov     [process], ecx
        xor     eax, eax
        test    ecx, ecx
        jz      @f
        add     ecx, PROC.heap_lock
        call    mutex_lock
        mov     ebx, [ofs]
        shr     ebx, 22
        mov     eax, [process]
        mov     eax, [eax+PROC.pdt_0+ebx*4]                 ;get page table
@@:
        xor     esi, esi
        and     eax, 0xFFFFF000
        jz      @f
        mov     esi, [pagedir]
        stdcall map_page, esi, eax, PG_SWR
@@:
        mov     ecx, shared_locked_mutex
        call    mutex_lock
        mov     edi, [lin_addr]
        shr     edi, 12
        mov     ecx, [mapped_size]
        add     ecx, 4095
        shr     ecx, 12
        mov     [mapped_size], ecx

        mov     edx, [ofs]
        shr     edx, 12
        and     edx, 0x3FF
        lea     esi, [esi+edx*4]
.map:
        call    unlock_and_unmap_page
        dec     [mapped_size]
        jz      .done

        inc     edi
        add     esi, 4
        test    esi, 0xFFF
        jnz     .map

        inc     ebx
        xor     esi, esi
        cmp     [process], 0
        jz      .map
        mov     eax, [process]
        mov     eax, [eax+PROC.pdt_0+ebx*4]
        and     eax, 0xFFFFF000
        jz      .map

        mov     esi, [pagedir]
        stdcall map_page, esi, eax, PG_SWR
        jmp     .map
.done:
        mov     ecx, shared_locked_mutex
        call    mutex_unlock
        cmp     [process], 0
        jz      .exit
        mov     ecx, [process]
        add     ecx, PROC.heap_lock
        call    mutex_unlock
.exit:
        mov     eax, [pagedir]
        mov     ecx, eax
        shr     ecx, 12
        mov     dword [page_tabs+ecx*4], 0
        invlpg  [eax]
        ret
endp

; in: esi+edx*4 = pointer to page table entry
; in: [slot], [req_access], [ofs] on the stack
; in: edi = linear address to map
; in: address space lock must be held
; out: CF cleared <=> failed
; destroys: only eax
proc lock_and_map_page stdcall, slot:dword, req_access:dword, ofs:dword
    locals
        locked_descr dd ?
    endl

        mov     eax, [esi+edx*4]
        test    eax, PG_READ
        jz      .not_present
        test    eax, PG_SHARED
        jnz     .resolve_shared
; normal case: not shared allocated page, mark as shared and map with requested access
        or      dword [esi+edx*4], PG_SHARED
.map:
        and     eax, not 0xFFF
        stdcall map_page, edi, eax, [req_access]
        stc
.fail:
        ret
.not_present:
; check for alloc-on-demand page
        test    eax, LAZY_ALLOC_PAGE
        jz      .fail
; allocate new page, save it to source page table
        push    ecx
        call    alloc_zero_page
        pop     ecx
        test    eax, eax
        jz      .fail
        or      eax, PG_READ+PG_SHARED
        test    dword [esi+edx*4], LAZY_ALLOC_UNREADABLE
        jnz     @f
        or      eax, PG_USER
@@:
        test    dword [esi+edx*4], LAZY_ALLOC_UNWRITABLE
        jnz     @f
        or      eax, PG_WRITE
@@:
        mov     [esi+edx*4], eax
        jmp     .map
.resolve_shared:
        push    ecx edx eax
        mov     eax, sizeof.SHARED_LOCKED_PAGE
        call    malloc
        mov     [locked_descr], eax
        test    eax, eax
        jz      .fail_pop
        mov     edx, [esp]
        and     edx, not 0xFFF
        mov     [eax+SHARED_LOCKED_PAGE.address], edx
        mov     eax, [slot]
        shl     eax, 8
        mov     eax, [SLOT_BASE+eax+APPDATA.process]
        mov     ecx, [eax+PROC.smap_list]
        add     eax, PROC.smap_list
.find_shared_parent:
        cmp     ecx, eax
        jz      .shared_orphan
        mov     edx, [ofs]
        sub     edx, [ecx+SMAP.base]
        cmp     edx, [ecx+SMAP.size]
        jb      .found_shared_parent
        mov     ecx, [ecx+SMAP.fd]
        jmp     .find_shared_parent
.shared_abandoned:
        call    mutex_unlock
.shared_orphan:
; no copy-on-write for orphans
        test    dword [esp], PG_WRITE
        jnz     @f
        test    [req_access], PG_WRITE
        jnz     .shared_forbidden
@@:
; locking the same normal page for second time:
; the first lock_and_map_page has set PG_SHARED,
; now we must cooperate with that other thread.
        mov     ecx, shared_locked_mutex
        call    mutex_lock
        mov     eax, [locked_descr]
        mov     [eax+SHARED_LOCKED_PAGE.parent], 0
.shared_common:
        mov     edx, [shared_locked_list+SHARED_LOCKED_PAGE.bk]
        mov     [eax+SHARED_LOCKED_PAGE.fd], shared_locked_list
        mov     [eax+SHARED_LOCKED_PAGE.bk], edx
        mov     [edx+SHARED_LOCKED_PAGE.fd], eax
        mov     [shared_locked_list+SHARED_LOCKED_PAGE.bk], edx
        call    mutex_unlock
        pop     eax edx ecx
        jmp     .map
.shared_forbidden_unlock:
        call    mutex_unlock
.shared_forbidden:
        mov     eax, [locked_descr]
        call    free
.fail_pop:
        pop     eax edx ecx
        clc
        ret
.found_shared_parent:
        shr     edx, 12
        mov     eax, [locked_descr]
        mov     [eax+SHARED_LOCKED_PAGE.offs], edx
        cmp     [ecx+SMAP.type], SMAP_TYPE_PE
        jnz     .parent_smap
        push    edx
        mov     ecx, [ecx+SMAP.parent]
        add     ecx, PEDESCR.page_array_lock
        call    mutex_lock
        pop     edx
        mov     eax, [esp]
        xor     eax, [ecx+sizeof.PEDESCR-PEDESCR.page_array_lock+edx*4]
        test    eax, not 0xFFF
        jnz     .shared_abandoned
        test    dword [esp], PG_WRITE
        jnz     @f
        test    [req_access], PG_WRITE
        jnz     .pedescr_try_cow
@@:
        mov     eax, [ecx+sizeof.PEDESCR-PEDESCR.page_array_lock+edx*4]
        inc     eax
        test    eax, 0xFF
        jnz     @f
        dec     eax
@@:
        mov     [ecx+sizeof.PEDESCR-PEDESCR.page_array_lock+edx*4], eax
        push    ecx
        mov     ecx, pe_list_mutex
        call    mutex_lock
        mov     eax, [esp]
        inc     dword [eax+PEDESCR.refcount-PEDESCR.page_array_lock]
        call    mutex_unlock
        pop     ecx
        call    mutex_unlock
        sub     ecx, PEDESCR.page_array_lock
        push    ecx
.shared_common2:
        mov     ecx, shared_locked_mutex
        call    mutex_lock
        mov     eax, [locked_descr]
        pop     [eax+SHARED_LOCKED_PAGE.parent]
        jmp     .shared_common
.pedescr_try_cow:
        mov     eax, [ecx+sizeof.PEDESCR-PEDESCR.page_array_lock+edx*4]
        test	eax, IMAGE_SCN_MEM_WRITE shr 20
        jz	@f
        or      dword [esp], PG_WRITE
@@:
        dec     eax
        test    eax, 0xFF
        jnz     .pedescr_alloc_copy
        mov     dword [ecx+sizeof.PEDESCR-PEDESCR.page_array_lock+edx*4], 0
        call    mutex_unlock
        mov     eax, [locked_descr]
        call    free
        pop     eax edx ecx
        or      eax, PG_SHARED
        mov     [esi+edx*4], eax
        jmp     .map
.pedescr_alloc_copy:
        push    ecx edx
        stdcall kernel_alloc, 0x1000
        pop     edx ecx
        test    eax, eax
        jz      .shared_forbidden_unlock
        dec     dword [ecx+sizeof.PEDESCR-PEDESCR.page_array_lock+edx*4]
        push    ecx esi edi
        mov     esi, edi
        mov     edi, eax
        stdcall map_page, esi, [ecx+sizeof.PEDESCR-PEDESCR.page_array_lock+edx*4], PG_READ
        mov     ecx, 0x1000/4
        rep movsd
        sub     esi, 0x1000
        sub     edi, 0x1000
        mov     eax, edi
        call    get_pg_addr
        and     dword [esp+12], 0xFFF
        or      dword [esp+12], eax
        stdcall map_page, esi, eax, [req_access]
        stdcall free_kernel_space, edi
        pop     edi esi ecx
        call    mutex_unlock
        mov     eax, [locked_descr]
        call    free
        pop     eax edx ecx
        or      eax, PG_SHARED
        mov     [esi+edx*4], eax
        stc
        ret
.parent_smap:
        test    dword [esp], PG_WRITE
        jnz     @f
        test    [req_access], PG_WRITE
        jz      .shared_forbidden
@@:
        push    [ecx+SMAP.parent]
        mov     ecx, shmem_list_mutex
        call    mutex_lock
        mov     eax, [esp]
        inc     dword [esp]
        inc     [eax+SMEM.refcount]
        call    mutex_unlock
        jmp     .shared_common2
endp

; in: esi -> process page table entry or esi < 0x1000 if no page table entry
; in: edi = page number for mapped copy
; in: shared_locked_mutex is held
; destroys eax, ecx, edx
proc unlock_and_unmap_page
        mov     edx, [page_tabs+edi*4]
        and     edx, not 0xFFF
        mov     dword [page_tabs+edi*4], 0
        mov     eax, edi
        shl     eax, 12
        invlpg  [eax]
        mov     eax, [shared_locked_list+SHARED_LOCKED_PAGE.fd]
.check_list:
        cmp     eax, shared_locked_list
        jz      .not_in_list
        cmp     edx, [eax+SHARED_LOCKED_PAGE.address]
        jz      .found_in_list
        mov     eax, [eax+SHARED_LOCKED_PAGE.fd]
        jmp     .check_list
.found_in_list:
        push    esi
        mov     esi, [eax+SHARED_LOCKED_PAGE.parent]
        mov     edx, [eax+SHARED_LOCKED_PAGE.fd]
        mov     ecx, [eax+SHARED_LOCKED_PAGE.bk]
        mov     [edx+SHARED_LOCKED_PAGE.bk], ecx
        mov     [ecx+SHARED_LOCKED_PAGE.fd], edx
        test    esi, esi
        jz      .orphan
        btr     esi, 0
        jc      .parent_smap
        push    eax
        lea     ecx, [esi+PEDESCR.page_array_lock]
        call    mutex_lock
        mov     edx, [esp]
        mov     edx, [edx+SHARED_LOCKED_PAGE.offs]
        mov     eax, [esi+sizeof.PEDESCR+edx*4]
        and     eax, 0xFF
        cmp     eax, 0xFF
        jz      .no_deref
        mov     eax, [esi+sizeof.PEDESCR+edx*4]
        dec     eax
        test    eax, 0xFF
        jnz     @f
        call    free_page
        xor     eax, eax
@@:
        mov     [esi+sizeof.PEDESCR+edx*4], eax
.no_deref:
        lea     ecx, [esi+PEDESCR.page_array_lock]
        call    mutex_unlock
        call    dereference_pe
        pop     eax
        call    free
        pop     esi
        ret
.parent_smap:
        call    free
        call    dereference_smem
        pop     esi
        ret
.orphan:
        call    free
        pop     esi
        ret
.not_in_list:
        cmp     esi, 0x1000
        jb      .just_free
        mov     eax, [esi]
        and     eax, not 0xFFF
        cmp     eax, edx
        jnz     .just_free
        and     dword [esi], not PG_SHARED
        ret
.just_free:
        mov     eax, edx
        call    free_page
        ret
endp

sys_IPC:
;input:
;  ebx=1 - set ipc buffer area
;    ecx=address of buffer
;    edx=size of buffer
;  eax=2 - send message
;    ebx=PID
;    ecx=address of message
;    edx=size of message

        dec     ebx
        jnz     @f

        mov     eax, [current_slot]
        pushf
        cli
        mov     [eax+APPDATA.ipc_start], ecx    ;set fields in extended information area
        mov     [eax+APPDATA.ipc_size], edx

        add     edx, ecx
        add     edx, 4095
        and     edx, not 4095

.touch:
        mov     eax, [ecx]
        add     ecx, 0x1000
        cmp     ecx, edx
        jb      .touch

        popf
        mov     [esp+32], ebx   ;ebx=0
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;2
@@:
        dec     ebx
        jnz     @f

        stdcall sys_ipc_send, ecx, edx, esi
        mov     [esp+32], eax
        ret
@@:
        or      eax, -1
        mov     [esp+32], eax
        ret

proc sys_ipc_send stdcall, PID:dword, msg_addr:dword, msg_size:dword
           locals
             dst_slot   dd ?
             dst_offset dd ?
             dst_ptr    dd ?
             buf_size   dd ?
             used_buf   dd ?
             mapped_size dd ?
             result     dd ?
           endl

        mov     ecx, ipc_mutex
        call    mutex_lock

        mov     eax, [PID]
        call    pid_to_slot
        test    eax, eax
        jz      .no_pid

        mov     [dst_slot], eax
        shl     eax, 8
        mov     edi, [eax+SLOT_BASE+APPDATA.ipc_start] ;is ipc area defined?
        test    edi, edi
        jz      .no_ipc_area
        mov     [dst_ptr], edi

        mov     ebx, edi
        and     ebx, 0xFFF
        mov     [dst_offset], ebx

        mov     esi, [eax+SLOT_BASE+APPDATA.ipc_size]
        mov     [buf_size], esi

        mov     ecx, [ipc_tmp]
        cmp     esi, 0x40000-0x1000; size of [ipc_tmp] minus one page
        jbe     @f
        push    esi edi
        add     esi, 0x1000
        stdcall alloc_kernel_space, esi
        mov     ecx, eax
        pop     edi esi
@@:
        mov     [used_buf], ecx
        stdcall map_memEx, ecx, [dst_slot], \
                edi, esi, PG_SWR, [ipc_ptab]
        mov     [mapped_size], eax
        mov     [result], 3 ; buffer overflow
        sub     eax, [dst_offset]
        jc      .no_copy_data
        cmp     [buf_size], eax
        jb      @f
        mov     [buf_size], eax
@@:
        cmp     [buf_size], 8
        jb      .no_copy_data

        mov     [result], 2 ; ipc blocked
        mov     edi, [dst_offset]
        add     edi, [used_buf]
        cmp     dword [edi], 0
        jnz     .no_copy_data         ;if dword [buffer]<>0 - ipc blocked now

        mov     [result], 3 ; buffer overflow
        mov     edx, dword [edi+4]
        lea     ebx, [edx+8]
        add     ebx, [msg_size]
        cmp     ebx, [buf_size]
        ja      .no_copy_data          ;esi<0 - not enough memory in buffer

        mov     [result], 0
        mov     dword [edi+4], ebx
        mov     eax, [TASK_BASE]
        mov     eax, [eax+TASKDATA.pid]        ;eax - our PID
        add     edi, edx
        mov     [edi], eax
        mov     ecx, [msg_size]

        mov     [edi+4], ecx
        add     edi, 8
        mov     esi, [msg_addr]
       ;    add esi, new_app_base
        cld
        rep movsb
.no_copy_data:
        stdcall unmap_memEx, [used_buf], [dst_slot], [dst_ptr], [mapped_size], [ipc_ptab]

        cmp     [result], 0
        jnz     @f
        mov     eax, [dst_slot]
        shl     eax, BSF sizeof.APPDATA
        or      [eax+SLOT_BASE+APPDATA.occurred_events], EVENT_IPC
@@:
        mov     eax, [used_buf]
        cmp     eax, [ipc_tmp]
        je      @f
        stdcall free_kernel_space, eax
@@:
        mov     ecx, ipc_mutex
        call    mutex_unlock
        mov     eax, [result]
        ret
.no_pid:
        mov     ecx, ipc_mutex
        call    mutex_unlock
        mov     eax, 4
        ret
.no_ipc_area:
        mov     ecx, ipc_mutex
        call    mutex_unlock
        xor     eax, eax
        inc     eax
        ret
endp

align 4
sysfn_meminfo:
        cmp     ecx, OS_BASE
        jae     .fail

        mov     eax, [pg_data.pages_count]
        mov     [ecx], eax
        shl     eax, 12
        mov     [esp+32], eax
        mov     eax, [pg_data.pages_free]
        mov     [ecx+4], eax
        mov     eax, [pg_data.pages_faults]
        mov     [ecx+8], eax
        mov     eax, [heap_size]
        mov     [ecx+12], eax
        mov     eax, [heap_free]
        mov     [ecx+16], eax
        mov     eax, [heap_blocks]
        mov     [ecx+20], eax
        mov     eax, [free_blocks]
        mov     [ecx+24], eax
        ret
.fail:
        or      dword [esp+32], -1
        ret

align 4
f68:
        cmp     ebx, 4
        jbe     sys_sheduler
        cmp     ebx, 11
        jb      undefined_syscall
        cmp     ebx, 32
        ja      undefined_syscall
        xor     eax, eax
        jmp     dword [f68call+ebx*4-11*4]
.11:
        call    init_heap
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.12:
        stdcall user_alloc, ecx
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.13:
        stdcall user_free, ecx
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.14:
        cmp     ecx, OS_BASE
        jae     .fail
        mov     edi, ecx
        call    get_event_ex
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.16:
        test    ecx, ecx
        jz      .fail
        cmp     ecx, OS_BASE
        jae     .fail
        stdcall get_service, ecx
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.17:
        call    srv_handlerEx   ;ecx
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.18:
        mov     eax, edx
.19:
        cmp     ecx, OS_BASE
        jae     .fail
        stdcall load_library, ecx, eax
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.20:
        mov     eax, edx
        mov     ebx, ecx
        call    user_realloc            ;in: eax = pointer, ebx = new size
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.21:
        cmp     ecx, OS_BASE
        jae     .fail
        cmp     edx, OS_BASE
        jae     .fail
        stdcall load_pe_driver, ecx, edx
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.22:
        cmp     ecx, OS_BASE
        jae     .fail
        stdcall shmem_open, ecx, edx, esi
        mov     [esp+SYSCALL_STACK._edx], edx
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.23:
        cmp     ecx, OS_BASE
        jae     .fail
        stdcall shmem_close, ecx
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.24:
        mov     eax, [current_slot]
        xchg    ecx, [eax+APPDATA.exc_handler]
        xchg    edx, [eax+APPDATA.except_mask]
        mov     [esp+SYSCALL_STACK._ebx], edx
        mov     [esp+SYSCALL_STACK._eax], ecx
        ret
.25:
        cmp     ecx, 32
        jae     .fail
        mov     eax, [current_slot]
        btr     [eax+APPDATA.except_mask], ecx
        setc    byte[esp+SYSCALL_STACK._eax]
        jecxz   @f
        bts     [eax+APPDATA.except_mask], ecx
@@:
        ret
.26:
        stdcall user_unmap, ecx, edx, esi
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.27:
        cmp     ecx, OS_BASE
        jae     .fail
        stdcall load_file_umode, ecx
        mov     [esp+SYSCALL_STACK._edx], edx
        mov     [esp+SYSCALL_STACK._eax], eax
        ret
.28:
        cmp     ecx, OS_BASE
        jae     .fail
        push    ecx edx
        stdcall kernel_alloc, maxPathLength
        mov     edi, eax
        pop     eax esi
        push    edi
        call    getFullPath
        pop     ebp
        test    eax, eax
        jz      @f
        stdcall load_file_umode, ebp
        mov     [esp+SYSCALL_STACK._edx], edx
@@:
        mov     [esp+SYSCALL_STACK._eax], eax
        stdcall kernel_free, ebp
        ret

.29:
        stdcall user_ring, ecx
        mov     [esp+SYSCALL_STACK._eax], eax
	ret
.30:
        cmp     ecx, OS_BASE
        jae     .fail
        stdcall load_file_maybe_pe, ecx
        test    esi, esi
        jz      .30.not_pe
        stdcall map_pe_usermode, esi, eax, ebx
        mov     [esp+32], eax
        ret
.30.resolve_fail:
        movi    eax, -5
        jmp     .30.error
.30.not_pe:
        cmp     eax, -0x1000
        ja      .30.error
        stdcall kernel_free, eax
        movi    eax, -31
.30.error:
        mov     [esp+32], eax
        ret

.31:
        stdcall unmap_pe_usermode, ecx
        mov     [esp+32], eax
        ret

.32:
        stdcall mprotect, edx, esi, ecx
        mov     [esp+32], eax
        ret

.fail:
        mov     [esp+SYSCALL_STACK._eax], eax
        ret

align 4
f68call:   ; keep this table closer to main code

           dd f68.11   ; init_heap
           dd f68.12   ; user_alloc
           dd f68.13   ; user_free
           dd f68.14   ; get_event_ex
           dd f68.fail ; moved to f68.24
           dd f68.16   ; get_service
           dd f68.17   ; call_service
           dd f68.18   ; loadLibUnicode
           dd f68.19   ; load_dll
           dd f68.20   ; user_realloc
           dd f68.21   ; load_driver
           dd f68.22   ; shmem_open
           dd f68.23   ; shmem_close
           dd f68.24   ; set exception handler
           dd f68.25   ; unmask exception
           dd f68.26   ; user_unmap
           dd f68.27   ; load_file_umode
           dd f68.28   ; loadFileUnicode
           dd f68.29   ; user_ring
           dd f68.30   ; map_pe_usermode
           dd f68.31   ; unmap_pe_usermode
           dd f68.32   ; mprotect

align 4
proc load_pe_driver stdcall, file:dword, cmdline:dword
        push    esi

        stdcall load_PE, [file]
        test    eax, eax
        jz      .fail

        mov     esi, eax
        push    [cmdline]
        push    DRV_ENTRY
        call    eax
        pop     ecx
        pop     ecx
        test    eax, eax
        jz      .fail

        mov     [eax+SRV.entry], esi
        pop     esi
        ret

.fail:
        xor     eax, eax
        pop     esi
        ret
endp

align 4
proc create_ring_buffer stdcall, size:dword, flags:dword
           locals
             buf_ptr  dd ?
           endl

        mov     eax, [size]
        test    eax, eax
        jz      .fail

        add     eax, eax
        stdcall alloc_kernel_space, eax
        test    eax, eax
        jz      .fail

        push    ebx

        mov     [buf_ptr], eax

        mov     ebx, [size]
        shr     ebx, 12
        push    ebx

        stdcall alloc_pages, ebx
        pop     ecx

        test    eax, eax
        jz      .mm_fail

        push    edi

        or      eax, [flags]
        mov     edi, [buf_ptr]
        mov     ebx, [buf_ptr]
        mov     edx, ecx
        shl     edx, 2
        shr     edi, 10
@@:
        mov     [page_tabs+edi], eax
        mov     [page_tabs+edi+edx], eax
        invlpg  [ebx]
        invlpg  [ebx+0x10000]
        add     eax, 0x1000
        add     ebx, 0x1000
        add     edi, 4
        dec     ecx
        jnz     @B

        mov     eax, [buf_ptr]
        pop     edi
        pop     ebx
        ret
.mm_fail:
        stdcall free_kernel_space, [buf_ptr]
        xor     eax, eax
        pop     ebx
.fail:
        ret
endp


align 4
proc print_mem
        mov     edi, BOOT.memmap_blocks
        mov     ecx, [edi-4]
        test    ecx, ecx
        jz      .done

@@:
        mov     eax, [edi]
        mov     edx, [edi+4]
        add     eax, [edi+8]
        adc     edx, [edi+12]

        DEBUGF  1, "K : E820 %x%x - %x%x type %d\n", \
                    [edi+4], [edi],\
                    edx, eax, [edi+16]
        add     edi, 20
        dec     ecx
        jnz     @b
.done:
        ret
endp
