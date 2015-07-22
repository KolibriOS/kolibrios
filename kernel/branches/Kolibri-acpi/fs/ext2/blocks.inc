;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Contains ext2 block handling code.                           ;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 5363 $


;---------------------------------------------------------------------
; Write ext2 block from memory to disk.
; Input:        eax = i_block (block number in ext2 terms);
;               ebx = buffer address
;               ebp = pointer to EXTFS
; Output:       eax = error code (0 implies no error)
;---------------------------------------------------------------------
ext2_block_write:
        push    edx ebx ecx

        mov     edx, fs_write32_sys
        jmp     ext2_block_modify

;---------------------------------------------------------------------
; Read ext2 block from disk to memory.
; Input:        eax = i_block (block number in ext2 terms);
;               ebx = address of where to read block
;               ebp = pointer to EXTFS
; Output:       eax = error code (0 implies no error)
;---------------------------------------------------------------------
ext2_block_read:
        push    edx ebx ecx
        
        mov     edx, fs_read32_sys
        jmp     ext2_block_modify      

;---------------------------------------------------------------------
; Modify ext2 block.
; Input:        eax = i_block (block number in ext2 terms);
;               ebx = I/O buffer address;
;               edx = fs_read/write32_sys
;               ebp = pointer to EXTFS
;               edx, ebx, ecx on stack.
; Output:       eax = error code (0 implies no error)
;---------------------------------------------------------------------
ext2_block_modify:
        ; Get block number in hard-disk terms in eax.
        mov     ecx, [ebp + EXTFS.log_block_size]
        shl     eax, cl
        mov     ecx, eax
        push    [ebp + EXTFS.count_block_in_block]

    @@:
        mov     eax, ecx
        call    edx
        test    eax, eax
        jnz     .fail
        
        inc     ecx
        add     ebx, 512
        dec     dword[esp]
        jnz     @B

        xor     eax, eax
    @@:
        pop     ecx
        pop     ecx ebx edx
        ret

    .fail:
        mov     eax, ERROR_DEVICE
        jmp     @B

;---------------------------------------------------------------------
; Zeroes a block.
; Input:        ebx = block ID.
;               ebp = pointer to EXTFS.
; Output:       eax = error code.
;---------------------------------------------------------------------
ext2_block_zero:
        push    ebx

        mov     eax, ebx
        mov     ebx, [ebp + EXTFS.ext2_temp_block]

        call    ext2_block_read
        test    eax, eax
        jnz     .return

        push    edi ecx
        xor     eax, eax
        mov     ecx, [ebp + EXTFS.block_size]
        mov     edi, [ebp + EXTFS.ext2_temp_block]
        rep stosb
        pop     ecx edi

        mov     eax, [esp]
        call    ext2_block_write

    .return:
        pop     ebx
        ret

;---------------------------------------------------------------------
; Allocates a block.
; Input:        eax = inode ID for "preference".
;               ebp = pointer to EXTFS.
; Output:       Block marked as set in block group.
;               eax = error code.
;               ebx = block ID.
;---------------------------------------------------------------------
ext2_block_alloc:
        push    [ebp + EXTFS.superblock + EXT2_SB_STRUC.blocks_count]
        push    EXT2_BLOCK_GROUP_DESC.free_blocks_count
        push    [ebp + EXTFS.superblock + EXT2_SB_STRUC.blocks_per_group]

        lea     ebx, [ebp + EXTFS.superblock + EXT2_SB_STRUC.free_block_count]
        push    ebx

        push    ext2_bg_read_blk_bitmap

        call    ext2_resource_alloc

        ret

;---------------------------------------------------------------------
; Zero-allocates a block.
; Input:        eax = inode ID for "preference".
;               ebp = pointer to EXTFS.
; Output:       Block marked as set in block group.
;               eax = error code.
;               ebx = block ID.
;---------------------------------------------------------------------
ext2_block_calloc:
        call    ext2_block_alloc
        test    eax, eax
        jnz     @F

        call    ext2_block_zero
    @@:
        ret

;---------------------------------------------------------------------
; Frees a block.
; Input:        eax = block ID.
;               ebp = pointer to EXTFS.
; Output:       Block marked as free in block group.
;               eax = error code.
;---------------------------------------------------------------------
ext2_block_free:
        push    edi ecx

        mov     edi, ext2_bg_read_blk_bitmap
        xor     ecx, ecx
        call    ext2_resource_free

        pop     ecx edi
        ret

;---------------------------------------------------------------------
; Find parent from file path in block.
; Input:        esi = file path.
;               ebx = pointer to directory block.
;               ebp = pointer to EXTFS structure.
; Output:       esi = name without parent, or not changed.
;               ebx = directory record matched.
;---------------------------------------------------------------------
ext2_block_find_parent:
        sub     esp, 256                      ; Space for EXT2 filename.
        mov     edx, ebx
        add     edx, [ebp + EXTFS.block_size] ; Save block end.

    .start_rec:
        cmp     [ebx + EXT2_DIR_STRUC.inode], 0
        jz      .next_rec

        mov     edi, esp
        push    esi
        movzx   ecx, [ebx + EXT2_DIR_STRUC.name_len]
        lea     esi, [ebx + EXT2_DIR_STRUC.name]
        call    utf8_to_cp866

        mov     ecx, edi
        lea     edi, [esp + 4]
        sub     ecx, edi                       ; Number of bytes in resulting string.

        mov     esi, [esp]

    ; esi: original file path.
    ; edi: converted string stored on stack.
    ; ecx: size of converted string.
    @@:
        ; If no bytes left in resulting string, test it.
        jecxz   .test_find
        dec     ecx

        lodsb
        call    char_toupper

        mov     ah, [edi]
        inc     edi
        xchg    al, ah
        call    char_toupper

        ; If both are same, check next byte.
        cmp     al, ah
        je      @B
    @@:                                         ; Doesn't match.
        pop     esi
    
    .next_rec:
        movzx   eax, [ebx + EXT2_DIR_STRUC.rec_len]
        add     ebx, eax                        ; Go to next record.
        cmp     ebx, edx                        ; Check if this is the end.
        jb      .start_rec
 
        add     esp, 256
        ret

    .test_find:
        cmp     byte [esi], 0
        je      .ret                            ; The end reached.
        cmp     byte [esi], '/'                 ; If not end of directory name, not matched.
        jne     @B
        inc     esi

    .ret:
        add     esp, 256 + 4
        ret

;---------------------------------------------------------------------
; Finds free space in a directory block, modifying last entry appropriately.
; Input:        ebp = pointer to EXTFS.
;               ecx = size of free space required.
;               [EXTFS.ext2_temp_block] contains the block relevant.
; Output:       edi = free entry.
;               rec_len of free entry is set.
;               eax = error code; if the block doesn't link to the next one, this is 0x00000001 on failure.
;                               ; else, 0xFFFFFFFF.
;---------------------------------------------------------------------
ext2_block_find_fspace:
        push    ebx edx

        mov     edi, [ebp + EXTFS.ext2_temp_block]
        mov     edx, edi
        add     edx, [ebp + EXTFS.block_size]

    @@:
        movzx   eax, [edi + EXT2_DIR_STRUC.rec_len]
        test    eax, eax
        jz      .zero_len

        cmp     [edi + EXT2_DIR_STRUC.inode], 0
        je      .unused_entry

        ; It's a used entry, so see if we can fit it between current one and next.
        ; Subtract the size used by the name and the structure from rec_len.
        movzx   ebx, [edi + EXT2_DIR_STRUC.name_len]
        add     ebx, 8 + 3
        and     ebx, 0xfffffffc                     ; Align it on the next 4-byte boundary.

        sub     eax, ebx
        add     edi, ebx
        cmp     eax, ecx
        jb      .next_iter

        sub     edi, ebx
        mov     [edi + EXT2_DIR_STRUC.rec_len], bx  ; Make previous entry point to us.
        add     edi, ebx

        mov     [edi + EXT2_DIR_STRUC.rec_len], ax  ; Make current entry point to next one.
        jmp     .found

    .unused_entry:
        ; It's an unused inode.
        cmp     eax, ecx
        jge     .found

    .next_iter:
        add     edi, eax
        cmp     edi, edx
        jb      @B

    .not_found:
        xor     eax, eax
        not     eax
        jmp     .ret

    ; Zero length entry means we have the rest of the block for us.
    .zero_len:
        mov     eax, edx
        sub     eax, edi

        ; Point to next block.
        mov     [edi + EXT2_DIR_STRUC.rec_len], ax

        cmp     eax, ecx
        jge     .fits

        mov     [edi + EXT2_DIR_STRUC.inode], 0

        ; It doesn't fit, but the block doesn't link to the next block.
        xor     eax, eax
        inc     eax
        jmp     .ret

    .fits:
        mov     [edi + EXT2_DIR_STRUC.rec_len], cx

    .found:
        xor     eax, eax

    .ret:
        pop     edx ebx
        ret

;---------------------------------------------------------------------
; Gets the block group's descriptor.
; Input:        eax = block group.
; Output:       eax = if zero, error; else, points to block group descriptor.
;               [EXTFS.ext2_temp_block] contains relevant block.
;               ebp = pointer to EXTFS.
;---------------------------------------------------------------------
ext2_bg_read_desc:
        push    edx ebx
        mov     edx, 32
        mul     edx                             ; Get index of descriptor in global_desc_table.

        ; eax: block group descriptor offset relative to global descriptor table start
        ; Find the block this block descriptor is in.
        div     [ebp + EXTFS.block_size]
        add     eax, [ebp + EXTFS.superblock + EXT2_SB_STRUC.first_data_block]
        inc     eax
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .fail     

        add     ebx, edx                        ; edx: local index of descriptor inside block
        mov     eax, ebx

    .return:
        pop     ebx edx
        ret

    .fail:
        xor     eax, eax
        jmp     .return

;---------------------------------------------------------------------
; Writes a block group's descriptor.
; Input:        eax = block group.
;               [EXTFS.ext2_temp_data] contains the block relevant.
;               ebp = pointer to EXTFS.
; Output:       eax = error code.
;---------------------------------------------------------------------
ext2_bg_write_desc:
        push    edx ebx
        mov     edx, 32
        mul     edx                             ; Get index of descriptor in global_desc_table.

        ; eax: block group descriptor offset relative to global descriptor table start
        ; Find the block this block descriptor is in.
        div     [ebp + EXTFS.block_size]
        add     eax, [ebp + EXTFS.superblock + EXT2_SB_STRUC.first_data_block]
        inc     eax
        mov     ebx, [ebp + EXTFS.ext2_temp_block]
        call    ext2_block_write

    .return:
        pop     ebx edx
        ret

;---------------------------------------------------------------------
; Gets the block group's block bitmap.
; Input:        eax = block group.
; Output:       eax = if zero, error; else, points to block group descriptor.
;               ebx = block bitmap's block (hard disk).
;---------------------------------------------------------------------
ext2_bg_read_blk_bitmap:
        push    ecx

        call    ext2_bg_read_desc
        test    eax, eax
        jz      .fail

        mov     ebx, [eax + EXT2_BLOCK_GROUP_DESC.block_bitmap] ; Block number of block group bitmap - in ext2 terms.

    .return:
        pop     ecx
        ret

    .fail:
        xor     eax, eax
        jmp     .return

;---------------------------------------------------------------------
; Updates superblock, plus backups.
; Input:        ebp = pointer to EXTFS.
; Output:       eax = error code.
;---------------------------------------------------------------------
ext2_sb_update:
        push    ebx

        mov     eax, 2
        lea     ebx, [ebp + EXTFS.superblock]
        call    fs_write32_sys

        pop     ebx
        ret
