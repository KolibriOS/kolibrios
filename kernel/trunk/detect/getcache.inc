
$Revision: 0 $

    pusha

    mov  eax,[pg_data.pages_free]
;   1/32
    shr eax,5
;   round off up to 8 pages
    shr eax,3
    shl eax,3
;   translate pages in butes *4096
    shl eax,12
;       check a upper size of the cache, no more than 1 Mb on the physical device
    cmp  eax,1024*1024
    jbe  @f
    mov  eax,1024*1024
    jmp  .continue
@@:
;       check a lower size of the cache, not less than 128 Kb on the physical device
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
    mov  [hdd_appl_data],1  ;al
    mov  [cd_appl_data],1
    
    mov  ch,[DRIVE_DATA+1]
    mov  cl,ch
    and  cl,11b
    je	.ide2
    mov  esi,cache_ide3
    call get_cache_ide
.ide2:
    mov  cl,ch
    shr  cl,2
    and  cl,11b
    je	.ide1
    mov  esi,cache_ide2
    call get_cache_ide
.ide1:
    mov  cl,ch
    shr  cl,4
    and  cl,11b
    je	.ide0
    mov  esi,cache_ide1
    call get_cache_ide
.ide0:
    mov  cl,ch
    shr  cl,6
    and  cl,11b
    je	@f
    mov  esi,cache_ide0
    call get_cache_ide
@@:
    xor ecx,ecx
    cmp [NumBiosDisks],ecx
    jz  .endbd
    mov esi,BiosDiskCaches
.loopbd:
    cmp byte [BiosDisksData+ecx*4+2],-1
    jnz .contbd
    mov eax,[cache_ide0_size]
    mov [esi+cache_ide0_size-cache_ide0],eax
    push ecx
    mov  cl,1
    call get_cache_ide
    pop  ecx
.contbd:
    add  esi,cache_ide1-cache_ide0
    inc  ecx
    cmp  ecx,[NumBiosDisks]
    jb   .loopbd
.endbd:
    jmp  end_get_cache

get_cache_ide:
    and  [esi+cache_ide0_search_start-cache_ide0],0
    and  [esi+cache_ide0_appl_search_start-cache_ide0],0
    push  ecx
    stdcall kernel_alloc,[esi+cache_ide0_size-cache_ide0]
    mov [esi+cache_ide0_pointer-cache_ide0],eax
    pop   ecx
    mov  edx,eax
    mov  eax,[esi+cache_ide0_size-cache_ide0]
    shr  eax,3
    mov  [esi+cache_ide0_system_data_size-cache_ide0],eax
    mov  ebx,eax
    imul eax,7
    mov  [esi+cache_ide0_appl_data_size-cache_ide0],eax
    add  ebx,edx
    mov  [esi+cache_ide0_data_pointer-cache_ide0],ebx

    cmp cl,10b
    je	.cd
    push ecx
    mov  eax,[esi+cache_ide0_system_data_size-cache_ide0]
    call calculate_for_hd
    add  eax,[esi+cache_ide0_pointer-cache_ide0]
    mov  [esi+cache_ide0_system_data-cache_ide0],eax
    mov  [esi+cache_ide0_system_sad_size-cache_ide0],ecx

    push  edi
    mov   edi,[esi+cache_ide0_pointer-cache_ide0]
    call  clear_ide_cache
    pop   edi

    mov  eax,[esi+cache_ide0_appl_data_size-cache_ide0]
    call calculate_for_hd
    add  eax,[esi+cache_ide0_data_pointer-cache_ide0]
    mov  [esi+cache_ide0_appl_data-cache_ide0],eax
    mov  [esi+cache_ide0_appl_sad_size-cache_ide0],ecx

    push  edi
    mov   edi,[esi+cache_ide0_data_pointer-cache_ide0]
    call  clear_ide_cache
    pop   edi

    pop  ecx
    ret
.cd:
    push ecx
    mov  eax,[esi+cache_ide0_system_data_size-cache_ide0]
    call calculate_for_cd
    add  eax,[esi+cache_ide0_pointer-cache_ide0]
    mov  [esi+cache_ide0_system_data-cache_ide0],eax
    mov  [esi+cache_ide0_system_sad_size-cache_ide0],ecx

    push  edi
    mov   edi,[esi+cache_ide0_pointer-cache_ide0]
    call  clear_ide_cache
    pop   edi

    mov  eax,[esi+cache_ide0_appl_data_size-cache_ide0]
    call calculate_for_cd
    add  eax,[esi+cache_ide0_data_pointer-cache_ide0]
    mov  [esi+cache_ide0_appl_data-cache_ide0],eax
    mov  [esi+cache_ide0_appl_sad_size-cache_ide0],ecx

    push  edi
    mov   edi,[esi+cache_ide0_data_pointer-cache_ide0]
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
    dec  ecx
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
    dec  ecx
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