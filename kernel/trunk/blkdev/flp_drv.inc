;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;**********************************************************
;  Direct work with floppy disk drive
;**********************************************************
; Source code author -  Kulakov Vladimir Gennadievich.
; Adaptation and improvement -  Mario79.

give_back_application_data_1:
        mov     esi, FDD_BUFF;FDD_DataBuffer  ;0x40000
        mov     ecx, 128
        cld
        rep movsd
        ret

take_data_from_application_1:
        mov     edi, FDD_BUFF;FDD_DataBuffer  ;0x40000
        mov     ecx, 128
        cld
        rep movsd
        ret

; Controller operations result codes (FDC_Status)
FDC_Normal         = 0 ; normal finish
FDC_TimeOut        = 1 ; time out error
FDC_DiskNotFound   = 2 ; no disk in drive
FDC_TrackNotFound  = 3 ; track not found
FDC_SectorNotFound = 4 ; sector not found

; Maximum values of the sector coordinates (specified
; values correspond to the parameters of the standard
; 3-inch 1.44 MB floppy disk)
MAX_Track   = 79
MAX_Head    =  1
MAX_Sector  = 18

uglobal
; Timer tick counter
TickCounter dd ?
; Operation completion code with the floppy disk drive controller
FDC_Status  DB ?
; Interrupt flag from floppy disk drive
FDD_IntFlag DB ?
; The moment of the beginning of the last operation with FDD
FDD_Time    DD ?
; Drive number
FDD_Type    db 0
; Sector coordinates
FDD_Track   DB ?
FDD_Head    DB ?
FDD_Sector  DB ?

; Operation result block
FDC_ST0 DB ?
FDC_ST1 DB ?
FDC_ST2 DB ?
FDC_C   DB ?
FDC_H   DB ?
FDC_R   DB ?
FDC_N   DB ?
; Read operation repetition counter
ReadRepCounter  DB ?
; Recalibration operation repetition counter
RecalRepCounter DB ?
endg
; Memory area for storing the readed sector
;FDD_DataBuffer:  times 512 db 0   ;DB 512 DUP (?)
fdd_motor_status db 0
timer_fdd_motor  dd 0

;**************************************
;* INITIALIZATION OF DMA MODE FOR FDD *
;**************************************
Init_FDC_DMA:
        pushad
        mov     al, 0
        out     0x0c, al; reset the flip-flop to a known state.
        mov     al, 6           ; mask channel 2 so we can reprogram it.
        out     0x0a, al
        mov     al, [dmamode]; 0x46 -> Read from floppy - 0x4A Write to floppy
        out     0x0b, al
        mov     al, 0
        out     0x0c, al; reset the flip-flop to a known state.
        mov     eax, 0xD000
        out     0x04, al; set the channel 2 starting address to 0
        shr     eax, 8
        out     0x04, al
        shr     eax, 8
        out     0x81, al
        mov     al, 0
        out     0x0c, al; reset flip-flop
        mov     al, 0xff;set count (actual size -1)
        out     0x5, al
        mov     al, 0x1;[dmasize]       ;(0x1ff = 511 / 0x23ff =9215)
        out     0x5, al
        mov     al, 2
        out     0xa, al
        popad
        ret

;***********************************
;* WRITE BYTE TO FDC DATA PORT     *
;* Parameters:                     *
;* AL - byte to write.             *
;***********************************
FDCDataOutput:
;       DEBUGF 1,'K : FDCDataOutput(%x)',al
;        pusha
        push    eax ecx edx
        mov     AH, AL    ; remember byte to AH
; Reset controller state variable
        mov     [FDC_Status], FDC_Normal
; Check the readiness of the controller to receive data
        mov     DX, 3F4h  ; (FDC state port)
        mov     ecx, 0x10000 ; set timeout counter
@@TestRS:
        in      AL, DX    ; read the RS register
        and     AL, 0C0h  ; get digits 6 and 7
        cmp     AL, 80h   ; check digits 6 and 7
        je      @@OutByteToFDC
        loop    @@TestRS
; Time out error
;       DEBUGF 1,' timeout\n'
        mov     [FDC_Status], FDC_TimeOut
        jmp     @@End_5
; Write byte to data port
@@OutByteToFDC:
        inc     DX
        mov     AL, AH
        out     DX, AL
;        DEBUGF 1,' ok\n'
@@End_5:
;        popa
        pop     edx ecx eax
        ret

;******************************************
;*   READ BYTE FROM FDC DATA PORT         *
;* Procedure doesnt have input params.    *
;* Output :                               *
;* AL - byte read.                        *
;******************************************
FDCDataInput:
        push    ECX
        push    DX
; Reset controller state variable
        mov     [FDC_Status], FDC_Normal
; Check the readiness of the controller to receive data
        mov     DX, 3F4h  ;(FDC state port)
        mov     ecx, 0x10000 ; set timeout counter
@@TestRS_1:
        in      AL, DX    ; read the RS register
        and     AL, 0C0h  ; get digits 6 and 7
        cmp     AL, 0C0h  ; check digits 6 and 7
        je      @@GetByteFromFDC
        loop    @@TestRS_1
; Time out error
;       DEBUGF 1,'K : FDCDataInput: timeout\n'
        mov     [FDC_Status], FDC_TimeOut
        jmp     @@End_6
; Get byte from data port
@@GetByteFromFDC:
        inc     DX
        in      AL, DX
;       DEBUGF 1,'K : FDCDataInput: %x\n',al
@@End_6:
        pop     DX
        pop     ECX
        ret

;*********************************************
;* FDC INTERRUPT HANDLER                     *
;*********************************************
FDCInterrupt:
;       dbgstr 'FDCInterrupt'
; Set the interrupt flag
        mov     [FDD_IntFlag], 1
        mov     al, 1
        ret

;*******************************************
;* WAIT FOR INTERRUPT FROM FDC             *
;*******************************************
WaitFDCInterrupt:
        pusha
; Reset operation status byte
        mov     [FDC_Status], FDC_Normal
; Zero out the tick counter
        mov     eax, [timer_ticks]
        mov     [TickCounter], eax
; Wait for the floppy disk interrupt flag to be set
@@TestRS_2:
        call    change_task
        cmp     [FDD_IntFlag], 0
        jnz     @@End_7           ; interrupt occured
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter]
        cmp     eax, 200;50 ;25   ;5 ; wait 5 ticks
        jb      @@TestRS_2
;        jl      @@TestRS_2
; Time out error
;       dbgstr 'WaitFDCInterrupt: timeout'
        mov     [FDC_Status], FDC_TimeOut
@@End_7:
        popa
        ret

;***********************************
;* Turn on the motor of drive "A:" *
;***********************************
FDDMotorON:
;       dbgstr 'FDDMotorON'
        pusha
;        cmp     [fdd_motor_status],1
;        je      fdd_motor_on
        mov     al, [flp_number]
        cmp     [fdd_motor_status], al
        je      fdd_motor_on
; Reset the FDD controller
        mov     DX, 3F2h  ; motor control port
        mov     AL, 0
        out     DX, AL
; Select and turn on the drive motor
        cmp     [flp_number], 1
        jne     FDDMotorON_B
;        call    FDDMotorOFF_B
        mov     AL, 1Ch   ; Floppy A
        jmp     FDDMotorON_1
FDDMotorON_B:
;        call    FDDMotorOFF_A
        mov     AL, 2Dh   ; Floppy B
FDDMotorON_1:
        out     DX, AL
; Zero out the tick counter
        mov     eax, [timer_ticks]
        mov     [TickCounter], eax
; wait 0.5 s
@@dT:
        call    change_task
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter]
        cmp     eax, 50 ;10
        jb      @@dT
; Read results of RESET command
        push    4
;       DEBUGF 1,'K : floppy reset results:'
@@:
        mov     al, 8
        call    FDCDataOutput
        call    FDCDataInput
;       DEBUGF 1,' %x',al
        call    FDCDataInput
;       DEBUGF 1,' %x',al
        dec     dword [esp]
        jnz     @b
;       DEBUGF 1,'\n'
        pop     eax
        cmp     [flp_number], 1
        jne     fdd_motor_on_B
        mov     [fdd_motor_status], 1
        jmp     fdd_motor_on
fdd_motor_on_B:
        mov     [fdd_motor_status], 2
fdd_motor_on:
        call    save_timer_fdd_motor
        popa
        ret

;*****************************************
;*  SAVING TIME STAMP                    *
;*****************************************
save_timer_fdd_motor:
        mov     eax, [timer_ticks]
        mov     [timer_fdd_motor], eax
        ret

;*****************************************
;*  CHECK THE MOTOR SHUTDOWN DELAY       *
;*****************************************
proc check_fdd_motor_status_has_work?
        cmp     [fdd_motor_status], 0
        jz      .no
        mov     eax, [timer_ticks]
        sub     eax, [timer_fdd_motor]
        cmp     eax, 500
        jb      .no
.yes:
        xor     eax, eax
        inc     eax
        ret
.no:
        xor     eax, eax
        ret
endp

align 4
check_fdd_motor_status:
        cmp     [fdd_motor_status], 0
        je      end_check_fdd_motor_status_1
        mov     eax, [timer_ticks]
        sub     eax, [timer_fdd_motor]
        cmp     eax, 500
        jb      end_check_fdd_motor_status
        call    FDDMotorOFF
        mov     [fdd_motor_status], 0
end_check_fdd_motor_status_1:
end_check_fdd_motor_status:
        ret

;**********************************
;* TURN OFF MOTOR OF DRIVE        *
;**********************************
FDDMotorOFF:
;       dbgstr 'FDDMotorOFF'
        push    AX
        push    DX
        cmp     [flp_number], 1
        jne     FDDMotorOFF_1
        call    FDDMotorOFF_A
        jmp     FDDMotorOFF_2
FDDMotorOFF_1:
        call    FDDMotorOFF_B
FDDMotorOFF_2:
        pop     DX
        pop     AX
        ; clearing caching flags due to information obsolescence
        or      [floppy_media_flags+0], FLOPPY_MEDIA_NEED_RESCAN
        or      [floppy_media_flags+1], FLOPPY_MEDIA_NEED_RESCAN
        ret

FDDMotorOFF_A:
        mov     DX, 3F2h  ; motor control port
        mov     AL, 0Ch   ; Floppy A
        out     DX, AL
        ret

FDDMotorOFF_B:
        mov     DX, 3F2h  ; motor control port
        mov     AL, 5h    ; Floppy B
        out     DX, AL
        ret

;*******************************
;* RECALIBRATE DRIVE "A:"      *
;*******************************
RecalibrateFDD:
;       dbgstr 'RecalibrateFDD'
        pusha
        call    save_timer_fdd_motor
; Clear the interrupt flag
        mov     [FDD_IntFlag], 0
; Send the "Recalibration" command
        mov     AL, 07h
        call    FDCDataOutput
        mov     AL, [flp_number]
        dec     AL
        call    FDCDataOutput
; Wait for the operation to complete
        call    WaitFDCInterrupt
        cmp     [FDC_Status], 0
        jne     .fail
; Read results of RECALIBRATE command
;       DEBUGF 1,'K : floppy recalibrate results:'
        mov     al, 8
        call    FDCDataOutput
        call    FDCDataInput
        push    eax
;       DEBUGF 1,' %x',al
        call    FDCDataInput
;       DEBUGF 1,' %x',al
;       DEBUGF 1,'\n'
        pop     eax
        test    al, 0xC0
        jz      @f
        mov     [FDC_Status], FDC_DiskNotFound
@@:
.fail:
        call    save_timer_fdd_motor
        popa
        ret

;*****************************************************
;*                    TRACK SEARCH                   *
;* Parameters are passed through global variables:   *
;* FDD_Track - track number (0-79);                  *
;* FDD_Head - head number (0-1).                     *
;* Result of operation is written to FDC_Status.     *
;*****************************************************
SeekTrack:
;       dbgstr 'SeekTrack'
        pusha
        call    save_timer_fdd_motor
; Clear the interrupt flag
        mov     [FDD_IntFlag], 0
; Send "Search" command
        mov     AL, 0Fh
        call    FDCDataOutput
        ; Send head / drive number byte
        mov     AL, [FDD_Head]
        shl     AL, 2
        call    FDCDataOutput
        ; Send track number byte
        mov     AL, [FDD_Track]
        call    FDCDataOutput
; Wait for the operation to complete
        call    WaitFDCInterrupt
        cmp     [FDC_Status], FDC_Normal
        jne     @@Exit
; Save search result
        mov     AL, 08h
        call    FDCDataOutput
        call    FDCDataInput
        mov     [FDC_ST0], AL
        call    FDCDataInput
        mov     [FDC_C], AL
; Check search result
        ; Is search finished?
        test    [FDC_ST0], 100000b
        je      @@Err
        ; Is the specified track found?
        mov     AL, [FDC_C]
        cmp     AL, [FDD_Track]
        jne     @@Err
        ; Does the head number match the specified one?
; The H bit (Head Address) in ST0 will always return a "0" (c) 82077AA datasheet,
; description of SEEK command. So we can not verify the proper head.
;        mov     AL, [FDC_ST0]
;        and     AL, 100b
;        shr     AL, 2
;        cmp     AL, [FDD_Head]
;        jne     @@Err
        ; Operation completed successfully
;        dbgstr 'SeekTrack: FDC_Normal'
        mov     [FDC_Status], FDC_Normal
        jmp     @@Exit
@@Err:  ; Track not found
;       dbgstr 'SeekTrack: FDC_TrackNotFound'
        mov     [FDC_Status], FDC_TrackNotFound
@@Exit:
        call    save_timer_fdd_motor
        popa
        ret

;*******************************************************
;*               READING A DATA SECTOR                 *
;* Parameters are passed through global variables:     *
;* FDD_Track - track number (0-79);                    *
;* FDD_Head - head number (0-1);                       *
;* FDD_Sector - sector number (1-18).                  *
;* Result of operation is written to FDC_Status.       *
;* If the read operation is successful, the contents   *
;*  of the sector will be written to FDD_DataBuffer.   *
;*******************************************************
ReadSector:
;       dbgstr 'ReadSector'
        pushad
        call    save_timer_fdd_motor
; Clear the interrupt flag
        mov     [FDD_IntFlag], 0
; Set transmit speed to 500 Kb / s
        mov     AX, 0
        mov     DX, 03F7h
        out     DX, AL
; Initialize the DMA channel
        mov     [dmamode], 0x46
        call    Init_FDC_DMA
; Send "Data read" command
        mov     AL, 0E6h ; reading in multi-track mode
        call    FDCDataOutput
        mov     AL, [FDD_Head]
        shl     AL, 2
        or      AL, [flp_number]
        dec     AL
        call    FDCDataOutput
        mov     AL, [FDD_Track]
        call    FDCDataOutput
        mov     AL, [FDD_Head]
        call    FDCDataOutput
        mov     AL, [FDD_Sector]
        call    FDCDataOutput
        mov     AL, 2   ; sector size code (512 byte)
        call    FDCDataOutput
        mov     AL, 18 ;+1; 3Fh  ;number of sectors per track
        call    FDCDataOutput
        mov     AL, 1Bh ; GPL value
        call    FDCDataOutput
        mov     AL, 0FFh; DTL value
        call    FDCDataOutput
; Waiting for an interrupt at the end of the operation
        call    WaitFDCInterrupt
        cmp     [FDC_Status], FDC_Normal
        jne     @@Exit_1
; Read the operation completion status
        call    GetStatusInfo
        test    [FDC_ST0], 11011000b
        jnz     @@Err_1
;        dbgstr 'ReadSector: FDC_Normal'
        mov     [FDC_Status], FDC_Normal
        jmp     @@Exit_1
@@Err_1:
;       dbgstr 'ReadSector: FDC_SectorNotFound'
        mov     [FDC_Status], FDC_SectorNotFound
@@Exit_1:
        call    save_timer_fdd_motor
        popad
        ret

;*******************************************************
;*   READ SECTOR (WITH RETRY OF OPERATION ON FAILURE)  *
;* Parameters are passed through global variables:     *
;* FDD_Track - track number (0-79);                    *
;* FDD_Head - head number (0-1);                       *
;* FDD_Sector - sector number (1-18).                  *
;* Result of operation is written to FDC_Status.       *
;* If the read operation is successful, the contents   *
;*  of the sector will be written to FDD_DataBuffer.   *
;*******************************************************
ReadSectWithRetr:
        pusha
; Reset the recalibration repetition counter
        mov     [RecalRepCounter], 0
@@TryAgain:
; Reset the read operation retry counter
        mov     [ReadRepCounter], 0
@@ReadSector_1:
        call    ReadSector
        cmp     [FDC_Status], 0
        je      @@Exit_2
        cmp     [FDC_Status], 1
        je      @@Err_3
        ; Three times repeat reading
        inc     [ReadRepCounter]
        cmp     [ReadRepCounter], 3
        jb      @@ReadSector_1
        ; Three times repeat recalibration
        call    RecalibrateFDD
        call    SeekTrack
        inc     [RecalRepCounter]
        cmp     [RecalRepCounter], 3
        jb      @@TryAgain
@@Exit_2:
        popa
        ret
@@Err_3:
        popa
        ret

;*******************************************************
;*               WRITE DATA SECTOR                     *
;* Parameters are passed through global variables:     *
;* FDD_Track - track number (0-79);                    *
;* FDD_Head - head number (0-1);                       *
;* FDD_Sector - sector number (1-18).                  *
;* Result of operation is written to FDC_Status.       *
;* If the write operation is successful, the contents  *
;* of FDD_DataBuffer will be written to the sector     *
;*******************************************************
WriteSector:
;       dbgstr 'WriteSector'
        pushad
        call    save_timer_fdd_motor
; Clear the interrupt flag
        mov     [FDD_IntFlag], 0
; Set transmit speed to 500 Kb / s
        mov     AX, 0
        mov     DX, 03F7h
        out     DX, AL
; Initialize the DMA channel
        mov     [dmamode], 0x4A
        call    Init_FDC_DMA
; Send "Write data" command
        mov     AL, 0xC5 ;0x45  ; write in multi-track mode
        call    FDCDataOutput
        mov     AL, [FDD_Head]
        shl     AL, 2
        or      AL, [flp_number]
        dec     AL
        call    FDCDataOutput
        mov     AL, [FDD_Track]
        call    FDCDataOutput
        mov     AL, [FDD_Head]
        call    FDCDataOutput
        mov     AL, [FDD_Sector]
        call    FDCDataOutput
        mov     AL, 2   ; sector size code (512 bytes)
        call    FDCDataOutput
        mov     AL, 18; 3Fh  ; sectors per track
        call    FDCDataOutput
        mov     AL, 1Bh ; GPL value
        call    FDCDataOutput
        mov     AL, 0FFh; DTL value
        call    FDCDataOutput
; Waiting for an interrupt at the end of the operation
        call    WaitFDCInterrupt
        cmp     [FDC_Status], FDC_Normal
        jne     @@Exit_3
; Reading the completion status of the operation
        call    GetStatusInfo
        test    [FDC_ST0], 11000000b ;11011000b
        jnz     @@Err_2
        mov     [FDC_Status], FDC_Normal
        jmp     @@Exit_3
@@Err_2:
        mov     [FDC_Status], FDC_SectorNotFound
@@Exit_3:
        call    save_timer_fdd_motor
        popad
        ret

;*******************************************************
;*   WRITE SECTOR (WITH REPEAT ON FAILURE)             *
;* Parameters are passed through global variables:     *
;* FDD_Track - track number (0-79);                    *
;* FDD_Head - head number (0-1);                       *
;* FDD_Sector - sector number (1-18).                  *
;* Result of operation is written to FDC_Status.       *
;* If the write operation is successful, the contents  *
;* of FDD_DataBuffer will be written to the sector     *
;*******************************************************
WriteSectWithRetr:
        pusha
; Reset the recalibration repetition counter
        mov     [RecalRepCounter], 0
@@TryAgain_1:
; Reset the read operation retry counter
        mov     [ReadRepCounter], 0
@@WriteSector_1:
        call    WriteSector
        cmp     [FDC_Status], 0
        je      @@Exit_4
        cmp     [FDC_Status], 1
        je      @@Err_4
        ; Three times repeat writing
        inc     [ReadRepCounter]
        cmp     [ReadRepCounter], 3
        jb      @@WriteSector_1
        ; Three times repeat recalibration
        call    RecalibrateFDD
        call    SeekTrack
        inc     [RecalRepCounter]
        cmp     [RecalRepCounter], 3
        jb      @@TryAgain_1
@@Exit_4:
        popa
        ret
@@Err_4:
        popa
        ret

;*********************************************
;* GET INFORMATION ABOUT THE RESULT OF THE OPERATION
;*********************************************
GetStatusInfo:
        push    AX
        call    FDCDataInput
        mov     [FDC_ST0], AL
        call    FDCDataInput
        mov     [FDC_ST1], AL
        call    FDCDataInput
        mov     [FDC_ST2], AL
        call    FDCDataInput
        mov     [FDC_C], AL
        call    FDCDataInput
        mov     [FDC_H], AL
        call    FDCDataInput
        mov     [FDC_R], AL
        call    FDCDataInput
        mov     [FDC_N], AL
        pop     AX
        ret

; Interface for disk subsystem.
; Assume fixed capacity for 1.44M.
FLOPPY_CAPACITY = 2880  ; in sectors

iglobal
align 4
floppy_functions:
        dd      .size
        dd      0       ; no close() function
        dd      0       ; no closemedia() function
        dd      floppy_querymedia
        dd      floppy_read
        dd      floppy_write
        dd      0       ; no flush() function
        dd      0       ; no adjust_cache_size() function
.size = $ - floppy_functions
endg

uglobal
floppy_media_flags      rb      2
n_sector    dd 0  ; temporary save for sector value
flp_number  db 0  ; 1- Floppy A, 2-Floppy B
old_track   db 0  ; old value track
flp_label   rb 15*2 ; Label and ID of inserted floppy disk
align 4
; Hardware does not allow to work with two floppies in parallel,
; so there is one mutex guarding access to any floppy.
floppy_mutex    MUTEX
endg
; Meaning of bits in floppy_media_flags
FLOPPY_MEDIA_PRESENT = 1        ; media was present when last asked
FLOPPY_MEDIA_NEED_RESCAN = 2    ; media was possibly changed, need to rescan
FLOPPY_MEDIA_LABEL_CHANGED = 4  ; temporary state

iglobal
floppy1_name    db      'fd',0
floppy2_name    db      'fd2',0
endg

; This function is called in boot process.
; It creates filesystems /fd and/or /fd2, if the system has one/two floppy drives.
proc floppy_init
        mov     ecx, floppy_mutex
        call    mutex_init
; First floppy is present if [DRIVE_DATA] and 0xF0 is nonzero.
        test    byte [DRIVE_DATA], 0xF0
        jz      .no1
        stdcall disk_add, floppy_functions, floppy1_name, 1, DISK_NO_INSERT_NOTIFICATION
.no1:
; Second floppy is present if [DRIVE_DATA] and 0x0F is nonzero.
        test    byte [DRIVE_DATA], 0x0F
        jz      .no2
        stdcall disk_add, floppy_functions, floppy2_name, 2, DISK_NO_INSERT_NOTIFICATION
.no2:
        ret
endp

; Returns information about disk media.
; Floppy drives do not support insert notifications,
; DISK_NO_INSERT_NOTIFICATION is set,
; the disk subsystem calls this function before each filesystem operation.
; If the media has changed, return error for the first call as signal
; to finalize work with old media and the true geometry for the second call.
; Assume that media is (possibly) changed anytime when motor is off.
proc floppy_querymedia
  virtual at esp+4
    .userdata dd ?
    .info dd ?
  end virtual
; 1. Acquire the global lock.
        mov     ecx, floppy_mutex
        call    mutex_lock
        mov     edx, [.userdata]        ; 1 for /fd, 2 for /fd2
; 2. If the media was reported and has been changed, forget it and report an error.
        mov     al, [floppy_media_flags+edx-1]
        and     al, FLOPPY_MEDIA_PRESENT + FLOPPY_MEDIA_NEED_RESCAN
        cmp     al, FLOPPY_MEDIA_PRESENT + FLOPPY_MEDIA_NEED_RESCAN
        jnz     .not_reported
.no_media:
        mov     [floppy_media_flags+edx-1], 0
.return_no_media:
        mov     ecx, floppy_mutex
        call    mutex_unlock
        mov     eax, DISK_STATUS_NO_MEDIA
        retn    8
.not_reported:
; 3. If we are in the temporary state LABEL_CHANGED, this is the second call
; after intermediate DISK_STATUS_NO_MEDIA due to media change;
; clear the flag and return the current geometry without rereading the bootsector.
        cmp     [floppy_media_flags+edx-1], FLOPPY_MEDIA_LABEL_CHANGED
        jz      .report_geometry
; 4. Try to read the bootsector.
        mov     [flp_number], dl
        mov     [FDC_Status], 0
        call    floppy_read_bootsector
; 5. If reading bootsector failed, assume that media is not present.
        mov     edx, [.userdata]
        cmp     [FDC_Status], 0
        jnz     .no_media
; 6. Check whether the previous status is "present". If not, go to 10.
        push    esi edi
        imul    edi, edx, 15
        add     edi, flp_label-15
        mov     esi, FDD_BUFF+39
        mov     ecx, 15
        test    [floppy_media_flags+edx-1], FLOPPY_MEDIA_PRESENT
        jz      .set_label
; 7. Compare the old label with the current one.
        rep cmpsb
; 8. If the label has not changed, go to 11.
        jz      .ok
; 9. If the label has changed, store it, enter temporary state LABEL_CHANGED
; and report DISK_STATUS_NO_MEDIA.
;       dbgstr 'floppy label changed'
        add     esi, ecx
        add     edi, ecx
        mov     ecx, 15
        sub     esi, ecx
        sub     edi, ecx
        rep movsb
        mov     [floppy_media_flags+edx-1], FLOPPY_MEDIA_LABEL_CHANGED
        pop     edi esi
        jmp     .return_no_media
.set_label:
; 10. The previous state was "not present". Copy the label.
        rep movsb
.ok:
        pop     edi esi
.report_geometry:
; 11. Fill DISKMEDIAINFO structure.
        mov     ecx, [.info]
        and     [ecx+DISKMEDIAINFO.Flags], 0
        mov     [ecx+DISKMEDIAINFO.SectorSize], 512
        mov     dword [ecx+DISKMEDIAINFO.Capacity], FLOPPY_CAPACITY
        and     dword [ecx+DISKMEDIAINFO.Capacity+4], 0
; 12. Update state: media is present, data are actual.
        mov     [floppy_media_flags+edx-1], FLOPPY_MEDIA_PRESENT
; 13. Release the global lock and return successful status.
        mov     ecx, floppy_mutex
        call    mutex_unlock
        xor     eax, eax
        retn    8
endp

proc floppy_read_bootsector
        pushad
        mov     [FDD_Track], 0  ; Cylinder
        mov     [FDD_Head], 0   ; Head
        mov     [FDD_Sector], 1 ; Sector
        call    FDDMotorON
        call    RecalibrateFDD
        cmp     [FDC_Status], 0
        jne     .nothing
        call    SeekTrack
        cmp     [FDC_Status], 0
        jne     .nothing
        call    ReadSectWithRetr
.nothing:
        popad
        ret
endp

read_chs_sector:
        call    calculate_chs
        call    ReadSectWithRetr
        ret

save_chs_sector:
        call    calculate_chs
        call    WriteSectWithRetr
        ret

calculate_chs:
        mov     bl, [FDD_Track]
        mov     [old_track], bl
        mov     ebx, 18
        xor     edx, edx
        div     ebx
        inc     edx
        mov     [FDD_Sector], dl
        mov     edx, eax
        shr     eax, 1
        and     edx, 1
        mov     [FDD_Track], al
        mov     [FDD_Head], dl
        mov     dl, [old_track]
        cmp     dl, [FDD_Track]
        je      no_seek_track_1
        call    SeekTrack
no_seek_track_1:
        ret

; Writes one or more sectors to the device.
proc floppy_write
        mov     dl, 1
        jmp     floppy_read_write
endp

; Reads one or more sectors from the device.
proc floppy_read
        mov     dl, 0
endp

; Common part of floppy_read and floppy_write.
proc floppy_read_write userdata:dword, buffer:dword, start_sector:qword, numsectors_ptr:dword
virtual at ebp-8
.sectors_todo   dd      ?
.operation      db      ?
end virtual
        push    edx             ; save operation code to [.operation]
; 1. Get number of sectors to read/write
; and zero number of sectors that were actually read/written.
        mov     eax, [numsectors_ptr]
        push    dword [eax]     ; initialize [.sectors_todo]
        and     dword [eax], 0
        push    ebx esi edi     ; save used registers to be stdcall
; 2. Acquire the global lock.
        mov     ecx, floppy_mutex
        call    mutex_lock
; 3. Set floppy number for this operation.
        mov     edx, [userdata]
        mov     [flp_number], dl
; 4. Read/write sector-by-sector.
.operation_loop:
; 4a. Check that the sector is inside the media.
        cmp     dword [start_sector+4], 0
        jnz     .end_of_media
        mov     eax, dword [start_sector]
        cmp     eax, FLOPPY_CAPACITY
        jae     .end_of_media
; 4b. For read operation, call read_chs_sector and then move data from FDD_BUFF to [buffer].
; For write operation, move data from [buffer] to FDD_BUFF and then call save_chs_sector.
        cmp     [.operation], 0
        jz      .read
        mov     esi, [buffer]
        mov     edi, FDD_BUFF
        mov     ecx, 512/4
        rep movsd
        mov     [buffer], esi
        call    save_chs_sector
        jmp     @f
.read:
        call    read_chs_sector
        mov     esi, FDD_BUFF
        mov     edi, [buffer]
        mov     ecx, 512/4
        rep movsd
        mov     [buffer], edi
@@:
; 4c. If there was an error, propagate it to the caller.
        cmp     [FDC_Status], 0
        jnz     .fail
; 4d. Otherwise, increment number of sectors processed and continue the loop.
        mov     eax, [numsectors_ptr]
        inc     dword [eax]
        inc     dword [start_sector]
        dec     [.sectors_todo]
        jnz     .operation_loop
; 5. Release the global lock and return with the correct status.
        push    0
.return:
        mov     ecx, floppy_mutex
        call    mutex_unlock
        pop     eax
        pop     edi esi ebx     ; restore used registers to be stdcall
        ret     ; this translates to leave/retn N and purges local variables
.fail:
        push    -1
        jmp     .return
.end_of_media:
        push    DISK_STATUS_END_OF_MEDIA
        jmp     .return
endp
