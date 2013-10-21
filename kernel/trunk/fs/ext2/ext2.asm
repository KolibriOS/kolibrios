;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Contains ext2 initialization, plus syscall handling code.    ;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved. ;;
;; Distributed under the terms of the new BSD license.          ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include 'ext2.inc'
include 'blocks.inc'
include 'inode.inc'
include 'resource.inc'

iglobal
align 4
ext2_user_functions:
        dd      ext2_free
        dd      (ext2_user_functions_end - ext2_user_functions - 4) / 4
        dd      ext2_Read
        dd      ext2_ReadFolder
        dd      ext2_Rewrite
        dd      ext2_Write
        dd      ext2_SetFileEnd
        dd      ext2_GetFileInfo
        dd      ext2_SetFileInfo
        dd      0
        dd      ext2_Delete
        dd      ext2_CreateFolder
ext2_user_functions_end:
endg

;--------------------------------------------------------------------- 
; Locks up an ext2 partition.
; Input:        ebp = pointer to EXTFS.
;---------------------------------------------------------------------
proc ext2_lock
        lea     ecx, [ebp + EXTFS.lock]
        jmp     mutex_lock
endp

;--------------------------------------------------------------------- 
; Unlocks up an ext2 partition.
; Input:        ebp = pointer to EXTFS.
;---------------------------------------------------------------------
proc ext2_unlock
        lea     ecx, [ebp + EXTFS.lock]
        jmp     mutex_unlock
endp

;---------------------------------------------------------------------
; Check if it's a valid ext* superblock.
; Input:        ebp:       first three fields of PARTITION structure.
;               ebx + 512: points to 512-bytes buffer that can be used for anything.
; Output:       eax:       clear if can't create partition; set to EXTFS otherwise.
;---------------------------------------------------------------------
proc ext2_create_partition
        push    ebx

        mov     eax, 2                          ; Superblock starts at 1024-bytes.
        add     ebx, 512                        ; Get pointer to fs-specific buffer.
        call    fs_read32_sys
        test    eax, eax
        jnz     .fail

        ; Allowed 1KiB, 2KiB, 4KiB, 8KiB.
        cmp     [ebx + EXT2_SB_STRUC.log_block_size], 3  
        ja      .fail

        cmp     [ebx + EXT2_SB_STRUC.magic], EXT2_SUPER_MAGIC
        jne     .fail

        cmp     [ebx + EXT2_SB_STRUC.state], EXT2_VALID_FS
        jne     .fail

        ; Can't have no inodes per group.
        cmp     [ebx + EXT2_SB_STRUC.inodes_per_group], 0
        je      .fail

        ; If incompatible features required, unusable superblock.
        mov     eax, [ebx + EXT2_SB_STRUC.feature_incompat] 
        test    eax, not EXT4_FEATURE_INCOMPAT_SUPP
        jz      .setup

    .fail:
        ; Not a (valid/usable) EXT2 superblock.
        pop     ebx
        xor     eax, eax
        ret

    .setup:
        movi    eax, sizeof.EXTFS
        call    malloc
        test    eax, eax
        jz      ext2_create_partition.fail

        ; Store the first sector field. 
        mov     ecx, dword[ebp + PARTITION.FirstSector]
        mov     dword[eax + EXTFS.FirstSector], ecx
        mov     ecx, dword [ebp + PARTITION.FirstSector+4]
        mov     dword [eax + EXTFS.FirstSector+4], ecx

        ; The length field.
        mov     ecx, dword[ebp + PARTITION.Length]
        mov     dword[eax + EXTFS.Length], ecx
        mov     ecx, dword[ebp + PARTITION.Length+4]
        mov     dword[eax + EXTFS.Length+4], ecx

        ; The disk field.
        mov     ecx, [ebp + PARTITION.Disk]
        mov     [eax + EXTFS.Disk], ecx

        mov     [eax + EXTFS.FSUserFunctions], ext2_user_functions

        push    ebp esi edi

        mov     ebp, eax
        lea     ecx, [eax + EXTFS.lock]
        call    mutex_init

        ; Copy superblock from buffer to reserved memory.
        mov     esi, ebx
        lea     edi, [ebp + EXTFS.superblock]
        mov     ecx, 512/4
        rep movsd

        ; Get total groups.
        mov     eax, [ebx + EXT2_SB_STRUC.blocks_count]
        sub     eax, [ebx + EXT2_SB_STRUC.first_data_block]
        dec     eax
        xor     edx, edx
        div     [ebx + EXT2_SB_STRUC.blocks_per_group]
        inc     eax
        mov     [ebp + EXTFS.groups_count], eax

        ; Get log(block_size), such that 1,2,3,4 equ 1KiB,2KiB,4KiB,8KiB.
        mov     ecx, [ebx + EXT2_SB_STRUC.log_block_size]
        inc     ecx
        mov     [ebp + EXTFS.log_block_size], ecx

        ; 512-byte blocks in ext2 blocks.
        mov     eax, 1
        shl     eax, cl
        mov     [ebp + EXTFS.count_block_in_block], eax

        ; Get block_size/4 (we'll find square later).
        shl     eax, 7
        mov     [ebp + EXTFS.count_pointer_in_block], eax
        mov     edx, eax

        ; Get block size.
        shl     eax, 2
        mov     [ebp + EXTFS.block_size], eax

        ; Save block size for 2 kernel_alloc calls.
        push    eax eax

        mov     eax, edx
        mul     edx
        mov     [ebp + EXTFS.count_pointer_in_block_square], eax

        ; Have temporary block storage for get_inode procedure, and one for global procedure.
        KERNEL_ALLOC [ebp + EXTFS.ext2_save_block], .error
        KERNEL_ALLOC [ebp + EXTFS.ext2_temp_block], .error
        
        mov     [ebp + EXTFS.partition_flags], 0x00000000
        mov     eax, [ebx + EXT2_SB_STRUC.feature_ro_compat]
        and     eax, not EXT2_FEATURE_RO_COMPAT_SUPP
        jnz     .read_only

        mov     eax, [ebx + EXT2_SB_STRUC.feature_incompat]
        and     eax, EXT4_FEATURE_INCOMPAT_W_NOT_SUPP
        jz      @F

    .read_only:
        ; Mark as read-only.
        or      [ebp + EXTFS.partition_flags], EXT2_RO
    @@:
        mov     ecx, [ebx + EXT2_SB_STRUC.blocks_per_group]
        mov     [ebp + EXTFS.blocks_per_group], ecx

        movzx   ecx, word[ebx + EXT2_SB_STRUC.inode_size]
        mov     [ebp + EXTFS.inode_size], ecx

        ; Allocate for three inodes (loop would be overkill).
        push    ecx ecx ecx

        KERNEL_ALLOC [ebp + EXTFS.ext2_save_inode], .error
        KERNEL_ALLOC [ebp + EXTFS.ext2_temp_inode], .error
        KERNEL_ALLOC [ebp + EXTFS.root_inode], .error

        ; Read root inode.
        mov     ebx, eax
        mov     eax, EXT2_ROOT_INO
        call    ext2_inode_read

        test    eax, eax
        jnz     .error

        ;call    ext2_sb_update
        ; Sync the disk.
        ;mov     esi, [ebp + PARTITION.Disk]
        ;call    disk_sync                       ; eax contains error code, if any.

        mov     eax, ebp                        ; Return pointer to EXTFS.
        pop     edi esi ebp ebx
        ret

    ; Error in setting up.
    .error:        
        ; Free save block.
        KERNEL_FREE [ebp + EXTFS.ext2_save_block], .fail

        ; Temporary block.
        KERNEL_FREE [ebp + EXTFS.ext2_temp_block], .fail

        ; All inodes.
        KERNEL_FREE [ebp + EXTFS.ext2_save_inode], .fail
        KERNEL_FREE [ebp + EXTFS.ext2_temp_inode], .fail
        KERNEL_FREE [ebp + EXTFS.root_inode], .fail

        mov     eax, ebp
        call    free

        jmp     .fail
endp

; FUNCTIONS PROVIDED BY SYSCALLS.

;---------------------------------------------------------------------
; Frees up all ext2 structures.
; Input:        eax = pointer to EXTFS.
;---------------------------------------------------------------------
proc ext2_free
        push    ebp

        xchg    ebp, eax
        stdcall kernel_free, [ebp+EXTFS.ext2_save_block]
        stdcall kernel_free, [ebp+EXTFS.ext2_temp_block]
        stdcall kernel_free, [ebp+EXTFS.ext2_save_inode]
        stdcall kernel_free, [ebp+EXTFS.ext2_temp_inode]
        stdcall kernel_free, [ebp+EXTFS.root_inode]

        xchg    ebp, eax
        call    free

        pop     ebp
        ret
endp

;---------------------------------------------------------------------
; Read disk folder.
; Input:        ebp = pointer to EXTFS structure.
;               esi + [esp + 4] = file name.
;               ebx = pointer to parameters from sysfunc 70.
; Output:       ebx = blocks read (or 0xFFFFFFFF, folder not found)
;               eax = error code (0 implies no error)
;---------------------------------------------------------------------
ext2_ReadFolder:
        ;DEBUGF  1, "Reading folder.\n"
        call    ext2_lock
        cmp     byte [esi], 0
        jz      .root_folder

        push    ebx
        stdcall ext2_inode_find, [esp + 4 + 4]            ; Get inode.
        pop     ebx

        mov     esi, [ebp + EXTFS.ext2_save_inode]
        test    eax, eax
        jnz     .error_ret

        ; If not a directory, then return with error.
        test    [esi + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR
        jz      .error_not_found
        jmp     @F

    .root_folder:
        mov     esi, [ebp + EXTFS.root_inode]
        test    [esi + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR
        jz      .error_root

        ; Copy the inode.
        mov     edi, [ebp + EXTFS.ext2_save_inode]
        mov     ecx, [ebp + EXTFS.inode_size]
        shr     ecx, 2
        
        push    edi
        rep movsd
        pop     esi

    @@:
        cmp     [esi + EXT2_INODE_STRUC.i_size], 0      ; Folder is empty.
        je      .error_empty_dir
        
        mov     edx, [ebx + 16]
        push    edx                                     ; Result address [edi + 28].
        push    0                                       ; End of the current block in folder [edi + 24]
        push    dword[ebx + 12]                         ; Blocks to read [edi + 20]
        push    dword[ebx + 4]                          ; The first wanted file [edi + 16]
        push    dword[ebx + 8]                          ; Flags [edi + 12]
        push    0                                       ; Read files [edi + 8]
        push    0                                       ; Files in folder [edi + 4]
        push    0                                       ; Number of blocks read in dir (and current block index) [edi]

        ; Fill header with zeroes.
        mov     edi, edx
        mov     ecx, 32/4
        rep stosd
        
        mov     edi, esp                                ; edi = pointer to local variables.
        add     edx, 32                                 ; edx = mem to return.

        xor     ecx, ecx                                ; Get number of first block.
        call    ext2_inode_get_block
        test    eax, eax
        jnz     .error_get_block

        mov     eax, ecx
        mov     ebx, [ebp + EXTFS.ext2_save_block]
        call    ext2_block_read                          ; Read the block.
        test    eax, eax
        jnz     .error_get_block

        mov     eax, ebx                                ; esi: current directory record
        add     eax, [ebp + EXTFS.block_size]
        
        mov     [edi + 24], eax

        mov     ecx, [edi + 16]                         ; ecx = first wanted (flags ommited)

    .find_wanted_start:
        jecxz   .find_wanted_end
        
    .find_wanted_cycle:
        cmp     [ebx + EXT2_DIR_STRUC.inode], 0         ; Don't count unused inode in total files.
        jz      @F

        inc     dword [edi + 4]                         ; EXT2 files in folder.
        dec     ecx
    @@:
        movzx   eax, [ebx + EXT2_DIR_STRUC.rec_len]
        
        cmp     eax, 12                                 ; Minimum record length.
        jb      .error_bad_len
        test    eax, 0x3                                ; Record length must be divisible by four.
        jnz     .error_bad_len

        sub     [esi + EXT2_INODE_STRUC.i_size], eax    ; Subtract "processed record" length directly from inode.
        add     ebx, eax                                ; Go to next record.
        cmp     ebx, [edi + 24]                         ; If not reached the next block, continue.
        jb      .find_wanted_start

        push    .find_wanted_start
   .end_block:                                          ; Get the next block.
        cmp     [esi + EXT2_INODE_STRUC.i_size], 0
        jle     .end_dir

        inc     dword [edi]                             ; Number of blocks read.
        
        ; Read the next block.
        push    ecx
        mov     ecx, [edi]
        call    ext2_inode_get_block
        test    eax, eax
        jnz     .error_get_block

        mov     eax, ecx
        mov     ebx, [ebp + EXTFS.ext2_save_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .error_get_block
        pop     ecx

        mov     eax, ebx
        add     eax, [ebp + EXTFS.block_size]
        mov     [edi + 24], eax                         ; Update the end of the current block variable.  
        ret

    .wanted_end:
        loop    .find_wanted_cycle                      ; Skip files till we reach wanted one.
        
    ; First requisite file.
    .find_wanted_end:
        mov     ecx, [edi + 20]
    .wanted_start:                                      ; Look for first_wanted + count.
        jecxz   .wanted_end

        cmp     [ebx + EXT2_DIR_STRUC.inode], 0         ; if (inode == 0): not used;
        jz      .empty_rec
 
        ; Increment "files in dir" and "read files" count.
        inc     dword [edi + 8]
        inc     dword [edi + 4]

        push    edi ecx
        mov     edi, edx                                ; Zero out till the name field.
        xor     eax, eax
        mov     ecx, 40 / 4
        rep stosd
        pop     ecx edi

        push    ebx edi edx
        mov     eax, [ebx + EXT2_DIR_STRUC.inode]       ; Get the child inode.
        mov     ebx, [ebp + EXTFS.ext2_temp_inode]
        call    ext2_inode_read
        test    eax, eax
        jnz     .error_read_subinode

        lea     edi, [edx + 8]

        mov     eax, [ebx + EXT2_INODE_STRUC.i_ctime]   ; Convert time in NTFS format.
        xor     edx, edx
        add     eax, 3054539008                         ; (369 * 365 + 89) * 24 * 3600
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        mov     eax, [ebx + EXT2_INODE_STRUC.i_atime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        mov     eax, [ebx + EXT2_INODE_STRUC.i_mtime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        pop     edx
        test    [ebx + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR ; If folder, don't report size.
        jnz     @F

        mov     eax, [ebx + EXT2_INODE_STRUC.i_size]    ; Low size
        stosd
        mov     eax, [ebx + EXT2_INODE_STRUC.i_dir_acl] ; High size
        stosd

        xor     dword [edx], FS_FT_DIR                  ; Mark as file.
    @@:
        xor     dword [edx], FS_FT_DIR                  ; Mark as directory.

        ; Copy name after converting from UTF-8 to CP866.
        push    ecx esi
        mov     esi, [esp + 12]
        movzx   ecx, [esi + EXT2_DIR_STRUC.name_len]
        lea     edi, [edx + 40]
        lea     esi, [esi + EXT2_DIR_STRUC.name]
        call    utf8_to_cp866
        and     byte [edi], 0
        pop     esi ecx edi ebx

        cmp     byte [edx + 40], '.'                    ; If it begins with ".", mark it as hidden.
        jne     @F
        or      dword [edx], FS_FT_HIDDEN
    
    @@:
        add     edx, 40 + 264                           ; Go to next record.
        dec     ecx
    .empty_rec:
        movzx   eax, [ebx + EXT2_DIR_STRUC.rec_len]

        cmp     eax, 12                                 ; Illegal length.
        jb      .error_bad_len
        test    eax, 0x3                                ; Not a multiple of four.
        jnz     .error_bad_len

        sub     [esi + EXT2_INODE_STRUC.i_size], eax    ; Subtract directly from the inode.
        add     ebx, eax     
        cmp     ebx, [edi + 24]                         ; Are we at the end of the block?
        jb      .wanted_start

        push    .wanted_start 
        jmp     .end_block

    .end_dir:                                           ; End of the directory.
        call    ext2_unlock
        mov     edx, [edi + 28]                         ; Address of where to return data.
        mov     ebx, [edi + 8]                          ; EXT2_read_in_folder
        mov     ecx, [edi + 4]                          ; EXT2_files_in_folder
        mov     dword [edx], 1                          ; Version
        mov     [edx + 4], ebx
        mov     [edx + 8], ecx
        
        lea     esp, [edi + 32]
        
        xor     eax, eax                                ; Reserved in current implementation.
        lea     edi, [edx + 12]
        mov     ecx, 20 / 4
        rep stosd

        ;DEBUGF  1, "Returning with: %x.\n", eax
        ret

    .error_bad_len:
        mov     eax, ERROR_FS_FAIL

    .error_read_subinode:
    .error_get_block:
        ; Fix the stack.
        lea     esp, [edi + 32]

    .error_ret:
        or      ebx, -1
        push    eax
        call    ext2_unlock
        pop     eax
        ;DEBUGF  1, "Returning with: %x.\n", eax
        ret
        
    .error_empty_dir:                                   ; inode of folder without blocks.
    .error_root:                                        ; Root has to be a folder.
        mov     eax, ERROR_FS_FAIL
        jmp     .error_ret

    .error_not_found:                                   ; Directory not found.
        mov     eax, ERROR_FILE_NOT_FOUND
        jmp     .error_ret

;---------------------------------------------------------------------
; Read file from the hard disk.
; Input:        esi + [esp + 4] = points to file name.
;               ebx = pointer to paramteres from sysfunc 70.
;               ebp = pointer to EXTFS structure.
; Output:       ebx = bytes read (0xFFFFFFFF -> file not found)
;               eax = error code (0 implies no error)
;---------------------------------------------------------------------
ext2_Read:
        ;DEBUGF  1, "Attempting read.\n"
        call    ext2_lock
        cmp     byte [esi], 0
        jnz     @F

    .this_is_nofile:
        call    ext2_unlock
        or      ebx, -1
        mov     eax, ERROR_ACCESS_DENIED
        ret

    @@:
        push    ebx
        stdcall ext2_inode_find, [esp + 4 + 4]
        pop     ebx

        mov     esi, [ebp + EXTFS.ext2_save_inode]
        test    eax, eax
        jz      @F

        call    ext2_unlock
        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret

    @@:
        mov     ax, [esi + EXT2_INODE_STRUC.i_mode]
        and     ax, EXT2_S_IFMT                         ; Leave the file format in AX.

        ; Check if file.
        cmp     ax, EXT2_S_IFREG
        jne     .this_is_nofile

        mov     edi, [ebx + 16]
        mov     ecx, [ebx + 12]

        mov     eax, [ebx + 4]
        mov     edx, [ebx + 8]                          ; edx:eax = start byte number.

        ; Check if file is big enough for us.
        cmp     [esi + EXT2_INODE_STRUC.i_dir_acl], edx
        ja      .size_greater
        jb      .size_less

        cmp     [esi + EXT2_INODE_STRUC.i_size], eax
        ja      .size_greater

    .size_less:
        call    ext2_unlock
        xor     ebx, ebx
        mov     eax, ERROR_END_OF_FILE
        ret
        
    @@:
    .size_greater:
        add     eax, ecx                                ; Get last byte.
        adc     edx, 0

        ; Check if we've to read whole file, or till requested.
        cmp     [esi + EXT2_INODE_STRUC.i_dir_acl], edx
        ja      .read_till_requested
        jb      .read_whole_file
        cmp     [esi + EXT2_INODE_STRUC.i_size], eax
        jae     .read_till_requested

    .read_whole_file:
        push    1                                       ; Read till the end of file.
        mov     ecx, [esi + EXT2_INODE_STRUC.i_size]
        sub     ecx, [ebx + 4]                          ; To read = (size - starting byte)
        jmp     @F

    .read_till_requested:
        push    0                                       ; Read as much as requested.

    @@:
        ; ecx = bytes to read.
        ; edi = return memory
        ; [esi] = starting byte.

        push    ecx                                     ; Number of bytes to read.
        
        ; Get part of the first block.
        mov     edx, [ebx + 8]
        mov     eax, [ebx + 4]
        div     [ebp + EXTFS.block_size]

        push    eax                                     ; Save block counter to stack.
        
        push    ecx
        mov     ecx, eax
        call    ext2_inode_get_block
        test    eax, eax
        jnz     .error_at_first_block

        mov     ebx, [ebp + EXTFS.ext2_save_block]
        mov     eax, ecx
        call    ext2_block_read
        test    eax, eax
        jnz     .error_at_first_block

        pop     ecx
        ; Get index inside block.
        add     ebx, edx

        neg     edx
        add     edx, [ebp + EXTFS.block_size]          ; Get number of bytes in this block.

        ; If it's smaller than total bytes to read, then only one block.
        cmp     ecx, edx
        jbe     .only_one_block

        mov     eax, ecx
        sub     eax, edx
        mov     ecx, edx

        push    esi
        mov     esi, ebx
        rep movsb                                       ; Copy part of 1st block.
        pop     esi

        ; eax -> bytes to read.
    .calc_blocks_count:
        mov     ebx, edi                                ; Read the block in ebx.
        xor     edx, edx
        div     [ebp + EXTFS.block_size]                ; Get number of bytes in last block in edx.
        mov     edi, eax                                ; Get number of blocks in edi.

    @@:
        ; Test if all blocks are done.
        test    edi, edi
        jz      .finish_block
        
        inc     dword [esp]
        mov     ecx, [esp]
        call    ext2_inode_get_block

        test    eax, eax
        jnz     .error_at_read_cycle

        mov     eax, ecx                                ; ebx already contains desired values.
        call    ext2_block_read

        test    eax, eax
        jnz     .error_at_read_cycle

        add     ebx, [ebp + EXTFS.block_size]

        dec     edi
        jmp     @B

    ; In edx -- number of bytes in the last block.
    .finish_block:          
        test    edx, edx
        jz      .end_read

        pop     ecx                                     ; Pop block counter in ECX.
        inc     ecx
        call    ext2_inode_get_block
        
        test    eax, eax
        jnz     .error_at_finish_block

        mov     edi, ebx
        mov     eax, ecx
        mov     ebx, [ebp + EXTFS.ext2_save_block]
        call    ext2_block_read

        test    eax, eax
        jnz     .error_at_finish_block

        mov     ecx, edx
        mov     esi, ebx
        rep movsb                                       ; Copy last piece of block.
        jmp     @F

    .end_read:
        pop     ecx                                     ; Pop block counter in ECX.
    @@:
        pop     ebx                                     ; Number of bytes read.
        call    ext2_unlock
        pop     eax                                     ; If we were asked to read more, say EOF.
        test    eax, eax
        jz      @F

        mov     eax, ERROR_END_OF_FILE
        ret
    @@:
        xor     eax, eax
        ;DEBUGF  1, "Returning with: %x.\n", eax
        ret
        
    .only_one_block:
        mov     esi, ebx
        rep movsb                                       ; Copy last piece of block.
        jmp     .end_read
        
    .error_at_first_block:
        pop     edx
    .error_at_read_cycle:
        pop     ebx
    .error_at_finish_block:
        pop     ecx edx
        or      ebx, -1
        push    eax
        call    ext2_unlock
        pop     eax

        ;DEBUGF  1, "Returning with: %x.\n", eax
        ret

;---------------------------------------------------------------------
; Read file information from block device.
; Input:        esi + [esp + 4] = file name.
;               ebx = pointer to paramteres from sysfunc 70.
;               ebp = pointer to EXTFS structure.
; Output:       eax = error code.
;---------------------------------------------------------------------
ext2_GetFileInfo:
        ;DEBUGF  1, "Calling for file info, for: %s.\n", esi
        call    ext2_lock
        mov     edx, [ebx + 16]
        cmp     byte [esi], 0
        jz      .is_root

        push    edx
        stdcall ext2_inode_find, [esp + 4 + 4]
        mov     ebx, edx
        pop     edx
        
        mov     esi, [ebp + EXTFS.ext2_save_inode]
        test    eax, eax
        jz      @F

        push    eax
        call    ext2_unlock
        pop     eax
        ;DEBUGF  1, "Returning with: %x.\n", eax
        ret

    .is_root:      
        xor     ebx, ebx                                ; Clear out first char, since we don't want to set hidden flag on root.
        mov     esi, [ebp + EXTFS.root_inode]

    @@:
        xor     eax, eax
        mov     edi, edx
        mov     ecx, 40/4
        rep stosd                                       ; Zero fill buffer.

        cmp     bl, '.'
        jne     @F
        or      dword [edx], FS_FT_HIDDEN

    @@:
        test    [esi + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR
        jnz     @F                                      ; If a directory, don't put in file size.

        mov     eax, [esi + EXT2_INODE_STRUC.i_size]    ; Low file size.
        mov     ebx, [esi + EXT2_INODE_STRUC.i_dir_acl] ; High file size.
        mov     dword [edx+32], eax
        mov     dword [edx+36], ebx

        xor     dword [edx], FS_FT_DIR                  ; Next XOR will clean this, to mark it as a file.
    @@:
        xor     dword [edx], FS_FT_DIR                  ; Mark as directory.

        lea     edi, [edx + 8]

        ; Store all time.
        mov     eax, [esi + EXT2_INODE_STRUC.i_ctime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        mov     eax, [esi + EXT2_INODE_STRUC.i_atime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        mov     eax, [esi + EXT2_INODE_STRUC.i_mtime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        call    ext2_unlock
        xor     eax, eax
        ;DEBUGF  1, "Returning with: %x.\n", eax
        ret

;---------------------------------------------------------------------
; Set file information for block device.
; Input:        esi + [esp + 4] = file name.
;               ebx = pointer to paramteres from sysfunc 70.
;               ebp = pointer to EXTFS structure.
; Output:       eax = error code.
;---------------------------------------------------------------------
ext2_SetFileInfo:
        test    [ebp + EXTFS.partition_flags], EXT2_RO
        jz      @F

        mov     eax, ERROR_UNSUPPORTED_FS
        ret

    @@:    
        push    edx esi edi ebx
        call    ext2_lock
        mov     edx, [ebx + 16]

        ; Is this read-only?
        test    [ebp + EXTFS.partition_flags], EXT2_RO
        jnz     .fail

        ; Not supported for root.
        cmp     byte [esi], 0
        je      .fail

    .get_inode:
        push    edx
        stdcall ext2_inode_find, [esp + 4 + 20]
        pop     edx

        test    eax, eax
        jnz     @F

        ; Save inode number.
        push    esi
        mov     esi, [ebp + EXTFS.ext2_save_inode]

        ; From the BDFE, we ignore read-only file flags, hidden file flags;
        ; We ignore system file flags, file was archived or not.

        ; Also ignored is file creation time. ext2 stores "inode modification"
        ; time in the ctime field, which is updated by the respective inode_write
        ; procedure, and any writes on it would be overwritten anyway.

        ; Access time.
        lea     edi, [esi + EXT2_INODE_STRUC.i_atime]
        lea     esi, [edx + 16]
        call    bdfe_to_unix_time

        ; Modification time.
        add     esi, 8
        add     edi, 8
        call    bdfe_to_unix_time

        mov     ebx, [ebp + EXTFS.ext2_save_inode] ; Get address of inode into ebx.
        pop     eax                             ; Get inode number in eax.
        call    ext2_inode_write                ; eax contains error code, if any.
        test    eax, eax
        jnz     @F

        call    ext2_sb_update
        ; Sync the disk.
        mov     esi, [ebp + PARTITION.Disk]
        call    disk_sync                       ; eax contains error code, if any.

    @@:
        push    eax
        call    ext2_unlock
        pop     eax

        pop     ebx edi esi edx
        ret

    .fail:
        call    ext2_sb_update
        ; Sync the disk.
        mov     esi, [ebp + PARTITION.Disk]
        call    disk_sync                       ; eax contains error code, if any.

        mov     eax, ERROR_UNSUPPORTED_FS
        jmp     @B

;---------------------------------------------------------------------
; Set file information for block device.
; Input:        esi + [esp + 4] = file name.
;               ebx = pointer to paramteres from sysfunc 70.
;               ebp = pointer to EXTFS structure.
; Output:       eax = error code.
;---------------------------------------------------------------------
ext2_Delete:
        ;DEBUGF  1, "Attempting Delete.\n"
        test    [ebp + EXTFS.partition_flags], EXT2_RO
        jz      @F

        mov     eax, ERROR_UNSUPPORTED_FS
        ret
 
    @@:    
        push    ebx ecx edx esi edi
        call    ext2_lock

        add     esi, [esp + 20 + 4]

        ; Can't delete root.
        cmp     byte [esi], 0
        jz      .error_access_denied

        push    esi 
        stdcall ext2_inode_find, 0
        mov     ebx, esi
        pop     esi

        test    eax, eax
        jnz     .error_access_denied

        mov     edx, [ebp + EXTFS.ext2_save_inode]
        movzx   edx, [edx + EXT2_INODE_STRUC.i_mode]
        and     edx, EXT2_S_IFMT                                ; Get the mask.
        cmp     edx, EXT2_S_IFDIR
        jne     @F                                              ; If not a directory, we don't need to check if it's empty.

        call    ext2_dir_empty                                  ; 0 means directory is empty.

        test    eax, eax
        jnz     .error_access_denied

    @@:
        ; Find parent.
        call    ext2_inode_find_parent
        test    eax, eax
        jnz     .error_access_denied
        mov     eax, esi

        ; Save file/dir & parent inode.
        push    ebx eax

        cmp     edx, EXT2_S_IFDIR
        jne     @F      

        ; Unlink '.'
        mov     eax, [esp + 4]
        call    ext2_inode_unlink
        cmp     eax, 0xFFFFFFFF
        je      .error_stack8

        ; Unlink '..'
        mov     eax, [esp + 4]
        mov     ebx, [esp]
        call    ext2_inode_unlink
        cmp     eax, 0xFFFFFFFF
        je      .error_stack8

    @@:
        pop     eax
        mov     ebx, [esp]
        ; Unlink the inode.
        call    ext2_inode_unlink
        cmp     eax, 0xFFFFFFFF
        je      .error_stack4
        
        ; If hardlinks aren't zero, shouldn't completely free.
        test    eax, eax
        jz      @F

        add     esp, 4
        jmp     .disk_sync

    @@:
        ; Read the inode.
        mov     eax, [esp]
        mov     ebx, [ebp + EXTFS.ext2_save_inode]
        call    ext2_inode_read
        test    eax, eax
        jnz     .error_stack4
        
        ; Free inode data.
        mov     esi, [ebp + EXTFS.ext2_save_inode]
        xor     ecx, ecx

    @@:
        push    ecx
        call    ext2_inode_get_block
        test    eax, eax
        jnz     .error_stack8
        mov     eax, ecx
        pop     ecx

        ; If 0, we're done.
        test    eax, eax
        jz      @F

        call    ext2_block_free
        test    eax, eax
        jnz     .error_stack4

        inc     ecx
        jmp     @B

    @@:
        ; Free indirect blocks.
        call    ext2_inode_free_indirect_blocks
        test    eax, eax
        jnz     .error_stack4

        ; Clear the inode, and add deletion time.
        mov     edi, [ebp + EXTFS.ext2_save_inode]
        xor     eax, eax
        mov     ecx, [ebp + EXTFS.inode_size]
        rep stosb

        mov     edi, [ebp + EXTFS.ext2_save_inode]
        add     edi, EXT2_INODE_STRUC.i_dtime
        call    current_unix_time

        ; Write the inode.
        mov     eax, [esp]
        mov     ebx, [ebp + EXTFS.ext2_save_inode]
        call    ext2_inode_write
        test    eax, eax
        jnz     .error_stack4

        ; Check if directory.
        cmp     edx, EXT2_S_IFDIR
        jne     @F

        ; If it is, decrement used_dirs_count.

        ; Get block group.
        mov     eax, [esp]
        dec     eax
        xor     edx, edx
        div     [ebp + EXTFS.superblock + EXT2_SB_STRUC.inodes_per_group]

        push    eax
        call    ext2_bg_read_desc
        test    eax, eax
        jz      .error_stack8

        dec     [eax + EXT2_BLOCK_GROUP_DESC.used_dirs_count]
        
        pop     eax
        call    ext2_bg_write_desc

    @@:
        pop     eax
        call    ext2_inode_free
        test    eax, eax
        jnz     .error_access_denied

    .disk_sync:
        call    ext2_sb_update

        ; Sync the disk.
        mov     esi, [ebp + PARTITION.Disk]
        call    disk_sync                       ; eax contains error code, if any.

    .return:    
        push    eax
        call    ext2_unlock
        pop     eax

        pop     edi esi edx ecx ebx
        ;DEBUGF  1, "And returning with: %x.\n", eax
        ret

    .error_stack8:
        add     esp, 4
    .error_stack4:
        add     esp, 4
    .error_access_denied:
        call    ext2_sb_update

        ; Sync the disk.
        mov     esi, [ebp + PARTITION.Disk]
        call    disk_sync                       ; eax contains error code, if any.

        mov     eax, ERROR_ACCESS_DENIED
        jmp     .return

;---------------------------------------------------------------------
; Set file information for block device.
; Input:        esi + [esp + 4] = file name.
;               ebx = pointer to paramteres from sysfunc 70.
;               ebp = pointer to EXTFS structure.
; Output:       eax = error code.
;---------------------------------------------------------------------
ext2_CreateFolder:
        ;DEBUGF  1, "Attempting to create folder.\n"
        test    [ebp + EXTFS.partition_flags], EXT2_RO
        jz      @F

        mov     eax, ERROR_UNSUPPORTED_FS
        ret
 
    @@:    
        push    ebx ecx edx esi edi
        call    ext2_lock

        add     esi, [esp + 20 + 4]

        ; Can't create root, but for CreateFolder already existing directory is success.
        cmp     byte [esi], 0
        jz      .success

        push    esi 
        stdcall ext2_inode_find, 0
        pop     esi

        ; If the directory is there, we've succeeded.
        test    eax, eax
        jz      .success

        ; Find parent.
        call    ext2_inode_find_parent
        test    eax, eax
        jnz     .error

        ; Inode ID for preference.
        mov     eax, esi
        call    ext2_inode_alloc
        test    eax, eax
        jnz     .error_full

        ; Save allocated inode in EDX; filename is in EDI; parent ID in ESI.
        mov     edx, ebx

        push    edi

        xor     al, al
        mov     edi, [ebp + EXTFS.ext2_temp_inode]
        mov     ecx, [ebp + EXTFS.inode_size]
        rep stosb

        mov     edi, [ebp + EXTFS.ext2_temp_inode]
        add     edi, EXT2_INODE_STRUC.i_atime
        call    current_unix_time

        add     edi, 8
        call    current_unix_time

        pop     edi

        mov     ebx, [ebp + EXTFS.ext2_temp_inode]
        mov     [ebx + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR or PERMISSIONS
        mov     eax, edx
        call    ext2_inode_write
        test    eax, eax
        jnz     .error

        ; Link to self.
        push    edx esi

        mov     eax, edx
        mov     ebx, eax
        mov     dl, EXT2_FT_DIR
        mov     esi, self_link
        call    ext2_inode_link

        pop     esi edx

        test    eax, eax
        jnz     .error

        ; Link to parent.
        push    edx esi

        mov     eax, ebx
        mov     ebx, esi
        mov     dl, EXT2_FT_DIR
        mov     esi, parent_link
        call    ext2_inode_link

        pop     esi edx

        test    eax, eax
        jnz     .error

        ; Link parent to child.
        mov     eax, esi
        mov     ebx, edx
        mov     esi, edi
        mov     dl, EXT2_FT_DIR
        call    ext2_inode_link
        test    eax, eax
        jnz     .error

        ; Get block group descriptor for allocated inode's block.
        mov     eax, ebx
        dec     eax
        xor     edx, edx
        
        ; EAX = block group.
        div     [ebp + EXTFS.superblock + EXT2_SB_STRUC.inodes_per_group]
        mov     edx, eax

        call    ext2_bg_read_desc
        test    eax, eax
        jz      .error

        inc     [eax + EXT2_BLOCK_GROUP_DESC.used_dirs_count]
        mov     eax, edx
        call    ext2_bg_write_desc
        test    eax, eax
        jnz     .error

    .success:
        call    ext2_sb_update

        ; Sync the disk.
        mov     esi, [ebp + PARTITION.Disk]
        call    disk_sync                       ; eax contains error code, if any.

    .return:
        push    eax
        call    ext2_unlock
        pop     eax

        pop     edi esi edx ecx ebx
        ;DEBUGF  1, "Returning with: %x.\n", eax
        ret

    .error:
        call    ext2_sb_update

        ; Sync the disk.
        mov     esi, [ebp + PARTITION.Disk]
        call    disk_sync                       ; eax contains error code, if any.
        
        mov     eax, ERROR_ACCESS_DENIED
        jmp     .return

    .error_full:
        mov     eax, ERROR_DISK_FULL
        jmp     .return

self_link   db ".", 0
parent_link db "..", 0

;---------------------------------------------------------------------
; Rewrite a file.
; Input:        esi + [esp + 4] = file name.
;               ebx = pointer to paramteres from sysfunc 70.
;               ebp = pointer to EXTFS structure.
; Output:       eax = error code.
;               ebx = bytes written.
;---------------------------------------------------------------------
ext2_Rewrite:
        ;DEBUGF  1, "Attempting Rewrite.\n"
        test    [ebp + EXTFS.partition_flags], EXT2_RO
        jz      @F

        mov     eax, ERROR_UNSUPPORTED_FS
        ret
 
    @@:
        push    ecx edx esi edi
        pushad

        call    ext2_lock

        add     esi, [esp + 16 + 32 + 4]
        ; Can't create root.
        cmp     byte [esi], 0
        jz      .error_access_denied

        push    esi 
        stdcall ext2_inode_find, 0
        pop     esi

        ; If the file is there, delete it.
        test    eax, eax
        jnz     @F

        pushad

        push    eax
        call    ext2_unlock
        pop     eax

        push    dword 0x00000000
        call    ext2_Delete
        add     esp, 4

        push    eax
        call    ext2_lock
        pop     eax

        test    eax, eax
        jnz     .error_access_denied_delete

        popad
    @@:
        ; Find parent.
        call    ext2_inode_find_parent
        test    eax, eax
        jnz     .error_access_denied

        ; Inode ID for preference.
        mov     eax, esi
        call    ext2_inode_alloc
        test    eax, eax
        jnz     .error_full

        ; Save allocated inode in EDX; filename is in EDI; parent ID in ESI.
        mov     edx, ebx

        push    edi

        xor     al, al
        mov     edi, [ebp + EXTFS.ext2_temp_inode]
        mov     ecx, [ebp + EXTFS.inode_size]
        rep stosb

        mov     edi, [ebp + EXTFS.ext2_temp_inode]
        add     edi, EXT2_INODE_STRUC.i_atime
        call    current_unix_time

        add     edi, 8
        call    current_unix_time

        pop     edi

        mov     ebx, [ebp + EXTFS.ext2_temp_inode]
        mov     [ebx + EXT2_INODE_STRUC.i_mode], EXT2_S_IFREG or PERMISSIONS
        mov     eax, edx
        call    ext2_inode_write
        test    eax, eax
        jnz     .error

        ; Link parent to child.
        mov     eax, esi
        mov     ebx, edx
        mov     esi, edi
        mov     dl, EXT2_FT_REG_FILE
        call    ext2_inode_link
        test    eax, eax
        jnz     .error

        popad
        push    eax
        call    ext2_unlock
        pop     eax

        push    dword 0x00000000
        call    ext2_Write
        add     esp, 4

        push    eax
        call    ext2_lock
        pop     eax

    .success:
        push    eax
        call    ext2_sb_update

        ; Sync the disk.
        mov     esi, [ebp + PARTITION.Disk]
        call    disk_sync                       ; eax contains error code, if any.
        pop     eax

    .return:
        push    eax
        call    ext2_unlock
        pop     eax

        pop     edi esi edx ecx

        ;DEBUGF  1, "And returning with: %x.\n", eax
        ret

    .error:       
        mov     eax, ERROR_ACCESS_DENIED
        jmp     .success

    .error_access_denied_delete:
        popad

    .error_access_denied:
        popad
        xor     ebx, ebx

        mov     eax, ERROR_ACCESS_DENIED
        jmp     .return

    .error_full:
        popad
        xor     ebx, ebx

        mov     eax, ERROR_DISK_FULL
        jmp     .return

;---------------------------------------------------------------------
; Write to a file.
; Input:        esi + [esp + 4] = file name.
;               ebx = pointer to paramteres from sysfunc 70.
;               ebp = pointer to EXTFS structure.
; Output:       eax = error code.
;               ebx = number of bytes written.
;---------------------------------------------------------------------
ext2_Write:
        ;DEBUGF 1, "Attempting write, "
        test    [ebp + EXTFS.partition_flags], EXT2_RO
        jz      @F

        mov     eax, ERROR_UNSUPPORTED_FS
        ret
 
    @@:    
        push    ecx edx esi edi
        call    ext2_lock

        add     esi, [esp + 16 + 4]

        ; Can't write to root.
        cmp     byte [esi], 0
        jz      .error

        push    ebx ecx edx
        stdcall ext2_inode_find, 0
        pop     edx ecx ebx
        ; If file not there, error.
        xor     ecx, ecx
        test    eax, eax
        jnz     .error_file_not_found

        ; Save the inode.
        push    esi

        ; Check if it's a file.
        mov     edx, [ebp + EXTFS.ext2_save_inode]
        test    [edx + EXT2_INODE_STRUC.i_mode], EXT2_S_IFREG
        jz      .error

        mov     eax, esi
        mov     ecx, [ebx + 4]

        call    ext2_inode_extend
        xor     ecx, ecx
        test    eax, eax
        jnz     .error_device

        ; ECX contains the size to write, and ESI points to it.
        mov     ecx, [ebx + 0x0C]
        mov     esi, [ebx + 0x10]

        ; Save the size of the inode.
        mov     eax, [edx + EXT2_INODE_STRUC.i_size]
        push    eax

        xor     edx, edx
        div     [ebp + EXTFS.block_size]

        test    edx, edx
        jz      .start_aligned

        ; Start isn't aligned, so deal with the non-aligned bytes.
        mov     ebx, [ebp + EXTFS.block_size]
        sub     ebx, edx

        cmp     ebx, ecx
        jbe     @F

        ; If the size to copy fits in current block, limit to that, instead of the entire block.
        mov     ebx, ecx

    @@:
        ; Copy EBX bytes, in EAX indexed block.
        push    eax
        call    ext2_inode_read_entry
        test    eax, eax
        pop     eax
        jnz     .error_inode_size

        push    ecx
        
        mov     ecx, ebx
        mov     edi, ebx
        add     edi, edx
        rep movsb

        pop     ecx

        ; Write the block.
        call    ext2_inode_write_entry
        test    eax, eax
        jnz     .error_inode_size

        add     [esp], ebx
        sub     ecx, ebx
        jz      .write_inode

    .start_aligned:
        cmp     ecx, [ebp + EXTFS.block_size]
        jb      @F

        mov     eax, [esp]
        xor     edx, edx
        div     [ebp + EXTFS.block_size]

        push    eax
        mov     edx, [esp + 8]
        call    ext2_inode_blank_entry
        test    eax, eax
        pop     eax
        jnz     .error_inode_size

        push    ecx

        mov     ecx, [ebp + EXTFS.block_size]
        mov     edi, [ebp + EXTFS.ext2_save_block]
        rep movsb

        pop     ecx

        call    ext2_inode_write_entry
        test    eax, eax
        jnz     .error_inode_size

        mov     eax, [ebp + EXTFS.block_size]
        sub     ecx, eax
        add     [esp], eax
        jmp     .start_aligned        

    ; Handle the remaining bytes.
    @@:
        test    ecx, ecx
        jz      .write_inode

        mov     eax, [esp]
        xor     edx, edx
        div     [ebp + EXTFS.block_size]

        push    eax
        call    ext2_inode_read_entry
        test    eax, eax
        pop     eax
        jz      @F

        push    eax
        mov     edx, [esp + 8]

        call    ext2_inode_blank_entry
        test    eax, eax
        pop     eax
        jnz     .error_inode_size

    @@:
        push    ecx
        mov     edi, [ebp + EXTFS.ext2_save_block]
        rep movsb
        pop     ecx

        call    ext2_inode_write_entry
        test    eax, eax
        jnz     .error_inode_size

        add     [esp], ecx
        xor     ecx, ecx

    .write_inode:
        mov     ebx, [ebp + EXTFS.ext2_temp_inode]
        pop     eax
        mov     [ebx + EXT2_INODE_STRUC.i_size], eax
        mov     eax, [esp]

        call    ext2_inode_write
        test    eax, eax
        jnz     .error_device

    .success:
        call    ext2_sb_update

        ; Sync the disk.
        mov     esi, [ebp + PARTITION.Disk]
        call    disk_sync                       ; eax contains error code, if any.

    .return:
        push    eax
        call    ext2_unlock
        pop     eax

        add     esp, 4

        mov     ebx, [esp + 12]
        sub     ebx, ecx
        pop     edi esi edx ecx

        ;DEBUGF  1, "and returning with: %x.\n", eax
        ret

    .error:
        mov     eax, ERROR_ACCESS_DENIED
        jmp     .return

    .error_file_not_found:
        mov     eax, ERROR_FILE_NOT_FOUND
        jmp     .return

    .error_inode_size:
        mov     ebx, [ebp + EXTFS.ext2_temp_inode]
        pop     eax
        mov     [ebx + EXT2_INODE_STRUC.i_size], eax
        mov     eax, [esp]

        call    ext2_inode_write

    .error_device:        
        call    ext2_sb_update

        ; Sync the disk.
        mov     esi, [ebp + PARTITION.Disk]
        call    disk_sync                       ; eax contains error code, if any.

        mov     eax, ERROR_DEVICE
        jmp     .return

;---------------------------------------------------------------------
; Set the end of a file.
; Input:        esi + [esp + 4] = file name.
;               ebx = pointer to paramteres from sysfunc 70.
;               ebp = pointer to EXTFS structure.
; Output:       eax = error code.
;---------------------------------------------------------------------
ext2_SetFileEnd:
        test    [ebp + EXTFS.partition_flags], EXT2_RO
        jz      @F

        mov     eax, ERROR_UNSUPPORTED_FS
        ret
 
    @@:    
        push    ebx ecx edx esi edi
        call    ext2_lock

        add     esi, [esp + 20 + 4]

        ; Can't write to root.
        cmp     byte [esi], 0
        jz      .error

        stdcall ext2_inode_find, 0
        ; If file not there, error.
        test    eax, eax
        jnz     .error_file_not_found

        ; Check if it's a file.
        mov     edx, [ebp + EXTFS.ext2_save_inode]
        cmp     [edx + EXT2_INODE_STRUC.i_mode], EXT2_S_IFREG
        jne     .error

        mov     eax, esi
        mov     ecx, [ebx + 4]
        call    ext2_inode_extend
        test    eax, eax
        jnz     .error_disk_full

        mov     eax, esi
        call    ext2_inode_truncate
        test    eax, eax
        jnz     .error_disk_full

        mov     eax, esi
        mov     ebx, [ebp + EXTFS.ext2_temp_inode]
        call    ext2_inode_write

        call    ext2_sb_update

        ; Sync the disk.
        mov     esi, [ebp + PARTITION.Disk]
        call    disk_sync                       ; eax contains error code, if any.

    .return:
        push    eax
        call    ext2_unlock
        pop     eax

        pop     edi esi edx ecx ebx
        ret

    .error:
        mov     eax, ERROR_ACCESS_DENIED
        jmp     .return

    .error_file_not_found:
        mov     eax, ERROR_FILE_NOT_FOUND
        jmp     .return

    .error_disk_full:        
        call    ext2_sb_update

        ; Sync the disk.
        mov     esi, [ebp + PARTITION.Disk]
        call    disk_sync                       ; eax contains error code, if any.

        mov     eax, ERROR_DISK_FULL
        jmp     .return
