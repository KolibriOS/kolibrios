;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Contains common resource allocation + freeing code.          ;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2014. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;---------------------------------------------------------------------
; Frees a resource (block/inode).
; Input:        eax = resource ID.
;               edi = function pointer of ext2_bg_*_bitmap form, to
;                     get bitmap of resource.
;               ecx = 0, block; 1, inode.
;               ebp = pointer to EXTFS.
; Output:       Block marked as free in block group.
;               eax = error code.
;---------------------------------------------------------------------
ext2_resource_free:
        push    ebx edx esi

        ; Get block group.
        sub     eax, [ebp + EXTFS.superblock + EXT2_SB_STRUC.first_data_block]
        xor     edx, edx
        div     [ebp + EXTFS.superblock + EXT2_SB_STRUC.blocks_per_group]
        push    eax edx

        call    edi
        test    eax, eax
        jz      .fail
        mov     esi, eax

        ; Read the bitmap.
        mov     eax, ebx
        mov     edx, eax
        mov     ebx, [ebp + EXTFS.ext2_save_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .fail

        pop     eax
        ; Mark bit free.
        call    bitmap_clear_bit
        test    eax, eax
        jz      @F

        ; No need to save anything.
        xor     eax, eax

        add     esp, 4
        jmp     .return

    @@:
        mov     eax, edx
        mov     ebx, [ebp + EXTFS.ext2_save_block]
        call    ext2_block_write
        test    eax, eax
        jnz     .fail

        ; Read the descriptor.
        mov     eax, [esp]
        call    ext2_bg_read_desc
        test    eax, eax
        jz      .fail_bg_desc_read

        lea     eax, [eax + EXT2_BLOCK_GROUP_DESC.free_blocks_count]
        shl     ecx, 1
        add     eax, ecx
        inc     word[eax]

        lea     eax, [ebp + EXTFS.superblock + EXT2_SB_STRUC.free_block_count]
        shl     ecx, 1
        add     eax, ecx
        inc     dword[eax]

        pop     eax
        call    ext2_bg_write_desc

    .return:
        pop     esi edx ebx
        ret

    .fail:
        add     esp, 4
    .fail_bg_desc_read:
        add     esp, 4
        xor     eax, eax
        not     eax
        jmp     .return

;---------------------------------------------------------------------
; Allocates a resource.
; Input:        eax = inode ID for "preference".
;               ebp = pointer to EXTFS.
;               [esp + 4], func pointer to ext2_bg_*_bitmap
;               [esp + 8], pointer to free_*_count in SB.
;               [esp + 12], *_per_group
;               [esp + 16], offset to free_*_count in bg descriptor.
;               [esp + 20], *_count
; Output:       Resource marked as set in block group.
;               eax = error code.
;               ebx = resource ID.
;---------------------------------------------------------------------
ext2_resource_alloc:
        ; Block allocation is a pretty serious area, since bad allocation
        ; can lead to fragmentation. Thus, the best way to allocate that
        ; comes to mind is to allocate around an inode as much as possible.
        ; On the other hand, this isn't about a single inode/file/directory,
        ; and focusing just around the preferred inode would lead to
        ; congestion. Thus, after much thought, the chosen allocation algorithm
        ; is to search forward, then backward.
        push    ecx edx esi edi

        cmp     dword[esp + 16 + 8], 0
        jnz     @F

        ; No free blocks.
        xor     eax, eax
        not     eax
        pop     edi esi edx ecx
        ret     20

    @@:
        ; Calculate which block group the preferred inode belongs to.
        dec     eax
        xor     edx, edx
        
        ; EAX = block group.
        div     [ebp + EXTFS.superblock + EXT2_SB_STRUC.inodes_per_group]
        push    eax
        push    eax

        mov     edi, .forward

    .test_block_group:
        call    dword[esp + 16 + 8 + 4]
        test    eax, eax
        jz      .fail
        mov     esi, eax

        mov     eax, ebx
        mov     edx, eax
        mov     ebx, [ebp + EXTFS.ext2_save_block]
        call    ext2_block_read
        test    eax, eax
        jnz     .fail

        mov     ecx, [esp + 16 + 8 + 12]
        call    ext2_find_free_bit
        cmp     eax, 0xFFFFFFFF
        jne     @F

        mov     eax, edi
        jmp     eax

    @@:
        mov     ecx, eax

        mov     eax, edx
        mov     ebx, [ebp + EXTFS.ext2_save_block]
        call    ext2_block_write
        test    eax, eax
        jnz     .fail

        ; ecx: the index of the matched entry.
        ; [esp]: block group where we found.
        ; [esp + 4]: starting block group.
        ; esi: block group descriptor.
        mov     eax, [esp]                             ; Index of block group in which we found.
        mul     dword[esp + 16 + 8 + 12]
        add     eax, ecx
        mov     ebx, eax

        mov     eax, [esp + 16 + 8 + 8]
        dec     dword[eax]

        mov     eax, esi
        add     eax, [esp + 16 + 8 + 16]
        dec     word[eax]

        pop     eax
        call    ext2_bg_write_desc

        add     esp, 4
        jmp     .return

    ; Continue forward.
    .forward:        
        inc     dword[esp]
        mov     eax, [esp]
        mul     dword[esp + 16 + 8 + 12]
        cmp     eax, [esp + 16 + 8 + 20]
        jbe     @F

        ; We need to go backward.
        mov     eax, [esp + 4]
        mov     [esp], eax
        mov     edi, .backward
        jmp     .backward

    @@:
        mov     eax, [esp]
        jmp     .test_block_group

    ; Continue backward.
    .backward:
        cmp     dword[esp], 0
        je      .fail

        dec     dword[esp]
        mov     eax, [esp]
        jmp     .test_block_group

    .return:
        pop     edi esi edx ecx
        ret     20

    .fail:
        add     esp, 8
        xor     eax, eax
        not     eax
        jmp     .return