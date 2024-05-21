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
