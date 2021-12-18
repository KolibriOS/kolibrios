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

        mov     eax, [pf_err_code]

        cmp     ebx, OS_BASE      ;ebx == .err_addr
        jb      .user_space       ;page in application memory

        cmp     ebx, page_tabs
        jb      .kernel_space     ;page in kernel memory

        cmp     ebx, kernel_tabs
        jb      .alloc;.app_tabs  ;page tables of application ;
                                  ;simply create one
.core_tabs:
.fail:  ;simply return to caller
        mov     esp, ebp
        pop     ebx               ;restore exception number (#PF)
        ret

.user_space:
        test    eax, PG_READ
        jnz     .err_access       ;Page presents
                                  ;Access error ?

        shr     ebx, 12
        mov     ecx, ebx
        shr     ecx, 10
        mov     edx, [master_tab+ecx*4]
        test    edx, PG_READ
        jz      .fail             ;page table is not created
                                  ;incorrect address in program

        mov     eax, [page_tabs+ebx*4]
        test    eax, 2
        jz      .fail             ;address is not reserved for usage. Error
                                  
.alloc:
        call    alloc_page
        test    eax, eax
        jz      .fail

        stdcall map_page, [.err_addr], eax, PG_UWR

        mov     edi, [.err_addr]
        and     edi, 0xFFFFF000
        mov     ecx, 1024
        xor     eax, eax
       ;cld     ;caller is duty for this
        rep stosd
.exit:  ;iret with repeat fault instruction
        add     esp, 12;clear in stack: locals(.err_addr) + #PF + ret_to_caller
        restore_ring3_context
        iretd

.err_access:
; access denied? this may be a result of copy-on-write protection for DLL
; check list of HDLLs
        and     ebx, not 0xFFF
        mov     eax, [current_process]
        mov     eax, [eax+PROC.dlls_list_ptr]
        test    eax, eax
        jz      .fail
        mov     esi, [eax+HDLL.fd]
.scan_hdll:
        cmp     esi, eax
        jz      .fail
        mov     edx, ebx
        sub     edx, [esi+HDLL.base]
        cmp     edx, [esi+HDLL.size]
        jb      .fault_in_hdll
.scan_hdll.next:
        mov     esi, [esi+HDLL.fd]
        jmp     .scan_hdll
.fault_in_hdll:
; allocate new page, map it as rw and copy data
        call    alloc_page
        test    eax, eax
        jz      .fail
        stdcall map_page, ebx, eax, PG_UWR
        mov     edi, ebx
        mov     ecx, 1024
        sub     ebx, [esi+HDLL.base]
        mov     esi, [esi+HDLL.parent]
        mov     esi, [esi+DLLDESCR.data]
        add     esi, ebx
        rep movsd
        jmp     .exit

.kernel_space:
        test    eax, PG_READ
        jz      .fail   ;page does not present

        test    eax, 12 ;U/S (+below)
        jnz     .fail   ;application requested kernel memory
                        
       ;test    eax, 8
       ;jnz     .fail   ;the reserved bit is set in page tables. Added in P4/Xeon
                        

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
        stdcall map_page, [.err_addr], eax, dword PG_SWR
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

; returns number of mapped bytes
proc map_mem_ipc stdcall, lin_addr:dword,slot:dword,\
                      ofs:dword,buf_size:dword,req_access:dword
        locals
             count   dd ?
             process dd ?
        endl

        mov     [count], 0
        cmp     [buf_size], 0
        jz      .exit

        mov     eax, [slot]
        shl     eax, 8
        mov     eax, [SLOT_BASE+eax+APPDATA.process]
        test    eax, eax
        jz      .exit

        mov     [process], eax
        mov     ebx, [ofs]
        shr     ebx, 22
        mov     eax, [eax+PROC.pdt_0+ebx*4]                 ;get page table
        mov     esi, [ipc_ptab]
        and     eax, 0xFFFFF000
        jz      .exit
        stdcall map_page, esi, eax, PG_SWR
@@:
        mov     edi, [lin_addr]
        and     edi, 0xFFFFF000
        mov     ecx, [buf_size]
        add     ecx, 4095
        shr     ecx, 12
        inc     ecx                  ; ???????????

        mov     edx, [ofs]
        shr     edx, 12
        and     edx, 0x3FF
.map:
        stdcall safe_map_page, [slot], [req_access], [ofs]
        jnc     .exit
        add     [count], PAGE_SIZE
        add     [ofs], PAGE_SIZE
        dec     ecx
        jz      .exit

        add     edi, PAGE_SIZE
        inc     edx
        cmp     edx, 1024
        jnz     .map

        inc     ebx
        mov     eax, [process]
        mov     eax, [eax+PROC.pdt_0+ebx*4]
        and     eax, 0xFFFFF000
        jz      .exit

        stdcall map_page, esi, eax, PG_SWR
        xor     edx, edx
        jmp     .map
.exit:
        mov     eax, [count]
        ret
endp

proc map_memEx stdcall, lin_addr:dword,slot:dword,\
                        ofs:dword,buf_size:dword,req_access:dword
        locals
             count   dd ?
             process dd ?
        endl

        mov     [count], 0
        cmp     [buf_size], 0
        jz      .exit

        mov     eax, [slot]
        shl     eax, 8
        mov     eax, [SLOT_BASE+eax+APPDATA.process]
        test    eax, eax
        jz      .exit

        mov     [process], eax
        mov     ebx, [ofs]
        shr     ebx, 22
        mov     eax, [eax+PROC.pdt_0+ebx*4]                 ;get page table
        mov     esi, [proc_mem_tab]
        and     eax, 0xFFFFF000
        jz      .exit
        stdcall map_page, esi, eax, PG_SWR
@@:
        mov     edi, [lin_addr]
        and     edi, 0xFFFFF000
        mov     ecx, [buf_size]
        add     ecx, 4095
        shr     ecx, 12
        inc     ecx                  ; ???????????

        mov     edx, [ofs]
        shr     edx, 12
        and     edx, 0x3FF
.map:
        stdcall safe_map_page, [slot], [req_access], [ofs]
        jnc     .exit
        add     [count], PAGE_SIZE
        add     [ofs], PAGE_SIZE
        dec     ecx
        jz      .exit

        add     edi, PAGE_SIZE
        inc     edx
        cmp     edx, 1024
        jnz     .map

        inc     ebx
        mov     eax, [process]
        mov     eax, [eax+PROC.pdt_0+ebx*4]
        and     eax, 0xFFFFF000
        jz      .exit

        stdcall map_page, esi, eax, PG_SWR
        xor     edx, edx
        jmp     .map
.exit:
        mov     eax, [count]
        ret
endp

; in: esi+edx*4 = pointer to page table entry
; in: [slot], [req_access], [ofs] on the stack
; in: edi = linear address to map
; out: CF cleared <=> failed
; destroys: only eax
proc safe_map_page stdcall, slot:dword, req_access:dword, ofs:dword
        mov     eax, [esi+edx*4]
        test    al, PG_READ
        jz      .not_present
        test    al, PG_WRITE
        jz      .resolve_readonly
; normal case: writable page, just map with requested access
.map:
        stdcall map_page, edi, eax, [req_access]
        stc
.fail:
        ret
.not_present:
; check for alloc-on-demand page
        test    al, 2
        jz      .fail
; allocate new page, save it to source page table
        push    ecx
        call    alloc_page
        pop     ecx
        test    eax, eax
        jz      .fail
        or      al, PG_UWR
        mov     [esi+edx*4], eax
        jmp     .map
.resolve_readonly:
; readonly page, probably copy-on-write
; check: readonly request of readonly page is ok
        test    [req_access], PG_WRITE
        jz      .map
; find control structure for this page
        pushf
        cli
        cld
        push    ebx ecx
        mov     eax, [slot]
        shl     eax, 8
        mov     eax, [SLOT_BASE+eax+APPDATA.process]
        mov     eax, [eax+PROC.dlls_list_ptr]
        test    eax, eax
        jz      .no_hdll
        mov     ecx, [eax+HDLL.fd]
.scan_hdll:
        cmp     ecx, eax
        jz      .no_hdll
        mov     ebx, [ofs]
        and     ebx, not 0xFFF
        sub     ebx, [ecx+HDLL.base]
        cmp     ebx, [ecx+HDLL.size]
        jb      .hdll_found
        mov     ecx, [ecx+HDLL.fd]
        jmp     .scan_hdll
.no_hdll:
        pop     ecx ebx
        popf
        clc
        ret
.hdll_found:
; allocate page, save it in page table, map it, copy contents from base
        mov     eax, [ecx+HDLL.parent]
        add     ebx, [eax+DLLDESCR.data]
        call    alloc_page
        test    eax, eax
        jz      .no_hdll
        or      al, PG_UWR
        mov     [esi+edx*4], eax
        stdcall map_page, edi, eax, [req_access]
        push    esi edi
        mov     esi, ebx
        mov     ecx, 4096/4
        rep movsd
        pop     edi esi
        pop     ecx ebx
        popf
        stc
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
             buf_size   dd ?
             used_buf   dd ?
           endl

        pushf
        cli

        mov     eax, [PID]
        call    pid_to_slot
        test    eax, eax
        jz      .no_pid

        mov     [dst_slot], eax
        shl     eax, 8
        mov     edi, [eax+SLOT_BASE+APPDATA.ipc_start] ;is ipc area defined?
        test    edi, edi
        jz      .no_ipc_area

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
        stdcall map_mem_ipc, ecx, [dst_slot], \
                edi, esi, PG_SWR

        mov     edi, [dst_offset]
        add     edi, [used_buf]
        cmp     dword [edi], 0
        jnz     .ipc_blocked          ;if dword [buffer]<>0 - ipc blocked now

        mov     edx, dword [edi+4]
        lea     ebx, [edx+8]
        add     ebx, [msg_size]
        cmp     ebx, [buf_size]
        ja      .buffer_overflow       ;esi<0 - not enough memory in buffer

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

        mov     ebx, [ipc_tmp]
        mov     edx, ebx
        shr     ebx, 12
        xor     eax, eax
        mov     [page_tabs+ebx*4], eax
        invlpg  [edx]

        mov     ebx, [ipc_pdir]
        mov     edx, ebx
        shr     ebx, 12
        xor     eax, eax
        mov     [page_tabs+ebx*4], eax
        invlpg  [edx]

        mov     ebx, [ipc_ptab]
        mov     edx, ebx
        shr     ebx, 12
        xor     eax, eax
        mov     [page_tabs+ebx*4], eax
        invlpg  [edx]

        mov     eax, [dst_slot]
        shl     eax, BSF sizeof.APPDATA
        or      [eax+SLOT_BASE+APPDATA.occurred_events], EVENT_IPC
        push    0
        jmp     .ret
.no_pid:
        popf
        mov     eax, 4
        ret
.no_ipc_area:
        popf
        xor     eax, eax
        inc     eax
        ret
.ipc_blocked:
        push    2
        jmp     .ret
.buffer_overflow:
        push    3
.ret:
        mov     eax, [used_buf]
        cmp     eax, [ipc_tmp]
        je      @f
        stdcall free_kernel_space, eax
@@:
        pop     eax
        popf
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
        cmp     ebx, 30
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

;  unload driver
.30:                         ; ecx = handl driver edx = cmdline
        mov     eax, -1
        cmp     edx, OS_BASE
        jae     .fail
        cmp     ecx, OS_BASE
        jbe     .fail

        mov     eax, [ecx+SRV.entry]
        test    eax, eax
        jz      .fail
        push    ecx    ;save handl

        push    edx         ;cmdline
        push    DRV_EXIT
        call    eax   ;the result is not checked
        lea     esp, [esp+8]
        push    ecx

        mov     ebx, [ecx+SRV.base]
        mov     eax, -2 ;error free RAM
        test    ebx, ebx
        jz      .fail
        stdcall kernel_free, ebx ;del driver

        mov     eax, [ecx+SRV.fd]
        mov     edx, [ecx+SRV.bk]
        mov     [edx+SRV.fd], eax
        mov     [eax+SRV.bk], edx
        stdcall free, ecx
        ;dec     [count_services]

        mov     [esp+SYSCALL_STACK._eax], eax
        ret
;.31:    ;prototype function for get list service
;
;        ret
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
           dd f68.30   ; unload_driver

align 4
proc load_pe_driver stdcall, file:dword, cmdline:dword
        push    esi

        stdcall load_PE, [file]
        test    eax, eax
        jz      .fail

        mov     esi, eax
        push    ebx  ;  base addres driver
        push    [cmdline]
        push    DRV_ENTRY
        call    eax
        pop     ecx
        pop     ecx
        pop     ebx
        test    eax, eax
        jz      .fail_init

        mov     [eax+SRV.base], ebx
        mov     [eax+SRV.entry], esi
        pop     esi
        ret
.fail_init:
        stdcall kernel_free, ebx  ;clear memory driver
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
