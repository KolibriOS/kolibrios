if ~defined mem_inc
mem_inc_fix:
mem_inc fix mem_inc_fix
;include "memmanag.inc"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;High-level memory management in MenuetOS. 
;;It uses memory manager in memmanager.inc 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
second_base_address=0xC0000000
std_application_base_address=0x10000000
general_page_table_ dd 0
general_page_table=general_page_table_+second_base_address
;-----------------------------------------------------------------------------
create_general_page_table:
;input
;  none
;output
;  none
;Procedure create general page directory and write
;it address to [general_page_table].
    pushad
    mov   eax,1                  ;alloc 1 page
    mov   ebx,general_page_table ;write address to [general_page_table]
    call  MEM_Alloc_Pages        ;allocate page directory
    mov   eax,[general_page_table]
    call  MEM_Get_Linear_Address ;eax - linear address of page directory
    mov   edi,eax
    mov   ebx,eax
    xor   eax,eax
    mov   ecx,4096/4
    cld
    rep   stosd                  ;clear page directory
    
    mov   eax,4
    mov   edx,eax
    call  MEM_Alloc_Pages        ;alloc page tables for 0x0-0x1000000 region 
    cmp   eax,edx
    jnz   $                      ;hang if not enough memory
    
;fill page tables
    xor   esi,esi  
    mov   ebp,7
    
.loop:
;esi - number of page in page directory
;ebp - current page address
;ebx - linear address of page directory
    mov   eax,[ebx+4*esi]
    add   dword [ebx+4*esi],7    ;add flags to address of page table
    call  MEM_Get_Linear_Address
;eax - linear address of page table     
    mov   ecx,4096/4
;ecx (counter) - number of pages in page table
;current address=4Mb*esi     
    cmp   esi,2
    jz    .start_lfb_map         ;lfb map begin at 0x800000
    cmp   esi,3
    jz    .end_lfb_map           ;lfb map end at 0xC00000
    jmp   .loop1
.start_lfb_map:
;current address=lfb address
    mov   ebp,[0x2f0000+0x9018]
    add   ebp,7                  ;add flags
    jmp   .loop1
.end_lfb_map:
;current address=linear address
    mov   ebp,12*0x100000+7
    
.loop1:
    mov   [eax],ebp              ;write page address (with flags) in page table 
    add   eax,4
    add   ebp,4096               ;size of page=4096 bytes
    loop  .loop1
    
    inc   esi                    ;next page directory entry
    cmp   esi,edx
    jnz   .loop
    
;map region 0x80000000-0x803fffff to 0x800000-0xcfffff
    mov   eax,1                  ;size of the region is 4Mb so only 1 page table needed
    mov   edx,ebx                ;ebx still contains linear address of the page directory
    add   ebx,0x800
    call  MEM_Alloc_Pages        ;alloc page table for the region
    mov   eax,[ebx]
    add   dword [ebx],7          ;add flags
    call  MEM_Get_Linear_Address ;get linear address of the page table
    mov   ebx,eax
    mov   ecx,4096/4             ;number of pages in page table
    mov   eax,8*0x100000+7
.loop3:
;ebx - linear address of page table
;eax - current linear address with flags
    mov   [ebx],eax
    add   ebx,4
    add   eax,4096
    loop  .loop3
    
;map region 0xC0000000-* to 0x0-*
    mov   esi,edx                ;esi=linear address of the page directory 
    lea   edi,[esi+(second_base_address shr 20)];add offset of entry (0xC00)
    mov   ecx,4
    rep   movsd                  ;first 16Mb of the region mapped as 0x0-0x1000000 block
    mov   eax,[0xfe8c]           ;eax=memory size
    add   eax,0x3fffff
    shr   eax,22                 
    mov   esi,eax         ;calculate number of entries in page directory
    sub   esi,4                  ;subtract entries for first 16Mb.
    mov   ebp,0x1000000+7        ;start physical address with flags
    
;mapping memory higher than 16Mb    
.loop4:
;esi (counter) - number of entries in page directory
;edi - address of entry
    test  esi,esi
    jle   .loop4end
    call  MEM_Alloc_Page         ;alloc page table for entry in page directory
    mov   [edi],eax
    add   dword [edi],7          ;write physical address of page table in page directory
    add   edi,4                  ;move entry pointer
    call  MEM_Get_Linear_Address
    mov   ecx,eax
    xor   edx,edx
    
.loop5:
;ecx - linear address of page table
;edx - index of page in page table
;ebp - current mapped physical address with flags
    mov   [ecx+4*edx],ebp        ;write address of page in page table
    add   ebp,0x1000             ;move to next page
    inc   edx
    cmp   edx,4096/4
    jl    .loop5
    
    dec   esi
    jmp   .loop4
.loop4end:    
   
.set_cr3:
;set value of cr3 register to the address of page directory
    mov   eax,[general_page_table]
    add   eax,8+16               ;add flags
    mov   cr3,eax                ;now we have full access paging
    
    popad
    ret
;-----------------------------------------------------------------------------
simple_clone_cr3_table:
;Parameters:
;  eax - physical address of cr3 table (page directory)
;result:
;  eax - physical address of clone of cr3 table.
;Function copy only page directory.
    push  ecx
    push  edx
    push  esi
    push  edi
    call  MEM_Get_Linear_Address
;eax - linear address of cr3 table    
    mov   esi,eax                
    call  MEM_Alloc_Page
    test  eax,eax
    jz    .failed
;eax - physical address of new page diretory
    mov   edx,eax                
    call  MEM_Get_Linear_Address    
    mov   edi,eax
    mov   ecx,4096/4
    cld
;esi - address of old page directory
;edi - address of new page directory     
    rep   movsd                  ;copy page directory
    mov   eax,edx
.failed:                    
    pop   edi
    pop   esi
    pop   edx
    pop   ecx
    ret
        
;-----------------------------------------------------------------------------    
create_app_cr3_table:
;Parameters:
;  eax - slot of process (index in 0x3000 table)
;result:
;  eax - physical address of table.
;This function create page directory for new process and
;write it physical address to offset 0xB8 of extended 
;process information.
    push  ebx

    mov   ebx,eax
    mov   eax,[general_page_table]
    call  simple_clone_cr3_table ;clone general page table
    shl   ebx,8
    mov   [second_base_address+0x80000+ebx+0xB8],eax ;save address of page directory
    
    pop   ebx
    ret
;-----------------------------------------------------------------------------
get_cr3_table:
;Input:
;  eax - slot of process    
;result:
;  eax - physical address of page directory
   shl    eax,8                  ;size of process extended information=256 bytes
   mov    eax,[second_base_address+0x80000+eax+0xB8]
   ret
;-----------------------------------------------------------------------------    
dispose_app_cr3_table:
;Input:
;  eax - slot of process
;result:
;  none
;This procedure frees page directory, 
;page tables and all memory of process.
    pushad
    mov   ebp,eax
;ebp = process slot in the procedure.
    shl   eax,8
    mov   eax,[second_base_address+0x80000+eax+0xB8]
    mov   ebx,eax
;ebx = physical address of page directory    
    call  MEM_Get_Linear_Address
    mov   edi,eax
;edi = linear address of page directory    
    mov   eax,[edi+(std_application_base_address shr 20)]
    and   eax,not (4096-1)
    call  MEM_Get_Linear_Address
    mov   esi,eax
;esi = linear address of first page table

;search threads
;    mov   ecx,0x200
    xor   edx,edx
    mov   eax,0x2
  
.loop:
;eax = current slot of process  
    mov   ecx,eax
    shl   ecx,5
    cmp   byte [second_base_address+0x3000+ecx+0xa],0 ;if process running?
    jnz   .next                  ;slot empty or process is terminating - go to next slot
    shl   ecx,3
    cmp   [second_base_address+0x80000+ecx+0xB8],ebx  ;compare page directory addresses
    jnz   .next
    inc   edx                    ;thread found
.next:
    inc   eax
    cmp   eax,[0x3004]           ;exit loop if we look through all processes
    jle   .loop
            
;edx = number of threads
;our process is zombi so it isn't counted
    test  edx,edx
    jnz   .threadsexists
;if there isn't threads then clear memory.
    add   edi,std_application_base_address shr 20
    
.loop1:    
;edi = linear address of current directory entry
;esi = linear address of current page table
    test  esi,esi
    jz    .loop1end
    xor   ecx,ecx
    
.loop2:
;ecx = index of page
    mov   eax,[esi+4*ecx]
    test  eax,eax
    jz    .loopend               ;skip empty entries
    and   eax,not (4096-1)       ;clear flags
    push  ecx
    call  MEM_Free_Page          ;free page
    pop   ecx
.loopend:    
    inc   ecx
    cmp   ecx,1024               ;there are 1024 pages in page table
    jl    .loop2
    
    mov   eax,esi
    call  MEM_Free_Page_Linear   ;free page table
.loop1end:    
    add   edi,4                  ;move to next directory entry
    mov   eax,[edi]
    and   eax,not (4096-1)
    call  MEM_Get_Linear_Address
    mov   esi,eax                ;calculate linear address of new page table
    test  edi,0x800
    jz    .loop1                 ;test if we at 0x80000000 address?
    
    and   edi,not (4096-1)       ;clear offset of page directory entry
    mov   eax,edi
    call  MEM_Free_Page_Linear   ;free page directory 
    popad
    ret
    
.threadsexists:                  ;do nothing   
    popad                        ;last thread will free memory
    ret    
;-----------------------------------------------------------------------------
mem_alloc_specified_region:
;eax - linear directory address
;ebx - start address (aligned to 4096 bytes)
;ecx - size in pages
;result:
; eax=1 - ok
; eax=0 - failed
;Try to alloc and map ecx pages to [ebx;ebx+4096*ecx) interval.
    pushad
    mov   ebp,ebx                ;save start address for recoil
    mov   esi,eax
.gen_loop:    
;esi = linear directory address
;ebx = current address
;ecx = remaining size in pages
    mov   edx,ebx
    shr   edx,22
    mov   edi,[esi+4*edx]        ;find directory entry for current address
    test  edi,edi
    jnz   .table_exists          ;check if page table allocated
    call  MEM_Alloc_Page         ;alloc page table
    test  eax,eax
    jz    .failed
    mov   [esi+4*edx],eax
    add   dword [esi+4*edx],7    ;write it address with flags
    call  MEM_Get_Linear_Address
    call  mem_fill_page          ;clear page table
    jmp   .table_linear        
.table_exists:
;calculate linear address of page table
    mov   eax,edi
    and   eax,not (4096-1)       ;clear flags
    call  MEM_Get_Linear_Address
.table_linear:
;eax = linear address of page table
    mov   edx,ebx
    shr   edx,12
    and   edx,(1024-1)           ;calculate index in page table
    mov   edi,eax
    
.loop:
;edi = linear address of page table
;edx = current page table index
;ecx = remaining size in pages
;ebx = current address
    test  ecx,ecx
    jle   .endloop1              ;all requested pages allocated
    
    call  MEM_Alloc_Page         ;alloc new page
    test  eax,eax
    jz    .failed
    mov   [edi+4*edx],eax
    add   dword [edi+4*edx],7    ;write it address with flags
    call  MEM_Get_Linear_Address
    call  mem_fill_page          ;clear new page
;go to next page table entry    
    dec   ecx
    add   ebx,4096
    inc   edx
    test  edx,(1024-1)
    jnz   .loop
    
    jmp   .gen_loop
     
.endloop1:                          
    popad
    mov   eax,1                  ;ok
    ret
    
.failed:
;calculate data for recoil
    sub   ebx,ebp
    shr   ebx,12
    mov   ecx,ebx                ;calculate number of allocated pages
    mov   eax,esi                ;restore linear address of page directory
    mov   ebx,ebp                ;restore initial address
    call  mem_free_specified_region ;free all allocated pages
    popad
    xor   eax,eax                ;fail
    ret
;-----------------------------------------------------------------------------    
mem_fill_page:
;Input:
;  eax - address
;result:
;  none
;set to zero 4096 bytes at eax address.
    push  ecx
    push  edi
    mov   edi,eax
    mov   ecx,4096/4
    xor   eax,eax
    rep   stosd
    lea   eax,[edi-4096]
    pop   edi
    pop   ecx
    ret
;-----------------------------------------------------------------------------    
mem_free_specified_region:
;eax - linear page directory address
;ebx - start address (aligned to 4096 bytes)
;ecx - size in pages
;result - none
;Free pages in [ebx;ebx+4096*ecx) region.
    pushad
    mov   esi,eax
    xor   ebp,ebp
    
.gen_loop:    
;esi = linear page directory address
;ebx = current address
;ecx = remaining pages
;ebp = 0 for first page table
;      1 otherwise
    mov   edx,ebx
    shr   edx,22
    mov   eax,[esi+4*edx]        ;find directory entry for current address
    and   eax,not (4096-1)
    test  eax,eax
    jnz   .table_exists          
;skip absent page tables    
    mov   edx,ebx
    shr   edx,12
    and   edx,(1024-1)           ;edx - index of current page
    add   ebx,1 shl 22
    add   ecx,edx
    and   ebx,not ((1 shl 22)-1)
    mov   ebp,1                  ;set flag
    sub   ecx,1024               ;ecx=ecx-(1024-edx)
    jg    .gen_loop
    popad
    ret
.table_exists:
    call  MEM_Get_Linear_Address
;eax - linear address of table
    mov   edx,ebx
    shr   edx,12
    and   edx,(1024-1)           ;edx - index of current page
    mov   edi,eax
    
.loop:
;edi = linear address of page table entry
;edx = index of page table entry
;ecx = remaining pages
    test  ecx,ecx
    jle   .endloop1
    
    mov   eax,[edi+4*edx]
    and   eax,not (4096-1)
    call  MEM_Free_Page          ;free page
    mov   dword [edi+4*edx],0    ;and clear page table entry
    dec   ecx
    inc   edx
    cmp   edx,1024
    jl    .loop
    
    test  ebp,ebp
    jz    .first_page
    mov   eax,edi                
    call  MEM_Free_Page_Linear   ;free page table
    mov   edx,ebx
    shr   edx,22
    mov   dword [esi+4*edx],0    ;and clear page directory entry
.first_page:
    add   ebx,1 shl 22
    and   ebx,not ((1 shl 22)-1) ;calculate new current address
    mov   ebp,1                  ;set flag
    jmp   .gen_loop 
    
.endloop1:   
    popad
    ret
end if
