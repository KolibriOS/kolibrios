$Revision: 448 $
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc alloc_page

           pushfd
           cli
           mov ebx, [page_start]
           mov ecx, [page_end]
.l1:
           bsf eax,[ebx];
           jnz .found
           add ebx,4
           cmp ebx, ecx
           jb .l1
           popfd
           xor eax,eax
           ret
.found:
           btr [ebx], eax
           mov [page_start],ebx
           sub ebx, sys_pgmap
           lea eax, [eax+ebx*8]
           shl eax, 12
           dec [pg_data.pages_free]
           popfd
           ret
endp

align 4
proc alloc_pages stdcall, count:dword
           pushfd
           cli
           mov eax, [count]
           add eax, 7
           shr eax, 3
           mov [count], eax
           cmp eax, [pg_data.pages_free]
           ja .fail

           mov ecx, [page_start]
           mov ebx, [page_end]
.find:
           mov edx, [count]
           mov edi, ecx
.match:
           cmp byte [ecx], 0xFF
           jne .next
           dec edx
           jz .ok
           inc ecx
           cmp ecx,ebx
           jb .match
.fail:     xor eax, eax
           popfd
           ret
.next:
           inc ecx
           cmp ecx, ebx
           jb .find
           popfd
           xor eax, eax
           ret
.ok:
           sub ecx, edi
           inc ecx
           mov esi, edi
           xor eax, eax
           rep stosb
           sub esi, sys_pgmap
           shl esi, 3+12
           mov eax, esi
           mov ebx, [count]
           shl ebx, 3
           sub [pg_data.pages_free], ebx
           popfd
           ret
endp

align 4
proc map_page stdcall,lin_addr:dword,phis_addr:dword,flags:dword
           push ebx
           mov eax, [phis_addr]
           and eax, not 0xFFF
           or eax, [flags]
           mov ebx, [lin_addr]
           shr ebx, 12
           mov [page_tabs+ebx*4], eax
           mov eax, [lin_addr]
           invlpg [eax]
           pop ebx
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
           shr eax, 12              ;page index
           bts dword [sys_pgmap], eax           ;that's all!
           cmc
           adc [pg_data.pages_free], 0
           shr eax, 3
           and eax, not 3           ;dword offset from page_map
           add eax, sys_pgmap
           cmp [page_start], eax
           ja @f
           popfd
           ret
@@:
           mov [page_start], eax
           popfd
           ret
endp

; param
;  eax= page base + page flags
;  ebx= liear address
;  ecx= count

align 4
commit_pages:

           test ecx, ecx
           jz .fail

           mov edi, ebx
           mov ebx, pg_data.pg_mutex
           call wait_mutex      ;ebx

           mov edx, 0x1000
           mov ebx, edi
           shr ebx, 12
@@:
           mov [page_tabs+ebx*4], eax
           invlpg [edi]
           add edi, edx
           add eax, edx
           inc ebx
           dec ecx
           jnz @B
           mov [pg_data.pg_mutex],ecx
.fail:
           ret


; param
;  eax= base
;  ecx= count

align 4
release_pages:

           pushad
           mov ebx, pg_data.pg_mutex
           call wait_mutex      ;ebx

           mov esi, eax
           mov edi, eax

           shr esi, 10
           add esi, page_tabs

           mov ebp, [pg_data.pages_free]
           mov ebx, [page_start]
           mov edx, sys_pgmap
@@:
           xor eax, eax
           xchg eax, [esi]
           invlpg [edi]

           test eax, 1
           jz .next

           shr eax, 12
           bts [edx], eax
           cmc
           adc ebp, 0
           shr eax, 3
           and eax, -4
           add eax, edx
           cmp eax, ebx
           jae .next

           mov ebx, eax
.next:
           add edi, 0x1000
           add esi, 4
           dec ecx
           jnz @B
           mov [pg_data.pages_free], ebp
           and [pg_data.pg_mutex],0
           popad
           ret

align 4
proc map_page_table stdcall, lin_addr:dword, phis_addr:dword
           push ebx
           mov ebx, [lin_addr]
           shr ebx, 22
           mov eax, [phis_addr]
           and eax, not 0xFFF
           or eax, PG_UW          ;+PG_NOCACHE
           mov dword [master_tab+ebx*4], eax
           mov eax, [lin_addr]
           shr eax, 10
           add eax, page_tabs
           invlpg [eax]
           pop ebx
           ret
endp

align 4
proc init_LFB
           locals
             pg_count dd ?
           endl

           cmp dword [LFBAddress], -1
           jne @f
           mov [BOOT_VAR+0x901c],byte 2
           stdcall kernel_alloc, 0x280000
           mov [LFBAddress], eax
           ret
@@:
           test [SCR_MODE],word 0100000000000000b
           jnz @f
           mov [BOOT_VAR+0x901c],byte 2
           ret
@@:
           mov edx, LFB_BASE
           mov esi, [LFBAddress]
           mov edi, 0x00800000
           mov dword [exp_lfb+4], edx

           shr edi, 12
           mov [pg_count], edi
           shr edi, 10

           bt [cpu_caps], CAPS_PSE
           jnc .map_page_tables
           or esi, PG_LARGE+PG_UW
           mov edx, sys_pgdir+(LFB_BASE shr 20)
@@:
           mov [edx], esi
           add edx, 4
           add esi, 0x00400000
           dec edi
           jnz @B

           bt [cpu_caps], CAPS_PGE
           jnc @F
           or dword [sys_pgdir+(LFB_BASE shr 20)], PG_GLOBAL
@@:
           mov dword [LFBAddress], LFB_BASE
           mov eax, cr3       ;flush TLB
           mov cr3, eax
           ret

.map_page_tables:

@@:
           call alloc_page
           stdcall map_page_table, edx, eax
           add edx, 0x00400000
           dec edi
           jnz @B

           mov eax, [LFBAddress]
           mov edi, page_tabs + (LFB_BASE shr 10)
           or eax, PG_UW
           mov ecx, [pg_count]
           cld
@@:
           stosd
           add eax, 0x1000
           dec ecx
           jnz @B

           mov dword [LFBAddress], LFB_BASE
           mov eax, cr3       ;flush TLB
           mov cr3, eax

           ret
endp

align 4
proc new_mem_resize stdcall, new_size:dword

           mov ebx, pg_data.pg_mutex
           call wait_mutex    ;ebx

           mov edi, [new_size]
           add edi,4095
           and edi,not 4095
           mov [new_size], edi

           mov edx,[current_slot]
           cmp [edx+APPDATA.heap_base],0
           jne .exit

           mov esi, [edx+APPDATA.mem_size]
           add esi, 4095
           and esi, not 4095

           cmp edi, esi
           jae .expand

           shr edi, 12
           shr esi, 12
@@:
           mov eax, [app_page_tabs+edi*4]
           test eax, 1
           jz .next
           mov dword [app_page_tabs+edi*4], 2
           mov ebx, edi
           shl ebx, 12
           invlpg [ebx+std_application_base_address]
           call free_page

.next:     add edi, 1
           cmp edi, esi
           jb @B

.update_size:
           mov     ebx, [new_size]
           call    update_mem_size

           xor eax, eax
           dec [pg_data.pg_mutex]
           ret
.expand:

           push esi
           push edi

           add edi, 0x3FFFFF
           and edi, not(0x3FFFFF)
           add esi, 0x3FFFFF
           and esi, not(0x3FFFFF)

           cmp esi, edi
           jae .grow

           xchg esi, edi

@@:
           call alloc_page
           test eax, eax
           jz .exit

           stdcall map_page_table, edi, eax

           push edi
           shr edi, 10
           add edi, page_tabs
           mov ecx, 1024
           xor eax, eax
           cld
           rep stosd
           pop edi

           add edi, 0x00400000
           cmp edi, esi
           jb @B
.grow:
           pop edi
           pop esi
@@:
           call alloc_page
           test eax, eax
           jz .exit
           stdcall map_page,esi,eax,dword PG_UW

           push edi
           mov edi, esi
           xor eax, eax
           mov ecx, 1024
           cld
           rep stosd
           pop edi

           add esi, 0x1000
           cmp esi, edi
           jb  @B

           jmp .update_size
.exit:
           xor eax, eax
           inc eax
           dec [pg_data.pg_mutex]
           ret
endp

update_mem_size:
; in: edx = slot base
;     ebx = new memory size
; destroys eax,ecx,edx

           mov    [APPDATA.mem_size+edx],ebx
;search threads and update
;application memory size infomation
           mov    ecx,[APPDATA.dir_table+edx]
           mov    eax,2

.search_threads:
;eax = current slot
;ebx = new memory size
;ecx = page directory
           cmp    eax,[TASK_COUNT]
           jg     .search_threads_end
           mov    edx,eax
           shl    edx,5
           cmp    word [CURRENT_TASK+edx+TASKDATA.state],9 ;if slot empty?
           jz     .search_threads_next
           shl    edx,3
           cmp    [SLOT_BASE+edx+APPDATA.dir_table],ecx     ;if it is our thread?
           jnz    .search_threads_next
           mov    [SLOT_BASE+edx+APPDATA.mem_size],ebx     ;update memory size
.search_threads_next:
           inc    eax
           jmp    .search_threads
.search_threads_end:
           ret

; param
;  eax= linear address
;
; retval
;  eax= phisical page address

align 4
get_pg_addr:
           shr eax, 12
           mov eax, [page_tabs+eax*4]
           and eax, 0xFFFFF000
           ret


align 4
proc page_fault_handler

        .err_code equ ebp+32
        .err_addr equ ebp-4

           pushad
           mov ebp, esp
           mov eax, cr2
           push eax

           mov ax, app_data
           mov ds, ax
           mov es, ax

           inc [pg_data.pages_faults]

     push eax
     push edx
     mov edx, 0x400   ;bochs
     mov al,0xff      ;bochs
     out dx, al       ;bochs
     pop edx
     pop eax

           mov ebx, [.err_addr]
           mov eax, [.err_code]

           cmp ebx, OS_BASE
           jb .user_space      ;страница в памяти приложения ;

           cmp ebx, page_tabs
           jb .kernel_space    ;страница в памяти ядра

           cmp ebx, kernel_tabs
           jb .alloc;.app_tabs ;таблицы страниц приложения ;
                               ;просто создадим одну

           cmp ebx, LFB_BASE
           jb .core_tabs       ;таблицы страниц ядра
                               ;Ошибка
.lfb:
                               ;область LFB
                               ;Ошибка
           jmp .fail

align 4
.user_space:
           test eax, PG_MAP
           jnz .err_access     ;Страница присутствует
                               ;Ошибка доступа ?

           shr ebx, 12
           mov ecx, ebx
           shr ecx, 10
           mov edx, [master_tab+ecx*4]
           test edx, PG_MAP
           jz .fail            ;таблица страниц не создана
                               ;неверный адрес в программе

           mov eax, [page_tabs+ebx*4]
           test eax, 2
           jz .fail            ;адрес не зарезервирован для ;
                               ;использования. Ошибка
.alloc:
           call alloc_page
           and eax, eax
           jz .fail

           stdcall map_page,[ebp-4],eax,dword PG_UW

           mov edi, [ebp-4]
           and edi, 0xFFFFF000
           mov ecx, 1024
           xor eax, eax
           cld
           rep stosd
.exit:
           mov esp, ebp
           popad
           add esp, 4
           iretd

.err_access:
;никогда не происходит
           jmp .fail

.kernel_space:
           test eax, PG_MAP
           jz .fail        ;страница не присутствует

           test eax, 4     ;U/S
           jnz .fail       ;приложение обратилось к памяти
                           ;ядра
           test eax, 8
           jnz .fail       ;установлен зарезервированный бит
                           ;в таблицах страниц. добавлено в P4/Xeon

;попытка записи в защищённую страницу ядра

           cmp ebx, tss._io_map_0
           jb .fail

           cmp ebx, tss._io_map_0+8192
           jae .fail

; io permission map
; copy-on-write protection

           call alloc_page
           and eax, eax
           jz .fail

           push eax
           stdcall map_page,[ebp-4],eax,dword PG_SW
           pop eax
           mov edi, [.err_addr]
           and edi, -4096
           lea esi, [edi+(not tss._io_map_0)+1]; -tss._io_map_0

           mov ebx, esi
           shr ebx, 12
           mov edx, [current_slot]
           or eax, PG_SW
           mov [edx+APPDATA.io_map+ebx*4], eax

           add esi, [default_io_map]
           mov ecx, 4096/4
           cld
           rep movsd
           jmp .exit


;не обрабатываем. Ошибка

.core_tabs:
.fail:
           mov esp, ebp
           popad
           add esp, 4
           iretd

           save_ring3_context     ;debugger support

           mov bl, 14
           jmp exc_c
           iretd
endp

align 4
proc map_mem stdcall, lin_addr:dword,pdir:dword,\
                      ofs:dword,buf_size:dword
           mov eax, [buf_size]
           test eax, eax
           jz .exit

           mov eax, [pdir]
           and eax, 0xFFFFF000

           stdcall map_page,[ipc_pdir],eax,dword PG_UW
           mov ebx, [ofs]
           shr ebx, 22
           mov esi, [ipc_pdir]
           mov edi, [ipc_ptab]
           mov eax, [esi+ebx*4]
           and eax, 0xFFFFF000
           test eax, eax
           jz .exit
           stdcall map_page,edi,eax,dword PG_UW
;           inc ebx
;           add edi, 0x1000
;           mov eax, [esi+ebx*4]
;           test eax, eax
;           jz @f
;          and eax, 0xFFFFF000
;           stdcall map_page, edi, eax

@@:        mov edi, [lin_addr]
           and edi, 0xFFFFF000
           mov ecx, [buf_size]
           add ecx, 4095
           shr ecx, 12
           inc ecx

           mov edx, [ofs]
           shr edx, 12
           and edx, 0x3FF
           mov esi, [ipc_ptab]

.map:      mov eax, [esi+edx*4]
           and eax, 0xFFFFF000
           test eax, eax
           jz .exit
           stdcall map_page,edi,eax,dword PG_UW
           add edi, 0x1000
           inc edx
           dec ecx
           jnz .map

.exit:
           ret
endp

align 4
proc map_memEx stdcall, lin_addr:dword,pdir:dword,\
                        ofs:dword,buf_size:dword
           mov eax, [buf_size]
           test eax, eax
           jz .exit

           mov eax, [pdir]
           and eax, 0xFFFFF000

           stdcall map_page,[proc_mem_pdir],eax,dword PG_UW
           mov ebx, [ofs]
           shr ebx, 22
           mov esi, [proc_mem_pdir]
           mov edi, [proc_mem_tab]
           mov eax, [esi+ebx*4]
           and eax, 0xFFFFF000
           test eax, eax
           jz .exit
           stdcall map_page,edi,eax,dword PG_UW

@@:        mov edi, [lin_addr]
           and edi, 0xFFFFF000
           mov ecx, [buf_size]
           add ecx, 4095
           shr ecx, 12
           inc ecx

           mov edx, [ofs]
           shr edx, 12
           and edx, 0x3FF
           mov esi, [proc_mem_tab]

.map:      mov eax, [esi+edx*4]
;           and eax, 0xFFFFF000
;           test eax, eax
;           jz .exit
           stdcall map_page,edi,eax,dword PG_UW
           add edi, 0x1000
           inc edx
           dec ecx
           jnz .map
.exit:
           ret
endp




sys_IPC:
;input:
;  eax=1 - set ipc buffer area
;    ebx=address of buffer
;    ecx=size of buffer
;  eax=2 - send message
;    ebx=PID
;    ecx=address of message
;    edx=size of message

           cmp  eax,1
           jne @f
           call set_ipc_buff
           mov [esp+36], eax
           ret
@@:
           cmp eax, 2
           jne @f
           stdcall sys_ipc_send, ebx, ecx, edx
           mov [esp+36], eax
           ret
@@:
           xor eax, eax
           not eax
           mov [esp+36], eax
           ret

align 4
proc set_ipc_buff

           mov  eax,[current_slot]
           pushf
           cli
           mov  [eax+APPDATA.ipc_start],ebx     ;set fields in extended information area
           mov  [eax+APPDATA.ipc_size],ecx

           add ecx, ebx
           add ecx, 4095
           and ecx, not 4095

.touch:    mov eax, [ebx]
           add ebx, 0x1000
           cmp ebx, ecx
           jna .touch

           popf
           xor eax, eax
           ret
endp

proc sys_ipc_send stdcall, PID:dword, msg_addr:dword, msg_size:dword
           locals
             dst_slot   dd ?
             dst_offset dd ?
             buf_size   dd ?
           endl

           pushf
           cli

           mov  eax, [PID]
           call pid_to_slot
           test eax,eax
           jz   .no_pid

           mov [dst_slot], eax
           shl  eax,8
           mov  edi,[eax+SLOT_BASE+0xa0]  ;is ipc area defined?
           test edi,edi
           jz   .no_ipc_area

           mov ebx, edi
       ;    add edi, new_app_base
           and ebx, 0xFFF
           mov [dst_offset], ebx

           mov esi, [eax+SLOT_BASE+0xa4]
           mov [buf_size], esi

           stdcall map_mem, [ipc_tmp], [SLOT_BASE+eax+0xB8],\
                             edi, esi

           mov edi, [dst_offset]
           add edi, [ipc_tmp]
           cmp dword [edi], 0
           jnz  .ipc_blocked          ;if dword [buffer]<>0 - ipc blocked now

           mov ebx, dword [edi+4]
           mov edx, ebx
           add ebx, 8
           add ebx, [msg_size]
           cmp ebx, [buf_size]
           ja .buffer_overflow         ;esi<0 - not enough memory in buffer

           mov dword [edi+4], ebx
           mov eax,[TASK_BASE]
           mov eax, [eax+0x04]         ;eax - our PID
           mov edi, [dst_offset]
           add edi, [ipc_tmp]
           add edi, edx
           mov [edi], eax
           mov ecx, [msg_size]

           mov [edi+4], ecx
           add edi, 8
           mov esi, [msg_addr]
       ;    add esi, new_app_base
           cld
           rep movsb

           mov ebx, [ipc_tmp]
           mov edx, ebx
           shr ebx, 12
           xor eax, eax
           mov [page_tabs+ebx*4], eax
           invlpg [edx]

           mov ebx, [ipc_pdir]
           mov edx, ebx
           shr ebx, 12
           xor eax, eax
           mov [page_tabs+ebx*4], eax
           invlpg [edx]

           mov ebx, [ipc_ptab]
           mov edx, ebx
           shr ebx, 12
           xor eax, eax
           mov [page_tabs+ebx*4], eax
           invlpg [edx]

           mov  eax, [dst_slot]
           shl eax, 8
           or   [eax+SLOT_BASE+0xA8],dword 0x40
           cmp  dword [check_idle_semaphore],20
           jge  .ipc_no_cis

           mov  dword [check_idle_semaphore],5
.ipc_no_cis:
           popf
           xor eax, eax
           ret
.no_pid:
           popf
           mov  eax, 4
           ret
.no_ipc_area:
           popf
           xor eax, eax
           inc eax
           ret
.ipc_blocked:
           popf
           mov  eax, 2
           ret
.buffer_overflow:
           popf
           mov  eax, 3
           ret
endp

align 4
sysfn_meminfo:

        ;   add ebx, new_app_base
           cmp ebx, OS_BASE
           jae .fail

           mov eax, [pg_data.pages_count]
           mov [ebx], eax
           shl eax, 12
           mov [esp+36], eax
           mov ecx, [pg_data.pages_free]
           mov [ebx+4], ecx
           mov edx, [pg_data.pages_faults]
           mov [ebx+8], edx
           mov esi, [heap_size]
           mov [ebx+12], esi
           mov edi, [heap_free]
           mov [ebx+16], edi
           mov eax, [heap_blocks]
           mov [ebx+20], eax
           mov ecx, [free_blocks]
           mov [ebx+24], ecx
           ret
.fail:
           mov dword [esp+36], -1
           ret

align 4
new_services:

           cmp  eax,4
           jle  sys_sheduler

           cmp eax, 11
           jb .fail
           ja @f

           call init_heap
           mov [esp+36], eax
           ret
@@:
           cmp eax, 12
           ja @f

           stdcall user_alloc, ebx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 13
           ja @f
           stdcall user_free, ebx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 14
           ja @f
           cmp ebx, OS_BASE
           jae .fail
           stdcall get_event_ex, ebx, ecx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 15
           ja @f
           mov ecx, [current_slot]
           mov eax, [ecx+APPDATA.fpu_handler]
           mov [ecx+APPDATA.fpu_handler], ebx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 16
           ja @f

           test ebx, ebx
           jz .fail
           cmp ebx, OS_BASE
           jae .fail
           stdcall get_service, ebx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 17
           ja @f
           call srv_handlerEx   ;ebx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 18
           ja @f
           mov ecx, [current_slot]
           mov eax, [ecx+APPDATA.sse_handler]
           mov [ecx+APPDATA.sse_handler], ebx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 19
           ja @f
           cmp ebx, OS_BASE
           jae .fail
           stdcall load_library, ebx
           mov [esp+36], eax
           ret

@@:
        cmp     eax, 20
        ja      .fail
        mov     eax, ecx
        call    user_realloc
        mov     [esp+36], eax
        ret

.fail:
           xor eax, eax
           mov [esp+36], eax
           ret

align 4
proc init_mtrr

           cmp [BOOT_VAR+0x901c],byte 2
           je  .exit

           bt [cpu_caps], CAPS_MTRR
           jnc .exit

           mov eax, cr0
           or eax, 0x60000000   ;disable caching
           mov cr0, eax
           wbinvd               ;invalidate cache

           mov ecx, 0x2FF
           rdmsr                ;
           push eax

           xor edx, edx
           xor eax, eax
           mov ecx, 0x2FF
           wrmsr                ;disable all MTRR

           stdcall set_mtrr, dword 0,dword 0,[MEM_AMOUNT],MEM_WB
           stdcall set_mtrr, dword 1,[LFBAddress],[LFBSize],MEM_WC
           xor edx, edx
           xor eax, eax
           mov ecx, 0x204
           mov ebx, 6
@@:
           wrmsr                ;disable unused MTRR
           inc ecx
           wrmsr
           inc ecx
           dec ebx
           jnz @b

           wbinvd               ;again invalidate

           pop eax
           or eax, 0x800        ;set default memtype to UC
           and al, 0xF0
           mov ecx, 0x2FF
           wrmsr                ;and enable MTRR

           mov eax, cr0
           and eax, not 0x60000000
           mov cr0, eax         ; enable caching
.exit:
           ret
endp

align 4
proc set_mtrr stdcall, reg:dword,base:dword,size:dword,mem_type:dword

           xor edx, edx
           mov eax, [base]
           or eax, [mem_type]
           mov ecx, [reg]
           lea ecx, [0x200+ecx*2]
           wrmsr

           mov ebx, [size]
           dec ebx
           mov eax, 0xFFFFFFFF
           mov edx, 0x0000000F
           sub eax, ebx
           sbb edx, 0
           or eax, 0x800
           inc ecx
           wrmsr
           ret
endp


align 4
proc strncmp stdcall, str1:dword, str2:dword, count:dword

          mov ecx,[count]
          jecxz .end

          mov ebx,ecx

          mov edi,[str1]
          mov esi,edi
          xor eax,eax
          repne scasb
          neg ecx             ; cx = count - strlen
          add ecx,ebx         ; strlen + count - strlen

.okay:
          mov edi,esi
          mov esi,[str2]
          repe cmpsb
          mov al,[esi-1]
          xor ecx,ecx

          cmp al,[edi-1]
          ja .str2_big
          je .end

.str1_big:
          sub ecx,2

.str2_big:
          not ecx
.end:
          mov eax,ecx
          ret
endp

align 4
proc stall stdcall, delay:dword
           push ecx
           push edx
           push ebx
           push eax

           mov eax, [delay]
           mul [stall_mcs]
           mov ebx, eax       ;low
           mov ecx, edx       ;high
           rdtsc
           add ebx, eax
           adc ecx,edx
@@:
           rdtsc
           sub eax, ebx
           sbb edx, ecx
           jb @B

           pop eax
           pop ebx
           pop edx
           pop ecx
           ret
endp

align 4
k_strrchr:
        push eax
        xor eax,eax
        or  ecx,-1
        repne scasb
        add ecx,1
        neg ecx
        sub edi,1
        pop eax
        std
        repne scasb
        cld
        add edi,1

        cmp [edi],al
        jne @F
        mov eax,edi
        ret
@@:
        xor eax,eax
        ret

align 4
proc k_strncpy stdcall, dest:dword, src:dword, maxlen:dword
        mov eax, [dest]
        mov esi, [src]
        mov ecx, [maxlen]
        test eax, eax
        jz .L9
        test esi, esi
        jz .L9
        test ecx, ecx
        jz .L9

        sub  esi, eax
        jmp .L1

align 4
.L2:
        mov edx, [esi+eax]
        mov [eax], dl
        test dl, dl
        jz .L7

        mov [eax+1], dh
        test dh, dh
        jz .L6

        shr edx, 16
        mov [eax+2],dl
        test dl, dl
        jz .L5

        mov [eax+3], dh
        test dh, dh
        jz .L4
        add eax, 4
.L1:
        sub ecx, 4
        jae .L2

        add ecx, 4
        jz .L9

        mov dl, [eax+esi]
        mov [eax], dl
        test dl, dl
        jz .L3

        inc eax
        dec ecx
        jz .L9

        mov dl, [eax+esi]
        mov [eax], dl
        test dl, dl
        jz .L3

        inc eax
        dec ecx
        jz .L9

        mov dl, [eax+esi]
        mov [eax], dl
        test dl, dl
        jz .L3

        inc eax
        jmp .L9

.L4:    dec ecx
        inc eax

.L5:    dec ecx
        inc eax

.L6:    dec ecx
        inc eax
.L7:
        add ecx,3
        jz .L9
.L8:
        mov byte [ecx+eax], 0
.L3:
        dec ecx
        jnz .L8
.L9:
	ret
endp

if 0

magic equ 0xfefefeff

k_strlen:
        mov eax,[esp+4]
        mov edx, 3

        and edx, eax
        jz .L1
        jp .L0

        cmp dh, byte [eax]
        je .L2

        inc eax
        cmp dh, byte [eax]

        je .L2

        inc eax
        xor edx, 2

        jz .L1
.L0:
        cmp dh, [eax]
        je .L2

        inc eax
        xor edx, edx

.L1:
        mov ecx, [eax]
        add eax, 4

        sub edx, ecx
        add ecx, magic

        dec edx
        jnc .L3

        xor edx, ecx
        and edx, not magic
        jne .L3

        mov ecx, [eax]
        add eax, 4

        sub edx, ecx
        add ecx, magic
        dec edx
        jnc .L3

        xor edx, ecx
        and edx, not magic
        jne .L3

        mov ecx, [eax]
        add eax, 4

        sub edx, ecx
        add ecx, magic

        dec edx
        jnc .L3

        xor edx, ecx

        and edx, not magic
        jne .L3

        mov ecx, [eax]
        add eax, 4

        sub edx, ecx
        add ecx, magic

        dec edx
        jnc .L3

        xor edx, ecx

        and edx, not magic
        je .L1

.L3:    sub eax ,4
        sub ecx, magic

        cmp cl, 0
        jz .L2

        inc eax
        test ch, ch
        jz .L2

        shr ecx, 16
        inc eax

        cmp cl,0
        jz .L2

        inc eax

.L2:
        sub eax, [esp+4]
	ret

end if

if 0
     push eax
     push edx
     mov edx, 0x400   ;bochs
     mov al,0xff      ;bochs
     out dx, al       ;bochs
     pop edx
     pop eax
end if


