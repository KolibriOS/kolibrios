;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

search_partitions:
        push    ecx
; 1. Fill missing parameters in HD_DATA structures.
        xor     eax, eax
        mov     edx, IDE_controller_1
        mov     ax, [edx + IDE_DATA.BAR0_val]
        mov     [hd0_data.hdbase], eax
        mov     [hd1_data.hdbase], eax
        mov     ax, [edx + IDE_DATA.BAR2_val]
        mov     [hd2_data.hdbase], eax
        mov     [hd3_data.hdbase], eax

        mov     edx, IDE_controller_2
        mov     ax, [edx + IDE_DATA.BAR0_val]
        mov     [hd4_data.hdbase], eax
        mov     [hd5_data.hdbase], eax
        mov     ax, [edx + IDE_DATA.BAR2_val]
        mov     [hd6_data.hdbase], eax
        mov     [hd7_data.hdbase], eax

        mov     edx, IDE_controller_3
        mov     ax, [edx + IDE_DATA.BAR0_val]
        mov     [hd8_data.hdbase], eax
        mov     [hd9_data.hdbase], eax
        mov     ax, [edx + IDE_DATA.BAR2_val]
        mov     [hd10_data.hdbase], eax
        mov     [hd11_data.hdbase], eax
; 2. Notify the system about /hd* disks.
; For every existing disk, call ide_disk_add with correct parameters.
; Generate name "hdN" on the stack; this is 4 bytes including terminating zero.
;-----------------------------------------------------------------------------
; 2a. /hd0: exists if mask 0x40 in [DRIVE_DATA+1] is set,
;     data: hd0_data,
;     number of partitions: [DRIVE_DATA+2]
        test    [DRIVE_DATA+1], byte 0x40
        jz      @f
        DEBUGF  1, "K : HD0\n"
        push    'hd0'
        mov     eax, esp        ; name
        mov     edx, hd0_data
        call    ide_disk_add
        mov     [DRIVE_DATA+2], al
        pop     ecx             ; restore the stack
;-----------------------------------------------------------------------------
@@:
; 2b. /hd1: exists if mask 0x10 in [DRIVE_DATA+1] is set,
;     data: hd1_data,
;     number of partitions: [DRIVE_DATA+3]
        test    [DRIVE_DATA+1], byte 0x10
        jz      @f
        DEBUGF  1, "K : HD1\n"
        push    'hd1'
        mov     eax, esp
        mov     edx, hd1_data
        call    ide_disk_add
        mov     [DRIVE_DATA+3], al
        pop     ecx
;-----------------------------------------------------------------------------
@@:
; 2c. /hd2: exists if mask 4 in [DRIVE_DATA+1] is set,
;     data: hd2_data,
;     number of partitions: [DRIVE_DATA+4]
        test    [DRIVE_DATA+1], byte 4
        jz      @f
        DEBUGF  1, "K : HD2\n"
        push    'hd2'
        mov     eax, esp
        mov     edx, hd2_data
        call    ide_disk_add
        mov     [DRIVE_DATA+4], al
        pop     ecx
;-----------------------------------------------------------------------------
@@:
; 2d. /hd3: exists if mask 1 in [DRIVE_DATA+1] is set,
;     data: hd3_data,
;     number of partitions: [DRIVE_DATA+5]
        test    [DRIVE_DATA+1], byte 1
        jz      @f
        DEBUGF  1, "K : HD3\n"
        push    'hd3'
        mov     eax, esp
        mov     edx, hd3_data
        call    ide_disk_add
        mov     [DRIVE_DATA+5], al
        pop     ecx
;-----------------------------------------------------------------------------
@@:
; 2e. /hd4: exists if mask 0x40 in [DRIVE_DATA+6] is set,
;     data: hd4_data,
;     number of partitions: [DRIVE_DATA+7]
        test    [DRIVE_DATA+6], byte 0x40
        jz      @f
        DEBUGF  1, "K : HD4\n"
        push    'hd4'
        mov     eax, esp        ; name
        mov     edx, hd4_data
        call    ide_disk_add
        mov     [DRIVE_DATA+7], al
        pop     ecx
;-----------------------------------------------------------------------------
@@:
; 2f. /hd5: exists if mask 0x10 in [DRIVE_DATA+6] is set,
;     data: hd5_data,
;     number of partitions: [DRIVE_DATA+8]
        test    [DRIVE_DATA+6], byte 0x10
        jz      @f
        DEBUGF  1, "K : HD5\n"
        push    'hd5'
        mov     eax, esp
        mov     edx, hd5_data
        call    ide_disk_add
        mov     [DRIVE_DATA+8], al
        pop     ecx
;-----------------------------------------------------------------------------
@@:
; 2g. /hd6: exists if mask 4 in [DRIVE_DATA+6] is set,
;     data: hd6_data,
;     number of partitions: [DRIVE_DATA+9]
        test    [DRIVE_DATA+6], byte 4
        jz      @f
        DEBUGF  1, "K : HD6\n"
        push    'hd6'
        mov     eax, esp
        mov     edx, hd6_data
        call    ide_disk_add
        mov     [DRIVE_DATA+9], al
        pop     ecx
;-----------------------------------------------------------------------------
@@:
; 2h. /hd7: exists if mask 1 in [DRIVE_DATA+6] is set,
;     data: hd7_data,
;     number of partitions: [DRIVE_DATA+10]
        test    [DRIVE_DATA+6], byte 1
        jz      @f
        DEBUGF  1, "K : HD7\n"
        push    'hd7'
        mov     eax, esp
        mov     edx, hd7_data
        call    ide_disk_add
        mov     [DRIVE_DATA+10], al
        pop     ecx
;-----------------------------------------------------------------------------
@@:
; 2i. /hd8: exists if mask 0x40 in [DRIVE_DATA+11] is set,
;     data: hd8_data,
;     number of partitions: [DRIVE_DATA+12]
        test    [DRIVE_DATA+11], byte 0x40
        jz      @f
        DEBUGF  1, "K : HD8\n"
        push    'hd8'
        mov     eax, esp        ; name
        mov     edx, hd8_data
        call    ide_disk_add
        mov     [DRIVE_DATA+12], al
        pop     ecx
;-----------------------------------------------------------------------------
@@:
; 2j. /hd9: exists if mask 0x10 in [DRIVE_DATA+11] is set,
;     data: hd9_data,
;     number of partitions: [DRIVE_DATA+13]
        test    [DRIVE_DATA+11], byte 0x10
        jz      @f
        DEBUGF  1, "K : HD9\n"
        push    'hd9'
        mov     eax, esp
        mov     edx, hd9_data
        call    ide_disk_add
        mov     [DRIVE_DATA+13], al
        pop     ecx
;-----------------------------------------------------------------------------
@@:
; 2k. /hd10: exists if mask 4 in [DRIVE_DATA+11] is set,
;     data: hd10_data,
;     number of partitions: [DRIVE_DATA+14]
        test    [DRIVE_DATA+14], byte 4
        jz      @f
        DEBUGF  1, "K : HD10\n"
        push    'hd10'
        mov     eax, esp
        mov     edx, hd10_data
        call    ide_disk_add
        mov     [DRIVE_DATA+9], al
        pop     ecx
;-----------------------------------------------------------------------------
@@:
; 2l. /hd11: exists if mask 1 in [DRIVE_DATA+11] is set,
;     data: hd11_data,
;     number of partitions: [DRIVE_DATA+15]
        test    [DRIVE_DATA+11], byte 1
        jz      @f
        DEBUGF  1, "K : HD11\n"
        push    'hd11'
        mov     eax, esp
        mov     edx, hd11_data
        call    ide_disk_add
        mov     [DRIVE_DATA+15], al
        pop     ecx
;-----------------------------------------------------------------------------
@@:
; 3. Notify the system about /bd* disks.
; 3a. Check whether there are BIOS disks. If no, skip step 3.
        xor     esi, esi
        cmp     esi, [NumBiosDisks]
        jz      .nobd
; Loop over all disks.
        push    0
        push    'bd'
.bdloop:
; 3b. Get the drive number for using in /bd* name.
        movzx   eax, byte [BiosDisksData+esi*4]
        sub     al, 80h
; 3c. Convert eax to decimal and store starting with [esp+3].
; First 2 bytes in [esp] are "bd".
        lea     edi, [esp+2]
; store digits in the stack, ending with -'0'
        push    -'0'
@@:
        xor     edx, edx
iglobal
align 4
_10     dd      10
endg
        div     [_10]
        push    edx
        test    eax, eax
        jnz     @b
; restore digits from the stack, this reverses the order;
; add '0', stop, when zero is reached
@@:
        pop     eax
        add     al, '0'
        stosb
        jnz     @b
; 3e. Call the API with userdata = 80h + ecx.
        mov     eax, esp
        lea     edx, [esi+80h]
        stdcall disk_add, bd_callbacks, eax, edx, 0
        test    eax, eax
        jz      @f
        stdcall disk_media_changed, eax, 1
@@:
; 3f. Continue the loop.
        inc     esi
        cmp     esi, [NumBiosDisks]
        jnz     .bdloop
        pop     ecx ecx ; restore stack after name
.nobd:
        DEBUGF  1, "K : jmp end_search_partitions\n"
        jmp     end_search_partitions
;-----------------------------------------------------------------------------
; Helper procedure for search_partitions, adds one IDE disk.
; For compatibility, number of partitions for IDE disks is kept in a separate
; variable, so the procedure returns number of partitions.
; eax -> name, edx -> disk data
proc ide_disk_add
        stdcall disk_add, ide_callbacks, eax, edx, 0
        test    eax, eax
        jz      @f
        push    eax
        stdcall disk_media_changed, eax, 1
        pop     eax
        mov     eax, [eax+DISK.NumPartitions]
        cmp     eax, 255
        jbe     @f
        mov     eax, 255
@@:
        ret
endp
;-----------------------------------------------------------------------------
end_search_partitions:
        pop     ecx
