;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


align 4
proc alloc_page

        pushfd
        cli
        push    ebx
;//-
        cmp     [pg_data.pages_free], 1
        jle     .out_of_memory
;//-

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
;//-
        dec     [pg_data.pages_free]
        jz      .out_of_memory
;//-
        btr     [ebx], eax
        mov     [page_start], ebx
        sub     ebx, sys_pgmap
        lea     eax, [eax+ebx*8]
        shl     eax, 12
;//-       dec [pg_data.pages_free]
        pop     ebx
        popfd
        ret
;//-
.out_of_memory:
        mov     [pg_data.pages_free], 1
        xor     eax, eax
        pop     ebx
        popfd
        ret
;//-
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
;//-
        mov     ebx, [pg_data.pages_free]
        sub     ebx, 9
        js      .out_of_memory
        shr     ebx, 3
        cmp     eax, ebx
        jg      .out_of_memory
;//-
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
proc map_page stdcall,lin_addr:dword,phis_addr:dword,flags:dword
        push    ebx
        mov     eax, [phis_addr]
        and     eax, not 0xFFF
        or      eax, [flags]
        mov     ebx, [lin_addr]
        shr     ebx, 12
        mov     [page_tabs+ebx*4], eax
        mov     eax, [lin_addr]
        invlpg  [eax]
        pop     ebx
        ret
endp

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
        and     edx, -4096
        or      edx, [flags]
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
        or      eax, PG_UW        ;+PG_NOCACHE
        mov     dword [master_tab+ebx*4], eax
        mov     eax, [lin_addr]
        shr     eax, 10
        add     eax, page_tabs
        invlpg  [eax]
        pop     ebx
        ret
endp

align 4
proc init_LFB
           locals
             pg_count dd ?
           endl

        cmp     dword [LFBAddress], -1
        jne     @f
        mov     [BOOT_VAR+0x901c], byte 2
; max VGA=640*480*4=1228800 bytes
; + 32*640*4=81920 bytes for mouse pointer
        stdcall alloc_pages, ((1228800+81920)/4096)

        push    eax
        call    alloc_page
        stdcall map_page_table, LFB_BASE, eax
        pop     eax
        or      eax, PG_UW
        mov     ebx, LFB_BASE
; max VGA=640*480*4=1228800 bytes
; + 32*640*4=81920 bytes for mouse pointer
        mov     ecx, (1228800+81920)/4096
        call    commit_pages
        mov     [LFBAddress], dword LFB_BASE
        ret
@@:
        test    [SCR_MODE], word 0100000000000000b
        jnz     @f
        mov     [BOOT_VAR+0x901c], byte 2
        ret
@@:
        call    init_mtrr

        mov     edx, LFB_BASE
        mov     esi, [LFBAddress]
        mov     edi, 0x00C00000
        mov     dword [exp_lfb+4], edx

        shr     edi, 12
        mov     [pg_count], edi
        shr     edi, 10

        bt      [cpu_caps], CAPS_PSE
        jnc     .map_page_tables
        or      esi, PG_LARGE+PG_UW
        mov     edx, sys_pgdir+(LFB_BASE shr 20)
@@:
        mov     [edx], esi
        add     edx, 4
        add     esi, 0x00400000
        dec     edi
        jnz     @B

        bt      [cpu_caps], CAPS_PGE
        jnc     @F
        or      dword [sys_pgdir+(LFB_BASE shr 20)], PG_GLOBAL
@@:
        mov     dword [LFBAddress], LFB_BASE
        mov     eax, cr3      ;flush TLB
        mov     cr3, eax
        ret

.map_page_tables:

@@:
        call    alloc_page
        stdcall map_page_table, edx, eax
        add     edx, 0x00400000
        dec     edi
        jnz     @B

        mov     eax, [LFBAddress]
        mov     edi, page_tabs + (LFB_BASE shr 10)
        or      eax, PG_UW
        mov     ecx, [pg_count]
        cld
@@:
        stosd
        add     eax, 0x1000
        dec     ecx
        jnz     @B

        mov     dword [LFBAddress], LFB_BASE
        mov     eax, cr3      ;flush TLB
        mov     cr3, eax

        ret
endp

align 4
proc new_mem_resize stdcall, new_size:dword

        mov     ecx, pg_data.mutex
        call    mutex_lock

        mov     edi, [new_size]
        add     edi, 4095
        and     edi, not 4095
        mov     [new_size], edi

        mov     edx, [current_slot]
        cmp     [edx+APPDATA.heap_base], 0
        jne     .exit

        mov     esi, [edx+APPDATA.mem_size]
        add     esi, 4095
        and     esi, not 4095

        cmp     edi, esi
        jae     .expand

        shr     edi, 12
        shr     esi, 12
@@:
        mov     eax, [app_page_tabs+edi*4]
        test    eax, 1
        jz      .next
        mov     dword [app_page_tabs+edi*4], 2
        mov     ebx, edi
        shl     ebx, 12
        push    eax
        invlpg  [ebx]
        pop     eax
        call    free_page

.next:
        add     edi, 1
        cmp     edi, esi
        jb      @B

.update_size:
        mov     ebx, [new_size]
        call    update_mem_size

        mov     ecx, pg_data.mutex
        call    mutex_unlock

        xor     eax, eax
        ret
.expand:

        push    esi
        push    edi

        add     edi, 0x3FFFFF
        and     edi, not(0x3FFFFF)
        add     esi, 0x3FFFFF
        and     esi, not(0x3FFFFF)

        cmp     esi, edi
        jae     .grow

        xchg    esi, edi

@@:
        call    alloc_page
        test    eax, eax
        jz      .exit_pop

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
;//-
        pop     edi
        push    edi
        mov     esi, [pg_data.pages_free]
        sub     esi, 1
        shr     edi, 12
        cmp     esi, edi
        jle     .out_of_memory
;//-
        pop     edi
        pop     esi
@@:
        call    alloc_page
        test    eax, eax
        jz      .exit
        stdcall map_page, esi, eax, dword PG_UW

        push    edi
        mov     edi, esi
        xor     eax, eax
        mov     ecx, 1024
        cld
        rep stosd
        pop     edi

        add     esi, 0x1000
        cmp     esi, edi
        jb      @B

        jmp     .update_size
;//-
.exit_pop:
.out_of_memory:
;//-
        pop     edi
        pop     esi
.exit:
        mov     ecx, pg_data.mutex
        call    mutex_unlock

        xor     eax, eax
        inc     eax
        ret
endp

update_mem_size:
; in: edx = slot base
;     ebx = new memory size
; destroys eax,ecx,edx

        mov     [APPDATA.mem_size+edx], ebx
;search threads and update
;application memory size infomation
        mov     ecx, [APPDATA.dir_table+edx]
        mov     eax, 2

.search_threads:
;eax = current slot
;ebx = new memory size
;ecx = page directory
        cmp     eax, [TASK_COUNT]
        jg      .search_threads_end
        mov     edx, eax
        shl     edx, 5
        cmp     word [CURRENT_TASK+edx+TASKDATA.state], 9  ;if slot empty?
        jz      .search_threads_next
        shl     edx, 3
        cmp     [SLOT_BASE+edx+APPDATA.dir_table], ecx      ;if it is our thread?
        jnz     .search_threads_next
        mov     [SLOT_BASE+edx+APPDATA.mem_size], ebx      ;update memory size
.search_threads_next:
        inc     eax
        jmp     .search_threads
.search_threads_end:
        ret

; param
;  eax= linear address
;
; retval
;  eax= phisical page address

align 4
get_pg_addr:
        shr     eax, 12
        mov     eax, [page_tabs+eax*4]
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
        jb      .user_space       ;страница в пам€ти приложени€ ;

        cmp     ebx, page_tabs
        jb      .kernel_space     ;страница в пам€ти €дра

        cmp     ebx, kernel_tabs
        jb      .alloc;.app_tabs  ;таблицы страниц приложени€ ;
                                  ;просто создадим одну
if 0 ;пока это просто лишнее
        cmp     ebx, LFB_BASE
        jb      .core_tabs        ;таблицы страниц €дра
                                  ;ќшибка
  .lfb:
                                  ;область LFB
                                  ;ќшибка
        jmp     .fail
end if
.core_tabs:
.fail:  ;simply return to caller
        mov     esp, ebp
        pop     ebx               ;restore exception number (#PF)
        ret

;        xchg bx, bx
;        add     esp,12 ;clear in stack: locals(.err_addr) + #PF + ret_to_caller
;        restore_ring3_context
;        iretd

.user_space:
        test    eax, PG_MAP
        jnz     .err_access       ;—траница присутствует
                                  ;ќшибка доступа ?

        shr     ebx, 12
        mov     ecx, ebx
        shr     ecx, 10
        mov     edx, [master_tab+ecx*4]
        test    edx, PG_MAP
        jz      .fail             ;таблица страниц не создана
                                  ;неверный адрес в программе

        mov     eax, [page_tabs+ebx*4]
        test    eax, 2
        jz      .fail             ;адрес не зарезервирован дл€ ;
                                  ;использовани€. ќшибка
.alloc:
        call    alloc_page
        test    eax, eax
        jz      .fail

        stdcall map_page, [.err_addr], eax, PG_UW

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
        mov     eax, [CURRENT_TASK]
        shl     eax, 8
        mov     eax, [SLOT_BASE+eax+APPDATA.dlls_list_ptr]
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
        stdcall map_page, ebx, eax, PG_UW
        mov     edi, ebx
        mov     ecx, 1024
        sub     ebx, [esi+HDLL.base]
        mov     esi, [esi+HDLL.parent]
        mov     esi, [esi+DLLDESCR.data]
        add     esi, ebx
        rep movsd
        jmp     .exit

.kernel_space:
        test    eax, PG_MAP
        jz      .fail   ;страница не присутствует

        test    eax, 12 ;U/S (+below)
        jnz     .fail   ;приложение обратилось к пам€ти
                        ;€дра
       ;test    eax, 8
       ;jnz     .fail   ;установлен зарезервированный бит
                        ;в таблицах страниц. добавлено в P4/Xeon

;попытка записи в защищЄнную страницу €дра

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
        stdcall map_page, [.err_addr], eax, dword PG_SW
        pop     eax
        mov     edi, [.err_addr]
        and     edi, -4096
        lea     esi, [edi+(not tss._io_map_0)+1]; -tss._io_map_0

        mov     ebx, esi
        shr     ebx, 12
        mov     edx, [current_slot]
        or      eax, PG_SW
        mov     [edx+APPDATA.io_map+ebx*4], eax

        add     esi, [default_io_map]
        mov     ecx, 4096/4
       ;cld     ;caller is duty for this
        rep movsd
        jmp     .exit
endp

; returns number of mapped bytes
proc map_mem stdcall, lin_addr:dword,slot:dword,\
                      ofs:dword,buf_size:dword,req_access:dword
        push    0 ; initialize number of mapped bytes

        cmp     [buf_size], 0
        jz      .exit

        mov     eax, [slot]
        shl     eax, 8
        mov     eax, [SLOT_BASE+eax+APPDATA.dir_table]
        and     eax, 0xFFFFF000

        stdcall map_page, [ipc_pdir], eax, PG_UW
        mov     ebx, [ofs]
        shr     ebx, 22
        mov     esi, [ipc_pdir]
        mov     edi, [ipc_ptab]
        mov     eax, [esi+ebx*4]
        and     eax, 0xFFFFF000
        jz      .exit
        stdcall map_page, edi, eax, PG_UW
;           inc ebx
;           add edi, 0x1000
;           mov eax, [esi+ebx*4]
;           test eax, eax
;           jz @f
;          and eax, 0xFFFFF000
;           stdcall map_page, edi, eax

@@:
        mov     edi, [lin_addr]
        and     edi, 0xFFFFF000
        mov     ecx, [buf_size]
        add     ecx, 4095
        shr     ecx, 12
        inc     ecx

        mov     edx, [ofs]
        shr     edx, 12
        and     edx, 0x3FF
        mov     esi, [ipc_ptab]

.map:
        stdcall safe_map_page, [slot], [req_access], [ofs]
        jnc     .exit
        add     dword [ebp-4], 4096
        add     [ofs], 4096
        dec     ecx
        jz      .exit
        add     edi, 0x1000
        inc     edx
        cmp     edx, 0x400
        jnz     .map
        inc     ebx
        mov     eax, [ipc_pdir]
        mov     eax, [eax+ebx*4]
        and     eax, 0xFFFFF000
        jz      .exit
        stdcall map_page, esi, eax, PG_UW
        xor     edx, edx
        jmp     .map

.exit:
        pop     eax
        ret
endp

proc map_memEx stdcall, lin_addr:dword,slot:dword,\
                        ofs:dword,buf_size:dword,req_access:dword
        push    0 ; initialize number of mapped bytes

        cmp     [buf_size], 0
        jz      .exit

        mov     eax, [slot]
        shl     eax, 8
        mov     eax, [SLOT_BASE+eax+APPDATA.dir_table]
        and     eax, 0xFFFFF000

        stdcall map_page, [proc_mem_pdir], eax, PG_UW
        mov     ebx, [ofs]
        shr     ebx, 22
        mov     esi, [proc_mem_pdir]
        mov     edi, [proc_mem_tab]
        mov     eax, [esi+ebx*4]
        and     eax, 0xFFFFF000
        test    eax, eax
        jz      .exit
        stdcall map_page, edi, eax, PG_UW

@@:
        mov     edi, [lin_addr]
        and     edi, 0xFFFFF000
        mov     ecx, [buf_size]
        add     ecx, 4095
        shr     ecx, 12
        inc     ecx

        mov     edx, [ofs]
        shr     edx, 12
        and     edx, 0x3FF
        mov     esi, [proc_mem_tab]

.map:
        stdcall safe_map_page, [slot], [req_access], [ofs]
        jnc     .exit
        add     dword [ebp-4], 0x1000
        add     edi, 0x1000
        add     [ofs], 0x1000
        inc     edx
        dec     ecx
        jnz     .map
.exit:
        pop     eax
        ret
endp

; in: esi+edx*4 = pointer to page table entry
; in: [slot], [req_access], [ofs] on the stack
; in: edi = linear address to map
; out: CF cleared <=> failed
; destroys: only eax
proc safe_map_page stdcall, slot:dword, req_access:dword, ofs:dword
        mov     eax, [esi+edx*4]
        test    al, PG_MAP
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
        or      al, PG_UW
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
        mov     eax, [SLOT_BASE+eax+APPDATA.dlls_list_ptr]
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
        or      al, PG_UW
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

;align 4
;proc set_ipc_buff

;           mov  eax,[current_slot]
;           pushf
;           cli
;           mov  [eax+APPDATA.ipc_start],ebx     ;set fields in extended information area
;           mov  [eax+APPDATA.ipc_size],ecx
;
;           add ecx, ebx
;           add ecx, 4095
;           and ecx, not 4095
;
;.touch:    mov eax, [ebx]
;           add ebx, 0x1000
;           cmp ebx, ecx
;           jb  .touch
;
;           popf
;           xor eax, eax
;           ret
;endp

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
        mov     edi, [eax+SLOT_BASE+0xa0] ;is ipc area defined?
        test    edi, edi
        jz      .no_ipc_area

        mov     ebx, edi
        and     ebx, 0xFFF
        mov     [dst_offset], ebx

        mov     esi, [eax+SLOT_BASE+0xa4]
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
        stdcall map_mem, ecx, [dst_slot], \
                edi, esi, PG_SW

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
        mov     eax, [eax+0x04]        ;eax - our PID
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
        shl     eax, 8
        or      [eax+SLOT_BASE+0xA8], dword 0x40
        cmp     dword [check_idle_semaphore], 20
        jge     .ipc_no_cis

        mov     dword [check_idle_semaphore], 5
.ipc_no_cis:
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
        jz      @f
        stdcall free_kernel_space, eax
@@:
        pop     eax
        popf
        ret
endp

align 4
sysfn_meminfo:

        ;   add ecx, new_app_base
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
        jb      .fail

        cmp     ebx, 25
        ja      .fail

        jmp     dword [f68call+ebx*4-11*4]
.11:
        call    init_heap
        mov     [esp+32], eax
        ret
.12:
        stdcall user_alloc, ecx
        mov     [esp+32], eax
        ret
.13:
        stdcall user_free, ecx
        mov     [esp+32], eax
        ret
.14:
        cmp     ecx, OS_BASE
        jae     .fail
        mov     edi, ecx
        call    get_event_ex
        mov     [esp+32], eax
        ret
.16:
        test    ecx, ecx
        jz      .fail
        cmp     ecx, OS_BASE
        jae     .fail
        stdcall get_service, ecx
        mov     [esp+32], eax
        ret
.17:
        call    srv_handlerEx   ;ecx
        mov     [esp+32], eax
        ret
.19:
        cmp     ecx, OS_BASE
        jae     .fail
        stdcall load_library, ecx
        mov     [esp+32], eax
        ret
.20:
        mov     eax, edx
        mov     ebx, ecx
        call    user_realloc            ;in: eax = pointer, ebx = new size
        mov     [esp+32], eax
        ret
.21:
        cmp     ecx, OS_BASE
        jae     .fail

        cmp     ebx, OS_BASE
        jae     .fail

        mov     edi, edx
        stdcall load_PE, ecx
        mov     esi, eax
        test    eax, eax
        jz      @F

        push    edi
        push    DRV_ENTRY
        call    eax
        add     esp, 8
        test    eax, eax
        jz      @F

        mov     [eax+SRV.entry], esi

@@:
        mov     [esp+32], eax
        ret
.22:
        cmp     ecx, OS_BASE
        jae     .fail

        stdcall shmem_open, ecx, edx, esi
        mov     [esp+24], edx
        mov     [esp+32], eax
        ret

.23:
        cmp     ecx, OS_BASE
        jae     .fail

        stdcall shmem_close, ecx
        mov     [esp+32], eax
        ret
.24:
        mov     eax, [current_slot]
        xchg    ecx, [eax+APPDATA.exc_handler]
        xchg    edx, [eax+APPDATA.except_mask]
        mov     [esp+32], ecx ; reg_eax+8
        mov     [esp+20], edx ; reg_ebx+8
        ret
.25:
        cmp     ecx, 32
        jae     .fail
        mov     eax, [current_slot]
        btr     [eax+APPDATA.except_mask], ecx
        setc    byte[esp+32]
        jecxz   @f
        bts     [eax+APPDATA.except_mask], ecx
@@:
        ret

.fail:
        xor     eax, eax
        mov     [esp+32], eax
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
           dd f68.fail ; moved to f68.25
           dd f68.19   ; load_dll
           dd f68.20   ; user_realloc
           dd f68.21   ; load_driver
           dd f68.22   ; shmem_open
           dd f68.23   ; shmem_close
           dd f68.24
           dd f68.25


align 4
proc load_pe_driver stdcall, file:dword

        stdcall load_PE, [file]
        test    eax, eax
        jz      .fail

        mov     esi, eax
        stdcall eax, DRV_ENTRY
        test    eax, eax
        jz      .fail

        mov     [eax+SRV.entry], esi
        ret

.fail:
        xor     eax, eax
        ret
endp


align 4
proc init_mtrr

        cmp     [BOOT_VAR+0x901c], byte 2
        je      .exit

        bt      [cpu_caps], CAPS_MTRR
        jnc     .exit

        mov     eax, cr0
        or      eax, 0x60000000 ;disable caching
        mov     cr0, eax
        wbinvd                  ;invalidate cache

        mov     ecx, 0x2FF
        rdmsr                   ;
; has BIOS already initialized MTRRs?
        test    ah, 8
        jnz     .skip_init
; rarely needed, so mainly placeholder
; main memory - cached
        push    eax

        mov     eax, [MEM_AMOUNT]
; round eax up to next power of 2
        dec     eax
        bsr     ecx, eax
        mov     ebx, 2
        shl     ebx, cl
        dec     ebx
; base of memory range = 0, type of memory range = MEM_WB
        xor     edx, edx
        mov     eax, MEM_WB
        mov     ecx, 0x200
        wrmsr
; mask of memory range = 0xFFFFFFFFF - (size - 1), ebx = size - 1
        mov     eax, 0xFFFFFFFF
        mov     edx, 0x0000000F
        sub     eax, ebx
        sbb     edx, 0
        or      eax, 0x800
        inc     ecx
        wrmsr
; clear unused MTRRs
        xor     eax, eax
        xor     edx, edx
@@:
        wrmsr
        inc     ecx
        cmp     ecx, 0x210
        jb      @b
; enable MTRRs
        pop     eax
        or      ah, 8
        and     al, 0xF0; default memtype = UC
        mov     ecx, 0x2FF
        wrmsr
.skip_init:
        stdcall set_mtrr, [LFBAddress], [LFBSize], MEM_WC

        wbinvd                  ;again invalidate

        mov     eax, cr0
        and     eax, not 0x60000000
        mov     cr0, eax        ; enable caching
.exit:
        ret
endp

align 4
proc set_mtrr stdcall, base:dword,size:dword,mem_type:dword
; find unused register
        mov     ecx, 0x201
@@:
        rdmsr
        dec     ecx
        test    ah, 8
        jz      .found
        rdmsr
        mov     al, 0; clear memory type field
        cmp     eax, [base]
        jz      .ret
        add     ecx, 3
        cmp     ecx, 0x210
        jb      @b
; no free registers, ignore the call
.ret:
        ret
.found:
; found, write values
        xor     edx, edx
        mov     eax, [base]
        or      eax, [mem_type]
        wrmsr

        mov     ebx, [size]
        dec     ebx
        mov     eax, 0xFFFFFFFF
        mov     edx, 0x00000000
        sub     eax, ebx
        sbb     edx, 0
        or      eax, 0x800
        inc     ecx
        wrmsr
        ret
endp

align 4
proc stall stdcall, delay:dword
        push    ecx
        push    edx
        push    ebx
        push    eax

        mov     eax, [delay]
        mul     [stall_mcs]
        mov     ebx, eax      ;low
        mov     ecx, edx      ;high
        rdtsc
        add     ebx, eax
        adc     ecx, edx
@@:
        rdtsc
        sub     eax, ebx
        sbb     edx, ecx
        jb      @B

        pop     eax
        pop     ebx
        pop     edx
        pop     ecx
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
