;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


; Low-level driver for HDD access
; DMA support by Mario79
; Access through BIOS by diamond
; LBA48 support by Mario79
;-----------------------------------------------------------------------------
struct HD_DATA
hdbase  dd      ?
hdid    dd      ?
hdpos   dd      ?
ends

iglobal
align 4
ide_callbacks:
        dd      ide_callbacks.end - ide_callbacks       ; strucsize
        dd      0       ; no close function
        dd      0       ; no closemedia function
        dd      ide_querymedia
        dd      ide_read
        dd      ide_write
        dd      0       ; no flush function
        dd      0       ; use default cache size
.end:

bd_callbacks:
        dd      bd_callbacks.end - bd_callbacks         ; strucsize
        dd      0       ; no close function
        dd      0       ; no closemedia function
        dd      bd_querymedia
        dd      bd_read_interface
        dd      bd_write_interface
        dd      0       ; no flush function
        dd      0       ; use default cache size
.end:

hd0_data        HD_DATA         ?,    0, 1
hd1_data        HD_DATA         ?, 0x10, 2
hd2_data        HD_DATA         ?,    0, 3
hd3_data        HD_DATA         ?, 0x10, 4
endg

uglobal
ide_mutex               MUTEX
ide_channel1_mutex      MUTEX
ide_channel2_mutex      MUTEX
endg

proc ide_read stdcall uses edi, \
        hd_data, buffer, startsector:qword, numsectors
        ; hd_data = pointer to hd*_data
        ; buffer = pointer to buffer for data
        ; startsector = 64-bit start sector
        ; numsectors = pointer to number of sectors on input,
        ;  must be filled with number of sectors really read
locals
sectors_todo    dd      ?
channel_lock    dd      ?
endl
; 1. Initialize number of sectors: get number of requested sectors
; and say that no sectors were read yet.
        mov     ecx, [numsectors]
        mov     eax, [ecx]
        mov     dword [ecx], 0
        mov     [sectors_todo], eax
; 2. Acquire the global lock.
        mov     ecx, ide_mutex
        call    mutex_lock
        mov     ecx, ide_channel2_mutex
        mov     eax, [hd_data]
        cmp     [eax+HD_DATA.hdbase], 0x1F0
        jne     .IDE_Channel_2
        mov     ecx, ide_channel1_mutex
.IDE_Channel_2:
        mov     [channel_lock], ecx
        call    mutex_lock
; 3. Convert parameters to the form suitable for worker procedures.
; Underlying procedures do not know about 64-bit sectors.
; Worker procedures use global variables and edi for [buffer].
        cmp     dword [startsector+4], 0
        jnz     .fail
        and     [hd_error], 0
        mov     ecx, [hd_data]
        mov     eax, [ecx+HD_DATA.hdbase]
        mov     [hdbase], eax
        mov     eax, [ecx+HD_DATA.hdid]
        mov     [hdid], eax
        mov     eax, [ecx+HD_DATA.hdpos]
        mov     [hdpos], eax
        mov     eax, dword [startsector]
        mov     edi, [buffer]
; 4. Worker procedures take one sectors per time, so loop over all sectors to read.
.sectors_loop:
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
        cmp     [hd_error], 0
        jnz     .fail
        mov     ecx, [numsectors]
        inc     dword [ecx]     ; one more sector is read
        dec     [sectors_todo]
        jz      .done
        inc     eax
        jnz     .sectors_loop
; 5. Loop is done, either due to error or because everything is done.
; Release the global lock and return the corresponding status.
.fail:
        mov     ecx, [channel_lock]
        call    mutex_unlock
        mov     ecx, ide_mutex
        call    mutex_unlock
        or      eax, -1
        ret
.done:
        mov     ecx, [channel_lock]
        call    mutex_unlock
        mov     ecx, ide_mutex
        call    mutex_unlock
        xor     eax, eax
        ret
endp

proc ide_write stdcall uses esi edi, \
        hd_data, buffer, startsector:qword, numsectors
        ; hd_data = pointer to hd*_data
        ; buffer = pointer to buffer with data
        ; startsector = 64-bit start sector
        ; numsectors = pointer to number of sectors on input,
        ;  must be filled with number of sectors really written
locals
sectors_todo    dd      ?
channel_lock    dd      ?
endl
; 1. Initialize number of sectors: get number of requested sectors
; and say that no sectors were read yet.      
        mov     ecx, [numsectors]
        mov     eax, [ecx]
        mov     dword [ecx], 0
        mov     [sectors_todo], eax
; 2. Acquire the global lock.
        mov     ecx, ide_mutex
        call    mutex_lock
        mov     ecx, ide_channel2_mutex
        mov     eax, [hd_data]
        cmp     [eax+HD_DATA.hdbase], 0x1F0
        jne     .IDE_Channel_2
        mov     ecx, ide_channel1_mutex
.IDE_Channel_2:
        mov     [channel_lock], ecx
        call    mutex_lock
; 3. Convert parameters to the form suitable for worker procedures.
; Underlying procedures do not know about 64-bit sectors.
; Worker procedures use global variables and esi for [buffer].
        cmp     dword [startsector+4], 0
        jnz     .fail
        and     [hd_error], 0
        mov     ecx, [hd_data]
        mov     eax, [ecx+HD_DATA.hdbase]
        mov     [hdbase], eax
        mov     eax, [ecx+HD_DATA.hdid]
        mov     [hdid], eax
        mov     eax, [ecx+HD_DATA.hdpos]
        mov     [hdpos], eax
        mov     esi, [buffer]
        lea     edi, [startsector]
        mov     [cache_chain_ptr], edi
; 4. Worker procedures take max 16 sectors per time,
; loop until all sectors will be processed.
.sectors_loop:
        mov     ecx, 16
        cmp     ecx, [sectors_todo]
        jbe     @f
        mov     ecx, [sectors_todo]
@@:
        mov     [cache_chain_size], cl
; DMA write is permitted only if [allow_dma_access]=1
        cmp     [allow_dma_access], 2
        jae     .nodma
        cmp     [dma_hdd], 1
        jnz     .nodma
        call    cache_write_dma
        jmp     .common
.nodma:
        mov     [cache_chain_size], 1
        call    cache_write_pio
.common:
        cmp     [hd_error], 0
        jnz     .fail
        movzx   ecx, [cache_chain_size]
        mov     eax, [numsectors]
        add     [eax], ecx
        sub     [sectors_todo], ecx
        jz      .done
        add     [edi], ecx
        jc      .fail
        shl     ecx, 9
        add     esi, ecx
        jmp     .sectors_loop
; 5. Loop is done, either due to error or because everything is done.
; Release the global lock and return the corresponding status.
.fail:
        mov     ecx, [channel_lock]
        call    mutex_unlock
        mov     ecx, ide_mutex
        call    mutex_unlock
        or      eax, -1
        ret
.done:
        mov     ecx, [channel_lock]
        call    mutex_unlock
        mov     ecx, ide_mutex
        call    mutex_unlock
        xor     eax, eax
        ret
endp

; This is a stub.
proc ide_querymedia stdcall, hd_data, mediainfo
        mov     eax, [mediainfo]
        mov     [eax+DISKMEDIAINFO.Flags], 0
        mov     [eax+DISKMEDIAINFO.SectorSize], 512
        or      dword [eax+DISKMEDIAINFO.Capacity], 0xFFFFFFFF
        or      dword [eax+DISKMEDIAINFO.Capacity+4], 0xFFFFFFFF
        xor     eax, eax
        ret
endp

proc bd_read_interface stdcall uses edi, \
        userdata, buffer, startsector:qword, numsectors
        ; userdata = old [hdpos] = 80h + index in NumBiosDisks
        ; buffer = pointer to buffer for data
        ; startsector = 64-bit start sector
        ; numsectors = pointer to number of sectors on input,
        ;  must be filled with number of sectors really read
locals
sectors_todo    dd      ?
endl
; 1. Initialize number of sectors: get number of requested sectors
; and say that no sectors were read yet.
        mov     ecx, [numsectors]
        mov     eax, [ecx]
        mov     dword [ecx], 0
        mov     [sectors_todo], eax
; 2. Acquire the global lock.
        mov     ecx, ide_mutex
        call    mutex_lock
; 3. Convert parameters to the form suitable for worker procedures.
; Underlying procedures do not know about 64-bit sectors.
; Worker procedures use global variables and edi for [buffer].
        cmp     dword [startsector+4], 0
        jnz     .fail
        and     [hd_error], 0
        mov     eax, [userdata]
        mov     [hdpos], eax
        mov     eax, dword [startsector]
        mov     edi, [buffer]
; 4. Worker procedures take one sectors per time, so loop over all sectors to read.
.sectors_loop:
        call    bd_read
        cmp     [hd_error], 0
        jnz     .fail
        mov     ecx, [numsectors]
        inc     dword [ecx]     ; one more sector is read
        dec     [sectors_todo]
        jz      .done
        inc     eax
        jnz     .sectors_loop
; 5. Loop is done, either due to error or because everything is done.
; Release the global lock and return the corresponding status.
.fail:
        mov     ecx, ide_mutex
        call    mutex_unlock
        or      eax, -1
        ret
.done:
        mov     ecx, ide_mutex
        call    mutex_unlock
        xor     eax, eax
        ret
endp

proc bd_write_interface stdcall uses esi edi, \
        userdata, buffer, startsector:qword, numsectors
        ; userdata = old [hdpos] = 80h + index in NumBiosDisks
        ; buffer = pointer to buffer with data
        ; startsector = 64-bit start sector
        ; numsectors = pointer to number of sectors on input,
        ;  must be filled with number of sectors really written
locals
sectors_todo    dd      ?
endl
; 1. Initialize number of sectors: get number of requested sectors
; and say that no sectors were read yet.      
        mov     ecx, [numsectors]
        mov     eax, [ecx]
        mov     dword [ecx], 0
        mov     [sectors_todo], eax
; 2. Acquire the global lock.
        mov     ecx, ide_mutex
        call    mutex_lock
; 3. Convert parameters to the form suitable for worker procedures.
; Underlying procedures do not know about 64-bit sectors.
; Worker procedures use global variables and esi for [buffer].
        cmp     dword [startsector+4], 0
        jnz     .fail
        and     [hd_error], 0
        mov     eax, [userdata]
        mov     [hdpos], eax
        mov     esi, [buffer]
        lea     edi, [startsector]
        mov     [cache_chain_ptr], edi
; 4. Worker procedures take max 16 sectors per time,
; loop until all sectors will be processed.
.sectors_loop:
        mov     ecx, 16
        cmp     ecx, [sectors_todo]
        jbe     @f
        mov     ecx, [sectors_todo]
@@:
        mov     [cache_chain_size], cl
        call    bd_write_cache_chain
        cmp     [hd_error], 0
        jnz     .fail
        movzx   ecx, [cache_chain_size]
        mov     eax, [numsectors]
        add     [eax], ecx
        sub     [sectors_todo], ecx
        jz      .done
        add     [edi], ecx
        jc      .fail
        shl     ecx, 9
        add     esi, ecx
        jmp     .sectors_loop
; 5. Loop is done, either due to error or because everything is done.
; Release the global lock and return the corresponding status.
.fail:
        mov     ecx, ide_mutex
        call    mutex_unlock
        or      eax, -1
        ret
.done:
        mov     ecx, ide_mutex
        call    mutex_unlock
        xor     eax, eax
        ret
endp

; This is a stub.
proc bd_querymedia stdcall, hd_data, mediainfo
        mov     eax, [mediainfo]
        mov     [eax+DISKMEDIAINFO.Flags], 0
        mov     [eax+DISKMEDIAINFO.SectorSize], 512
        or      dword [eax+DISKMEDIAINFO.Capacity], 0xFFFFFFFF
        or      dword [eax+DISKMEDIAINFO.Capacity+4], 0xFFFFFFFF
        xor     eax, eax
        ret
endp

;-----------------------------------------------------------------------------
align 4
; input: eax = sector, edi -> buffer
; output: edi = edi + 512
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

        mov     ecx, 256
        mov     edx, [hdbase]
        cld
        rep insw
        popfd

        pop     edx eax
        ret
;-----------------------------------------------------------------------------
align 4
; edi -> sector, esi -> data
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
        mov     eax, [edi]
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
        mov     eax, [edi]      ; eax = sector to write
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
        mov     eax, [edi]
        rol     eax, 8
        out     dx, al ; LBA Low Previous LBA (31:24)
        xor     eax, eax ; because only 32 bit cache
        inc     edx
        out     dx, al ; LBA Mid Previous LBA (39:32)
        inc     edx
        out     dx, al ; LBA High Previous LBA (47:40)
        sub     edx, 2
        mov     eax, [edi]
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
        dd IDE_DMA
        dw 0x2000
        dw 0x8000

dma_cur_sector  dd not 40h
dma_hdpos       dd 0
irq14_func      dd hdd_irq_null
irq15_func      dd hdd_irq_null
endg
;-----------------------------------------------------------------------------
uglobal
; all uglobals are zeroed at boot
dma_process         dd 0
dma_slot_ptr        dd 0
cache_chain_pos     dd 0
cache_chain_ptr     dd 0
cache_chain_size    db 0
cache_chain_started db 0
dma_task_switched   db 0
dma_hdd             db 0
allow_dma_access    db 0
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
        push    ecx esi
        mov     esi, eax

        mov     ecx, 512/4
        cld
        rep movsd
        pop     esi ecx
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
cache_write_dma:
        mov     eax, [cache_chain_ptr]
        push    esi
        mov     eax, IDE_descriptor_table
        mov     edx, eax
        pusha
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
IDEContrRegsBaseAddr         dw ?
IDEContrProgrammingInterface dw ?
IDE_BAR0_val    dw ?
IDE_BAR1_val    dw ?
IDE_BAR2_val    dw ?
IDE_BAR3_val    dw ?
endg
;-----------------------------------------------------------------------------
; \begin{diamond}
uglobal
bios_hdpos      dd 0       ; 0 is invalid value for [hdpos]
bios_cur_sector dd ?
bios_read_len   dd ?
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
        push    ecx esi
        mov     esi, eax
        mov     ecx, 512/4
        cld
        rep movsd
        pop     esi ecx
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
int13_regs_in   rb sizeof.v86_regs
int13_regs_out  rb sizeof.v86_regs
endg
;-----------------------------------------------------------------------------
align 4
int13_call:
; Because this code uses fixed addresses,
; it can not be run simultaniously by many threads.
; In current implementation it is protected by common mutex 'ide_status'
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
