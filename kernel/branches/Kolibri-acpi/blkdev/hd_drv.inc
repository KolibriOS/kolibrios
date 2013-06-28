;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


; Low-level driver for HDD access
; DMA support by Mario79
; Access through BIOS by diamond
; LBA48 support by Mario79
;-----------------------------------------------------------------------------
align 4
hd_read:
;-----------------------------------------------------------
; input  : eax = block to read
;          ebx = destination
;-----------------------------------------------------------
        and     [hd_error], 0
        push    ecx esi edi     ; scan cache

        call    calculate_cache
        add     esi, 8

        mov     edi, 1

hdreadcache:
        cmp     dword [esi+4], 0 ; empty
        je      nohdcache

        cmp     [esi], eax      ; correct sector
        je      yeshdcache

nohdcache:
        add     esi, 8
        inc     edi
        dec     ecx
        jnz     hdreadcache

        call    find_empty_slot ; ret in edi
        cmp     [hd_error], 0
        jne     return_01
; Read through BIOS?
        cmp     [hdpos], 0x80
        jae     .bios
; DMA read is permitted if [allow_dma_access]=1 or 2
        cmp     [allow_dma_access], 2
        ja      .nodma
        cmp     [dma_hdd], 1
        jnz     .nodma
        call    hd_read_dma
        jmp     @f
.nodma:
        call    hd_read_pio
        jmp     @f
.bios:
        call    bd_read
@@:
        cmp     [hd_error], 0
        jne     return_01

        call    calculate_cache_1
        lea     esi, [edi*8+esi]

        mov     [esi], eax      ; sector number
        mov     dword [esi+4], 1 ; hd read - mark as same as in hd

yeshdcache:
        mov     esi, edi
        shl     esi, 9

        push    eax
        call    calculate_cache_2
        add     esi, eax
        pop     eax

        mov     edi, ebx
        mov     ecx, 512/4
        cld
        rep movsd               ; move data

return_01:
        pop     edi esi ecx
        ret
;-----------------------------------------------------------------------------
align 4
hd_read_pio:
        push    eax edx

; Select the desired drive
        mov     edx, [hdbase]
        add     edx, 6   ;адрес регистра головок
        mov     al, byte [hdid]
        add     al, 128+64+32
        out     dx, al; номер головки/номер диска
        
        call    wait_for_hd_idle
        cmp     [hd_error], 0
        jne     hd_read_error

; ATA with 28 or 48 bit for sector number?
        mov     eax, [esp+4]
        cmp     eax, 0x10000000
        jae     .lba48
;--------------------------------------
.lba28:
        pushfd
        cli
        xor     eax, eax
        mov     edx, [hdbase]
        inc     edx
        out     dx, al ; ATA Features регистр "особенностей"
        inc     edx
        inc     eax
        out     dx, al ; ATA Sector Counter счётчик секторов
        inc     edx
        mov     eax, [esp+4+4]
        out     dx, al ; LBA Low LBA (7:0)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA Mid LBA (15:8)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA High LBA (23:16)
        shr     eax, 8
        inc     edx
        and     al, 1+2+4+8 ; LBA (27:24)
        add     al, byte [hdid]
        add     al, 128+64+32
        out     dx, al ; номер головки/номер диска
        inc     edx
        mov     al, 20h ; READ SECTOR(S)
        out     dx, al ; ATACommand регистр команд
        popfd
        jmp     .continue
;--------------------------------------
.lba48:
        pushfd
        cli
        xor     eax, eax
        mov     edx, [hdbase]
        inc     edx
        out     dx, al ; Features Previous Reserved
        out     dx, al ; Features Current Reserved
        inc     edx
        out     dx, al ; Sector Count Previous Sector count (15:8)
        inc     eax
        out     dx, al ; Sector Count Current Sector count (7:0)
        inc     edx
        mov     eax, [esp+4+4]
        rol     eax, 8
        out     dx, al ; LBA Low Previous LBA (31:24)
        xor     eax, eax ; because only 32 bit cache
        inc     edx
        out     dx, al ; LBA Mid Previous LBA (39:32)
        inc     edx
        out     dx, al ; LBA High Previous LBA (47:40)
        sub     edx, 2
        mov     eax, [esp+4+4]
        out     dx, al ; LBA Low Current LBA (7:0)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA Mid Current LBA (15:8)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA High Current LBA (23:16)
        inc     edx
        mov     al, byte [hdid]
        add     al, 128+64+32
        out     dx, al ; номер головки/номер диска
        inc     edx
        mov     al, 24h ; READ SECTOR(S) EXT
        out     dx, al ; ATACommand регистр команд
        popfd
;--------------------------------------
.continue:
        call    wait_for_sector_buffer

        cmp     [hd_error], 0
        jne     hd_read_error

        pushfd
        cli
        push    edi
        shl     edi, 9

        push    eax
        call    calculate_cache_2
        add     edi, eax
        pop     eax

        mov     ecx, 256
        mov     edx, [hdbase]
        cld
        rep insw
        pop     edi
        popfd

        pop     edx eax
        ret
;-----------------------------------------------------------------------------
align 4
hd_write:
;-----------------------------------------------------------
; input  : eax = block
;          ebx = pointer to memory
;-----------------------------------------------------------
        push    ecx esi edi

; check if the cache already has the sector and overwrite it
        call    calculate_cache
        add     esi, 8
        mov     edi, 1

hdwritecache:
        cmp     dword [esi+4], 0 ; if cache slot is empty
        je      not_in_cache_write

        cmp     [esi], eax      ; if the slot has the sector
        je      yes_in_cache_write

not_in_cache_write:
        add     esi, 8
        inc     edi
        dec     ecx
        jnz     hdwritecache

; sector not found in cache
; write the block to a new location
        call    find_empty_slot ; ret in edi
        cmp     [hd_error], 0
        jne     hd_write_access_denied

        call    calculate_cache_1
        lea     esi, [edi*8+esi]
        mov     [esi], eax      ; sector number

yes_in_cache_write:
        mov     dword [esi+4], 2 ; write - differs from hd

        shl     edi, 9

        push    eax
        call    calculate_cache_2
        add     edi, eax
        pop     eax

        mov     esi, ebx
        mov     ecx, 512/4
        cld
        rep movsd               ; move data

hd_write_access_denied:
        pop     edi esi ecx
        ret
;-----------------------------------------------------------------------------
align 4
cache_write_pio:
; Select the desired drive
        mov     edx, [hdbase]
        add     edx, 6   ;адрес регистра головок
        mov     al, byte [hdid]
        add     al, 128+64+32
        out     dx, al ; номер головки/номер диска

        call    wait_for_hd_idle
        cmp     [hd_error], 0
        jne     hd_write_error

; ATA with 28 or 48 bit for sector number?
        mov     eax, [esi]
        cmp     eax, 0x10000000
        jae     .lba48
;--------------------------------------
.lba28:
        pushfd
        cli
        xor     eax, eax
        mov     edx, [hdbase]
        inc     edx
        out     dx, al ; ATA Features регистр "особенностей"
        inc     edx
        inc     eax
        out     dx, al ; ATA Sector Counter счётчик секторов
        inc     edx
        mov     eax, [esi]      ; eax = sector to write
        out     dx, al ; LBA Low LBA (7:0)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA Mid LBA (15:8)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA High LBA (23:16)
        shr     eax, 8
        inc     edx
        and     al, 1+2+4+8 ; LBA (27:24)
        add     al, byte [hdid]
        add     al, 128+64+32
        out     dx, al ; номер головки/номер диска
        inc     edx
        mov     al, 30h ; WRITE SECTOR(S)
        out     dx, al ; ATACommand регистр команд
        popfd
        jmp     .continue
;--------------------------------------
.lba48:
        pushfd
        cli
        xor     eax, eax
        mov     edx, [hdbase]
        inc     edx
        out     dx, al ; Features Previous Reserved
        out     dx, al ; Features Current Reserved
        inc     edx
        out     dx, al ; Sector Count Previous Sector count (15:8)
        inc     eax
        out     dx, al ; Sector Count Current Sector count (7:0)
        inc     edx
        mov     eax, [esi]
        rol     eax, 8
        out     dx, al ; LBA Low Previous LBA (31:24)
        xor     eax, eax ; because only 32 bit cache
        inc     edx
        out     dx, al ; LBA Mid Previous LBA (39:32)
        inc     edx
        out     dx, al ; LBA High Previous LBA (47:40)
        sub     edx, 2
        mov     eax, [esi]
        out     dx, al ; LBA Low Current LBA (7:0)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA Mid Current LBA (15:8)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA High Current LBA (23:16)
        inc     edx
        mov     al, byte [hdid]
        add     al, 128+64+32
        out     dx, al ; номер головки/номер диска
        inc     edx
        mov     al, 34h ; WRITE SECTOR(S) EXT
        out     dx, al ; ATACommand регистр команд
        popfd
;--------------------------------------
.continue:
        call    wait_for_sector_buffer

        cmp     [hd_error], 0
        jne     hd_write_error

        push    ecx esi

        pushfd
        cli
        mov     esi, edi
        shl     esi, 9

        push    eax
        call    calculate_cache_2
        add     esi, eax
        pop     eax

        mov     ecx, 256
        mov     edx, [hdbase]
        cld
        rep outsw
        popfd

        pop     esi ecx
        ret
;-----------------------------------------------------------------------------
align 4
save_hd_wait_timeout:
        push    eax
        mov     eax, [timer_ticks]
        add     eax, 300        ; 3 sec timeout
        mov     [hd_wait_timeout], eax
        pop     eax
        ret
;-----------------------------------------------------------------------------
align 4
check_hd_wait_timeout:
        push    eax
        mov     eax, [hd_wait_timeout]
        cmp     [timer_ticks], eax
        jg      hd_timeout_error

        pop     eax
        mov     [hd_error], 0
        ret
;-----------------------------------------------------------------------------
hd_timeout_error:
    if lang eq sp
    DEBUGF 1,"K : FS - HD tiempo de espera agotado\n"
    else
    DEBUGF 1,"K : FS - HD timeout\n"
    end if
        mov     [hd_error], 1
        pop     eax
        ret
;-----------------------------------------------------------------------------
hd_read_error:
    if lang eq sp
    DEBUGF 1,"K : FS - HD error de lectura\n"
    else
    DEBUGF 1,"K : FS - HD read error\n"
    end if
        pop     edx eax
        ret
;-----------------------------------------------------------------------------
hd_write_error_dma:
        pop     esi
hd_write_error:
    if lang eq sp
    DEBUGF 1,"K : FS - HD error de escritura\n"
    else
    DEBUGF 1,"K : FS - HD write error\n"
    end if
        ret
;-----------------------------------------------------------------------------
hd_lba_error:
     if lang eq sp
     DEBUGF 1,"K : FS - HD error en LBA\n"
     else
     DEBUGF 1,"K : FS - HD LBA error\n"
     end if
        jmp     LBA_read_ret
;-----------------------------------------------------------------------------
align 4
wait_for_hd_idle:
        push    eax edx

        call    save_hd_wait_timeout

        mov     edx, [hdbase]
        add     edx, 0x7
;--------------------------------------
align 4
wfhil1:
        call    check_hd_wait_timeout
        cmp     [hd_error], 0
        jne     @f

        in      al, dx
        test    al, 128
        jnz     wfhil1

@@:
        pop     edx eax
        ret
;-----------------------------------------------------------------------------
align 4
wait_for_sector_buffer:
        push    eax edx

        mov     edx, [hdbase]
        add     edx, 0x7

        call    save_hd_wait_timeout
;--------------------------------------
align 4
hdwait_sbuf:                  ; wait for sector buffer to be ready
        call    check_hd_wait_timeout
        cmp     [hd_error], 0
        jne     @f

        in      al, dx
        test    al, 8
        jz      hdwait_sbuf

        mov     [hd_error], 0

        cmp     [hd_setup], 1   ; do not mark error for setup request
        je      buf_wait_ok

        test    al, 1           ; previous command ended up with an error
        jz      buf_wait_ok
@@:
        mov     [hd_error], 1

buf_wait_ok:
        pop     edx eax
        ret
;-----------------------------------------------------------------------------
align 4
wait_for_sector_dma_ide0:
        push    eax
        push    edx
        call    save_hd_wait_timeout
;--------------------------------------
align 4
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
;-----------------------------------------------------------------------------
align 4
wait_for_sector_dma_ide1:
        push    eax
        push    edx
        call    save_hd_wait_timeout
;--------------------------------------
align 4
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
;-----------------------------------------------------------------------------
iglobal
align 4
; note that IDE descriptor table must be 4-byte aligned and do not cross 4K boundary
IDE_descriptor_table:
        dd      IDE_DMA
        dw      0x2000
        dw      0x8000

dma_cur_sector  dd      not 40h
dma_hdpos       dd      0
irq14_func      dd      hdd_irq_null
irq15_func      dd      hdd_irq_null
endg
;-----------------------------------------------------------------------------
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
;-----------------------------------------------------------------------------
align 4
hdd_irq14:
        pushfd
        cli
        pushad
        mov     [irq14_func], hdd_irq_null
        mov     dx, [IDEContrRegsBaseAddr]
        mov     al, 0
        out     dx, al
        popad
        popfd
align 4
hdd_irq_null:
        ret
;-----------------------------------------------------------------------------
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
        popad
        popfd
        ret
;-----------------------------------------------------------------------------
align 4
hd_read_dma:
        push    eax
        push    edx
        mov     edx, [dma_hdpos]
        cmp     edx, [hdpos]
        jne     .notread
        mov     edx, [dma_cur_sector]
        cmp     eax, edx
        jb      .notread
        add     edx, 15
        cmp     [esp+4], edx
        ja      .notread
        mov     eax, [esp+4]
        sub     eax, [dma_cur_sector]
        shl     eax, 9
        add     eax, (OS_BASE+IDE_DMA)
        push    ecx esi edi
        mov     esi, eax
        shl     edi, 9

        push    eax
        call    calculate_cache_2
        add     edi, eax
        pop     eax

        mov     ecx, 512/4
        cld
        rep movsd
        pop     edi esi ecx
        pop     edx
        pop     eax
        ret
.notread:
        mov     eax, IDE_descriptor_table
        mov     dword [eax], IDE_DMA
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

; Select the desired drive
        mov     edx, [hdbase]
        add     edx, 6   ; адрес регистра головок
        mov     al, byte [hdid]
        add     al, 128+64+32
        out     dx, al ; номер головки/номер диска

        call    wait_for_hd_idle
        cmp     [hd_error], 0
        jnz     hd_read_error

; ATA with 28 or 48 bit for sector number?
        mov     eax, [esp+4]
; -10h because the PreCache hits the boundary between lba28 and lba48
; 10h = 16  - size of PreCache
        cmp     eax, 0x10000000-10h
        jae     .lba48
;--------------------------------------
.lba28:
        pushfd
        cli
        xor     eax, eax
        mov     edx, [hdbase]
        inc     edx
        out     dx, al ; ATA Features регистр "особенностей"
        inc     edx
        mov     eax, 10h ; Sector Counter = 16 ; PreCache
        out     dx, al ; ATA Sector Counter счётчик секторов
        inc     edx
        mov     eax, [esp+4+4]
        out     dx, al ; LBA Low LBA (7:0)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA Mid LBA (15:8)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA High LBA (23:16)
        shr     eax, 8
        inc     edx
        and     al, 0xF ; LBA (27:24)
        add     al, byte [hdid]
        add     al, 11100000b
        out     dx, al ; номер головки/номер диска
        inc     edx
        mov     al, 0xC8 ; READ DMA
        out     dx, al ; ATACommand регистр команд
        jmp     .continue
;--------------------------------------
.lba48:
        pushfd
        cli
        xor     eax, eax
        mov     edx, [hdbase]
        inc     edx
        out     dx, al ; Features Previous Reserved
        out     dx, al ; Features Current Reserved
        inc     edx
        out     dx, al ; Sector Count Previous Sector count (15:8)
        mov     eax, 10h ; Sector Counter = 16 PreCache
        out     dx, al ; Sector Count Current Sector count (7:0)
        inc     edx
        mov     eax, [esp+4+4]
        rol     eax, 8
        out     dx, al ; LBA Low Previous LBA (31:24)
        xor     eax, eax ; because only 32 bit cache
        inc     edx
        out     dx, al ; LBA Mid Previous LBA (39:32)
        inc     edx
        out     dx, al ; LBA High Previous LBA (47:40)
        sub     edx, 2
        mov     eax, [esp+4+4]
        out     dx, al ; LBA Low Current LBA (7:0)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA Mid Current LBA (15:8)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA High Current LBA (23:16)
        inc     edx
        mov     al, byte [hdid]
        add     al, 128+64+32
        out     dx, al ; номер головки/номер диска
        inc     edx
        mov     al, 25h ; READ DMA EXT
        out     dx, al ; ATACommand регистр команд
;--------------------------------------
.continue:
        mov     dx, [IDEContrRegsBaseAddr]
        mov     eax, [hd_address_table]
        cmp     [hdbase], eax ; 0x1F0
        jz      @f
        add     dx, 8
@@:
        mov     al, 9
        out     dx, al
        mov     eax, [CURRENT_TASK]
        mov     [dma_process], eax
        mov     eax, [TASK_BASE]
        mov     [dma_slot_ptr], eax
        mov     eax, [hd_address_table]
        cmp     [hdbase], eax ; 0x1F0
        jnz     .ide1
        mov     [irq14_func], hdd_irq14
        jmp     @f
.ide1:
        mov     [irq15_func], hdd_irq15
@@:
        popfd
        mov     eax, [hd_address_table]
        cmp     [hdbase], eax ; 0x1F0
        jnz     .wait_ide1
        call    wait_for_sector_dma_ide0
        jmp     @f
.wait_ide1:
        call    wait_for_sector_dma_ide1
@@:
        cmp     [hd_error], 0
        jnz     hd_read_error
        mov     eax, [hdpos]
        mov     [dma_hdpos], eax
        pop     edx
        pop     eax
        mov     [dma_cur_sector], eax
        jmp     hd_read_dma
;-----------------------------------------------------------------------------
align 4
write_cache_sector:
        mov     [cache_chain_size], 1
        mov     [cache_chain_pos], edi
;--------------------------------------
align 4
write_cache_chain:
        cmp     [hdpos], 0x80
        jae     bd_write_cache_chain
        mov     eax, [cache_chain_ptr]
        push    esi
        mov     eax, IDE_descriptor_table
        mov     edx, eax
        pusha
        mov     esi, [cache_chain_pos]
        shl     esi, 9
        call    calculate_cache_2
        add     esi, eax
        mov     edi, (OS_BASE+IDE_DMA)
        mov     dword [edx], IDE_DMA
        movzx   ecx, [cache_chain_size]
        shl     ecx, 9
        mov     word [edx+4], cx
        shr     ecx, 2
        cld
        rep movsd
        popa
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

; Select the desired drive
        mov     edx, [hdbase]
        add     edx, 6   ; адрес регистра головок
        mov     al, byte [hdid]
        add     al, 128+64+32
        out     dx, al ; номер головки/номер диска

        call    wait_for_hd_idle
        cmp     [hd_error], 0
        jnz     hd_write_error_dma

; ATA with 28 or 48 bit for sector number?
        mov     esi, [cache_chain_ptr]
        mov     eax, [esi]
; -40h because the PreCache hits the boundary between lba28 and lba48
; 40h = 64  - the maximum number of sectors to be written for one command
        cmp     eax, 0x10000000-40h
        jae     .lba48
;--------------------------------------
.lba28:
        pushfd
        cli
        xor     eax, eax
        mov     edx, [hdbase]
        inc     edx
        out     dx, al ; ATA Features регистр "особенностей"
        inc     edx
        mov     al, [cache_chain_size] ; Sector Counter
        out     dx, al ; ATA Sector Counter счётчик секторов
        inc     edx
        mov     eax, [esi]
        out     dx, al ; LBA Low LBA (7:0)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA Mid LBA (15:8)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA High LBA (23:16)
        shr     eax, 8
        inc     edx
        and     al, 0xF ; LBA (27:24)
        add     al, byte [hdid]
        add     al, 11100000b
        out     dx, al ; номер головки/номер диска
        inc     edx
        mov     al, 0xCA ; WRITE DMA
        out     dx, al ; ATACommand регистр команд
        jmp     .continue
;--------------------------------------
.lba48:
        pushfd
        cli
        xor     eax, eax
        mov     edx, [hdbase]
        inc     edx
        out     dx, al ; Features Previous Reserved
        out     dx, al ; Features Current Reserved
        inc     edx
        out     dx, al ; Sector Count Previous Sector count (15:8)
        mov     al, [cache_chain_size] ; Sector Counter
        out     dx, al ; Sector Count Current Sector count (7:0)
        inc     edx
        mov     eax, [esi]
        rol     eax, 8
        out     dx, al ; LBA Low Previous LBA (31:24)
        xor     eax, eax ; because only 32 bit cache
        inc     edx
        out     dx, al ; LBA Mid Previous LBA (39:32)
        inc     edx
        out     dx, al ; LBA High Previous LBA (47:40)
        sub     edx, 2
        mov     eax, [esi]
        out     dx, al ; LBA Low Current LBA (7:0)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA Mid Current LBA (15:8)
        shr     eax, 8
        inc     edx
        out     dx, al ; LBA High Current LBA (23:16)
        inc     edx
        mov     al, byte [hdid]
        add     al, 128+64+32
        out     dx, al ; номер головки/номер диска
        inc     edx
        mov     al, 35h ; WRITE DMA EXT
        out     dx, al ; ATACommand регистр команд
;--------------------------------------
.continue:
        mov     dx, [IDEContrRegsBaseAddr]
        mov     eax, [hd_address_table]
        cmp     [hdbase], eax ; 0x1F0
        jz      @f
        add     dx, 8
@@:
        mov     al, 1
        out     dx, al
        mov     eax, [CURRENT_TASK]
        mov     [dma_process], eax
        mov     eax, [TASK_BASE]
        mov     [dma_slot_ptr], eax
        mov     eax, [hd_address_table]
        cmp     [hdbase], eax ; 0x1F0
        jnz     .ide1
        mov     [irq14_func], hdd_irq14
        jmp     @f
.ide1:
        mov     [irq15_func], hdd_irq15
@@:
        popfd
        mov     [dma_cur_sector], not 0x40
        mov     eax, [hd_address_table]
        cmp     [hdbase], eax ; 0x1F0
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
;-----------------------------------------------------------------------------
uglobal
IDEContrRegsBaseAddr    dw      ?
IDEContrProgrammingInterface dw ?
IDE_BAR0_val    dw ?
IDE_BAR1_val    dw ?
IDE_BAR2_val    dw ?
IDE_BAR3_val    dw ?
endg
;-----------------------------------------------------------------------------
; \begin{diamond}
uglobal
bios_hdpos      dd      0       ; 0 is invalid value for [hdpos]
bios_cur_sector dd      ?
bios_read_len   dd      ?
endg
;-----------------------------------------------------------------------------
align 4
bd_read:
        push    eax
        push    edx
        mov     edx, [bios_hdpos]
        cmp     edx, [hdpos]
        jne     .notread
        mov     edx, [bios_cur_sector]
        cmp     eax, edx
        jb      .notread
        add     edx, [bios_read_len]
        dec     edx
        cmp     eax, edx
        ja      .notread
        sub     eax, [bios_cur_sector]
        shl     eax, 9
        add     eax, (OS_BASE+0x9A000)
        push    ecx esi edi
        mov     esi, eax
        shl     edi, 9

        push    eax
        call    calculate_cache_2
        add     edi, eax
        pop     eax

        mov     ecx, 512/4
        cld
        rep movsd
        pop     edi esi ecx
        pop     edx
        pop     eax
        ret
.notread:
        push    ecx
        mov     dl, 42h
        mov     ecx, 16
        call    int13_call
        pop     ecx
        test    eax, eax
        jnz     .v86err
        test    edx, edx
        jz      .readerr
        mov     [bios_read_len], edx
        mov     edx, [hdpos]
        mov     [bios_hdpos], edx
        pop     edx
        pop     eax
        mov     [bios_cur_sector], eax
        jmp     bd_read
.readerr:
.v86err:
        mov     [hd_error], 1
        jmp     hd_read_error
;-----------------------------------------------------------------------------
align 4
bd_write_cache_chain:
        pusha
        mov     esi, [cache_chain_pos]
        shl     esi, 9
        call    calculate_cache_2
        add     esi, eax
        mov     edi, OS_BASE + 0x9A000
        movzx   ecx, [cache_chain_size]
        push    ecx
        shl     ecx, 9-2
        rep movsd
        pop     ecx
        mov     dl, 43h
        mov     eax, [cache_chain_ptr]
        mov     eax, [eax]
        call    int13_call
        test    eax, eax
        jnz     .v86err
        cmp     edx, ecx
        jnz     .writeerr
        popa
        ret
.v86err:
.writeerr:
        popa
        mov     [hd_error], 1
        jmp     hd_write_error
;-----------------------------------------------------------------------------
uglobal
int13_regs_in   rb      sizeof.v86_regs
int13_regs_out  rb      sizeof.v86_regs
endg
;-----------------------------------------------------------------------------
align 4
int13_call:
; Because this code uses fixed addresses,
; it can not be run simultaniously by many threads.
; In current implementation it is protected by common mutex 'hd1_status'
        mov     word [OS_BASE + 510h], 10h             ; packet length
        mov     word [OS_BASE + 512h], cx              ; number of sectors
        mov     dword [OS_BASE + 514h], 9A000000h      ; buffer 9A00:0000
        mov     dword [OS_BASE + 518h], eax
        and     dword [OS_BASE + 51Ch], 0
        push    ebx ecx esi edi
        mov     ebx, int13_regs_in
        mov     edi, ebx
        mov     ecx, sizeof.v86_regs/4
        xor     eax, eax
        rep stosd
        mov     byte [ebx+v86_regs.eax+1], dl
        mov     eax, [hdpos]
        lea     eax, [BiosDisksData+(eax-80h)*4]
        mov     dl, [eax]
        mov     byte [ebx+v86_regs.edx], dl
        movzx   edx, byte [eax+1]
;        mov     dl, 5
        test    edx, edx
        jnz     .hasirq
        dec     edx
        jmp     @f
.hasirq:
        pushad
        stdcall enable_irq, edx
        popad
@@:
        mov     word [ebx+v86_regs.esi], 510h
        mov     word [ebx+v86_regs.ss], 9000h
        mov     word [ebx+v86_regs.esp], 0A000h
        mov     word [ebx+v86_regs.eip], 500h
        mov     [ebx+v86_regs.eflags], 20200h
        mov     esi, [sys_v86_machine]
        mov     ecx, 0x502
        push    fs
        call    v86_start
        pop     fs
        and     [bios_hdpos], 0
        pop     edi esi ecx ebx
        movzx   edx, byte [OS_BASE + 512h]
        test    byte [int13_regs_out+v86_regs.eflags], 1
        jnz     @f
        mov     edx, ecx
@@:
        ret
; \end{diamond}
;-----------------------------------------------------------------------------
align 4
reserve_hd1:
        cli
        cmp     [hd1_status], 0
        je      reserve_ok1

        sti
        call    change_task
        jmp     reserve_hd1

reserve_ok1:
        push    eax
        mov     eax, [CURRENT_TASK]
        shl     eax, 5
        mov     eax, [eax+CURRENT_TASK+TASKDATA.pid]
        mov     [hd1_status], eax
        pop     eax
        sti
        ret
;-----------------------------------------------------------------------------
uglobal
hd_in_cache db ?
endg
;-----------------------------------------------------------------------------
align 4
reserve_hd_channel:
; BIOS disk accesses are protected with common mutex hd1_status
; This must be modified when hd1_status will not be valid!
        cmp     [hdpos], 0x80
        jae     .ret
        cmp     [hdbase], 0x1F0
        jne     .IDE_Channel_2
.IDE_Channel_1:
        cli
        cmp     [IDE_Channel_1], 0
        je      .reserve_ok_1
        sti
        call    change_task
        jmp     .IDE_Channel_1
.IDE_Channel_2:
        cli
        cmp     [IDE_Channel_2], 0
        je      .reserve_ok_2
        sti
        call    change_task
        jmp     .IDE_Channel_2
.reserve_ok_1:
        mov     [IDE_Channel_1], 1
        push    eax
        mov     al, 1
        jmp     @f
.reserve_ok_2:
        mov     [IDE_Channel_2], 1
        push    eax
        mov     al, 3
@@:
        cmp     [hdid], 1
        sbb     al, -1
        mov     [hd_in_cache], al
        pop     eax
        sti
.ret:
        ret
;-----------------------------------------------------------------------------
free_hd_channel:
; see comment at reserve_hd_channel
        cmp     [hdpos], 0x80
        jae     .ret
        cmp     [hdbase], 0x1F0
        jne     .IDE_Channel_2
.IDE_Channel_1:
        mov     [IDE_Channel_1], 0
.ret:
        ret
.IDE_Channel_2:
        mov     [IDE_Channel_2], 0
        ret
;-----------------------------------------------------------------------------
