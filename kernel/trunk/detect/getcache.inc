;cache_ide0_pointer          dd 0
;cache_ide0_size             dd 0   ; not use
;cache_ide0_data_pointer     dd 0
;cache_ide0_system_data_size dd 0   ; not use
;cache_ide0_appl_data_size   dd 0   ; not use
;cache_ide0_system_data      dd 0
;cache_ide0_appl_data        dd 0
;cache_ide0_system_sad_size  dd 0
;cache_ide0_appl_sad_size    dd 0

    pusha

    mov  eax,[pg_data.pages_free]
;   1/32
    shr eax,5
;   round off up to 8 pages
    shr eax,3
    shl eax,3
;   translate pages in butes *4096
    shl eax,12
;	check a upper size of the cache, no more than 1 Mb on the physical device
    cmp  eax,1024*1024
    jbe  @f
    mov  eax,1024*1024
    jmp  .continue
@@:
;	check a lower size of the cache, not less than 128 Kb on the physical device
    cmp  eax,128*1024
    jae  @f
    mov  eax,128*1024
@@:
.continue:
    mov  [cache_ide0_size],eax
    mov  [cache_ide1_size],eax
    mov  [cache_ide2_size],eax
    mov  [cache_ide3_size],eax
    xor  eax,eax
    mov  [cache_ide0_search_start],eax
    mov  [cache_ide0_appl_search_start],eax
    mov  [cache_ide1_search_start],eax
    mov  [cache_ide1_appl_search_start],eax
    mov  [cache_ide2_search_start],eax
    mov  [cache_ide2_appl_search_start],eax
    mov  [cache_ide3_search_start],eax
    mov  [cache_ide3_appl_search_start],eax
    mov  [hdd_appl_data],1  ;al
    
    mov  cl,[DRIVE_DATA+1]
    mov  ch,cl
    and  cl,11b
    cmp cl,0
    je  .ide2
    call get_cache_ide3
.ide2:
    mov  cl,ch
    and  cl,1100b
    cmp cl,0
    je  .ide1
    call get_cache_ide2
.ide1:
    mov  cl,ch
    and  cl,110000b
    cmp cl,0
    je  .ide0
    call get_cache_ide1
.ide0:
    mov  cl,ch
    and  cl,11000000b
    cmp cl,0
    je  end_get_cache
    call get_cache_ide0
    jmp  end_get_cache

get_cache_ide0:
    push  ecx
    stdcall kernel_alloc,[cache_ide0_size]
    mov [cache_ide0_pointer],eax
    pop   ecx
    mov  edx,eax
    mov  eax,[cache_ide0_size]
    shr  eax,3
    mov  [cache_ide0_system_data_size],eax
    mov  ebx,eax
    imul eax,7
    mov  [cache_ide0_appl_data_size],eax
    add  ebx,edx
    mov  [cache_ide0_data_pointer],ebx
    
    cmp cl,10000000b
    je  .cd
    push ecx
    mov  eax,[cache_ide0_system_data_size]
    call calculate_for_hd
    add  eax,[cache_ide0_pointer]
    mov  [cache_ide0_system_data],eax
    mov  [cache_ide0_system_sad_size],ecx
    
    push  edi
    mov   edi,[cache_ide0_pointer]
    call  clear_ide_cache
    pop   edi
    
    mov  eax,[cache_ide0_appl_data_size]
    call calculate_for_hd
    add  eax,[cache_ide0_data_pointer]
    mov  [cache_ide0_appl_data],eax
    mov  [cache_ide0_appl_sad_size],ecx

    push  edi
    mov   edi,[cache_ide0_data_pointer]
    call  clear_ide_cache
    pop   edi
    
    pop  ecx
    ret
.cd:
    push ecx
    mov  eax,[cache_ide0_system_data_size]
    call calculate_for_cd
    add  eax,[cache_ide0_pointer]
    mov  [cache_ide0_system_data],eax
    mov  [cache_ide0_system_sad_size],ecx

    push  edi
    mov   edi,[cache_ide0_pointer]
    call  clear_ide_cache
    pop   edi
    
    mov  eax,[cache_ide0_appl_data_size]
    call calculate_for_cd
    add  eax,[cache_ide0_data_pointer]
    mov  [cache_ide0_appl_data],eax
    mov  [cache_ide0_appl_sad_size],ecx
    
    push  edi
    mov   edi,[cache_ide0_data_pointer]
    call  clear_ide_cache
    pop   edi
    
    pop  ecx
    ret
    
get_cache_ide1:
    push  ecx
    stdcall kernel_alloc,[cache_ide1_size]
    mov [cache_ide1_pointer],eax
    pop   ecx
    mov  edx,eax
    mov  eax,[cache_ide1_size]
    shr  eax,3
    mov  [cache_ide1_system_data_size],eax
    mov  ebx,eax
    imul eax,7
    mov  [cache_ide1_appl_data_size],eax
    add  ebx,edx
    mov  [cache_ide1_data_pointer],ebx
    
    cmp cl,100000b
    je  .cd
    push ecx
    mov  eax,[cache_ide1_system_data_size]
    call calculate_for_hd
    add  eax,[cache_ide1_pointer]
    mov  [cache_ide1_system_data],eax
    mov  [cache_ide1_system_sad_size],ecx

    push  edi
    mov   edi,[cache_ide1_pointer]
    call  clear_ide_cache
    pop   edi

    mov  eax,[cache_ide1_appl_data_size]
    call calculate_for_hd
    add  eax,[cache_ide1_data_pointer]
    mov  [cache_ide1_appl_data],eax
    mov  [cache_ide1_appl_sad_size],ecx
    
    push  edi
    mov   edi,[cache_ide1_data_pointer]
    call  clear_ide_cache
    pop   edi
    
    pop  ecx
    ret
.cd:
    push ecx
    mov  eax,[cache_ide1_system_data_size]
    call calculate_for_cd
    add  eax,[cache_ide1_pointer]
    mov  [cache_ide1_system_data],eax
    mov  [cache_ide1_system_sad_size],ecx

    push  edi
    mov   edi,[cache_ide1_pointer]
    call  clear_ide_cache
    pop   edi

    mov  eax,[cache_ide1_appl_data_size]
    call calculate_for_cd
    add  eax,[cache_ide1_data_pointer]
    mov  [cache_ide1_appl_data],eax
    mov  [cache_ide1_appl_sad_size],ecx
    
    push  edi
    mov   edi,[cache_ide1_data_pointer]
    call  clear_ide_cache
    pop   edi
    
    pop  ecx
    ret

get_cache_ide2:
    push  ecx
    stdcall kernel_alloc,[cache_ide2_size]
    mov [cache_ide2_pointer],eax
    pop   ecx
    mov  edx,eax
    mov  eax,[cache_ide2_size]
    shr  eax,3
    mov  [cache_ide2_system_data_size],eax
    mov  ebx,eax
    imul eax,7
    mov  [cache_ide2_appl_data_size],eax
    add  ebx,edx
    mov  [cache_ide2_data_pointer],ebx
    
    cmp cl,1000b
    je  .cd
    push ecx
    mov  eax,[cache_ide2_system_data_size]
    call calculate_for_hd
    add  eax,[cache_ide2_pointer]
    mov  [cache_ide2_system_data],eax
    mov  [cache_ide2_system_sad_size],ecx

    push  edi
    mov   edi,[cache_ide2_pointer]
    call  clear_ide_cache
    pop   edi

    mov  eax,[cache_ide2_appl_data_size]
    call calculate_for_hd
    add  eax,[cache_ide2_data_pointer]
    mov  [cache_ide2_appl_data],eax
    mov  [cache_ide2_appl_sad_size],ecx
    
    push  edi
    mov   edi,[cache_ide2_data_pointer]
    call  clear_ide_cache
    pop   edi
    
    pop  ecx
    ret
.cd:
    push ecx
    mov  eax,[cache_ide2_system_data_size]
    call calculate_for_cd
    add  eax,[cache_ide2_pointer]
    mov  [cache_ide2_system_data],eax
    mov  [cache_ide2_system_sad_size],ecx

    push  edi
    mov   edi,[cache_ide2_pointer]
    call  clear_ide_cache
    pop   edi

    mov  eax,[cache_ide2_appl_data_size]
    call calculate_for_cd
    add  eax,[cache_ide2_data_pointer]
    mov  [cache_ide2_appl_data],eax
    mov  [cache_ide2_appl_sad_size],ecx
    
    push  edi
    mov   edi,[cache_ide2_data_pointer]
    call  clear_ide_cache
    pop   edi
    
    pop  ecx
    ret

get_cache_ide3:
    push  ecx
    stdcall kernel_alloc,[cache_ide3_size]
    mov [cache_ide3_pointer],eax
    pop   ecx
    mov  edx,eax
    mov  eax,[cache_ide3_size]
    shr  eax,3
    mov  [cache_ide3_system_data_size],eax
    mov  ebx,eax
    imul eax,7
    mov  [cache_ide3_appl_data_size],eax
    add  ebx,edx
    mov  [cache_ide3_data_pointer],ebx
    
    cmp cl,10b
    je  .cd
    push ecx
    mov  eax,[cache_ide3_system_data_size]
    call calculate_for_hd
    add  eax,[cache_ide3_pointer]
    mov  [cache_ide3_system_data],eax
    mov  [cache_ide3_system_sad_size],ecx

    push  edi
    mov   edi,[cache_ide3_pointer]
    call  clear_ide_cache
    pop   edi

    mov  eax,[cache_ide3_appl_data_size]
    call calculate_for_hd
    add  eax,[cache_ide3_data_pointer]
    mov  [cache_ide3_appl_data],eax
    mov  [cache_ide3_appl_sad_size],ecx
    
    push  edi
    mov   edi,[cache_ide3_data_pointer]
    call  clear_ide_cache
    pop   edi
    
    pop  ecx
    ret
.cd:
    push ecx
    mov  eax,[cache_ide3_system_data_size]
    call calculate_for_cd
    add  eax,[cache_ide3_pointer]
    mov  [cache_ide3_system_data],eax
    mov  [cache_ide3_system_sad_size],ecx

    push  edi
    mov   edi,[cache_ide3_pointer]
    call  clear_ide_cache
    pop   edi

    mov  eax,[cache_ide3_appl_data_size]
    call calculate_for_cd
    add  eax,[cache_ide3_data_pointer]
    mov  [cache_ide3_appl_data],eax
    mov  [cache_ide3_appl_sad_size],ecx
    
    push  edi
    mov   edi,[cache_ide3_data_pointer]
    call  clear_ide_cache
    pop   edi
    
    pop  ecx
    ret

calculate_for_hd:
    push eax
    mov  ebx,eax
    shr  eax,9
    shl  eax,3
    sub  ebx,eax
    shr  ebx,9
    mov  ecx,ebx
    shl  ebx,9
    pop  eax
    sub  eax,ebx
    ret

calculate_for_cd:
    push eax
    mov  ebx,eax
    shr  eax,11
    shl  eax,3
    sub  ebx,eax
    shr  ebx,11
    mov  ecx,ebx
    shl  ebx,11
    pop  eax
    sub  eax,ebx
    ret
    
clear_ide_cache:
    push   eax
    shl   ecx,1
    xor   eax,eax
    cld
    rep   stosd
    pop   eax
    ret
    
end_get_cache:
;    mov  [cache_ide0_pointer],HD_CACHE
;    mov  [cache_ide0_system_data],HD_CACHE+65536
;    mov  [cache_ide0_system_sad_size],1919
    popa