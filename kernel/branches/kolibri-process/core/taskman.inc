;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 4424 $


GREEDY_KERNEL  equ 0

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


struct  APP_PARAMS
        app_cmdline     dd ?    ;0x00
        app_path        dd ?    ;0x04
        app_eip         dd ?    ;0x08
        app_esp         dd ?    ;0x0C
        app_mem         dd ?    ;0x10
ends

macro _clear_ op
{  mov ecx, op/4
        xor     eax, eax
        cld
        rep stosd
}

fs_execute_from_sysdir:
        xor     ebx, ebx
fs_execute_from_sysdir_param:
        xor     edx, edx
        mov     esi, sysdir_path

align 4
proc fs_execute

;fn_read:dword, file_size:dword, cluster:dword

; ebx - cmdline
; edx - flags
; ebp - full filename
; [esp+4] = procedure DoRead, [esp+8] = filesize & [esp+12]... - arguments for it

       locals
         cmdline_size  dd ? ; +0 ; cmdline -12
         cmdline_adr   dd ? ; +4 ; cmdline -8
         cmdline_flag  dd ? ; +8 ; cmdline -4
         cmdline       rd 64    ;256/4
         filename      rd 256   ;1024/4
         flags         dd ?

         save_proc     dd ?
         slot          dd ?
         slot_base     dd ?
         file_base     dd ?
         file_size     dd ?
         handle        dd ? ;temp. for default cursor handle for curr. thread
                      ;app header data
         hdr_cmdline   dd ? ;0x00
         hdr_path      dd ? ;0x04
         hdr_eip       dd ? ;0x08
         hdr_esp       dd ? ;0x0C
         hdr_mem       dd ? ;0x10
         hdr_i_end     dd ? ;0x14
       endl

        pushad

        cmp     [SCR_MODE], word 0x13
        jbe     @f
        pushad
        stdcall set_cursor, [def_cursor_clock]
        mov     [handle], eax
        mov     [redrawmouse_unconditional], 1
        call    wakeup_osloop
        popad
@@:
        mov     [flags], edx

; [ebp]  pointer to filename

        lea     edi, [filename]
        lea     ecx, [edi+1024]
        mov     al, '/'
        stosb
@@:
        cmp     edi, ecx
        jae     .bigfilename
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     esi, [ebp]
        test    esi, esi
        jz      .namecopied
        mov     byte [edi-1], '/'
@@:
        cmp     edi, ecx
        jae     .bigfilename
        lodsb
        stosb
        test    al, al
        jnz     @b
        jmp     .namecopied
.bigfilename:
        popad
        mov     eax, -ERROR_FILE_NOT_FOUND

        jmp     .final

.namecopied:
        xor     eax, eax
        mov     [cmdline_flag], eax
        mov     [cmdline_adr], eax
        mov     [cmdline_size], eax

        mov     [cmdline], ebx
        test    ebx, ebx
        jz      .no_copy
;--------------------------------------
        pushad
        pushfd
        mov     esi, ebx
        mov     ecx, 65536 ; 64 Kb max for ext.cmdline
        cld
@@:
        dec     ecx
        jz      .end_string

        lodsb
        test    al, al
        jnz     @b

.end_string:
        mov     eax, 65536 ; 64 Kb max for ext.cmdline
        sub     eax, ecx
        mov     [cmdline_size], eax
        cmp     eax, 255
        ja      @f

        popfd
        popad
        jmp     .old_copy

@@:
        xor     eax, eax
        dec     eax
        mov     [cmdline_flag], eax
        popfd
        popad
; get memory for the extended command line
        stdcall kernel_alloc, [cmdline_size] ;eax
        test    eax, eax
        jz      .old_copy ; get memory failed

        mov     [cmdline_adr], eax

        pushad
        pushfd
        mov     esi, ebx
        mov     edi, eax
        mov     ecx, [cmdline_size]
        cld
        rep movsb
        popfd
        popad
        jmp     .no_copy

.old_copy:
; clear flag because old method with 256 bytes
        xor     eax, eax
        mov     [cmdline_flag], eax
;--------------------------------------
        lea     eax, [cmdline]
        mov     dword [eax+252], 0
.copy:
        stdcall strncpy, eax, ebx, 255
.no_copy:
        lea     eax, [filename]
        stdcall load_file, eax

        mov     esi, -ERROR_FILE_NOT_FOUND
        test    eax, eax
        jz      .err_file

        mov     [file_base], eax
        mov     [file_size], ebx

        lea     ebx, [hdr_cmdline]
        call    test_app_header
        mov     esi, -0x1F
        test    eax, eax
        jz      .err_hdr

        call    lock_application_table

        call    alloc_thread_slot
        test    eax, eax
        mov     esi, -0x20 ; too many processes
        jz      .err

        mov     [slot], eax
        shl     eax, 8
        add     eax, SLOT_BASE
        mov     [slot_base], eax
        mov     edi, eax
       _clear_ 256     ;clean extended information about process

; write application name
        lea     eax, [filename]
        stdcall strrchr, eax, '/'  ; now eax points to name without path

        lea     esi, [eax+1]
        test    eax, eax
        jnz     @F
        lea     esi, [filename]
@@:
        mov     ecx, 11 ; 11 chars for name! 8 - is old value!
        mov     edi, [slot_base]
.copy_process_name_loop:
        lodsb
        cmp     al, '.'
        jz      .copy_process_name_done
        test    al, al
        jz      .copy_process_name_done
        stosb
        loop    .copy_process_name_loop
.copy_process_name_done:

        mov     ebx, [current_process]
        mov     [save_proc], ebx

        stdcall create_process, [hdr_mem], [file_base], [file_size]
        mov     esi, -30; no memory
        test    eax, eax
        jz      .failed

        mov     ebx, [hdr_mem]
        mov     [eax+PROC.mem_used], ebx

        mov     ebx, [slot_base]
        mov     [ebx+APPDATA.process], eax

        xor     edx, edx
        cmp     word [6], '02'
        jne     @f

        not     edx
@@:
        mov     [ebx+APPDATA.tls_base], edx

if GREEDY_KERNEL
else
        mov     ecx, [hdr_mem]
        mov     edi, [file_size]
        add     edi, 4095
        and     edi, not 4095
        sub     ecx, edi
        jna     @F

        xor     eax, eax
        cld
        rep stosb
@@:
end if

; release only virtual space, not phisical memory

        stdcall free_kernel_space, [file_base]
        lea     eax, [hdr_cmdline]
        lea     ebx, [cmdline]
        lea     ecx, [filename]
        stdcall set_app_params , [slot], eax, ebx, ecx, [flags]

        mov     eax, [save_proc]
        call    set_cr3

        mov     eax, [process_number];set result
        call    unlock_application_table

        jmp     .final

.failed:
        mov     eax, [save_proc]
        call    set_cr3
.err:
.err_hdr:
        stdcall kernel_free, [file_base]
.err_file:
        call    unlock_application_table
        mov     eax, esi
.final:
        cmp     [SCR_MODE], word 0x13
        jbe     @f
        pushad
        stdcall set_cursor, [handle]
        mov     [redrawmouse_unconditional], 1
        call    wakeup_osloop
        popad
@@:
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
        mov     [ebx+0x08], ecx             ;app_eip
        mov     edx, [APP_HEADER_00.mem_size]
        mov     [ebx+0x10], edx             ;app_mem
        shr     edx, 1
        sub     edx, 0x10
        mov     [ebx+0x0C], edx             ;app_esp
        mov     ecx, [APP_HEADER_00.i_param]
        mov     [ebx], ecx                  ;app_cmdline
        mov     [ebx+4], dword 0            ;app_path
        mov     edx, [APP_HEADER_00.i_end]
        mov     [ebx+0x14], edx
        ret

 .check_01_header:

        cmp     [eax+6], word '01'
        je      @f
        cmp     [eax+6], word '02'
        jne     .fail
@@:
        mov     ecx, [APP_HEADER_01.start]
        mov     [ebx+0x08], ecx             ;app_eip
        mov     edx, [APP_HEADER_01.mem_size]

; \begin{diamond}[20.08.2006]
; sanity check (functions 19,58 load app_i_end bytes and that must
; fit in allocated memory to prevent kernel faults)
        cmp     edx, [APP_HEADER_01.i_end]
        jb      .fail
; \end{diamond}[20.08.2006]

        mov     [ebx+0x10], edx             ;app_mem
        mov     ecx, [APP_HEADER_01.stack_top]
        mov     [ebx+0x0C], ecx             ;app_esp
        mov     edx, [APP_HEADER_01.i_param]
        mov     [ebx], edx                  ;app_cmdline
        mov     ecx, [APP_HEADER_01.i_icon]
        mov     [ebx+4], ecx                ;app_path
        mov     edx, [APP_HEADER_01.i_end]
        mov     [ebx+0x14], edx
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
;It doesn't increase [TASK_COUNT]!


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
proc create_process stdcall, app_size:dword,img_base:dword,img_size:dword
       locals
         app_pages   dd ?
         img_pages   dd ?
         process     dd ?
         app_tabs    dd ?
       endl

        mov     ecx, pg_data.mutex
        call    mutex_lock

        xor     eax, eax
        mov     [process], eax

        mov     eax, [app_size]
        add     eax, 4095
        and     eax, NOT(4095)
        mov     [app_size], eax
        mov     ebx, eax
        shr     eax, 12
        mov     [app_pages], eax

        add     ebx, 0x3FFFFF
        and     ebx, NOT(0x3FFFFF)
        shr     ebx, 22
        mov     [app_tabs], ebx

        mov     ecx, [img_size]
        add     ecx, 4095
        and     ecx, NOT(4095)

        mov     [img_size], ecx
        shr     ecx, 12
        mov     [img_pages], ecx

        lea     eax, [eax+ebx+2];all requested memory

        cmp     eax, [pg_data.pages_free]
        ja      .fail

        stdcall    kernel_alloc, 0x2000
        test    eax, eax
        jz      .fail
        mov     [process], eax

        lea     edi, [eax+PROC.heap_lock]
        mov     ecx, (4096-PROC.heap_lock)/4

        list_init eax
        add     eax, PROC.thr_list
        list_init eax

        xor     eax, eax
        cld
        rep stosd

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
        or      eax, PG_SW
        mov     [edi-4096+(page_tabs shr 20)], eax

        lea     eax, [edi-8192]
        call    set_cr3

        mov     edx, [app_tabs]
        xor     edi, edi
@@:
        call    alloc_page
        test    eax, eax
        jz      .fail

        stdcall map_page_table, edi, eax
        add     edi, 0x00400000
        dec     edx
        jnz     @B

        mov     edi, page_tabs

        mov     ecx, [app_tabs]
        shl     ecx, 10
        xor     eax, eax
        rep stosd

        mov     ecx, [img_pages]
        mov     ebx, PG_UW
        xor     edx, edx
        mov     esi, [img_base]
        shr     esi, 10
        add     esi, page_tabs
        mov     edi, page_tabs
.remap:
        lodsd
        and     eax, 0xFFFFF000
        or      eax, ebx; force user level r/w access
        stosd
        add     edx, 0x1000
        dec     [app_pages]
        dec     ecx
        jnz     .remap

        mov     ecx, [app_pages]
        test    ecx, ecx
        jz      .done

.alloc:
        call    alloc_page
        test    eax, eax
        jz      .fail

        stdcall map_page, edx, eax, dword PG_UW
        add     edx, 0x1000
        dec     [app_pages]
        jnz     .alloc

.done:
        mov     ecx, pg_data.mutex
        call    mutex_unlock
        mov     eax, [process]
        ret
.fail:
        mov     ecx, pg_data.mutex
        call    mutex_unlock
        cmp     [process], 0
        je      @f
;;        stdcall destroy_app_space, [dir_addr], 0
@@:
        xor     eax, eax
        ret
endp

align 4
set_cr3:
        pushfd
        cli
        mov     ebx, [current_slot]
        mov     [current_process], eax
        mov     [ebx+APPDATA.process], eax
        mov     eax, [eax+PROC.pdt_0_phys]
        mov     cr3, eax
        popfd
        ret

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
proc destroy_app_space stdcall, pg_dir:dword, dlls_list:dword

        xor     edx, edx
        push    edx
        mov     eax, 0x1
        mov     ebx, [pg_dir]
.loop:
;eax = current slot of process
        mov     ecx, eax
        shl     ecx, 5
        cmp     byte [CURRENT_TASK+ecx+0xa], 9;if process running?
        jz      @f           ;skip empty slots
        shl     ecx, 3
        add     ecx, SLOT_BASE
        cmp     [ecx+APPDATA.process], ebx;compare page directory addresses
        jnz     @f
        mov     [ebp-4], ecx
        inc     edx             ;thread found
@@:
        inc     eax
        cmp     eax, [TASK_COUNT]   ;exit loop if we look through all processes
        jle     .loop

;edx = number of threads
;our process is zombi so it isn't counted
        pop     ecx
        cmp     edx, 1
        jg      .ret
;if there isn't threads then clear memory.
        mov     esi, [dlls_list]
        call    destroy_all_hdlls;ecx=APPDATA

        mov     ecx, pg_data.mutex
        call    mutex_lock

        mov     eax, [pg_dir]
        and     eax, not 0xFFF
;        stdcall map_page, [tmp_task_pdir], eax, PG_SW
;        mov     esi, [tmp_task_pdir]
        mov     edi, (OS_BASE shr 20)/4
.destroy:
        mov     eax, [esi]
        test    eax, 1
        jz      .next
        and     eax, not 0xFFF
        stdcall map_page, [tmp_task_ptab], eax, PG_SW
        stdcall destroy_page_table, [tmp_task_ptab]
        mov     eax, [esi]
        call    free_page
.next:
        add     esi, 4
        dec     edi
        jnz     .destroy

        mov     eax, [pg_dir]
        call    free_page
.exit:
        stdcall map_page, [tmp_task_ptab], 0, PG_UNMAP
;        stdcall map_page, [tmp_task_pdir], 0, PG_UNMAP
        mov     ecx, pg_data.mutex
        call    mutex_unlock
.ret:
        ret
endp

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
        mov     ebx, [TASK_COUNT]
        shl     ebx, 5
        mov     ecx, 2*32

.loop:
;ecx=offset of current process info entry
;ebx=maximum permitted offset
        cmp     byte [CURRENT_TASK+ecx+0xa], 9
        jz      .endloop ;skip empty slots
        cmp     [CURRENT_TASK+ecx+0x4], eax;check PID
        jz      .pid_found
.endloop:
        add     ecx, 32
        cmp     ecx, ebx
        jle     .loop

        pop     ecx
        pop     ebx
        xor     eax, eax
        ret

.pid_found:
        shr     ecx, 5
        mov     eax, ecx ;convert offset to index of slot
        pop     ecx
        pop     ebx
        ret

check_region:
;input:
;  esi - start of buffer
;  edx - size of buffer
;result:
;  eax = 1 region lays in app memory
;  eax = 0 region don't lays in app memory
        mov     eax, [CURRENT_TASK]
;     jmp  check_process_region
;-----------------------------------------------------------------------------
;check_process_region:
;input:
;  eax - slot
;  esi - start of buffer
;  edx - size of buffer
;result:
;  eax = 1 region lays in app memory
;  eax = 0 region don't lays in app memory

        test    edx, edx
        jle     .ok
        shl     eax, 5
        cmp     word [CURRENT_TASK+eax+0xa], 0
        jnz     .failed
        shl     eax, 3
        mov     eax, [SLOT_BASE+eax+0xb8]
        test    eax, eax
        jz      .failed

        mov     eax, 1
        ret


;    call MEM_Get_Linear_Address
;    push ebx
;    push ecx
;    push edx
;    mov  edx,ebx
;    and  edx,not (4096-1)
;    sub  ebx,edx
;    add  ecx,ebx
;    mov  ebx,edx
;    add  ecx,(4096-1)
;    and  ecx,not (4096-1)
;.loop:
;;eax - linear address of page directory
;;ebx - current page
;;ecx - current size
;    mov  edx,ebx
;    shr  edx,22
;    mov  edx,[eax+4*edx]
;    and  edx,not (4096-1)
;    test edx,edx
;    jz   .failed1
;    push eax
;    mov  eax,edx
;    call MEM_Get_Linear_Address
;    mov  edx,ebx
;    shr  edx,12
;    and  edx,(1024-1)
;    mov  eax,[eax+4*edx]
;    and  eax,not (4096-1)
;    test eax,eax
;    pop  eax
;    jz   .failed1
;    add  ebx,4096
;    sub  ecx,4096
;    jg   .loop
;    pop  edx
;    pop  ecx
;    pop  ebx
.ok:
        mov     eax, 1
        ret
;
;.failed1:
;    pop  edx
;    pop  ecx
;    pop  ebx
.failed:
        xor     eax, eax
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
       endl

        mov     [slot], eax
        mov     [buff], ecx
        and     [r_count], 0
        mov     [tmp_r_cnt], edx
        mov     [offset], esi

        pushad
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
                [slot], ebx, ecx, PG_MAP
        pop     ecx

        mov     esi, [offset]
        and     esi, 0xfff
        sub     eax, esi
        jbe     .ret
        cmp     ecx, eax
        jbe     @f
        mov     ecx, eax
        mov     [tmp_r_cnt], eax
@@:
        add     esi, [proc_mem_map]
        mov     edi, [buff]
        mov     edx, ecx
        rep movsb
        add     [r_count], edx

        add     [offset], edx
        sub     [tmp_r_cnt], edx
        jnz     .read_mem
.ret:
        popad
        mov     eax, [r_count]
        ret
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
       endl

        mov     [slot], eax
        mov     [buff], ecx
        and     [w_count], 0
        mov     [tmp_w_cnt], edx
        mov     [offset], esi

        pushad
.read_mem:
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
      ;     add ebx, new_app_base
        push    ecx
        stdcall map_memEx, [proc_mem_map], \
                [slot], ebx, ecx, PG_SW
        pop     ecx

        mov     edi, [offset]
        and     edi, 0xfff
        sub     eax, edi
        jbe     .ret
        cmp     ecx, eax
        jbe     @f
        mov     ecx, eax
        mov     [tmp_w_cnt], eax
@@:
        add     edi, [proc_mem_map]
        mov     esi, [buff]
        mov     edx, ecx
        rep movsb

        add     [w_count], edx
        add     [offset], edx
        sub     [tmp_w_cnt], edx
        jnz     .read_mem
.ret:
        popad
        mov     eax, [w_count]
        ret
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
        mov     ecx, 256/4
        xor     eax, eax
        cld
        rep stosd             ;clean extended information about new thread
        mov     esi, ebx
        mov     edi, edx
        mov     ecx, 11
        rep movsb             ;copy process name

        mov     eax, [ebx+APPDATA.process]
        mov     [edx+APPDATA.process], eax

        mov     eax, [ebx+APPDATA.dlls_list_ptr]
        mov     [edx+APPDATA.dlls_list_ptr], eax

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

        lea     eax, [app_cmdline]
        stdcall set_app_params , [slot], eax, dword 0, \
                dword 0, [flags]

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

align 4
tls_app_entry:

        call    init_heap
        stdcall user_alloc, 4096

        mov     edx, [current_slot]
        mov     [edx+APPDATA.tls_base], eax
        mov     [tls_data_l+2], ax
        shr     eax, 16
        mov     [tls_data_l+4], al
        mov     [tls_data_l+7], ah
        mov     dx, app_tls
        mov     fs, dx
        popad
        iretd


EFL_IF      equ 0x0200
EFL_IOPL1   equ 0x1000
EFL_IOPL2   equ 0x2000
EFL_IOPL3   equ 0x3000


align 4
proc set_app_params stdcall,slot:dword, params:dword,\
            cmd_line:dword, app_path:dword, flags:dword

       locals
         pl0_stack dd ?
       endl

        stdcall kernel_alloc, RING0_STACK_SIZE+512
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
        mov     ecx, [SLOT_BASE+256+APPDATA.io_map]
        mov     [eax+SLOT_BASE+APPDATA.io_map], ecx
        mov     ecx, [SLOT_BASE+256+APPDATA.io_map+4]
        mov     [eax+SLOT_BASE+APPDATA.io_map+4], ecx

        mov     esi, fpu_data
        mov     ecx, 512/4
        rep movsd

        cmp     ebx, [TASK_COUNT]
        jle     .noinc
        inc     dword [TASK_COUNT]     ;update number of processes
.noinc:
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
        stdcall kernel_alloc, 0x1000
        pop     ebx
        mov     esi, [current_slot]
        mov     esi, [esi+APPDATA.cur_dir]
        mov     ecx, 0x1000/4
        mov     edi, eax
        mov     [ebx+SLOT_BASE+APPDATA.cur_dir], eax
        rep movsd

        shr     ebx, 3
        mov     eax, new_app_base
        mov     dword [CURRENT_TASK+ebx+0x10], eax

.add_command_line:
        mov     edx, [params]
        mov     edx, [edx] ;app_cmdline
        test    edx, edx
        jz      @f     ;application doesn't need parameters

        mov     eax, edx
        add     eax, 256
        jc      @f

;        cmp     eax, [SLOT_BASE+APPDATA.mem_size+ebx*8]
;        ja      @f

        mov     eax, [cmd_line]

        cmp     [edx], dword 0xffffffff ; extended destination tag
        jne     .no_ext_dest

        mov     edx, [edx+4] ; extended destination for cmdline
        jmp     .continue

.no_ext_dest:
        mov     [eax-12], dword 255
.continue:
        mov     byte [edx], 0  ;force empty string if no cmdline given

        test    eax, eax
        jz      @f
;--------------------------------------
        cmp     [eax-4], dword 0xffffffff ; cmdline_flag
        jne     .old_copy

        push    eax
        stdcall strncpy, edx, [eax-8], [eax-12]
        pop     eax

        stdcall kernel_free, [eax-8]
        jmp     @f

.old_copy:
;--------------------------------------
        stdcall strncpy, edx, eax, 256
@@:
        mov     edx, [params]
        mov     edx, [edx+4];app_path
        test    edx, edx
        jz      @F     ;application don't need path of file
        mov     eax, edx
        add     eax, 1024
        jc      @f
;        cmp     eax, [SLOT_BASE+APPDATA.mem_size+ebx*8]
;        ja      @f
        stdcall strncpy, edx, [app_path], 1024
@@:
        mov     ebx, [slot]
        mov     eax, ebx
        shl     ebx, 5
        lea     ecx, [draw_data+ebx];ecx - pointer to draw data

        mov     edx, irq0.return
        cmp     [ebx*8+SLOT_BASE+APPDATA.tls_base], -1
        jne     @F
        mov     edx, tls_app_entry
@@:
; set window state to 'normal' (non-minimized/maximized/rolled-up) state
        mov     [ebx+window_data+WDATA.fl_wstate], WSTATE_NORMAL
        mov     [ebx+window_data+WDATA.fl_redraw], 1
        add     ebx, CURRENT_TASK     ;ebx - pointer to information about process
        mov     [ebx+TASKDATA.wnd_number], al;set window number on screen = process slot

        mov     [ebx+TASKDATA.event_mask], dword 1+2+4;set default event flags (see 40 function)

        inc     dword [process_number]
        mov     eax, [process_number]
        mov     [ebx+4], eax    ;set PID

;set draw data to full screen
        xor     eax, eax
        mov     [ecx+0], dword eax
        mov     [ecx+4], dword eax
        mov     eax, [Screen_Max_X]
        mov     [ecx+8], eax
        mov     eax, [Screen_Max_Y]
        mov     [ecx+12], eax

        mov     ebx, [pl0_stack]
        mov     esi, [params]
        lea     ecx, [ebx+REG_EIP]
        xor     eax, eax

        mov     [ebx+REG_RET], edx
        mov     [ebx+REG_EDI], eax
        mov     [ebx+REG_ESI], eax
        mov     [ebx+REG_EBP], eax
        mov     [ebx+REG_ESP], ecx;ebx+REG_EIP
        mov     [ebx+REG_EBX], eax
        mov     [ebx+REG_EDX], eax
        mov     [ebx+REG_ECX], eax
        mov     [ebx+REG_EAX], eax

        mov     eax, [esi+0x08]             ;app_eip
        mov     [ebx+REG_EIP], eax          ;app_entry
        mov     [ebx+REG_CS], dword app_code
        mov     ecx, USER_PRIORITY

        test    byte [flags], 2
        jz      @F

        mov     [ebx+REG_CS], dword os_code ; kernel thread
        mov     ecx, MAX_PRIORITY

@@:
        mov     [ebx+REG_EFLAGS], dword EFL_IOPL1+EFL_IF

        mov     eax, [esi+0x0C]  ;app_esp
        mov     [ebx+REG_APP_ESP], eax;app_stack
        mov     [ebx+REG_SS], dword app_data

        lea     edx, [ebx+REG_RET]
        mov     ebx, [slot]
        shl     ebx, 5
        mov     [ebx*8+SLOT_BASE+APPDATA.saved_esp], edx

        xor     edx, edx; process state - running
; set if debuggee
        test    byte [flags], 1
        jz      .no_debug
        inc     edx ; process state - suspended
        mov     eax, [CURRENT_TASK]
        mov     [SLOT_BASE+ebx*8+APPDATA.debugger_slot], eax
.no_debug:
        mov     [CURRENT_TASK+ebx+TASKDATA.state], dl
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
