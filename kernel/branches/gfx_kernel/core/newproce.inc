if ~defined newprocess_inc
newprocess_inc_fix:
newprocess_inc fix newprocess_inc_fix
include "mem.inc"
include "memmanag.inc"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;Working with new types of processes.
;;Author: Khalyavin Andrey halyavin@land.ru
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
iglobal
    new_process_loading db 'K : New Process - loading',13,10,0
    new_process_running db 'K : New Process - done',13,10,0
    start_not_enough_memory db 'K : New Process - not enough memory',13,10,0
endg
;-----------------------------------------------------------------------------
 
find_new_process_place:
;input:
;  none
;result:
;  eax=[new_process_place]<>0 - ok
;      0 - failed.
;This function find least empty slot.
;It doesn't increase [0x3004]!
    mov    eax,0x3000+second_base_address
    push   ebx
    mov    ebx,[0x3004]
    inc    ebx
    shl    ebx,5
    add    ebx,eax               ;ebx - address of process information for (last+1) slot
.newprocessplace:
;eax = address of process information for current slot
    cmp    eax,ebx
    jz     .endnewprocessplace   ;empty slot after high boundary
    add    eax,0x20
    cmp    word [eax+TASKDATA.state],9      ;check process state, 9 means that process slot is empty
    jnz    .newprocessplace
.endnewprocessplace:
    mov    ebx,eax
    sub    eax,0x3000+second_base_address
    shr    eax,5                 ;calculate slot index
    cmp    eax,256
    jge    .failed               ;it should be <256
    mov    word [ebx+TASKDATA.state],9      ;set process state to 9 (for slot after hight boundary)
    mov    [new_process_place],eax ;save process slot
    pop    ebx
    ret    
.failed:
    xor    eax,eax
    pop    ebx
    ret
;-----------------------------------------------------------------------------
safe_sti:
    cmp    byte [0xe000], 1
    jne    @f
    sti
 @@:ret
 
new_start_application_floppy:
;input:
;  eax - pointer to filename
;  ebx - parameters to pass
;  edx - flags
;result:
;  eax - pid of new process
;        or 0 if call fails.
    mov    [appl_path],edi
    pushad
    mov    esi,new_process_loading
    call   sys_msg_board_str     ;write to debug board
    
;wait application_table_status mutex
.table_status:    
    cli
    cmp    [application_table_status],0
    jz     .stf
    sti
    call   change_task
    jmp    .table_status
.stf:
    call   set_application_table_status
;we can change system tables now
    push   edi
    push   ebx
    push   eax
    call   find_new_process_place ;find empty process slot 
    sti
    test   eax,eax
	mov	ecx, -0x20	; too many processes
    jz     .failed

    mov    edi,eax
    shl    edi,8
    add    edi,0x80000
    mov    ecx,256/4
    xor    eax,eax
    cld
    rep    stosd                 ;clean extended information about process
    
;set new process name
    mov    [appl_path_size],eax
    pop    eax
    push   eax
.find_last_byte:
    cmp    byte [eax],0    
    jz     .find_last_byte_end
    inc    eax
    inc    [appl_path_size]
    jmp    .find_last_byte
.find_last_byte_end:
    add    [appl_path_size],24    
    sub    eax,11                ;last 11 bytes = application name
;    mov    eax,[esp]             ;eax - pointer to file name
    mov    ebx,[new_process_place]
    shl    ebx,8
    add    ebx,0x80000 + APPDATA.app_name
    mov    ecx,11
    call   memmove
      
;read header of file
    mov    eax,[esp]
    mov    ebx,1                 ;index of first block
    mov    ecx,2                 ;number of blocks
    mov    edx,0x90000           ;temp area
    mov    esi,12                ;file name length
    mov    edi,[esp+8]
;    cli
    call   floppy_fileread       ;read file from FD
;    sti
	mov	ecx, eax
	neg	ecx
	jnz	.cleanfailed
;check MENUET signature
	mov	ecx, -0x1F	; not Menuet/Kolibri executable
    cmp    [0x90000],dword 'MENU'
    jnz    .cleanfailed
    cmp    [0x90004],word 'ET'
    jnz    .cleanfailed
    
    call   get_app_params        ;parse header fields
    jc     .cleanfailed
    
    mov    eax,[new_process_place]
    	inc	ecx		; -0x1E = no memory
    call   create_app_cr3_table   ;create page directory for new process
    test   eax,eax
    jz     .cleanfailed_mem
    
    call   MEM_Get_Linear_Address ;calculate linear address of it
    
    mov    ebx,std_application_base_address
    mov    ecx,[app_mem]
    add    ecx,4095
    shr    ecx,12
    mov    edx,eax
    call   mem_alloc_specified_region ;allocate memory for application
    test   eax,eax
	mov	ecx, -0x1E
    jz     .cleanfailed_mem1
    
    mov    eax,[edx+(std_application_base_address shr 20)]
    and    eax,not (4096-1)      ;eax - physical address of first (for application memory) page table
    call   MEM_Get_Linear_Address
    mov    edx,eax
 
;read file
    mov    ebx,1
    mov    esi,12                ;length of file name
.loop1:
;edx = linear address of current page table entry
;ebx = index of current block in file
    push   edx
    mov    eax,[edx]
    and    eax,not (4096-1)
    call   MEM_Get_Linear_Address
    mov    edx,eax               ;read file block to current page
    mov    eax,[esp+4]           ;restore pointer to file name
    mov    ecx,8                 ;number of blocks read
    mov    ebp,edx               ;save buffer address for .endofimage
    push   ebx
    mov    edi,[esp+16]    
;    cli
    call   floppy_fileread
;ebx=file size    
;    sti
    pop    ecx
    add    ecx,8
    test   eax,eax
    jnz    .endloop1             ;check io errors
    mov    eax,[app_i_end]
    add    eax,511
    shr    eax,9
    cmp    ecx,eax
    jg     .endofimage           ;we have loaded whole program
    add    ebx,511
    shr    ebx,9
    cmp    ecx,ebx
    jg     .endloop1             ;if end of file?
    mov    ebx,ecx
    pop    edx
    add    edx,4
    jmp    .loop1
    
.endofimage:                     ;set to zero memory at end of page
    mov    ecx,[app_i_end]
    and    ecx,4096-1
    jz     .endloop1
    lea    edi,[ebp+ecx]
    neg    ecx
    add    ecx,4096
    xor    eax,eax
    cld
    rep    stosb
.endloop1:
    add    esp,8+4                 ;pop linear address of page table entry and pointer to file name
    call   new_start_application_fl.add_app_parameters
    mov    [esp+28],eax
    popad
    ret
    
.cleanfailed_mem1:
;there is mem for directory entry, but there is no mem for pages
;so free directory entry
    mov    eax,[new_process_place]    
    shl    eax,8
    mov    eax,[0x80000+eax+APPDATA.dir_table]
    call   MEM_Free_Page    
.cleanfailed_mem:
;there is no mem for directory entry, display message.
    mov    esi,start_not_enough_memory
    call   sys_msg_board_str
.cleanfailed:                    ;clean process name
	push	ecx	; save error code
;can't read file, clean process name. 
;this avoid problems with panel application.
    mov    edi,[new_process_place]
    shl    edi,8
    add    edi,0x80000 + APPDATA.app_name
    mov    ecx,11
    mov    eax,' '
    cld
    rep    stosb
	pop	eax
.failed:
;no more slots
    add    esp,8+4
    mov    [application_table_status],0
    mov    [esp+1Ch], eax
    popad
    sti
    ret  

;-----------------------------------------------------------------------------    
new_start_application_fl:
;input:
;  eax - pointer to filename
;  ebx - parameters to pass
;  edx - flags
;result:
;  eax - pid of new process
;        or 0 if call fails.
    mov    [appl_path],edi
    mov    [appl_path_size],36
    pushad
    mov    esi,new_process_loading
    call   sys_msg_board_str     ;write to debug board
    
;wait application_table_status mutex
.table_status:    
    cli
    cmp    [application_table_status],0
    jz     .stf
    sti
    call   change_task
    jmp    .table_status
.stf:
    call   set_application_table_status
;we can change system tables now    
    push   ebx
    push   eax
    call   find_new_process_place ;find empty process slot 
    call   safe_sti
    test   eax,eax
	mov	ecx, -0x20	; too many processes
    jz     .failed

    mov    edi,eax
    shl    edi,8
    add    edi,0x80000
    mov    ecx,256/4
    xor    eax,eax
    cld
    rep    stosd                 ;clean extended information about process
    
;set new process name
    mov    eax,[esp]             ;eax - pointer to file name
    mov    ebx,[new_process_place]
    shl    ebx,8
    add    ebx,0x80000 + APPDATA.app_name
    mov    ecx,11
    call   memmove
      
;read header of file
    mov    ebx,1                 ;index of first block
    mov    ecx,2                 ;number of blocks
    mov    edx,0x90000           ;temp area
    mov    esi,12                ;file name length
    cli
    call   fileread              ;read file from RD
    call   safe_sti
	mov	ecx, eax
	neg	ecx
	jnz	.cleanfailed
;check MENUET signature
	mov	ecx, -0x1F	; not Menuet/Kolibri executable
    cmp    [0x90000],dword 'MENU'
    jnz    .cleanfailed
    cmp    [0x90004],word 'ET'
    jnz    .cleanfailed
    
    call   get_app_params        ;parse header fields
    jc     .cleanfailed
    
    mov    eax,[new_process_place]
    inc    ecx		; -0x1E = no memory
    call   create_app_cr3_table   ;create page directory for new process
    test   eax,eax
    jz     .cleanfailed_mem
    
    call   MEM_Get_Linear_Address ;calculate linear address of it
    
    mov    ebx,std_application_base_address
    mov    ecx,[app_mem]
    add    ecx,4095
    shr    ecx,12
    mov    edx,eax
    call   mem_alloc_specified_region ;allocate memory for application
    test   eax,eax
	mov	ecx, -0x1E
    jz     .cleanfailed_mem1
    
    mov    eax,[edx+(std_application_base_address shr 20)]
    and    eax,not (4096-1)      ;eax - physical address of first (for application memory) page table
    call   MEM_Get_Linear_Address
    mov    edx,eax
 
;read file
    mov    ebx,1
    mov    esi,12                ;length of file name
.loop1:
;edx = linear address of current page table entry
;ebx = index of current block in file
    push   edx
    mov    eax,[edx]
    and    eax,not (4096-1)
    call   MEM_Get_Linear_Address
    mov    edx,eax               ;read file block to current page
    mov    eax,[esp+4]           ;restore pointer to file name
    mov    ecx,8                 ;number of blocks read
    mov    ebp,edx               ;save buffer address for .endofimage
    push   ebx
    cli
    call   fileread
;ebx=file size    
    call   safe_sti
    pop    ecx
    add    ecx,8
    test   eax,eax
    jnz    .endloop1             ;check io errors
    mov    eax,[app_i_end]
    add    eax,511
    shr    eax,9
    cmp    ecx,eax
    jg     .endofimage           ;we have loaded whole program
    add    ebx,511
    shr    ebx,9
    cmp    ecx,ebx
    jg     .endloop1             ;if end of file?
    mov    ebx,ecx
    pop    edx
    add    edx,4
    jmp    .loop1
    
.endofimage:                     ;set to zero memory at end of page
    mov    ecx,[app_i_end]
    and    ecx,4096-1
    jz     .endloop1
    lea    edi,[ebp+ecx]
    neg    ecx
    add    ecx,4096
    xor    eax,eax
    cld
    rep    stosb
.endloop1:
    add    esp,8                 ;pop linear address of page table entry and pointer to file name
    call   .add_app_parameters
    mov    [esp+28],eax
    popad
    ret
    
.cleanfailed_mem1:
;there is mem for directory entry, but there is no mem for pages
;so free directory entry
    mov    eax,[new_process_place]    
    shl    eax,8
    mov    eax,[0x80000+eax+APPDATA.dir_table]
    call   MEM_Free_Page
.cleanfailed_mem:
;there is no mem for directory entry, display message.
    mov    esi,start_not_enough_memory
    call   sys_msg_board_str
.cleanfailed:                    ;clean process name
	push	ecx	; save error code
;can't read file, clean process name. 
;this avoid problems with panel application.
    mov    edi,[new_process_place]
    shl    edi,8
    add    edi,0x80000+APPDATA.app_name
    mov    ecx,11
    mov    eax,' '
    cld
    rep    stosb
	pop	eax
.failed:
;no more slots
    add    esp,8
    mov    [application_table_status],0
    mov    [esp+1Ch], eax
    popad
    call   safe_sti
    ret
        
.add_app_parameters:
;input:
;  [esp] - pointer to parameters
;  [esp+4]-[esp+36] pushad registers.
;result
;  eax - pid of new process
;        or zero if failed
    cli
    mov    ebx,[new_process_place]
    cmp    ebx,[0x3004]
    jle    .noinc
    inc    dword [0x3004]        ;update number of processes
.noinc:

;   mov    ebx,[new_process_place]
;set 0x8c field of extended information about process
;(size of application memory)
    shl    ebx,8
    mov    eax,[app_mem]
    mov    [second_base_address+0x80000+APPDATA.mem_size+ebx],eax             
;set 0x10 field of information about process
;(application base address)     
;    mov    ebx,[new_process_place]
;    shl    ebx,5
    shr    ebx,3
    mov    dword [second_base_address+0x3000+ebx+TASKDATA.mem_start],std_application_base_address

;add command line parameters
.add_command_line:
    mov    edx,[app_i_param]
    test   edx,edx
    jz     .no_command_line      ;application don't need parameters
    mov    eax,[esp+4]
    test   eax,eax
    jz     .no_command_line      ;no parameters specified
;calculate parameter length    
    mov    esi,eax
    xor    ecx,ecx
    inc    ecx          ; include terminating null
.command_line_len:
    cmp    byte [esi],0
    jz     .command_line_len_end
    inc    esi
    inc    ecx
    cmp    ecx,256
    jl     .command_line_len
    
.command_line_len_end:
;ecx - parameter length
;edx - address of parameters in new process address space
    mov    ebx,eax               ;ebx - address of parameters in our address space
    mov    eax,[new_process_place]
    call   write_process_memory  ;copy parameters to new process address space
    
.no_command_line:
;******************************************************************
    mov    edx,[app_i_icon]
    test   edx,edx
    jz     .no_command_line_1      ;application don't need path of file
    mov    ebx,[appl_path]
    mov    ecx,[appl_path_size]
    mov    eax,[new_process_place]
    call   write_process_memory  ;copy path of file to new process address space
.no_command_line_1:
;******************************************************************
    mov    ebx,[new_process_place]
    mov    eax,ebx
    shl    ebx,5
    add    ebx,0x3000            ;ebx - pointer to information about process
    mov    [ebx+TASKDATA.wnd_number],al  ;set window number on screen = process slot
    
    mov    [ebx+TASKDATA.event_mask],dword 1+2+4     ;set default event flags (see 40 function)
    
    inc    dword [process_number]
    mov    eax,[process_number]
    mov    [ebx+TASKDATA.pid],eax           ;set PID
    
    mov    ecx,ebx
    add    ecx,draw_data-0x3000  ;ecx - pointer to draw data
;set draw data to full screen    
    mov    [ecx+RECT.left],dword 0       
    mov    [ecx+RECT.top],dword 0
    mov    eax,[0xfe00]
    mov    [ecx+RECT.right],eax
    mov    eax,[0xfe04]
    mov    [ecx+RECT.bottom],eax
;set window state to 'normal' (non-minimized/maximized/rolled-up) state
    mov    [ecx+WDATA.fl_wstate],WSTATE_NORMAL
;set cr3 register in TSS of application    
    mov    ecx,[new_process_place]     
    shl    ecx,8
    mov    eax,[0x80000+APPDATA.dir_table+ecx]
    add    eax,8+16              ;add flags
    mov    [l.cr3],eax
    
    mov    eax,[app_start]
    mov    [l.eip],eax           ;set eip in TSS
    mov    eax,[app_esp]
    mov    [l.esp],eax           ;set stack in TSS
    
;gdt
    ;mov    ebx,[new_process_place]
    ;shl    ebx,3
    mov    ax,app_code           ;ax - selector of code segment
    ;add    ax,bx
    mov    [l.cs],ax
    mov    ax,app_data
    ;add    ax,bx                 ;ax - selector of data segment
    mov    [l.ss],ax
    mov    [l.ds],ax
    mov    [l.es],ax
    mov    [l.fs],ax
    mov    ax,graph_data         ;ax - selector of graphic segment
    mov    [l.gs],ax
    mov    [l.io],word 128
    mov    [l.eflags],dword 0x11202
    mov    [l.ss0],os_data
    mov    ebx,[new_process_place]
    shl    ebx,12
    add    ebx,sysint_stack_data+4096
    mov    [l.esp0],ebx

;copy tss to it place
    mov    eax,tss_sceleton
    mov    ebx,[new_process_place]
    imul   ebx,tss_step
    add    ebx,tss_data          ;ebx - address of application TSS
    mov    ecx,120               
    call   memmove
    
;Add IO access table - bit array of permitted ports
    or     eax,-1
    mov    edi,[new_process_place]
    imul   edi,tss_step
    add    edi,tss_data+128
    mov    ecx,2048
    cld
    rep    stosd                 ;full access to 2048*8=16384 ports
    
    mov    ecx,ebx               ;ecx - address of application TSS
    mov    edi,[new_process_place]
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
    mov    [0xf400],byte 0
    mov    [0xf500],byte 0

    mov    edi,[new_process_place]
    shl    edi,5
    add    edi,window_data
    mov    ebx,[new_process_place]
    movzx  esi,word [0xC000+ebx*2]
    lea    esi,[0xC400+esi*2]
    call   windowactivate        ;gui initialization

    mov    ebx,[new_process_place]
    shl    ebx,5
; set if debuggee
        test    byte [esp+28], 1
        jz      .no_debug
        mov     [0x3000+ebx+TASKDATA.state], 1        ; set process state - suspended
        mov     eax, [0x3000]
        mov     [0x80000+ebx*8+APPDATA.debugger_slot], eax ;set debugger PID - current
        jmp     .debug
.no_debug:
        mov     [0x3000+ebx+TASKDATA.state], 0        ; set process state - running
.debug:
    
    mov    esi,new_process_running
    call   sys_msg_board_str     ;output information about succefull startup
    
;    add    esp,4                 ;pop pointer to parameters 
;    popad
    mov    eax,[process_number]  ;set result
    mov    [application_table_status],0 ;unlock application_table_status mutex
    call   safe_sti
    ret    4
;-----------------------------------------------------------------------------    
new_sys_threads:
;eax=1 - create thread
;   ebx=thread start
;   ecx=thread stack value
;result:
;   eax=pid
    xor    edx,edx	; flags=0
    pushad
   
    cmp    eax,1
    jnz    .ret                  ;other subfunctions
    mov    esi,new_process_loading
    call   sys_msg_board_str
;lock application_table_status mutex
.table_status:    
    cli
    cmp    [application_table_status],0
    je     .stf
    sti
    call   change_task
    jmp    .table_status
.stf:
    call   set_application_table_status
;find free process slot

    call   find_new_process_place
    test   eax,eax
    jz     .failed
    
;set parameters for thread
    xor    eax,eax
    mov    [app_i_param],eax
    mov    [app_i_icon],eax
    mov    [app_start],ebx
    mov    [app_esp],ecx

    mov    esi,[0x3000]
    shl    esi,8
    add    esi,0x80000+APPDATA.app_name
    mov    ebx,esi               ;ebx=esi - pointer to extended information about current thread
    
    mov    edi,[new_process_place]
    shl    edi,8
    add    edi,0x80000
    lea    edx, [edi+APPDATA.app_name] ;edx=edi - pointer to extended infomation about new thread
    mov    ecx,256/4
    rep    stosd                 ;clean extended information about new thread
    mov    edi,edx
    mov    ecx,11
    rep    movsb                 ;copy process name
    mov    eax,[ebx+APPDATA.mem_size]
    mov    [app_mem],eax         ;set memory size
    mov    eax,[ebx+APPDATA.dir_table]
    mov    dword [edx-APPDATA.app_name+APPDATA.dir_table],eax        ;copy page directory
;    mov    eax,[new_process_place]
;    mov    ebx,[0x3000]
;    call   addreference_app_cr3_table

    push   0                     ;no parameters
    call    new_start_application_fl.add_app_parameters ;start thread 
    mov    [esp+28],eax
    popad
    ret
   
.failed:
    sti
    popad
    mov    eax,-1
    ret    
.ret:
    popad
    ret
;-----------------------------------------------------------------------------    
new_mem_resize:
;input:
;  ebx - new size
;result:
;  [esp+36]:=0 - normal
;  [esp+36]:=1 - error
;This function set new application memory size.
    mov    esi,ebx               ;save new size
    add    ebx,4095
    and    ebx,not (4096-1)      ;round up size
    mov    ecx,[0x3000]
    shl    ecx,8
    mov    edx,[0x80000 + APPDATA.mem_size +ecx]     
    add    edx,4095
    and    edx,not (4096-1)      ;old size
    mov    eax,[0x80000 + APPDATA.dir_table+ecx]
    call   MEM_Get_Linear_Address
;eax - linear address of page directory    
    call   MEM_Heap_Lock         ;guarantee that two threads willn't 
                                 ;change memory size simultaneously
    cmp    ebx,edx
;    mov    esi,ebx               ;save new size
    jg     .expand
    
.free:
    sub    edx,ebx
    jz     .unlock               ;do nothing
    mov    ecx,edx
    shr    ecx,12
    add    ebx,std_application_base_address 
    call   mem_free_specified_region  ;free unnecessary pages
    jmp    .unlock

.expand:
    sub    ebx,edx
    mov    ecx,ebx
    shr    ecx,12
    mov    ebx,edx
    add    ebx,std_application_base_address
    call   mem_alloc_specified_region ;alloc necessary pages
    test   eax,eax
    jz     .failed               ;not enough memory
    
.unlock:
    mov    ebx,esi
    mov    eax,[0x3000]
    shl    eax,8
    mov    [eax+0x80000 + APPDATA.mem_size],ebx     ;write new memory size
;search threads and update 
;application memory size infomation    
    mov    ecx,[eax+0x80000 + APPDATA.dir_table] 
    mov    eax,2
    
.search_threads:
;eax = current slot
;ebx = new memory size
;ecx = page directory
    cmp    eax,[0x3004]
    jg     .search_threads_end
    mov    edx,eax
    shl    edx,5
    cmp    word [0x3000+edx+TASKDATA.state],9 ;if slot empty?
    jz     .search_threads_next
    shl    edx,3
    cmp    [edx+0x80000+APPDATA.dir_table],ecx     ;if it is our thread?
    jnz    .search_threads_next
    mov    [edx+0x80000+APPDATA.mem_size],ebx     ;update memory size
.search_threads_next:
    inc    eax
    jmp    .search_threads
.search_threads_end:

    call   MEM_Heap_UnLock
    mov    dword [esp+36],0
    ret
   
.failed:
    call   MEM_Heap_UnLock
    mov    dword [esp+36],1
    ret    
;-----------------------------------------------------------------------------
pid_to_slot:
;Input:
;  eax - pid of process
;Output:
;  eax - slot of process or 0 if process don't exists
;Search process by PID.
    push   ebx
    push   ecx
    mov    ebx,[0x3004]
    shl    ebx,5
    mov    ecx,2*32
    
.loop:
;ecx=offset of current process info entry
;ebx=maximum permitted offset
    cmp    byte [second_base_address+0x3000+ecx+TASKDATA.state],9
    jz     .endloop              ;skip empty slots
    cmp    [second_base_address+0x3000+ecx+TASKDATA.pid],eax ;check PID
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
;-----------------------------------------------------------------------------    
is_new_process:
;Input:
;  eax - process slot
;Output:
;  eax=1 - it is new process
;  eax=0 - it is old process
;    shl   eax,5
;    mov   eax,[second_base_address+0x3000+eax+0x10]
;    cmp   eax,std_application_base_address  ;check base address of application
;    jz    .new_process
;    xor   eax,eax
;    ret
    
;.new_process:
    mov   eax,1
    ret
;-----------------------------------------------------------------------------    
write_process_memory:
;Input:
;  eax - process slot
;  ebx - buffer address
;  ecx - buffer size
;  edx - start address in other process
;Output:
;  eax - number of bytes written
    pushad
    shl  eax,8
    mov  eax,[0x80000+eax+APPDATA.dir_table]
    call MEM_Get_Linear_Address
    mov  ebp,eax
;ebp=linear address of page directory of other process.    
    add  edx,std_application_base_address  ;convert to linear address
    test ecx,ecx
    jle  .ret
    
.write_loop:
;ebx = current buffer address
;ecx>0 = current size
;edx = current address in other process
;ebp = linear address of page directory

    call MEM_Heap_Lock           ;cli
    mov  esi,edx
    shr  esi,22
    mov  eax,[ebp+4*esi]         ;find page directory entry
    and  eax,not (4096-1)        ;clear flags
    test eax,eax
    jz   .page_not_found
    call MEM_Get_Linear_Address  ;calculate linear address of page table
    test eax,eax
    jz   .page_not_found
    mov  esi,edx
    shr  esi,12
    and  esi,1023                
    mov  eax,[eax+4*esi]         ;find page table entry
    and  eax,not (4096-1)
    test eax,eax
    jz   .page_not_found
    call MEM_Get_Linear_Address  ;calculate linear address of page
    test eax,eax
    jz   .page_not_found
    mov  edi,eax
    call MEM_Add_Reference_Linear;guarantee that page willn't disappear
    call MEM_Heap_UnLock         ;sti
    
    mov  esi,edx
    and  esi,4095
    add  edi,esi                 ;add offset in page
;edi = linear address corresponding edx in other process
    sub  esi,4096
    neg  esi                     ;esi - number of remaining bytes in page
    cmp  esi,ecx
    jl   .min_ecx
    mov  esi,ecx
.min_ecx:                        ;esi=min(ecx,esi) - number of bytes to write
    sub  ecx,esi
    push ecx
    mov  ecx,esi                 ;ecx - number of bytes to write
    mov  esi,ebx                 ;esi - source, edi - destination
    add  edx,ecx                 ;move pointer in address space of other process
    push edi
    
;move ecx bytes    
    test ecx,3
    jnz  .not_aligned
    shr  ecx,2
    rep  movsd
    jmp  .next_iter
.not_aligned:
    rep  movsb
.next_iter:

    pop  eax                     
    and  eax,not (4096-1)        ;eax - linear address of current page
    call MEM_Free_Page_Linear    ;free reference
    mov  ebx,esi                 ;new pointer to buffer - movsb automaticaly advance it.
    pop  ecx                     ;restore number of remaining bytes
    test ecx,ecx
    jnz  .write_loop
.ret:    
    popad
    mov  eax,ecx
    ret
    
.page_not_found:
    call MEM_Heap_UnLock         ;error has appeared in critical region
    sub  ecx,[esp+24]            ;[esp+24]<-->ecx
    neg  ecx                     ;ecx=number_of_written_bytes
    mov  [esp+28],ecx            ;[esp+28]<-->eax
    popad
    ret    
;-----------------------------------------------------------------------------    
syscall_test:
;for testing memory manager from applications.
    mov  edx,ecx
    mov  ecx,ebx
    call trans_address
    mov  ebx,eax
    mov  eax,[0x3000]
    call read_process_memory
    ret
;-----------------------------------------------------------------------------    
read_process_memory:
;Input:
;  eax - process slot
;  ebx - buffer address
;  ecx - buffer size
;  edx - start address in other process
;Output:
;  eax - number of bytes read.
    pushad
    shl  eax,8
    mov  eax,[0x80000+eax+APPDATA.dir_table]
    call MEM_Get_Linear_Address
    mov  ebp,eax
    add  edx,std_application_base_address
.read_loop:
;ebx = current buffer address
;ecx>0 = current size
;edx = current address in other process
;ebp = linear address of page directory

    call MEM_Heap_Lock           ;cli
    mov  esi,edx
    shr  esi,22
    mov  eax,[ebp+4*esi]         ;find page directory entry
    and  eax,not (4096-1)
    test eax,eax
    jz   .page_not_found
    call MEM_Get_Linear_Address
    test eax,eax
    jz   .page_not_found
    mov  esi,edx
    shr  esi,12
    and  esi,1023
    mov  eax,[eax+4*esi]         ;find page table entry
    and  eax,not (4096-1)
    test eax,eax
    jz   .page_not_found
    call MEM_Get_Linear_Address  ;calculate linear address of page
    test eax,eax
    jz   .page_not_found
    mov  esi,eax
    call MEM_Add_Reference_Linear;guarantee that page willn't disappear
    call MEM_Heap_UnLock         ;sti
    
    mov  edi,edx
    and  edi,4095                
    add  esi,edi                 ;add offset in page
;esi = linear address corresponding edx in other process
    sub  edi,4096
    neg  edi
    
;edi=min(edi,ecx) - number of bytes to copy  
    cmp  edi,ecx
    jl   .min_ecx
    mov  edi,ecx
.min_ecx:

    sub  ecx,edi                 ;update size of remaining bytes
    add  edx,edi                 ;update current pointer in other address space.
    push ecx
    mov  ecx,edi                 ;ecx - number of bytes to read
    mov  edi,ebx                 ;esi - source, edi - destination
    push esi
;move ecx bytes    
    test ecx,3
    jnz  .not_aligned
    shr  ecx,2
    rep  movsd
    jmp  .next_iter
.not_aligned:
    rep  movsb
.next_iter:
    pop  eax
    and  eax,not (4096-1)        ;eax - linear address of current page
    call MEM_Free_Page_Linear    ;free reference
    mov  ebx,edi                 ;new pointer to buffer - movsb automaticaly advance it.
    pop  ecx                     ;restore number of remaining bytes
    test ecx,ecx
    jnz  .read_loop
    
    popad
    mov  eax,ecx
    ret
    
.page_not_found:
    call MEM_Heap_UnLock         ;error has appeared in critical region
    sub  ecx,[esp+24]            ;[esp+24]<-->ecx
    neg  ecx                     ;ecx=number_of_read_bytes
    mov  [esp+28],ecx            ;[esp+28]<-->eax
    popad
    ret
;-----------------------------------------------------------------------------
check_region:
;input:
;  ebx - start of buffer
;  ecx - size of buffer
;result:
;  eax = 1 region lays in app memory
;  eax = 0 region don't lays in app memory
    mov  eax,[0x3000]
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
    cmp  word [0x3000+eax+TASKDATA.state],0
    jnz  .failed
    shl  eax,3
    mov  eax,[0x80000+eax+APPDATA.dir_table]
    test eax,eax
    jz   .failed
    call MEM_Get_Linear_Address
    push ebx
    push ecx
    push edx
    mov  edx,ebx
    and  edx,not (4096-1)
    sub  ebx,edx
    add  ecx,ebx
    mov  ebx,edx
    add  ecx,(4096-1)
    and  ecx,not (4096-1)
.loop:
;eax - linear address of page directory    
;ebx - current page
;ecx - current size
    mov  edx,ebx
    shr  edx,22
    mov  edx,[eax+4*edx]
    and  edx,not (4096-1)
    test edx,edx
    jz   .failed1
    push eax
    mov  eax,edx
    call MEM_Get_Linear_Address
    mov  edx,ebx
    shr  edx,12
    and  edx,(1024-1)
    mov  eax,[eax+4*edx]
    and  eax,not (4096-1)
    test eax,eax
    pop  eax
    jz   .failed1
    add  ebx,4096
    sub  ecx,4096
    jg   .loop
    pop  edx
    pop  ecx
    pop  ebx
.ok:
    mov  eax,1
    ret
    
.failed1:
    pop  edx
    pop  ecx
    pop  ebx
.failed:
    xor  eax,eax
    ret
;-----------------------------------------------------------------------------
new_sys_ipc:
;input:
;  eax=1 - set ipc buffer area
;    ebx=address of buffer
;    ecx=size of buffer
;  eax=2 - send message
;    ebx=PID
;    ecx=address of message
;    edx=size of message
    cmp  eax,1
    jnz  .no_ipc_def
;set ipc buffer area    
    mov  edi,[0x3000]
    shl  edi,8
    add  edi,0x80000 
    cli
    mov  [edi+APPDATA.ipc_start],ebx          ;set fields in extended information area
    mov  [edi+APPDATA.ipc_size],ecx
    sti
    mov  [esp+36],dword 0        ;success
    ret
       
.no_ipc_def:
    cmp  eax,2
    jnz  .no_ipc_send
;send message    
    cli
;obtain slot from PID    
    mov  eax,ebx
    call pid_to_slot
    test eax,eax
    jz   .no_pid
    mov  ebp,eax
;ebp = slot of other process    
    shl  eax,8
    mov  edi,[eax+0x80000+APPDATA.ipc_start]  ;is ipc area defined?
    test edi,edi
    jz   .no_ipc_area
    mov  esi,[eax+0x80000+APPDATA.ipc_size]  ;esi - size of buffer
    push dword -1                ;temp variable for read_process_memory
    mov  ebx,esp
    push ecx
    push edx
    mov  ecx,4                   ;read 4 bytes
    mov  eax,ebp
    mov  edx,edi                 ;from beginning of buffer.
    call read_process_memory
    mov  eax,[esp+8]
    test eax,eax
    jnz  .ipc_blocked            ;if dword [buffer]<>0 - ipc blocked now
    add  edx,4                   ;move to next 4 bytes
    mov  eax,ebp
    call read_process_memory     ;read size of occupied space in buffer
    sub  esi,8
    sub  esi,[esp]
    sub  esi,[esp+8]             ;esi=(buffer size)-(occupied size)-(message size)-(header of message size)
    js   .buffer_overflow        ;esi<0 - not enough memory in buffer
    mov  esi,[esp+8]             ;previous offset
    add  dword [esp+8],8
    mov  edi,[esp]
    add  [esp+8],edi             ;add (size of message)+(size of header of message) to [buffer+4]
    mov  eax,ebp
    call write_process_memory
    add  edx,esi                 
    sub  edx,4                   ;move to beginning of place for our message
    mov  eax,[second_base_address+0x3010]
    mov  eax,[eax+TASKDATA.pid]           ;eax - our PID
    mov  [esp+8],eax
    mov  eax,ebp
    call write_process_memory    ;write PID
    mov  ebx,esp                 ;address of size of message
    mov  eax,ebp
    add  edx,4
    call write_process_memory    ;write size of message
    add  edx,4
    pop  ecx                     ;ecx - size of message
    pop  eax
    call trans_address
    mov  ebx,eax                 ;ebx - linear address of message
    add  esp,4                   ;pop temporary variable
    mov  eax,ebp
    call write_process_memory    ;write message
    sti
;awake other process    
    shl  ebp,8
    mov  eax,ebp
    or   [eax+0x80000+APPDATA.event_mask],dword 0x40
    
    cmp  dword [check_idle_semaphore],20
    jge  .ipc_no_cis
    mov  dword [check_idle_semaphore],5
.ipc_no_cis:
    mov  dword [esp+36],0
    ret
.no_ipc_send:
    mov  dword [esp+36],-1
    ret
.no_pid:
    sti
    mov  dword [esp+36],4
    ret
.no_ipc_area:
    sti
    mov  dword [esp+36],1
    ret
.ipc_blocked:
    sti
    add  esp,12
    mov  dword [esp+36],2
    ret
.buffer_overflow:
    sti
    add  esp,12
    mov  dword [esp+36],3
    ret
;-----------------------------------------------------------------------------    
trans_address:
;Input
;  eax - application address
;Output
;  eax - linear address for kernel      
    add   eax,std_application_base_address
    ret
;-----------------------------------------------------------------------------    
new_start_application_hd:
;eax - file name (kernel address)
;ebx - file name length
;ecx - work area (kernel address)
;edx - flags
;ebp - parameters
    mov    [appl_path],edi
    pushad
    
    mov    esi,new_process_loading
    call   sys_msg_board_str     ;write message to message board
    
;lock application_table_status mutex
.table_status:
    cli
    cmp    [application_table_status],0
    jz     .stf
    sti
    call   change_task
    jmp    .table_status
.stf:
    call   set_application_table_status
    
    push   ebp
    push   ebx
    push   eax
    push   ecx
    call   find_new_process_place ;find new process slot
    sti
    test   eax,eax
	mov	ecx, -0x20	; too many processes
    jz     .failed
    
;write application name
    xor    eax,eax
    mov    [appl_path_size],eax    
    mov    eax,[esp+4]
.find_last_byte:
    cmp    byte [eax],0    
    jz     .find_last_byte_end
    inc    eax
    inc    [appl_path_size]
    jmp    .find_last_byte
.find_last_byte_end:
    add    [appl_path_size],24    
    lea    esi,[eax-11]          ;last 11 bytes = application name
    mov    edi,[new_process_place]
    shl    edi,8
    add    edi,0x80000+APPDATA.app_name
    mov    ecx,11
    cld
    rep    movsb                 ;copy name to extended information about process
    
;read header    
    mov    eax,[esp+4]           ;file name
    mov    esi,[esp]             ;work area
    mov    ecx,1                 ;read from first block
    mov    edx,1                 ;read 1 block
    call   read_hd_file
	mov	ecx, eax
	neg	ecx
	jnz	.cleanfailed

    pop    esi
    push   esi
;check menuet signature
	mov	ecx, -0x1F	; not Menuet/Kolibri executable
    cmp    [esi+1024+0],dword 'MENU'  ;read_hd_file function write file to +1024 offset
    jnz    .cleanfailed
    cmp    [esi+1024+4],word 'ET'
    jnz    .cleanfailed
    add    esi,1024
    mov    edi,0x90000
    mov    ecx,512/4
    cld
    rep    movsd                 ;copy first block to 0x90000 address for get_app_params function
    call   get_app_params
	mov	ecx, -0x1F	; not Menuet/Kolibri executable
    jc     .cleanfailed
    
    mov    eax,[new_process_place]
	inc	ecx		; -0x1E = no memory
    call   create_app_cr3_table  ;create page directory
    test   eax,eax
    jz     .cleanfailed_mem
    
    call   MEM_Get_Linear_Address
    
    mov    ebx,std_application_base_address
    mov    ecx,[app_mem]
    add    ecx,4096-1
    shr    ecx,12
    mov    edx,eax               ;edx - linear address of page directory
    call   mem_alloc_specified_region ;allocate memory for application
	mov	ecx, -0x1E	; no memory
    test   eax,eax
    jz     .cleanfailed_mem1
    
    add    edx,(std_application_base_address shr 20)
    mov    eax,[edx]
    and    eax,not (4096-1)
    call   MEM_Get_Linear_Address
    push   edx                   ;save pointer to first page table
    mov    edx,eax
;read file
    mov    ecx,1
    xor    ebp,ebp
.loop1:
;[esp] - pointer to current page directory entry
;edx - pointer to current page table
;ebp - offset in page
;ecx - current cluster
    push   edx
    mov    eax,[esp+12]          ;file name
    mov    ebx,[esp+16]          ;file name length
    mov    esi,[esp+8]           ;work area
    mov    edx,1                 ;number of blocks to read
    push   ecx
    push   ebp
    cli
    call   read_hd_file
    sti
    pop    ebp
    test   eax,eax
    jnz    .endloop1             ;check io errors
    
    mov    esi,[esp+8+4]         ;work area
    add    esi,1024
    mov    eax,[esp+4]           ;current page table 
    mov    eax,[eax]
    and    eax,not (4096-1)        
    call   MEM_Get_Linear_Address;calculate linear page address
    lea    edi,[eax+ebp]         ;add page offset
    mov    ecx,512/4
    cld
    rep    movsd                 ;copy data
    
    pop    ecx
    inc    ecx                   ;next block
    mov    eax,[app_i_end] ;todo: precalculate ([app_i_end]+4095)/4096
    add    eax,512-1
    shr    eax,9                 ;calculate application image size
    cmp    ecx,eax
    jg     .endloop11
    pop    edx
    add    ebp,512               ;new offset
    test   ebp,4096
    jz     .loop1
    xor    ebp,ebp
    add    edx,4                 ;go to next page
    test   edx,(4096-1)
    jnz    .loop1
    add    dword [esp],4         ;go to next directory entry
    mov    eax,[esp]
    mov    eax,[eax]
    and    eax,not (4096-1)
    call   MEM_Get_Linear_Address
    mov    edx,eax
    jmp    .loop1
.endloop1:
    add    esp,4                 ;pop ecx
.endloop11: 
    add    esp,4+4               ;pop edx, pop edx
    
;add_app_parameters
    add    esp,12                ;now pointer to parameters is on the top of the stack
    call   new_start_application_fl.add_app_parameters ;start process
    mov    [esp+28],eax
    popad
    ret
    
.cleanfailed_mem1:
;there is mem for directory entry, but there is no mem for pages
;so free directory entry
    mov    eax,[new_process_place]
    shl    eax,8
    mov    eax,[0x80000+eax+APPDATA.dir_table]
    call   MEM_Free_Page
.cleanfailed_mem:
;there is no mem for directory entry, display message.
    mov    esi,start_not_enough_memory
    call   sys_msg_board_str    
.cleanfailed:                    ;clean process name
	push	ecx
;can't read file, clean process name. 
;this avoid problems with panel application.
    mov    edi,[new_process_place]
    shl    edi,8
    add    edi,0x80000+APPDATA.app_name
    mov    ecx,11
    mov    eax,' '
    cld
    rep    stosb
	pop	eax
.failed:
;no more slots
    add    esp,16
    mov    [esp+1Ch], eax
    popad
    mov    [application_table_status],0
    sti
    ret
end if

; \begin{diamond}
        include 'debug.inc'

fs_execute:
; ebx - cmdline
; edx - flags
; ebp - full filename
; [esp+4] = procedure DoRead, [esp+8] = filesize & [esp+12]... - arguments for it
        pushad
; check filename length - with terminating NULL must be no more than 1024 symbols
        mov     edi, ebp
        mov     ecx, 1024
        xor     eax, eax
        repnz   scasb
        jz      @f
        popad
        mov     eax, -ERROR_FILE_NOT_FOUND
        ret
@@:

        mov     esi, new_process_loading
        call    sys_msg_board_str       ; write message to message board

; lock application_table_status mutex
.table_status:
        cli
        cmp     [application_table_status], 0
        jz      .stf
        sti
        call    change_task
        jmp     .table_status
.stf:
        call    set_application_table_status
        push    ebx     ; save command line pointer for add_app_parameters

        call    find_new_process_place  ; find new process slot
        call    safe_sti
        test    eax, eax
        mov     ecx, -0x20      ; too many processes
        jz      .failed

; write application name
        push    edi
        mov     ecx, edi
        sub     ecx, ebp
        mov     [appl_path], ebp
        mov     [appl_path_size], ecx
        dec     edi
        std
        mov     al, '/'
        repnz   scasb
        cld
        jnz     @f
        inc     edi
@@:
        inc     edi
; now edi points to name without path
        mov     esi, edi
        mov     ecx, 8  ; 8 chars for name
        mov     edi, [new_process_place]
        shl     edi, cl
        add     edi, 0x80000+APPDATA.app_name
.copy_process_name_loop:
        lodsb
        cmp     al, '.'
        jz      .copy_process_name_done
        test    al, al
        jz      .copy_process_name_done
        stosb
        loop    .copy_process_name_loop
.copy_process_name_done:
        mov     al, ' '
        rep     stosb
        pop     eax
        mov     cl, 3   ; 3 chars for extension
        dec     esi
@@:
        dec     eax
        cmp     eax, esi
        jbe     .copy_process_ext_done
        cmp     byte [eax], '.'
        jnz     @b
        lea     esi, [eax+1]
.copy_process_ext_loop:
        lodsb
        test    al, al
        jz      .copy_process_ext_done
        stosb
        loop    .copy_process_ext_loop
.copy_process_ext_done:
        mov     al, ' '
        rep     stosb

; read header
        lea     eax, [esp+8+36]
        mov     edi, 0x90000
        call    dword [eax-4]
        mov     ecx, eax
        neg     ecx
        jnz     .cleanfailed
; check menuet signature
        mov     ecx, -0x1F
        cmp     dword [0x90000], 'MENU'
        jnz     .cleanfailed
        cmp     word [0x90004], 'ET'
        jnz     .cleanfailed
        call    get_app_params
        mov     ecx, -0x1F
        jc      .cleanfailed
; sanity check - because we will load all file,
; file size must be not greater than memory size
        mov     eax, [esp+8+36]
        cmp     [app_mem], eax
        jb      .cleanfailed

        mov     eax, [new_process_place]
        inc     ecx     ; -0x1E = no memory
        call    create_app_cr3_table
        test    eax, eax
        jz      .cleanfailed_mem

        call    MEM_Get_Linear_Address

        mov     ebx, std_application_base_address
        mov     ecx, [app_mem]
        add     ecx, 4095
        shr     ecx, 12
        mov     edx, eax        ; edx - linear address of page directory
        call    mem_alloc_specified_region
        mov     ecx, -0x1E      ; no memory
        test    eax, eax
        jz      .cleanfailed_mem1

        add     edx, std_application_base_address shr 20
        mov     eax, [edx]
        and     eax, not 4095
        call    MEM_Get_Linear_Address
        push    edx             ; save pointer to first page table
        mov     edx, eax
; read file
; first block is already read to 0x90000
        mov     eax, [edx]
        and     eax, not 0xFFF
        call    MEM_Get_Linear_Address
        mov     esi, 0x90000
        mov     edi, eax
        mov     ecx, 512/4
        rep     movsd
        sub     edi, eax
.loop1:
; [esp] = pointer to current page directory entry
; edx = pointer to current page table
; edi = offset in page
        mov     eax, [edx]
        and     eax, not 0xFFF
        call    MEM_Get_Linear_Address
        push    edi
        add     edi, eax
        lea     eax, [esp+8+36+8]
        call    dword [eax-4]
        pop     edi
        test    eax, eax
        jnz     .endloop1
        add     edi, 512        ; new offset
        cmp     edi, 4096
        jb      .loop1
        xor     edi, edi
        add     edx, 4          ; go to next page
        test    edx, 4096-1
        jnz     .loop1
        pop     eax
        add     eax, 4          ; go to next directory entry
        push    eax
        mov     eax, [eax]
        and     eax, not 0xFFF
        call    MEM_Get_Linear_Address
        mov     edx, eax
        jmp     .loop1
.endloop1:
        pop     edx
        cmp     eax, 6
        jnz     .cleanfailed_mem2
        call    new_start_application_fl.add_app_parameters
        mov     [esp+28], eax
        popad
        ret

.cleanfailed_mem2:
; file read error; free all allocated mem
        mov     ecx, eax
        neg     ecx
        mov     eax, [new_process_place]
        call    dispose_app_cr3_table
        jmp     .cleanfailed
.cleanfailed_mem1:
; there is mem for directory entry, but there is no mem for pages
; so free directory entry
        mov     eax, [new_process_place]
        shl     eax, 8
        mov     eax, [0x80000+eax+0xB8]
        call    MEM_Free_Page
.cleanfailed_mem:
; there is no mem for directory entry, display message
        mov     esi, start_not_enough_memory
        call    sys_msg_board_str
.cleanfailed:
        push    ecx
; clean process name, this avoid problems with @panel
        mov     edi, [new_process_place]
        shl     edi, 8
        add     edi, 0x80000+APPDATA.app_name
        mov     ecx, 11
        mov     al, ' '
        rep     stosb
        pop     eax
.failed:
        pop     ebx
        mov     [esp+28], eax
        popad
        mov     [application_table_status], 0
        call    safe_sti
        ret
; \end{diamond}
