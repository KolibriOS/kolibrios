;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; CD external functions
;   in:
; esi -> path string in UTF-8
; ebx -> parameter structure +4
; ecx = bytes to read
; edx -> buffer
;   out:
; eax, ebx = return values for sysfunc 70
iglobal
align 4
fs_CdServices:
        dd      fs_CdRead
        dd      fs_CdReadFolder
        dd      fs_NotImplemented
        dd      fs_NotImplemented
        dd      fs_NotImplemented
        dd      fs_CdGetFileInfo
        dd      fs_NotImplemented
        dd      0
        dd      fs_NotImplemented
        dd      fs_NotImplemented
fs_NumCdServices = ($ - fs_CdServices)/4
endg

uglobal
align 4
cd_current_pointer_of_input     dd  0
cd_current_pointer_of_input_2   dd  0
cd_mem_location                 dd  0
cd_counter_block                dd  0
cd_status                       dd  0
endg

;-----------------------------------------------------------------------------
fs_NotImplemented:
        movi    eax, ERROR_UNSUPPORTED_FS
        ret
;-----------------------------------------------------------------------------
reserve_cd:
        cli
        cmp     [cd_status], 0
        je      reserve_ok2

        sti
        call    change_task
        jmp     reserve_cd
;-----------------------------------------------------------------------------
reserve_ok2:
        push    eax
        mov     eax, [current_slot]
        mov     eax, [eax + APPDATA.tid]
        mov     [cd_status], eax
        pop     eax
        sti
        ret
;-----------------------------------------------------------------------------
reserve_cd_channel:
        pushad
        mov     eax, [cdpos]
        dec     eax
        shr     eax, 2

        test    eax, eax
        jnz     .1

        cmp     [ChannelNumber], 1
        jne     @f

        mov     ecx, ide_channel1_mutex
        jmp     .mutex_lock
;--------------------------------------
@@:
        mov     ecx, ide_channel2_mutex
        jmp     .mutex_lock
;--------------------------------------
.1:
        dec     eax
        jnz     .2

        cmp     [ChannelNumber], 1
        jne     @f

        mov     ecx, ide_channel3_mutex
        jmp     .mutex_lock
;--------------------------------------
@@:
        mov     ecx, ide_channel4_mutex
        jmp     .mutex_lock
;--------------------------------------
.2:
        cmp     [ChannelNumber], 1
        jne     @f

        mov     ecx, ide_channel5_mutex
        jmp     .mutex_lock
;--------------------------------------
@@:
        mov     ecx, ide_channel6_mutex
.mutex_lock:
        call    mutex_lock
        popad
        ret
;-----------------------------------------------------------------------------
free_cd_channel:
        pushad
        mov     eax, [cdpos]
        dec     eax
        shr     eax, 2

        test    eax, eax
        jnz     .1

        cmp     [ChannelNumber], 1
        jne     @f

        mov     ecx, ide_channel1_mutex
        jmp     .mutex_unlock
;--------------------------------------
@@:
        mov     ecx, ide_channel2_mutex
        jmp     .mutex_unlock
;--------------------------------------
.1:
        dec     eax
        jnz     .2

        cmp     [ChannelNumber], 1
        jne     @f

        mov     ecx, ide_channel3_mutex
        jmp     .mutex_unlock
;--------------------------------------
@@:
        mov     ecx, ide_channel4_mutex
        jmp     .mutex_unlock
;--------------------------------------
.2:
        cmp     [ChannelNumber], 1
        jne     @f

        mov     ecx, ide_channel5_mutex
        jmp     .mutex_unlock
;--------------------------------------
@@:
        mov     ecx, ide_channel6_mutex
.mutex_unlock:
        call    mutex_unlock
        popad
        ret

;-----------------------------------------------------------------------------
fs_CdRead:
        call    cd_find_lfn
        jc      .notFound
        mov     edi, [cd_current_pointer_of_input]
        test    byte [edi+25], 10b  ; do not allow read directories
        jnz     .noaccess
        test    ebx, ebx
        jz      .l1
        cmp     dword [ebx+4], 0
        jz      @f
        xor     ebx, ebx
        movi    eax, ERROR_END_OF_FILE
        ret

.notFound:
        cmp     [DevErrorCode], 0
        jne     .noaccess
        xor     ebx, ebx
        movi    eax, ERROR_FILE_NOT_FOUND
        ret

.noaccess_3:
        pop     eax edx ecx
.noaccess:
        xor     ebx, ebx
        movi    eax, ERROR_ACCESS_DENIED
        ret

@@:
        mov     ebx, [ebx]
.l1:
        push    ecx edx 0
        mov     eax, [edi+10]   ; real size of the file section
        sub     eax, ebx
        jb      .eof
        cmp     eax, ecx
        jae     @f
        mov     ecx, eax
        pop     eax
        push    ERROR_END_OF_FILE
@@:
        mov     eax, [edi+2]
        mov     [CDSectorAddress], eax
; now eax=cluster, ebx=position, ecx=count, edx=buffer for data
.new_sector:
        test    ecx, ecx
        jz      .done
        sub     ebx, 2048
        jae     .next
        add     ebx, 2048
        jnz     .incomplete_sector
        cmp     ecx, 2048
        jb      .incomplete_sector
; we may read and memmove complete sector
        mov     [CDDataBuf_pointer], edx
        call    ReadCDWRetr
        cmp     [DevErrorCode], 0
        jne     .noaccess_3
        add     edx, 2048
        sub     ecx, 2048
.next:
        inc     dword [CDSectorAddress]
        jmp     .new_sector

.eof:
        pop     eax
        push    ERROR_END_OF_FILE
.done:
        mov     ebx, edx
        pop     eax edx ecx
        sub     ebx, edx
        ret

.incomplete_sector: ; we must read and memmove incomplete sector
        mov     [CDDataBuf_pointer], CDDataBuf
        call    ReadCDWRetr
        cmp     [DevErrorCode], 0
        jne     .noaccess_3
        push    ecx
        add     ecx, ebx
        cmp     ecx, 2048
        jbe     @f
        mov     ecx, 2048
@@:
        sub     ecx, ebx
        push    edi esi ecx
        mov     edi, edx
        lea     esi, [CDDataBuf + ebx]
        cld
        rep movsb
        pop     ecx esi edi
        add     edx, ecx
        sub     [esp], ecx
        pop     ecx
        xor     ebx, ebx
        jmp     .next

;-----------------------------------------------------------------------------
fs_CdReadFolder:
        push    edi
        call    cd_find_lfn
        jnc     .found
        pop     edi
        cmp     [DevErrorCode], 0
        jne     .noaccess_1
        xor     ebx, ebx
        mov     eax, ERROR_FILE_NOT_FOUND
        ret

.found:
        mov     edi, [cd_current_pointer_of_input]
        test    byte [edi+25], 10b    ; do not allow read directories
        jnz     .found_dir
        pop     edi
.noaccess_1:
        xor     ebx, ebx
        mov     eax, ERROR_ACCESS_DENIED
        ret

.end_buffer:
        pop     edx eax
        sub     eax, 2048   ; directory is over?
        ja      .read_to_buffer
        mov     eax, [cd_counter_block]
        mov     [edx+8], eax
        mov     eax, [ebx]
        sub     [edx+4], eax
        xor     eax, eax
        dec     ecx
        js      @f
        mov     al, ERROR_END_OF_FILE
@@:
        pop     ecx edi
        mov     ebx, [edx+4]
        ret

.found_dir:
        mov     eax, [edi+2]    ; eax=cluster
        mov     [CDSectorAddress], eax
        mov     eax, [edi+10]   ; directory size
        push    eax ecx
        mov     edi, edx
        mov     ecx, 32/4
        xor     eax, eax
        rep stosd
        pop     ecx eax
        mov     byte [edx], 1   ; version
        mov     [cd_mem_location], edx
        add     [cd_mem_location], 32
        mov     [cd_counter_block], dword 0
        dec     dword [CDSectorAddress]
        push    ecx
.read_to_buffer:
        inc     dword [CDSectorAddress]
        mov     [CDDataBuf_pointer], CDDataBuf
        call    ReadCDWRetr     ; read sector of directory
        cmp     [DevErrorCode], 0
        jne     .noaccess_1
        mov     [cd_current_pointer_of_input_2], CDDataBuf
        push    eax edx
.get_names_from_buffer:
        call    cd_get_name
        jc      .end_buffer
        inc     dword [cd_counter_block]
        mov     eax, [cd_counter_block]
        cmp     [ebx], eax
        jae     .get_names_from_buffer
        test    ecx, ecx
        jz      .get_names_from_buffer
        mov     edi, [cd_counter_block]
        mov     [edx+4], edi
        dec     ecx
        mov     esi, ebp
        call    cd_get_parameters_of_file
        add     edi, 40
        mov     ax, '.'
        cmp     dword[ebx+4], 2
        jz      .utf16
        cmp     dword[ebx+4], 3
        jz      .utf8
        cmp     [cd_counter_block], 2
        jbe     .parentDirectory
@@:
        lodsw
        xchg    ah, al
        call    uni2ansi_char
        stosb
        call    .checkForEnd
        jc      @b
@@:
        mov     [edi], byte 0
        add     [cd_mem_location], 304
        jmp     .get_names_from_buffer

.parentDirectory:
        stosb
        cmp     [cd_counter_block], 2
        jnz     @b
        stosb
        jmp     @b

.utf8:
        add     [cd_mem_location], 256
        cmp     [cd_counter_block], 2
        jbe     .parentDirectory
        push    ecx
        mov     ecx, 519
@@:
        lodsw
        xchg    ah, al
        call    UTF16to8
        js      @f
        call    .checkForEnd
        jc      @b
@@:
        pop     ecx
        mov     [edi], byte 0
        add     [cd_mem_location], 304
        jmp     .get_names_from_buffer

.checkForEnd:
        mov     ax, [esi]
        cmp     ax, 3B00h   ; ';'
        jz      @f
; check for files not ending with separator
        movzx   eax, byte [ebp-33]
        add     eax, ebp
        sub     eax, 34
        cmp     esi, eax
        jz      @f
; check the end of the directory
        movzx   eax, byte [ebp-1]
        add     eax, ebp
        cmp     esi, eax
@@:
        ret

.utf16:
        cmp     [cd_counter_block], 2
        jbe     .utf16ParentDirectory
@@:
        lodsw
        xchg    ah, al
        stosw
        call    .checkForEnd
        jc      @b
@@:
        mov     [edi], word 0
        add     [cd_mem_location], 560
        jmp     .get_names_from_buffer

.utf16ParentDirectory:
        stosw
        cmp     [cd_counter_block], 2
        jnz     @b
        stosw
        jmp     @b

cd_get_parameters_of_file:
        mov     edi, [cd_mem_location]
cd_get_parameters_of_file_1:
; get file attributes
        xor     eax, eax
; file is not archived
        inc     eax
        shl     eax, 1
; is a directory?
        test    [ebp-8], byte 2
        jz      .file
        inc     eax
.file:
; not as a volume label in the FAT, in this form not available
; file is not a system
        shl     eax, 3
; file is hidden? (attribute of existence)
        test    [ebp-8], byte 1
        jz      .hidden
        inc     eax
.hidden:
        shl     eax, 1
; file is always read-only, as this CD
        inc     eax
        mov     [edi], eax
        mov     eax, [ebx+4]
        mov     [edi+4], eax
; get the time to file
; hour
        movzx   eax, byte [ebp-12]
        shl     eax, 8
; minute
        mov     al, [ebp-11]
        shl     eax, 8
; second
        mov     al, [ebp-10]
; file creation time
        mov     [edi+8], eax
; last access time
        mov     [edi+16], eax
; last write time
        mov     [edi+24], eax
; get date for file
; year
        movzx   eax, byte [ebp-15]
        add     eax, 1900
        shl     eax, 8
; month
        mov     al, [ebp-14]
        shl     eax, 8
; day
        mov     al, [ebp-13]
; file creation date
        mov     [edi+12], eax
; last access date
        mov     [edi+20], eax
; last write date
        mov     [edi+28], eax
; get the file size in bytes
        xor     eax, eax
        mov     [edi+32+4], eax
        mov     eax, [ebp-23]
        mov     [edi+32], eax
        ret

;-----------------------------------------------------------------------------
fs_CdGetFileInfo:
        call    cd_find_lfn
        movi    eax, ERROR_FILE_NOT_FOUND
        jc      @f
        mov     edi, edx
        mov     eax, [ebx+4]
        mov     [edx+4], eax
        cmp     byte [esi], 0
        jz      .volume
        mov     ebp, [cd_current_pointer_of_input]
        add     ebp, 33
        call    cd_get_parameters_of_file_1
        xor     eax, eax
@@:
        ret

.volume:
        test    eax, eax
        jz      .size
        mov     ecx, 16
        mov     esi, CDDataBuf+40
        add     edi, 40
        cmp     eax, 2
        jz      .utf16
        cmp     eax, 3
        jz      .utf8
@@:
        lodsw
        xchg    al, ah
        call    uni2ansi_char
        stosb
        loop    @b
        jmp     .size

.utf16:
        lodsw
        xchg    al, ah
        stosw
        loop    .utf16
        jmp     .size

.utf8:
        mov     ebx, ecx
        shl     ecx, 1
@@:
        lodsw
        xchg    ah, al
        call    UTF16to8
        dec     ebx
        jnz     @b
.size:
        mov     eax, [CDDataBuf+80]
        shl     eax, 11
        mov     [edx+32], eax
        xor     eax, eax
        mov     [edx+36], eax
        stosw
        mov     byte [edx], 8
        ret

;-----------------------------------------------------------------------------
cd_find_lfn:
        mov     [cd_appl_data], 0
; in: esi -> path string in UTF-8
; out: [cd_current_pointer_of_input] -> direntry, CF=1 -> file not found
        push    eax esi
; Sector 16 - start set of volume descriptors
        call    WaitUnitReady
        cmp     [DevErrorCode], 0
        jne     .access_denied

        call    prevent_medium_removal
; testing of reading
        mov     [CDSectorAddress], dword 16
        mov     [CDDataBuf_pointer], CDDataBuf
        call    ReadCDWRetr;_1
        cmp     [DevErrorCode], 0
        jne     .access_denied

; calculation of the last session
        call    WaitUnitReady
        cmp     [DevErrorCode], 0
        jne     .access_denied

        call    Read_TOC
        mov     ah, [CDDataBuf+4+4]
        mov     al, [CDDataBuf+4+5]
        shl     eax, 16
        mov     ah, [CDDataBuf+4+6]
        mov     al, [CDDataBuf+4+7]
        add     eax, 15
        mov     [CDSectorAddress], eax
;  mov  [CDSectorAddress],dword 15
        mov     [CDDataBuf_pointer], CDDataBuf
;--------------------------------------
.start:
        inc     dword [CDSectorAddress]
        call    ReadCDWRetr;_1
        cmp     [DevErrorCode], 0
        jne     .access_denied

.start_check:
; checking for "lice"
        cmp     [CDDataBuf+1], dword 'CD00'
        jne     .access_denied

        cmp     [CDDataBuf+5], byte '1'
        jne     .access_denied
; sector is the terminator of set of descriptors volumes?
        cmp     [CDDataBuf], byte 0xff
        je      .access_denied
; sector is an additional and improved descriptor of volume?
        cmp     [CDDataBuf], byte 0x2
        jne     .start
; sector is an additional descriptor of volume?
        cmp     [CDDataBuf+6], byte 0x1
        jne     .start

; parameters of root directory
        mov     eax, [CDDataBuf+0x9c+2]; start of root directory
        mov     [CDSectorAddress], eax
        mov     eax, [CDDataBuf+0x9c+10]; size of root directory
        cmp     byte [esi], 0
        jnz     @f

        mov     [cd_current_pointer_of_input], CDDataBuf+0x9c
        jmp     .done
;--------------------------------------
@@:
; start the search
.mainloop:
        dec     dword [CDSectorAddress]
;--------------------------------------
.read_to_buffer:
        inc     dword [CDSectorAddress]
        mov     [CDDataBuf_pointer], CDDataBuf
        call    ReadCDWRetr      ; read sector of directory
        cmp     [DevErrorCode], 0
        jne     .access_denied
        call    cd_find_name_in_buffer
        jnc     .found
        sub     eax, 2048
; directory is over?
        cmp     eax, 0
        ja      .read_to_buffer
; desired element of chain is not found
.access_denied:
        pop     esi eax
        mov     [cd_appl_data], 1
        stc
        ret
;--------------------------------------
; desired element of chain found
.found:
; the end of the file path
        cmp     byte [esi-1], 0
        jz      .done
        mov     eax, [cd_current_pointer_of_input]
        push    dword [eax+2]
        pop     dword [CDSectorAddress] ; beginning of the directory
        mov     eax, [eax+2+8] ; size of directory
        jmp     .mainloop
;--------------------------------------
; file pointer found
.done:
        pop     esi eax
        mov     [cd_appl_data], 1
        clc
        ret
;-----------------------------------------------------------------------------
cd_find_name_in_buffer:
        mov     [cd_current_pointer_of_input_2], CDDataBuf
;--------------------------------------
.start:
        call    cd_get_name
        jc      .not_found

        call    cd_compare_name
        jc      .start
;--------------------------------------
.found:
        clc
        ret
;--------------------------------------
.not_found:
        stc
        ret
;-----------------------------------------------------------------------------
cd_get_name:
        push    eax
        mov     ebp, [cd_current_pointer_of_input_2]
        mov     [cd_current_pointer_of_input], ebp
        mov     eax, [ebp]
        test    eax, eax ; entry's is over?
        jz      .next_sector

        cmp     ebp, CDDataBuf+2048  ; buffer is over?
        jae     .next_sector

        movzx   eax, byte [ebp]
        add     [cd_current_pointer_of_input_2], eax ; next entry of directory
        add     ebp, 33; pointer is set to the beginning of the name
        pop     eax
        clc
        ret
;--------------------------------------
.next_sector:
        pop     eax
        stc
        ret
;-----------------------------------------------------------------------------
cd_compare_name:
; in: esi -> UTF-8 name, ebp -> UTF-16BE name
; out: CF=0 -> names match, esi -> next component of name
;      CF=1 -> esi is not changed
        push    edx edi eax esi
        mov     edi, ebp
.loop:
        call    utf8to16
        call    utf16toUpper
        mov     edx, eax
        mov     ax, [edi]
        xchg    al, ah
        call    utf16toUpper
        cmp     ax, dx
        jne     .name_not_coincide
        add     edi, 2
        cmp     [esi], byte '/' ; path separator is end of current element
        je      .done
        cmp     [esi], byte 0 ; path separator end of name
        jne     .loop
.done:
; check end of file
        cmp     [edi], word 3B00h; separator end of file ';'
        je      .done_1
; check for files not ending with separator
        movzx   eax, byte [ebp-33]
        add     eax, ebp
        sub     eax, 34
        cmp     edi, eax
        je      .done_1
; check the end of directory
        movzx   eax, byte [ebp-1]
        add     eax, ebp
        cmp     edi, eax
        jne     .name_not_coincide
.done_1:
        pop     eax eax edi edx
        inc     esi
        ret

.name_not_coincide:
        pop     esi eax edi edx
        stc
        ret

;;; New driver

DESCRIPTOR_TYPE_BOOT          = 0x00    ; not used
DESCRIPTOR_TYPE_PRIMARY       = 0x01
DESCRIPTOR_TYPE_SUPPLEMENTARY = 0x02
; DESCRIPTOR_TYPE_PRIMARY for Juliet(UTF-16)
DESCRIPTOR_TYPE_PARTITION     = 0x02
DESCRIPTOR_TYPE_TERMANATOR    = 0xff    ; end scan descriptors

struct  ISO9660_VOLUME_DESCRIPTOR
        type    rb 1
        magic   rb 5  ; 'CD001'
        version rb 1
ends

struct  ISO9660_PRIMARY_DESCRIPTOR  ISO9660_VOLUME_DESCRIPTOR
                        rb 1
                        rb 32
        VolumeName      rb 32 ; ascii or utf-16
                        rb 8
        VolumeSpaceSize rb 8
                        rb 32
                        rb 4 ; +120
                        rb 4
        LBA_Size        rb 4 ; +128
        PathTableSize   rb 8         ; Do not use it!!!
        LBA_PathTable   rb 4 ; +140  ; Do not use it!!!
                        rb 4 ; +144  ; Do not use it!!!
                        rb 8
        root_dir_record rb 34  ;ISO9660_DIRECTORY_RECORD
                        rb 128*4 + 37*3
        date_created    rb 17
        date_modiffed   rb 17
                        rb 17
        file_struct_ver rb 1 ;always 0x01
ends

struct  ISO9660_DIRECTORY_RECORD
        size            rb 1
                        rb 1
        lba             rd 1
                        rd 1
        data_length     rd 1
                        rd 1
        date_time       rb 7
        flags           rb 1 ; 10b - directory  1b - hidden
                        rb 2+4
        name_len        rb 1
        name            rb 1 ;  rb [name_len]
ends

; Internal data for every ISO9660 partition.
struct ISO9660 PARTITION
       type_encoding    rd 1 ; 0 - ascii 1 - UCS-2
       lba_size         rd 1 ; default 2048 TODO
       primary_descr    rd 1 ; sector num
       root_dir_lba     rd 1 ; lba32
       root_dir_len     rd 1 ;
ends
;-----------------------------------------------------------------------------

; ISO9660 external functions
;   in:
; ebx -> parameter structure of sysfunc 70
; ebp -> ISO9660 structure
; esi -> path string in UTF-8
;   out:
; eax, ebx = return values for sysfunc 70
iglobal
align 4
iso9660_user_functions:
        dd      iso9660_free
        dd      (.end - $ - 4) / 4
        dd      iso9660_Read
        dd      iso9660_ReadFolder
        dd      0
        dd      0
        dd      0
        dd      iso9660_GetFileInfo
.end:
endg

; mount if it's a valid ISO9660 partition
iso9660_create_partition:
;   in:
; ebp -> PARTITION structure
; ebx -> boot sector
; ebx + 2048 -> buffer
; esi -> DISK structure
;   out:
; eax -> iso9660 partition structure, 0 = not iso9660
        cmp     dword [esi + DISK.MediaInfo.SectorSize], 2048 ; cd disks
        jnz     .fail_disk_sector

        push    ebx
        sub     esp, 4*3 ; locals value: primary_lba, encoding, select_sector
        mov     dword[esp], 16-1 ; sector number
        mov     eax, [esi + DISK.MediaInfo.LastSessionSector]
        add     [esp], eax
        mov     dword[esp + 4], 0 ; base encoding - ascii

        add     ebx, 2048
.new_descr:
        inc     dword[esp]
        ; read 16 sector, check header of descriptor
        xor     edx, edx
        mov     eax, [esp]
        mov     ecx, 1
        ; ebx - buffer
        ; edx:eax - num sector
        ; ebp - PARTITION
        ; ecx - number sectors
        call    fs_read64_sys
        test    eax, eax
        jnz     .err_disk_1

        cmp     dword[ebx + 1], 'CD00'
        jnz     .err_disk_1
        cmp     byte[ebx + 5], '1'
        jnz     .err_disk_1

        cmp     byte[ebx], DESCRIPTOR_TYPE_TERMANATOR
        jz      .end_scan

        cmp     byte[ebx], DESCRIPTOR_TYPE_PRIMARY
        jnz     @f

        cmp     dword[esp + 4], 0
        jnz     .new_descr

        mov     dword[esp + 4], 0 ; set UCS-2 encoding
        mov     eax, [esp]
        mov     dword[esp + 8], eax ; set lba address

        jmp     .new_descr
@@:
        cmp     byte[ebx], DESCRIPTOR_TYPE_SUPPLEMENTARY
        jnz     .new_descr

        mov     dword[esp + 4], 1 ; set UCS-2 encoding
        mov     eax, [esp]
        mov     dword[esp + 8], eax ; set lba address

        jmp     .new_descr
.end_scan:

        ; get root dir lba, root dir size, size lba
        xor     edx, edx
        mov     eax, [esp + 8]
        mov     ecx, 1
        call    fs_read64_sys
        test    eax, eax
        jnz     .err_disk_1

        ; alloc memory for ISO9660 struct
        mov     eax, sizeof.ISO9660
        call    malloc
        test    eax, eax
        jz      .err_disk_1
        ; copy data on struct
        add     esp, 4
        pop     dword[eax + ISO9660.type_encoding]
        mov     dword[eax + ISO9660.lba_size], 2048 ;TODO
        pop     dword[eax + ISO9660.primary_descr]

        mov     ecx, dword[ebx + ISO9660_PRIMARY_DESCRIPTOR.root_dir_record + ISO9660_DIRECTORY_RECORD.lba]
        mov     [eax + ISO9660.root_dir_lba], ecx

        mov     ecx, dword[ebx + ISO9660_PRIMARY_DESCRIPTOR.root_dir_record + ISO9660_DIRECTORY_RECORD.data_length]
        mov     [eax + ISO9660.root_dir_len], ecx

        push    edi esi
        mov     edi, eax
        mov     esi, ebp
        mov     ecx, sizeof.PARTITION/4
        rep movsd
        pop     esi edi
        mov     [eax + PARTITION.FSUserFunctions], iso9660_user_functions

        pop     ebx
        ret
.err_disk_1:
        add     esp, 4*3
        pop     ebx
.fail_disk_sector:
        xor     eax, eax
        ret

; IN: eax - ptr PARTITION
; OUT: -
; SAVE: esi, edi
;  Function free PARTITION struct and all object this structure
iso9660_free:
        jmp     free



;-----------------------------------------------------------------------------
; ISO9660 external functions
;   in:
; ebx -> parameter structure of sysfunc 70
; ebp -> ISO9660 structure
; esi -> path string in UTF-8
;   out:
; eax, ebx = return values for sysfunc 70
iso9660_Read:
        sub     esp, 4 ; for ptr on memory page
        call    iso9660_find_file
        mov     esi, eax
        mov     edi, [ebx + 16] ; ptr to programm buffer

        test    byte[esi + ISO9660_DIRECTORY_RECORD.flags], 10b ; check dir
        jnz     iso9660_find_file.not_found

        ; check offset (offset <= size)
        mov     edx, [ebx + 4]  ; low offset
        cmp     dword[ebx + 8], 0  ; high offset
        jnz     iso9660_find_file.bad_offset ; error offset > max size

        cmp     edx, [esi + ISO9660_DIRECTORY_RECORD.data_length]
        jae     iso9660_find_file.bad_offset ; error offset > file length

        ; good file - copy file data
        sub     esp, 4*4
        mov     dword[esp + 3*4], 0
        mov     [esp + 1*4], edx ; offset to start copy
        mov     dword[esp], 0 ; count copping byte

        ; check end offset (offset+size_buff <= size)
        mov     ecx, [esi + ISO9660_DIRECTORY_RECORD.data_length]
        mov     eax, [ebx + 12] ;size copy data - buffer size
        sub     ecx, edx
        mov     [esp + 2*4], eax  ; set count copy = buffer size
        cmp     ecx, eax           ; max copy size > buffer size
        jae     @f

        mov     [esp + 2*4], ecx
        mov     dword[esp + 3*4], ERROR_END_OF_FILE

@@:
        mov     esi, [esi + ISO9660_DIRECTORY_RECORD.lba]
        ; [esp + 4*4] = ptr temp buffer
        ; [esp+3*4] = fs err code  [esp+2*4] = size copy data
        ; [esp+1*4] = offset copy  [esp] = count copying data

.full_size:
        ; check offset mod sector_size = 0
        test    edx, not -2048
        jz      .first_align ; no creat buffer for first align

        mov     ebx, [esp + 4*4]

        ; read sector
        push    edx

        and     edx, -2048
        shr     edx, BSF 2048
        mov     eax, esi ; ISO9660_DIRECTORY_RECORD.lba
        add     eax, edx
        mov     ecx, 1
        xor     edx, edx
        ; ebx - buffer
        ; edx:eax - num sector
        ; ebp - PARTITION
        ; ecx - number sectors
        call    fs_read64_app

        pop     edx
        test    eax, eax
        jnz     .err_disk_1

        mov     ecx, edx
        neg     edx
        and     edx, not -2048
        and     ecx, not -2048
        ; create new offset
        add     dword[esp + 1*4], not -2048
        and     dword[esp + 1*4], -2048

        cmp     dword[esp + 2*4], edx  ; copy data > read in this sector
        jae     @f
        mov     edx, [esp + 2*4]
@@:
        sub     dword[esp + 2*4], edx
        add     dword[esp], edx

        ;DEBUGF  1, "K : iso c=%x d=%x Hz\n", ecx, edx
        push    esi
        ; copy (2048 - offset) and -2048
        mov     esi, ebx ; buffer
        add     esi, ecx
        mov     ecx, edx
        rep movsb
        pop     esi

        ;stdcall kernel_free, ebx
.first_align:

        mov     ecx, [esp + 2*4]
        and     ecx, -2048

        cmp     ecx, 2048
        jb      .copy_finish_block

        mov     eax, [esp + 1*4]
        shr     eax, BSF 2048
        ; copy main block
        mov     ebx, edi
        add     edi, ecx
        sub     dword[esp + 2*4], ecx
        add     dword[esp + 1*4], ecx
        add     dword[esp], ecx

        shr     ecx, BSF 2048
        xor     edx, edx
        add     eax, esi ; ISO9660_DIRECTORY_RECORD.lba
        ; ebx - buffer
        ; edx:eax - num sector
        ; ebp - PARTITION
        ; ecx - number sectors
        call    fs_read64_app
        test    eax, eax
        jnz     .err_disk

.copy_finish_block:

        cmp     dword[esp + 2*4], 0
        jz      .end_align ; creat buffer for end read sector

        mov     ebx, [esp + 4*4]

        ;copy finish block
        mov     eax, [esp + 1*4]
        shr     eax, BSF 2048
        xor     edx, edx

        mov     ecx, 1
        add     eax, esi ; ISO9660_DIRECTORY_RECORD.lba
        ; ebx - buffer
        ; edx:eax - num sector
        ; ebp - PARTITION
        ; ecx - number sectors
        call    fs_read64_app
        test    eax, eax
        jnz     .err_disk_1

        mov     esi, ebx
        mov     ecx, [esp + 2*4]
        add     dword[esp], ecx
        rep movsb

        ;stdcall kernel_free, ebx

.end_align:
        ; set ebx size copy data
        mov     ebx, [esp]
        mov     esi, [esp + 3*4]
        add     esp, 4*4
        call    kernel_free

        mov     eax, esi
        ret

.err_disk_1:
        stdcall kernel_free, ebx
.err_disk:
        add     esp, 4*4
        call    kernel_free
        xor     ebx, ebx
        mov     eax, ERROR_DEVICE
        ret

;-----------------------------------------------------------------------------
; ISO9660 external functions
;   in:
; ebx -> parameter structure of sysfunc 70
; ebp -> ISO9660 structure
; esi -> path string in UTF-8
;   out:
; eax, ebx = return values for sysfunc 70
iso9660_ReadFolder:
        sub     esp, 4 ; for ptr on memory page
        call    iso9660_find_file

        test    byte[eax + ISO9660_DIRECTORY_RECORD.flags], 10b ; check dir
        jz      iso9660_find_file.not_found

        mov     edi, [ebx + 16] ; buffer
        push    dword[ebx + 16]
        push    dword[ebx + 4]  ; first file
        push    dword[ebx + 8]  ; encoding
        push    dword 0
        push    dword 0
        push    dword[ebx + 12] ; count file
        push    dword[eax + ISO9660_DIRECTORY_RECORD.data_length]
        push    dword[eax + ISO9660_DIRECTORY_RECORD.lba]

        ; [esp] - lba   [esp + 4] - size
        ; [esp + 8] - max count
        ; [esp + 12] - counter
        ; [esp + 16] - all count files
        ; [esp + 20] - encoding
        ; [esp + 24] - first item 0..(2^32 -1)
        ; [esp + 28] - user buffer
        ; edi - user buffer

        ; set header(32 byte) in buffer
        mov     dword[edi], 1
        add     edi, 32 ;set on first item

        ; loop copy file info and name
.read_sector:
        mov     ebx, [esp + 32]
        mov     ecx, 1
        xor     edx, edx
        mov     eax, [esp]
        ; ebx - buffer
        ; edx:eax - num sector
        ; ebp - PARTITION
        ; ecx - number sectors
        call    fs_read64_sys
        test    eax, eax
        jnz     .err_disk
.new_file:
        cmp     byte[ebx], 0
        jz      .next_sector

        ; inc counter all files
        inc     dword[esp + 16]
        ; check copy
        mov     eax, [esp + 24]
        cmp     [esp + 16], eax
        jbe     .skip

        mov     eax, [esp + 12]
        cmp     [esp + 8], eax
        je      .skip

        inc     dword[esp + 12]

        mov     eax, ebx
        mov     ecx, edi
        call    iso9660_GetFileInfo.copy_file_info

        ; copy encoding
        movzx   eax, byte[esp + 20]
        mov     [edi + 4], eax

        add     edi, 40
;-----------------------------------------------------------------------------
        ; copy name
        push    ebx
        push    edi
        push    eax

        add     ebx, ISO9660_DIRECTORY_RECORD.name
        mov     esi, ebx
        mov     cl, [ebx - 1]
        cmp     cl, 1
        jne     @f
        cmp     byte[ebx], 0
        jne     @f
        mov     dword[edi], '.'
        jmp     .end_copy
@@:
        cmp     cl, byte[ebx]
        jne     @f
        mov     dword[edi], '..'
        cmp     eax, 2
        jne     .end_copy
        mov     dword[edi], ('.' shl 16) + '.'
        mov     word[edi+ 2], 0
        jmp     .end_copy
@@:
;-----------------------------------------------------------------------------
        ; copy real name
        movzx   ecx, byte[ebx - 1]
        cmp     [ebp + ISO9660.type_encoding], 0
        jnz     .ucs2
        cmp     eax, 2
        je      .ascii2utf16
@@:
        sub     ecx, 1 ; CF + ZF
        jbe     @f
        movsd
        cmp     byte[esi], ';'
        jne     @b
@@:
        mov     byte[edi], 0
        jmp     .end_copy
;-----------------------------------------------------------------------------
.ascii2utf16:
        lodsb
        call    ansi2uni_char
        stosw
        cmp     byte[esi], ';'
        je      @f
        loop    .ascii2utf16
@@:
        mov     word[edi], 0
        jmp     .end_copy
;-----------------------------------------------------------------------------
.ucs2:
        shr     ecx, 1
        cmp     eax, 1
        ja      .ucs2utf16
        ; convert ucs2 to ascii
@@:
        lodsw
        cmp     ax, 0x3B00
        je      @f
        xchg    al, ah
        call    uni2ansi_char
        stosb
        loop    @b
@@:
        mov     byte[edi], 0
        jmp     .end_copy
;-----------------------------------------------------------------------------
.ucs2utf16:
        cmp     edx, 2 ; utf16 encoding
        jne     .ucs2utf8
        ; convert ucs2 to utf16LE
@@:
        lodsw
        cmp     ax, 0x3B00
        je      @f
        xchg    al, ah
        stosw
        loop    @b
@@:
        mov     word[edi], 0
        jmp     .end_copy
;-----------------------------------------------------------------------------
.ucs2utf8:
        ; convert ucs2 to utf8
        mov     ebx, ecx
        shl     ecx, 1
@@:
        lodsw
        cmp     ax, 0x3B00
        je      @f
        xchg    ah, al
        call    UTF16to8
        dec     ebx
        jnz     @b
@@:
        mov     byte[edi], 0
;-----------------------------------------------------------------------------
.end_copy:
        pop     eax
        pop     edi
        pop     ebx
        add     edi, 264
        cmp     eax, 2
        jb      .skip
        add     edi, 520-264
.skip:
        movzx   ecx, byte[ebx]
        add     ebx, ecx

        test    ebx, 2048
        jnz     .next_sector

        mov     eax, ebx
        and     eax, not -2048
        cmp     eax, [esp + 4]
        jb      .new_file

        mov     eax, [esp + 12]
        cmp     eax, [esp + 8]
        jb      .new_file

        jmp     .end_loop

.next_sector:
        inc     dword[esp]
        sub     dword[esp + 4], 2048
        ja      .read_sector
.end_loop:
        mov     ecx, [esp + 28]
        mov     ebx, [esp + 12]
        mov     [ecx + 4], ebx
        mov     esi, [esp + 16]
        mov     [ecx + 8], esi
        mov     esi, [esp + 8] ; max count
        ; free buffer
        add     esp, 8*4
        call    kernel_free

        xor     eax, eax
        cmp     ebx, esi
        je      @f
        mov     eax, ERROR_END_OF_FILE
@@:
        ret
.err_disk:
        mov     ecx, [esp + 28]
        mov     ebx, [esp + 12]
        mov     [ecx + 4], ebx
        mov     esi, [esp + 16]
        mov     [ecx + 8], esi
        ; free buffer
        add     esp, 8*4
        call    kernel_free

        mov     eax, ERROR_DEVICE
        ret

;-----------------------------------------------------------------------------
; ISO9660 external functions
;   in:
; ebx -> parameter structure of sysfunc 70
; ebp -> ISO9660 structure
; esi -> path string in UTF-8
;   out:
; eax, ebx = return values for sysfunc 70
iso9660_GetFileInfo:
        cmp     byte[esi], 0
        jz      .rootdir

        sub     esp, 4 ; for ptr on memory page
        call    iso9660_find_file

        mov     ecx, [ebx + 16] ; buffer

        call    .copy_file_info

        call    kernel_free
        xor     eax, eax
        mov     ebx, 40
        ret

;  IN: eax -> ISO966_DIRECTORY_RECORD
;      ecx -> buffer
; destruct: edx
.copy_file_info:
         ; copy size
        mov     [ecx + 36], dword 0
        mov     edx, [eax + ISO9660_DIRECTORY_RECORD.data_length]
        mov     [ecx + 32], edx

        ; copy flags(dir of file)
        xor     edx, edx
        or      dl, 000001b
        test    byte[eax + ISO9660_DIRECTORY_RECORD.flags], 1b ; check hidden flag
        jz      @f
        or      dl, 10b
@@:
        test    byte[eax + ISO9660_DIRECTORY_RECORD.flags], 10b ; check dir
        jz      @f
        or      dl, 10000b ; dir flag
        mov     dword[ecx + 32], 0 ; size = zero
@@:
        mov     [ecx], edx

        ; copy date creat file
        movzx   edx, byte[eax + ISO9660_DIRECTORY_RECORD.date_time]
        add     edx, 1900 ; year
        shl     edx, 8

        mov     dl, byte[eax + ISO9660_DIRECTORY_RECORD.date_time + 1] ;month
        shl     edx, 8
        mov     dl, byte[eax + ISO9660_DIRECTORY_RECORD.date_time + 2] ;day

        mov     [ecx + 12], edx
        mov     [ecx + 20], edx
        mov     [ecx + 28], edx

        ; copy time creat file
        movzx   edx, byte[eax + ISO9660_DIRECTORY_RECORD.date_time + 3] ;hour
        shl     edx, 8
        mov     dl, byte[eax + ISO9660_DIRECTORY_RECORD.date_time + 4] ; minute
        shl     edx, 8
        mov     dl, byte[eax + ISO9660_DIRECTORY_RECORD.date_time + 5] ; second

        mov     [ecx + 8], edx
        mov     [ecx + 16], edx
        mov     [ecx + 24], edx

        ret

.rootdir:
        mov     edi, [ebx + 16] ; edi = buffer
        ; copy flags (dir)
        mov     byte [edi], 8
        ; copy size drive
        mov     eax, dword[ebp + PARTITION.Length+DQ.lo]
        mov     edx, dword[ebp + PARTITION.Length+DQ.hi]
        mov     ecx, [ebp + PARTITION.Disk]
        mov     ecx, [ecx + DISK.MediaInfo.SectorSize]
        bsf     ecx, ecx
        shld    edx, eax, cl
        shl     eax, cl
        mov     [edi + 32], eax  ; bdfe.size.lo
        mov     [edi + 36], edx  ; bdfe.size.hi

        mov     eax, [ebx + 8]
        ; copy encoding
        mov     [edi + 4], eax
        ; check encoding on fs struct
        test    eax, eax ; check f70s5arg.xflags
        jz      .no_name

        ; stdcall is_region_userspace, edi, ecx

        ; alloc memory for read primary descriptor
        stdcall kernel_alloc, PAGE_SIZE
        test    eax, eax
        jz      .no_memory

        push    eax
        mov     esi, eax
        ; read primary descriptor
        mov     ebx, eax
        mov     ecx, 1
        xor     edx, edx
        mov     eax, [ebp + ISO9660.primary_descr]
        ; ebx - buffer
        ; edx:eax - num sector
        ; ebp - PARTITION
        ; ecx - number sectors
        call    fs_read64_sys
        test    eax, eax
        jnz     .err_read_part

        add     esi, ISO9660_PRIMARY_DESCRIPTOR.VolumeName
        mov     edx, [edi + 4]
        add     edi, 40 ; offset partition name
        mov     ecx, 32
        call    iso9660_copy_name

        ; free memory
        call    kernel_free
.no_name:
        xor     eax, eax
        mov     ebx, 40
        ret
.err_read_part:
        call    kernel_free

        mov     eax, ERROR_DEVICE
        ret
.no_memory:
        mov     eax, TASKMAN_ERROR_OUT_OF_MEMORY
        ret


;-----------------------------------------------------------------------------

; IN: esi - ptr string UTF-8 or zero for root directory
; OUT: eax - ptr to directory record
;      [esp + 4] - ptr to memory page for destruct
iso9660_find_file:

        stdcall kernel_alloc, 4096 ;
        test    eax, eax
        jz      .err_get_memory

        mov     [esp + 4], eax

        cmp     byte[esi], 0
        jz      .rootdir

        push    ebx     ; save and buffer = esp + 8
        push    dword[ebp + ISO9660.root_dir_len]
        push    dword[ebp + ISO9660.root_dir_lba]
        ; [esp] - sector num  [esp + 4] - size dir
        ; [esp + 8] - ebx     [esp + 16] - buffer
        ; get size root dir (not record size)
.read_sector:
        ; get sector for directory
        mov     edi, [esp + 16]
        mov     ebx, [esp + 16]
        mov     ecx, 1
        xor     edx, edx
        mov     eax, [esp]
        ; ebx - buffer
        ; edx:eax - num sector
        ; ebp - PARTITION
        ; ecx - number sectors
        call    fs_read64_sys
        test    eax, eax
        jnz     .err_disk_1

        mov     ecx, [esp + 4]
.next_record:

        ; check size
        cmp     byte[edi], 0
        jz      .next_sector

        ; check file name
        call    iso9660_compare_name
        jnc     .found

        movzx   edx, byte[edi + ISO9660_DIRECTORY_RECORD.size]
        add     edi, edx

        test    edi, 2048 ;worked for allocate of page
        jnz     .next_sector

        sub     ecx, edx
        jc      .not_found_2

        jmp     .next_record

.next_sector:
        sub     dword[esp + 4], 2048
        jbe     .not_found_2
        inc     dword[esp]
        jmp     .read_sector

.found:
        ; finish name?
        cmp     byte[esi], '/'
        jne     .done

        inc     esi
        mov     edx, [edi + ISO9660_DIRECTORY_RECORD.lba]
        mov     dword[esp], edx
        mov     edx, [edi + ISO9660_DIRECTORY_RECORD.data_length]
        mov     dword[esp + 4], edx
        jmp     .read_sector
.done:
        mov     ebx, [esp + 8]
        add     esp, 4*3
        mov     eax, edi
        ret

;-----------------------------------------------------------------------------
.rootdir:
        ; read primary descriptor
        pusha
        mov     ebx, eax
        mov     ecx, 1
        xor     edx, edx
        mov     eax, [ebp + ISO9660.primary_descr]
        ; ebx - buffer
        ; edx:eax - num sector
        ; ebp - PARTITION
        ; ecx - number sectors
        call    fs_read64_sys
        test    eax, eax
        popa
        jnz     .err_disk

        mov     eax, [esp + 4]
        add     eax, ISO9660_PRIMARY_DESCRIPTOR.root_dir_record
        ret

; errors
.err_disk_1:
        ; free stack values
        mov     ebx, [esp + 8]
        add     esp, 4*3
.err_disk:
        add     esp, 4
        call    kernel_free
        xor     ebx, ebx
        mov     eax, ERROR_DEVICE
        ret

.not_found_2:
        mov     ebx, [esp + 8]
        add     esp, 4*3
.not_found_1:
        add     esp, 4
.not_found: ; [esp] - ptr to page
        call    kernel_free

        mov     eax, ERROR_FILE_NOT_FOUND
        xor     ebx, ebx
        ret

.bad_offset: ; [esp] - ptr to page
        call    kernel_free

        mov     eax, ERROR_END_OF_FILE
        xor     ebx, ebx
        ret


.err_get_memory:
        add     esp, 8   ; skip addr return and dword for ptr to buffer
.no_memory:
        mov     eax, TASKMAN_ERROR_OUT_OF_MEMORY
        xor     ebx, ebx
        ret



iso9660_compare_name:
; in: esi -> UTF-8 name, ebp -> ISO9660 struct
;     edi -> ISO9660_DIRECTORY_RECORD
; out: CF=0 -> names match, esi -> next component of name
;      CF=1 -> esi is not changed
        push    ecx eax esi edi
        add     edi, ISO9660_DIRECTORY_RECORD.name
.loop:
        call    utf8to16
        call    utf16toUpper
        mov     edx, eax

        mov     ax, [edi]

        cmp     [ebp + ISO9660.type_encoding], 0
        jnz     @f
        shl     ax, 8 ; ah=al al=0
        dec     edi
@@:
        xchg    al, ah
        call    utf16toUpper

        cmp     ax, dx
        jne     .name_not_coincide

        add     edi, 2
        cmp     byte[esi], '/' ; path separator is end of current element
        je      .done
        cmp     byte[esi], 0 ; path separator end of name
        jne     .loop
.done:
; check end of file
        cmp     [ebp + ISO9660.type_encoding], 0
        jnz     @f
        cmp     byte[edi], ';'
        je      .done_1
@@:
        cmp     [ebp + ISO9660.type_encoding], 0
        jz      @f
        cmp     word[edi], 0x3B00; separator end of file ';'
        je      .done_1
@@:
; check for files not ending with separator
;        movzx   eax, byte [ebp-33]
;        add     eax, ebp
;        sub     eax, 34
;        cmp     edi, eax
;        je      .done_1
; check the end of directory
        mov     ecx, [esp]
        movzx   eax, byte [ecx + ISO9660_DIRECTORY_RECORD.name_len]
        add     ecx, ISO9660_DIRECTORY_RECORD.name
        add     eax, ecx
        cmp     edi, eax
        jne     .name_not_coincide
.done_1:
        pop     edi eax eax ecx
        ret

.name_not_coincide:
        pop     edi esi eax ecx
        stc
        ret




;IN: ebp -> PARTITION structure
;    ebx -> FS structure
;    esi -> input string (ascii or ucs2)
;    ecx =  count bytes
;    edi -> output string (buffer)
;    edx =  encoding output string
;OUT: edi increasing
;     esi increasing
;     ecx = 0 or other value
iso9660_copy_name:
        cmp     [ebp + ISO9660.type_encoding], 0
        jnz     .ucs2

        cmp     edx, 1 ; cp866 encoding
        jne     @f
        ; no convert chars
        rep movsb
        jmp     .end_copy_name
@@:
        cmp     edx, 2 ; utf16 encoding
        jne     .ascii2utf8
        ; convert ascii to utf16LE
@@:
        lodsb
        call    ansi2uni_char
        stosw
        loop    @b
        jmp     .end_copy_name

.ascii2utf8:
        cmp     edx, 3 ; utf8 encoding
        jne     .end_copy_name
        ; convert ascii to utf8
        call    cp866toUTF8_string
        jmp     .end_copy_name
.ucs2:
        shr     ecx, 1 ; ecx/2 - ucs2 chars

        cmp     edx, 1 ; cp866 encoding
        jne     .ucs2utf16
        ; convert ucs2 to ascii
@@:
        lodsw
        xchg    al, ah
        call    uni2ansi_char
        stosb
        loop    @b

        jmp     .end_copy_name
.ucs2utf16:
        cmp     edx, 2 ; utf16 encoding
        jne     .ucs2utf8
        ; convert ucs2 to utf16LE
@@:
        lodsw
        xchg    al, ah
        stosw
        loop    @b
        jmp     .end_copy_name

.ucs2utf8:
        cmp     edx, 3 ; utf8 encoding
        jne     .end_copy_name
        ; convert ucs2 to utf8
        mov     ebx, ecx
        shl     ecx, 1
@@:
        lodsw
        xchg    ah, al
        call    UTF16to8
        dec     ebx
        jnz     @b

.end_copy_name:
        mov     byte[edi], 0
        cmp     [ebp + ISO9660.type_encoding], 0
        jz      @f
        mov     word[edi], 0
@@:
        ret

