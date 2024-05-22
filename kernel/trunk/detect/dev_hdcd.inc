;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; HDD and CD search

        cmp     [ecx+IDE_DATA.ProgrammingInterface], 0
        je      EndFindHDD
FindHDD:
        xor     ebx, ebx
        inc     ebx
        mov     [DeviceNumber], 0
        cmp     ecx, IDE_controller_1
        jz      .find
        add     bl, 5
        add     [DeviceNumber], sizeof.HD_DATA*4
        cmp     ecx, IDE_controller_2
        jz      .find
        add     bl, 5
        add     [DeviceNumber], sizeof.HD_DATA*4
.find:
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 0
        call    FindHDD_1
        inc     [DiskNumber]
        call    FindHDD_2
        inc     [ChannelNumber]
        dec     [DiskNumber]
        call    FindHDD_2
        inc     [DiskNumber]
        call    FindHDD_2
        jmp     EndFindHDD
;-----------------------------------------------------------------------------
FindHDD_2:
        add     [DeviceNumber], sizeof.HD_DATA
        shl     byte [ebx+DRIVE_DATA], 2
FindHDD_1:
        DEBUGF  1, "K : Channel %d ",[ChannelNumber]:1
        DEBUGF  1, "Disk %d\n",[DiskNumber]:1
        push    ecx ebx
        call    ReadHDD_ID
        cmp     [DevErrorCode], 7
        je      .end
        cmp     [DevErrorCode], 0
        jne     .FindCD
        cmp     [Sector512+6], word 16
        ja      .FindCD
        cmp     [Sector512+12], word 255
        ja      .FindCD
        pop     ebx
        movzx   eax, [DeviceNumber]
        mov     ecx, [Sector512+120]
        mov     dword[eax+hd0_data.sectors], ecx
        and     dword[eax+hd0_data.sectors+4], 0
        bt      word [Sector512+166], 10
        jnc     .Print_Device_Name
        mov     [eax+hd0_data.hd48], 1
        mov     ecx, [Sector512+200]
        mov     dword[eax+hd0_data.sectors], ecx
        mov     ecx, [Sector512+204]
        mov     dword[eax+hd0_data.sectors+4], ecx
        jmp     .Print_Device_Name
;--------------------------------------
.FindCD:
        call    DeviceReset
        cmp     [DevErrorCode], 0
        jne     .end
        call    ReadCD_ID
        cmp     [DevErrorCode], 0
        jne     .end
        pop     ebx
        inc     byte [ebx+DRIVE_DATA]
;--------------------------------------
.Print_Device_Name:
        inc     byte [ebx+DRIVE_DATA]
        pop     ecx
        pushad
        movzx   ebx, [ChannelNumber]
        dec     ebx
        shl     ebx, 1
        add     bl, [DiskNumber]
        shl     ebx, 1
        call    calculate_IDE_device_values_storage
;--------------------------------------
.copy_dev_name:
        mov     esi, Sector512+27*2
        mov     edi, dev_name
        mov     ecx, 20
        cld
;--------------------------------------
@@:
        lodsw
        xchg    ah, al
        stosw
        loop    @b
        DEBUGF 1, "K : Dev: %s \n", dev_name
        xor     eax, eax
        mov     ax, [Sector512+64*2]
        DEBUGF  1, "K : PIO possible modes %x\n", al
        mov     ax, [Sector512+51*2]
        mov     al, ah
        call    convert_Sector512_value
        DEBUGF  1, "K : PIO set mode %x\n", ah
        mov     ax, [Sector512+63*2]
        DEBUGF  1, "K : Multiword DMA possible modes %x\n", al
        mov     al, ah
        call    convert_Sector512_value
        DEBUGF  1, "K : Multiword DMA set mode %x\n", ah
        mov     ax, [Sector512+88*2]
        DEBUGF  1, "K : Ultra DMA possible modes %x\n", al
        mov     [ebx+IDE_DEVICE.UDMA_possible_modes], al
        mov     al, ah
        call    convert_Sector512_value
        DEBUGF  1, "K : Ultra DMA set mode %x\n", ah
        mov     [ebx+IDE_DEVICE.UDMA_set_mode], ah
        popad
        ret

.end:
        DEBUGF  1, "K : Device not found\n"
        pop     ebx ecx
        ret
;-----------------------------------------------------------------------------
calculate_IDE_device_values_storage:
        cmp     ecx, IDE_controller_1
        jne     @f

        add     ebx, IDE_device_1
        jmp     .exit
;--------------------------------------
@@:
        cmp     ecx, IDE_controller_2
        jne     @f

        add     ebx, IDE_device_2
        jmp     .exit
;--------------------------------------
@@:
        add     ebx, IDE_device_3
;--------------------------------------
.exit:
        ret
;-----------------------------------------------------------------------------
convert_Sector512_value:
        mov     ecx, 8
        xor     ah, ah
;--------------------------------------
@@:
        test    al, 1b
        jnz     .end

        shr     al, 1
        inc     ah
        loop    @b

        xor     ah, ah
;--------------------------------------
.end:
        ret
;-----------------------------------------------------------------------------
; Address of reading sector in LBA mode
uglobal
SectorAddress   dd ?
dev_name:
        rb 41
endg
;-----------------------------------------------------------------------------
;*************************************************
;*     READING THE HARD DISK IDENTIFIER          *
;* Input parameters are passed through the global*
;* variables:                                    *
;* ChannelNumber - channel number (1 or 2);      *
;* DiskNumber - disk number on channel (0 or 1)  *
;* Block of identificational data is reading     *
;* to Sector512 array.                           *
;*************************************************
ReadHDD_ID:
; set up CHS mode
        mov     [ATAAddressMode], 0
; send device identification command
        mov     [ATAFeatures], 0
        mov     [ATAHead], 0
        mov     [ATACommand], 0xEC
        call    SendCommandToHDD
        cmp     [DevErrorCode], 0 ; check the error code
        jne     @@End    ; finish, saving the error code

        mov     dx, [ATABasePortAddr]
        add     dx, 7    ; address of state register
        mov     ecx, 0xffff
@@WaitCompleet:
        ; Check command execution time
        dec     ecx
        jz      @@Error1 ; timeout error
        ; Check if ready or not
        in      al, dx
        test    al, 80h  ; BSY signal state
        jnz     @@WaitCompleet

        test    al, 1    ; ERR signal state
        jnz     @@Error6

        test    al, 08h  ; DRQ signal state
        jz      @@WaitCompleet
; Receive data block from controller
        mov     edi, Sector512
        mov     dx, [ATABasePortAddr]; data register
        mov     cx, 256  ; number of word to receive
        rep insw         ; receive data block
        ret
; write the error code
@@Error1:
        mov     [DevErrorCode], 1
        ret
@@Error6:
        mov     [DevErrorCode], 6
@@End:
        ret
;-----------------------------------------------------------------------------
uglobal
; Standart base addresses of channels 1 or 2
StandardATABases dw ?, ? ; 1F0h, 170h
; Channel number
ChannelNumber   db ?
; Disk number
DiskNumber      db ?
DeviceNumber    db ?
; Base address of ATA controller's port group
ATABasePortAddr dw ?
; ATA-command parameters
ATAFeatures     db ? ; features
ATASectorCount  db ? ; count of processing sectors
ATASectorNumber db ? ; initial sector number
ATACylinder     dw ? ; initial cylinder number
ATAHead         db ? ; initial head number
ATAAddressMode  db ? ; addressing mode (0 - CHS, 1 - LBA)
ATACommand      db ? ; executing command number
; Error code (0 - no errors, 1 - waiting time limit exceed
; 2 - incorrect code of addressing mode,
; 3 - incorrect channel number, 4 - incorrect disk number,
; 5 - incorrect head number, 6 - command execution error,
; 7 - time out when choosing channel)
DevErrorCode dd ?
endg
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
SendCommandToHDD:
; Check the addressing mode code
        cmp     [ATAAddressMode], 1
        ja      @@Err2
; Check the channel number correctness
        movzx   ebx, [ChannelNumber]
        dec     ebx
        cmp     ebx, 1
        ja      @@Err3
; Set the base address
        shl     ebx, 1
        mov     ax, [ebx+StandardATABases]
        mov     [ATABasePortAddr], ax
; Waiting for HDD ready to receive a command
        ; Choose desired disk
        mov     dx, [ATABasePortAddr]
        add     dx, 6   ; address of the heads register
        mov     al, [DiskNumber]
        cmp     al, 1   ; check the disk number
        ja      @@Err4

        shl     al, 4
        or      al, 10100000b
        out     dx, al
        ; Waiting for disk ready
        inc     dx
        mov     ecx, 0xfff
@@WaitHDReady:
        ; Check waiting time
        dec     ecx
        jz      @@Err1
        ; Read the state register
        in      al, dx
        ; Check the state of BSY signal
        test    al, 80h
        jnz     @@WaitHDReady
        ; Check the state of DRQ signal
        test    al, 08h
        jnz     @@WaitHDReady
; load command to controller's registers
        cli
        mov     dx, [ATABasePortAddr]
        inc     dx      ; "features" register
        mov     al, [ATAFeatures]
        out     dx, AL
        inc     dx      ; sector counter
        mov     al, [ATASectorCount]
        out     dx, AL
        inc     dx      ; sector number register
        mov     al, [ATASectorNumber]
        out     dx, AL
        inc     dx      ; cylinder number (low byte)
        mov     ax, [ATACylinder]
        out     dx, AL
        inc     dx      ; cylinder number (high byte)
        mov     al, AH
        out     dx, AL
        inc     dx      ; head number / disk number
        mov     al, [DiskNumber]
        shl     al, 4
        cmp     [ATAHead], 0xF ; check head number
        ja      @@Err5

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
; reset the error sign
        mov     [DevErrorCode], 0
        ret
; write error code
@@Err1:
        mov     [DevErrorCode], 7
        ret
@@Err2:
        mov     [DevErrorCode], 2
        ret
@@Err3:
        mov     [DevErrorCode], 3
        ret
@@Err4:
        mov     [DevErrorCode], 4
        ret
@@Err5:
        mov     [DevErrorCode], 5
; finish work
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*     READ ATAPI DEVICE IDENTIFIER              *
;* Input parameters are passed through the global*
;* variables:                                    *
;* ChannelNumber - channel number;               *
;* DiskNumber - disk number on channel.          *
;* Block of identificational data is reading     *
;* to Sector512 array.                           *                           *
;*************************************************
ReadCD_ID:
; Set CHS mode
        mov     [ATAAddressMode], 0
; Send command for device identification
        mov     [ATAFeatures], 0
        mov     [ATASectorCount], 0
        mov     [ATASectorNumber], 0
        mov     [ATACylinder], 0
        mov     [ATAHead], 0
        mov     [ATACommand], 0xA1
        call    SendCommandToHDD
        cmp     [DevErrorCode], 0 ; check the error code
        jne     @@End_1  ; finish, saving the error code
; Wait for HDD data ready
        mov     dx, [ATABasePortAddr]
        add     dx, 7      ; port 1х7h
        mov     ecx, 0xffff
@@WaitCompleet_1:
        ; Check time
        dec     ecx
        jz      @@Error1_1 ; time out error
        ; Check readyness
        in      al, dx
        test    al, 80h    ; BSY signal state
        jnz     @@WaitCompleet_1

        test    al, 1      ; ERR signal state
        jnz     @@Error6_1

        test    al, 08h    ; DRQ signal state
        jz      @@WaitCompleet_1
; Receive data block from controller
        mov     edi, Sector512 ; offset Sector512
        mov     dx, [ATABasePortAddr] ; port 1x0h
        mov     cx, 256    ; words read count
        rep insw
        ret
; write the error code
@@Error1_1:
        mov     [DevErrorCode], 1
        ret
@@Error6_1:
        mov     [DevErrorCode], 6
@@End_1:
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*                DEVICE RESET                   *
;* Input parameters are passed through the global*
;* variables:                                    *
;* ChannelNumber - channel number (1 or 2);      *
;* DiskNumber - disk number (0 or 1).            *
;*************************************************
DeviceReset:
; Check the channel number correctness
        movzx   ebx, [ChannelNumber]
        dec     ebx
        cmp     ebx, 1
        ja      @@Err3_2
; Set base address
        shl     ebx, 1
        mov     dx, [ebx+StandardATABases]
        mov     [ATABasePortAddr], dx
; Choose desired disk
        add     dx, 6   ; address of heads register
        mov     al, [DiskNumber]
        cmp     al, 1   ; check disk number
        ja      @@Err4_2

        shl     al, 4
        or      al, 10100000b
        out     dx, al
; Send the "Reset" command
        mov     al, 0x8
        inc     dx      ; command register
        out     dx, al
        mov     ecx, 0x80000
@@WaitHDReady_1:
        ; Check waiting time
        dec     ecx
        je      @@Err1_2 ; time out error
        ; read the state register
        in      al, dx
        ; Check the state of BSY signal
        test    al, 80h
        jnz     @@WaitHDReady_1
; reset the error sign
        mov     [DevErrorCode], 0
        ret
; error processing
@@Err1_2:
        mov     [DevErrorCode], 1
        ret
@@Err3_2:
        mov     [DevErrorCode], 3
        ret
@@Err4_2:
        mov     [DevErrorCode], 4
; write error code
        ret
;-----------------------------------------------------------------------------
EndFindHDD:
