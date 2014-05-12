;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 4700 $

;-----------------------------------------------------------------------------
uglobal
cd_current_pointer_of_input    dd  0
cd_current_pointer_of_input_2  dd  0
cd_mem_location                dd  0
cd_counter_block               dd  0
endg
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
        mov     eax, [CURRENT_TASK]
        shl     eax, 5
        mov     eax, [eax+CURRENT_TASK+TASKDATA.pid]
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
uglobal
cd_status dd 0
endg
;-----------------------------------------------------------------------------
;
;  fs_CdRead - LFN variant for reading CD disk
;
;  esi  points to filename /dir1/dir2/.../dirn/file,0
;  ebx  pointer to 64-bit number = first wanted byte, 0+
;       may be ebx=0 - start from first byte
;  ecx  number of bytes to read, 0+
;  edx  mem location to return data
;
;  ret ebx = bytes read or 0xffffffff file not found
;      eax = 0 ok read or other = errormsg
;
;-----------------------------------------------------------------------------
fs_CdRead:
        push    edi
        cmp     byte [esi], 0
        jnz     @f
;--------------------------------------
.noaccess:
        pop     edi
;--------------------------------------
.noaccess_2:
        or      ebx, -1
        mov     eax, ERROR_ACCESS_DENIED
        ret
;--------------------------------------
.noaccess_3:
        pop     eax edx ecx edi
        jmp     .noaccess_2
;--------------------------------------
@@:
        call    cd_find_lfn
        jnc     .found

        pop     edi
        cmp     [DevErrorCode], 0
        jne     .noaccess_2

        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
;--------------------------------------
.found:
        mov     edi, [cd_current_pointer_of_input]
        test    byte [edi+25], 10b; do not allow read directories
        jnz     .noaccess

        test    ebx, ebx
        jz      .l1

        cmp     dword [ebx+4], 0
        jz      @f

        xor     ebx, ebx
;--------------------------------------
.reteof:
        mov     eax, 6; end of file
        pop     edi
        ret
;--------------------------------------
@@:
        mov     ebx, [ebx]
;--------------------------------------
.l1:
        push    ecx edx
        push    0
        mov     eax, [edi+10] ; real size of the file section
        sub     eax, ebx
        jb      .eof

        cmp     eax, ecx
        jae     @f

        mov     ecx, eax
        mov     byte [esp], 6
;--------------------------------------
@@:
        mov     eax, [edi+2]
        mov     [CDSectorAddress], eax
;--------------------------------------
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
        call    ReadCDWRetr ; read sector of file
        cmp     [DevErrorCode], 0
        jne     .noaccess_3

        add     edx, 2048
        sub     ecx, 2048
;--------------------------------------
.next:
        inc     dword [CDSectorAddress]
        jmp     .new_sector
;--------------------------------------
.incomplete_sector:
; we must read and memmove incomplete sector
        mov     [CDDataBuf_pointer], CDDataBuf
        call    ReadCDWRetr ; read sector of file
        cmp     [DevErrorCode], 0
        jne     .noaccess_3

        push    ecx
        add     ecx, ebx
        cmp     ecx, 2048
        jbe     @f

        mov     ecx, 2048
;--------------------------------------
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
;--------------------------------------
.done:
        mov     ebx, edx
        pop     eax edx ecx edi
        sub     ebx, edx
        ret
;--------------------------------------
.eof:
        mov     ebx, edx
        pop     eax edx ecx
        sub     ebx, edx
        jmp     .reteof
;-----------------------------------------------------------------------------
;
;  fs_CdReadFolder - LFN variant for reading CD disk folder
;
;  esi  points to filename  /dir1/dir2/.../dirn/file,0
;  ebx  pointer to structure 32-bit number = first wanted block, 0+
;                          & flags (bitfields)
; flags: bit 0: 0=ANSI names, 1=UNICODE names
;  ecx  number of blocks to read, 0+
;  edx  mem location to return data
;
;  ret ebx = blocks read or 0xffffffff folder not found
;      eax = 0 ok read or other = errormsg
;
;-----------------------------------------------------------------------------
fs_CdReadFolder:
        push    edi
        call    cd_find_lfn
        jnc     .found

        pop     edi
        cmp     [DevErrorCode], 0
        jne     .noaccess_1

        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
;--------------------------------------
.found:
        mov     edi, [cd_current_pointer_of_input]
        test    byte [edi+25], 10b    ; do not allow read directories
        jnz     .found_dir

        pop     edi
;--------------------------------------
.noaccess_1:
        or      ebx, -1
        mov     eax, ERROR_ACCESS_DENIED
        ret
;--------------------------------------
.found_dir:
        mov     eax, [edi+2]    ; eax=cluster
        mov     [CDSectorAddress], eax
        mov     eax, [edi+10]   ; directory size
;--------------------------------------
.doit:
; init header
        push    eax ecx
        mov     edi, edx
        mov     ecx, 32/4
        xor     eax, eax
        rep stosd
        pop     ecx eax
        mov     byte [edx], 1   ; version
        mov     [cd_mem_location], edx
        add     [cd_mem_location], 32
;.mainloop:
        mov     [cd_counter_block], dword 0
        dec     dword [CDSectorAddress]
        push    ecx
;--------------------------------------
.read_to_buffer:
        inc     dword [CDSectorAddress]
        mov     [CDDataBuf_pointer], CDDataBuf
        call    ReadCDWRetr         ; read sector of directory
        cmp     [DevErrorCode], 0
        jne     .noaccess_1

        call    .get_names_from_buffer
        sub     eax, 2048
; directory is over?
        ja      .read_to_buffer

        mov     edi, [cd_counter_block]
        mov     [edx+8], edi
        mov     edi, [ebx]
        sub     [edx+4], edi
        xor     eax, eax
        dec     ecx
        js      @f

        mov     al, ERROR_END_OF_FILE
;--------------------------------------
@@:
        pop     ecx edi
        mov     ebx, [edx+4]
        ret
;--------------------------------------
.get_names_from_buffer:
        mov     [cd_current_pointer_of_input_2], CDDataBuf
        push    eax esi edi edx
;--------------------------------------
.get_names_from_buffer_1:
        call    cd_get_name
        jc      .end_buffer

        inc     dword [cd_counter_block]
        mov     eax, [cd_counter_block]
        cmp     [ebx], eax
        jae     .get_names_from_buffer_1

        test    ecx, ecx
        jz      .get_names_from_buffer_1

        mov     edi, [cd_counter_block]
        mov     [edx+4], edi
        dec     ecx
        mov     esi, ebp
        mov     edi, [cd_mem_location]
        add     edi, 40
        test    dword [ebx+4], 1; 0=ANSI, 1=UNICODE
        jnz     .unicode
;--------------------------------------
.ansi:
        cmp     [cd_counter_block], 2
        jbe     .ansi_parent_directory

        cld
        lodsw
        xchg    ah, al
        call    uni2ansi_char
        cld
        stosb
; check end of file
        mov     ax, [esi]
        cmp     ax, word 3B00h ; separator end of file ';'
        je      .cd_get_parameters_of_file_1
; check for files not ending with separator
        movzx   eax, byte [ebp-33]
        add     eax, ebp
        sub     eax, 34
        cmp     esi, eax
        je      .cd_get_parameters_of_file_1
; check the end of the directory
        movzx   eax, byte [ebp-1]
        add     eax, ebp
        cmp     esi, eax
        jb      .ansi
;--------------------------------------
.cd_get_parameters_of_file_1:
        mov     [edi], byte 0
        call    cd_get_parameters_of_file
        add     [cd_mem_location], 304
        jmp     .get_names_from_buffer_1
;--------------------------------------
.ansi_parent_directory:
        cmp     [cd_counter_block], 2
        je      @f

        mov     [edi], byte '.'
        inc     edi
        jmp     .cd_get_parameters_of_file_1
;--------------------------------------
@@:
        mov     [edi], word '..'
        add     edi, 2
        jmp     .cd_get_parameters_of_file_1
;--------------------------------------
.unicode:
        cmp     [cd_counter_block], 2
        jbe     .unicode_parent_directory

        cld
        movsw
; check end of file
        mov     ax, [esi]
        cmp     ax, word 3B00h; separator end of file ';'
        je      .cd_get_parameters_of_file_2
; check for files not ending with separator
        movzx   eax, byte [ebp-33]
        add     eax, ebp
        sub     eax, 34
        cmp     esi, eax
        je      .cd_get_parameters_of_file_2
; check the end of the directory
        movzx   eax, byte [ebp-1]
        add     eax, ebp
        cmp     esi, eax
        jb      .unicode
;--------------------------------------
.cd_get_parameters_of_file_2:
        mov     [edi], word 0
        call    cd_get_parameters_of_file
        add     [cd_mem_location], 560
        jmp     .get_names_from_buffer_1
;--------------------------------------
.unicode_parent_directory:
        cmp     [cd_counter_block], 2
        je      @f

        mov     [edi], word 2E00h; '.'
        add     edi, 2
        jmp     .cd_get_parameters_of_file_2
;--------------------------------------
@@:
        mov     [edi], dword 2E002E00h; '..'
        add     edi, 4
        jmp     .cd_get_parameters_of_file_2
;--------------------------------------
.end_buffer:
        pop     edx edi esi eax
        ret
;-----------------------------------------------------------------------------
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
;--------------------------------------
.file:
; not as a volume label in the FAT, in this form not available
; file is not a system
        shl     eax, 3
; file is hidden? (attribute of existence)
        test    [ebp-8], byte 1
        jz      .hidden

        inc     eax
;--------------------------------------
.hidden:
        shl     eax, 1
; file is always read-only, as this CD
        inc     eax
        mov     [edi], eax
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
; get the data type of name
        xor     eax, eax
        test    dword [ebx+4], 1; 0=ANSI, 1=UNICODE
        jnz     .unicode_1

        mov     [edi+4], eax
        jmp     @f
;--------------------------------------
.unicode_1:
        inc     eax
        mov     [edi+4], eax
;--------------------------------------
@@:
; get the file size in bytes
        xor     eax, eax
        mov     [edi+32+4], eax
        mov     eax, [ebp-23]
        mov     [edi+32], eax
        ret
;-----------------------------------------------------------------------------
;
;  fs_CdGetFileInfo - LFN variant for CD
;                     get file/directory attributes structure
;
;-----------------------------------------------------------------------------
fs_CdGetFileInfo:
        cmp     byte [esi], 0
        jnz     @f

        mov     eax, 2
        ret
;--------------------------------------
@@:
        push    edi
        call    cd_find_lfn
        pushfd
        cmp     [DevErrorCode], 0
        jz      @f

        popfd
        pop     edi
        mov     eax, 11
        ret
;--------------------------------------
@@:
        popfd
        jnc     @f

        pop     edi
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
;--------------------------------------
@@:

        mov     edi, edx
        push    ebp
        mov     ebp, [cd_current_pointer_of_input]
        add     ebp, 33
        call    cd_get_parameters_of_file_1
        pop     ebp
        and     dword [edi+4], 0
        pop     edi
        xor     eax, eax
        ret
;-----------------------------------------------------------------------------
cd_find_lfn:
        mov     [cd_appl_data], 0
; in: esi+ebp -> name
; out: CF=1 - file not found
; else CF=0 and [cd_current_pointer_of_input] direntry
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

        push    ebp
        call    cd_find_name_in_buffer
        pop     ebp
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
  .nested:
        mov     eax, [cd_current_pointer_of_input]
        push    dword [eax+2]
        pop     dword [CDSectorAddress] ; beginning of the directory
        mov     eax, [eax+2+8] ; size of directory
        jmp     .mainloop
;--------------------------------------
; file pointer found
.done:
        test    ebp, ebp
        jz      @f

        mov     esi, ebp
        xor     ebp, ebp
        jmp     .nested
;--------------------------------------
@@:
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
; compares ASCIIZ-names, case-insensitive (cp866 encoding)
; in: esi->name, ebp->name
; out: if names match: ZF=1 and esi->next component of name
;      else: ZF=0, esi is not changed
; destroys eax
        push    esi eax edi
        mov     edi, ebp
;--------------------------------------
.loop:
        cld
        lodsb
        push    eax
        call    char_todown
        call    ansi2uni_char
        xchg    ah, al
        scasw
        pop     eax
        je      .coincides
        call    char_toupper
        call    ansi2uni_char
        xchg    ah, al
        sub     edi, 2
        scasw
        jne     .name_not_coincide
;--------------------------------------
.coincides:
        cmp     [esi], byte '/' ; path separator is end of current element
        je      .done

        cmp     [esi], byte 0 ; path separator end of name
        je      .done

        jmp     .loop
;--------------------------------------
.name_not_coincide:
        pop     edi eax esi
        stc
        ret
;--------------------------------------
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
;--------------------------------------
.done_1:
        pop     edi eax
        add     esp, 4
        inc     esi
        clc
        ret
;-----------------------------------------------------------------------------
char_todown:
; convert character to uppercase, using cp866 encoding
; in: al=symbol
; out: al=converted symbol
        cmp     al, 'A'
        jb      .ret

        cmp     al, 'Z'
        jbe     .az

        cmp     al, 0x80 ; 'А'
        jb      .ret

        cmp     al, 0x90 ; 'Р'
        jb      .rus1

        cmp     al, 0x9F ; 'Я'
        ja      .ret
; 0x90-0x9F -> 0xE0-0xEF
        add     al, 0xE0-0x90
;--------------------------------------
.ret:
        ret
;--------------------------------------
.rus1:
; 0x80-0x8F -> 0xA0-0xAF
.az:
        add     al, 0x20
        ret
;-----------------------------------------------------------------------------
uni2ansi_char:
; convert UNICODE character in al to ANSI character in ax, using cp866 encoding
; in: ax=UNICODE character
; out: al=converted ANSI character
        cmp     ax, 0x80
        jb      .ascii

        cmp     ax, 0x401
        jz      .yo1

        cmp     ax, 0x451
        jz      .yo2

        cmp     ax, 0x410
        jb      .unk

        cmp     ax, 0x440
        jb      .rus1

        cmp     ax, 0x450
        jb      .rus2
;--------------------------------------
.unk:
        mov     al, '_'
        jmp     .doit
;--------------------------------------
.yo1:
        mov     al, 0xF0 ; 'Ё' in cp866
        jmp     .doit
;--------------------------------------
.yo2:
        mov     al, 0xF1 ; 'ё' in cp866
        jmp     .doit
;--------------------------------------
.rus1:
; 0x410-0x43F -> 0x80-0xAF
        add     al, 0x70
        jmp     .doit
;--------------------------------------
.rus2:
; 0x440-0x44F -> 0xE0-0xEF
        add     al, 0xA0
;--------------------------------------
.ascii:
.doit:
        ret
;-----------------------------------------------------------------------------
