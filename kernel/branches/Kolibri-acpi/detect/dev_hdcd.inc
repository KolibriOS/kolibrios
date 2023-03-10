;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; HDD and CD search

        cmp     [ecx+IDE_DATA.ProgrammingInterface], 0
        je      EndFindHDD
FindHDD:
        push    ecx
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

        pop     ecx
        jmp     EndFindHDD
;-----------------------------------------------------------------------------
FindHDD_2:
        add     [DeviceNumber], sizeof.HD_DATA
        shl     byte [ebx+DRIVE_DATA], 2
FindHDD_1:
        DEBUGF  1, "K : Channel %d ",[ChannelNumber]:2
        DEBUGF  1, "Disk %d\n",[DiskNumber]:1
        push    ebx ecx
        call    ReadHDD_ID
        pop     ecx ebx
        cmp     [DevErrorCode], 7
        je      .end
        cmp     [DevErrorCode], 0
        jne     .FindCD

        cmp     [Sector512+6], word 16
        ja      .FindCD

        cmp     [Sector512+12], word 255
        ja      .FindCD

        inc     byte [ebx+DRIVE_DATA]
        movzx   eax, [DeviceNumber]
        bt      word [Sector512+166], 10
        adc     [eax+hd0_data.hd48], 0
        jmp     .Print_Device_Name
;--------------------------------------
.FindCD:
        push    ebx ecx
        call    DeviceReset
        pop     ecx ebx
        cmp     [DevErrorCode], 0
        jne     .end

        push    ebx ecx
        call    ReadCD_ID
        pop     ecx ebx
        cmp     [DevErrorCode], 0
        jne     .end

        add     [ebx+DRIVE_DATA], byte 2
;--------------------------------------
.Print_Device_Name: 
        pushad
        pushfd

        xor     ebx, ebx
        mov     bx, [ChannelNumber]
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

        popfd
        popad
        ret
;--------------------------------------
.end:
        DEBUGF  1, "K : Device not found\n"
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
; ?????????? ???????????????????????? ?????????????? ?? ???????????? LBA
uglobal
SectorAddress   dd ?
dev_name:
        rb 41
endg
;-----------------------------------------------------------------------------
;*************************************************
;*     ???????????? ???????????????????????????? ???????????????? ??????????      *
;* ?????????????? ?????????????????? ???????????????????? ?????????? ???????????????????? *
;* ????????????????????:                                   *
;* ChannelNumber - ?????????? ???????????? (1 ?????? 2);       *
;* DiskNumber - ?????????? ?????????? ???? ???????????? (0 ?????? 1). *
;* ?????????????????????????????????? ???????? ???????????? ??????????????????????     *
;* ?? ???????????? Sector512.                           *
;*************************************************
ReadHDD_ID:
; ???????????? ?????????? CHS
        mov     [ATAAddressMode], 0
; ?????????????? ?????????????? ?????????????????????????? ????????????????????
        mov     [ATAFeatures], 0
        mov     [ATAHead], 0
        mov     [ATACommand], 0xEC
        call    SendCommandToHDD
        cmp     [DevErrorCode], 0 ;?????????????????? ?????? ????????????
        jne     @@End  ;??????????????????, ???????????????? ?????? ????????????

        mov     dx, [ATABasePortAddr]
        add     dx, 7    ;?????????? ???????????????? ????????????????
        mov     ecx, 0xffff
@@WaitCompleet:
        ; ?????????????????? ?????????? ???????????????????? ??????????????
        dec     ecx
        jz      @@Error1  ;???????????? ????????-????????
        ; ?????????????????? ????????????????????
        in      al, dx
        test    al, 80h  ;?????????????????? ?????????????? BSY
        jnz     @@WaitCompleet

        test    al, 1    ;?????????????????? ?????????????? ERR
        jnz     @@Error6

        test    al, 08h  ;?????????????????? ?????????????? DRQ
        jz      @@WaitCompleet
; ?????????????? ???????? ???????????? ???? ??????????????????????
        mov     edi, Sector512
        mov     dx, [ATABasePortAddr];?????????????? ????????????
        mov     cx, 256  ;?????????? ?????????????????????? ????????
        rep insw         ;?????????????? ???????? ????????????
        ret
; ???????????????? ?????? ????????????
@@Error1:
        mov     [DevErrorCode], 1
        ret
@@Error6:
        mov     [DevErrorCode], 6
@@End:
        ret
;-----------------------------------------------------------------------------
uglobal
; ?????????????????????? ?????????????? ???????????? ?????????????? 1 ?? 2
StandardATABases dw ?, ? ; 1F0h, 170h
; ?????????? ????????????
ChannelNumber   dw ?
; ?????????? ??????????
DiskNumber      db ?
DeviceNumber    db ?
; ?????????????? ?????????? ???????????? ???????????? ?????????????????????? ATA
ATABasePortAddr dw ?
; ?????????????????? ATA-??????????????
ATAFeatures     db ? ;??????????????????????
ATASectorCount  db ? ;???????????????????? ???????????????????????????? ????????????????
ATASectorNumber db ? ;?????????? ???????????????????? ??????????????
ATACylinder     dw ? ;?????????? ???????????????????? ????????????????
ATAHead         db ? ;?????????? ?????????????????? ??????????????
ATAAddressMode  db ? ;?????????? ?????????????????? (0 - CHS, 1 - LBA)
ATACommand      db ? ;?????? ??????????????, ???????????????????? ????????????????????
; ?????? ???????????? (0 - ?????? ????????????, 1 - ???????????????? ????????????????????
; ???????????????? ????????????????, 2 - ???????????????? ?????? ???????????? ??????????????????,
; 3 - ???????????????? ?????????? ????????????, 4 - ???????????????? ?????????? ??????????,
; 5 - ???????????????? ?????????? ??????????????, 6 - ???????????? ?????? ????????????????????
; ??????????????, 7 - ?????????????? ?????? ???????????? ????????????)
DevErrorCode dd ?
endg
;-----------------------------------------------------------------------------
;****************************************************
;*          ?????????????? ?????????????? ?????????????????? ??????????         *
;* ?????????????? ?????????????????? ???????????????????? ?????????? ????????????????????    *
;* ????????????????????:                                      *
;* ChannelNumber - ?????????? ???????????? (1 ?????? 2);          *
;* DiskNumber - ?????????? ?????????? (0 ?????? 1);              *
;* ATAFeatures - "??????????????????????";                     *
;* ATASectorCount - ???????????????????? ????????????????;            *
;* ATASectorNumber - ?????????? ???????????????????? ??????????????;      *
;* ATACylinder - ?????????? ???????????????????? ????????????????;         *
;* ATAHead - ?????????? ?????????????????? ??????????????;               *
;* ATAAddressMode - ?????????? ?????????????????? (0-CHS, 1-LBA); *
;* ATACommand - ?????? ??????????????.                        *
;* ?????????? ?????????????????? ???????????????????? ??????????????:              *
;* ?? ATABasePortAddr - ?????????????? ?????????? HDD;           *
;* ?? DevErrorCode - ????????.                           *
;* ?????? ?????????????????????????? ???????????? ?? DevErrorCode ??????????    *
;* ?????????????????? ?????? ????????????.                            *
;****************************************************
SendCommandToHDD:
; ?????????????????? ???????????????? ???????? ????????????
        cmp     [ATAAddressMode], 1
        ja      @@Err2
; ?????????????????? ???????????????????????? ???????????? ????????????
        mov     bx, [ChannelNumber]
        cmp     bx, 1
        jb      @@Err3

        cmp     bx, 2
        ja      @@Err3
; ???????????????????? ?????????????? ??????????
        dec     bx
        shl     bx, 1
        movzx   ebx, bx
        mov     ax, [ebx+StandardATABases]
        mov     [ATABasePortAddr], ax
; ???????????????? ???????????????????? HDD ?? ???????????? ??????????????
        ; ?????????????? ???????????? ????????
        mov     dx, [ATABasePortAddr]
        add     dx, 6   ;?????????? ???????????????? ??????????????
        mov     al, [DiskNumber]
        cmp     al, 1   ;?????????????????? ???????????? ??????????
        ja      @@Err4

        shl     al, 4
        or      al, 10100000b
        out     dx, al
        ; ??????????????, ???????? ???????? ???? ?????????? ??????????
        inc     dx
        mov     ecx, 0xfff
@@WaitHDReady:
        ; ?????????????????? ?????????? ??????????????
        dec     ecx
        jz      @@Err1
        ; ?????????????????? ?????????????? ????????????????
        in      al, dx
        ; ?????????????????? ?????????????????? ?????????????? BSY
        test    al, 80h
        jnz     @@WaitHDReady
        ; ?????????????????? ?????????????????? ?????????????? DRQ
        test    al, 08h
        jnz     @@WaitHDReady
; ?????????????????? ?????????????? ?? ???????????????? ??????????????????????
        cli
        mov     dx, [ATABasePortAddr]
        inc     dx      ;?????????????? "????????????????????????"
        mov     al, [ATAFeatures]
        out     dx, AL
        inc     dx      ;?????????????? ????????????????
        mov     al, [ATASectorCount]
        out     dx, AL
        inc     dx      ;?????????????? ???????????? ??????????????
        mov     al, [ATASectorNumber]
        out     dx, AL
        inc     dx      ;?????????? ???????????????? (?????????????? ????????)
        mov     ax, [ATACylinder]
        out     dx, AL
        inc     dx      ;?????????? ???????????????? (?????????????? ????????)
        mov     al, AH
        out     dx, AL
        inc     dx      ;?????????? ??????????????/?????????? ??????????
        mov     al, [DiskNumber]
        shl     al, 4
        cmp     [ATAHead], 0xF ;?????????????????? ?????????? ??????????????
        ja      @@Err5

        or      al, [ATAHead]
        or      al, 10100000b
        mov     ah, [ATAAddressMode]
        shl     ah, 6
        or      al, ah
        out     dx, al
; ?????????????? ??????????????
        mov     al, [ATACommand]
        inc     dx      ;?????????????? ????????????
        out     dx, al
        sti
; ???????????????? ?????????????? ????????????
        mov     [DevErrorCode], 0
        ret
; ???????????????? ?????? ????????????
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
; ???????????????????? ???????????? ??????????????????
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*     ???????????? ???????????????????????????? ???????????????????? ATAPI    *
;* ?????????????? ?????????????????? ???????????????????? ?????????? ???????????????????? *
;* ??????????????????:                                    *
;* ChannelNumber - ?????????? ????????????;                 *
;* DiskNumber - ?????????? ?????????? ???? ????????????.           *
;* ?????????????????????????????????? ???????? ???????????? ??????????????????????     *
;* ?? ???????????? Sector512.                           *
;*************************************************
ReadCD_ID:
; ???????????? ?????????? CHS
        mov     [ATAAddressMode], 0
; ?????????????? ?????????????? ?????????????????????????? ????????????????????
        mov     [ATAFeatures], 0
        mov     [ATASectorCount], 0
        mov     [ATASectorNumber], 0
        mov     [ATACylinder], 0
        mov     [ATAHead], 0
        mov     [ATACommand], 0xA1
        call    SendCommandToHDD
        cmp     [DevErrorCode], 0;?????????????????? ?????? ????????????
        jne     @@End_1  ;??????????????????, ???????????????? ?????? ????????????
; ?????????????? ???????????????????? ???????????? HDD
        mov     dx, [ATABasePortAddr]
        add     dx, 7  ;???????? 1??7h
        mov     ecx, 0xffff
@@WaitCompleet_1:
        ; ?????????????????? ????????
        dec     ecx
        jz      @@Error1_1 ;???????????? ????????-????????
        ; ?????????????????? ????????????????????
        in      al, dx
        test    al, 80h  ;?????????????????? ?????????????? BSY
        jnz     @@WaitCompleet_1

        test    al, 1    ;?????????????????? ?????????????? ERR
        jnz     @@Error6_1

        test    al, 08h  ;?????????????????? ?????????????? DRQ
        jz      @@WaitCompleet_1
; ?????????????? ???????? ???????????? ???? ??????????????????????
        mov     edi, Sector512 ;offset Sector512
        mov     dx, [ATABasePortAddr];???????? 1x0h
        mov     cx, 256;?????????? ?????????????????????? ????????
        rep insw
        ret
; ???????????????? ?????? ????????????
@@Error1_1:
        mov     [DevErrorCode], 1
        ret
@@Error6_1:
        mov     [DevErrorCode], 6
@@End_1:
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*                ?????????? ????????????????????               *
;* ?????????????? ?????????????????? ???????????????????? ?????????? ???????????????????? *
;* ????????????????????:                                   *
;* ChannelNumber - ?????????? ???????????? (1 ?????? 2);       *
;* DiskNumber - ?????????? ?????????? (0 ?????? 1).           *
;*************************************************
DeviceReset:
; ?????????????????? ???????????????????????? ???????????? ????????????
        mov     bx, [ChannelNumber]
        cmp     bx, 1
        jb      @@Err3_2

        cmp     bx, 2
        ja      @@Err3_2
; ???????????????????? ?????????????? ??????????
        dec     bx
        shl     bx, 1
        movzx   ebx, bx
        mov     dx, [ebx+StandardATABases]
        mov     [ATABasePortAddr], dx
; ?????????????? ???????????? ????????
        add     dx, 6   ;?????????? ???????????????? ??????????????
        mov     al, [DiskNumber]
        cmp     al, 1   ;?????????????????? ???????????? ??????????
        ja      @@Err4_2

        shl     al, 4
        or      al, 10100000b
        out     dx, al
; ?????????????? ?????????????? "??????????"
        mov     al, 0x8
        inc     dx      ;?????????????? ????????????
        out     dx, al
        mov     ecx, 0x80000
@@WaitHDReady_1:
        ; ?????????????????? ?????????? ??????????????
        dec     ecx
        je      @@Err1_2 ;???????????? ????????-????????
        ; ?????????????????? ?????????????? ????????????????
        in      al, dx
        ; ?????????????????? ?????????????????? ?????????????? BSY
        test    al, 80h
        jnz     @@WaitHDReady_1
; ???????????????? ?????????????? ????????????
        mov     [DevErrorCode], 0
        ret
; ?????????????????? ????????????
@@Err1_2:
        mov     [DevErrorCode], 1
        ret
@@Err3_2:
        mov     [DevErrorCode], 3
        ret
@@Err4_2:
        mov     [DevErrorCode], 4
; ???????????????? ?????? ????????????
        ret
;-----------------------------------------------------------------------------
EndFindHDD:
