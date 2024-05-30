;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; HDD driver

struct HD_DATA
hdpos   dw  ?
hdid    dw  ?
hdbase  dw  ?
hd48    dw  ?
sectors dq  ?
ends
;-----------------------------------------------------------------
iglobal
align 4
ide_callbacks:
    dd  ide_callbacks.end - ide_callbacks
    dd  0   ; no close function
    dd  0   ; no closemedia function
    dd  ide_querymedia
    dd  ide_read
    dd  ide_write
    dd  0   ; no flush function
    dd  0   ; use default cache size
.end:

hd0_data    HD_DATA     1,  0
hd1_data    HD_DATA     2,  16
hd2_data    HD_DATA     3,  0
hd3_data    HD_DATA     4,  16
hd4_data    HD_DATA     5,  0
hd5_data    HD_DATA     6,  16
hd6_data    HD_DATA     7,  0
hd7_data    HD_DATA     8,  16
hd8_data    HD_DATA     9,  0
hd9_data    HD_DATA     10, 16
hd10_data   HD_DATA     11, 0
hd11_data   HD_DATA     12, 16

ide_mutex_table:
    dd  ide_channel1_mutex
    dd  ide_channel2_mutex
    dd  ide_channel3_mutex
    dd  ide_channel4_mutex
    dd  ide_channel5_mutex
    dd  ide_channel6_mutex
endg
;-----------------------------------------------------------------
uglobal
ide_mutex               MUTEX
ide_channel1_mutex      MUTEX
ide_channel2_mutex      MUTEX
ide_channel3_mutex      MUTEX
ide_channel4_mutex      MUTEX
ide_channel5_mutex      MUTEX
ide_channel6_mutex      MUTEX
blockSize:
rb 4
sector:
rb 6
allow_dma_access        db ?
IDE_common_irq_param    db ?
eventPointer            dd ?
eventID                 dd ?
endg
;-----------------------------------------------------------------
ide_read:
        mov     al, 25h     ; READ DMA EXT
        jmp     ide_read_write

ide_write:
        mov     al, 35h     ; WRITE DMA EXT
proc ide_read_write stdcall uses esi edi ebx, \
        hd_data, buffer, startsector:qword, numsectors
        ; hd_data = pointer to hd*_data
        ; buffer = pointer to buffer with/for data
        ; startsector = 64-bit start sector
        ; numsectors = pointer to number of sectors on input,
        ;  must be filled with number of sectors really read/written
locals
sectors_todo    dd      ?
channel_lock    dd      ?
endl
        mov     bl, al
; get number of requested sectors and say that no sectors were read yet
        mov     ecx, [numsectors]
        mov     eax, [ecx]
        mov     dword [ecx], 0
        mov     [sectors_todo], eax
; acquire the global lock
        mov     ecx, ide_mutex
        call    mutex_lock
        mov     ecx, [hd_data]
        movzx   ecx, [ecx+HD_DATA.hdpos]
        dec     ecx
        shr     ecx, 1
        shl     ecx, 2
        mov     ecx, [ecx + ide_mutex_table]
        mov     [channel_lock], ecx
        call    mutex_lock
; prepare worker procedures variables
        mov     esi, [buffer]
        mov     edi, esi
        mov     ecx, [hd_data]
        movzx   eax, [ecx+HD_DATA.hdbase]
        mov     [hdbase], eax
        mov     ax, [ecx+HD_DATA.hdid]
        mov     [hdid], eax
        mov     eax, dword [startsector]
        mov     [sector], eax
        cmp     [ecx+HD_DATA.hd48], 0
        jz      .LBA28
        mov     ax, word [startsector+4]
        mov     [sector+4], ax
        movzx   ecx, [ecx+HD_DATA.hdpos]
        mov     [hdpos], ecx
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
        jz      .next
        dec     ebx     ; READ/WRITE SECTOR(S) EXT
; LBA48 supports max 10000h sectors per time
; loop until all sectors will be processed
.next:
        mov     ecx, 8000h
        cmp     ecx, [sectors_todo]
        jbe     @f
        mov     ecx, [sectors_todo]
@@:
        mov     [blockSize], ecx
        push    ecx
        call    IDE_transfer
        pop     ecx
        jc      .out
        mov     eax, [numsectors]
        add     [eax], ecx
        sub     [sectors_todo], ecx
        jz      .out
        add     [sector], ecx
        adc     word [sector+4], 0
        jmp     .next

.LBA28:
        add     eax, [sectors_todo]
        add     eax, 0xF0000000
        jc      .out
        sub     bl, 5   ; READ/WRITE SECTOR(S)
; LBA28 supports max 256 sectors per time
; loop until all sectors will be processed
.next28:
        mov     ecx, 256
        cmp     ecx, [sectors_todo]
        jbe     @f
        mov     ecx, [sectors_todo]
@@:
        mov     [blockSize], ecx
        push    ecx
        call    IDE_transfer.LBA28
        pop     ecx
        jc      .out
        mov     eax, [numsectors]
        add     [eax], ecx
        sub     [sectors_todo], ecx
        jz      .out
        add     [sector], ecx
        jmp     .next28

; loop is done, either due to error or because everything is done
; release the global lock and return the corresponding status
.out:
        sbb     eax, eax
        push    eax
        mov     ecx, [channel_lock]
        call    mutex_unlock
        mov     ecx, ide_mutex
        call    mutex_unlock
        pop     eax
        ret
endp
;-----------------------------------------------------------------
proc ide_querymedia stdcall, hd_data, mediainfo
        mov     eax, [mediainfo]
        mov     edx, [hd_data]
        mov     [eax+DISKMEDIAINFO.Flags], 0
        mov     [eax+DISKMEDIAINFO.SectorSize], 512
        mov     ecx, dword[edx+HD_DATA.sectors]
        mov     dword[eax+DISKMEDIAINFO.Capacity], ecx
        mov     ecx, dword[edx+HD_DATA.sectors+4]
        mov     dword[eax+DISKMEDIAINFO.Capacity+4], ecx
        xor     eax, eax
        ret
endp
;-----------------------------------------------------------------
; input: esi -> buffer, bl = command, [sector], [blockSize]
; output: esi -> next block in buffer
; for pio read esi equal edi
IDE_transfer:
        mov     edx, [hdbase]
        add     edx, 6
        mov     al, byte [hdid]
        add     al, 224
        out     dx, al  ; select the desired drive
        call    save_hd_wait_timeout
        inc     edx
@@:
        call    check_hd_wait_timeout
        jc      .hd_error
        in      al, dx
        test    al, 128 ; ready for command?
        jnz     @b
        pushfd          ; fill the ports
        cli
        mov     edx, [hdbase]
        inc     edx
        inc     edx
        mov     al, [blockSize+1]
        out     dx, al  ; Sector count (15:8)
        inc     edx
        mov     eax, [sector+3]
        out     dx, al  ; LBA (31:24)
        inc     edx
        shr     eax, 8
        out     dx, al  ; LBA (39:32)
        inc     edx
        shr     eax, 8
        out     dx, al  ; LBA (47:40)
        sub     edx, 3
        mov     al, [blockSize]
        out     dx, al  ; Sector count (7:0)
        inc     edx
        mov     eax, [sector]
        out     dx, al  ; LBA (7:0)
        inc     edx
        shr     eax, 8
        out     dx, al  ; LBA (15:8)
        inc     edx
        shr     eax, 8
        out     dx, al  ; LBA (23:16)
        inc     edx
        mov     al, byte [hdid]
        add     al, 224
        out     dx, al
        test    bl, 1
        jz      .PIO
; DMA
        mov     dword [esp], 0x1000
        call    kernel_alloc
        mov     edi, eax
        push    eax
        shl     dword [blockSize], 9
        mov     eax, esi
        add     eax, [blockSize]
        push    eax
; check buffer pages physical addresses and fill the scatter-gather list
; buffer may be not aligned and may have size not divisible by page size
; [edi] = block physical address, [edi+4] = block size in bytes
; block addresses can not cross 10000h borders
        mov     ecx, esi
        and     ecx, 0xFFF
        jz      .aligned
        mov     eax, esi
        call    get_pg_addr
        add     eax, ecx
        neg     ecx
        add     ecx, 0x1000
        mov     [edi], eax
        cmp     ecx, [blockSize]
        jnc     .end
        mov     [edi+4], ecx
        add     esi, 0x1000
        add     edi, 8
        sub     [blockSize], ecx
.aligned:
        mov     eax, esi
        call    get_pg_addr
        mov     ecx, eax
        mov     [edi], eax
        and     ecx, 0xFFFF
        neg     ecx
        add     ecx, 0x10000
        cmp     [blockSize], ecx
        jnc     @f
        mov     ecx, [blockSize]
        and     ecx, 0xF000
        jz      .end
@@:
        push    ecx
@@:
        add     esi, 0x1000
        add     eax, 0x1000
        sub     ecx, 0x1000
        jz      @f
        mov     edx, eax
        mov     eax, esi
        call    get_pg_addr
        cmp     eax, edx
        jz      @b
@@:
        pop     edx
        sub     edx, ecx
        mov     [edi+4], edx
        add     edi, 8
        sub     [blockSize], edx
        jnz     .aligned
        sub     edi, 8
        jmp     @f

.end:
        mov     ecx, [blockSize]
        mov     [edi+4], ecx
@@:
        mov     byte [edi+7], 80h   ; list end
        pop     esi
        pop     edi
; select controller Primary or Secondary
        mov     ecx, [IDE_controller_pointer]
        mov     dx, [ecx+IDE_DATA.RegsBaseAddres]
        mov     eax, [hdpos]
        dec     eax
        test    eax, 10b
        jz      @f
        add     edx, 8
@@:
        add     edx, 2      ; Bus Master IDE Status register
        mov     al, 6
        out     dx, al      ; clear Error bit and Interrupt bit

        add     edx, 2      ; Bus Master IDE PRD Table Address
        mov     eax, edi
        call    get_pg_addr
        out     dx, eax     ; send scatter-gather list physical address

        push    edx
        mov     edx, [hdbase]
        add     edx, 7      ; ATACommand
        mov     al, bl
        out     dx, al      ; Start hard drive
        pop     edx

        sub     edx, 4      ; Bus Master IDE Command register
        mov     al, 1       ; set direction
        cmp     bl, 35h     ; write
        jz      @f
        add     al, 8       ; read
@@:
        out     dx, al      ; Start Bus Master
        mov     [IDE_common_irq_param], 14
        mov     eax, [hdpos]
        dec     eax
        test    eax, 10b
        jz      @f
        inc     [IDE_common_irq_param]
@@:
        push    edi esi ebx
        xor     ecx, ecx
        xor     esi, esi
        call    create_event
        mov     [eventPointer], eax
        mov     [eventID], edx
        sti
        mov     ebx, edx
        mov     ecx, 300
        call    wait_event_timeout
        test    eax, eax
        jnz     @f
        dbgstr 'IDE DMA IRQ timeout'
        mov     [IDE_common_irq_param], 0
        mov     eax, [eventPointer]
        mov     ebx, [eventID]
        call    destroy_event
        mov     [eventPointer], 0
@@:
        pop     ebx esi
        call    kernel_free
        cmp     [eventPointer], 0
        jz      .hd_error
        ret

.LBA28:
        mov     edx, [hdbase]
        add     edx, 6
        mov     al, byte [hdid]
        add     al, 224
        out     dx, al  ; select the desired drive
        call    save_hd_wait_timeout
        inc     edx
@@:
        call    check_hd_wait_timeout
        jc      .hd_error
        in      al, dx
        test    al, 128 ; ready for command?
        jnz     @b
        pushfd          ; fill the ports
        cli
        mov     edx, [hdbase]
        inc     edx
        inc     edx
        mov     al, [blockSize]
        out     dx, al  ; Sector count (7:0)
        inc     edx
        mov     eax, [sector]
        out     dx, al  ; LBA (7:0)
        inc     edx
        shr     eax, 8
        out     dx, al  ; LBA (15:8)
        inc     edx
        shr     eax, 8
        out     dx, al  ; LBA (23:16)
        inc     edx
        shr     eax, 8
        add     al, byte [hdid]
        add     al, 224
        out     dx, al  ; LBA (27:24)
.PIO:
        inc     edx     ; ATACommand
        mov     al, bl
        out     dx, al  ; Start hard drive
        popfd
.sectorTransfer:
        call    save_hd_wait_timeout
        in      al, dx
        in      al, dx
        in      al, dx
        in      al, dx
@@:
        call    check_hd_wait_timeout
        jc      .hd_error
        in      al, dx
        test    al, 8   ; ready for transfer?
        jz      @b
        cmp     [hd_setup], 1   ; do not mark error for setup request
        jz      @f
        test    al, 1   ; previous command ended up with an error
        jnz     .pio_error
@@:
        pushfd
        cli
        cld
        mov     ecx, 256
        mov     edx, [hdbase]
        cmp     bl, 30h
        jnc     .write
        rep insw
        jmp     @f

.write:
        rep outsw
@@:
        popfd
        add     edx, 7
        dec     dword [blockSize]
        jnz     .sectorTransfer
        ret

.pio_error:
        dbgstr 'IDE PIO transfer error'
.hd_error:
        cmp     bl, 30h
        jnc     hd_write_error
;-----------------------------------------------------------------
hd_read_error:
        dbgstr 'HD read error'
        stc
        ret
;-----------------------------------------------------------------
hd_write_error:
        dbgstr 'HD write error'
        stc
        ret
;-----------------------------------------------------------------
save_hd_wait_timeout:
        mov     eax, [timer_ticks]
        add     eax, 300        ; 3 sec timeout
        mov     [hd_wait_timeout], eax
        ret
;-----------------------------------------------------------------
check_hd_wait_timeout:
        mov     eax, [timer_ticks]
        cmp     [hd_wait_timeout], eax
        jc      @f
        ret

@@:
        dbgstr 'IDE device timeout'
        stc
        ret
;-----------------------------------------------------------------
align 4
IDE_irq_14_handler:
IDE_irq_15_handler:
IDE_common_irq_handler:
; Most of the time, we are here because we have requested
; a DMA transfer for the corresponding drive.
; However,
; (a) we can be here because IDE IRQ is shared with some other device,
;    that device has actually raised IRQ,
;    it has nothing to do with IDE;
; (b) we can be here because IDE controller just does not want
;    to be silent and reacts to something even though
;    we have, in theory, disabled IRQs.
; If the interrupt corresponds to our current request,
; remove the interrupt request and raise the event for the waiting code.
; In the case (a), just return zero - not our interrupt.
; In the case (b), remove the interrupt request and hope for the best.
; DEBUGF 1, 'K : IDE_irq_handler %x\n', [IDE_common_irq_param]:2
        mov     ecx, [esp+4]
        mov     dx, [ecx+IDE_DATA.RegsBaseAddres]
        add     edx, 2  ; Bus Master IDE Status register
        in      al, dx
        test    al, 4
        jnz     .interrupt_from_primary
        add     edx, 8
        in      al, dx
        test    al, 4
        jnz     .interrupt_from_secondary
        xor     eax, eax ; not our interrupt
        ret

.interrupt_from_primary:
        out     dx, al  ; clear Interrupt bit
        sub     edx, 2
        xor     eax, eax
        out     dx, al  ; clear Bus Master IDE Command register
        mov     dx, [ecx+IDE_DATA.BAR0_val]
        add     edx, 7
        in      al, dx  ; read status register
        cmp     [IDE_common_irq_param], 14
        jz      .raise
.exit_our:
        mov     al, 1
        ret

.interrupt_from_secondary:
        out     dx, al  ; clear Interrupt bit
        sub     edx, 2
        xor     eax, eax
        out     dx, al  ; clear Bus Master IDE Command register
        mov     dx, [ecx+IDE_DATA.BAR2_val]
        add     edx, 7
        in      al, dx  ; read status register
        cmp     [IDE_common_irq_param], 15
        jnz     .exit_our
.raise:
        cmp     ecx, [IDE_controller_pointer]
        jnz     .exit_our
        pushad
        mov     eax, [eventPointer]
        mov     ebx, [eventID]
        xor     edx, edx
        xor     esi, esi
        call    raise_event
        popad
        mov     al, 1   ; remove the interrupt request
        ret
