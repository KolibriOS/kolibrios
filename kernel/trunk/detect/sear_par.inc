;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

search_partitions:
; 1. Fill missing parameters in HD_DATA structures.
        mov     eax, [hd_address_table]
        mov     [hd0_data.hdbase], eax   ;0x1f0
        mov     [hd1_data.hdbase], eax
        mov     eax, [hd_address_table+16]
        mov     [hd2_data.hdbase], eax
        mov     [hd3_data.hdbase], eax
; 2. Notify the system about /hd* disks.
; For every existing disk, call ide_disk_add with correct parameters.
; Generate name "hdN" on the stack; this is 4 bytes including terminating zero.
; 2a. /hd0: exists if mask 0x40 in [DRIVE_DATA+1] is set,
;     data: hd0_data,
;     number of partitions: [DRIVE_DATA+2]
        test    [DRIVE_DATA+1], byte 0x40
        jz      @f
        push    'hd0'
        mov     eax, esp        ; name
        mov     edx, hd0_data
        call    ide_disk_add
        mov     [DRIVE_DATA+2], al
        pop     ecx             ; restore the stack
@@:
; 2b. /hd1: exists if mask 0x10 in [DRIVE_DATA+1] is set,
;     data: hd1_data,
;     number of partitions: [DRIVE_DATA+3]
        test    [DRIVE_DATA+1], byte 0x10
        jz      @f
        push    'hd1'
        mov     eax, esp
        mov     edx, hd1_data
        call    ide_disk_add
        mov     [DRIVE_DATA+3], al
        pop     ecx
@@:
; 2c. /hd2: exists if mask 4 in [DRIVE_DATA+1] is set,
;     data: hd2_data,
;     number of partitions: [DRIVE_DATA+4]
        test    [DRIVE_DATA+1], byte 4
        jz      @f
        push    'hd2'
        mov     eax, esp
        mov     edx, hd2_data
        call    ide_disk_add
        mov     [DRIVE_DATA+4], al
        pop     ecx
@@:
; 2d. /hd3: exists if mask 1 in [DRIVE_DATA+1] is set,
;     data: hd3_data,
;     number of partitions: [DRIVE_DATA+5]
        test    [DRIVE_DATA+1], byte 1
        jz      @f
        push    'hd3'
        mov     eax, esp
        mov     edx, hd3_data
        call    ide_disk_add
        mov     [DRIVE_DATA+5], al
        pop     ecx
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
        jmp     end_search_partitions

; Helper procedure for search_partitions, adds one IDE disk.
; For compatibility, number of partitions for IDE disks is kept in a separate variable,
; so the procedure returns number of partitions.
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

 end_search_partitions:

