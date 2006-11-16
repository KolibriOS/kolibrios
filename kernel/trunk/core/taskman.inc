GREEDY_KERNEL  equ 0; 1


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

align 4
proc test_app_header stdcall, header:dword
           virtual at ebx
             APP_HEADER_00 APP_HEADER_00
           end virtual

           mov ebx, [header]
           cmp [ebx+6], word '00'
           jne  .check_01_header

           mov  eax,[APP_HEADER_00.start]
           mov  [app_start],eax
           mov  eax,[APP_HEADER_00.i_end]
           mov  [app_i_end],eax
           mov  eax,[APP_HEADER_00.mem_size]
           mov  [app_mem],eax
           shr  eax,1
           sub  eax,0x10
           mov  [app_esp],eax
           mov  eax,[APP_HEADER_00.i_param]
           mov  [app_i_param],eax
           mov  [app_i_icon],dword 0

           mov  eax,1
           ret

 .check_01_header:
           virtual at ebx
             APP_HEADER_01 APP_HEADER_01
           end virtual

           cmp  [ebx+6],word '01'
           jne  .no_01_header

           mov  eax,[APP_HEADER_01.start]
           mov  [app_start],eax
           mov  eax,[APP_HEADER_01.i_end]
           mov  [app_i_end],eax
           mov  eax,[APP_HEADER_01.mem_size]
           mov  [app_mem],eax
           mov  eax,[APP_HEADER_01.stack_top]
           mov  [app_esp],eax
           mov  eax,[APP_HEADER_01.i_param]
           mov  [app_i_param],eax
           mov  eax,[APP_HEADER_01.i_icon]
           mov  [app_i_icon],eax

           mov  eax,1
           ret

.no_01_header:

           xor eax, eax
           ret
endp

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
        ;   mov [new_process_place], eax
           ret

.failed:
           xor    eax,eax
           ret
endp

align 4
proc create_app_space stdcall, app_size:dword,img_size:dword
           locals
             app_pages   dd ?
             img_pages   dd ?
             dir_addr    dd ?
             app_tabs    dd ?
           endl

           stdcall wait_mutex, pg_data.pg_mutex

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
           mov ecx, 384
           cld
           rep movsd

           mov ecx, 384
           xor eax, eax
           cld
           rep stosd

           mov ecx, 256
           mov esi, sys_pgdir+0xc00
           rep movsd

           mov eax, [dir_addr]
           or eax, PG_SW
           mov ebx, [tmp_task_pdir]
           mov [ebx+0x600], eax

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
           add edi, pages_tab
           mov ecx, [app_tabs]
           shl ecx, 10
           xor eax, eax
           rep stosd

           mov edx, new_app_base
.alloc:
           call alloc_page
           test eax, eax
           jz .fail

           stdcall map_page,edx,eax,dword PG_UW
           add edx, 0x1000
           sub [app_pages], 1
           sub [img_pages], 1
           jnz .alloc

           mov ecx, [app_pages]
           and ecx, ecx
           jz .next

           mov ebx, edx
           shr edx, 12
.reserve:
           mov dword [pages_tab+edx*4], 0x02
           invlpg [ebx]
           inc edx
           dec ecx
           jnz .reserve
.next:
           mov edi, new_app_base
           mov ecx, [img_size]
           shr ecx, 2
           xor eax, eax
           cld
           rep stosd

           stdcall map_page,[tmp_task_pdir],dword 0,dword PG_UNMAP

           dec [pg_data.pg_mutex]
           mov eax, [dir_addr]
           ret
.fail:
           dec [pg_data.pg_mutex]
           cmp [dir_addr], 0
           jz @f
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
           mov [PROC_BASE+esi+0xB8],eax
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

           stdcall wait_mutex, pg_data.pg_mutex

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
           cmp   [PROC_BASE+ecx+0xB8],ebx       ;compare page directory addresses
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
           add esi, 0x604
           mov edi, 383
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

align 4
proc fs_execute

;fn_read:dword, file_size:dword, cluster:dword

; ebx - cmdline
; edx - flags
; ebp - full filename
; [esp+4] = procedure DoRead, [esp+8] = filesize & [esp+12]... - arguments for it

           locals
             cmdline    dd ?
             flags      dd ?
             filename   dd ?
             retval     dd ?
           endl

           pushad

           mov [cmdline], ebx
           mov [flags], edx
           mov eax, [ebp]
           mov [filename], eax

           stdcall wait_mutex, pg_data.tmp_task_mutex

           mov edi, [tmp_task_data]
           mov ecx, (2048+256)/4
           xor eax, eax
           rep stosd

           mov esi, [filename]
           mov edi, [tmp_task_data]
           add edi, TMP_FILE_NAME
           mov ecx, 1024
           rep movsb

           mov esi, [filename]
           mov edi, [tmp_task_data]
           add edi, TMP_ICON_OFFS
           mov ecx, 1024
           rep movsb

           mov esi, [cmdline]
           test esi, esi
           jz @f
           mov edi, [tmp_task_data]
           add edi, TMP_CMD_LINE
           mov ecx, 256
           rep movsb
@@:
           mov eax, TMP_FILE_NAME
           add eax, [tmp_task_data]
           mov ebx, [tmp_task_data]    ;cmd line
           add ebx, TMP_CMD_LINE

           stdcall fs_exec, eax, ebx, [flags], [ebp+8],\
                            [ebp+12], [ebp+16],[ebp+20]
           mov [retval], eax
           popad
           mov [pg_data.tmp_task_mutex], 0
           mov eax, [retval]
           ret

endp

align 4
proc fs_exec stdcall file_name:dword, cmd_line:dword, flags:dword,\
                       fn_read:dword, file_size:dword,\
                       cluster:dword, some_data:dword

           locals
             slot          dd ?
             app_path_size dd ?
             save_cr3      dd ?
             img_size      dd ?
           endl

; check filename length - with terminating NULL must be no more than 1024 symbols

           mov edi, [file_name]
           mov ecx, 1024
           xor eax, eax
           repnz scasb
           jz @f
           mov     eax, -ERROR_FILE_NOT_FOUND
           ret
@@:
           sub edi, [file_name]
           mov [app_path_size], edi

           mov esi, new_process_loading
           call sys_msg_board_str       ; write message to message board

           pushfd
           cli

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
           mov ecx, -0x20      ; too many processes
           jz .err
           mov [slot], eax

           mov    edi,eax
           shl    edi,8
           add    edi,PROC_BASE
           mov    ecx,256/4
           xor    eax,eax
           cld
           rep    stosd              ;clean extended information about process

; write application name

           mov edi, [file_name]
           mov ecx, [app_path_size]
           add edi, ecx
           dec edi
           std
           mov al, '/'
           repnz scasb
           cld
           jnz @f
           inc edi
@@:
           inc edi
; now edi points to name without path

           mov esi, edi
           mov ecx, 8  ; 8 chars for name
           mov edi, [slot]
           shl edi, cl
           add edi, PROC_BASE
.copy_process_name_loop:
           lodsb
           cmp al, '.'
           jz .copy_process_name_done
           test al, al
           jz .copy_process_name_done
           stosb
           loop .copy_process_name_loop
.copy_process_name_done:
           mov al, ' '
           rep stosb
           pop eax
           mov cl, 3   ; 3 chars for extension
           dec esi
@@:
           dec eax
           cmp eax, esi
           jbe .copy_process_ext_done
           cmp byte [eax], '.'
           jnz @b
           lea esi, [eax+1]
.copy_process_ext_loop:
           lodsb
           test al, al
           jz  .copy_process_ext_done
           stosb
           loop .copy_process_ext_loop
.copy_process_ext_done:
           mov al, ' '
           rep stosb

; read header
           lea  eax, [file_size]
           mov ebx, [eax]
           mov [img_size], ebx
           mov  edi, TMP_BUFF
           call [fn_read]

           test eax, eax
           jnz .err

; check menuet signature

           mov ecx, -0x1F
;check MENUET signature
           cmp    [TMP_BUFF],dword 'MENU'
           jnz    .err
           cmp    [TMP_BUFF+4],word 'ET'
           jnz    .err

           stdcall test_app_header, TMP_BUFF
           test eax, eax
           jz .err

           mov eax, cr3
           mov [save_cr3], eax
     if GREEDY_KERNEL
           stdcall create_app_space,[app_mem],[img_size]
     else
           stdcall create_app_space,[app_mem],[app_mem]
     end if
           test eax, eax
           jz .failed

           mov    ebx,[slot]
           shl   ebx,8
           mov   [PROC_BASE+ebx+0xB8],eax

           mov esi, TMP_BUFF
           mov edi, new_app_base
           mov ecx, 512/4
           cld
           rep movsd

;read file
@@:
           lea  eax, [file_size]
           cmp dword [eax], 0
           jz .done
           push edi
           call [fn_read]
           pop edi
           add edi, 512
           test eax, eax
           jz  @b
           cmp ebx, 6
           jne .failed
.done:
           stdcall add_app_parameters, [slot], new_app_base,\
                                       [cmd_line],[file_name],[flags]

           mov eax, [save_cr3]
           call set_cr3

           xor eax, eax
           mov    [application_table_status],eax ;unlock application_table_status mutex
           popfd
           mov    eax,[process_number]  ;set result
           ret

.failed:
           mov eax, [save_cr3]
           call set_cr3
.err:

           popfd
           xor eax, eax
           mov [application_table_status],eax
           ret
endp

align 4
proc add_app_parameters stdcall,slot:dword,img_base:dword,\
                        cmd_line:dword, app_path:dword, flags:dword

           mov eax,[slot]
           bt [cpu_caps], CAPS_FXSR
           jnc .no_SSE
           shl eax, 8
           mov ebx, eax
           add eax, eax
           add eax, [fpu_data]
           mov [ebx+PROC_BASE+APPDATA.fpu_state], eax
           mov [ebx+PROC_BASE+APPDATA.fpu_handler], 0
           mov [ebx+PROC_BASE+APPDATA.sse_handler], 0
           jmp .m1
.no_SSE:
           mov ecx, eax
           mov ebx, eax
           shl eax, 7
           shl ebx, 4
           sub eax, ebx       ;eax*=112
           add eax, [fpu_data]
           shl ecx, 8
           mov [ecx+PROC_BASE+APPDATA.fpu_state], eax
           mov [ecx+PROC_BASE+APPDATA.fpu_handler], 0
           mov [ecx+PROC_BASE+APPDATA.sse_handler], 0
.m1:
           mov    ebx,[slot]
           cmp    ebx,[TASK_COUNT]
           jle    .noinc
           inc    dword [TASK_COUNT]       ;update number of processes
.noinc:
           shl    ebx,8
           mov    eax,[app_mem]
           mov    [PROC_BASE+0x8c+ebx],eax

           shr    ebx,3
           mov eax, new_app_base
           mov    dword [CURRENT_TASK+ebx+0x10],eax

.add_command_line:
           mov    edx,[app_i_param]
           test   edx,edx
           jz     .no_command_line      ;application don't need parameters
           mov    eax,[cmd_line]
           test   eax,eax
           jz     .no_command_line      ;no parameters specified
;calculate parameter length
           xor    ecx,ecx
.command_line_len:
           cmp    byte [eax],0
           jz     .command_line_len_end
           inc    eax
           inc    ecx
           cmp    ecx,255
           jl     .command_line_len

.command_line_len_end:
;ecx - parameter length
;edx - address of parameters in new process address space
           inc ecx
           mov edi, [img_base]
           add edi, edx
           mov esi, [cmd_line]
           rep movsb

.no_command_line:

            mov    edx,[app_i_icon]
            test   edx,edx
            jz     .no_command_line_1      ;application don't need path of file
            mov    esi,[app_path]
            test esi, esi
            jz     .no_command_line_1      ;application don't need path of file
            mov    ecx, 64
            mov    edi, [img_base]
            add edi, edx
            rep movsb

.no_command_line_1:
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
           mov    eax,[SCR_X_SIZE]
           mov    [ecx+8],eax
           mov    eax,[SCR_Y_SIZE]
           mov    [ecx+12],eax
;set cr3 register in TSS of application

           mov    ecx,[slot]
           shl    ecx,8
           mov    eax,[PROC_BASE+0xB8+ecx]
           ;or     eax,  PG_NOCACHE
           mov    [l.cr3],eax

           mov    eax,[app_start]
           mov    [l.eip],eax           ;set eip in TSS
           mov    eax,[app_esp]
           mov    [l.esp],eax           ;set stack in TSS

;gdt
           mov    ax,app_code           ;ax - selector of code segment
           mov    [l.cs],ax
           mov    ax,app_data
           mov    [l.ss],ax
           mov    [l.ds],ax
           mov    [l.es],ax
           mov    [l.fs],ax
           mov    ax,graph_data         ;ax - selector of graphic segment
           mov    [l.gs],ax
           mov    [l.io],word 128
           mov    [l.eflags],dword 0x1202

           mov    [l.ss0],os_data
           mov    ebx,[slot]
           shl    ebx,12
           add    ebx,sysint_stack_data+4096
           mov    [l.esp0],ebx

;copy tss to it place
           mov    eax,tss_sceleton
           mov    ebx,[slot]
           imul   ebx,tss_step
           add    ebx,tss_data          ;ebx - address of application TSS
           mov    ecx,120
           call   memmove

;Add IO access table - bit array of permitted ports
           or     eax,-1
           mov    edi,[slot]
           imul   edi,tss_step
           add    edi,tss_data+128
           mov    ecx,2048
           cld
           rep    stosd                 ;full access to 2048*8=16384 ports

           mov    ecx,ebx               ;ecx - address of application TSS
           mov    edi,[slot]
           shl    edi,3
;set TSS descriptor
           mov    [edi+gdts+tss0+0],word tss_step ;limit (size)
           mov    [edi+gdts+tss0+2],cx  ;part of offset
           mov    eax,ecx
           shr    eax,16
           mov    [edi+gdts+tss0+4],al  ;part of offset
           mov    [edi+gdts+tss0+7],ah  ;part of offset
           mov    [edi+gdts+tss0+5],word 01010000b*256+11101001b ;system flags

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
           test   byte [flags], 1
           jz     .no_debug
           mov    [CURRENT_TASK+ebx+0xa],byte 1 ;set process state - suspended
           mov    eax,[CURRENT_TASK]
           mov    [PROC_BASE+ebx*8+0xac],eax ;set debugger PID - current
.no_debug:

           mov    esi,new_process_running
           call   sys_msg_board_str     ;output information about succefull startup

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
     mov  eax,[PROC_BASE+eax+0xb8]
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
                              [PROC_BASE+eax+0xB8],\
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
                              [PROC_BASE+eax+0xB8],\
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
             thread_start  dd ?
             thread_stack  dd ?
             params        dd ?
             slot          dd ?
           endl

           mov [thread_start], ebx
           mov [thread_stack], ecx
           mov [params], 0

           xor    edx,edx      ; flags=0

           cmp    eax,1
           jnz    .failed                  ;other subfunctions
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

           xor    eax,eax
           mov    [app_i_param],eax
           mov    [app_i_icon],eax

           mov ebx, [thread_start]
           mov ecx, [thread_stack]

           mov    [app_start],ebx
           mov    [app_esp],ecx

           mov    esi,[CURRENT_TASK]
           shl    esi,8
           add    esi,PROC_BASE
           mov    ebx,esi               ;ebx=esi - pointer to extended information about current thread

           mov    edi,[slot]
           shl    edi,8
           add    edi,PROC_BASE
           mov    edx,edi               ;edx=edi - pointer to extended infomation about new thread
           mov    ecx,256/4
           rep    stosd                 ;clean extended information about new thread
           mov    edi,edx
           mov    ecx,11
           rep    movsb                 ;copy process name
           mov    eax,[ebx+APPDATA.heap_base]
           mov    [edx+APPDATA.heap_base], eax
           mov    ecx,[ebx+APPDATA.heap_top]
           mov    [edx+APPDATA.heap_top], ecx
           mov    eax,[ebx+APPDATA.mem_size]
           mov    [app_mem],eax         ;set memory size
           mov    eax,[ebx+0xb8]
           mov    [edx+0xb8],eax        ;copy page directory

           stdcall add_app_parameters, [slot], new_app_base,\
                                        [params], dword 0,dword 0

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

align 4
proc wait_mutex stdcall, mutex:dword
           mov ebx, [mutex]
.wait_lock:
           cmp dword [ebx],0
           je .get_lock
           push ebx
           call change_task
           pop ebx
           jmp .wait_lock

.get_lock:
           mov eax, 1
           xchg eax, [ebx]
           test eax, eax
           jnz .wait_lock
           ret
endp


include "debug.inc"

iglobal
    new_process_loading db 'K : New Process - loading',13,10,0
    new_process_running db 'K : New Process - done',13,10,0
    start_not_enough_memory db 'K : New Process - not enough memory',13,10,0
endg

