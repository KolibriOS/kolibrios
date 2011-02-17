;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


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


struc APP_PARAMS
{ .app_cmdline   ;0x00
  .app_path  ;0x04
  .app_eip   ;0x08
  .app_esp   ;0x0C
  .app_mem   ;0x10
}

macro _clear_ op
{  mov ecx, op/4
   xor eax, eax
   cld
   rep stosd
}

fs_execute_from_sysdir:
    xor ebx, ebx
    xor edx, edx
    mov esi, sysdir_path

align 4
proc fs_execute

;fn_read:dword, file_size:dword, cluster:dword

; ebx - cmdline
; edx - flags
; ebp - full filename
; [esp+4] = procedure DoRead, [esp+8] = filesize & [esp+12]... - arguments for it

       locals
         cmdline       rd 64    ;256/4
         filename      rd 256   ;1024/4
         flags     dd ?

         save_cr3      dd ?
         slot      dd ?
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

       mov [flags], edx

; [ebp]  pointer to filename

       lea edi, [filename]
       lea ecx, [edi+1024]
       mov al, '/'
       stosb
@@:
       cmp edi, ecx
       jae .bigfilename
       lodsb
       stosb
       test al, al
       jnz @b
       mov esi, [ebp]
       test esi, esi
       jz .namecopied
       mov byte [edi-1], '/'
@@:
       cmp edi, ecx
       jae .bigfilename
       lodsb
       stosb
       test al, al
       jnz @b
       jmp .namecopied
.bigfilename:
       popad
       mov eax, -ERROR_FILE_NOT_FOUND
       ret

.namecopied:

       mov [cmdline], ebx
       test ebx, ebx
       jz @F

       lea eax, [cmdline]
       mov dword [eax+252], 0
       stdcall strncpy, eax, ebx, 255
@@:
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

       ;mov esi, new_process_loading
       ;call sys_msg_board_str       ; write message to message board

.wait_lock:
       cmp [application_table_status],0
       je .get_lock
       call   change_task
       jmp .wait_lock

.get_lock:
       mov eax, 1
       xchg eax, [application_table_status]
       test eax, eax
       jnz .wait_lock

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
       lea eax, [filename]
       stdcall strrchr,  eax, '/'  ; now eax points to name without path

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

       xor edx, edx
       cmp word [6], '02'
       jne @f

       not edx
@@:
       mov   [ebx+APPDATA.tls_base],edx

if GREEDY_KERNEL
else
       mov ecx, [hdr_mem]
       mov edi, [file_size]
       add edi, 4095
       and edi, not 4095
       sub ecx, edi
       jna @F

       xor eax, eax
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
       mov  [ebx+0x08], ecx                 ;app_eip
       mov  edx,[APP_HEADER_00.mem_size]
       mov  [ebx+0x10], edx                 ;app_mem
       shr  edx,1
       sub  edx,0x10
       mov  [ebx+0x0C], edx                 ;app_esp
       mov  ecx,[APP_HEADER_00.i_param]
       mov  [ebx], ecx                      ;app_cmdline
       mov  [ebx+4], dword 0                ;app_path
       mov  edx, [APP_HEADER_00.i_end]
       mov  [ebx+0x14], edx
       ret

 .check_01_header:

       cmp [eax+6], word '01'
       je @f
       cmp [eax+6], word '02'
       jne .fail
@@:
       mov  ecx,[APP_HEADER_01.start]
       mov  [ebx+0x08], ecx                 ;app_eip
       mov  edx,[APP_HEADER_01.mem_size]

; \begin{diamond}[20.08.2006]
; sanity check (functions 19,58 load app_i_end bytes and that must
; fit in allocated memory to prevent kernel faults)
       cmp  edx,[APP_HEADER_01.i_end]
       jb   .fail
; \end{diamond}[20.08.2006]

       mov  [ebx+0x10], edx                 ;app_mem
       mov  ecx,[APP_HEADER_01.stack_top]
       mov  [ebx+0x0C], ecx                 ;app_esp
       mov  edx,[APP_HEADER_01.i_param]
       mov  [ebx], edx                      ;app_cmdline
       mov  ecx,[APP_HEADER_01.i_icon]
       mov  [ebx+4], ecx                    ;app_path
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
       add    ebx,eax       ;ebx - address of process information for (last+1) slot
.newprocessplace:
;eax = address of process information for current slot
       cmp    eax,ebx
       jz     .endnewprocessplace   ;empty slot after high boundary
       add    eax,0x20
       cmp    word [eax+0xa],9  ;check process state, 9 means that process slot is empty
       jnz    .newprocessplace
.endnewprocessplace:
       mov    ebx,eax
       sub    eax,CURRENT_TASK
       shr    eax,5         ;calculate slot index
       cmp    eax,256
       jge    .failed       ;it should be <256
       mov    word [ebx+0xa],9  ;set process state to 9 (for slot after hight boundary)
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

       mov edi, [tmp_task_pdir]
       mov ecx, (OS_BASE shr 20)/4
       xor eax, eax
       cld
       rep stosd

       mov ecx, (OS_BASE shr 20)/4
       mov esi, sys_pgdir+(OS_BASE shr 20)
       rep movsd

       mov eax, [dir_addr]
       or eax, PG_SW
       mov [edi-4096+(page_tabs shr 20)], eax

       and eax, -4096
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
       rep stosd
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
       stdcall destroy_app_space, [dir_addr], 0
@@:
       xor eax, eax
       ret
endp

align 4
set_cr3:

       mov ebx, [current_slot]
       mov [ebx+APPDATA.dir_table], eax
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
           test eax, 1 shl 9
           jnz .next                      ;skip shared pages
       call free_page
.next:
       add esi, 4
       dec ecx
       jnz .free
       pop esi
       ret
endp

align 4
proc destroy_app_space stdcall, pg_dir:dword, dlls_list:dword

       xor   edx,edx
       push  edx
       mov   eax,0x2
       mov ebx, [pg_dir]
.loop:
;eax = current slot of process
       mov   ecx,eax
       shl   ecx,5
       cmp   byte [CURRENT_TASK+ecx+0xa],9  ;if process running?
       jz    @f              ;skip empty slots
       shl   ecx,3
       add   ecx,SLOT_BASE
       cmp   [ecx+APPDATA.dir_table],ebx   ;compare page directory addresses
       jnz   @f
       mov   [ebp-4],ecx
       inc   edx                ;thread found
@@:
       inc   eax
       cmp   eax,[TASK_COUNT]       ;exit loop if we look through all processes
       jle   .loop

;edx = number of threads
;our process is zombi so it isn't counted
       pop   ecx
       cmp   edx,1
       jg    .ret
;if there isn't threads then clear memory.
       mov esi, [dlls_list]
       call destroy_all_hdlls

       mov ebx, pg_data.pg_mutex
       call wait_mutex   ;ebx

       mov eax, [pg_dir]
       and eax, not 0xFFF
       stdcall map_page,[tmp_task_pdir],eax,PG_SW
       mov esi, [tmp_task_pdir]
       mov edi, (OS_BASE shr 20)/4
.destroy:
       mov eax, [esi]
       test eax, 1
       jz .next
       and eax, not 0xFFF
       stdcall map_page,[tmp_task_ptab],eax,PG_SW
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
       stdcall map_page,[tmp_task_ptab],0,PG_UNMAP
       stdcall map_page,[tmp_task_pdir],0,PG_UNMAP
       dec [pg_data.pg_mutex]
.ret:
       ret
endp

align 4
get_pid:
       mov eax, [TASK_BASE]
       mov eax, [eax+TASKDATA.pid]
       ret

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
    jz     .endloop      ;skip empty slots
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
    mov    eax,ecx       ;convert offset to index of slot
    pop    ecx
    pop    ebx
    ret

check_region:
;input:
;  esi - start of buffer
;  edx - size of buffer
;result:
;  eax = 1 region lays in app memory
;  eax = 0 region don't lays in app memory
     mov  eax,[CURRENT_TASK]
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

     test edx,edx
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

       mov [slot], eax
       mov [buff], ecx
       and [r_count], 0
       mov [tmp_r_cnt], edx
       mov [offset], esi

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
       mov ebx, [offset]

       push ecx
       stdcall map_memEx, [proc_mem_map],\
                  [slot], ebx, ecx, PG_MAP
       pop ecx

       mov esi, [offset]
       and esi, 0xfff
       sub eax, esi
       jbe .ret
       cmp ecx, eax
       jbe @f
       mov ecx, eax
       mov [tmp_r_cnt], eax
@@:
       add esi, [proc_mem_map]
       mov edi, [buff]
       mov edx, ecx
       rep movsb
       add [r_count], edx

       add [offset], edx
       sub [tmp_r_cnt], edx
       jnz .read_mem
.ret:
       popad
       mov eax, [r_count]
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

       mov [slot], eax
       mov [buff], ecx
       and [w_count], 0
       mov [tmp_w_cnt], edx
       mov [offset], esi

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
       mov ebx, [offset]
      ;     add ebx, new_app_base
       push ecx
       stdcall map_memEx, [proc_mem_map],\
                  [slot], ebx, ecx, PG_SW
       pop ecx

       mov edi, [offset]
       and edi, 0xfff
       sub eax, edi
       jbe .ret
       cmp ecx, eax
       jbe @f
       mov ecx, eax
       mov [tmp_w_cnt], eax
@@:
       add edi, [proc_mem_map]
       mov esi, [buff]
       mov edx, ecx
       rep movsb

       add [w_count], edx
       add [offset], edx
       sub [tmp_w_cnt], edx
       jnz .read_mem
.ret:
       popad
       mov eax, [w_count]
       ret
endp

align 4
proc new_sys_threads
       locals
         slot      dd ?
         app_cmdline   dd ? ;0x00
         app_path      dd ? ;0x04
         app_eip       dd ? ;0x08
         app_esp       dd ? ;0x0C
         app_mem       dd ? ;0x10
       endl

       cmp ebx,1
       jne .failed          ;other subfunctions

       xor  eax,eax
       mov [app_eip], ecx
       mov [app_cmdline], eax
       mov [app_esp], edx
       mov [app_path], eax
       ;mov    esi,new_process_loading
       ;call   sys_msg_board_str
.wait_lock:
       cmp [application_table_status],0
       je .get_lock
       call   change_task
       jmp .wait_lock

.get_lock:
       mov eax, 1
       xchg eax, [application_table_status]
       test eax, eax
       jnz .wait_lock

       call   set_application_table_status

       call get_new_process_place
       test eax, eax
       jz .failed

       mov [slot], eax

       mov    esi,[current_slot]
       mov    ebx,esi         ;ebx=esi - pointer to extended information about current thread

       mov    edi, eax
       shl    edi,8
       add    edi,SLOT_BASE
       mov    edx,edi         ;edx=edi - pointer to extended infomation about new thread
       mov    ecx,256/4
       xor eax, eax
       cld
       rep    stosd           ;clean extended information about new thread
       mov    esi,ebx
       mov    edi,edx
       mov    ecx,11
       rep    movsb           ;copy process name

       mov eax,[ebx+APPDATA.heap_base]
       mov [edx+APPDATA.heap_base], eax

       mov ecx,[ebx+APPDATA.heap_top]
       mov [edx+APPDATA.heap_top], ecx

       mov eax,[ebx+APPDATA.mem_size]
       mov [edx+APPDATA.mem_size], eax

       mov ecx,[ebx+APPDATA.dir_table]
       mov [edx+APPDATA.dir_table],ecx      ;copy page directory

       mov eax,[ebx+APPDATA.dlls_list_ptr]
       mov [edx+APPDATA.dlls_list_ptr],eax

       mov eax, [ebx+APPDATA.tls_base]
       test eax, eax
       jz @F

       push edx
       stdcall user_alloc, 4096
       pop edx
       test eax, eax
       jz .failed1	;eax=0
@@:
       mov [edx+APPDATA.tls_base], eax

       lea eax, [app_cmdline]
       stdcall set_app_params ,[slot],eax,dword 0,\
                     dword 0,dword 0

       ;mov    esi,new_process_running
       ;call   sys_msg_board_str            	;output information about succefull startup
       xor    eax,eax
       mov    [application_table_status],eax	;unlock application_table_status mutex
       mov    eax,[process_number]          	;set result
       ret
.failed:
       xor    eax,eax
.failed1:
       mov    [application_table_status],eax
       dec    eax	;-1
       ret
endp

; param
;  ebx=mutex

align 4
wait_mutex:
;;Maxis use atomic bts for mutex 4.4.2009
       push eax
       push ebx
.do_wait:
           bts dword [ebx],0
           jnc .locked
       call change_task
       jmp .do_wait
.locked:
       pop ebx
       pop eax
       ret

align 4
tls_app_entry:

        call init_heap
        stdcall user_alloc, 4096

        mov edx, [current_slot]
        mov [edx+APPDATA.tls_base], eax
        mov [tls_data_l+2],ax
        shr eax,16
        mov [tls_data_l+4],al
        mov [tls_data_l+7],ah
        mov dx, app_tls
        mov fs, dx
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
       mov [pl0_stack], eax

       lea edi, [eax+RING0_STACK_SIZE]

       mov eax, [slot]
       mov ebx, eax

       shl eax, 8
       mov [eax+SLOT_BASE+APPDATA.fpu_state], edi
       mov [eax+SLOT_BASE+APPDATA.exc_handler], 0
       mov [eax+SLOT_BASE+APPDATA.except_mask], 0

;set default io permission map
       mov ecx, [SLOT_BASE+256+APPDATA.io_map]
       mov [eax+SLOT_BASE+APPDATA.io_map], ecx
       mov ecx, [SLOT_BASE+256+APPDATA.io_map+4]
       mov [eax+SLOT_BASE+APPDATA.io_map+4], ecx

       mov esi, fpu_data
       mov ecx, 512/4
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
       add eax, RING0_STACK_SIZE
       mov [SLOT_BASE+APPDATA.saved_esp0+ebx], eax

       push ebx
       stdcall kernel_alloc, 0x1000
       pop ebx
       mov esi,[current_slot]
       mov esi,[esi+APPDATA.cur_dir]
       mov ecx,0x1000/4
       mov edi,eax
       mov [ebx+SLOT_BASE+APPDATA.cur_dir],eax
       rep movsd

       shr ebx,3
       mov eax, new_app_base
       mov dword [CURRENT_TASK+ebx+0x10],eax

.add_command_line:
       mov edx,[params]
       mov edx,[edx]       ;app_cmdline
       test edx,edx
       jz @f           ;application doesn't need parameters

       mov     eax, edx
       add     eax, 256
       jc      @f

       cmp     eax, [SLOT_BASE+APPDATA.mem_size+ebx*8]
       ja      @f

       mov     byte [edx], 0   ;force empty string if no cmdline given
       mov     eax, [cmd_line]
       test    eax, eax
       jz      @f
       stdcall strncpy, edx, eax, 256
@@:
       mov edx,[params]
       mov edx, [edx+4]    ;app_path
       test edx,edx
       jz @F           ;application don't need path of file
       mov     eax, edx
       add     eax, 1024
       jc      @f
       cmp     eax, [SLOT_BASE+APPDATA.mem_size+ebx*8]
       ja      @f
       stdcall strncpy, edx, [app_path], 1024
@@:
       mov    ebx,[slot]
       mov    eax,ebx
       shl    ebx,5
       lea    ecx,[draw_data+ebx]  ;ecx - pointer to draw data

       mov edx, irq0.return
       cmp [ebx*8+SLOT_BASE+APPDATA.tls_base], -1
       jne @F
       mov edx, tls_app_entry
@@:
; set window state to 'normal' (non-minimized/maximized/rolled-up) state
       mov     [ebx+window_data+WDATA.fl_wstate], WSTATE_NORMAL
       mov     [ebx+window_data+WDATA.fl_redraw], 1
       add    ebx,CURRENT_TASK        ;ebx - pointer to information about process
       mov    [ebx+TASKDATA.wnd_number],al;set window number on screen = process slot

       mov    [ebx+TASKDATA.event_mask],dword 1+2+4 ;set default event flags (see 40 function)

       inc    dword [process_number]
       mov    eax,[process_number]
       mov    [ebx+4],eax       ;set PID

;set draw data to full screen
       xor    eax,eax
       mov    [ecx+0],dword eax
       mov    [ecx+4],dword eax
       mov    eax,[Screen_Max_X]
       mov    [ecx+8],eax
       mov    eax,[Screen_Max_Y]
       mov    [ecx+12],eax

       mov ebx, [pl0_stack]
       mov esi,[params]
       lea ecx, [ebx+REG_EIP]
       xor eax, eax

       mov [ebx+REG_RET], edx
       mov [ebx+REG_EDI], eax
       mov [ebx+REG_ESI], eax
       mov [ebx+REG_EBP], eax
       mov [ebx+REG_ESP], ecx   ;ebx+REG_EIP
       mov [ebx+REG_EBX], eax
       mov [ebx+REG_EDX], eax
       mov [ebx+REG_ECX], eax
       mov [ebx+REG_EAX], eax

       mov eax, [esi+0x08]       ;app_eip
       mov [ebx+REG_EIP],  eax   ;app_entry
       mov [ebx+REG_CS], dword app_code
       mov [ebx+REG_EFLAGS], dword EFL_IOPL1+EFL_IF

       mov eax, [esi+0x0C]       ;app_esp
       mov [ebx+REG_APP_ESP], eax    ;app_stack
       mov [ebx+REG_SS], dword app_data

       lea ecx, [ebx+REG_RET]
       mov ebx, [slot]
       shl ebx, 5
       mov [ebx*8+SLOT_BASE+APPDATA.saved_esp], ecx

       xor  ecx, ecx    ; process state - running
; set if debuggee
       test byte [flags], 1
       jz   .no_debug
       inc  ecx     ; process state - suspended
       mov  eax,[CURRENT_TASK]
       mov  [SLOT_BASE+ebx*8+APPDATA.debugger_slot],eax
.no_debug:
       mov  [CURRENT_TASK+ebx+TASKDATA.state], cl
       ;mov    esi,new_process_running
       ;call   sys_msg_board_str     ;output information about succefull startup
       ret
endp


align 4

get_stack_base:
       mov eax,[current_slot]
       mov eax,[eax+APPDATA.pl0_stack]
       ret


include "debug.inc"
