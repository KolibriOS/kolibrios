;**************************************************************************
;
;   [cache_ide[X]_pointer]
;   or [cache_ide[X]_data_pointer]  first entry in cache list
;
;            +0   - lba sector
;            +4   - state of cache sector
;                   0 = empty
;                   1 = used for read  ( same as in hd )
;                   2 = used for write ( differs from hd )
;
;  [cache_ide[X]_system_data]
;  or [cache_ide[x]_appl_data] - cache entries
;
;**************************************************************************
align 4
write_cache:
;-----------------------------------------------------------
; write all changed sectors to disk
;-----------------------------------------------------------
    push  eax ecx edx esi edi

    ; write difference ( 2 ) from cache to hd

;    mov   ecx,cache_max
;    mov   esi,HD_CACHE+8
    call  calculate_cache
    add   esi,8
    
    mov   edi,1

  write_cache_more:

    cmp   dword [esi+4],2       ; if cache slot is not different
    jne   .write_chain

    mov   dword [esi+4],1       ; same as in hd
    mov   eax,[esi]             ; eax = sector to write

    cmp   eax,[PARTITION_START]
    jb    danger
    cmp   eax,[PARTITION_END]
    ja    danger

; DMA write is permitted only if [allow_dma_access]=1
        cmp     [allow_dma_access], 2
        jae     .nodma
        cmp     [dma_hdd], 1
        jnz     .nodma
; Объединяем запись цепочки последовательных секторов в одно обращение к диску
        cmp     ecx, 1
        jz      .nonext
        cmp     dword [esi+8+4], 2
        jnz     .nonext
        push    eax
        inc     eax
        cmp     eax, [esi+8]
        pop     eax
        jnz     .nonext
        cmp     [cache_chain_started], 1
        jz      @f
        mov     [cache_chain_started], 1
        mov     [cache_chain_size], 0
        mov     [cache_chain_pos], edi
        mov     [cache_chain_ptr], esi
@@:
        inc     [cache_chain_size]
        cmp     [cache_chain_size], 64
        jnz     .continue
        jmp     .write_chain
.nonext:
        call    flush_cache_chain
        mov     [cache_chain_size], 1
        mov     [cache_chain_ptr], esi
        call    write_cache_sector
        jmp     .continue
.nodma:
        call    cache_write_pio
.write_chain:
        call    flush_cache_chain

.continue:
  danger:

    add   esi,8
    inc   edi
    dec   ecx
    jnz   write_cache_more
        call    flush_cache_chain
 return_02:
    pop   edi esi edx ecx eax
    ret

flush_cache_chain:
        cmp     [cache_chain_started], 0
        jz      @f
        call    write_cache_chain
        mov     [cache_chain_started], 0
@@:
        ret
        

align 4
find_empty_slot:
;-----------------------------------------------------------
; find empty or read slot, flush cache if next 10% is used by write
; output : edi = cache slot
;-----------------------------------------------------------
;    push  ecx esi

  search_again:

;    mov   ecx,cache_max*10/100
;    mov   edi,[cache_search_start]
    call  calculate_cache_3
;    push  eax edx
;    mov   eax,ecx
;    mov   ecx,10
;    xor   edx,edx
;    div  ecx
;    mov   ecx,eax
;    pop   edx eax
    shr   ecx,3    
  search_for_empty:

    inc   edi
;    cmp   edi,cache_max
;    push   eax
    call  calculate_cache_4
;    cmp   edi,eax
;    pop   eax
    jbe   inside_cache
    mov   edi,1

  inside_cache:

;    cmp   dword [edi*8+HD_CACHE+4],2    ; get cache slot info
    push   esi
    call  calculate_cache_1
    cmp   dword [edi*8+esi+4],2
    pop   esi
    jb    found_slot                    ; it's empty or read
    dec   ecx
    jnz   search_for_empty

    call  write_cache                   ; no empty slots found, write all
    cmp   [hd_error],0
    jne   found_slot_access_denied

    jmp   search_again                  ; and start again

  found_slot:

;    mov   [cache_search_start],edi
    call  calculate_cache_5
  found_slot_access_denied:
    ret

align 4
clear_hd_cache:
;    push  eax ecx edi
;    mov   edi, HD_CACHE
;    mov   ecx,16384
;    xor   eax,eax
;    cld
;    rep   stosd                 ; clear hd cache with 0
;    mov   [cache_search_start],eax
    mov   [fat_in_cache],-1
    mov   [fat_change],0
;    pop   edi ecx eax
    ret
    
;--------------------------------------------------------------------
align 4
calculate_cache:
;    mov   ecx,cache_max         ; entries in cache
;    mov   esi,HD_CACHE+8

; 1 - IDE0 ... 4 - IDE3
.ide0:
    cmp    [hdpos],1
    jne    .ide1
    cmp    [hdd_appl_data],0
    jne    .ide0_appl_data
    mov    ecx,[cache_ide0_system_sad_size]
    mov    esi,[cache_ide0_pointer]
    ret
.ide0_appl_data:
    mov    ecx,[cache_ide0_appl_sad_size]
    mov    esi,[cache_ide0_data_pointer]
    ret
.ide1:
    cmp    [hdpos],2
    jne    .ide2
    cmp    [hdd_appl_data],0
    jne    .ide1_appl_data
    mov    ecx,[cache_ide1_system_sad_size]
    mov    esi,[cache_ide1_pointer]
    ret
.ide1_appl_data:
    mov    ecx,[cache_ide1_appl_sad_size]
    mov    esi,[cache_ide1_data_pointer]
    ret
.ide2:
    cmp    [hdpos],3
    jne    .ide3
    cmp    [hdd_appl_data],0
    jne    .ide2_appl_data
    mov    ecx,[cache_ide2_system_sad_size]
    mov    esi,[cache_ide2_pointer]    
    ret
.ide2_appl_data:
    mov    ecx,[cache_ide2_appl_sad_size]
    mov    esi,[cache_ide2_data_pointer]
    ret
.ide3:
    cmp    [hdd_appl_data],0
    jne    .ide3_appl_data
    mov    ecx,[cache_ide3_system_sad_size]
    mov    esi,[cache_ide3_pointer]      
    ret
.ide3_appl_data:
    mov    ecx,[cache_ide3_appl_sad_size]
    mov    esi,[cache_ide3_data_pointer]
    ret
;--------------------------------------------------------------------
align 4
calculate_cache_1:
;    lea   esi,[edi*8+HD_CACHE]
; 1 - IDE0 ... 4 - IDE3
.ide0:
    cmp    [hdpos],1
    jne    .ide1
    cmp    [hdd_appl_data],0
    jne    .ide0_appl_data
    mov    esi,[cache_ide0_pointer]
    ret
.ide0_appl_data:
    mov    esi,[cache_ide0_data_pointer]
    ret
.ide1:
    cmp    [hdpos],2
    jne    .ide2
    cmp    [hdd_appl_data],0
    jne    .ide1_appl_data
    mov    esi,[cache_ide1_pointer]
    ret
.ide1_appl_data:
    mov    esi,[cache_ide1_data_pointer]
    ret
.ide2:
    cmp    [hdpos],3
    jne    .ide3
    cmp    [hdd_appl_data],0
    jne    .ide2_appl_data
    mov    esi,[cache_ide2_pointer]
    ret
.ide2_appl_data:
    mov    esi,[cache_ide2_data_pointer]
    ret
.ide3:
    cmp    [hdd_appl_data],0
    jne    .ide3_appl_data
    mov    esi,[cache_ide3_pointer]    
    ret
.ide3_appl_data:
    mov    esi,[cache_ide3_data_pointer]
    ret
;--------------------------------------------------------------------
align 4
calculate_cache_2:
;    add   esi,HD_CACHE+65536
; 1 - IDE0 ... 4 - IDE3
.ide0:
    cmp    [hdpos],1
    jne    .ide1
    cmp    [hdd_appl_data],0
    jne    .ide0_appl_data
    mov    eax,[cache_ide0_system_data]    
    ret
.ide0_appl_data:
    mov    eax,[cache_ide0_appl_data]
    ret
.ide1:
    cmp    [hdpos],2
    jne    .ide2
    cmp    [hdd_appl_data],0
    jne    .ide1_appl_data
    mov    eax,[cache_ide1_system_data]
    ret
.ide1_appl_data:
    mov    eax,[cache_ide1_appl_data]
    ret
.ide2:
    cmp    [hdpos],3
    jne    .ide3
    cmp    [hdd_appl_data],0
    jne    .ide2_appl_data
    mov    eax,[cache_ide2_system_data]
    ret
.ide2_appl_data:
    mov    eax,[cache_ide2_appl_data]
    ret
.ide3:
    cmp    [hdd_appl_data],0
    jne    .ide3_appl_data
    mov    eax,[cache_ide3_system_data]    
    ret
.ide3_appl_data:
    mov    eax,[cache_ide3_appl_data]
    ret
;--------------------------------------------------------------------
align 4
calculate_cache_3:
;    mov   ecx,cache_max*10/100
;    mov   edi,[cache_search_start]

; 1 - IDE0 ... 4 - IDE3
.ide0:
    cmp    [hdpos],1
    jne    .ide1
    cmp    [hdd_appl_data],0
    jne    .ide0_appl_data
    mov    ecx,[cache_ide0_system_sad_size]
    mov    edi,[cache_ide0_search_start]
    ret
.ide0_appl_data:
    mov    ecx,[cache_ide0_appl_sad_size]
    mov    edi,[cache_ide0_appl_search_start]
    ret
.ide1:
    cmp    [hdpos],2
    jne    .ide2
    cmp    [hdd_appl_data],0
    jne    .ide1_appl_data
    mov    ecx,[cache_ide1_system_sad_size]
    mov    edi,[cache_ide1_search_start]
    ret
.ide1_appl_data:
    mov    ecx,[cache_ide1_appl_sad_size]
    mov    edi,[cache_ide1_appl_search_start]
    ret
.ide2:
    cmp    [hdpos],3
    jne    .ide3
    cmp    [hdd_appl_data],0
    jne    .ide2_appl_data
    mov    ecx,[cache_ide2_system_sad_size]
    mov    edi,[cache_ide2_search_start]    
    ret
.ide2_appl_data:
    mov    ecx,[cache_ide2_appl_sad_size]
    mov    edi,[cache_ide2_appl_search_start]
    ret
.ide3:
    cmp    [hdd_appl_data],0
    jne    .ide3_appl_data
    mov    ecx,[cache_ide3_system_sad_size]
    mov    edi,[cache_ide3_search_start]      
    ret
.ide3_appl_data:
    mov    ecx,[cache_ide3_appl_sad_size]
    mov    edi,[cache_ide3_appl_search_start]
    ret
;--------------------------------------------------------------------
align 4
calculate_cache_4:
;    cmp   edi,cache_max
; 1 - IDE0 ... 4 - IDE3
.ide0:
    cmp    [hdpos],1
    jne    .ide1
    cmp    [hdd_appl_data],0
    jne    .ide0_appl_data
    cmp   edi,[cache_ide0_system_sad_size]
    ret
.ide0_appl_data:
    cmp   edi,[cache_ide0_appl_sad_size]
    ret
.ide1:
    cmp    [hdpos],2
    jne    .ide2
    cmp    [hdd_appl_data],0
    jne    .ide1_appl_data
    cmp   edi,[cache_ide1_system_sad_size]
    ret
.ide1_appl_data:
    cmp   edi,[cache_ide1_appl_sad_size]
    ret
.ide2:
    cmp    [hdpos],3
    jne    .ide3
    cmp    [hdd_appl_data],0
    jne    .ide2_appl_data
    cmp   edi,[cache_ide2_system_sad_size]
    ret
.ide2_appl_data:
    cmp   edi,[cache_ide2_appl_sad_size]
    ret
.ide3:
    cmp    [hdd_appl_data],0
    jne    .ide3_appl_data
    cmp   edi,[cache_ide3_system_sad_size]
    ret
.ide3_appl_data:
    cmp   edi,[cache_ide3_appl_sad_size]
    ret
;--------------------------------------------------------------------
align 4
calculate_cache_5:
;    mov   [cache_search_start],edi
; 1 - IDE0 ... 4 - IDE3
.ide0:
    cmp    [hdpos],1
    jne    .ide1
    cmp    [hdd_appl_data],0
    jne    .ide0_appl_data
    mov   [cache_ide0_search_start],edi    
    ret
.ide0_appl_data:
    mov   [cache_ide0_appl_search_start],edi
    ret
.ide1:
    cmp    [hdpos],2
    jne    .ide2
    cmp    [hdd_appl_data],0
    jne    .ide1_appl_data
    mov   [cache_ide1_search_start],edi
    ret
.ide1_appl_data:
    mov   [cache_ide1_appl_search_start],edi
    ret
.ide2:
    cmp    [hdpos],3
    jne    .ide3
    cmp    [hdd_appl_data],0
    jne    .ide2_appl_data
    mov   [cache_ide2_search_start],edi
    ret
.ide2_appl_data:
    mov   [cache_ide2_appl_search_start],edi
    ret
.ide3:
    cmp    [hdd_appl_data],0
    jne    .ide3_appl_data
    mov   [cache_ide3_search_start],edi    
    ret
.ide3_appl_data:
    mov   [cache_ide3_appl_search_start],edi
    ret
;--------------------------------------------------------------------
align 4
calculate_linear_to_real:
    shr eax, 12
    mov eax, [page_tabs+eax*4] 
    and eax, 0xFFFFF000
    ret
