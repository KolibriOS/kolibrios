
tmp_page_tab      equ 0x00C00000

align 4
proc mem_test

           mov eax, cr0
           and eax, not (CR0_CD+CR0_NW)
           or eax, CR0_CD         ;disable caching
           mov cr0, eax
           wbinvd                 ;invalidate cache

           xor edi, edi
           mov ebx, 'TEST'
@@:
           add edi, 0x400000
           xchg ebx, dword [edi]
           cmp dword [edi], 'TEST'
           xchg ebx, dword [edi]
           je @b

           and eax, not (CR0_CD+CR0_NW)  ;enable caching
           mov cr0, eax
           mov eax, edi
           mov [LFBSize], 0x00800000
           ret
endp

align 4
proc init_memEx
           xor eax, eax
           mov edi, sys_pgdir
           mov ecx, 2048
           rep stosd

           bt [cpu_caps], CAPS_PSE
           jnc .no_PSE

           mov ebx, cr4
           or ebx, CR4_PSE
           mov eax, PG_LARGE+PG_SW
           bt [cpu_caps], CAPS_PGE
           jnc @F
           or eax, PG_GLOBAL
           or ebx, CR4_PGE
@@:
           mov cr4, ebx

           mov dword [sys_pgdir], eax
           add eax, 0x00400000
           mov dword [sys_pgdir+4], eax
           add eax, 0x00400000
           mov dword [sys_pgdir+8], eax

           mov dword [sys_pgdir+0x600], sys_pgdir+PG_SW

           mov ecx, [pg_data.kernel_tables]
           sub ecx, 3 ;4
           mov eax, tmp_page_tab+PG_SW
           mov edi, sys_pgdir+12     ;16

           jmp .map_kernel_tabs
.no_PSE:
           mov eax, PG_SW
           mov esi, tmp_page_tab
           mov ecx, 3072/4;  4096/4      ;0x0 - 0x00FFFFFF
.map_low:
           mov [esi], eax
           add eax, 0x1000
           mov [esi+4], eax
           add eax, 0x1000
           mov [esi+8], eax
           add eax, 0x1000
           mov [esi+12], eax
           add eax, 0x1000
           add esi, 16
           dec ecx
           jnz .map_low            ;ядро

           mov ecx, [pg_data.kernel_tables]
           mov eax, tmp_page_tab+PG_SW
           mov edi, sys_pgdir

.map_kernel_tabs:

           mov [edi], eax
           add eax, 0x1000
           add edi, 4
           dec ecx
           jnz .map_kernel_tabs

           mov edi, tmp_page_tab
           bt [cpu_caps], CAPS_PSE
           jc @F
           add edi, 3072*4    ;4096*4  ;skip low kernel memory
@@:
           mov ecx, [pg_data.kernel_tables]
           sub ecx, 3
           shl ecx, 10
           xor eax, eax
           cld
           rep stosd

           mov dword [sys_pgdir+0x600], sys_pgdir+PG_SW

           ret
endp

align 4
proc init_page_map
           mov edi, sys_pgmap
           mov ecx, 384/4
           xor eax,eax
           cld
           rep stosd

           not eax
           mov ecx, [pg_data.pagemap_size]
           sub ecx, 384
           shr ecx, 2
           rep stosd

           mov edi, sys_pgmap+384
           mov edx, [pg_data.pages_count]
           mov ecx, [pg_data.kernel_tables]
           bt [cpu_caps], CAPS_PSE
           jnc @f
           sub ecx, 3
@@:
           sub edx, 3072
           sub edx, ecx
           mov [pg_data.pages_free], edx

           xor eax, eax
           mov ebx, ecx
           shr ecx, 5
           rep stosd

           not eax
           mov ecx, ebx
           and ecx, 31
           shl eax, cl
           stosd

           mov [page_start], sys_pgmap+384
           mov ebx, sys_pgmap
           add ebx, [pg_data.pagemap_size]
           mov [page_end], ebx

           mov [pg_data.pg_mutex], 0

           ret
endp

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
           mov eax, [phis_addr]
           and eax, not 0xFFF
           or eax, [flags]
           mov ebx, [lin_addr]
           shr ebx, 12
           mov [pages_tab+ebx*4], eax
           mov eax, [lin_addr]
           invlpg [eax]
           ret
endp

align 4
proc free_page
;arg:  eax  page address
           pushfd
           cli
           inc [pg_data.pages_free]
           shr eax, 12              ;page index
           mov ebx, sys_pgmap
           bts [ebx], eax           ;that's all!
           shr eax, 3
           and eax, not 3           ;dword offset from page_map
           add eax, ebx
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
;  ecx= size
;  eax= base

align 4
release_pages:

           mov ebx, pg_data.pg_mutex
           call wait_mutex      ;ebx

           shr ecx, 12
           mov esi, eax
           mov edi, eax

           shr esi, 10
           add esi, pages_tab

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
           inc [pg_data.pages_free]
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

           and [pg_data.pg_mutex],0
           ret

align 4
proc map_page_table stdcall, lin_addr:dword, phis_addr:dword
           mov ebx, [lin_addr]
           shr ebx, 22
           mov eax, [phis_addr]
           and eax, not 0xFFF
           or eax, PG_UW          ;+PG_NOCACHE
           mov dword [current_pgdir+ebx*4], eax
           mov eax, [lin_addr]
           shr eax, 10
           add eax, pages_tab
           invlpg [eax]
           ret
endp

align 4
proc init_LFB
           cmp dword [LFBAddress], -1
           jne @f
           mov [0x2f0000+0x901c],byte 2
           stdcall kernel_alloc, 0x280000
           mov [LFBAddress], eax
           ret
@@:
           test [SCR_MODE],word 0100000000000000b
           jnz @f
           mov [0x2f0000+0x901c],byte 2
           ret
@@:
           call map_LFB
           ret
endp

align 4
proc map_LFB
           locals
             pg_count dd ?
           endl

           mov edi, [LFBSize]
           mov esi, [LFBAddress]
           mov dword [exp_lfb+4], esi

           shr edi, 12
           mov [pg_count], edi
           shr edi, 10

           bt [cpu_caps], CAPS_PSE
           jnc .map_page_tables
           mov ebx, esi
           or esi, PG_LARGE+PG_UW
           shr ebx, 20
           mov ecx, ebx
@@:
           mov [sys_pgdir+ebx], esi
           add ebx, 4
           add esi, 0x00400000
           dec edi
           jnz @B

           bt [cpu_caps], CAPS_PGE
           jnc @F
           or dword [sys_pgdir+ecx], PG_GLOBAL
@@:
           mov eax, cr3       ;flush TLB
           mov cr3, eax
           ret

.map_page_tables:

@@:
           call alloc_page
           stdcall map_page_table, esi, eax
           add esi, 0x00400000
           dec edi
           jnz @B

           mov eax, [LFBAddress]
           mov esi, eax
           shr esi, 10
           add esi, pages_tab
           or eax, PG_UW
           mov ecx, [pg_count]
           shr ecx, 2
.map:
           mov [esi], eax
           add eax, 0x1000
           mov [esi+4], eax
           add eax, 0x1000
           mov [esi+8], eax
           add eax, 0x1000
           mov [esi+12], eax
           add eax, 0x1000
           add esi, 16
           sub ecx, 1
           jnz .map

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

           mov edx,[CURRENT_TASK]
           shl edx,8
           cmp [PROC_BASE+APPDATA.heap_base+edx],0
           jne .exit

           mov esi, [PROC_BASE+APPDATA.mem_size+edx]
           add esi, 4095
           and esi, not 4095

           cmp edi, esi
           jae .expand

           shr edi, 12
           shr esi, 12
@@:
           mov eax, [pages_tab+0x00181000+edi*4]
           test eax, 1
           jz .next
           mov dword [pages_tab+0x00181000+edi*4], 2
           mov ebx, edi
           shl ebx, 12
           invlpg [ebx+std_application_base_address]
           call free_page

.next:     add edi, 1
           cmp edi, esi
           jb @B

.update_size:

           mov ebx, [new_size]
           mov    [PROC_BASE+0x8c+edx],ebx

;search threads and update
;application memory size infomation
           mov    ecx,[PROC_BASE+0xb8+edx]
           mov    eax,2

.search_threads:
;eax = current slot
;ebx = new memory size
;ecx = page directory
           cmp    eax,[TASK_COUNT]
           jg     .search_threads_end
           mov    edx,eax
           shl    edx,5
           cmp    word [CURRENT_TASK+edx+0xa],9 ;if slot empty?
           jz     .search_threads_next
           shl    edx,3
           cmp    [PROC_BASE+edx+0xb8],ecx     ;if it is our thread?
           jnz    .search_threads_next
           mov    [PROC_BASE+edx+0x8c],ebx     ;update memory size
.search_threads_next:
           inc    eax
           jmp    .search_threads
.search_threads_end:
           xor eax, eax
           dec [pg_data.pg_mutex]
           ret

.expand:
           add edi, new_app_base
           add esi, new_app_base

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
           add edi, pages_tab
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
           jna @B

           jmp .update_size
.exit:
           xor eax, eax
           inc eax
           dec [pg_data.pg_mutex]
           ret
endp

align 4
proc get_pg_addr stdcall, lin_addr:dword
           mov ebx, [lin_addr]
           shr ebx, 12
           mov eax, [pages_tab+ebx*4]
           and eax, 0xFFFFF000
           ret
endp

align 4
proc page_fault_handler
           pushad

           mov ebp, esp
           mov eax, cr2
           push eax
           push ds
           push es

           mov ax, 0x10
           mov ds, ax
           mov es, ax

           inc [pg_data.pages_faults]

           mov ebx, [ebp-4]

           cmp ebx, 0xe0000000
           jae .lfb_addr

           cmp ebx, 0x60400000
           jae .user_space

           cmp ebx, master_tab+0x1000
           jae .alloc

           cmp ebx, 0x60000000
           jae .tab_space

           jmp .kernel_space

.user_space:
           shr ebx, 12
           mov ecx, ebx
           shr ecx, 10
           mov edx, [master_tab+ecx*4]
           test edx, 1
           jz .fail

           mov eax, [pages_tab+ebx*4]
           test eax, 2
           jz .fail
.alloc:
           call alloc_page
           and eax, eax
           jz .exit

           stdcall map_page,[ebp-4],eax,dword PG_UW

           mov edi, [ebp-4]
           and edi, 0xFFFFF000
           mov ecx, 1024
           xor eax, eax
           cld
           rep stosd
.exit:
           pop es
           pop ds
           mov esp, ebp
           popad
           add esp, 4
           iretd
.fail:
           pop es
           pop ds
           mov esp, ebp
           popad
           add esp, 4

           save_ring3_context     ;debugger support

           mov bl, 14
           jmp exc_c
           iretd

.kernel_space:
;           shr ebx, 12
;           mov eax, [pages_tab+ebx*4]
;           shr ebx, 10
;           mov eax, [master_tab+ebx*4]
           jmp .exit
.old_addr:
;           shr ebx, 12
;           mov eax, [pages_tab+ebx*4]
;           shr ebx, 10
;           mov eax, [master_tab+ebx*4]
           jmp .exit
.lfb_addr:
;           shr ebx, 22
;           ;mov ecx, [sys_page_dir]
;           mov eax, [master_tab+ebx*4]
           jmp .exit
.tab_space:
;           shr ebx, 12
;           mov eax, [pages_tab+ebx*4]
;           shr ebx, 10
;           ;mov ecx, [sys_page_dir]
;           mov eax, [master_tab+ebx*4]
           jmp .exit
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

           mov  eax,[CURRENT_TASK]
           shl  eax,8
           add  eax, PROC_BASE
           pushf
           cli
           mov  [eax+0xA0],ebx     ;set fields in extended information area
           mov  [eax+0xA4],ecx

           add ebx,  new_app_base
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
           mov  edi,[eax+PROC_BASE+0xa0]  ;is ipc area defined?
           test edi,edi
           jz   .no_ipc_area

           mov ebx, edi
           add edi, new_app_base
           and ebx, 0xFFF
           mov [dst_offset], ebx

           mov esi, [eax+PROC_BASE+0xa4]
           mov [buf_size], esi

           stdcall map_mem, [ipc_tmp], [PROC_BASE+eax+0xB8],\
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
           add esi, new_app_base
           cld
           rep movsb

           mov ebx, [ipc_tmp]
           mov edx, ebx
           shr ebx, 12
           xor eax, eax
           mov [pages_tab+ebx*4], eax
           invlpg [edx]

           mov ebx, [ipc_pdir]
           mov edx, ebx
           shr ebx, 12
           xor eax, eax
           mov [pages_tab+ebx*4], eax
           invlpg [edx]

           mov ebx, [ipc_ptab]
           mov edx, ebx
           shr ebx, 12
           xor eax, eax
           mov [pages_tab+ebx*4], eax
           invlpg [edx]

           mov  eax, [dst_slot]
           shl eax, 8
           or   [eax+PROC_BASE+0xA8],dword 0x40
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

           add ebx, new_app_base
           cmp ebx, new_app_base
           jb .fail

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
           add ebx, new_app_base
           stdcall user_free, ebx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 14
           ja @f
           add ebx, new_app_base
           cmp ebx, new_app_base
           jb .fail
           stdcall get_event_ex, ebx, ecx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 15
           ja @f
           mov ecx, [CURRENT_TASK]
           shl ecx, 8
           mov eax, [ecx+PROC_BASE+APPDATA.fpu_handler]
           mov [ecx+PROC_BASE+APPDATA.fpu_handler], ebx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 16
           ja @f

           test ebx, ebx
           jz .fail
           add ebx, new_app_base
           cmp ebx, new_app_base
           jb .fail
           stdcall get_service, ebx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 17
           ja @f
           stdcall srv_handlerEx, ebx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 18
           ja @f
           mov ecx, [CURRENT_TASK]
           shl ecx, 8
           mov eax, [ecx+PROC_BASE+APPDATA.sse_handler]
           mov [ecx+PROC_BASE+APPDATA.sse_handler], ebx
           mov [esp+36], eax
           ret
@@:
           cmp eax, 19
           ja .fail
           add ebx, new_app_base
           cmp ebx, new_app_base
           jb .fail
           stdcall load_library, ebx
           mov [esp+36], eax
           ret

.fail:
           xor eax, eax
           mov [esp+36], eax
           ret

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
proc test_cpu
           locals
              cpu_type   dd ?
              cpu_id     dd ?
              cpu_Intel  dd ?
              cpu_AMD    dd ?
           endl

           mov [cpu_type], 0
           xor eax, eax
           mov [cpu_caps], eax
           mov [cpu_caps+4], eax

           pushfd
           pop eax
           mov ecx, eax
           xor eax, 0x40000
           push eax
           popfd
           pushfd
           pop eax
           xor eax, ecx
           mov [cpu_type], CPU_386
           jz .end_cpuid
           push ecx
           popfd

           mov [cpu_type], CPU_486
           mov eax, ecx
           xor eax, 0x200000
           push eax
           popfd
           pushfd
           pop eax
           xor eax, ecx
           je .end_cpuid
           mov [cpu_id], 1

           xor eax, eax
           cpuid
           mov [cpu_vendor], ebx
           mov [cpu_vendor+4], edx
           mov [cpu_vendor+8], ecx
           cmp ebx, dword [intel_str]
           jne .check_AMD
           cmp edx, dword [intel_str+4]
           jne .check_AMD
           cmp ecx, dword [intel_str+8]
           jne .check_AMD
           mov [cpu_Intel], 1
           cmp eax, 1
           jl .end_cpuid
           mov eax, 1
           cpuid
           mov [cpu_sign], eax
           mov [cpu_info],  ebx
           mov [cpu_caps],  edx
           mov [cpu_caps+4],ecx

           shr eax, 8
           and eax, 0x0f
           ret
.end_cpuid:
           mov eax, [cpu_type]
           ret

.check_AMD:
           cmp ebx, dword [AMD_str]
           jne .unknown
           cmp edx, dword [AMD_str+4]
           jne .unknown
           cmp ecx, dword [AMD_str+8]
           jne .unknown
           mov [cpu_AMD], 1
           cmp eax, 1
           jl .unknown
           mov eax, 1
           cpuid
           mov [cpu_sign], eax
           mov [cpu_info],  ebx
           mov [cpu_caps],  edx
           mov [cpu_caps+4],ecx
           shr eax, 8
           and eax, 0x0f
           ret
.unknown:
           mov eax, 1
           cpuid
           mov [cpu_sign], eax
           mov [cpu_info],  ebx
           mov [cpu_caps],  edx
           mov [cpu_caps+4],ecx
           shr eax, 8
           and eax, 0x0f
           ret
endp

MEM_WB     equ 6               ;write-back memory
MEM_WC     equ 1               ;write combined memory
MEM_UC     equ 0               ;uncached memory

align 4
proc init_mtrr

           cmp [0x2f0000+0x901c],byte 2
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

iglobal
align 4
  intel_str    db "GenuineIntel",0
  AMD_str      db "AuthenticAMD",0
endg

uglobal
align 16
  irq_tab           rd 16


  MEM_FreeSpace     rd 1

  ipc_tmp           rd 1
  ipc_pdir          rd 1
  ipc_ptab          rd 1

  proc_mem_map      rd 1
  proc_mem_pdir     rd 1
  proc_mem_tab      rd 1

  tmp_task_pdir     rd 1
  tmp_task_ptab     rd 1
  tmp_task_data     rd 1

  fpu_data          rd 1
  fdd_buff          rd 1
  LFBSize           rd 1

  stall_mcs         rd 1
;;CPUID information

  cpu_vendor        rd 3
  cpu_sign          rd 1
  cpu_info          rd 1

;;;;;   cursors data

align 16
cur_saved_data   rb 4096

;cursors          rb CURSOR_SIZE*64
;cursor_map       rd 2
;cursor_start     rd 1
;cursor_end       rd 1

def_cursor       rd 1
hw_cursor        rd 1

scr_width        rd 1
scr_height       rd 1

cur_def_interl   rd 1
cur_saved_base   rd 1
cur_saved_interl rd 1
cur_saved_w      rd 1
cur_saved_h      rd 1

endg

uglobal
align 16

   mst MEM_STATE

;   dll_tab         rb 32*32
;   srv_tab         rb 36*32
   mem_block_map   rb 512
   event_map       rb 128
   mem_block_list  rd 64
   mem_block_mask  rd 2

;   dll_map         rd 1
;   srv_map         rd 1

   srv.fd          rd 1
   srv.bk          rd 1

   mem_used_list   rd 1
   mem_block_arr   rd 1
   mem_block_start rd 1
   mem_block_end   rd 1

   heap_mutex      rd 1
   heap_size       rd 1
   heap_free       rd 1
   heap_blocks     rd 1
   free_blocks     rd 1

   page_start      rd 1
   page_end        rd 1
   events          rd 1
   event_start     rd 1
   event_end       rd 1

   sys_page_map    rd 1
endg


;     push eax
;     push edx
;     mov edx, 0x400   ;bocsh
;     mov al,0xff      ;bocsh
;     out dx, al       ;bocsh
;     pop edx
;     pop eax


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
