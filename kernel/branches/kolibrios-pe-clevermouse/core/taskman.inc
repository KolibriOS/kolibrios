;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


GREEDY_KERNEL  = 0

struct  APP_HEADER_00_
        banner          dq ?
        version         dd ?    ;+8
        start           dd ?    ;+12
        i_end           dd ?    ;+16
        mem_size        dd ?    ;+20
        i_param         dd ?    ;+24
ends

struct  APP_HEADER_01_
        banner          dq ?
        version         dd ?    ;+8
        start           dd ?    ;+12
        i_end           dd ?    ;+16
        mem_size        dd ?    ;+20
        stack_top       dd ?    ;+24
        i_param         dd ?    ;+28
        i_icon          dd ?    ;+32
ends

struct  APP_HDR
        cmdline         rd 1    ;0x00
        path            rd 1    ;0x04
        eip             rd 1    ;0x08
        esp             rd 1    ;0x0C
        _edata          rd 1    ;0x10
        _emem           rd 1    ;0x14
        img_base        rd 1    ;0x18
        img_size        rd 1
        filename_size   rd 1
        cmdline_size    rd 1
        path_string     rd 1
        pedescr         rd 1
ends

; Pointer to this structure is passed as the third argument
; to usermode PE loader by the kernel.
struct kernel_init_data
version         dw      ?
flags           dw      ?
syscall_method  dd      ?
; either one of SYSCALL_METHOD_xxx or pointer to procedure
exe_base        dd      ?
stack_base      dd      ?
stack_size      dd      ?
exe_path        dd      ?
command_line    dd      ?
environment     dd      ?
ends
SYSCALL_METHOD_I40 = 1
SYSCALL_METHOD_SYSENTER = 2
SYSCALL_METHOD_SYSCALL = 3

macro _clear_ op
{
        mov     ecx, op/4
        xor     eax, eax
        cld
        rep stosd
}

align 4
_strnlen:
        mov     edx, ecx
        xor     eax, eax
        repne scasb
        jne     @F
        inc     ecx
@@:
        mov     eax, edx
        sub     eax, ecx
        retn

fs_execute_from_sysdir:
        xor     ebx, ebx
fs_execute_from_sysdir_param:
        stdcall kernel_alloc, maxPathLength
        push    eax ebx
        mov     esi, ebp
        mov     edi, eax
        xor     eax, eax
        call    getFullPath
        pop     ecx ebx
        xor     edx, edx
; @brief Executes a program
; @param edx Flags
; @param ecx Commandline
; @param ebx Absolute file path
; @param eax String length
; @returns Negated error code or new process number
proc fs_execute
    locals
        cmdline         rd  1
        flags           rd  1
        slot            rd  1  ; index of new thread slot
        slot_base       rd  1  ; base address of it
; app header data
        hdr_cmdline     rd  1
        hdr_path        rd  1
        hdr_eip         rd  1
        hdr_esp         rd  1
        hdr_edata       rd  1
        hdr_emem        rd  1
        file_base       rd  1
        file_size       rd  1
        filename_size   rd  1
        cmdline_size    rd  1
        path_string     rd  1
        pedescr         rd  1
    endl

        mov     [flags], edx
        mov     [cmdline], ecx
        mov     [path_string], ebx
        mov     [filename_size], eax
        mov     edi, -ERROR_FILE_NOT_FOUND
        test    eax, eax
        jz      .err_file
        stdcall load_file, ebx
        test    eax, eax
        jz      .err_file
        stdcall load_file_maybe_pe, [path_string]
        mov     [pedescr], esi
        mov     [file_base], eax
        mov     [file_size], ebx

        test    esi, esi
        jnz     .file_ok

        mov     edi, eax
        cmp     eax, -0x1000
        ja      .err_file

        lea     ebx, [hdr_cmdline]
        call    test_app_header  ; fill our app header data locals with values from header of given program (if its correct)
        mov     edi, -TASKMAN_ERROR_NOT_A_EXECUTABLE
        test    eax, eax
        jz      .err_hdr

.file_ok:
        call    lock_application_table
        call    alloc_thread_slot   ; create a slot for new thread
        mov     edi, -TASKMAN_ERROR_TOO_MANY_PROCESSES
        test    eax, eax
        jz      .err_0

        mov     [slot], eax
        shl     eax, 8
        lea     edi, [SLOT_BASE+eax]
        mov     [slot_base], edi
; clean extended information about process
        mov     ecx, sizeof.APPDATA/4
        xor     eax, eax
        cld
        rep stosd
; write application name ( APPDATA.appname )
        stdcall strrchr, [path_string], '/'
        lea     esi, [eax+1]    ; -> name without path
        mov     ecx, 11
        mov     edi, [slot_base]
@@:
        call    utf8to16
        call    uni2ansi_char
        cmp     al, '.'
        jz      @f
        test    al, al
        jz      @f
        stosb
        loop    @b
@@:
        mov     edi, [cmdline]
        xor     eax, eax
        test    edi, edi
        jz      @f
        mov     ecx, 65535
        call    _strnlen
        cmp     eax, 256
        jb      @f
; if cmdline length >= 256 then increase needed memory size by this length
        lea     ebx, [eax+1]
        add     [hdr_emem], ebx
@@:
        mov     [cmdline_size], eax
        xor     eax, eax
        cmp     [pedescr], eax
        jz      @f
        mov     [hdr_eip], eax
        mov     [hdr_esp], eax
        mov     [hdr_emem], eax
@@:
        stdcall create_process, [hdr_emem]  ; create a new process
        mov     edi, -TASKMAN_ERROR_OUT_OF_MEMORY
        test    eax, eax
        jz      .err_hdr

; add new process to the list
        mov     ebx, [sys_proc+LHEAD.prev]
        __list_add eax, ebx, sys_proc
; fill the structure fields:
        mov     ebx, [hdr_emem]
        mov     [eax+PROC.mem_used], ebx

; write that main thread of app belongs to new process
        mov     ebx, [slot_base]
        mov     [ebx+APPDATA.process], eax

; initialize the thread list of process: at this moment it consists only of one main thread
        lea     edx, [ebx+APPDATA.list]
        lea     ecx, [eax+PROC.thr_list]
        list_add_tail edx, ecx

; allocate space and copy app header data locals and cmdline string there, put pointer to exec_params of new thread
        mov     eax, [cmdline_size]
        add     eax, sizeof.APP_HDR
        stdcall kernel_alloc, eax
        mov     [ebx+APPDATA.exec_params], eax
        mov     edi, eax
        lea     esi, [hdr_cmdline]
        mov     ecx, sizeof.APP_HDR/4
        rep movsd
        mov     ecx, [cmdline_size]
        mov     esi, [cmdline]
        rep movsb
; set other parameters of application
        lea     eax, [hdr_cmdline]
        stdcall set_app_params , [slot], eax, [flags]
        mov     eax, [process_number]   ; return process number
        call    unlock_application_table
        ret

.err_0:
        mov     esi, [pedescr]
        test    esi, esi
        jz      @f
        call    dereference_pe
@@:
        call    unlock_application_table
.err_hdr:
        stdcall kernel_free, [file_base]
.err_file:
        stdcall kernel_free, [path_string]
        mov     eax, edi
        ret
endp

align 4
test_app_header:
       virtual at eax
         APP_HEADER_00 APP_HEADER_00_
       end virtual
       virtual at eax
         APP_HEADER_01 APP_HEADER_01_
       end virtual

        cmp     dword [eax], 'MENU'
        jne     .fail
        cmp     word [eax+4], 'ET'
        jne     .fail

        cmp     [eax+6], word '00'
        jne     .check_01_header

        mov     ecx, [APP_HEADER_00.start]
        mov     [ebx+APP_HDR.eip], ecx
        mov     edx, [APP_HEADER_00.mem_size]
        mov     [ebx+APP_HDR._emem], edx
        shr     edx, 1
        sub     edx, 0x10
        mov     [ebx+APP_HDR.esp], edx
        mov     ecx, [APP_HEADER_00.i_param]
        mov     [ebx+APP_HDR.cmdline], ecx
        mov     [ebx+APP_HDR.path], 0
        mov     edx, [APP_HEADER_00.i_end]
        mov     [ebx+APP_HDR._edata], edx
        ret

 .check_01_header:

        cmp     [eax+6], word '01'
        je      @f
        cmp     [eax+6], word '02'
        jne     .fail
@@:
        mov     ecx, [APP_HEADER_01.start]
        mov     [ebx+0x08], ecx
        mov     edx, [APP_HEADER_01.mem_size]

; \begin{diamond}[20.08.2006]
; sanity check (functions 19,58 load app_i_end bytes and that must
; fit in allocated memory to prevent kernel faults)
        cmp     edx, [APP_HEADER_01.i_end]
        jb      .fail
; \end{diamond}[20.08.2006]

        mov     [ebx+APP_HDR._emem], edx
        mov     ecx, [APP_HEADER_01.stack_top]
        mov     [ebx+APP_HDR.esp], ecx
        mov     edx, [APP_HEADER_01.i_param]
        mov     [ebx+APP_HDR.cmdline], edx
        mov     ecx, [APP_HEADER_01.i_icon]
        mov     [ebx+APP_HDR.path], ecx
        mov     edx, [APP_HEADER_01.i_end]
        mov     [ebx+APP_HDR._edata], edx
        ret
.fail:
        xor     eax, eax
        ret

align 4
alloc_thread_slot:
;input:
;  none
;result:
;  eax=[new_thread_slot]<>0 - ok
;      0 - failed.
;This function find least empty slot.
;It doesn't increase [thread_count]!


        mov     edx, thr_slot_map
        pushfd
        cli
.l1:
        bsf     eax, [edx]
        jnz     .found
        add     edx, 4
        cmp     edx, thr_slot_map+32
        jb      .l1

        popfd
        xor     eax, eax
        ret
.found:
        btr     [edx], eax
        sub     edx, thr_slot_map
        lea     eax, [eax+edx*8]
        popfd
        ret

align 4
proc create_process stdcall, app_size:dword
       locals
         process     dd ?
         app_tabs    dd ?
       endl

        push    ebx
        push    esi
        push    edi

        xor     eax, eax
        mov     [process], eax

        mov     eax, [app_size]
        add     eax, 0x3FFFFF
        shr     eax, 22
        mov     [app_tabs], eax

        stdcall kernel_alloc, 0x2000
        test    eax, eax
        jz      .fail
        mov     [process], eax

        lea     edi, [eax+PROC.heap_base]

        list_init eax
        add     eax, PROC.thr_list
        list_init eax
        add     eax, PROC.smap_list - PROC.thr_list
        list_init eax

        lea     ecx, [eax+PROC.heap_lock-PROC.smap_list]
        call    mutex_init

        mov     ecx, (PROC.ht_free-PROC.heap_base)/4
        xor     eax, eax
        cld
        rep stosd

        mov     [edi], dword (PROC.pdt_0 - PROC.htab)/4 - 3
        mov     [edi+4], dword 3           ;reserve handles for stdin stdout and stderr
        mov     ecx, (PROC.pdt_0 - PROC.htab)/4
        add     edi, 8
        inc     eax
@@:
        stosd
        inc     eax
        cmp     eax, ecx
        jbe     @B

        mov     eax, edi
        call    get_pg_addr
        mov     [edi-4096+PROC.pdt_0_phys], eax

        mov     ecx, (OS_BASE shr 20)/4
        xor     eax, eax
        rep stosd

        mov     ecx, (OS_BASE shr 20)/4
        mov     esi, sys_proc+PROC.pdt_0+(OS_BASE shr 20)
        rep movsd

        mov     eax, [edi-8192+PROC.pdt_0_phys]
        or      eax, PG_SWR
        mov     [edi-4096+(page_tabs shr 20)], eax

        lea     edx, [edi-4096]
        mov     esi, [app_tabs]
        test    esi, esi
        jz      .no_page_dirs

.alloc_page_dir:
        call    alloc_page
        test    eax, eax
        jz      .fail
        or      eax, PG_UWR
        mov     [edx], eax

        mov     edi, [tmp_task_ptab]
        stdcall map_page, edi, eax, PG_SWR
        mov     ecx, 1024
        xor     eax, eax
        rep stosd

        add     edx, 4
        dec     esi
        jnz     .alloc_page_dir

.no_page_dirs:
        stdcall map_page, [tmp_task_ptab], 0, PG_UNMAP
        mov     eax, [process]

        pop     edi
        pop     esi
        pop     ebx
        ret
.fail:
        mov     ecx, [process]
        jecxz   @F

        call    destroy_process
@@:
        xor     eax, eax
        pop     edi
        pop     esi
        pop     ebx
        ret
endp

align 4
proc destroy_page_table stdcall, pg_tab:dword

        push    esi

        mov     esi, [pg_tab]
        mov     ecx, 1024
.free:
        mov     eax, [esi]
        test    eax, 1
        jz      .next
        test    eax, 1 shl 9
        jnz     .next                     ;skip shared pages
        call    free_page
.next:
        add     esi, 4
        dec     ecx
        jnz     .free
        pop     esi
        ret
endp

align 4
destroy_process: ;fastcall ecx= ptr to process

        lea     eax, [ecx+PROC.thr_list]
        cmp     eax, [eax+LHEAD.next]
        jne     .exit

align 4
.internal:
        push    ecx

        mov     esi, ecx
        list_del esi

        lea     ebx, [esi+PROC.smap_list]
        mov     edi, [esi+PROC.smap_list+SMAP.fd]
.smap_list_destroy:
        cmp     edi, ebx
        jz      .smap_list_done
        push    [edi+SMAP.fd]
        stdcall destroy_smap, [esp+4]
        pop     edi
        jmp     .smap_list_destroy
.smap_list_done:

        mov     esi, [esp]
        add     esi, PROC.pdt_0
        mov     edi, (0x80000000 shr 20)/4
.destroy:
        mov     eax, [esi]
        test    eax, 1
        jz      .next
        and     eax, not 0xFFF
        stdcall map_page, [tmp_task_ptab], eax, PG_SWR
        stdcall destroy_page_table, [tmp_task_ptab]
        mov     eax, [esi]
        call    free_page
.next:
        add     esi, 4
        dec     edi
        jnz     .destroy

        call    kernel_free     ;ecx still in stack
        stdcall map_page, [tmp_task_ptab], 0, PG_UNMAP
.exit:
        ret

align 4
get_pid:
        mov     eax, [TASK_BASE]
        mov     eax, [eax+TASKDATA.pid]
        ret

pid_to_slot:
;Input:
;  eax - pid of process
;Output:
;  eax - slot of process or 0 if process don't exists
;Search process by PID.
        push    ebx
        push    ecx
        mov     ebx, [thread_count]
        shl     ebx, BSF sizeof.TASKDATA ; multiply by size 
        ; add 2*32 cause:
        ; [TASK_TABLE; TASK_TABLE + 32) isnt a task actually
        ; skip first process in the task table
        mov     ecx, 2*32

.loop:
;ecx = offset of current process info entry
;ebx = maximum permitted offset
        cmp     [TASK_TABLE+ecx+TASKDATA.state], TSTATE_FREE
        jz      .endloop ;skip empty slots
        cmp     [TASK_TABLE+ecx+TASKDATA.pid], eax;check PID
        jz      .pid_found
.endloop:
        add     ecx, sizeof.TASKDATA
        cmp     ecx, ebx
        jle     .loop

        pop     ecx
        pop     ebx
        xor     eax, eax
        ret

.pid_found:
        shr     ecx, BSF sizeof.TASKDATA ; divide by size
        mov     eax, ecx ;convert offset to index of slot
        pop     ecx
        pop     ebx
        ret


align 4
proc read_process_memory
;Input:
;  eax - process slot
;  ecx - buffer address
;  edx - buffer size
;  esi - start address in other process
;Output:
;  eax - number of bytes read.
       locals
         slot   dd ?
         buff   dd ?
         r_count    dd ?
         offset dd ?
         tmp_r_cnt  dd ?
         mapped_size dd ?
       endl

        mov     [slot], eax
        mov     [buff], ecx
        and     [r_count], 0
        mov     [tmp_r_cnt], edx
        mov     [offset], esi

        pushad
        mov     ecx, proc_mem_mutex
        call    mutex_lock
.read_mem:
        mov     edx, [offset]
        mov     ebx, [tmp_r_cnt]

        mov     ecx, 0x400000
        and     edx, 0x3FFFFF
        sub     ecx, edx
        cmp     ecx, ebx
        jbe     @f
        mov     ecx, ebx
@@:
        cmp     ecx, 0x8000
        jna     @F
        mov     ecx, 0x8000
@@:
        mov     ebx, [offset]

        push    ecx
        stdcall map_memEx, [proc_mem_map], \
                [slot], ebx, ecx, PG_READ, [proc_mem_tab]
        mov     [mapped_size], eax
        pop     ecx

        mov     esi, [offset]
        and     esi, 0xfff
        sub     eax, esi
        jbe     .ret_unmap
        cmp     ecx, eax
        jbe     @f
        mov     ecx, eax
        mov     [tmp_r_cnt], eax
@@:
        add     esi, [proc_mem_map]
        mov     edi, [buff]
        push    ecx
        rep movsb
        stdcall unmap_memEx, [proc_mem_map], \
                [slot], ebx, [mapped_size], [proc_mem_tab]
        pop     ecx
        add     [r_count], ecx

        add     [offset], ecx
        sub     [tmp_r_cnt], ecx
        jnz     .read_mem
.ret:
        mov     ecx, proc_mem_mutex
        call    mutex_unlock
        popad
        mov     eax, [r_count]
        ret
.ret_unmap:
        stdcall unmap_memEx, [proc_mem_map], \
                [slot], ebx, [mapped_size], [proc_mem_tab]
        jmp     .ret
endp

align 4
proc write_process_memory
;Input:
;  eax - process slot
;  ecx - buffer address
;  edx - buffer size
;  esi - start address in other process
;Output:
;  eax - number of bytes written

       locals
         slot   dd ?
         buff   dd ?
         w_count    dd ?
         offset dd ?
         tmp_w_cnt  dd ?
         mapped_size dd ?
       endl

        mov     [slot], eax
        mov     [buff], ecx
        and     [w_count], 0
        mov     [tmp_w_cnt], edx
        mov     [offset], esi

        pushad
        mov     ecx, proc_mem_mutex
        call    mutex_lock
.write_mem:
        mov     edx, [offset]
        mov     ebx, [tmp_w_cnt]

        mov     ecx, 0x400000
        and     edx, 0x3FFFFF
        sub     ecx, edx
        cmp     ecx, ebx
        jbe     @f
        mov     ecx, ebx
@@:
        cmp     ecx, 0x8000
        jna     @F
        mov     ecx, 0x8000
@@:
        mov     ebx, [offset]
        push    ecx
        stdcall map_memEx, [proc_mem_map], \
                [slot], ebx, ecx, PG_SWR, [proc_mem_tab]
        mov     [mapped_size], eax
        pop     ecx

        mov     edi, [offset]
        and     edi, 0xfff
        sub     eax, edi
        jbe     .ret_unmap
        cmp     ecx, eax
        jbe     @f
        mov     ecx, eax
        mov     [tmp_w_cnt], eax
@@:
        add     edi, [proc_mem_map]
        mov     esi, [buff]
        push    ecx
        rep movsb
        stdcall unmap_memEx, [proc_mem_map], \
                [slot], ebx, [mapped_size], [proc_mem_tab]
        pop     ecx

        add     [w_count], ecx
        add     [offset], ecx
        sub     [tmp_w_cnt], ecx
        jnz     .write_mem
.ret:
        mov     ecx, proc_mem_mutex
        call    mutex_unlock
        popad
        mov     eax, [w_count]
        ret
.ret_unmap:
        stdcall unmap_memEx, [proc_mem_map], [slot], ebx, [mapped_size], [proc_mem_tab]
        jmp     .ret
endp

;ebx = 1 - kernel thread
;ecx=thread entry point
;edx=thread stack pointer
;creation flags  0x01 - debugged
;                0x02 - kernel

align 4
proc new_sys_threads
       locals
         slot          dd ?
         flags         dd ?
         app_cmdline   dd ? ;0x00
         app_path      dd ? ;0x04
         app_eip       dd ? ;0x08
         app_esp       dd ? ;0x0C
         app_mem       dd ? ;0x10
       endl

        shl     ebx, 1
        mov     [flags], ebx

        xor     eax, eax
        mov     [app_eip], ecx
        mov     [app_cmdline], eax
        mov     [app_esp], edx
        mov     [app_path], eax

        call    lock_application_table

        call    alloc_thread_slot
        test    eax, eax
        jz      .failed

        mov     [slot], eax

        mov     esi, [current_slot]
        mov     ebx, esi      ;ebx=esi - pointer to extended information about current thread

        mov     edi, eax
        shl     edi, 8
        add     edi, SLOT_BASE
        mov     edx, edi      ;edx=edi - pointer to extended infomation about new thread
        mov     ecx, sizeof.APPDATA/4
        xor     eax, eax
        cld
        rep stosd             ;clean extended information about new thread
        mov     esi, ebx
        mov     edi, edx
        mov     ecx, 11
        rep movsb             ;copy process name


        mov     eax, [ebx+APPDATA.tls_base]
        test    eax, eax
        jz      @F

        push    edx
        stdcall user_alloc, 4096
        pop     edx
        test    eax, eax
        jz      .failed1;eax=0
@@:
        mov     [edx+APPDATA.tls_base], eax

        mov     eax, [ebx+APPDATA.process]
        mov     [edx+APPDATA.process], eax

        lea     ebx, [edx+APPDATA.list]
        lea     ecx, [eax+PROC.thr_list]
        list_add_tail ebx, ecx               ;add thread to process child's list

        lea     eax, [app_cmdline]
        stdcall set_app_params , [slot], eax, [flags]

        mov     eax, [process_number]           ;set result
        call    unlock_application_table
        ret
.failed:
        xor     eax, eax
.failed1:
        call    unlock_application_table
        dec     eax     ;-1
        ret
endp

proc map_process_image stdcall, img_size:dword, file_base:dword, file_size:dword

        mov     edx, [img_size]
        mov     esi, [file_base]
        mov     ecx, [file_size]
        add     edx, 4095
        add     ecx, 4095
        shr     edx, 12        ; total pages
        shr     ecx, 12        ; image pages

        mov     edi, page_tabs
        shr     esi, 10
        add     esi, edi

.map_image:
        lodsd
        and     eax, -4096
        or      eax, PG_UWR
        stosd
        dec     edx
        loop    .map_image

        test    edx, edx
        jz      .done
.map_bss:
        call    alloc_page
        test    eax, eax
        jz      .fail

        or      eax, PG_UWR
        stosd
        dec     edx
        jnz     .map_bss

        mov     edi, [file_size]
        mov     ecx, [img_size]
        add     edi, 4095
        and     edi, -4096
        add     ecx, 4095
        and     ecx, -4096
        sub     ecx, edi
        shr     ecx, 2
        xor     eax, eax
        rep stosd
.done:
.fail:
        ret
endp

align 4
common_app_entry:
        mov     ebp, [current_slot]
        mov     ebp, [ebp+APPDATA.exec_params]
        test    ebp, ebp
        jz      .exit
; APPDATA.exec_params have first thread only,
; so second and next threads don't get here (they jump to .exit) 
        cmp     [ebp+APP_HDR.pedescr], 0
        jz      .init_legacy_app
; init PE application
        mov     eax, [current_process]
        mov     [eax+PROC.mem_used], 0xF000 ; leave first 64K as unallocatable
        call    init_heap
        mov     eax, [current_process]
        mov     [eax+PROC.mem_used], 0
        stdcall map_pe_usermode, [ebp+APP_HDR.pedescr],\
                [ebp+APP_HDR.img_base], [ebp+APP_HDR.img_size]
        cmp     eax, -0x1000
        ja      .failed
        push    eax
        stdcall load_file_maybe_pe, pe_loader_usermode
        test    esi, esi
        jz      .pe_loader_notfound
        mov     edx, [esi+PEDESCR.entry]
        mov     [esp+4+20h], edx
        stdcall map_pe_usermode, esi, eax, ebx
        cmp     eax, -0x1000
        ja      .pe_loader_failed
        add     [esp+4+20h], eax
        push    eax
        mov     eax, [ebp+APP_HDR.filename_size]
        add     eax, [ebp+APP_HDR.cmdline_size]
        add     eax, sizeof.kernel_init_data + 2
        stdcall user_alloc, eax
        test    eax, eax
        jz      .failed
        mov     ebx, eax
        mov     dword [eax+kernel_init_data.version], 1 + (0 shl 16) ; version, flags
        mov     [eax+kernel_init_data.syscall_method], SYSCALL_METHOD_I40
        lea     edi, [eax+sizeof.kernel_init_data]
        mov     [eax+kernel_init_data.exe_path], edi
        mov     esi, [ebp+APP_HDR.path_string]
        mov     ecx, [ebp+APP_HDR.filename_size]
        rep movsb
        mov     byte [edi], 0
        inc     edi
        mov     [eax+kernel_init_data.command_line], edi
        lea     esi, [ebp+sizeof.APP_HDR]
        mov     ecx, [ebp+APP_HDR.cmdline_size]
        rep movsb
        mov     byte [edi], 0
        mov     ecx, [ebp+APP_HDR.pedescr]
        mov     ecx, [ecx+PEDESCR.stacksize]
        mov     [eax+kernel_init_data.stack_size], ecx
        stdcall user_alloc, ecx
        test    eax, eax
        jz      .failed
        mov     [ebx+kernel_init_data.stack_base], eax
        add     eax, [ebx+kernel_init_data.stack_size]
        sub     eax, 16
        pop     dword [eax+4]
        pop     [ebx+kernel_init_data.exe_base]
        mov     dword [eax+8], 1 ; DLL_PROCESS_ATTACH
        mov     dword [eax+12], ebx
        mov     [esp+2Ch], eax
        jmp     .common_tls
.pe_loader_notfound:
        cmp     eax, -0x1000
        ja      .pe_loader_failed
        stdcall kernel_free, eax
.pe_loader_failed:
        dbgstr 'Failed to load kolibri.dll'
.failed:
        stdcall kernel_free, [ebp+APP_HDR.path_string]
        jmp     sys_end
.init_legacy_app:
; init MENUETxx application
        stdcall map_process_image, [ebp+APP_HDR._emem],\
                [ebp+APP_HDR.img_base], [ebp+APP_HDR.img_size]
        mov     esi, [ebp+APP_HDR.path_string]
        mov     edi, [ebp+APP_HDR.path]
        mov     ecx, [ebp+APP_HDR.filename_size]
        cmp     ecx, 1023
        jc      @f
        mov     ecx, 1022
@@:
        push    esi
        test    edi, edi
        jz      @f
        stdcall is_region_userspace, edi, [ebp+APP_HDR.filename_size]
        jnz     @f
        mov     al, '/'
        stosb
        rep movsb
        mov     byte [edi], 0
@@:
        call    kernel_free
        mov     edi, [ebp+APP_HDR.cmdline]
        test    edi, edi
        jz      .check_tls_header
        lea     esi, [ebp+sizeof.APP_HDR]
        mov     ecx, [ebp+APP_HDR.cmdline_size]
        cmp     ecx, 256
        jb      .copy_cmdline
        mov     edi, [ebp+APP_HDR._emem]
        add     edi, 4095
        and     edi, -4096
        sub     edi, ecx
        dec     edi
        cmp     word [6], '00'
        jne     @f
        mov     [APP_HEADER_00_.i_param], edi
        jmp     .copy_cmdline
@@:
        mov     [APP_HEADER_01_.i_param], edi
.copy_cmdline:
        inc     ecx  ; keep in mind about 0 in the end
        stdcall is_region_userspace, edi, ecx
        jnz     .check_tls_header
        dec     ecx
        rep movsb
        mov     byte [edi], 0
.check_tls_header:
        cmp     word [6], '02'
        jne     .try_load_dll ;.common
        call    init_heap
.common_tls:
        stdcall user_alloc, 4096
        mov     edx, [current_slot]
        mov     [edx+APPDATA.tls_base], eax
        mov     [tls_data_l+2], ax
        shr     eax, 16
        mov     [tls_data_l+4], al
        mov     [tls_data_l+7], ah
        mov     dx, app_tls
        mov     fs, dx       
; { Patch by Coldy, For DLL autoload    
.try_load_dll:         
; Test app header version 
        mov     ecx, dword[ebp+APP_HDR.img_base]
        cmp     dword[ecx+8], 2
        jne     .common
;if APP_HEADER.version = 2 => load lib/dll.obj & change eip to APP_STARTUP_THUNK
        DEBUGF 1, 'K : App header version 2\n'
        stdcall load_library, dll_lib_path, 0
        cmp     eax, 0
        jne     @f
; Something went wrong (TODO: Next 2 line is code copy after .cleanup)  
        stdcall free_kernel_space, [ebp+APP_HDR.img_base]
        stdcall kernel_free, ebp
        DEBUGF 1, 'K : DLL.OBJ not found! Terminate application!\n'
        mov     ebx, dll_error_msg
        mov     ebp, notifyapp
        call    fs_execute_from_sysdir_param
; Terminate process (TODO: Need jump to .cleanup after sys_end ?) 
        call    sys_end

@@:       
; Find APP_STARTUP_THUNK in DLL.OBJ
        sub     eax, 4
        mov     eax, [eax]

;.change_eip:
        mov     ecx, [current_slot]
        mov     ecx, [ecx+APPDATA.pl0_stack]
        mov     [ecx+REG_EIP], eax
        
; } End patch by Coldy, For DLL autoload
        mov     fs, dx

.common:
        stdcall free_kernel_space, [ebp+APP_HDR.img_base]
        stdcall kernel_free, ebp
        mov     ebx, [current_slot]
        cmp     [ebx+APPDATA.debugger_slot], 0
        je      .exit
        mov     eax, [TASK_BASE]
        mov     [eax+TASKDATA.state], TSTATE_RUN_SUSPENDED
        call    change_task
.exit:
        popad
        iretd

EFL_IF      = 0x0200
EFL_IOPL1   = 0x1000
EFL_IOPL2   = 0x2000
EFL_IOPL3   = 0x3000

align 4
proc set_app_params stdcall,slot:dword, params:dword, flags:dword

       locals
         pl0_stack dd ?
       endl

        mov     eax, [xsave_area_size]
        add     eax, RING0_STACK_SIZE
        stdcall kernel_alloc, eax
        mov     [pl0_stack], eax

        lea     edi, [eax+RING0_STACK_SIZE]

        mov     eax, [slot]
        mov     ebx, eax

        shl     eax, 8
        mov     [eax+SLOT_BASE+APPDATA.fpu_state], edi
        mov     [eax+SLOT_BASE+APPDATA.exc_handler], 0
        mov     [eax+SLOT_BASE+APPDATA.except_mask], 0
        mov     [eax+SLOT_BASE+APPDATA.terminate_protection], 80000001h

;set default io permission map
        mov     ecx, [SLOT_BASE+sizeof.APPDATA+APPDATA.io_map]
        mov     [eax+SLOT_BASE+APPDATA.io_map], ecx
        mov     ecx, [SLOT_BASE+sizeof.APPDATA+APPDATA.io_map+4]
        mov     [eax+SLOT_BASE+APPDATA.io_map+4], ecx

        mov     esi, fpu_data
        mov     ecx, [xsave_area_size]
        add     ecx, 3
        shr     ecx, 2
        rep movsd

        cmp     [thread_count], ebx
        adc     [thread_count], 0   ; update number of processes
        shl     ebx, 8
        lea     edx, [ebx+SLOT_BASE+APP_EV_OFFSET]
        mov     [SLOT_BASE+APPDATA.fd_ev+ebx], edx
        mov     [SLOT_BASE+APPDATA.bk_ev+ebx], edx

        add     edx, APP_OBJ_OFFSET-APP_EV_OFFSET
        mov     [SLOT_BASE+APPDATA.fd_obj+ebx], edx
        mov     [SLOT_BASE+APPDATA.bk_obj+ebx], edx

        mov     ecx, [def_cursor]
        mov     [SLOT_BASE+APPDATA.cursor+ebx], ecx
        mov     eax, [pl0_stack]
        mov     [SLOT_BASE+APPDATA.pl0_stack+ebx], eax
        add     eax, RING0_STACK_SIZE
        mov     [SLOT_BASE+APPDATA.saved_esp0+ebx], eax

        push    ebx
        stdcall kernel_alloc, maxPathLength
        pop     ebx
        mov     esi, [current_slot]
        mov     esi, [esi+APPDATA.cur_dir]
        mov     ecx, maxPathLength/4
        mov     edi, eax
        mov     [ebx+SLOT_BASE+APPDATA.cur_dir], eax
        rep movsd

        shr     ebx, 3
        mov     dword [TASK_TABLE+ebx+TASKDATA.mem_start], 0

        mov     ebx, [slot]
        mov     eax, ebx
        shl     ebx, 5
        lea     ecx, [draw_data+ebx];ecx - pointer to draw data

; set window state to 'normal' (non-minimized/maximized/rolled-up) state
        mov     [ebx+window_data+WDATA.fl_wstate], WSTATE_NORMAL
        mov     [ebx+window_data+WDATA.fl_redraw], 1
        add     ebx, TASK_TABLE     ;ebx - pointer to information about process
        mov     [ebx+TASKDATA.wnd_number], al;set window number on screen = process slot

        mov     [ebx+TASKDATA.event_mask], dword 1+2+4;set default event flags (see 40 function)

        inc     dword [process_number]
        mov     eax, [process_number]
        mov     [ebx+TASKDATA.pid], eax    ;set PID

;set draw data to full screen
        xor     eax, eax
        mov     [ecx+0], dword eax
        mov     [ecx+4], dword eax
        mov     eax, [screen_workarea.right]
        mov     [ecx+8], eax
        mov     eax, [screen_workarea.bottom]
        mov     [ecx+12], eax

        mov     ebx, [pl0_stack]
        mov     esi, [params]
        lea     ecx, [ebx+REG_EIP]
        xor     eax, eax

        mov     [ebx+REG_RET], dword common_app_entry
        mov     [ebx+REG_EDI], eax
        mov     [ebx+REG_ESI], eax
        mov     [ebx+REG_EBP], eax
        mov     [ebx+REG_ESP], ecx;ebx+REG_EIP
        mov     [ebx+REG_EBX], eax
        mov     [ebx+REG_EDX], eax
        mov     [ebx+REG_ECX], eax
        mov     [ebx+REG_EAX], eax

        mov     eax, [esi+APP_HDR.eip]
        mov     [ebx+REG_EIP], eax
        mov     [ebx+REG_CS], dword app_code
        mov     ecx, USER_PRIORITY

        test    byte [flags], 2
        jz      @F

        mov     [ebx+REG_CS], dword os_code ; kernel thread
        mov     ecx, MAX_PRIORITY
@@:
        mov     [ebx+REG_EFLAGS], dword EFL_IOPL1+EFL_IF

        mov     eax, [esi+APP_HDR.esp]
        mov     [ebx+REG_APP_ESP], eax
        mov     [ebx+REG_SS], dword app_data

        lea     edx, [ebx+REG_RET]
        mov     ebx, [slot]
        shl     ebx, 5
        mov     [ebx*8+SLOT_BASE+APPDATA.saved_esp], edx

        xor     edx, edx; process state - running
; set if debuggee
        test    byte [flags], 1
        jz      .no_debug
        mov     eax, [current_slot_idx]
        mov     [SLOT_BASE+ebx*8+APPDATA.debugger_slot], eax
.no_debug:
        mov     [TASK_TABLE+ebx+TASKDATA.state], dl
        lea     edx, [SLOT_BASE+ebx*8]
        call    scheduler_add_thread
        ret
endp

align 4
get_stack_base:
        mov     eax, [current_slot]
        mov     eax, [eax+APPDATA.pl0_stack]
        ret


include "debug.inc"
