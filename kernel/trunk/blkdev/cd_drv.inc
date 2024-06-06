;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;-----------------------------------------------------------------------------
;**********************************************************
;  Direct work with CD (ATAPI) device
;**********************************************************
; Author of a part of the source code -  Kulakov Vladimir Gennadievich
; Adaptation, revision and development -  Mario79, <Lrz>

; Maximum number of repeats of a read operation
MaxRetr = 10
; Maximum waiting time for ready to receive a command
; (in ticks)
BSYWaitTime = 1000  ;2
NoTickWaitTime = 0xfffff
CDBlockSize = 2048
;********************************************
;*        READING SECTOR WITH REPEATS       *
;*      Repeated reads on failures          *
;********************************************
ReadCDWRetr:
;-----------------------------------------------------------
; input  : eax = block to read
;          ebx = destination
;-----------------------------------------------------------
        pushad
        mov     eax, [CDSectorAddress]
        mov     ebx, [CDDataBuf_pointer]
        call    cd_calculate_cache
        xor     edi, edi
        add     esi, 8
        inc     edi
;--------------------------------------
align 4
.hdreadcache:
        cmp     [esi], eax      ; correct sector
        je      .yeshdcache

        add     esi, 8
        inc     edi
        dec     ecx
        jnz     .hdreadcache

        call    find_empty_slot_CD_cache ; ret in edi

        push    edi
        push    eax
        call    cd_calculate_cache_2
        shl     edi, 11
        add     edi, eax
        mov     [CDDataBuf_pointer], edi
        pop     eax
        pop     edi

        call    ReadCDWRetr_1
        cmp     [DevErrorCode], 0
        jne     .exit

        mov     [CDDataBuf_pointer], ebx
        call    cd_calculate_cache_1
        lea     esi, [edi*8+esi]
        mov     [esi], eax      ; sector number
;--------------------------------------
.yeshdcache:
        mov     esi, edi
        shl     esi, 11 ;9
        push    eax
        call    cd_calculate_cache_2
        add     esi, eax
        pop     eax
        mov     edi, ebx ;[CDDataBuf_pointer]
        mov     ecx, 512 ;/4
        cld
        rep movsd               ; move data
;--------------------------------------
.exit:
        popad
        ret
;-----------------------------------------------------------------------------
ReadCDWRetr_1:
        pushad
; Loop until the command is successful or the number of attempts is over
        mov     ecx, MaxRetr
;--------------------------------------
align 4
@@NextRetr:
; Send a command
;*************************************************
;*      FULL READ OF COMPACT DISK SECTOR         *
;* User data, subchannel                         *
;* information and control information are read  *
;* Input parameters are passed through global    *
;* variables:                                    *
;* ChannelNumber - channel number;               *
;* DiskNumber - disc number on channel;          *
;* CDSectorAddress - address of reading sector.  *
;* The data is read into the CDDataBuf array.    *
;*************************************************
;ReadCD:
        push    ecx
; Flush the packet command buffer
        call    clear_packet_buffer
; Generate a packet command to read a data sector
; Set the command code Read CD
        mov     [PacketCommand], byte 0x28 ;0xBE
; Set the sector address
        mov     ax, word [CDSectorAddress+2]
        xchg    al, ah
        mov     word [PacketCommand+2], ax
        mov     ax, word [CDSectorAddress]
        xchg    al, ah
        mov     word [PacketCommand+4], ax
; Set the number of sectors to read
        mov     [PacketCommand+8], byte 1
; Send a command
        call    SendPacketDatCommand
        pop     ecx

        test    eax, eax
        jz      @@End_4

        or      ecx, ecx        ;{SPraid.simba} (for cd load)
        jz      @@End_4

        dec     ecx

        cmp     [timer_ticks_enable], 0
        jne     @f

        mov     eax, NoTickWaitTime
;--------------------------------------
align 4
.wait:
        dec     eax
        jz      @@NextRetr

        jmp     .wait
;--------------------------------------
align 4
@@:
        loop    @@NextRetr
;--------------------------------------
@@End_4:
        mov     dword [DevErrorCode], eax
        popad
        ret
;-----------------------------------------------------------------------------
; General purpose procedures to execute packet commands in PIO Mode
; Maximum allowable waiting time for the device to respond to a packet command (in ticks)
;-----------------------------------------------------------------------------
MaxCDWaitTime = 1000 ;200 ;10 seconds
uglobal
; Memory area for generating a packet command
PacketCommand:
                 rb 12  ;DB 12 DUP (?)
; address of reading data sector
CDSectorAddress:   dd ?
; Start time of the next disk operation
TickCounter_1     dd 0
; Time to start waiting for device readiness
WURStartTime      dd 0
; pointer to buffer to read data into
CDDataBuf_pointer dd 0
endg
;-----------------------------------------------------------------------------
;****************************************************
;*    SEND TO ATAPI DEVICE PACKET COMMAND,          *
;* THAT MEANS TRASMIT ONE DATA SECTOR OF SIZE       *
;*     2048 BYTE FROM DEVICE TO HOST                *
;* Input parameters are passed through global       *
;* variables:                                       *
;* ChannelNumber - channel number;                  *
;* DiskNumber - disk number on channel.             *
;* PacketCommand - 12-byte command packet;          *
;* CDBlockSize - size of receiving data block.      *
; return eax DevErrorCode
;****************************************************
SendPacketDatCommand:
        xor     eax, eax
; Set CHS mode
        mov     byte [ATAAddressMode], al
; Send ATA command to send packet command
        mov     byte [ATAFeatures], al
        mov     byte [ATASectorCount], al
        mov     byte [ATASectorNumber], al
; Load the size of the sending block
        mov     [ATAHead], al
        mov     [ATACylinder], CDBlockSize
        mov     [ATACommand], 0xA0
        call    SendCommandToHDD_1
        test    eax, eax
        jnz     @@End_8    ; finish, saving the error code
; Waiting for the drive to be ready to receive a packet command
        mov     dx, [ATABasePortAddr]
        add     dx, 7    ; port 1x7h
        mov     ecx, NoTickWaitTime
;--------------------------------------
align 4
@@WaitDevice0:
        cmp     [timer_ticks_enable], 0
        jne     @f

        dec     ecx
        jz      @@Err1_1

        jmp     .test
;--------------------------------------
align 4
@@:
        call    change_task
        ; Check command execution time
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter_1]
        cmp     eax, BSYWaitTime
        ja      @@Err1_1   ; time out error
        ; Check readiness
;--------------------------------------
align 4
.test:
        in      al, dx
        test    al, 0x80  ; BSY signal state
        jnz     @@WaitDevice0

        test    al, 1     ; ERR signal state
        jnz     @@Err6

        test    al, 0x8   ; DRQ signal state
        jz      @@WaitDevice0
; Send a packet command
        cli
        mov     dx, [ATABasePortAddr]
        mov     ax, [PacketCommand]
        out     dx, ax
        mov     ax, [PacketCommand+2]
        out     dx, ax
        mov     ax, [PacketCommand+4]
        out     dx, ax
        mov     ax, [PacketCommand+6]
        out     dx, ax
        mov     ax, [PacketCommand+8]
        out     dx, ax
        mov     ax, [PacketCommand+10]
        out     dx, ax
        sti
; Waiting for data to be ready
        mov     dx, [ATABasePortAddr]
        add     dx, 7  ; port 1x7h
        mov     ecx, NoTickWaitTime
;--------------------------------------
align 4
@@WaitDevice1:
        cmp     [timer_ticks_enable], 0
        jne     @f

        dec     ecx
        jz      @@Err1_1

        jmp     .test_1
;--------------------------------------
align 4
@@:
        call    change_task
        ; Check command execution time
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter_1]
        cmp     eax, MaxCDWaitTime
        ja      @@Err1_1   ; time out error
        ; Check readiness
;--------------------------------------
align 4
.test_1:
        in      al, dx
        test    al, 0x80  ; BSY signal state
        jnz     @@WaitDevice1

        test    al, 1    ; ERR signal state
        jnz     @@Err6_temp

        test    al, 0x8  ; DRQ signal state
        jz      @@WaitDevice1
; Receive data block from controller
        mov     edi, [CDDataBuf_pointer]
        ; Load controller's data register address
        mov     dx, [ATABasePortAddr]
        ; Load the block size in bytes into the counter
        xor     ecx, ecx
        mov     cx, CDBlockSize
        ; Calculate block size in 16-bit words
        shr     cx, 1 ; divide block size by 2
        ; Receive data block
        cli
        cld
        rep insw
        sti
;--------------------------------------
; Successful completion of data receive
@@End_8:
        xor     eax, eax
        ret
;--------------------------------------
; Write error code
@@Err1_1:
        xor     eax, eax
        inc     eax
        ret
;--------------------------------------
@@Err6_temp:
        mov     eax, 7
        ret
;--------------------------------------
@@Err6:
        mov     eax, 6
        ret
;-----------------------------------------------------------------------------
;***********************************************
;*  SEND TO ATAPI DEVICE PACKET COMMAND,       *
;*     THAT DOESNT MEAN TRANSMIT DATA          *
;* Input parameters are passed through global  *
;* variables:                                  *
;* ChannelNumber - channel number;             *
;* DiskNumber - disk number on channel.        *
;* PacketCommand - 12-byte command packet.     *
;***********************************************
SendPacketNoDatCommand:
        pushad
        xor     eax, eax
; Set CHS mode
        mov     byte [ATAAddressMode], al
; Send ATA command to send packet command
        mov     byte [ATAFeatures], al
        mov     byte [ATASectorCount], al
        mov     byte [ATASectorNumber], al
        mov     word [ATACylinder], ax
        mov     byte [ATAHead], al
        mov     [ATACommand], 0xA0
        call    SendCommandToHDD_1
        test    eax, eax
        jnz     @@End_9  ; finish, saving the error code
; Waiting for the drive to be ready to receive a packet command
        mov     dx, [ATABasePortAddr]
        add     dx, 7  ; port 1x7h
;--------------------------------------
align 4
@@WaitDevice0_1:
        call    change_task
        ; Check waiting time
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter_1]
        cmp     eax, BSYWaitTime
        ja      @@Err1_3  ; time out error
        ; Check readiness
        in      al, dx
        test    al, 0x80  ; BSY signal state
        jnz     @@WaitDevice0_1

        test    al, 1     ; ERR signal state
        jnz     @@Err6_1

        test    al, 0x8   ; DRQ signal state
        jz      @@WaitDevice0_1
; Send packet command
;        cli
        mov     dx, [ATABasePortAddr]
        mov     ax, word [PacketCommand]
        out     dx, ax
        mov     ax, word [PacketCommand+2]
        out     dx, ax
        mov     ax, word [PacketCommand+4]
        out     dx, ax
        mov     ax, word [PacketCommand+6]
        out     dx, ax
        mov     ax, word [PacketCommand+8]
        out     dx, ax
        mov     ax, word [PacketCommand+10]
        out     dx, ax
;        sti
        cmp     [ignore_CD_eject_wait], 1
        je      @@clear_DEC
; Waiting for confirmation of command receive
        mov     dx, [ATABasePortAddr]
        add     dx, 7  ; port 1x7h
;--------------------------------------
align 4
@@WaitDevice1_1:
        call    change_task
        ; Check command execution time
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter_1]
        cmp     eax, MaxCDWaitTime
        ja      @@Err1_3   ; time out error
        ; Wait for device release
        in      al, dx
        test    al, 0x80   ; BSY signal state
        jnz     @@WaitDevice1_1

        test    al, 1      ; ERR signal state
        jnz     @@Err6_1

        test    al, 0x40   ; DRDY signal state
        jz      @@WaitDevice1_1
;--------------------------------------
@@clear_DEC:
        and     [DevErrorCode], 0
        popad
        ret
;--------------------------------------
; Write error code
@@Err1_3:
        xor     eax, eax
        inc     eax
        jmp     @@End_9
;--------------------------------------
@@Err6_1:
        mov     eax, 6
;--------------------------------------
@@End_9:
        mov     [DevErrorCode], eax
        popad
        ret
;-----------------------------------------------------------------------------
;****************************************************
;*          SEND COMMAND TO GIVEN DISK              *
;* Input parameters are passed through the global   *
;* variables:                                       *
;* ChannelNumber - channel number (1 or 2);         *
;* DiskNumber - disk number (0 or 1);               *
;* ATAFeatures - "features";                        *
;* ATASectorCount - sector count;                   *
;* ATASectorNumber - initial sector number;         *
;* ATACylinder - initial cylinder number;           *
;* ATAHead - initial head number;                   *
;* ATAAddressMode - addressing mode (0-CHS, 1-LBA); *
;* ATACommand - command code.                       *
;* If the function finished successfully:           *
;* in ATABasePortAddr - base address of HDD;        *
;* in DevErrorCode - zero.                          *
;* If error has occured then in DevErrorCode will   *
;* be the error code.                               *
;****************************************************
SendCommandToHDD_1:
; Check the addressing mode code
        cmp     [ATAAddressMode], 1
        ja      @@Err2_4
; Check the channel number correctness
        movzx   ebx, [ChannelNumber]
        dec     ebx
        cmp     ebx, 1
        ja      @@Err3_4
; Set the base address
        shl     ebx, 2
        mov     eax, [cdpos]
        dec     eax
        shr     eax, 2
        imul    eax, sizeof.IDE_DATA
        add     eax, IDE_controller_1
        add     eax, ebx
        mov     ax, [eax+IDE_DATA.BAR0_val]
        mov     [ATABasePortAddr], ax
; Waiting for HDD ready to receive a command
        ; Choose desired disk
        mov     dx, [ATABasePortAddr]
        add     dx, 6   ; address of the heads register
        mov     al, [DiskNumber]
        cmp     al, 1   ; check the disk number
        ja      @@Err4_4

        shl     al, 4
        or      al, 10100000b
        out     dx, al
        ; Waiting for disk ready
        inc     dx
        mov     eax, [timer_ticks]
        mov     [TickCounter_1], eax
        mov     ecx, NoTickWaitTime
;--------------------------------------
align 4
@@WaitHDReady_2:
        cmp     [timer_ticks_enable], 0
        jne     @f

        dec     ecx
        jz      @@Err1_4

        jmp     .test
;--------------------------------------
align 4
@@:
        call    change_task
        ; Check waiting time
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter_1]
        cmp     eax, BSYWaitTime ;300    ; wait for 3 seconds
        ja      @@Err1_4   ; time out error
;--------------------------------------
align 4
.test:
        in      al, dx ; Read the state register
        ; Check the state of BSY signal
        test    al, 0x80
        jnz     @@WaitHDReady_2
        ; Check the state of DRQ signal
        test    al, 0x8
        jnz     @@WaitHDReady_2
; load command to controller's registers
        cli
        mov     dx, [ATABasePortAddr]
        inc     dx      ; "features" register
        mov     al, [ATAFeatures]
        out     dx, al
        inc     dx      ; sector counter
        mov     al, [ATASectorCount]
        out     dx, al
        inc     dx      ; sector number register
        mov     al, [ATASectorNumber]
        out     dx, al
        inc     dx      ; cylinder number (low byte)
        mov     ax, [ATACylinder]
        out     dx, al
        inc     dx      ; cylinder number (high byte)
        mov     al, ah
        out     dx, al
        inc     dx      ; head number / disk number
        mov     al, [DiskNumber]
        shl     al, 4
        cmp     [ATAHead], 0xF ; check head number
        ja      @@Err5_4

        or      al, [ATAHead]
        or      al, 10100000b
        mov     ah, [ATAAddressMode]
        shl     ah, 6
        or      al, ah
        out     dx, al
; Send command
        mov     al, [ATACommand]
        inc     dx      ; command register
        out     dx, al
        sti
;--------------------------------------
@@End_10:
        xor     eax, eax
        ret
;--------------------------------------
; Write error code
@@Err1_4:
        xor     eax, eax
        inc     eax
        ret
;--------------------------------------
@@Err2_4:
        mov     eax, 2
        ret
;--------------------------------------
@@Err3_4:
        mov     eax, 3
        ret
;--------------------------------------
@@Err4_4:
        mov     eax, 4
        ret
;--------------------------------------
@@Err5_4:
        mov     eax, 5
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*    WAIT FOR THE DEVICE IS READY FOR WORK      *
;* Input parameters are passed through global    *
;* variables:                                    *
;* ChannelNumber - channel number;               *
;* DiskNumber - disk number on channel.          *
;*************************************************
WaitUnitReady:
        pusha
; Remember the peration start time
        mov     eax, [timer_ticks]
        mov     [WURStartTime], eax
; Clear the packet command buffer
        call    clear_packet_buffer
; Generate TEST UNIT READY command
        mov     [PacketCommand], word 0
; waiting loop for device readiness
        mov     ecx, NoTickWaitTime
;--------------------------------------
align 4
@@SendCommand:
        ; Send readiness check command
        call    SendPacketNoDatCommand
        cmp     [timer_ticks_enable], 0
        jne     @f

        cmp     [DevErrorCode], 0
        je      @@End_11

        dec     ecx
        jz      .Error

        jmp     @@SendCommand
;--------------------------------------
align 4
@@:
        call    change_task
        ; Check the error code
        cmp     [DevErrorCode], 0
        je      @@End_11
        ; Check waiting time
        mov     eax, [timer_ticks]
        sub     eax, [WURStartTime]
        cmp     eax, MaxCDWaitTime
        jb      @@SendCommand
;--------------------------------------
.Error:
        ; time out error
        mov     [DevErrorCode], 1
;--------------------------------------
@@End_11:
        popa
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*            FORBID DISK CHANGE                 *
;* Input parameters are passed through global    *
;* variables:                                    *
;* ChannelNumber - channel number;               *
;* DiskNumber - disk number on channel.          *
;*************************************************
prevent_medium_removal:
        pusha
; Clear the packet command buffer
        call    clear_packet_buffer
; Set command code
        mov     [PacketCommand], byte 0x1E
; Set "Forbid" code
        mov     [PacketCommand+4], byte 11b
; Send command
        call    SendPacketNoDatCommand
        mov     eax, ATAPI_IDE0_lock
        add     eax, [cdpos]
        dec     eax
        mov     [eax], byte 1
        popa
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*            ALLOW DISK CHANGE                  *
;* Input parameters are passed through global    *
;* variables:                                    *
;* ChannelNumber - channel number;               *
;* DiskNumber - disk number on channel.          *
;*************************************************
allow_medium_removal:
        pusha
; Clear the packet command buffer
        call    clear_packet_buffer
; Set command code
        mov     [PacketCommand], byte 0x1E
; unset "Forbid" code
        mov     [PacketCommand+4], byte 0
; Send command
        call    SendPacketNoDatCommand
        mov     eax, ATAPI_IDE0_lock
        add     eax, [cdpos]
        dec     eax
        mov     [eax], byte 0
        popa
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*         LOAD DISK TO THE DRIVE                *
;* Input parameters are passed through global    *
;* variables:                                    *
;* ChannelNumber - channel number;               *
;* DiskNumber - disk number on channel.          *
;*************************************************
LoadMedium:
        pusha
; Clear the packet command buffer
        call    clear_packet_buffer
; Generate START/STOP UNIT command
        ; Set command code
        mov     [PacketCommand], word 0x1B
        ; Set disk loading operation
        mov     [PacketCommand+4], word 00000011b
; Send command
        call    SendPacketNoDatCommand
        popa
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*         REMOVE THE DISK FROM THE DRIVE        *
;* Input parameters are passed through global    *
;* variables:                                    *
;* ChannelNumber - channel number;               *
;* DiskNumber - disk number on channel.          *
;*************************************************
EjectMedium:
        pusha
; Clear the packet command buffer
        call    clear_packet_buffer
; Generate START/STOP UNIT command
        ; Set command code
        mov     [PacketCommand], word 0x1B
        ; Set the operation to eject disk
        mov     [PacketCommand+4], word 00000010b
; Send command
        call    SendPacketNoDatCommand
        popa
        ret
;-----------------------------------------------------------------------------
;*************************************************
;* Check the event of pressing the eject button  *
;*                                               *
;* Input parameters are passed through global    *
;* variables:                                    *
;* ChannelNumber - channel number;               *
;* DiskNumber - disk number on channel.          *
;*************************************************
proc check_ATAPI_device_event_has_work?
        mov     eax, [timer_ticks]
        sub     eax, [timer_ATAPI_check]
        cmp     eax, 100
        jb      .no

        xor     eax, eax
        inc     eax
        ret
;--------------------------------------
.no:
        xor     eax, eax
        ret
endp
;-----------------------------------------------------------------------------
align 4
check_ATAPI_device_event:
        pusha
        mov     eax, [timer_ticks]
        sub     eax, [timer_ATAPI_check]
        cmp     eax, 100
        jb      .end_1

        pushfd
        mov     al, [DRIVE_DATA+1]
        and     al, 11b
        cmp     al, 10b
        jz      .ide3
;--------------------------------------
.ide2_1:
        mov     al, [DRIVE_DATA+1]
        and     al, 1100b
        cmp     al, 1000b
        jz      .ide2
;--------------------------------------
.ide1_1:
        mov     al, [DRIVE_DATA+1]
        and     al, 110000b
        cmp     al, 100000b
        jz      .ide1
;--------------------------------------
.ide0_1:
        mov     al, [DRIVE_DATA+1]
        and     al, 11000000b
        cmp     al, 10000000b
        jz      .ide0
;--------------------------------------
.ide7_1:
        mov     al, [DRIVE_DATA+6]
        and     al, 11b
        cmp     al, 10b
        jz      .ide7
;--------------------------------------
.ide6_1:
        mov     al, [DRIVE_DATA+6]
        and     al, 1100b
        cmp     al, 1000b
        jz      .ide6
;--------------------------------------
.ide5_1:
        mov     al, [DRIVE_DATA+6]
        and     al, 110000b
        cmp     al, 100000b
        jz      .ide5
;--------------------------------------
.ide4_1:
        mov     al, [DRIVE_DATA+6]
        and     al, 11000000b
        cmp     al, 10000000b
        jz      .ide4
;--------------------------------------
.ide11_1:
        mov     al, [DRIVE_DATA+11]
        and     al, 11b
        cmp     al, 10b
        jz      .ide11
;--------------------------------------
.ide10_1:
        mov     al, [DRIVE_DATA+11]
        and     al, 1100b
        cmp     al, 1000b
        jz      .ide10
;--------------------------------------
.ide9_1:
        mov     al, [DRIVE_DATA+11]
        and     al, 110000b
        cmp     al, 100000b
        jz      .ide9
;--------------------------------------
.ide8_1:
        mov     al, [DRIVE_DATA+11]
        and     al, 11000000b
        cmp     al, 10000000b
        jz      .ide8
;--------------------------------------
.end:
        popfd
        mov     eax, [timer_ticks]
        mov     [timer_ATAPI_check], eax
;--------------------------------------
.end_1:
        popa
        ret
;-----------------------------------------------------------------------------
.ide3:
        cli
        cmp     [ATAPI_IDE3_lock], 1
        jne     .ide2_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel2_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 2
        mov     [DiskNumber], 1
        mov     [cdpos], 4
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide2_1
;-----------------------------------------------------------------------------
.ide2:
        cli
        cmp     [ATAPI_IDE2_lock], 1
        jne     .ide1_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel2_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 2
        mov     [DiskNumber], 0
        mov     [cdpos], 3
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide1_1
;-----------------------------------------------------------------------------
.ide1:
        cli
        cmp     [ATAPI_IDE1_lock], 1
        jne     .ide0_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel1_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 1
        mov     [cdpos], 2
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide0_1
;-----------------------------------------------------------------------------
.ide0:
        cli
        cmp     [ATAPI_IDE0_lock], 1
        jne     .ide7_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel1_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 0
        mov     [cdpos], 1
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide7_1
;-----------------------------------------------------------------------------
.ide7:
        cli
        cmp     [ATAPI_IDE7_lock], 1
        jne     .ide6_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel4_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 2
        mov     [DiskNumber], 1
        mov     [cdpos], 8
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide6_1
;-----------------------------------------------------------------------------
.ide6:
        cli
        cmp     [ATAPI_IDE6_lock], 1
        jne     .ide5_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel4_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 2
        mov     [DiskNumber], 0
        mov     [cdpos], 7
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide5_1
;-----------------------------------------------------------------------------
.ide5:
        cli
        cmp     [ATAPI_IDE5_lock], 1
        jne     .ide4_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel3_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 1
        mov     [cdpos], 6
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide4_1
;-----------------------------------------------------------------------------
.ide4:
        cli
        cmp     [ATAPI_IDE4_lock], 1
        jne     .ide11_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel3_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 0
        mov     [cdpos], 5
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide11_1
;-----------------------------------------------------------------------------
.ide11:
        cli
        cmp     [ATAPI_IDE11_lock], 1
        jne     .ide10_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel6_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 2
        mov     [DiskNumber], 1
        mov     [cdpos], 12
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide10_1
;-----------------------------------------------------------------------------
.ide10:
        cli
        cmp     [ATAPI_IDE10_lock], 1
        jne     .ide9_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel6_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 2
        mov     [DiskNumber], 0
        mov     [cdpos], 11
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide9_1
;-----------------------------------------------------------------------------
.ide9:
        cli
        cmp     [ATAPI_IDE9_lock], 1
        jne     .ide8_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel5_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 1
        mov     [cdpos], 10
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide8_1
;-----------------------------------------------------------------------------
.ide8:
        cli
        cmp     [ATAPI_IDE8_lock], 1
        jne     .end

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel5_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 0
        mov     [cdpos], 9
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .end
;-----------------------------------------------------------------------------
.eject:
        call    clear_CD_cache
        call    allow_medium_removal
        mov     [ignore_CD_eject_wait], 1
        call    EjectMedium
        mov     [ignore_CD_eject_wait], 0
        ret
;-----------------------------------------------------------------------------
iglobal
timer_ATAPI_check dd 0
ATAPI_IDE0_lock db 0
ATAPI_IDE1_lock db 0
ATAPI_IDE2_lock db 0
ATAPI_IDE3_lock db 0
ATAPI_IDE4_lock db 0
ATAPI_IDE5_lock db 0
ATAPI_IDE6_lock db 0
ATAPI_IDE7_lock db 0
ATAPI_IDE8_lock db 0
ATAPI_IDE9_lock db 0
ATAPI_IDE10_lock db 0
ATAPI_IDE11_lock db 0
ignore_CD_eject_wait db 0
endg
;-----------------------------------------------------------------------------
;*************************************************
;* Get an event or device status message         *
;*                                               *
;* Input parameters are passed through global    *
;* variables:                                    *
;* ChannelNumber - channel number;               *
;* DiskNumber - disk number on channel           *
;*************************************************
GetEvent_StatusNotification:
        pusha
        mov     [CDDataBuf_pointer], CDDataBuf
; Clear the packet command buffer
        call    clear_packet_buffer
; Set command code
        mov     [PacketCommand], byte 4Ah
        mov     [PacketCommand+1], byte 00000001b
; Set message class request
        mov     [PacketCommand+4], byte 00010000b
; Size of allocated area
        mov     [PacketCommand+7], byte 8h
        mov     [PacketCommand+8], byte 0h
; Send command
        call    SendPacketDatCommand
        popa
        ret
;-----------------------------------------------------------------------------
;*************************************************
; Read information from TOC (Table of contents)  *
;* Input parameters are passed through global    *
;* variables:                                    *
;* ChannelNumber - channel number;               *
;* DiskNumber - disk number on channel           *
;*************************************************
Read_TOC:
        pusha
        mov     [CDDataBuf_pointer], CDDataBuf
; Clear the packet command buffer
        call    clear_packet_buffer
; Generate a packet command to read a data sector
        mov     [PacketCommand], byte 0x43
        ; Set format
        mov     [PacketCommand+2], byte 1
; Size of allocated area
        mov     [PacketCommand+7], byte 0xFF
        mov     [PacketCommand+8], byte 0h
; Send a command
        call    SendPacketDatCommand
        popa
        ret
;-----------------------------------------------------------------------------
;*****************************************************
;* DETERMINE THE TOTAL NUMBER OF SECTORS ON THE DISK *
;* Input parameters are passed through global        *
;* variables:                                        *
;* ChannelNumber - channel number;                   *
;* DiskNumber - disk number on channel               *
;*****************************************************
;ReadCapacity:
;       pusha
;; Clear the packet command buffer
;       call  clear_packet_buffer
;; Set the buffer size in bytes
;       mov     [CDBlockSize],8
;; Generate READ CAPACITY command
;       mov     [PacketCommand],word 25h
;; Send command
;       call    SendPacketDatCommand
;       popa
;       ret
;-----------------------------------------------------------------------------
clear_packet_buffer:
; Clear the packet command buffer
        and     [PacketCommand], dword 0
        and     [PacketCommand+4], dword 0
        and     [PacketCommand+8], dword 0
        ret
;-----------------------------------------------------------------------------
