if ~defined memmanager_inc
memmanager_inc_fix:
memmanager_inc fix memmanager_inc_fix
;for testing in applications
if defined B32
  iskernel=1
else
  iskernel=0
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Memory allocator for MenuetOS kernel
;; Andrey Halyavin, halyavin@land.ru 2005
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; heap block structure - 
;; you can handle several ranges of 
;; pages simultaneosly.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.heap_linear_address   equ 0
.heap_block_size       equ 4
.heap_physical_address equ 8
.heap_reserved         equ 12
.heap_block_info       equ 16
max_heaps equ 8
.range_info            equ 36
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; memory manager data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MEM_heap_block_    rd .heap_block_info*max_heaps/4
MEM_heap_block=MEM_heap_block_+second_base_address
MEM_heap_count_    rd 1
MEM_heap_count=MEM_heap_count_+second_base_address
if iskernel = 0
MEM_general_mutex rd 1
MEM_call_count    rd 1
MEM_mutex_pid     rd 1
MEM_mutex_count   rd 1
else
MEM_cli_count_    rd 1
MEM_cli_count=MEM_cli_count_+second_base_address
MEM_cli_prev_     rd 1
MEM_cli_prev=MEM_cli_prev_+second_base_address
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Init
;;Initialize memory manager structures.
;;Must be called first.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MEM_Init:
    push  eax
    xor   eax,eax
if iskernel = 0
    mov   [MEM_heap_count],eax
    mov   [MEM_general_mutex],eax
    mov   [MEM_call_count],eax
    mov   [MEM_mutex_pid],eax
    mov   [MEM_mutex_count],eax
else
    mov   [MEM_cli_prev],eax     ;init value = 0
    dec   eax
    mov   [MEM_cli_count],eax    ;init value = -1
end if
    pop   eax
    ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;change_task
;;procedure for changing tasks.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;    
if iskernel = 0
change_task:
    push  eax
    push  ebx
    mov   eax,5
    xor   ebx,ebx
    inc   ebx
    int   0x40
    pop   ebx
    pop   eax
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_get_pid
;;determine current pid
;;result:
;; eax - pid
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if iskernel = 0
MEM_get_pid:
    push  ebx
    push  ecx
    sub   esp,1024
    mov   eax,9
    mov   ebx,esp
    mov   ecx,-1
    int   0x40
    mov   eax,[esp+30]
    add   esp,1024
    pop   ecx
    pop   ebx
    ret
else
;    pid_address dd 0x3000
;MEM_get_pid:
;    mov   eax,[pid_address]
;    mov   eax,[eax]
;    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Heap_Lock
;;Wait until all operations with heap will be finished.
;;Between MEM_Heap_Lock and MEM_Heap_UnLock operations 
;;with heap are forbidden.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MEM_Heap_Lock:
if iskernel = 0
    push  eax
    inc   dword [MEM_call_count]
MEM_Heap_Lock_wait:
    mov   eax,1
    xchg  [MEM_general_mutex],eax
    test  eax,eax
    jz    MEM_Heap_Lock_end
    call  MEM_get_pid
    cmp   [MEM_mutex_pid],eax
    jz    MEM_Heap_Lock_end1
    call  change_task        
    jmp   MEM_Heap_Lock_wait
MEM_Heap_Lock_end1:
    inc   dword [MEM_mutex_count]
    pop   eax
    ret
MEM_Heap_Lock_end:
    call  MEM_get_pid
    mov   [MEM_mutex_pid],eax
    mov   dword [MEM_mutex_count],1
    pop   eax
    ret
else
    pushfd
    cli
    inc   dword [MEM_cli_count]  
    jz    MEM_Heap_First_Lock
    add   esp,4
    ret
MEM_Heap_First_Lock:             ;save interrupt flag
    shr  dword [esp],9
    and  dword [esp],1
    pop  dword [MEM_cli_prev]
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Heap_UnLock
;;After this routine operations with heap are allowed.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MEM_Heap_UnLock:
if iskernel = 0
    push  eax
    xor   eax,eax
    dec   dword [MEM_mutex_count]
    jnz   MEM_Heap_UnLock_No_Wait1
    dec   dword [MEM_call_count]
    mov   [MEM_mutex_pid],eax
    mov   [MEM_general_mutex],eax;release mutex BEFORE task switching
    jz    MEM_Heap_UnLock_No_Wait
    call  change_task            ;someone want to use heap - switch tasks            
MEM_Heap_UnLock_No_Wait:
    pop   eax
    ret
MEM_Heap_UnLock_No_Wait1:
    dec   dword [MEM_call_count]
    jz    MEM_Heap_UnLock_No_Wait2
    call  change_task
MEM_Heap_UnLock_No_Wait2:
    pop   eax
    ret
else
    dec   dword [MEM_cli_count]
    js    MEM_Heap_UnLock_last
    ret
MEM_Heap_UnLock_last:
    cmp   dword [MEM_cli_prev],0 ;restore saved interrupt flag
    jz    MEM_Heap_UnLock_No_sti
    sti
MEM_Heap_UnLock_No_sti:
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Add_Heap
;;Add new range to memory manager.
;;eax - linear address
;;ebx - size in pages
;;ecx - physical address
;;Result:
;; eax=1 - success
;; eax=0 - failed
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MEM_Add_Heap:
    push  edx
    call  MEM_Heap_Lock
    mov   edx,[MEM_heap_count]
    cmp   edx,max_heaps
    jz    MEM_Add_Heap_Error
    inc   dword [MEM_heap_count]
    shl   edx,4
    mov   [MEM_heap_block+edx+.heap_linear_address],eax
    mov   [MEM_heap_block+edx+.heap_block_size],ebx
    shl   dword [MEM_heap_block+edx+.heap_block_size],12
    mov   [MEM_heap_block+edx+.heap_physical_address],ecx
    lea   edx,[4*ebx+.range_info+4095]   ;calculate space for page info table
    and   edx,0xFFFFF000
    mov   [eax],eax
    add   [eax],edx              ;first 4 bytes - pointer to first free page
;clean page info area    
    push  edi
    lea   edi,[eax+4]
    mov   ecx,edx
    shr   ecx,2
    push  eax
    xor   eax,eax
    rep   stosd                          
    pop   eax
    pop   edi
;create free pages list.    
    mov   ecx,[eax]
    shl   ebx,12
    add   eax,ebx                ;eax - address after block
MEM_Add_Heap_loop:
    add   ecx,4096
    mov   [ecx-4096],ecx         ;set forward pointer
    cmp   ecx,eax
    jnz   MEM_Add_Heap_loop
    mov   dword [ecx-4096],0     ;set end of list
MEM_Add_Heap_ret:
    call  MEM_Heap_UnLock
    pop   edx    
    ret
MEM_Add_Heap_Error:
    xor   eax,eax
    jmp   MEM_Add_Heap_ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Get_Physical_Address
;;Translate linear address to physical address
;;Parameters:
;; eax - linear address
;;Result:
;; eax - physical address
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Get_Physical_Address
MEM_Get_Physical_Address:
    push   ecx
    call   MEM_Heap_Lock
    mov    ecx,[MEM_heap_count]
    dec    ecx
    shl    ecx,4
MEM_Get_Physical_Address_loop:
    sub    eax,[MEM_heap_block+ecx+.heap_linear_address]
    jl     MEM_Get_Physical_Address_next
    cmp    eax,[MEM_heap_block+ecx+.heap_block_size]
    jge    MEM_Get_Physical_Address_next
    add    eax,[MEM_heap_block+ecx+.heap_physical_address]
    jmp    MEM_Get_Physical_Address_loopend
MEM_Get_Physical_Address_next:
    add    eax,[MEM_heap_block+ecx+.heap_linear_address]
    sub    ecx,16
    jns    MEM_Get_Physical_Address_loop
    xor    eax,eax               ;address not found
MEM_Get_Physical_Address_loopend:
    call   MEM_Heap_UnLock
    pop    ecx
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Get_Linear_Address
;;Translate physical address to linear address.
;;Parameters:
;; eax - physical address
;;Result:
;; eax - linear address
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Get_Linear_Address
MEM_Get_Linear_Address:
    push   ecx
    call   MEM_Heap_Lock
    mov    ecx,[MEM_heap_count]
    dec    ecx
    shl    ecx,4
MEM_Get_Linear_Address_loop:
    sub    eax,[MEM_heap_block+ecx+.heap_physical_address]
    jl     MEM_Get_Linear_Address_Next
    cmp    eax,[MEM_heap_block+ecx+.heap_block_size]
    jge    MEM_Get_Linear_Address_Next
    add    eax,[MEM_heap_block+ecx+.heap_linear_address]
    call   MEM_Heap_UnLock
    pop    ecx
    ret
MEM_Get_Linear_Address_Next:
    add    eax,[MEM_heap_block+ecx+.heap_physical_address]
    sub    ecx,16
    jns    MEM_Get_Linear_Address_loop
    call   MEM_Heap_UnLock
    pop    ecx
    xor    eax,eax               ;address not found
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Alloc_Page
;;Allocate and add reference to page
;;Result:
;; eax<>0 - physical address of page
;; eax=0  - not enough memory
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Alloc_Page
MEM_Alloc_Page:
    push   ecx
    call   MEM_Heap_Lock
    mov    ecx,[MEM_heap_count]
    dec    ecx
    shl    ecx,4
MEM_Alloc_Page_loop:
    push   ecx
    mov    ecx,[MEM_heap_block+ecx+.heap_linear_address]
    cmp    dword [ecx],0
    jz     MEM_Alloc_Page_loopend
    mov    eax,[ecx]
    push   dword [eax]
    pop    dword [ecx]
    sub    eax,ecx
    push   eax
    shr    eax,10
    mov    word [ecx+.range_info+eax],1
    pop    eax
    pop    ecx
    add    eax,[MEM_heap_block+ecx+.heap_physical_address]
    jmp    MEM_Alloc_Page_ret
MEM_Alloc_Page_loopend:
    pop    ecx
    sub    ecx,16
    jns    MEM_Alloc_Page_loop
    xor    eax,eax
MEM_Alloc_Page_ret:
    call   MEM_Heap_UnLock
    pop    ecx
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Alloc_Page_Linear
;;Allocate and add reference to page
;;Result:
;; eax<>0 - linear address of page
;; eax=0  - not enough memory
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Alloc_Page_Linear
MEM_Alloc_Page_Linear:
    push  ecx
    call  MEM_Heap_Lock
    mov   ecx,[MEM_heap_count]
    dec   ecx
    shl   ecx,4
MEM_Alloc_Page_Linear_loop:
    push  ecx
    mov   ecx,[MEM_heap_block+ecx+.heap_linear_address]
    cmp   dword [ecx],0
    jz    MEM_Alloc_Page_Linear_loopend
    mov   eax,[ecx]
    push  dword [eax]
    pop   dword [ecx]
    push  eax
    sub   eax,ecx
    shr   eax,10
    mov   word [ecx+.range_info+eax],1
    pop   eax
    pop   ecx
    jmp   MEM_Alloc_Page_Linear_ret
MEM_Alloc_Page_Linear_loopend:
    pop   ecx
    sub   ecx,16
    jns   MEM_Alloc_Page_Linear_loop
    xor   eax,eax
MEM_Alloc_Page_Linear_ret:
    call  MEM_Heap_UnLock
    pop   ecx
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Free_Page
;;Remove reference and free page if number of 
;;references is equal to 0
;;Parameters:
;;  eax - physical address of page
;;Result:
;;  eax - 1 success
;;  eax - 0 failed
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Free_Page
MEM_Free_Page:
    test  eax,eax
    jz    MEM_Free_Page_Zero
    test  eax,0xFFF
    jnz   MEM_Free_Page_Not_Aligned
    push  ebx
    push  ecx
    push  edx
    call  MEM_Heap_Lock
    mov   ecx,[MEM_heap_count]
    dec   ecx
    shl   ecx,4
MEM_Free_Page_Heap_loop:
    sub   eax,[MEM_heap_block+ecx+.heap_physical_address]
    js    MEM_Free_Page_Heap_loopnext
    cmp   eax,[MEM_heap_block+ecx+.heap_block_size]
    jl    MEM_Free_Page_Heap_loopend
MEM_Free_Page_Heap_loopnext:
    add   eax,[MEM_heap_block+ecx+.heap_physical_address]
    sub   ecx,16
    jns   MEM_Free_Page_Heap_loop
    xor   eax,eax
    inc   eax
    jmp   MEM_Free_Page_ret
MEM_Free_Page_Heap_loopend:
    mov   ecx,[MEM_heap_block+ecx+.heap_linear_address]
    mov   ebx,eax
    add   eax,ecx
    shr   ebx,10
    mov   edx,[ecx+.range_info+ebx]
    test  edx,0x80000000
    jnz   MEM_Free_Page_Bucket
    test  dx,dx
    jz    MEM_Free_Page_Error
    dec   word [ecx+.range_info+ebx]
    jnz   MEM_Free_Page_OK
MEM_Free_Page_Bucket:
    push  dword [ecx]
    mov   [ecx],eax
    pop   dword [eax]
    mov   dword [ecx+.range_info+ebx],0
MEM_Free_Page_OK:
    mov   eax,1
MEM_Free_Page_ret:    
    call  MEM_Heap_UnLock
    pop   edx
    pop   ecx
    pop   ebx
    ret
MEM_Free_Page_Error:
    xor   eax,eax
    jmp   MEM_Free_Page_ret
MEM_Free_Page_Zero:
    inc   eax
    ret
MEM_Free_Page_Not_Aligned:
    xor   eax,eax
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Free_Page_Linear
;;Remove reference and free page if number of 
;;references is equal to 0
;;Parameters:
;;  eax - linear address of page
;;Result:
;;  eax - 1 success
;;  eax - 0 failed
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Free_Page_Linear
MEM_Free_Page_Linear:
    test  eax,eax
    jz    MEM_Free_Page_Linear_Zero
    test  eax,0xFFF
    jnz   MEM_Free_Page_Linear_Not_Aligned
    push  ebx
    push  ecx
    push  edx
    call  MEM_Heap_Lock
    mov   ecx,[MEM_heap_count]
    dec   ecx
    shl   ecx,4
    
MEM_Free_Page_Linear_Heap_loop:
    sub   eax,[MEM_heap_block+ecx+.heap_linear_address]
    js    MEM_Free_Page_Linear_Heap_loopnext
    cmp   eax,[MEM_heap_block+ecx+.heap_block_size]
    jl    MEM_Free_Page_Linear_Heap_loopend
MEM_Free_Page_Linear_Heap_loopnext:
    add   eax,[MEM_heap_block+ecx+.heap_linear_address]
    sub   ecx,16
    jns   MEM_Free_Page_Linear_Heap_loop
    xor   eax,eax
    inc   eax
    jmp   MEM_Free_Page_Linear_ret
    
MEM_Free_Page_Linear_Heap_loopend:
    mov   ecx,[MEM_heap_block+ecx+.heap_linear_address]
    mov   ebx,eax
    add   eax,ecx   
    shr   ebx,10
    mov   edx,[ecx+.range_info+ebx]
    test  edx,0x80000000
    jnz   MEM_Free_Page_Linear_Bucket
    test  dx,dx
    jz    MEM_Free_Page_Linear_Error
    dec   word [ecx+.range_info+ebx]
    jnz   MEM_Free_Page_Linear_OK
MEM_Free_Page_Linear_Bucket:
    mov   edx,[ecx]
    mov   [eax],edx
    mov   dword [eax+4],0
    mov   [ecx],eax
    test  edx,edx
    jz    MEM_Free_Page_No_Next
    mov   [edx+4],eax
MEM_Free_Page_No_Next:
    mov   dword [ecx+.range_info+ebx],0
MEM_Free_Page_Linear_OK:
    xor   eax, eax
    inc   eax
MEM_Free_Page_Linear_ret:    
    call  MEM_Heap_UnLock
    pop   edx
    pop   ecx
    pop   ebx
    ret
    
MEM_Free_Page_Linear_Error:
    xor   eax,eax
    jmp   MEM_Free_Page_Linear_ret
    
MEM_Free_Page_Linear_Zero:
    inc   eax
    ret
    
MEM_Free_Page_Linear_Not_Aligned:
    xor   eax,eax
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Alloc_Pages
;;Allocates set of pages.
;;Parameters:
;; eax - number of pages
;; ebx - buffer for physical addresses
;;Result:
;; eax - number of allocated pages
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Alloc_Pages
MEM_Alloc_Pages:
    push  eax
    push  ebx
    push  ecx
    mov   ecx,eax
    test  ecx,ecx
    jz    MEM_Alloc_Pages_ret
MEM_Alloc_Pages_loop:
    call  MEM_Alloc_Page
    test  eax,eax
    jz    MEM_Alloc_Pages_ret
    mov   [ebx],eax
    add   ebx,4
    dec   ecx
    jnz   MEM_Alloc_Pages_loop
MEM_Alloc_Pages_ret:
    sub   [esp+8],ecx
    pop   ecx
    pop   ebx
    pop   eax
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Alloc_Pages_Linear
;;Allocates set of pages.
;;Parameters:
;; eax - number of pages
;; ebx - buffer for linear addresses
;;Result:
;; eax - number of allocated pages
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Alloc_Pages_Linear
MEM_Alloc_Pages_Linear:
    push  eax
    push  ebx
    push  ecx
    mov   ecx,eax
    test  ecx,ecx
    jz    MEM_Alloc_Pages_Linear_ret
MEM_Alloc_Pages_Linear_loop:
    call  MEM_Alloc_Page_Linear
    test  eax,eax
    jz    MEM_Alloc_Pages_Linear_ret
    mov   [ebx],eax
    add   ebx,4
    dec   ecx
    jnz   MEM_Alloc_Pages_Linear_loop
MEM_Alloc_Pages_Linear_ret:
    sub   [esp+8],ecx
    pop   ecx
    pop   ebx
    pop   eax
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Free_Pages
;;Parameters:
;; eax - number of pages
;; ebx - array of addresses
;;Result:
;; eax=1 - succcess
;; eax=0 - failed
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Free_Pages
MEM_Free_Pages:
    push  ebx
    push  ecx
    mov   ecx,eax
    test  ecx,ecx
    jz    MEM_Free_Pages_ret
MEM_Free_Pages_loop:
    mov   eax,[ebx]
    call  MEM_Free_Page
    add   ebx,4
    test  eax,eax
    jz    MEM_Free_Pages_ret
    dec   ecx
    jnz   MEM_Free_Pages_loop
MEM_Free_Pages_ret:
    pop   ecx
    pop   ebx
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Free_Pages_Linear
;;Parameters:
;; eax - number of pages
;; ebx - array of addresses
;;Result:
;; eax=1 - succcess
;; eax=0 - failed
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Free_Pages_Linear
MEM_Free_Pages_Linear:
    push  ebx
    push  ecx
    mov   ecx,eax
    test  ecx,ecx
    jz    MEM_Free_Pages_Linear_ret
MEM_Free_Pages_Linear_loop:
    mov   eax,[ebx]
    call  MEM_Free_Page_Linear
    add   ebx,4
    test  eax,eax
    jz    MEM_Free_Pages_Linear_ret
    dec   ecx
    jnz   MEM_Free_Pages_Linear_loop
MEM_Free_Pages_Linear_ret:
    pop   ecx
    pop   ebx
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Get_Heap_Number
;;Calculate number of heap which pointer belongs to.
;;Parameter:
;; eax - address
;;Result:
;; ecx - number of heap*16.
;; eax=0 if address not found.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Get_Heap_Number
MEM_Get_Heap_Number:
    call  MEM_Heap_Lock
    mov   ecx,[MEM_heap_count]
    dec   ecx
    shl   ecx,4
MEM_Get_Heap_loop:
    sub   eax,[MEM_heap_block+ecx+.heap_physical_address]
    jl    MEM_Get_Heap_loopnext
    cmp   eax,[MEM_heap_block+ecx+.heap_block_size]
    jl    MEM_Get_Heap_loopend
MEM_Get_Heap_loopnext:
    add   eax,[MEM_heap_block+ecx+.heap_physical_address]
    sub   ecx,16
    jns   MEM_Get_Heap_loop
    call  MEM_Heap_UnLock
    xor   eax,eax
    ret
MEM_Get_Heap_loopend:
    add   eax,[MEM_heap_block+ecx+.heap_physical_address]
    call  MEM_Heap_UnLock
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Get_Heap_Number_Linear
;;Calculate number of heap which pointer belongs to.
;;Parameter:
;; eax - address
;;Result:
;; ecx - number of heap*16.
;; eax=0 if address not found.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Get_Heap_Number_Linear
MEM_Get_Heap_Number_Linear:
    call  MEM_Heap_Lock
    mov   ecx,[MEM_heap_count]
    dec   ecx
    shl   ecx,4
MEM_Get_Heap_Linear_loop:
    sub   eax,[MEM_heap_block+ecx+.heap_linear_address]
    jl    MEM_Get_Heap_Linear_loopnext
    cmp   eax,[MEM_heap_block+ecx+.heap_block_size]
    jl    MEM_Get_Heap_Linear_loopend
MEM_Get_Heap_Linear_loopnext:
    add   eax,[MEM_heap_block+ecx+.heap_linear_address]
    sub   ecx,16
    jns   MEM_Get_Heap_Linear_loop
    call  MEM_Heap_UnLock
    xor   eax,eax
    ret
MEM_Get_Heap_Linear_loopend:
    add   eax,[MEM_heap_block+ecx+.heap_linear_address]
    call  MEM_Heap_UnLock
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Alloc
;;Allocate small region.
;;Parameters:
;; eax - size (0<eax<=4096)
;;Result:
;; eax - linear address
;; eax=0 - not enough memory
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Alloc
MEM_Alloc:
;find chain
    test  eax,eax
    jng   MEM_Alloc_Wrong_Size
    cmp   eax,4096
    jg    MEM_Alloc_Wrong_Size
    push  ebx
    push  ecx
    push  edx
    push  esi
    dec   eax
    shr   eax,4
    xor   edx,edx
MEM_Alloc_Find_Size:
    add   edx,4
    shr   eax,1
    jnz   MEM_Alloc_Find_Size
MEM_Alloc_Size_Found:
    mov   ecx,edx
    shr   ecx,2
    add   ecx,4
    mov   eax,1
    shl   eax,cl
    mov   esi,eax
;esi - block size
;edx - offset
    call  MEM_Heap_Lock
    mov   ecx,[MEM_heap_count]
    dec   ecx
    shl   ecx,4
MEM_Alloc_Find_Heap:
    mov   eax,[MEM_heap_block+ecx+.heap_linear_address]
    cmp   dword [eax+edx],0
    jnz   MEM_Alloc_Use_Existing
    sub   ecx,16
    jns   MEM_Alloc_Find_Heap
;create new bucket page
    call  MEM_Alloc_Page_Linear
    call  MEM_Get_Heap_Number_Linear
    mov   ecx,[MEM_heap_block+ecx+.heap_linear_address]
    mov   [ecx+edx],eax
    lea   ebx,[eax+4096]
MEM_Alloc_List_loop:
    mov   [eax],eax
    mov   [eax+4],eax
    add   [eax],esi
    sub   [eax+4],esi
    add   eax,esi
    cmp   eax,ebx
    jnz   MEM_Alloc_List_loop
    sub   ebx,esi
    mov   dword [ebx],0
    sub   eax,4096
    mov   dword [eax+4],0
    mov   eax,ecx
        
MEM_Alloc_Use_Existing:
    mov   ebx,eax
    mov   eax,[eax+edx]
    mov   ecx,[eax]
    mov   [ebx+edx],ecx
    test  ecx,ecx
    jz    MEM_Alloc_Became_Empty
    mov   dword [ecx+4],0
MEM_Alloc_Became_Empty:
    mov   ecx,eax
    sub   ecx,ebx
    shr   ecx,10
    and   ecx,0xFFFFFFFC
    inc   byte [ebx+.range_info+ecx+2]
    shr   edx,2
    add   edx,128
    dec   edx
    mov   [ebx+.range_info+ecx+3],dl
    
MEM_Alloc_ret:
    call  MEM_Heap_UnLock
    pop   esi
    pop   edx
    pop   ecx
    pop   ebx
    ret
MEM_Alloc_Wrong_Size:
    xor   eax,eax
    ret 
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Free
;;Parameters:
;; eax - linear address
;;Result:
;; eax=1 - success
;; eax=0 - failed
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Free
MEM_Free:
    test  eax,eax
    jz    MEM_Free_Zero
    push  ebx
    push  ecx
    push  edx
    push  esi
    push  edi
    push  ebp
    call  MEM_Heap_Lock
    call  MEM_Get_Heap_Number_Linear
    test  eax,eax
    jz    MEM_Free_ret
    mov   edx,eax
    mov   ecx,[MEM_heap_block+ecx+.heap_linear_address]
    sub   edx,ecx
    shr   edx,10
    and   edx,0xFFFFFFFC
    mov   ebx,[ecx+.range_info+edx]
    mov   esi,ebx
    shr   esi,24
    sub   esi,128
    mov   edi,[ecx+4+4*esi]
    mov   [eax],edi
    mov   dword [eax+4],0
    test  edi,edi
    jz    MEM_Free_Empty_List
    mov   [edi+4],eax
MEM_Free_Empty_List:
    mov   [ecx+4+4*esi],eax
    sub   ebx,0x10000
    mov   [ecx+.range_info+edx],ebx
    test  ebx,0xFF0000
    jnz   MEM_Free_ret
;delete empty blocks on the page
    lea   edx,[esi+5]
    and   eax,0xFFFFF000
    mov   edi,eax
    mov   eax,1
    xchg  ecx,edx
    shl   eax,cl
    mov   ecx,edx
    mov   edx,eax 
;edx - size of block
;edi - start of page
    mov   eax,edi
    lea   ebx,[eax+4096]
MEM_Free_Block_loop:
    cmp   dword [eax+4],0
    jnz   MEM_Free_Block_Not_First
    mov   ebp,dword [eax]
    mov   [ecx+4+4*esi],ebp
    test  ebp,ebp
    jz    MEM_Free_Block_Last
    mov   dword [ebp+4],0
MEM_Free_Block_Last:
    jmp   MEM_Free_Block_loop_end
MEM_Free_Block_Not_First:
    mov   ebp,dword [eax]
    push  ebp
    mov   ebp,dword [eax+4]
    pop   dword [ebp]
    mov   ebp,dword [eax]
    test  ebp,ebp
    jz    MEM_Free_Block_loop_end
    push  dword [eax+4]
    pop   dword [ebp+4]
;    jmp   MEM_Free_Block_loop_end
MEM_Free_Block_loop_end:
    add   eax,edx
    cmp   eax,ebx
    jnz   MEM_Free_Block_loop
    mov   eax,edi
    call  MEM_Free_Page_Linear    
MEM_Free_ret:
    call  MEM_Heap_UnLock
    pop   ebp
    pop   edi
    pop   esi
    pop   edx
    pop   ecx
    pop   ebx
    ret
MEM_Free_Zero:
    inc   eax
    ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Add_Reference
;; eax - physical address of page
;;Result:
;; eax=1 - success
;; eax=0 - failed
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Add_Reference
MEM_Add_Reference:
    push  ebx
    push  ecx
    call  MEM_Heap_Lock
    call  MEM_Get_Heap_Number
    test  eax,eax
    jz    MEM_Add_Reference_ret
    sub   eax,[MEM_heap_block+ecx+.heap_physical_address]
    mov   ecx,[MEM_heap_block+ecx+.heap_linear_address]
    shr   eax,10
    and   eax,0xFFFFFFFC
    test  dword [ecx+eax+.range_info],0x80000000
    jnz   MEM_Add_Reference_failed
    inc   dword [ecx+eax+.range_info]
MEM_Add_Reference_ret:   
    call  MEM_Heap_UnLock
    pop   ecx
    pop   ebx
    ret
MEM_Add_Reference_failed:
    xor   eax,eax
    jmp   MEM_Add_Reference_ret
end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MEM_Add_Reference_Linear
;; eax - linear address of page
;;Result:
;; eax=1 - success
;; eax=0 - failed
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if used MEM_Add_Reference_Linear
MEM_Add_Reference_Linear:
    push  ebx
    push  ecx
    call  MEM_Heap_Lock
    call  MEM_Get_Heap_Number_Linear
    test  eax,eax
    jz    MEM_Add_Reference_Linear_ret
    mov   ecx,[MEM_heap_block+ecx+.heap_linear_address]
    sub   eax,ecx
    shr   eax,10
    and   eax,0xFFFFFFFC
    test  dword [ecx+eax+.range_info],0x80000000
    jnz   MEM_Add_Reference_Linear_failed
    inc   dword [ecx+eax+.range_info]
    mov   eax,1
MEM_Add_Reference_Linear_ret:   
    call  MEM_Heap_UnLock
    pop   ecx
    pop   ebx
    ret
MEM_Add_Reference_Linear_failed:
    xor   eax,eax
    jmp   MEM_Add_Reference_Linear_ret
end if
end if ;memmanager.inc
