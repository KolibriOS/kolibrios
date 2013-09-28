;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Contains ext2 inode handling code.                           ;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved. ;;
;; Distributed under the terms of the new BSD license.          ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; KSOC_EXT2_WRITE_END_TODO: clean this up.
;---------------------------------------------------------------------
; Receives block number from extent-based inode.
; Input:        ecx = number of block in inode
;               esi = address of extent header
;               ebp = pointer to EXTFS
; Output:       ecx = address of next block, if successful
;               eax = error code (0 implies no error)
;---------------------------------------------------------------------
ext4_block_recursive_search:
        cmp     word [esi + EXT4_EXTENT_HEADER.eh_magic], 0xF30A ;EXT4_EXT_MAGIC
        jne     .fail
        
        movzx   ebx, [esi + EXT4_EXTENT_HEADER.eh_entries]
        add     esi, sizeof.EXT4_EXTENT_HEADER
        cmp     word [esi - sizeof.EXT4_EXTENT_HEADER + EXT4_EXTENT_HEADER.eh_depth], 0
        je      .leaf_block     ;листовой ли это блок?
        
        ;не листовой блок, а индексный ; eax - ext4_extent_idx
        test    ebx, ebx
        jz      .fail               ;пустой индексный блок -> ошибка

        ;цикл по индексам экстентов
    @@:
        cmp     ebx, 1              ;у индексов не хранится длина, 
        je      .end_search_index   ;поэтому, если остался последний - то это нужный
        
        cmp     ecx, [esi + EXT4_EXTENT_IDX.ei_block]
        jb      .fail
        
        cmp     ecx, [esi + sizeof.EXT4_EXTENT_IDX + EXT4_EXTENT_IDX.ei_block] ;блок слeдующего индекса
        jb      .end_search_index ;следующий дальше - значит текущий, то что нам нужен
        
        add     esi, sizeof.EXT4_EXTENT_IDX
        dec     ebx
        jmp     @B

    .end_search_index:
        ;ebp указывает на нужный extent_idx, считываем следующий блок
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        mov     eax, [esi + EXT4_EXTENT_IDX.ei_leaf_lo]
        call    ext2_block_read
        test    eax, eax
        jnz     .fail
        mov     esi, ebx
        jmp     ext4_block_recursive_search ;рекурсивно прыгаем в начало
        
    .leaf_block:    ;листовой блок esi - ext4_extent
        ;цикл по экстентам
    @@: 
        test    ebx, ebx
        jz      .fail       ;ни один узел не подошел - ошибка

        mov     edx, [esi + EXT4_EXTENT.ee_block]
        cmp     ecx, edx
        jb      .fail       ;если меньше, значит он был в предыдущих блоках -> ошибка

        movzx   edi, [esi + EXT4_EXTENT.ee_len]
        add     edx, edi
        cmp     ecx, edx
        jb      .end_search_extent     ;нашли нужный блок
        
        add     esi, sizeof.EXT4_EXTENT
        dec     ebx
        jmp     @B
        
    .end_search_extent:
        mov     edx, [esi + EXT4_EXTENT.ee_start_lo]
        sub     ecx, [esi + EXT4_EXTENT.ee_block] ;разница в ext4 блоках
        add     ecx, edx
        xor     eax, eax
        ret

    .fail:
        mov     eax, ERROR_FS_FAIL
        ret

;---------------------------------------------------------------------
; Sets block ID for indirect-addressing inode.
; Input:        ecx = index of block in inode
;               edi = block ID to set to
;               esi = address of inode
;               ebp = pointer to EXTFS.
; Output:       eax = error code (0 implies no error)
;---------------------------------------------------------------------
ext2_set_inode_block:
        push    ebx ecx edx

        ; 0 to 11: direct blocks.
        cmp     ecx, 12
        jb      .direct_block

        ; Indirect blocks
        sub     ecx, 12
        cmp     ecx, [ebp + EXTFS.count_pointer_in_block] 
        jb      .indirect_block

        ; Double indirect blocks.
        sub     ecx, [ebp + EXTFS.count_pointer_in_block]
        cmp     ecx, [ebp + EXTFS.count_pointer_in_block_square]
        jb      .double_indirect_block

        ; Triple indirect blocks.
        sub     ecx, [ebp + EXTFS.count_pointer_in_block_square]

        ; Get triply-indirect block in temp_block.
        mov     eax, [esi + EXT2_INODE_STRUC.i_block + 14*4]
        test    eax, eax
        jnz     @F

        ; TODO: fix to have correct preference.
        mov     eax, EXT2_ROOT_INO
        call    ext2_block_calloc
        test    eax, eax
        jnz     .fail_alloc

        mov     [esi + EXT2_INODE_STRUC.i_block + 14*4], ebx
        mov     eax, ebx

    @@:
        push    eax
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .fail_alloc_4

        ; Get index in triply-indirect block.
        xor     edx, edx
        mov     eax, ecx
        div     [ebp + EXTFS.count_pointer_in_block_square]

        ; eax: index in triply-indirect block, edx: index in doubly-indirect block.
        lea     ecx, [ebx + eax*4]
        mov     eax, [ebx + eax*4]
        test    eax, eax
        jnz     @F

        mov     eax, EXT2_ROOT_INO
        call    ext2_block_calloc
        test    eax, eax
        jnz     .fail_alloc_4

        mov     [ecx], ebx

        mov     eax, [esp]
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_write    
        test    eax, eax
        jnz     .fail_alloc_4

        mov     eax, [ecx]
    @@:
        mov     [esp], eax
        call    ext2_block_read
        test    eax, eax
        jnz     .fail_alloc_4

        mov     eax, edx
        jmp     @F

    .double_indirect_block:
        ; Get doubly-indirect block.
        mov     eax, [esi + EXT2_INODE_STRUC.i_block + 13*4]
        test    eax, eax
        jnz     .double_indirect_present

        ; TODO: fix to have correct preference.
        mov     eax, EXT2_ROOT_INO
        call    ext2_block_calloc
        test    eax, eax
        jnz     .fail_alloc

        mov     [esi + EXT2_INODE_STRUC.i_block + 13*4], ebx
        mov     eax, ebx

    .double_indirect_present:
        ; Save block we're at.
        push    eax

        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .fail_alloc_4

        mov     eax, ecx
    @@:
        xor     edx, edx
        div     [ebp + EXTFS.count_pointer_in_block]

        ; eax: index in doubly-indirect block, edx: index in indirect block.
        lea     ecx, [ebx + edx*4]
        push    ecx

        lea     ecx, [ebx + eax*4]
        cmp     dword[ecx], 0
        jne     @F

        ; TODO: fix to have correct preference.
        mov     eax, EXT2_ROOT_INO
        call    ext2_block_calloc
        test    eax, eax
        jnz     .fail_alloc_8

        mov     [ecx], ebx

        mov     eax, [esp + 4]
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_write    
        test    eax, eax
        jnz     .fail_alloc_8       

    @@:
        mov     eax, [ecx]
        push    eax
        call    ext2_block_read
        test    eax, eax
        jnz     .fail_alloc_12

        pop     eax
        pop     ecx
        mov     [ecx], edi
        call    ext2_block_write

        add     esp, 4
        jmp     .return

    .indirect_block:
        ; Get index of indirect block.
        mov     eax, [esi + EXT2_INODE_STRUC.i_block + 12*4]
        test    eax, eax
        jnz     @F

        ; TODO: fix to have correct preference.
        mov     eax, EXT2_ROOT_INO
        call    ext2_block_calloc
        test    eax, eax
        jnz     .fail_alloc

        mov     [esi + EXT2_INODE_STRUC.i_block + 12*4], ebx
        mov     eax, ebx
    
    @@:
        push    eax
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .fail_alloc_4
        
        ; Get the block ID.
        mov     [ebx + ecx*4], edi
        pop     eax
        call    ext2_block_write
        jmp     .return

    .direct_block:
        mov     [esi + EXT2_INODE_STRUC.i_block + ecx*4], edi
        xor     eax, eax

    .return:
        pop     edx ecx ebx
        ret

    .fail_alloc:
        xor     eax, eax
        not     eax
        jmp     .return

    .fail_alloc_12:
        add     esp, 4
    .fail_alloc_8:
        add     esp, 4
    .fail_alloc_4:
        add     esp, 4
        jmp     .fail_alloc

;---------------------------------------------------------------------
; Receives block ID from indirect-addressing inode.
; Input:        ecx = index of block in inode
;               esi = address of inode
;               ebp = pointer to EXTFS
; Output:       ecx = block ID, if successful
;               eax = error code (0 implies no error)
;---------------------------------------------------------------------
ext2_get_inode_block:
        ; If inode is extent-based, use ext4_block_recursive_search.
        test    [esi + EXT2_INODE_STRUC.i_flags], EXT2_EXTENTS_FL
        jz      @F

        pushad

        ; Get extent header in EBP.
        add     esi, EXT2_INODE_STRUC.i_block
        call    ext4_block_recursive_search
        mov     PUSHAD_ECX, ecx
        mov     PUSHAD_EAX, eax

        popad
        ret

    @@:
        ; 0 to 11: direct blocks.
        cmp     ecx, 12
        jb      .get_direct_block

        ; Indirect blocks
        sub     ecx, 12
        cmp     ecx, [ebp + EXTFS.count_pointer_in_block] 
        jb      .get_indirect_block

        ; Double indirect blocks.
        sub     ecx, [ebp + EXTFS.count_pointer_in_block]
        cmp     ecx, [ebp + EXTFS.count_pointer_in_block_square]
        jb      .get_double_indirect_block

        ; Triple indirect blocks.
        sub     ecx, [ebp + EXTFS.count_pointer_in_block_square]
        push    edx ebx

        ; Get triply-indirect block in temp_block.
        mov     eax, [esi + EXT2_INODE_STRUC.i_block + 14*4]
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .fail

        ; Get index in triply-indirect block.
        xor     edx, edx
        mov     eax, ecx
        div     [ebp + EXTFS.count_pointer_in_block_square]

        ; eax: index in triply-indirect block, edx: index in doubly-indirect block.
        mov     eax, [ebx + eax*4]
        call    ext2_block_read
        test    eax, eax
        jnz     .fail

        mov     eax, edx
        jmp     @F

    .get_double_indirect_block:
        push    edx ebx

        ; Get doubly-indirect block.
        mov     eax, [esi + EXT2_INODE_STRUC.i_block + 13*4]
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .fail

        mov     eax, ecx
    @@:
        xor     edx, edx
        div     [ebp + EXTFS.count_pointer_in_block]

        ; eax: index in doubly-indirect block, edx: index in indirect block.
        mov     eax, [ebx + eax*4]

        call    ext2_block_read
        test    eax, eax
        jnz     .fail

        mov     ecx, [ebx + edx*4]
    .fail:
        pop     ebx edx

        ret

    .get_indirect_block:
        push    ebx

        ; Get index of indirect block.
        mov     eax, [esi + EXT2_INODE_STRUC.i_block + 12*4]
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_read
        test    eax, eax
        jnz     @F
        
        ; Get the block ID.
        mov     ecx, [ebx + ecx*4]
    @@:
        pop     ebx

        ret

    .get_direct_block:
        mov     ecx, [esi + EXT2_INODE_STRUC.i_block + ecx*4]
        xor     eax, eax

        ret

;---------------------------------------------------------------------
; Get block containing inode.
; Input:        eax = inode number.
;               ebp = pointer to EXTFS.
; Output:       ebx = block (hard disk) containing inode.
;               edx = index inside block.
;               eax = error code (0 implies no error)
;---------------------------------------------------------------------
ext2_read_block_of_inode:
        pushad

        dec     eax
        xor     edx, edx
        
        ; EAX = block group.
        div     [ebp + EXTFS.superblock + EXT2_SB_STRUC.inodes_per_group]

        push    edx                             ; Index in group.

        mov     edx, 32
        mul     edx                             ; Get index of descriptor in global_desc_table.

        ; eax: inode group offset relative to global descriptor table start
        ; Find the block this block descriptor is in.
        div     [ebp + EXTFS.block_size]
        add     eax, [ebp + EXTFS.superblock + EXT2_SB_STRUC.first_data_block]
        inc     eax
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .return     

        add     ebx, edx                        ; edx: local index of descriptor inside block
        mov     eax, [ebx + EXT2_BLOCK_GROUP_DESC.inode_table]   ; Block number of inode table - in ext2 terms.
        mov     ecx, [ebp + EXTFS.log_block_size]
        shl     eax, cl

        ; eax: points to inode table on HDD.
        mov     esi, eax

        ; Add local address of inode.
        pop     eax
        mov     ecx, [ebp + EXTFS.inode_size]
        mul     ecx                             ; (index * inode_size)

        mov     ebp, 512
        div     ebp                             ; Divide by hard disk block size.

        add     eax, esi                        ; Found block to read.
        mov     ebx, eax                        ; Get it inside ebx.

        xor     eax, eax
    .return:
        mov     PUSHAD_EAX, eax
        mov     PUSHAD_EBX, ebx
        mov     PUSHAD_EDX, edx

        popad
        ret

;---------------------------------------------------------------------
; Sets content of inode by number.
; Input:        eax = inode number.
;               ebx = address from where to write inode content.
;               ebp = pointer to EXTFS.
; Output:       eax = error code (0 implies no error)
;---------------------------------------------------------------------
ext2_inode_write:
        push    edx edi esi ecx ebx
        mov     esi, ebx

        ; Ext2 actually stores time of modification of inode in ctime.
        lea     edi, [ebx + EXT2_INODE_STRUC.i_ctime]
        call    current_unix_time

        ; Get block where inode is situated.
        call    ext2_read_block_of_inode
        test    eax, eax
        jnz     .error

        mov     eax, ebx                        ; Get block into EAX.
        mov     ebx, [ebp + EXTFS.ext2_temp_block]

        mov     ecx, eax                        ; Save block.    
        call    fs_read32_sys
        test    eax, eax
        jz      @F

    .error:
        mov     eax, ERROR_DEVICE
        jmp     .return

    @@:
        mov     eax, ecx
        mov     ecx, [ebp + EXTFS.inode_size]
        mov     edi, edx                        ; The index into the block.
        add     edi, ebx
        rep movsb

        ; Write the block.
        call    fs_write32_sys

    .return:
        pop     ebx ecx esi edi edx
        ret

;---------------------------------------------------------------------
; Get content of inode by number.
; Input:        eax = inode number.
;               ebx = address where to store inode content.
;               ebp = pointer to EXTFS.
; Output:       eax = error code (0 implies no error)
;---------------------------------------------------------------------
ext2_inode_read:
        push    edx edi esi ecx ebx
        mov     edi, ebx

        ; Get block where inode is situated.
        call    ext2_read_block_of_inode
        test    eax, eax
        jnz     .error

        mov     eax, ebx                        ; Get block into EAX.
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    fs_read32_sys
        test    eax, eax
        jz      @F

    .error:
        mov     eax, ERROR_DEVICE
        jmp     .return

    @@:
        mov     ecx, [ebp + EXTFS.inode_size]
        mov     esi, edx                        ; The index into the inode.
        add     esi, ebx
        rep movsb

        xor     eax, eax
    .return:
        pop     ebx ecx esi edi edx
        ret

;---------------------------------------------------------------------
; Seek inode from the path.
; Input:        esi + [esp + 4] = name.
;               ebp = pointer to EXTFS.
; Output:       eax = error code (0 implies no error)
;               esi = inode number.
;                dl = first byte of file/folder name.
;                     [ext2_data.ext2_save_inode] stores the inode.
;---------------------------------------------------------------------
ext2_inode_find:
        mov     edx, [ebp + EXTFS.root_inode]

        ; Check for empty root.
        cmp     [edx + EXT2_INODE_STRUC.i_blocks], 0
        je      .error_empty_root

        ; Check for root.
        cmp     byte[esi], 0
        jne     .next_path_part

        push    edi ecx
        mov     esi, [ebp + EXTFS.root_inode]
        mov     edi, [ebp + EXTFS.ext2_save_inode]
        mov     ecx, [ebp + EXTFS.inode_size]
        rep movsb
        pop     ecx edi

        xor     eax, eax
        xor     dl, dl
        mov     esi, EXT2_ROOT_INO
        ret     4
    
    .next_path_part:
        push    [edx + EXT2_INODE_STRUC.i_blocks]
        xor     ecx, ecx

    .folder_block_cycle:
        push    ecx
        xchg    esi, edx
        call    ext2_get_inode_block
        xchg    esi, edx
        test    eax, eax
        jnz     .error_get_inode_block
        
        mov     eax, ecx
        mov     ebx, [ebp + EXTFS.ext2_save_block]              ; Get directory records from directory.
        call    ext2_block_read
        test    eax, eax
        jnz     .error_get_block
        
        push    esi
        push    edx
        call    ext2_block_find_parent
        pop     edx
        pop     edi ecx

        cmp     edi, esi                                        ; Did something match?
        je      .next_folder_block                              ; No, move to next block.
        
        cmp     byte [esi], 0                                   ; Reached the "end" of path successfully. 
        jnz     @F
        cmp     dword[esp + 8], 0
        je      .get_inode_ret
        mov     esi, [esp + 8]
        mov     dword[esp + 8], 0

    @@:
        mov     eax, [ebx + EXT2_DIR_STRUC.inode]
        mov     ebx, [ebp + EXTFS.ext2_save_inode]
        call    ext2_inode_read
        test    eax, eax
        jnz     .error_get_inode

        movzx   eax, [ebx + EXT2_INODE_STRUC.i_mode]
        and     eax, EXT2_S_IFMT                                ; Get the mask.
        cmp     eax, EXT2_S_IFDIR
        jne     .not_found                                      ; Matched till part, but directory entry we got doesn't point to folder.

        pop     ecx                                             ; Stack top contains number of blocks.
        mov     edx, ebx
        jmp     .next_path_part
        
    .next_folder_block:
        ; Next block in current folder.
        pop     eax                                             ; Get blocks counter.
        sub     eax, [ebp + EXTFS.count_block_in_block]
        jle     .not_found
        
        push    eax
        inc     ecx
        jmp     .folder_block_cycle

    .not_found:
        mov     eax, ERROR_FILE_NOT_FOUND
        ret     4

    .get_inode_ret:
        pop     ecx                                             ; Stack top contains number of blocks.

        mov     dl, [ebx + EXT2_DIR_STRUC.name]                 ; First character of file-name.
        mov     eax, [ebx + EXT2_DIR_STRUC.inode]
        mov     ebx, [ebp + EXTFS.ext2_save_inode]
        mov     esi, eax

        ; If we can't get the inode, eax contains the error.        
        call    ext2_inode_read
        ret     4

    .error_get_inode_block:
    .error_get_block:
        pop     ecx
    .error_get_inode:
        pop     ebx
    .error_empty_root:
        mov     eax, ERROR_FS_FAIL
        ret     4

;---------------------------------------------------------------------
; Seeks parent inode from path.
; Input:        esi = path.
;               ebp = pointer to EXTFS.
; Output:       eax = error code.
;               esi = inode.
;               edi = pointer to file name.
;---------------------------------------------------------------------
ext2_inode_find_parent:
        push    esi
        xor     edi, edi  

    .loop:
        cmp     byte[esi], '/'
        jne     @F

        mov     edi, esi
        inc     esi
        jmp     .loop

    @@:
        inc     esi
        cmp     byte[esi - 1], 0
        jne     .loop

        ; If it was just a filename (without any additional directories),
        ; use the last byte as "parent path".
        cmp     edi, 0
        jne     @F

        pop     edi
        dec     esi
        jmp     .get_inode

        ; It had some additional directories, so handle it that way.
    @@:
        mov     byte[edi], 0
        inc     edi
        pop     esi

    .get_inode:
        push    ebx edx
        stdcall ext2_inode_find, 0
        pop     edx ebx

    .return:
        ret

;---------------------------------------------------------------------
; Link an inode.
; Input:        eax = inode on which to link.
;               ebx = inode to link.
;                dl = file type.
;               esi = name.
;               ebp = pointer to EXTFS.
; Output:       eax = error code.
;---------------------------------------------------------------------
ext2_inode_link:
        push    eax
        push    esi edi ebx ecx edx

        ; Get string length, and then directory entry structure size.
        call    strlen
        add     ecx, 8

        push    esi ebx ecx

        xor     ecx, ecx
        mov     esi, [ebp + EXTFS.ext2_temp_inode]
        mov     ebx, esi

        call    ext2_inode_read
        test    eax, eax
        jnz     .error_inode_read

        ; Get the maximum addressible i_block index by (i_blocks/(2 << s_log_block_size)).
        ; Note that i_blocks contains number of reserved 512B blocks, which is why we've to
        ; find out the ext2 blocks.
        mov     eax, 2
        mov     ecx, [ebp + EXTFS.superblock + EXT2_SB_STRUC.log_block_size]
        shl     eax, cl
        mov     ecx, eax

        mov     eax, [esi + EXT2_INODE_STRUC.i_blocks]
        xor     edx, edx

        div     ecx

        ; EAX is the maximum index inside i_block we can go.
        push    eax
        push    dword 0

        ; ECX contains the "block inside i_block" index.
        xor     ecx, ecx
    @@:
        call    ext2_get_inode_block
        test    eax, eax
        jnz     .error_get_inode_block
        test    ecx, ecx
        jz      .alloc_block        ; We've got no block here, so allocate one.

        push    ecx                 ; Save block number.

        mov     eax, ecx
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .error_block_read

        ; Try to find free space in current block.
        mov     ecx, [esp + 8]
        call    ext2_block_find_fspace
        test    eax, eax
        jz      .found

        cmp     eax, 0x00000001
        jne     .next_iter

        ; This block wasn't linking to the next block, so fix that, and use the next one.
        ; Write the block.
        pop     eax
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_write    
        test    eax, eax
        jnz     .error_get_inode_block

        inc     dword [esp]
        mov     ecx, [esp]
        call    ext2_get_inode_block
        test    eax, eax
        jnz     .error_get_inode_block

        test    ecx, ecx
        jz      .alloc_block

        ; If there was a block there, prepare it for our use!
        push    ecx
        jmp     .prepare_block

    .next_iter:
        add     esp, 4

        inc     dword [esp]
        mov     ecx, [esp]
        cmp     ecx, [esp + 4]
        jbe     @B      

    .alloc_block:
        mov     eax, [esp + 12]     ; Get inode ID of what we're linking.
        call    ext2_block_calloc
        test    eax, eax
        jnz     .error_get_inode_block

        mov     ecx, [esp]          ; Get the index of it inside the inode.
        mov     edi, ebx            ; And what to set to.
        call    ext2_set_inode_block
        test    eax, eax
        jnz     .error_get_inode_block

        ; Update i_size.
        mov     eax, [ebp + EXTFS.block_size]
        add     [esi + EXT2_INODE_STRUC.i_size], eax

        ; Update i_blocks.
        mov     ecx, [ebp + EXTFS.superblock + EXT2_SB_STRUC.log_block_size]
        mov     eax, 2
        shl     eax, cl
        add     [esi + EXT2_INODE_STRUC.i_blocks], eax

        ; Write the inode.
        mov     eax, [esp + 40]
        mov     ebx, esi
        call    ext2_inode_write
        test    eax, eax
        jnz     .error_get_inode_block

        push    edi                 ; Save the block we just allocated.

    ; If we've allocated/using-old-block outside of loop, prepare it.
    .prepare_block:
        mov     eax, [esp]
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .error_block_read

        mov     edi, ebx
        mov     eax, [ebp + EXTFS.block_size]
        mov     [edi + EXT2_DIR_STRUC.rec_len], ax

    .found:
        pop     edx 
        add     esp, 8
        pop     ecx ebx esi

        push    ebx
        mov     [edi], ebx          ; Save inode.

        mov     eax, [esp + 4]      ; Get EDX off the stack -- contains the file_type.
        cmp     [ebp + EXTFS.superblock + EXT2_SB_STRUC.rev_level], EXT2_GOOD_OLD_REV
        je      .name

        ; Set the file-type.
        mov     [edi + EXT2_DIR_STRUC.file_type], al

    .name:
        ; Save name.
        sub     ecx, 8
        mov     [edi + EXT2_DIR_STRUC.name_len], cl
        add     edi, 8
        rep movsb

        ; Write block.
        mov     eax, edx
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_write
        test    eax, eax
        jnz     .error_block_write

        mov     eax, [esp]
        mov     ebx, [ebp + EXTFS.ext2_temp_inode]
        call    ext2_inode_read
        test    eax, eax
        jnz     .error_block_write

        pop     eax
        inc     [ebx + EXT2_INODE_STRUC.i_links_count]
        call    ext2_inode_write
        test    eax, eax
        jnz     .error

        xor     eax, eax
    .ret:
        pop     edx ecx ebx edi esi
        add     esp, 4
        ret

    .error_block_read:
        add     esp, 4 
    .error_get_inode_block:
        add     esp, 8
    .error_inode_read:
        add     esp, 8
    .error_block_write:
        add     esp, 4
    .error:
        xor     eax, eax
        not     eax
        jmp     .ret

;---------------------------------------------------------------------
; Unlink an inode.
; Input:        eax = inode from which to unlink.
;               ebx = inode to unlink.
;               ebp = pointer to EXTFS.
; Output:       eax = number of links to inode, after unlinking (0xFFFFFFFF implies error)
;---------------------------------------------------------------------
ext2_inode_unlink:
        push    ebx ecx edx esi edi

        push    ebx
        mov     ebx, [ebp + EXTFS.ext2_temp_inode]
        call    ext2_inode_read

        test    eax, eax
        jnz     .fail_get_inode

        ; The index into the inode block data.
        push    dword 0
        mov     esi, [ebp + EXTFS.ext2_temp_inode]
        
    .loop:
        mov     ecx, [esp]
        call    ext2_get_inode_block

        test    eax, eax
        jnz     .fail_loop
        test    ecx, ecx
        jz      .fail_loop

        mov     eax, ecx
        mov     edi, eax
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .fail_loop

    ; edi -> block.
    .first_dir_entry:
        mov     eax, [esp + 4]
        cmp     [ebx], eax
        jne     @F

        mov     dword[ebx], 0                               ; inode.
        mov     word[ebx + 6], 0                            ; name_len + file_type.
        jmp     .write_block

    @@:
        mov     edx, ebx
        add     edx, [ebp + EXTFS.block_size]
        push    edx

        mov     edx, ebx
        movzx   ecx, [ebx + EXT2_DIR_STRUC.rec_len]
        add     ebx, ecx

    .dir_entry:
        cmp     [ebx], eax
        jne     @F

        mov     cx, [ebx + EXT2_DIR_STRUC.rec_len]
        add     [edx + EXT2_DIR_STRUC.rec_len], cx
        add     esp, 4
        jmp     .write_block

    @@:
        mov     edx, ebx
        movzx   ecx, [ebx + EXT2_DIR_STRUC.rec_len]
        
        ; If it's a zero length entry, error.
        test    ecx, ecx
        jz      .fail_inode

        add     ebx, ecx

        cmp     ebx, [esp]
        jb      .dir_entry

        add     esp, 4
        inc     dword[esp]
        jmp     .loop

    .write_block:
        mov     eax, edi
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_write
        test    eax, eax
        jnz     .fail_loop

        add     esp, 4
        mov     ebx, [ebp + EXTFS.ext2_temp_inode]
        mov     eax, [esp]
        call    ext2_inode_read
        test    eax, eax
        jnz     .fail_get_inode

        dec     word[ebx + EXT2_INODE_STRUC.i_links_count]
        movzx   eax, word[ebx + EXT2_INODE_STRUC.i_links_count]
        push    eax

        mov     eax, [esp + 4]
        call    ext2_inode_write
        test    eax, eax
        jnz     .fail_loop

        pop     eax
        add     esp, 4
    .return:
        pop     edi esi edx ecx ebx
        ret

    .fail_inode:
        add     esp, 4

    .fail_loop:
        add     esp, 4

    .fail_get_inode:
        add     esp, 4

    .fail:
        xor     eax, eax
        not     eax
        jmp     .return

;---------------------------------------------------------------------
; Checks if a directory is empty.
; Input:        ebx = inode to check.
;               ebp = pointer to EXTFS.
;               [EXTFS.ext2_save_inode] = points to saved inode.
; Output:       eax = 0 signifies empty directory.
;---------------------------------------------------------------------
ext2_dir_empty:
        push    ebx ecx edx
        
        ; The index into the inode block data.
        push    dword 0
        mov     esi, [ebp + EXTFS.ext2_save_inode]
        
    .loop:
        mov     ecx, [esp]
        call    ext2_get_inode_block

        ; Treat a failure as not-empty.
        test    eax, eax
        jnz     .not_empty
        test    ecx, ecx
        jz      .empty

        mov     eax, ecx
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .not_empty

        mov     edx, ebx
        add     edx, [ebp + EXTFS.block_size]

        movzx   ecx, [ebx + EXT2_DIR_STRUC.rec_len]
        add     ebx, ecx

    .dir_entry:
        ; Process entry.
        cmp     byte[ebx + EXT2_DIR_STRUC.name_len], 1
        jne     @F

        cmp     byte[ebx + EXT2_DIR_STRUC.name], '.'
        jne     .not_empty

    @@:
        cmp     byte[ebx + EXT2_DIR_STRUC.name_len], 2
        jne     .not_empty

        cmp     word[ebx + EXT2_DIR_STRUC.name], '..'
        jne     .not_empty

    @@:
        movzx   ecx, [ebx + EXT2_DIR_STRUC.rec_len]
        add     ebx, ecx

        cmp     ebx, edx
        jb      .dir_entry

        inc     dword[esp]
        jmp     .loop

    .empty:
        xor     eax, eax
    .return:
        add     esp, 4
        pop     edx ecx ebx
        ret

    .not_empty:
        xor     eax, eax
        not     eax
        jmp     .return

;---------------------------------------------------------------------
; Gets the block group's inode bitmap.
; Input:        eax = block group.
; Output:       eax = if zero, error; else, points to block group descriptor.
;               ebx = inode bitmap's block (hard disk).
;---------------------------------------------------------------------
ext2_bg_read_inode_bitmap:
        push    ecx

        call    ext2_bg_read_desc
        test    eax, eax
        jz      .fail

        mov     ebx, [eax + EXT2_BLOCK_GROUP_DESC.inode_bitmap] ; Block number of inode bitmap - in ext2 terms.

    .return:
        pop     ecx
        ret

    .fail:
        xor     eax, eax
        jmp     .return

;---------------------------------------------------------------------
; Allocates a inode.
; Input:        eax = inode ID for "preference".
;               ebp = pointer to EXTFS.
; Output:       Inode marked as set in inode group.
;               eax = error code.
;               ebx = inode ID.
;---------------------------------------------------------------------
ext2_inode_alloc:
        push    [ebp + EXTFS.superblock + EXT2_SB_STRUC.inodes_count]
        push    EXT2_BLOCK_GROUP_DESC.free_inodes_count
        push    [ebp + EXTFS.superblock + EXT2_SB_STRUC.inodes_per_group]

        lea     ebx, [ebp + EXTFS.superblock + EXT2_SB_STRUC.free_inodes_count]
        push    ebx

        push    ext2_bg_read_inode_bitmap

        call    ext2_resource_alloc

        ; Inode table starts with 1.
        inc     ebx

        ret

;---------------------------------------------------------------------
; Frees a inode.
; Input:        eax = inode ID.
;               ebp = pointer to EXTFS.
; Output:       inode marked as free in block group.
;               eax = error code.
;---------------------------------------------------------------------
ext2_inode_free:
        push    edi ecx

        ; Inode table starts with 1.
        dec     eax

        mov     edi, ext2_bg_read_inode_bitmap
        xor     ecx, ecx
        inc     cl
        call    ext2_resource_free

        pop     ecx edi
        ret