$Revision: 437 $
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

GREEDY_KERNEL  equ 0

struc APP_HEADER_00
{ .banner      dq ?
  .version     dd ?    ;+8
  .start       dd ?    ;+12
  .i_end       dd ?    ;+16
  .mem_size    dd ?    ;+20
  .i_param     dd ?    ;+24
}

struc APP_HEADER_01
{ .banner      dq ?
  .version     dd ?    ;+8
  .start       dd ?    ;+12
  .i_end       dd ?    ;+16
  .mem_size    dd ?    ;+20
  .stack_top   dd ?    ;+24
  .i_param     dd ?    ;+28
  .i_icon      dd ?    ;+32
}

struc TSS
{
  ._back   rw 2
  ._esp0   rd 1
  ._ss0    rw 2
  ._esp1   rd 1
  ._ss1    rw 2
  ._esp2   rd 1
  ._ss2    rw 2
  ._cr3    rd 1
  ._eip    rd 1
  ._eflags rd 1
  ._eax    rd 1
  ._ecx    rd 1
  ._edx    rd 1
  ._ebx    rd 1
  ._esp    rd 1
  ._ebp    rd 1
  ._esi    rd 1
  ._edi    rd 1
  ._es     rw 2
  ._cs     rw 2
  ._ss     rw 2
  ._ds     rw 2
  ._fs     rw 2
  ._gs     rw 2
  ._ldt    rw 2
  ._trap   rw 1
  ._io     rw 1
}

virtual at 0
  TSS  TSS
end virtual

struc APP_PARAMS
{ .app_cmdline   ;0x00
  .app_path      ;0x04
  .app_eip       ;0x08
  .app_esp       ;0x0C
  .app_mem       ;0x10
}

macro _clear_ op
{  mov ecx, op/4
   xor eax, eax
   cld
   rep stosd
}


align 4
proc fs_execute

;fn_read:dword, file_size:dword, cluster:dword

; ebx - cmdline
; edx - flags
; ebp - full filename
; [esp+4] = procedure DoRead, [esp+8] = filesize & [esp+12]... - arguments for it

           locals
             cmdline       rd 64        ;256/4
             filename      rd 256       ;1024/4
             flags         dd ?

             save_cr3      dd ?
             slot          dd ?
             slot_base     dd ?
             file_base     dd ?
             file_size     dd ?
                                          ;app header data
             hdr_cmdline   dd ? ;0x00
             hdr_path      dd ? ;0x04
             hdr_eip       dd ? ;0x08
             hdr_esp       dd ? ;0x0C
             hdr_mem       dd ? ;0x10
             hdr_i_end     dd ? ;0x14
           endl

           pushad

           mov [cmdline], ebx
           mov [flags], edx

; [ebp]  pointer to filename

           lea eax, [filename]
           mov dword [eax+1020],0              ;force terminate
                                               ;string
           stdcall k_strncpy, eax, [ebp], 1023

           lea eax, [cmdline]
           mov dword [eax+252], 0
           stdcall k_strncpy, eax, [cmdline], 255

           lea eax, [filename]
           stdcall load_file, eax
           mov  ecx, -ERROR_FILE_NOT_FOUND
           test eax, eax
           jz .err_file

           mov [file_base], eax
           mov [file_size], ebx

           lea ebx, [hdr_cmdline]
           call test_app_header
           mov ecx, -0x1F
           test eax, eax
           jz .err_hdr

           mov esi, new_process_loading
           call sys_msg_board_str       ; write message to message board

.wait_lock:
           cmp [application_table_status],0
           je .get_lock
           call   change_task
           jmp .wait_lock

.get_lock:
           mov eax, 1
           xchg eax, [application_table_status]
           cmp eax, 0
           jne .wait_lock

;           pushfd
;           cli

           call set_application_table_status

           call get_new_process_place
           test eax, eax
           mov ecx, -0x20      ; too many processes
           jz .err

           mov [slot], eax
           shl eax, 8
           add eax, SLOT_BASE
           mov [slot_base], eax
           mov edi, eax
           _clear_ 256     ;clean extended information about process

; write application name
           lea edi, [filename]
           mov al, '/'
           call k_strrchr  ; now eax points to name without path

           lea esi, [eax+1]
           test eax, eax
           jnz @F
           lea esi, [filename]
@@:
           mov ecx, 8  ; 8 chars for name
           mov edi, [slot_base]
.copy_process_name_loop:
           lodsb
           cmp al, '.'
           jz .copy_process_name_done
           test al, al
           jz .copy_process_name_done
           stosb
           loop .copy_process_name_loop
.copy_process_name_done:

           mov ebx, cr3
           mov [save_cr3], ebx

           stdcall create_app_space,[hdr_mem],[file_base],[file_size]
           mov ecx, -30  ; no memory
           test eax, eax
           jz .failed

           mov   ebx,[slot_base]
           mov   [ebx+APPDATA.dir_table],eax
           mov   eax,[hdr_mem]
           mov   [ebx+APPDATA.mem_size],eax

if GREEDY_KERNEL
else
           mov ecx, [hdr_mem]
           mov edi, [file_size]
           add edi, 4095
           and edi, not 4095
           sub ecx, edi
           jna @F

           xor eax, eax
           add edi, new_app_base
           cld
           rep stosb
@@:
end if

; release only virtual space, not phisical memory

           stdcall free_kernel_space, [file_base]
           lea eax, [hdr_cmdline]
           lea ebx, [cmdline]
           lea ecx, [filename]
           stdcall set_app_params ,[slot],eax,ebx,ecx,[flags]

           mov eax, [save_cr3]
           call set_cr3

           xor ebx, ebx
           mov [application_table_status],ebx ;unlock application_table_status mutex
           mov eax,[process_number]  ;set result
           ret
.failed:
           mov eax, [save_cr3]
           call set_cr3
.err:
.err_hdr:
           stdcall kernel_free,[file_base]
.err_file:
           xor eax, eax
           mov [application_table_status],eax
           mov eax, ecx
           ret
endp

align 4
test_app_header:
           virtual at eax
             APP_HEADER_00 APP_HEADER_00
           end virtual
           virtual at eax
             APP_HEADER_01 APP_HEADER_01
           end virtual

           cmp dword [eax], 'MENU'
           jne .fail
           cmp word [eax+4],'ET'
           jne .fail

           cmp [eax+6], word '00'
           jne  .check_01_header

           mov  ecx,[APP_HEADER_00.start]
           mov  [ebx+0x08], ecx                ;app_eip
           mov  edx,[APP_HEADER_00.mem_size]
           mov  [ebx+0x10], edx                ;app_mem
           shr  edx,1
           sub  edx,0x10
           mov  [ebx+0x0C], edx                ;app_esp
           mov  ecx,[APP_HEADER_00.i_param]
           mov  [ebx], ecx                     ;app_cmdline
           mov  [ebx+4], dword 0               ;app_path
           mov  edx, [APP_HEADER_00.i_end]
           mov  [ebx+0x14], edx
           ret

 .check_01_header:

           cmp  [eax+6],word '01'
           jne  .fail

           mov  ecx,[APP_HEADER_01.start]
           mov  [ebx+0x08], ecx                ;app_eip
           mov  edx,[APP_HEADER_01.mem_size]

; \begin{diamond}[20.08.2006]
; sanity check (functions 19,58 load app_i_end bytes and that must
; fit in allocated memory to prevent kernel faults)
           cmp  edx,[APP_HEADER_01.i_end]
           jb   .fail
; \end{diamond}[20.08.2006]

           mov  [ebx+0x10], edx                ;app_mem
           mov  ecx,[APP_HEADER_01.stack_top]
           mov  [ebx+0x0C], ecx                ;app_esp
           mov  edx,[APP_HEADER_01.i_param]
           mov  [ebx], edx                     ;app_cmdline
           mov  ecx,[APP_HEADER_01.i_icon]
           mov  [ebx+4], ecx                   ;app_path
           mov  edx, [APP_HEADER_01.i_end]
           mov  [ebx+0x14], edx
           ret
.fail:
           xor eax, eax
           ret

align 4
proc get_new_process_place
;input:
;  none
;result:
;  eax=[new_process_place]<>0 - ok
;      0 - failed.
;This function find least empty slot.
;It doesn't increase [TASK_COUNT]!
           mov    eax,CURRENT_TASK
           mov    ebx,[TASK_COUNT]
           inc    ebx
           shl    ebx,5
           add    ebx,eax               ;ebx - address of process information for (last+1) slot
.newprocessplace:
;eax = address of process information for current slot
           cmp    eax,ebx
           jz     .endnewprocessplace   ;empty slot after high boundary
           add    eax,0x20
           cmp    word [eax+0xa],9      ;check process state, 9 means that process slot is empty
           jnz    .newprocessplace
.endnewprocessplace:
           mov    ebx,eax
           sub    eax,CURRENT_TASK
           shr    eax,5                 ;calculate slot index
           cmp    eax,256
           jge    .failed               ;it should be <256
           mov    word [ebx+0xa],9      ;set process state to 9 (for slot after hight boundary)
           ret
.failed:
           xor    eax,eax
           ret
endp

align 4
proc create_app_space stdcall, app_size:dword,img_base:dword,img_size:dword
           locals
             app_pages   dd ?
             img_pages   dd ?
             dir_addr    dd ?
             app_tabs    dd ?
           endl

           mov ebx, pg_data.pg_mutex
           call wait_mutex   ;ebx

           xor eax, eax
           mov [dir_addr], eax

           mov eax, [app_size]
           add eax, 4095
           and eax, NOT(4095)
           mov [app_size], eax
           mov ebx, eax
           shr eax, 12
           mov [app_pages], eax

           add ebx, 0x3FFFFF
           and ebx, NOT(0x3FFFFF)
           shr ebx, 22
           mov [app_tabs], ebx

           mov ecx, [img_size]
           add ecx, 4095
           and ecx, NOT(4095)

           mov [img_size], ecx
           shr ecx, 12
           mov [img_pages], ecx

     if GREEDY_KERNEL
           lea eax, [ecx+ebx+2]    ;only image size
     else
           lea eax, [eax+ebx+2]    ;all requested memory
     end if
           cmp eax, [pg_data.pages_free]
           ja .fail

           call alloc_page
           test eax, eax
           jz .fail
           mov [dir_addr], eax
           stdcall map_page,[tmp_task_pdir],eax,dword PG_SW

           mov esi, sys_pgdir
           mov edi, [tmp_task_pdir]
           mov ecx, (page_tabs shr 20)/4
           cld
           rep movsd

           mov eax, [dir_addr]
           or eax, PG_SW
           stosd                 ; [(page_tabs shr 20)]= eax

           mov ecx, 0x800/4
           xor eax, eax
           rep stosd

           mov eax, [dir_addr]
           call set_cr3

           mov edx, [app_tabs]
           mov edi, new_app_base
@@:
           call alloc_page
           test eax, eax
           jz .fail

           stdcall map_page_table, edi, eax
           add edi, 0x00400000
           dec edx
           jnz @B

           mov edi, new_app_base
           shr edi, 10
           add edi, page_tabs

           mov ecx, [app_tabs]
           shl ecx, 10
           xor eax, eax
           rep stosd

           mov ecx, [img_pages]
           mov ebx, PG_UW
           mov edx, new_app_base
           mov esi, [img_base]
           mov edi, new_app_base
           shr esi, 10
           shr edi, 10
           add esi, page_tabs
           add edi, page_tabs
.remap:
           lodsd
           or eax, ebx      ; force user level r/w access
           stosd
           add edx, 0x1000
           dec [app_pages]
           dec ecx
           jnz .remap

           mov ecx, [app_pages]
           test ecx, ecx
           jz .done

if GREEDY_KERNEL
           mov eax, 0x02
.reserve:
           stosd
           invlpg [edx]
           add edx, 4096
           dec ecx
           jnz .reserve
else

.alloc:
           call alloc_page
           test eax, eax
           jz .fail

           stdcall map_page,edx,eax,dword PG_UW
           add edx, 0x1000
           dec [app_pages]
           jnz .alloc
end if

.done:
           stdcall map_page,[tmp_task_pdir],dword 0,dword PG_UNMAP

           dec [pg_data.pg_mutex]
           mov eax, [dir_addr]
           ret
.fail:
           dec [pg_data.pg_mutex]
           cmp [dir_addr], 0
           je @f
           stdcall destroy_app_space, [dir_addr]
@@:
           xor eax, eax
           ret
endp

align 4
set_cr3:
           mov esi, [CURRENT_TASK]
           mov ebx, esi
           shl esi,8
           mov [SLOT_BASE+esi+0xB8],eax
           imul   ebx,tss_step
           add    ebx,tss_data
           mov [ebx+28], eax
           mov cr3, eax
           ret

align 4
proc destroy_page_table stdcall, pg_tab:dword

           push esi

           mov esi, [pg_tab]
           mov ecx, 1024
.free:
           mov eax, [esi]
           test eax, 1
           jz .next
           call free_page
.next:
           add esi, 4
           dec ecx
           jnz .free
           pop esi
           ret
endp

align 4
proc destroy_app_space stdcall, pg_dir:dword

           mov ebx, pg_data.pg_mutex
           call wait_mutex   ;ebx

           xor   edx,edx
           mov   eax,0x2
           mov ebx, [pg_dir]
.loop:
;eax = current slot of process
           mov   ecx,eax
           shl   ecx,5
           cmp   byte [CURRENT_TASK+ecx+0xa],9  ;if process running?
           jz    @f                              ;skip empty slots
           shl   ecx,3
           cmp   [SLOT_BASE+ecx+0xB8],ebx       ;compare page directory addresses
           jnz   @f
           inc   edx                            ;thread found
@@:
           inc   eax
           cmp   eax,[TASK_COUNT]               ;exit loop if we look through all processes
           jle   .loop

;edx = number of threads
;our process is zombi so it isn't counted
           cmp   edx,1
           jg    .exit
;if there isn't threads then clear memory.

           mov eax, [pg_dir]
           and eax, not 0xFFF
           stdcall map_page,[tmp_task_pdir],eax,dword PG_SW
           mov esi, [tmp_task_pdir]
           add esi, 0x800
           mov edi, 0x800/4
.destroy:
           mov eax, [esi]
           test eax, 1
           jz .next
           and eax, not 0xFFF
           stdcall map_page,[tmp_task_ptab],eax,dword PG_SW
           stdcall destroy_page_table, [tmp_task_ptab]
           mov eax, [esi]
           call free_page
.next:
           add esi, 4
           dec edi
           jnz .destroy

           mov eax, [pg_dir]
           call free_page
.exit:
           stdcall map_page,[tmp_task_ptab],dword 0,dword PG_UNMAP
           stdcall map_page,[tmp_task_pdir],dword 0,dword PG_UNMAP
           dec [pg_data.pg_mutex]
           ret
endp

pid_to_slot:
;Input:
;  eax - pid of process
;Output:
;  eax - slot of process or 0 if process don't exists
;Search process by PID.
    push   ebx
    push   ecx
    mov    ebx,[TASK_COUNT]
    shl    ebx,5
    mov    ecx,2*32

.loop:
;ecx=offset of current process info entry
;ebx=maximum permitted offset
    cmp    byte [CURRENT_TASK+ecx+0xa],9
    jz     .endloop              ;skip empty slots
    cmp    [CURRENT_TASK+ecx+0x4],eax ;check PID
    jz     .pid_found
.endloop:
    add    ecx,32
    cmp    ecx,ebx
    jle    .loop

    pop    ecx
    pop    ebx
    xor    eax,eax
    ret

.pid_found:
    shr    ecx,5
    mov    eax,ecx               ;convert offset to index of slot
    pop    ecx
    pop    ebx
    ret

check_region:
;input:
;  ebx - start of buffer
;  ecx - size of buffer
;result:
;  eax = 1 region lays in app memory
;  eax = 0 region don't lays in app memory
     mov  eax,[CURRENT_TASK]
     jmp  check_process_region
;-----------------------------------------------------------------------------
check_process_region:
;input:
;  eax - slot
;  ebx - start of buffer
;  ecx - size of buffer
;result:
;  eax = 1 region lays in app memory
;  eax = 0 region don't lays in app memory

     test ecx,ecx
     jle  .ok
     shl  eax,5
     cmp  word [CURRENT_TASK+eax+0xa],0
     jnz  .failed
     shl  eax,3
     mov  eax,[SLOT_BASE+eax+0xb8]
     test eax,eax
     jz   .failed

     mov  eax,1
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
    mov  eax,1
    ret
;
;.failed1:
;    pop  edx
;    pop  ecx
;    pop  ebx
.failed:
    xor  eax,eax
    ret

align 4
proc read_process_memory
;Input:
;  eax - process slot
;  ebx - buffer address
;  ecx - buffer size
;  edx - start address in other process
;Output:
;  eax - number of bytes read.
           locals
             slot       dd ?
             buff       dd ?
             r_count    dd ?
             offset     dd ?
             tmp_r_cnt  dd ?
           endl

           mov [slot], eax
           mov [buff], ebx
           mov [r_count], ecx
           mov [tmp_r_cnt], ecx
           mov [offset], edx

           pushad
.read_mem:
           mov edx, [offset]
           mov ebx, [tmp_r_cnt]

           mov ecx, 0x400000
           and edx, 0x3FFFFF
           sub ecx, edx
           cmp ecx, ebx
           jbe @f
           mov ecx, ebx
@@:
           cmp ecx, 0x8000
           jna @F
           mov ecx, 0x8000
@@:
           mov eax, [slot]
           shl  eax,8
           mov ebx, [offset]
           add ebx, new_app_base
           push ecx
           stdcall map_memEx, [proc_mem_map],\
                              [SLOT_BASE+eax+0xB8],\
                              ebx, ecx
           pop ecx

           mov esi, [offset]
           and esi, 0xfff
           add esi, [proc_mem_map]
           mov edi, [buff]
           mov edx, ecx
           rep movsb

           add [offset], edx
           sub [tmp_r_cnt], edx
           jnz .read_mem

           popad
           mov eax, [r_count]
           ret
endp

align 4
proc write_process_memory
;Input:
;  eax - process slot
;  ebx - buffer address
;  ecx - buffer size
;  edx - start address in other process
;Output:
;  eax - number of bytes written

           locals
             slot       dd ?
             buff       dd ?
             w_count    dd ?
             offset     dd ?
             tmp_w_cnt  dd ?
           endl

           mov [slot], eax
           mov [buff], ebx
           mov [w_count], ecx
           mov [tmp_w_cnt], ecx
           mov [offset], edx

           pushad
.read_mem:
           mov edx, [offset]
           mov ebx, [tmp_w_cnt]

           mov ecx, 0x400000
           and edx, 0x3FFFFF
           sub ecx, edx
           cmp ecx, ebx
           jbe @f
           mov ecx, ebx
@@:
           cmp ecx, 0x8000
           jna @F
           mov ecx, 0x8000
@@:
           mov eax, [slot]
           shl  eax,8
           mov ebx, [offset]
           add ebx, new_app_base
           push ecx
           stdcall map_memEx, [proc_mem_map],\
                              [SLOT_BASE+eax+0xB8],\
                              ebx, ecx
           pop ecx

           mov edi, [offset]
           and edi, 0xfff
           add edi, [proc_mem_map]
           mov esi, [buff]
           mov edx, ecx
           rep movsb

           add [offset], edx
           sub [tmp_w_cnt], edx
           jnz .read_mem

           popad
           mov eax, [w_count]
           ret
endp

align 4
proc new_sys_threads
           locals
             slot          dd ?
             app_cmdline   dd ? ;0x00
             app_path      dd ? ;0x04
             app_eip       dd ? ;0x08
             app_esp       dd ? ;0x0C
             app_mem       dd ? ;0x10
           endl

           cmp eax,1
           jne .failed                  ;other subfunctions

           xor  eax,eax
           mov [app_cmdline], eax
           mov [app_path], eax
           mov [app_eip], ebx
           mov [app_esp], ecx

           mov    esi,new_process_loading
           call   sys_msg_board_str
.wait_lock:
           cmp [application_table_status],0
           je .get_lock
           call   change_task
           jmp .wait_lock

.get_lock:
           mov eax, 1
           xchg eax, [application_table_status]
           cmp eax, 0
           jne .wait_lock

           call   set_application_table_status

           call get_new_process_place
           test eax, eax
           jz .failed

           mov [slot], eax

           mov    esi,[CURRENT_TASK]
           shl    esi,8
           add    esi,SLOT_BASE
           mov    ebx,esi             ;ebx=esi - pointer to extended information about current thread

           mov    edi, eax
           shl    edi,8
           add    edi,SLOT_BASE
           mov    edx,edi             ;edx=edi - pointer to extended infomation about new thread
           mov    ecx,256/4
           xor eax, eax
           cld
           rep    stosd               ;clean extended information about new thread
           mov    esi,ebx
           mov    edi,edx
           mov    ecx,11
           rep    movsb               ;copy process name

           mov eax,[ebx+APPDATA.heap_base]
           mov [edx+APPDATA.heap_base], eax

           mov ecx,[ebx+APPDATA.heap_top]
           mov [edx+APPDATA.heap_top], ecx

           mov eax,[ebx+APPDATA.mem_size]
           mov [edx+APPDATA.mem_size], eax

           mov ecx,[ebx+APPDATA.dir_table]
           mov [edx+APPDATA.dir_table],ecx      ;copy page directory

           lea eax, [app_cmdline]
           stdcall set_app_params ,[slot],eax,dword 0,\
                                         dword 0,dword 0

           mov    esi,new_process_running
           call   sys_msg_board_str     ;output information about succefull startup

           mov    [application_table_status],0 ;unlock application_table_status mutex
           mov    eax,[process_number]  ;set result
           ret
.failed:
           mov    [application_table_status],0
           mov    eax,-1
           ret
endp

; param
;  ebx=mutex

align 4
wait_mutex:
           push eax
           push ebx
.do_wait:
           cmp dword [ebx],0
           je .get_lock
           call change_task
           jmp .do_wait
.get_lock:
           mov eax, 1
           xchg eax, [ebx]
           test eax, eax
           jnz .do_wait
           pop ebx
           pop eax
           ret

align 4
proc set_app_params stdcall,slot:dword, params:dword,\
                        cmd_line:dword, app_path:dword, flags:dword

           locals
             pl0_stack dd ?
           endl

           stdcall kernel_alloc, RING0_STACK_SIZE+512
           mov [pl0_stack], eax

           lea edi, [eax+RING0_STACK_SIZE]

           mov eax, [slot]
           mov ebx, eax

           shl eax, 8
           mov [eax+SLOT_BASE+APPDATA.fpu_state], edi
           mov [eax+SLOT_BASE+APPDATA.fpu_handler], 0
           mov [eax+SLOT_BASE+APPDATA.sse_handler], 0

           mov esi, fpu_data
           mov ecx, 512/4
           cld
           rep movsd

           cmp    ebx,[TASK_COUNT]
           jle    .noinc
           inc    dword [TASK_COUNT]       ;update number of processes
.noinc:
           shl ebx,8
           lea edx, [ebx+SLOT_BASE+APP_EV_OFFSET]
           mov [SLOT_BASE+APPDATA.fd_ev+ebx],edx
           mov [SLOT_BASE+APPDATA.bk_ev+ebx],edx

           add edx, APP_OBJ_OFFSET-APP_EV_OFFSET
           mov [SLOT_BASE+APPDATA.fd_obj+ebx],edx
           mov [SLOT_BASE+APPDATA.bk_obj+ebx],edx

           mov ecx, [def_cursor]
           mov [SLOT_BASE+APPDATA.cursor+ebx],ecx
           mov eax, [pl0_stack]
           mov [SLOT_BASE+APPDATA.pl0_stack+ebx],eax

           shr ebx,3
           mov eax, new_app_base
           mov dword [CURRENT_TASK+ebx+0x10],eax

.add_command_line:
           mov edx,[params]
           mov edx,[edx]           ;app_cmdline
           test edx,edx
           jz @F                   ;application don't need parameters

           mov     eax, edx
           add     eax, 256
           jc      @f

           cmp     eax, [SLOT_BASE+APPDATA.mem_size+ebx*8]
           ja      @f

           add edx, new_app_base
           stdcall k_strncpy, edx, [cmd_line], 256
@@:
           mov edx,[params]
           mov edx, [edx+4]        ;app_path
           test edx,edx
           jz @F                   ;application don't need path of file
        mov     eax, edx
        add     eax, 1024
        jc      @f
        cmp     eax, [SLOT_BASE+APPDATA.mem_size+ebx*8]
        ja      @f
           add edx, new_app_base
           stdcall k_strncpy, edx, [app_path], 1024
@@:
           mov    ebx,[slot]
           mov    eax,ebx
           shl    ebx,5
; set window state to 'normal' (non-minimized/maximized/rolled-up) state
           mov     [ebx+window_data+WDATA.fl_wstate], WSTATE_NORMAL
           mov     [ebx+window_data+WDATA.fl_redraw], 1
           add    ebx,CURRENT_TASK            ;ebx - pointer to information about process
           mov    [ebx+TASKDATA.wnd_number],al;set window number on screen = process slot

           mov    [ebx+TASKDATA.event_mask],dword 1+2+4 ;set default event flags (see 40 function)

           inc    dword [process_number]
           mov    eax,[process_number]
           mov    [ebx+4],eax           ;set PID

           mov    ecx,ebx
           add    ecx,(draw_data-CURRENT_TASK)  ;ecx - pointer to draw data
;set draw data to full screen

           mov    [ecx+0],dword 0
           mov    [ecx+4],dword 0
           mov    eax,[ScreenWidth]
           mov    [ecx+8],eax
           mov    eax,[ScreenHeight]
           mov    [ecx+12],eax

           mov edi,[slot]
           imul edi,tss_step
           add edi,tss_data
           mov ecx,128/4
           xor eax, eax
           cld
           rep stosd
;Add IO access table - bit array of permitted ports
           not eax
           mov ecx,2048
           rep stosd                 ; access to 4096*8=65536 ports
           sub edi, tss_step

;set cr3 register in TSS of application
           mov ecx, [slot]
           shl ecx, 8
           mov eax,[SLOT_BASE+ecx+APPDATA.dir_table]
           mov [edi+TSS._cr3],eax

           mov esi,[params]
           mov eax, [esi+0x08]       ;app_eip
           mov [edi+TSS._eip],eax    ;set eip in TSS
           mov eax, [esi+0x0C]       ;app_esp
           mov [edi+TSS._esp],eax    ;set stack in TSS
           mov [edi+TSS._eflags],dword 0x1202

           mov [edi+TSS._cs],app_code  ;selector of code segment
           mov [edi+TSS._ss],app_data
           mov [edi+TSS._ds],app_data
           mov [edi+TSS._es],app_data
           mov [edi+TSS._fs],app_data
           mov [edi+TSS._gs],graph_data ;selector of graphic segment
           mov [edi+TSS._io],word 128
           mov [edi+TSS._ss0], os_data
           mov ebx, [pl0_stack]
           add ebx, RING0_STACK_SIZE
           mov [edi+TSS._esp0],ebx

           mov ecx, edi    ;ecx - address of application TSS
           mov ebx,[slot]
           shl ebx,3
;set TSS descriptor
           mov [ebx+gdts+tss0+0],word tss_step ;limit (size)
           mov [ebx+gdts+tss0+2],cx  ;part of offset
           shr ecx,16
           mov [ebx+gdts+tss0+4],cl  ;part of offset
           mov [ebx+gdts+tss0+7],ch  ;part of offset
           mov [ebx+gdts+tss0+5],word 01010000b*256+11101001b ;system flags

;flush keyboard and buttons queue
           mov    [KEY_COUNT],byte 0
           mov    [BTN_COUNT],byte 0

           mov    edi,[slot]
           shl    edi,5
           add    edi,window_data
           mov    ebx,[slot]
           movzx  esi,word [WIN_STACK+ebx*2]
           lea    esi,[WIN_POS+esi*2]
           call   windowactivate        ;gui initialization

           mov    ebx,[slot]
           shl    ebx,5
           mov    [CURRENT_TASK+ebx+0xa],byte 0 ;set process state - running
; set if debuggee
           mov eax, [flags]
           test byte [flags], 1
           jz   .no_debug
           mov  [CURRENT_TASK+ebx+0xa],byte 1 ;set process state - suspended
           mov  eax,[CURRENT_TASK]
           mov  [SLOT_BASE+ebx*8+0xac],eax ;set debugger PID - current
.no_debug:
           mov    esi,new_process_running
           call   sys_msg_board_str     ;output information about succefull startup
           ret
endp



include "debug.inc"

iglobal
    new_process_loading db 'K : New Process - loading',13,10,0
    new_process_running db 'K : New Process - done',13,10,0
    start_not_enough_memory db 'K : New Process - not enough memory',13,10,0
endg

