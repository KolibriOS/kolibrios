$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Low-level driver for HDD access
; DMA support by Mario79

;**************************************************************************
;
;   0x600008  - first entry in cache list
;
;            +0   - lba sector
;            +4   - state of cache sector
;                   0 = empty
;                   1 = used for read  ( same as in hd )
;                   2 = used for write ( differs from hd )
;
;      +65536 - cache entries
;
;**************************************************************************

align 4
hd_read:
;-----------------------------------------------------------
; input  : eax = block to read
;          ebx = destination
;-----------------------------------------------------------
        and     [hd_error], 0
    push  ecx esi edi           ; scan cache

    mov   ecx,cache_max         ; entries in cache
    mov   esi,HD_CACHE+8
    mov   edi,1

  hdreadcache:

    cmp   dword [esi+4],0       ; empty
    je    nohdcache

    cmp   [esi],eax             ; correct sector
    je    yeshdcache

  nohdcache:

    add   esi,8
    inc   edi
    dec   ecx
    jnz   hdreadcache

    call  find_empty_slot       ; ret in edi
    cmp   [hd_error],0
    jne   return_01
; DMA read is permitted if [allow_dma_access]=1 or 2
        cmp     [allow_dma_access], 2
        ja      .nodma
        cmp     [dma_hdd], 1
        jnz     .nodma
        call    hd_read_dma
        jmp     @f
.nodma:
        call    hd_read_pio
@@:

    lea   esi,[edi*8+HD_CACHE]
    mov   [esi],eax             ; sector number
    mov   dword [esi+4],1       ; hd read - mark as same as in hd

  yeshdcache:

    mov   esi,edi
    shl   esi,9
    add   esi,HD_CACHE+65536
    mov   edi,ebx
    mov   ecx,512/4
    cld
    rep   movsd                 ; move data
 return_01:
    pop   edi esi ecx
    ret

align 4
hd_read_pio:
    push  eax edx

    call  wait_for_hd_idle
    cmp   [hd_error],0
    jne   hd_read_error

    cli
    xor   eax,eax
    mov   edx,[hdbase]
    inc   edx
    out   dx,al   ; ATAFeatures регистр "особенностей"
    inc   edx
    inc   eax
    out   dx,al   ; ATASectorCount счётчик секторов
    inc   edx
    mov   eax,[esp+4]
    out   dx,al   ; ATASectorNumber регистр номера сектора
    shr   eax,8
    inc   edx
    out   dx,al   ; ATACylinder номер цилиндра (младший байт)
    shr   eax,8
    inc   edx
    out   dx,al   ; номер цилиндра (старший байт)
    shr   eax,8
    inc   edx
    and   al,1+2+4+8
    add   al,byte [hdid]
    add   al,128+64+32
    out   dx,al   ; номер головки/номер диска
    inc   edx
    mov   al,20h
    out   dx,al   ; ATACommand регистр команд
    sti

    call  wait_for_sector_buffer

    cmp   [hd_error],0
    jne   hd_read_error

    cli
    push  edi
    shl   edi,9
    add   edi,HD_CACHE+65536
    mov   ecx,256
    mov   edx,[hdbase]
    cld
    rep   insw
    pop   edi
    sti

    pop   edx eax
    ret

disable_ide_int:
;        mov edx,[hdbase]
;        add edx,0x206
;        mov al,2
;        out dx,al
        cli
        ret

enable_ide_int:
;        mov edx,[hdbase]
;        add edx,0x206
;        mov al,0
;        out dx,al
        sti
        ret

align 4
hd_write:
;-----------------------------------------------------------
; input  : eax = block
;          ebx = pointer to memory
;-----------------------------------------------------------
    push  ecx esi edi

    ; check if the cache already has the sector and overwrite it

    mov   ecx,cache_max
    mov   esi,HD_CACHE+8
    mov   edi,1

  hdwritecache:

    cmp   dword [esi+4],0       ; if cache slot is empty
    je    not_in_cache_write

    cmp   [esi],eax             ; if the slot has the sector
    je    yes_in_cache_write

  not_in_cache_write:

    add   esi,8
    inc   edi
    dec   ecx
    jnz   hdwritecache

    ; sector not found in cache
    ; write the block to a new location

    call  find_empty_slot       ; ret in edi
    cmp   [hd_error],0
    jne   hd_write_access_denied

    lea   esi,[edi*8+HD_CACHE]
    mov   [esi],eax             ; sector number

  yes_in_cache_write:

    mov   dword [esi+4],2       ; write - differs from hd

    shl   edi,9
    add   edi,HD_CACHE+65536
    mov   esi,ebx
    mov   ecx,512/4
    cld
    rep   movsd                 ; move data
 hd_write_access_denied:
    pop   edi esi ecx
    ret


write_cache:
;-----------------------------------------------------------
; write all changed sectors to disk
;-----------------------------------------------------------
    push  eax ecx edx esi edi

    ; write difference ( 2 ) from cache to hd

    mov   ecx,cache_max
    mov   esi,HD_CACHE+8
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
cache_write_pio:
    call  disable_ide_int

    call  wait_for_hd_idle
    cmp   [hd_error],0
    jne   hd_write_error

;    cli
    xor   eax,eax
    mov   edx,[hdbase]
    inc   edx
    out   dx,al
    inc   edx
    inc   eax
    out   dx,al
    inc   edx
    mov   eax,[esi]             ; eax = sector to write
    out   dx,al
    shr   eax,8
    inc   edx
    out   dx,al
    shr   eax,8
    inc   edx
    out   dx,al
    shr   eax,8
    inc   edx
    and   al,1+2+4+8
    add   al,byte [hdid]
    add   al,128+64+32
    out   dx,al
    inc   edx
    mov   al,30h
    out   dx,al
;    sti

    call  wait_for_sector_buffer

    cmp   [hd_error],0
    jne   hd_write_error

    push  ecx esi

;    cli
    mov   esi,edi
    shl   esi,9
    add   esi,HD_CACHE+65536    ; esi = from memory position
    mov   ecx,256
    mov   edx,[hdbase]
    cld
    rep   outsw
;    sti

    call  enable_ide_int
    pop   esi ecx

    ret

align 4
find_empty_slot:
;-----------------------------------------------------------
; find empty or read slot, flush cache if next 10% is used by write
; output : edi = cache slot
;-----------------------------------------------------------
;    push  ecx esi

  search_again:

    mov   ecx,cache_max*10/100
    mov   edi,[cache_search_start]

  search_for_empty:

    inc   edi
    cmp   edi,cache_max
    jbe   inside_cache
    mov   edi,1

  inside_cache:

    cmp   dword [edi*8+HD_CACHE+4],2    ; get cache slot info
    jb    found_slot                    ; it's empty or read
    dec   ecx
    jnz   search_for_empty

    call  write_cache                   ; no empty slots found, write all
    cmp   [hd_error],0
    jne   found_slot_access_denied

    jmp   search_again                  ; and start again

  found_slot:

    mov   [cache_search_start],edi
  found_slot_access_denied:
    ret

align 4
clear_hd_cache:

    push  eax ecx edi
    mov   edi, HD_CACHE
    mov   ecx,16384
    xor   eax,eax
    cld
    rep   stosd                 ; clear hd cache with 0
    mov   [cache_search_start],eax
    mov   [fat_in_cache],-1
    mov   [fat_change],0
    pop   edi ecx eax
    ret

save_hd_wait_timeout:

    push  eax
    mov   eax,[timer_ticks]
    add   eax,300               ; 3 sec timeout
    mov   [hd_wait_timeout],eax
    pop   eax
    ret

align 4
check_hd_wait_timeout:

    push  eax
    mov   eax,[hd_wait_timeout]
    cmp   [timer_ticks], eax
    jg    hd_timeout_error
    pop   eax
    mov   [hd_error],0
    ret

;iglobal
;  hd_timeout_str   db 'K : FS - HD timeout',0
;  hd_read_str      db 'K : FS - HD read error',0
;  hd_write_str     db 'K : FS - HD write error',0
;  hd_lba_str       db 'K : FS - HD LBA error',0
;endg

hd_timeout_error:

;    call  clear_hd_cache
;    call  clear_application_table_status
;    mov   esi,hd_timeout_str
;    call  sys_msg_board_str
    DEBUGF 1,"K : FS - HD timeout\n"

    mov   [hd_error],1
    pop   eax
    ret

hd_read_error:

;    call  clear_hd_cache
;    call  clear_application_table_status
;    mov   esi,hd_read_str
;    call  sys_msg_board_str
    DEBUGF 1,"K : FS - HD read error\n"
    pop   edx eax
    ret

hd_write_error:

;    call  clear_hd_cache
;    call  clear_application_table_status
;     mov   esi,hd_write_str
;     call  sys_msg_board_str
    DEBUGF 1,"K : FS - HD write error\n"
     ret

hd_write_error_dma:
;        call    clear_hd_cache
;        call    clear_application_table_status
;        mov     esi, hd_write_str
;        call    sys_msg_board_str
        DEBUGF 1,"K : FS - HD read error\n"
        pop     esi
        ret

hd_lba_error:
;    call  clear_hd_cache
;    call  clear_application_table_status
;    mov   esi,hd_lba_str
;    call  sys_msg_board_str
     DEBUGF 1,"K : FS - HD LBA error\n"
     jmp   LBA_read_ret


align 4
wait_for_hd_idle:

    push  eax edx

    call  save_hd_wait_timeout

    mov   edx,[hdbase]
    add   edx,0x7

  wfhil1:

    call  check_hd_wait_timeout
    cmp   [hd_error],0
    jne   @f

    in    al,dx
    test  al,128
    jnz   wfhil1

 @@:

    pop   edx eax
    ret


align 4
wait_for_sector_buffer:

    push  eax edx

    mov   edx,[hdbase]
    add   edx,0x7

    call  save_hd_wait_timeout

  hdwait_sbuf:                  ; wait for sector buffer to be ready

    call  check_hd_wait_timeout
    cmp   [hd_error],0
    jne   @f

    in    al,dx
    test  al,8
    jz    hdwait_sbuf

    mov   [hd_error],0

    cmp   [hd_setup],1          ; do not mark error for setup request
    je    buf_wait_ok

    test  al,1                  ; previous command ended up with an error
    jz    buf_wait_ok
 @@:
    mov   [hd_error],1

  buf_wait_ok:

    pop   edx eax
    ret

; \begin{Mario79}
align 4
wait_for_sector_dma_ide0:
        push    eax
        push    edx
        call    save_hd_wait_timeout
.wait:
        call    change_task
        cmp     [irq14_func], hdd_irq14
        jnz     .done
        call    check_hd_wait_timeout
        cmp     [hd_error], 0
        jz      .wait
        mov     [irq14_func], hdd_irq_null
        mov     dx, [IDEContrRegsBaseAddr]
        mov     al, 0
        out     dx, al
.done:
        pop     edx
        pop     eax
        ret

align 4
wait_for_sector_dma_ide1:
        push    eax
        push    edx
        call    save_hd_wait_timeout
.wait:
        call    change_task
        cmp     [irq15_func], hdd_irq15
        jnz     .done
        call    check_hd_wait_timeout
        cmp     [hd_error], 0
        jz      .wait
        mov     [irq15_func], hdd_irq_null
        mov     dx, [IDEContrRegsBaseAddr]
        add     dx, 8
        mov     al, 0
        out     dx, al
.done:
        pop     edx
        pop     eax
        ret

iglobal
align 4
; note that IDE descriptor table must be 4-byte aligned and do not cross 4K boundary
IDE_descriptor_table:
        dd      0x284000
        dw      0x2000
        dw      0x8000

dma_cur_sector  dd      not 40h
irq14_func      dd      hdd_irq_null
irq15_func      dd      hdd_irq_null
endg

uglobal
; all uglobals are zeroed at boot
dma_process     dd      0
dma_slot_ptr    dd      0
cache_chain_pos dd      0
cache_chain_ptr dd      0
cache_chain_size        db      0
cache_chain_started     db      0
dma_task_switched       db      0
dma_hdd         db      0
allow_dma_access db      0
endg

align 4
hdd_irq14:
        pushfd
        cli
        pushad
        mov     [irq14_func], hdd_irq_null
        mov     dx, [IDEContrRegsBaseAddr]
        mov     al, 0
        out     dx, al
;        call    update_counters
;        mov     ebx, [dma_process]
;        cmp     [CURRENT_TASK], ebx
;        jz      .noswitch
;        mov     [dma_task_switched], 1
;        mov     edi, [dma_slot_ptr]
;        mov     eax, [CURRENT_TASK]
;        mov     [dma_process], eax
;        mov     eax, [TASK_BASE]
;        mov     [dma_slot_ptr], eax
;        mov     [CURRENT_TASK], ebx
;        mov     [TASK_BASE], edi
;        mov     byte [DONT_SWITCH], 1
;        call    do_change_task
.noswitch:
        popad
        popfd
align 4
hdd_irq_null:
        ret

align 4
hdd_irq15:
        pushfd
        cli
        pushad
        mov     [irq15_func], hdd_irq_null
        mov     dx, [IDEContrRegsBaseAddr]
        add     dx, 8
        mov     al, 0
        out     dx, al
;        call    update_counters
;        mov     ebx, [dma_process]
;        cmp     [CURRENT_TASK], ebx
;        jz      .noswitch
;        mov     [dma_task_switched], 1
;        mov     edi, [dma_slot_ptr]
;        mov     eax, [CURRENT_TASK]
;        mov     [dma_process], eax
;        mov     eax, [TASK_BASE]
;        mov     [dma_slot_ptr], eax
;        mov     [CURRENT_TASK], ebx
;        mov     [TASK_BASE], edi
;        mov     byte [DONT_SWITCH], 1
;        call    do_change_task
.noswitch:
        popad
        popfd
        ret

align 4
hd_read_dma:
        push    eax
        push    edx
        mov     edx, [dma_cur_sector]
        cmp     eax, edx
        jb      .notread
        add     edx, 15
        cmp     [esp+4], edx
        ja      .notread
        mov     eax, [esp+4]
        sub     eax, [dma_cur_sector]
        shl     eax, 9
        add     eax, OS_BASE+0x284000
        push    ecx esi edi
        mov     esi, eax
        shl     edi, 9
        add     edi, HD_CACHE+0x10000
        mov     ecx, 512/4
        cld
        rep     movsd
        pop     edi esi ecx
        pop     edx
        pop     eax
        ret
.notread:
        mov     eax, IDE_descriptor_table
        mov     dword [eax],  0x284000
        mov     word [eax+4], 0x2000
        sub     eax, OS_BASE
        mov     dx, [IDEContrRegsBaseAddr]
        cmp     [hdbase], 0x1F0
        jz      @f
        add     edx, 8
@@:
        push    edx
        add     edx, 4
        out     dx, eax
        pop     edx
        mov     al, 0
        out     dx, al
        add     edx, 2
        mov     al, 6
        out     dx, al
        call    wait_for_hd_idle
        cmp     [hd_error], 0
        jnz     hd_read_error
        call    disable_ide_int
        xor     eax, eax
        mov     edx, [hdbase]
        inc     edx
        out     dx, al
        inc     edx
        mov     eax, 10h
        out     dx, al
        inc     edx
        mov     eax, [esp+4]
        out     dx, al
        shr     eax, 8
        inc     edx
        out     dx, al
        shr     eax, 8
        inc     edx
        out     dx, al
        shr     eax, 8
        inc     edx
        and     al, 0xF
        add     al, byte [hdid]
        add     al, 11100000b
        out     dx, al
        inc     edx
        mov     al, 0xC8
        out     dx, al
        mov     dx, [IDEContrRegsBaseAddr]
        cmp     [hdbase], 0x1F0
        jz      @f
        add     dx, 8
@@:
        mov     al, 9
        out     dx, al
        mov     eax, [CURRENT_TASK]
        mov     [dma_process], eax
        mov     eax, [TASK_BASE]
        mov     [dma_slot_ptr], eax
        cmp     [hdbase], 0x1F0
        jnz     .ide1
        mov     [irq14_func], hdd_irq14
        jmp     @f
.ide1:
        mov     [irq15_func], hdd_irq15
@@:
        call    enable_ide_int
        cmp     [hdbase], 0x1F0
        jnz     .wait_ide1
        call    wait_for_sector_dma_ide0
        jmp     @f
.wait_ide1:
        call    wait_for_sector_dma_ide1
@@:
        cmp     [hd_error], 0
        jnz     hd_read_error
        pop     edx
        pop     eax
        mov     [dma_cur_sector], eax
        jmp     hd_read_dma

align 4
write_cache_chain:
        push    esi
        mov     eax, IDE_descriptor_table
        mov     edx, [cache_chain_pos]
        shl     edx, 9
        add     edx, DMA_HD_MEM+0x10000
        mov     [eax], edx
        movzx   edx, [cache_chain_size]
        shl     edx, 9
        mov     [eax+4], dx
        jmp     do_write_dma
write_cache_sector:
        push    esi
        mov     eax, IDE_descriptor_table
        mov     edx, edi
        shl     edx, 9
        add     edx, DMA_HD_MEM+0x10000
        mov     [eax], edx
        mov     word [eax+4], 0x200
do_write_dma:
        sub     eax, OS_BASE
        mov     dx, [IDEContrRegsBaseAddr]
        cmp     [hdbase], 0x1F0
        jz      @f
        add     edx, 8
@@:
        push    edx
        add     edx, 4
        out     dx, eax
        pop     edx
        mov     al, 0
        out     dx, al
        add     edx, 2
        mov     al, 6
        out     dx, al
        call    wait_for_hd_idle
        cmp     [hd_error], 0
        jnz     hd_write_error_dma
        call    disable_ide_int
        xor     eax, eax
        mov     edx, [hdbase]
        inc     edx
        out     dx, al
        inc     edx
        mov     al, [cache_chain_size]
        out     dx, al
        inc     edx
        mov     esi, [cache_chain_ptr]
        mov     eax, [esi]
        out     dx, al
        shr     eax, 8
        inc     edx
        out     dx, al
        shr     eax, 8
        inc     edx
        out     dx, al
        shr     eax, 8
        inc     edx
        and     al, 0xF
        add     al, byte [hdid]
        add     al, 11100000b
        out     dx, al
        inc     edx
        mov     al, 0xCA
        out     dx, al
        mov     dx, [IDEContrRegsBaseAddr]
        cmp     [hdbase], 0x1F0
        jz      @f
        add     dx, 8
@@:
        mov     al, 1
        out     dx, al
        mov     eax, [CURRENT_TASK]
        mov     [dma_process], eax
        mov     eax, [TASK_BASE]
        mov     [dma_slot_ptr], eax
        cmp     [hdbase], 0x1F0
        jnz     .ide1
        mov     [irq14_func], hdd_irq14
        jmp     @f
.ide1:
        mov     [irq15_func], hdd_irq15
@@:
        call    enable_ide_int
        mov     [dma_cur_sector], not 0x40
        cmp     [hdbase], 0x1F0
        jnz     .wait_ide1
        call    wait_for_sector_dma_ide0
        jmp     @f
.wait_ide1:
        call    wait_for_sector_dma_ide1
@@:
        cmp     [hd_error], 0
        jnz     hd_write_error_dma
        pop     esi
        ret

uglobal
IDEContrRegsBaseAddr    dw      ?
endg
; \end{Mario79}
