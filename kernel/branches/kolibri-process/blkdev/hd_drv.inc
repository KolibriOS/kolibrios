;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 4420 $


; Low-level driver for HDD access
; DMA support by Mario79
; LBA48 support by Mario79
;-----------------------------------------------------------------------------
struct HD_DATA
hdbase  dd      ?
hdid    dd      ?
hdpos   dd      ?
ends
;-----------------------------------------------------------------------------
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

hd0_data        HD_DATA         ?,    0, 1
hd1_data        HD_DATA         ?, 0x10, 2
hd2_data        HD_DATA         ?,    0, 3
hd3_data        HD_DATA         ?, 0x10, 4
hd4_data        HD_DATA         ?,    0, 5
hd5_data        HD_DATA         ?, 0x10, 6
hd6_data        HD_DATA         ?,    0, 7
hd7_data        HD_DATA         ?, 0x10, 8
hd8_data        HD_DATA         ?,    0, 9
hd9_data        HD_DATA         ?, 0x10, 10
hd10_data       HD_DATA         ?,    0, 11
hd11_data       HD_DATA         ?, 0x10, 12

ide_mutex_table:
        dd ide_channel1_mutex
        dd ide_channel2_mutex
        dd ide_channel3_mutex
        dd ide_channel4_mutex
        dd ide_channel5_mutex
        dd ide_channel6_mutex
endg
;-----------------------------------------------------------------------------
uglobal
ide_mutex               MUTEX
ide_channel1_mutex      MUTEX
ide_channel2_mutex      MUTEX
ide_channel3_mutex      MUTEX
ide_channel4_mutex      MUTEX
ide_channel5_mutex      MUTEX
ide_channel6_mutex      MUTEX
endg
;-----------------------------------------------------------------------------
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

        mov     ecx, [hd_data]
        mov     ecx, [ecx+HD_DATA.hdpos]
        dec     ecx
        shr     ecx, 1
        shl     ecx, 2
        mov     ecx, [ecx + ide_mutex_table]
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

        push    eax ecx
        mov     ecx, [hdpos]
        dec     ecx
        shr     ecx, 2
        imul    ecx, sizeof.IDE_DATA
        add     ecx, IDE_controller_1
        mov     [IDE_controller_pointer], ecx

        mov     eax, [hdpos]
        dec     eax
        and     eax, 11b
        shr     eax, 1
        add     eax, ecx
        cmp     [eax+IDE_DATA.dma_hdd_channel_1], 1
        pop     ecx eax
        jnz     .nodma

        call    hd_read_dma
        jmp     @f
;--------------------------------------
.nodma:
        call    hd_read_pio
;--------------------------------------
@@:
        cmp     [hd_error], 0
        jnz     .fail

        mov     ecx, [numsectors]
        inc     dword [ecx]     ; one more sector is read
        dec     [sectors_todo]
        jz      .done

        inc     eax
        jnz     .sectors_loop
;--------------------------------------
; 5. Loop is done, either due to error or because everything is done.
; Release the global lock and return the corresponding status.
.fail:
        mov     ecx, [channel_lock]
        call    mutex_unlock

        mov     ecx, ide_mutex
        call    mutex_unlock

        or      eax, -1
        ret
;--------------------------------------
.done:
        mov     ecx, [channel_lock]
        call    mutex_unlock

        mov     ecx, ide_mutex
        call    mutex_unlock

        xor     eax, eax
        ret
endp
;-----------------------------------------------------------------------------
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

        mov     ecx, [hd_data]
        mov     ecx, [ecx+HD_DATA.hdpos]
        dec     ecx
        shr     ecx, 1
        shl     ecx, 2
        mov     ecx, [ecx + ide_mutex_table]
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
;--------------------------------------
@@:
        mov     [cache_chain_size], cl
; DMA write is permitted only if [allow_dma_access]=1
        cmp     [allow_dma_access], 2
        jae     .nodma

        push    eax ecx
        mov     ecx, [hdpos]
        dec     ecx
        shr     ecx, 2
        imul    ecx, sizeof.IDE_DATA
        add     ecx, IDE_controller_1
        mov     [IDE_controller_pointer], ecx

        mov     eax, [hdpos]
        dec     eax
        and     eax, 11b
        shr     eax, 1
        add     eax, ecx
        cmp     [eax+IDE_DATA.dma_hdd_channel_1], 1
        pop     ecx eax
        jnz     .nodma

        call    cache_write_dma
        jmp     .common
;--------------------------------------
.nodma:
        mov     [cache_chain_size], 1
        call    cache_write_pio
;--------------------------------------
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
;--------------------------------------
; 5. Loop is done, either due to error or because everything is done.
; Release the global lock and return the corresponding status.
.fail:
        mov     ecx, [channel_lock]
        call    mutex_unlock

        mov     ecx, ide_mutex
        call    mutex_unlock

        or      eax, -1
        ret
;--------------------------------------
.done:
        mov     ecx, [channel_lock]
        call    mutex_unlock

        mov     ecx, ide_mutex
        call    mutex_unlock

        xor     eax, eax
        ret
endp
;-----------------------------------------------------------------------------
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
;--------------------------------------
.continue:
        popfd
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
;--------------------------------------
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
;--------------------------------------
@@:
        mov     [hd_error], 1
;--------------------------------------
buf_wait_ok:
        pop     edx eax
        ret
;-----------------------------------------------------------------------------
irq14_num equ byte 14
irq15_num equ byte 15
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

        cmp     [IDE_common_irq_param], 0
        jz      .done

        call    check_hd_wait_timeout

        cmp     [hd_error], 0
        jz      .wait

        mov     [IDE_common_irq_param], 0
;--------------------------------------
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

        cmp     [IDE_common_irq_param], 0
        jz      .done

        call    check_hd_wait_timeout

        cmp     [hd_error], 0
        jz      .wait

        mov     [IDE_common_irq_param], 0
;--------------------------------------
.done:
        pop     edx
        pop     eax
        ret
;-----------------------------------------------------------------------------
iglobal
align 4
; note that IDE descriptor table must be 4-byte aligned
; and do not cross 4K boundary
IDE_descriptor_table:
        dd IDE_DMA
        dw 0x2000
        dw 0x8000

dma_cur_sector  dd not 40h
dma_hdpos       dd 0
IDE_common_irq_param db 0
endg
;-----------------------------------------------------------------------------
uglobal
; all uglobals are zeroed at boot
cache_chain_ptr     dd 0
cache_chain_size    db 0
allow_dma_access    db 0
endg
;-----------------------------------------------------------------------------
align 4
IDE_irq_14_handler:
;        DEBUGF  1, 'K : IDE_irq_14_handler %x\n', [IDE_common_irq_param]:2
        cmp     [IDE_common_irq_param], irq14_num
        jne     .exit

        pushfd
        cli
        pushad
        mov     [IDE_common_irq_param], 0
        mov     ecx, [IDE_controller_pointer]
        mov     dx, [ecx+IDE_DATA.RegsBaseAddres]
; test whether it is our interrupt?
        add     edx, 2
        in      al, dx
        test    al, 100b
        jz      @f
; clear Bus Master IDE Status register
; clear Interrupt bit
        out     dx, al
; clear Bus Master IDE Command register
        sub     edx, 2
        xor     eax, eax
        out     dx, al
; read status register and remove the interrupt request
        mov     edx, [hdbase]
        add     edx, 0x7
        in      al, dx
        popad
        popfd
        mov     al, 1
        ret
;--------------------------------------
@@:
        popad
        popfd
;--------------------------------------
.exit:
        mov     al, 0
        ret
;-----------------------------------------------------------------------------
align 4
IDE_irq_15_handler:
;        DEBUGF  1, 'K : IDE_irq_15_handler %x\n', [IDE_common_irq_param]:2
        cmp     [IDE_common_irq_param], irq15_num
        jne     .exit

        pushfd
        cli
        pushad
        mov     [IDE_common_irq_param], 0
        mov     ecx, [IDE_controller_pointer]
        mov     dx, [ecx+IDE_DATA.RegsBaseAddres]
        add     dx, 8
; test whether it is our interrupt?
        add     edx, 2
        in      al, dx
        test    al, 100b
        jz      @f
; clear Bus Master IDE Status register
; clear Interrupt bit
        out     dx, al
; clear Bus Master IDE Command register
        sub     edx, 2
        mov     al, 0
        out     dx, al
; read status register and remove the interrupt request
        mov     edx, [hdbase]
        add     edx, 0x7
        in      al, dx
        popad
        popfd
        mov     al, 1
        ret
;--------------------------------------
@@:
        popad
        popfd
;--------------------------------------
.exit:
        mov     al, 0
        ret
;-----------------------------------------------------------------------------
align 4
IDE_common_irq_handler:
;        DEBUGF  1, 'K : IDE_common_irq_handler %x\n', [IDE_common_irq_param]:2
        pushfd
        cli
        cmp     [IDE_common_irq_param], 0
        je      .exit

        pushad
        xor     ebx, ebx
        mov     ecx, [IDE_controller_pointer]
        mov     dx, [ecx+IDE_DATA.RegsBaseAddres]
        mov     eax, IDE_common_irq_param
        cmp     [eax], irq14_num
        mov     [eax], bl
        je      @f

        add     dx, 8
;--------------------------------------
@@:
; test whether it is our interrupt?
        add     edx, 2
        in      al, dx
        test    al, 100b
        jz      @f
; clear Bus Master IDE Status register
; clear Interrupt bit
        out     dx, al
; clear Bus Master IDE Command register
        sub     edx, 2
        xor     eax, eax
        out     dx, al
; read status register and remove the interrupt request
        mov     edx, [hdbase]
        add     edx, 0x7
        in      al, dx
        popad
        popfd
        mov     al, 1
        ret
;--------------------------------------
@@:
        popad
;--------------------------------------
.exit:
        popfd
        mov     al, 0
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
;--------------------------------------
.notread:
; set data for PRD Table
        mov     eax, IDE_descriptor_table
        mov     dword [eax], IDE_DMA
        mov     word [eax+4], 0x2000
        sub     eax, OS_BASE
; select controller Primary or Secondary
        mov     ecx, [IDE_controller_pointer]
        mov     dx, [ecx+IDE_DATA.RegsBaseAddres]

        push    eax
        mov     eax, [hdpos]
        dec     eax
        test    eax, 10b
        pop     eax
        jz      @f

        add     edx, 8
;--------------------------------------
@@:
        push    edx
; Bus Master IDE PRD Table Address
        add     edx, 4
; save IDE_descriptor_table
        out     dx, eax
        pop     edx
; clear Bus Master IDE Command register
        mov     al, 0
        out     dx, al
; clear Bus Master IDE Status register
; clear Error bit and Interrupt bit
        add     edx, 2
        mov     al, 6 ; 110b
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
; select controller Primary or Secondary
        mov     ecx, [IDE_controller_pointer]
        mov     dx, [ecx+IDE_DATA.RegsBaseAddres]

        mov     eax, [hdpos]
        dec     eax
        test    eax, 10b
        jz      @f

        add     dx, 8
;--------------------------------------
@@:
; set write to memory and Start Bus Master
        mov     al, 9
        out     dx, al

        mov     eax, [hdpos]
        dec     eax
        test    eax, 10b
        jnz     .ide1

        mov     [IDE_common_irq_param], irq14_num
        jmp     @f
;--------------------------------------
.ide1:
        mov     [IDE_common_irq_param], irq15_num
;--------------------------------------
@@:
        popfd
; wait for interrupt
        mov     eax, [hdpos]
        dec     eax
        test    eax, 10b
        jnz     .wait_ide1

        call    wait_for_sector_dma_ide0
        jmp     @f
;--------------------------------------
.wait_ide1:
        call    wait_for_sector_dma_ide1
;--------------------------------------
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
        mov     eax, [cache_chain_ptr] ; for what?
        push    esi
; set data for PRD Table
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
; select controller Primary or Secondary
        mov     ecx, [IDE_controller_pointer]
        mov     dx, [ecx+IDE_DATA.RegsBaseAddres]

        push    eax
        mov     eax, [hdpos]
        dec     eax
        test    eax, 10b
        pop     eax
        jz      @f

        add     edx, 8
;--------------------------------------
@@:
        push    edx
; Bus Master IDE PRD Table Address
        add     edx, 4
; save IDE_descriptor_table
        out     dx, eax
        pop     edx
; clear Bus Master IDE Command register
        mov     al, 0
        out     dx, al
; clear Bus Master IDE Status register
; clear Error bit and Interrupt bit
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
; select controller Primary or Secondary
        mov     ecx, [IDE_controller_pointer]
        mov     dx, [ecx+IDE_DATA.RegsBaseAddres]

        mov     eax, [hdpos]
        dec     eax
        test    eax, 10b
        jz      @f

        add     dx, 8
;--------------------------------------
@@:
; set write to device and Start Bus Master
        mov     al, 1
        out     dx, al

        mov     eax, [hdpos]
        dec     eax
        test    eax, 10b
        jnz     .ide1

        mov     [IDE_common_irq_param], irq14_num
        jmp     @f
;--------------------------------------
.ide1:
        mov     [IDE_common_irq_param], irq15_num
;--------------------------------------
@@:
        popfd
; wait for interrupt
        mov     [dma_cur_sector], not 0x40

        mov     eax, [hdpos]
        dec     eax
        test    eax, 10b
        jnz     .wait_ide1

        call    wait_for_sector_dma_ide0

        jmp     @f
;--------------------------------------
.wait_ide1:
        call    wait_for_sector_dma_ide1
;--------------------------------------
@@:
        cmp     [hd_error], 0
        jnz     hd_write_error_dma
        pop     esi
        ret
;-----------------------------------------------------------------------------
proc clear_pci_ide_interrupts
        mov     esi, pcidev_list
;--------------------------------------
align 4
.loop:
        mov     esi, [esi+PCIDEV.fd]
        cmp     esi, pcidev_list
        jz      .done

;        cmp     [esi+PCIDEV.class], 0x01018F
        mov     eax, [esi+PCIDEV.class]
        shr     eax, 4
        cmp     eax, 0x01018
        jnz     .loop

        mov     ah, [esi+PCIDEV.bus]
        mov     al, 2
        mov     bh, [esi+PCIDEV.devfn]
        mov     bl, 0x20
        call    pci_read_reg

        and     eax, 0FFFCh
        mov     edx, eax
        add     edx, 2
        in      al, dx
        DEBUGF 1,'K : clear_pci_ide_interrupts: port[%x] = %x ',dx,al
        out     dx, al
        in      al, dx
        DEBUGF 1,'-> %x; ',al
        add     edx, 8
        in      al, dx
        DEBUGF 1,'port[%x] = %x ',dx,al
        out     dx, al
        in      al, dx
        DEBUGF 1,'-> %x\n',al
        jmp     .loop
;--------------------------------------
.done:
        ret
endp
;-----------------------------------------------------------------------------
