;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 5363 $


include 'xfs.inc'

;
; This file contains XFS related code.
; For more information on XFS check sources below.
;
; 1. XFS Filesystem Structure, 2nd Edition, Revision 1. Silicon Graphics Inc. 2006
; 2. Linux source http://kernel.org
;


; test partition type (valid XFS one?)
; alloc and fill XFS (see xfs.inc) structure
; this function is called for each partition
; returns 0 (not XFS or invalid) / pointer to partition structure
xfs_create_partition:
        push    ebx ecx edx esi edi
        cmp     dword [esi+DISK.MediaInfo.SectorSize], 512
        jnz     .error
        cmp     dword[ebx + xfs_sb.sb_magicnum], XFS_SB_MAGIC   ; signature
        jne     .error

        ; TODO: check XFS.versionnum and XFS.features2
        ;       print superblock params for debugging (waiting for bug reports)

        movi    eax, sizeof.XFS
        call    malloc
        test    eax, eax
        jz      .error

        ; standard partition initialization, common for all file systems

        mov     edi, eax
        mov     eax, dword[ebp + PARTITION.FirstSector]
        mov     dword[edi + XFS.FirstSector], eax
        mov     eax, dword[ebp + PARTITION.FirstSector + 4]
        mov     dword[edi + XFS.FirstSector + 4], eax
        mov     eax, dword[ebp + PARTITION.Length]
        mov     dword[edi + XFS.Length], eax
        mov     eax, dword[ebp + PARTITION.Length + 4]
        mov     dword[edi + XFS.Length + 4], eax
        mov     eax, [ebp + PARTITION.Disk]
        mov     [edi + XFS.Disk], eax
        mov     [edi + XFS.FSUserFunctions], xfs_user_functions

        ; here we initialize only one mutex so far (for the entire partition)
        ; XFS potentially allows parallel r/w access to several AGs, keep it in mind for SMP times

        lea     ecx, [edi + XFS.Lock]
        call    mutex_init

        ; read superblock and fill just allocated XFS partition structure

        mov     eax, [ebx + xfs_sb.sb_blocksize]
        bswap   eax                                     ; XFS is big endian
        mov     [edi + XFS.blocksize], eax
        movzx   eax, word[ebx + xfs_sb.sb_sectsize]
        xchg    al, ah
        mov     [edi + XFS.sectsize], eax
        movzx   eax, word[ebx + xfs_sb.sb_versionnum]
        xchg    al, ah
        mov     [edi + XFS.versionnum], eax
        mov     eax, [ebx + xfs_sb.sb_features2]
        bswap   eax
        mov     [edi + XFS.features2], eax
        movzx   eax, word[ebx + xfs_sb.sb_inodesize]
        xchg    al, ah
        mov     [edi + XFS.inodesize], eax
        movzx   eax, word[ebx + xfs_sb.sb_inopblock]    ; inodes per block
        xchg    al, ah
        mov     [edi + XFS.inopblock], eax
        movzx   eax, byte[ebx + xfs_sb.sb_blocklog]     ; log2 of block size, in bytes
        mov     [edi + XFS.blocklog], eax
        movzx   eax, byte[ebx + xfs_sb.sb_sectlog]
        mov     [edi + XFS.sectlog], eax
        movzx   eax, byte[ebx + xfs_sb.sb_inodelog]
        mov     [edi + XFS.inodelog], eax
        movzx   eax, byte[ebx + xfs_sb.sb_inopblog]
        mov     [edi + XFS.inopblog], eax
        movzx   eax, byte[ebx + xfs_sb.sb_dirblklog]
        mov     [edi + XFS.dirblklog], eax
        mov     eax, dword[ebx + xfs_sb.sb_rootino + 4] ;
        bswap   eax                                     ; big
        mov     dword[edi + XFS.rootino + 0], eax       ; endian
        mov     eax, dword[ebx + xfs_sb.sb_rootino + 0] ; 64bit
        bswap   eax                                     ; number
        mov     dword[edi + XFS.rootino + 4], eax       ; 

        mov     eax, [edi + XFS.blocksize]
        mov     ecx, [edi + XFS.dirblklog]
        shl     eax, cl
        mov     [edi + XFS.dirblocksize], eax           ; blocks for files, dirblocks for directories

        ; sector is always smaller than block
        ; so precalculate shift order to allow faster sector_num->block_num conversion

        mov     ecx, [edi + XFS.blocklog]
        sub     ecx, [edi + XFS.sectlog]
        mov     [edi + XFS.blockmsectlog], ecx

        mov     eax, 1
        shl     eax, cl
        mov     [edi + XFS.sectpblock], eax

        ; shift order for inode_num->block_num conversion

        mov     eax, [edi + XFS.blocklog]
        sub     eax, [edi + XFS.inodelog]
        mov     [edi + XFS.inodetoblocklog], eax

        mov     eax, [ebx + xfs_sb.sb_agblocks]
        bswap   eax
        mov     [edi + XFS.agblocks], eax
        movzx   ecx, byte[ebx + xfs_sb.sb_agblklog]
        mov     [edi + XFS.agblklog], ecx

        ; get the mask for block numbers
        ; block numbers are AG relative!
        ; bitfield length may vary between partitions

        mov     eax, 1
        shl     eax, cl
        dec     eax
        mov     dword[edi + XFS.agblockmask + 0], eax
        mov     eax, 1
        sub     ecx, 32
        jc      @f
        shl     eax, cl
    @@:
        dec     eax
        mov     dword[edi + XFS.agblockmask + 4], eax

        ; calculate magic offsets for directories

        mov     ecx, [edi + XFS.blocklog]
        mov     eax, XFS_DIR2_LEAF_OFFSET AND 0xffffffff        ; lo
        mov     edx, XFS_DIR2_LEAF_OFFSET SHR 32                ; hi
        shrd    eax, edx, cl
        mov     [edi + XFS.dir2_leaf_offset_blocks], eax

        mov     ecx, [edi + XFS.blocklog]
        mov     eax, XFS_DIR2_FREE_OFFSET AND 0xffffffff        ; lo
        mov     edx, XFS_DIR2_FREE_OFFSET SHR 32                ; hi
        shrd    eax, edx, cl
        mov     [edi + XFS.dir2_free_offset_blocks], eax

;        mov     ecx, [edi + XFS.dirblklog]
;        mov     eax, [edi + XFS.blocksize]
;        shl     eax, cl
;        mov     [edi + XFS.dirblocksize], eax

        mov     eax, [edi + XFS.blocksize]
        call    malloc
        test    eax, eax
        jz      .error
        mov     [edi + XFS.cur_block], eax

        ; we do need XFS.blocksize bytes for single inode
        ; minimal file system structure is block, inodes are packed in blocks

        mov     eax, [edi + XFS.blocksize]
        call    malloc
        test    eax, eax
        jz      .error
        mov     [edi + XFS.cur_inode], eax

        ; temporary inode
        ; used for browsing directories

        mov     eax, [edi + XFS.blocksize]
        call    malloc
        test    eax, eax
        jz      .error
        mov     [edi + XFS.tmp_inode], eax

        ; current sector
        ; only for sector size structures like AGI
        ; inodes has usually the same size, but never store them here

        mov     eax, [edi + XFS.sectsize]
        call    malloc
        test    eax, eax
        jz      .error
        mov     [edi + XFS.cur_sect], eax

        ; current directory block

        mov     eax, [edi + XFS.dirblocksize]
        call    malloc
        test    eax, eax
        jz      .error
        mov     [edi + XFS.cur_dirblock], eax

  .quit:
        mov     eax, edi                ; return pointer to allocated XFS partition structure
        pop     edi esi edx ecx ebx
        ret
  .error:
        xor     eax, eax
        pop     edi esi edx ecx ebx
        ret


iglobal
align 4
xfs_user_functions:
        dd      xfs_free
        dd      (xfs_user_functions_end - xfs_user_functions - 4) / 4
        dd      xfs_Read
        dd      xfs_ReadFolder
        dd      0;xfs_Rewrite
        dd      0;xfs_Write
        dd      0;xfs_SetFileEnd
        dd      xfs_GetFileInfo
        dd      0;xfs_SetFileInfo
        dd      0
        dd      0;xfs_Delete
        dd      0;xfs_CreateFolder
xfs_user_functions_end:
endg


; lock partition access mutex
proc xfs_lock
;DEBUGF 1,"xfs_lock\n"
        lea     ecx, [ebp + XFS.Lock]
        jmp     mutex_lock
endp


; unlock partition access mutex
proc xfs_unlock
;DEBUGF 1,"xfs_unlock\n"
        lea     ecx, [ebp + XFS.Lock]
        jmp     mutex_unlock
endp


; free all the allocated memory
; called on partition destroy
proc xfs_free
        push    ebp
        xchg    ebp, eax
        stdcall kernel_free, [ebp + XFS.cur_block]
        stdcall kernel_free, [ebp + XFS.cur_inode]
        stdcall kernel_free, [ebp + XFS.cur_sect]
        stdcall kernel_free, [ebp + XFS.cur_dirblock]
        stdcall kernel_free, [ebp + XFS.tmp_inode]
        xchg    ebp, eax
        call    free
        pop     ebp
        ret
endp


;---------------------------------------------------------------
; block number (AG relative)
; eax -- inode_lo
; edx -- inode_hi
; ebx -- buffer
;---------------------------------------------------------------
xfs_read_block:
        push    ebx esi

        push    edx
        push    eax

        ; XFS block numbers are AG relative
        ; they come in bitfield form of concatenated AG and block numbers
        ; to get absolute block number for fs_read32_sys we should
        ; 1. extract AG number (using precalculated mask)
        ; 2. multiply it by the AG size in blocks
        ; 3. add AG relative block number

        ; 1.
        mov     ecx, [ebp + XFS.agblklog]
        shrd    eax, edx, cl
        shr     edx, cl
        ; 2.
        mul     dword[ebp + XFS.agblocks]
        pop     ecx
        pop     esi
        and     ecx, dword[ebp + XFS.agblockmask + 0]
        and     esi, dword[ebp + XFS.agblockmask + 4]
        ; 3.
        add     eax, ecx
        adc     edx, esi

;DEBUGF 1,"read block: 0x%x%x\n",edx,eax
        ; there is no way to read file system block at once, therefore we
        ; 1. calculate the number of sectors first
        ; 2. and then read them in series

        ; 1.
        mov     ecx, [ebp + XFS.blockmsectlog]
        shld    edx, eax, cl
        shl     eax, cl
        mov     esi, [ebp + XFS.sectpblock]

        ; 2.
  .next_sector:
        push    eax edx
        call    fs_read32_sys
        mov     ecx, eax
        pop     edx eax
        test    ecx, ecx
        jnz     .error
        add     eax, 1                          ; be ready to fs_read64_sys
        adc     edx, 0
        add     ebx, [ebp + XFS.sectsize]       ; update buffer offset
        dec     esi
        jnz     .next_sector

  .quit:
        xor     eax, eax
        pop     esi ebx
        ret
  .error:
        mov     eax, ecx
        pop     esi ebx
        ret


;---------------------------------------------------------------
; push buffer
; push startblock_hi
; push startblock_lo
; call xfs_read_dirblock
; test eax, eax
;---------------------------------------------------------------
xfs_read_dirblock:
;mov eax, [esp + 4]
;mov edx, [esp + 8]
;DEBUGF 1,"read dirblock at: %d %d\n",edx,eax
;DEBUGF 1,"dirblklog: %d\n",[ebp + XFS.dirblklog]
        push    ebx esi

        mov     eax, [esp + 12]         ; startblock_lo
        mov     edx, [esp + 16]         ; startblock_hi
        mov     ebx, [esp + 20]         ; buffer

        ; dirblock >= block
        ; read dirblocks by blocks

        mov     ecx, [ebp + XFS.dirblklog]
        mov     esi, 1
        shl     esi, cl
  .next_block:
        push    eax edx
        call    xfs_read_block
        mov     ecx, eax
        pop     edx eax
        test    ecx, ecx
        jnz     .error
        add     eax, 1          ; be ready to fs_read64_sys
        adc     edx, 0
        add     ebx, [ebp + XFS.blocksize]
        dec     esi
        jnz     .next_block

  .quit:
        xor     eax, eax
        pop     esi ebx
        ret     12
  .error:
        mov     eax, ecx
        pop     esi ebx
        ret     12


;---------------------------------------------------------------
; push buffer
; push inode_hi
; push inode_lo
; call xfs_read_inode
; test eax, eax
;---------------------------------------------------------------
xfs_read_inode:
;DEBUGF 1,"reading inode: 0x%x%x\n",[esp+8],[esp+4]
        push    ebx
        mov     eax, [esp + 8]  ; inode_lo
        mov     edx, [esp + 12] ; inode_hi
        mov     ebx, [esp + 16] ; buffer

        ; inodes are packed into blocks
        ; 1. calculate block number
        ; 2. read the block
        ; 3. add inode offset to block base address

        ; 1.
        mov     ecx, [ebp + XFS.inodetoblocklog]
        shrd    eax, edx, cl
        shr     edx, cl
        ; 2.
        call    xfs_read_block
        test    eax, eax
        jnz     .error

        ; note that inode numbers should be first extracted from bitfields using mask

        mov     eax, [esp + 8]
        mov     edx, 1
        mov     ecx, [ebp + XFS.inopblog]
        shl     edx, cl
        dec     edx             ; get inode number mask
        and     eax, edx        ; apply mask
        mov     ecx, [ebp + XFS.inodelog]
        shl     eax, cl
        add     ebx, eax

        cmp     word[ebx], XFS_DINODE_MAGIC     ; test signature
        jne     .error
  .quit:
        xor     eax, eax
        mov     edx, ebx
        pop     ebx
        ret     12
  .error:
        movi    eax, ERROR_FS_FAIL
        mov     edx, ebx
        pop     ebx
        ret     12


;----------------------------------------------------------------
; push encoding         ; ASCII / UNICODE
; push src              ; inode
; push dst              ; bdfe
; push entries_to_read
; push start_number     ; from 0
;----------------------------------------------------------------
xfs_dir_get_bdfes:
DEBUGF 1,"xfs_dir_get_bdfes: %d entries from %d\n",[esp+8],[esp+4]
        sub     esp, 4     ; local vars
        push    ecx edx esi edi

        mov     ebx, [esp + 36]         ; src
        mov     edx, [esp + 32]         ; dst
        mov     ecx, [esp + 24]         ; start_number

        ; define directory ondisk format and jump to corresponding label

        cmp     byte[ebx + xfs_inode.di_core.di_format], XFS_DINODE_FMT_LOCAL
        jne     .not_shortdir
        jmp     .shortdir
  .not_shortdir:
        cmp     byte[ebx + xfs_inode.di_core.di_format], XFS_DINODE_FMT_EXTENTS
        jne     .not_blockdir
        mov     eax, [ebx + xfs_inode.di_core.di_nextents]
        bswap   eax
        cmp     eax, 1
        jne     .not_blockdir
        jmp     .blockdir
  .not_blockdir:
        cmp     byte[ebx + xfs_inode.di_core.di_format], XFS_DINODE_FMT_EXTENTS
        jne     .not_leafdir
        mov     eax, [ebx + xfs_inode.di_core.di_nextents]
        bswap   eax
        cmp     eax, 4
        ja      .not_leafdir
        jmp     .leafdir
  .not_leafdir:
        cmp     byte[ebx + xfs_inode.di_core.di_format], XFS_DINODE_FMT_EXTENTS
        jne     .not_nodedir
        jmp     .nodedir
  .not_nodedir:
        cmp     byte[ebx + xfs_inode.di_core.di_format], XFS_DINODE_FMT_BTREE
        jne     .not_btreedir
        jmp     .btreedir
  .not_btreedir:
        movi    eax, ERROR_FS_FAIL
        jmp     .error

        ; short form directory (all the data fits into inode)
  .shortdir:
;DEBUGF 1,"shortdir\n",
        movzx   eax, word[ebx + xfs_inode.di_u + xfs_dir2_sf_hdr.count]
        test    al, al                  ; is count zero?
        jnz     @f                      ; if not, use it (i8count must be zero then)
        shr     eax, 8                  ; use i8count
    @@:
        add     eax, 1                  ; '..' and '.' are implicit
        mov     dword[edx + 0], 1       ; version
        mov     [edx + 8], eax          ; total entries
        sub     eax, [esp + 24]         ; start number
        cmp     eax, [esp + 28]         ; entries to read
        jbe     @f
        mov     eax, [esp + 28]
    @@:
        mov     [esp + 28], eax
        mov     [edx + 4], eax          ; number of actually read entries
        mov     [ebp + XFS.entries_read], eax

        ; inode numbers are often saved as 4 bytes (iff they fit)
        ; compute the length of inode numbers

        mov     eax, 4          ; 4 by default
        cmp     byte[ebx + xfs_inode.di_u + xfs_dir2_sf_hdr.i8count], 0
        je      @f
        add     eax, eax        ; 4+4=8, iff i8count != 0
    @@:
        mov     dword[edx + 12], 0      ; reserved
        mov     dword[edx + 16], 0      ; 
        mov     dword[edx + 20], 0      ; 
        mov     dword[edx + 24], 0      ; 
        mov     dword[edx + 28], 0      ; 
        add     edx, 32
        lea     esi, [ebx + xfs_inode.di_u + xfs_dir2_sf_hdr.parent + eax]
        dec     ecx
        js      .shortdir.fill

        ; skip some entries if the first entry to read is not 0

  .shortdir.skip:
        test    ecx, ecx
        jz      .shortdir.skipped
        movzx   edi, byte[esi + xfs_dir2_sf_entry.namelen]
        lea     esi, [esi + xfs_dir2_sf_entry.name + edi]
        add     esi, eax
        dec     ecx
        jnz     .shortdir.skip
        mov     ecx, [esp + 28]         ; entries to read
        jmp     .shortdir.skipped
  .shortdir.fill:
        mov     ecx, [esp + 28]         ; total number
        test    ecx, ecx
        jz      .quit
        push    ecx
;DEBUGF 1,"ecx: %d\n",ecx
        lea     edi, [edx + 40]         ; get file name offset
;DEBUGF 1,"filename: ..\n"
        mov     dword[edi], '..'
        mov     edi, edx
        push    eax ebx edx esi
        stdcall xfs_get_inode_number_sf, dword[ebx + xfs_inode.di_u + xfs_dir2_sf_hdr.count], dword[ebx + xfs_inode.di_u + xfs_dir2_sf_hdr.parent + 4], dword[ebx + xfs_inode.di_u + xfs_dir2_sf_hdr.parent]
        stdcall xfs_read_inode, eax, edx, [ebp + XFS.tmp_inode]
;        test    eax, eax
;        jnz     .error
        stdcall xfs_get_inode_info, edx, edi
        test    eax, eax
        pop     esi edx ebx eax
        jnz     .error
        mov     ecx, [esp + 44]         ; file name encding
        mov     [edx + 4], ecx
        add     edx, 304                ; ASCII only for now
        pop     ecx
        dec     ecx
        jz      .quit

;        push    ecx
;        lea     edi, [edx + 40]
;DEBUGF 1,"filename: .\n"
;        mov     dword[edi], '.'
;        mov     edi, edx
;        push    eax edx
;        stdcall xfs_get_inode_info, [ebp + XFS.cur_inode], edi
;        test    eax, eax
;        pop     edx eax
;        jnz     .error
;        mov     ecx, [esp + 44]
;        mov     [edx + 4], ecx
;        add     edx, 304                ; ASCII only for now
;        pop     ecx
;        dec     ecx
;        jz      .quit

        ; we skipped some entries
        ; now we fill min(required, present) number of bdfe's

  .shortdir.skipped:
;DEBUGF 1,"ecx: %d\n",ecx
        push    ecx
        movzx   ecx, byte[esi + xfs_dir2_sf_entry.namelen]
        add     esi, xfs_dir2_sf_entry.name
        lea     edi, [edx + 40]         ; bdfe offset of file name
;DEBUGF 1,"filename: |%s|\n",esi
        rep movsb
        mov     word[edi], 0            ; terminator (ASCIIZ)

        push    eax ebx ecx edx esi
;        push    edx     ; for xfs_get_inode_info
        mov     edi, edx
        stdcall xfs_get_inode_number_sf, dword[ebx + xfs_inode.di_u + xfs_dir2_sf_hdr.count], [esi + 4], [esi]
        stdcall xfs_read_inode, eax, edx, [ebp + XFS.tmp_inode]
;        test    eax, eax
;        jnz     .error
        stdcall xfs_get_inode_info, edx, edi
        test    eax, eax
        pop     esi edx ecx ebx eax
        jnz     .error
        mov     ecx, [esp + 44]         ; file name encoding
        mov     [edx + 4], ecx

        add     edx, 304                ; ASCII only for now
        add     esi, eax
        pop     ecx
        dec     ecx
        jnz     .shortdir.skipped
        jmp     .quit

  .blockdir:
;DEBUGF 1,"blockdir\n"
        push    edx
        lea     eax, [ebx + xfs_inode.di_u + xfs_bmbt_rec.l0]
        stdcall xfs_extent_unpack, eax
;DEBUGF 1,"extent.br_startoff  : 0x%x%x\n",[ebp+XFS.extent.br_startoff+4],[ebp+XFS.extent.br_startoff+0]
;DEBUGF 1,"extent.br_startblock: 0x%x%x\n",[ebp+XFS.extent.br_startblock+4],[ebp+XFS.extent.br_startblock+0]
;DEBUGF 1,"extent.br_blockcount: %d\n",[ebp+XFS.extent.br_blockcount]
;DEBUGF 1,"extent.br_state     : %d\n",[ebp+XFS.extent.br_state]
        stdcall xfs_read_dirblock, dword[ebp + XFS.extent.br_startblock + 0], dword[ebp + XFS.extent.br_startblock + 4], [ebp + XFS.cur_dirblock]
        test    eax, eax
        pop     edx
        jnz     .error
;DEBUGF 1,"dirblock signature: %s\n",[ebp+XFS.cur_dirblock]
        mov     ebx, [ebp + XFS.cur_dirblock]
        mov     dword[edx + 0], 1       ; version
        mov     eax, [ebp + XFS.dirblocksize]
        mov     ecx, [ebx + eax - sizeof.xfs_dir2_block_tail + xfs_dir2_block_tail.stale]
        mov     eax, [ebx + eax - sizeof.xfs_dir2_block_tail + xfs_dir2_block_tail.count]
        bswap   ecx
        bswap   eax
        sub     eax, ecx                ; actual number of entries = count - stale
        mov     [edx + 8], eax          ; total entries
;DEBUGF 1,"total entries: %d\n",eax
        sub     eax, [esp + 24]         ; start number
        cmp     eax, [esp + 28]         ; entries to read
        jbe     @f
        mov     eax, [esp + 28]
    @@:
        mov     [esp + 28], eax
        mov     [edx + 4], eax          ; number of actually read entries
        mov     [ebp + XFS.entries_read], eax
;DEBUGF 1,"actually read entries: %d\n",eax
        mov     dword[edx + 12], 0      ; reserved
        mov     dword[edx + 16], 0      ; 
        mov     dword[edx + 20], 0      ; 
        mov     dword[edx + 24], 0      ; 
        mov     dword[edx + 28], 0      ; 
        add     ebx, xfs_dir2_block.u

        mov     ecx, [esp + 24]         ; start entry number
                                        ; also means how many to skip
        test    ecx, ecx
        jz      .blockdir.skipped
  .blockdir.skip:
        cmp     word[ebx + xfs_dir2_data_union.unused.freetag], XFS_DIR2_DATA_FREE_TAG
        jne     @f
        movzx   eax, word[ebx + xfs_dir2_data_union.unused.length]
        xchg    al, ah
        add     ebx, eax
        jmp     .blockdir.skip
    @@:
        movzx   eax, [ebx + xfs_dir2_data_union.xentry.namelen]
        lea     ebx, [ebx + xfs_dir2_data_union.xentry.name + eax + 2]       ; 2 bytes for 'tag'
        add     ebx, 7                  ; align on 8 bytes
        and     ebx, not 7
        dec     ecx
        jnz     .blockdir.skip
  .blockdir.skipped:
        mov     ecx, [edx + 4]          ; actually read entries
        test    ecx, ecx
        jz      .quit
        add     edx, 32                 ; set edx to the first bdfe
  .blockdir.next_entry:
        cmp     word[ebx + xfs_dir2_data_union.unused.freetag], XFS_NULL
        jne     @f
        movzx   eax, word[ebx + xfs_dir2_data_union.unused.length]
        xchg    al, ah
        add     ebx, eax
        jmp     .blockdir.next_entry
    @@:
        push    ecx
        push    eax ebx ecx edx esi
        mov     edi, edx
        mov     edx, dword[ebx + xfs_dir2_data_union.xentry.inumber + 0]
        mov     eax, dword[ebx + xfs_dir2_data_union.xentry.inumber + 4]
        bswap   edx
        bswap   eax
        stdcall xfs_read_inode, eax, edx, [ebp + XFS.tmp_inode]
        stdcall xfs_get_inode_info, edx, edi
        test    eax, eax
        pop     esi edx ecx ebx eax
        jnz     .error
        mov     ecx, [esp + 44]
        mov     [edx + 4], ecx
        lea     edi, [edx + 40]
        movzx   ecx, byte[ebx + xfs_dir2_data_union.xentry.namelen]
        lea     esi, [ebx + xfs_dir2_data_union.xentry.name]
;DEBUGF 1,"filename: |%s|\n",esi
        rep movsb
;        call    utf8_to_cp866
        mov     word[edi], 0            ; terminator
        lea     ebx, [esi + 2]          ; skip 'tag'
        add     ebx, 7                  ; xfs_dir2_data_entries are aligned to 8 bytes
        and     ebx, not 7
        add     edx, 304
        pop     ecx
        dec     ecx
        jnz     .blockdir.next_entry
        jmp     .quit

  .leafdir:
;DEBUGF 1,"readdir: leaf\n"
        mov     [ebp + XFS.cur_inode_save], ebx
        push    ebx ecx edx
        lea     eax, [ebx + xfs_inode.di_u + xfs_bmbt_rec.l0]
        mov     edx, [ebx + xfs_inode.di_core.di_nextents]
        bswap   edx
        stdcall xfs_extent_list_read_dirblock, eax, [ebp + XFS.dir2_leaf_offset_blocks], 0, edx, 0xffffffff, 0xffffffff
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        pop     edx ecx ebx
        jz      .error

        mov     eax, [ebp + XFS.cur_dirblock]
        movzx   ecx, word[eax + xfs_dir2_leaf.hdr.stale]
        movzx   eax, word[eax + xfs_dir2_leaf.hdr.count]
        xchg    cl, ch
        xchg    al, ah
        sub     eax, ecx
;DEBUGF 1,"total count: %d\n",eax

        mov     dword[edx + 0], 1       ; version
        mov     [edx + 8], eax          ; total entries
        sub     eax, [esp + 24]         ; start number
        cmp     eax, [esp + 28]         ; entries to read
        jbe     @f
        mov     eax, [esp + 28]
    @@:
        mov     [esp + 28], eax
        mov     [edx + 4], eax          ; number of actually read entries

        mov     dword[edx + 12], 0      ; reserved
        mov     dword[edx + 16], 0      ; 
        mov     dword[edx + 20], 0      ; 
        mov     dword[edx + 24], 0      ; 
        mov     dword[edx + 28], 0      ; 

        mov     eax, [ebp + XFS.cur_dirblock]
        add     eax, [ebp + XFS.dirblocksize]
        mov     [ebp + XFS.max_dirblockaddr], eax
        mov     dword[ebp + XFS.next_block_num + 0], 0
        mov     dword[ebp + XFS.next_block_num + 4], 0

        mov     ebx, [ebp + XFS.max_dirblockaddr]       ; to read dirblock immediately
        mov     ecx, [esp + 24]         ; start number
        test    ecx, ecx
        jz      .leafdir.skipped
  .leafdir.skip:
        cmp     ebx, [ebp + XFS.max_dirblockaddr]
        jne     @f
        push    ecx edx
        mov     ebx, [ebp + XFS.cur_inode_save]
        lea     eax, [ebx + xfs_inode.di_u + xfs_bmbt_rec.l0]
        mov     edx, [ebx + xfs_inode.di_core.di_nextents]
        bswap   edx
        stdcall xfs_extent_list_read_dirblock, eax, dword[ebp + XFS.next_block_num + 0], dword[ebp + XFS.next_block_num + 4], edx, [ebp + XFS.dir2_leaf_offset_blocks], 0
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        jz      .error
        add     eax, 1
        adc     edx, 0
        mov     dword[ebp + XFS.next_block_num + 0], eax
        mov     dword[ebp + XFS.next_block_num + 4], edx
        mov     ebx, [ebp + XFS.cur_dirblock]
        add     ebx, sizeof.xfs_dir2_data_hdr
        pop     edx ecx
    @@:
        cmp     word[ebx + xfs_dir2_data_union.unused.freetag], XFS_DIR2_DATA_FREE_TAG
        jne     @f
        movzx   eax, word[ebx + xfs_dir2_data_union.unused.length]
        xchg    al, ah
        add     ebx, eax
        jmp     .leafdir.skip
    @@:
        movzx   eax, [ebx + xfs_dir2_data_union.xentry.namelen]
        lea     ebx, [ebx + xfs_dir2_data_union.xentry.name + eax + 2]       ; 2 for 'tag'
        add     ebx, 7
        and     ebx, not 7
        dec     ecx
        jnz     .leafdir.skip
  .leafdir.skipped:
        mov     [ebp + XFS.entries_read], 0
        mov     ecx, [edx + 4]          ; actually read entries
        test    ecx, ecx
        jz      .quit
        add     edx, 32                 ; first bdfe entry
  .leafdir.next_entry:
;DEBUGF 1,"next_extry\n"
        cmp     ebx, [ebp + XFS.max_dirblockaddr]
        jne     .leafdir.process_current_block
        push    ecx edx
        mov     ebx, [ebp + XFS.cur_inode_save]
        lea     eax, [ebx + xfs_inode.di_u + xfs_bmbt_rec.l0]
        mov     edx, [ebx + xfs_inode.di_core.di_nextents]
        bswap   edx
        stdcall xfs_extent_list_read_dirblock, eax, dword[ebp + XFS.next_block_num + 0], dword[ebp + XFS.next_block_num + 4], edx, [ebp + XFS.dir2_leaf_offset_blocks], 0
;DEBUGF 1,"RETVALUE: %d %d\n",edx,eax
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        jnz     @f
        pop     edx ecx
        jmp     .quit
    @@:
        add     eax, 1
        adc     edx, 0
        mov     dword[ebp + XFS.next_block_num + 0], eax
        mov     dword[ebp + XFS.next_block_num + 4], edx
        mov     ebx, [ebp + XFS.cur_dirblock]
        add     ebx, sizeof.xfs_dir2_data_hdr
        pop     edx ecx
  .leafdir.process_current_block:
        cmp     word[ebx + xfs_dir2_data_union.unused.freetag], XFS_DIR2_DATA_FREE_TAG
        jne     @f
        movzx   eax, word[ebx + xfs_dir2_data_union.unused.length]
        xchg    al, ah
        add     ebx, eax
        jmp     .leafdir.next_entry
    @@:
        push    eax ebx ecx edx esi
        mov     edi, edx
        mov     edx, dword[ebx + xfs_dir2_data_union.xentry.inumber + 0]
        mov     eax, dword[ebx + xfs_dir2_data_union.xentry.inumber + 4]
        bswap   edx
        bswap   eax
        stdcall xfs_read_inode, eax, edx, [ebp + XFS.tmp_inode]
        stdcall xfs_get_inode_info, edx, edi
        test    eax, eax
        pop     esi edx ecx ebx eax
        jnz     .error
        push    ecx
        mov     ecx, [esp + 44]
        mov     [edx + 4], ecx
        lea     edi, [edx + 40]
        movzx   ecx, byte[ebx + xfs_dir2_data_union.xentry.namelen]
        lea     esi, [ebx + xfs_dir2_data_union.xentry.name]
;DEBUGF 1,"filename: |%s|\n",esi
        rep movsb
        pop     ecx
        mov     word[edi], 0
        lea     ebx, [esi + 2]  ; skip 'tag'
        add     ebx, 7          ; xfs_dir2_data_entries are aligned to 8 bytes
        and     ebx, not 7
        add     edx, 304        ; ASCII only for now
        inc     [ebp + XFS.entries_read]
        dec     ecx
        jnz     .leafdir.next_entry
        jmp     .quit

  .nodedir:
;DEBUGF 1,"readdir: node\n"
        push    edx
        mov     [ebp + XFS.cur_inode_save], ebx
        mov     [ebp + XFS.entries_read], 0
        lea     eax, [ebx + xfs_inode.di_u + xfs_bmbt_rec.l0]
        mov     edx, [ebx + xfs_inode.di_core.di_nextents]
        bswap   edx
        stdcall xfs_dir2_node_get_numfiles, eax, edx, [ebp + XFS.dir2_leaf_offset_blocks]
        pop     edx
        test    eax, eax
        jnz     .error
        mov     eax, [ebp + XFS.entries_read]
        mov     [ebp + XFS.entries_read], 0
;DEBUGF 1,"numfiles: %d\n",eax
        mov     dword[edx + 0], 1       ; version
        mov     [edx + 8], eax          ; total entries
        sub     eax, [esp + 24]         ; start number
        cmp     eax, [esp + 28]         ; entries to read
        jbe     @f
        mov     eax, [esp + 28]
    @@:
        mov     [esp + 28], eax
        mov     [edx + 4], eax          ; number of actually read entries

        mov     dword[edx + 12], 0      ; reserved
        mov     dword[edx + 16], 0      ; 
        mov     dword[edx + 20], 0      ; 
        mov     dword[edx + 24], 0      ; 
        mov     dword[edx + 28], 0      ; 

        mov     eax, [ebp + XFS.cur_dirblock]
        add     eax, [ebp + XFS.dirblocksize]
        mov     [ebp + XFS.max_dirblockaddr], eax
        mov     dword[ebp + XFS.next_block_num + 0], 0
        mov     dword[ebp + XFS.next_block_num + 4], 0

        mov     ebx, [ebp + XFS.max_dirblockaddr]       ; to read dirblock immediately
        mov     ecx, [esp + 24]         ; start number
        test    ecx, ecx
        jz      .leafdir.skipped
        jmp     .leafdir.skip

  .btreedir:
;DEBUGF 1,"readdir: btree\n"
        mov     [ebp + XFS.cur_inode_save], ebx
        push    ebx edx
        mov     eax, [ebx + xfs_inode.di_core.di_nextents]
        bswap   eax
        mov     [ebp + XFS.ro_nextents], eax
        mov     eax, [ebp + XFS.inodesize]
        sub     eax, xfs_inode.di_u
        sub     eax, sizeof.xfs_bmdr_block
        shr     eax, 4
;DEBUGF 1,"maxnumresc: %d\n",eax
        mov     edx, dword[ebx + xfs_inode.di_u + sizeof.xfs_bmdr_block + sizeof.xfs_bmbt_key*eax + 0]
        mov     eax, dword[ebx + xfs_inode.di_u + sizeof.xfs_bmdr_block + sizeof.xfs_bmbt_key*eax + 4]
        bswap   eax
        bswap   edx
        mov     ebx, [ebp + XFS.cur_block]
;DEBUGF 1,"read_block: %x %x ",edx,eax
        stdcall xfs_read_block
        pop     edx ebx
        test    eax, eax
        jnz     .error
;DEBUGF 1,"ok\n"

        mov     ebx, [ebp + XFS.cur_block]
        push    edx
        mov     [ebp + XFS.entries_read], 0
        lea     eax, [ebx + sizeof.xfs_bmbt_block]
        mov     edx, [ebp + XFS.ro_nextents]
        stdcall xfs_dir2_node_get_numfiles, eax, edx, [ebp + XFS.dir2_leaf_offset_blocks]
        pop     edx
        test    eax, eax
        jnz     .error
        mov     eax, [ebp + XFS.entries_read]
        mov     [ebp + XFS.entries_read], 0
;DEBUGF 1,"numfiles: %d\n",eax

        mov     dword[edx + 0], 1       ; version
        mov     [edx + 8], eax          ; total entries
        sub     eax, [esp + 24]         ; start number
        cmp     eax, [esp + 28]         ; entries to read
        jbe     @f
        mov     eax, [esp + 28]
    @@:
        mov     [esp + 28], eax
        mov     [edx + 4], eax          ; number of actually read entries

        mov     dword[edx + 12], 0
        mov     dword[edx + 16], 0
        mov     dword[edx + 20], 0
        mov     dword[edx + 24], 0
        mov     dword[edx + 28], 0

        mov     eax, [ebp + XFS.cur_dirblock]   ; fsblock?
        add     eax, [ebp + XFS.dirblocksize]
        mov     [ebp + XFS.max_dirblockaddr], eax
        mov     dword[ebp + XFS.next_block_num + 0], 0
        mov     dword[ebp + XFS.next_block_num + 4], 0

        mov     ebx, [ebp + XFS.max_dirblockaddr]       ; to read dirblock immediately
        mov     ecx, [esp + 24]         ; start number
        test    ecx, ecx
        jz      .btreedir.skipped
;        jmp     .btreedir.skip
  .btreedir.skip:
        cmp     ebx, [ebp + XFS.max_dirblockaddr]
        jne     @f
        push    ecx edx
        mov     ebx, [ebp + XFS.cur_block]
        lea     eax, [ebx + sizeof.xfs_bmbt_block]
        mov     edx, [ebp + XFS.ro_nextents]
        stdcall xfs_extent_list_read_dirblock, eax, dword[ebp + XFS.next_block_num + 0], dword[ebp + XFS.next_block_num + 4], edx, [ebp + XFS.dir2_leaf_offset_blocks], 0
;DEBUGF 1,"RETVALUE: %d %d\n",edx,eax
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        jz      .error
        add     eax, 1
        adc     edx, 0
        mov     dword[ebp + XFS.next_block_num + 0], eax
        mov     dword[ebp + XFS.next_block_num + 4], edx
        mov     ebx, [ebp + XFS.cur_dirblock]
        add     ebx, sizeof.xfs_dir2_data_hdr
        pop     edx ecx
    @@:
        cmp     word[ebx + xfs_dir2_data_union.unused.freetag], XFS_DIR2_DATA_FREE_TAG
        jne     @f
        movzx   eax, word[ebx + xfs_dir2_data_union.unused.length]
        xchg    al, ah
        add     ebx, eax
        jmp     .btreedir.skip
    @@:
        movzx   eax, [ebx + xfs_dir2_data_union.xentry.namelen]
        lea     ebx, [ebx + xfs_dir2_data_union.xentry.name + eax + 2]       ; 2 for 'tag'
        add     ebx, 7
        and     ebx, not 7
        dec     ecx
        jnz     .btreedir.skip
  .btreedir.skipped:
        mov     [ebp + XFS.entries_read], 0
        mov     ecx, [edx + 4]          ; actually read entries
        test    ecx, ecx
        jz      .quit
        add     edx, 32
  .btreedir.next_entry:
;mov eax, [ebp + XFS.entries_read]
;DEBUGF 1,"next_extry: %d\n",eax
        cmp     ebx, [ebp + XFS.max_dirblockaddr]
        jne     .btreedir.process_current_block
        push    ecx edx
        mov     ebx, [ebp + XFS.cur_block]
        lea     eax, [ebx + sizeof.xfs_bmbt_block]
        mov     edx, [ebp + XFS.ro_nextents]
        stdcall xfs_extent_list_read_dirblock, eax, dword[ebp + XFS.next_block_num + 0], dword[ebp + XFS.next_block_num + 4], edx, [ebp + XFS.dir2_leaf_offset_blocks], 0
;DEBUGF 1,"RETVALUE: %d %d\n",edx,eax
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        jnz     @f
        pop     edx ecx
        jmp     .quit
    @@:
        add     eax, 1
        adc     edx, 0
        mov     dword[ebp + XFS.next_block_num + 0], eax
        mov     dword[ebp + XFS.next_block_num + 4], edx
        mov     ebx, [ebp + XFS.cur_dirblock]
        add     ebx, sizeof.xfs_dir2_data_hdr
        pop     edx ecx
  .btreedir.process_current_block:
        cmp     word[ebx + xfs_dir2_data_union.unused.freetag], XFS_DIR2_DATA_FREE_TAG
        jne     @f
        movzx   eax, word[ebx + xfs_dir2_data_union.unused.length]
        xchg    al, ah
        add     ebx, eax
        jmp     .btreedir.next_entry
    @@:
        push    eax ebx ecx edx esi
        mov     edi, edx
        mov     edx, dword[ebx + xfs_dir2_data_union.xentry.inumber + 0]
        mov     eax, dword[ebx + xfs_dir2_data_union.xentry.inumber + 4]
        bswap   edx
        bswap   eax
        stdcall xfs_read_inode, eax, edx, [ebp + XFS.tmp_inode]
        stdcall xfs_get_inode_info, edx, edi
        test    eax, eax
        pop     esi edx ecx ebx eax
        jnz     .error
        push    ecx
        mov     ecx, [esp + 44]
        mov     [edx + 4], ecx
        lea     edi, [edx + 40]
        movzx   ecx, byte[ebx + xfs_dir2_data_union.xentry.namelen]
        lea     esi, [ebx + xfs_dir2_data_union.xentry.name]
;DEBUGF 1,"filename: |%s|\n",esi
        rep movsb
        pop     ecx
        mov     word[edi], 0
        lea     ebx, [esi + 2]  ; skip 'tag'
        add     ebx, 7          ; xfs_dir2_data_entries are aligned to 8 bytes
        and     ebx, not 7
        add     edx, 304
        inc     [ebp + XFS.entries_read]
        dec     ecx
        jnz     .btreedir.next_entry
        jmp     .quit


  .quit:
        pop     edi esi edx ecx
        add     esp, 4  ; pop vars
        xor     eax, eax
;        mov     ebx, [esp + 8]
        mov     ebx, [ebp + XFS.entries_read]
DEBUGF 1,"xfs_dir_get_bdfes done: %d\n",ebx
        ret     20
  .error:
        pop     edi esi edx ecx
        add     esp, 4  ; pop vars
        mov     eax, ERROR_FS_FAIL
        movi    ebx, -1
        ret     20


;----------------------------------------------------------------
; push inode_hi
; push inode_lo
; push name
;----------------------------------------------------------------
xfs_get_inode_short:
        ; this function searches for the file in _current_ dir
        ; it is called recursively for all the subdirs /path/to/my/file

;DEBUGF 1,"xfs_get_inode_short: %s\n",[esp+4]
        mov     esi, [esp + 4]  ; name
        movzx   eax, word[esi]
        cmp     eax, '.'        ; current dir; it is already read, just return
        je      .quit
        cmp     eax, './'       ; same thing
        je      .quit

        ; read inode

        mov     eax, [esp + 8]  ; inode_lo
        mov     edx, [esp + 12] ; inode_hi
        stdcall xfs_read_inode, eax, edx, [ebp + XFS.cur_inode]
        test    eax, eax
        movi    eax, ERROR_FS_FAIL
        jnz     .error

        ; find file name in directory
        ; switch directory ondisk format

        mov     ebx, edx
        mov     [ebp + XFS.cur_inode_save], ebx
        cmp     byte[ebx + xfs_inode.di_core.di_format], XFS_DINODE_FMT_LOCAL
        jne     .not_shortdir
;DEBUGF 1,"dir: shortdir\n"
        jmp     .shortdir
  .not_shortdir:
        cmp     byte[ebx + xfs_inode.di_core.di_format], XFS_DINODE_FMT_EXTENTS
        jne     .not_blockdir
        mov     eax, [ebx + xfs_inode.di_core.di_nextents]
        bswap   eax
        cmp     eax, 1
        jne     .not_blockdir
        jmp     .blockdir
  .not_blockdir:
        cmp     byte[ebx + xfs_inode.di_core.di_format], XFS_DINODE_FMT_EXTENTS
        jne     .not_leafdir
        mov     eax, [ebx + xfs_inode.di_core.di_nextents]
        bswap   eax
        cmp     eax, 4
        ja      .not_leafdir
        jmp     .leafdir
  .not_leafdir:
        cmp     byte[ebx + xfs_inode.di_core.di_format], XFS_DINODE_FMT_EXTENTS
        jne     .not_nodedir
        jmp     .nodedir
  .not_nodedir:
        cmp     byte[ebx + xfs_inode.di_core.di_format], XFS_DINODE_FMT_BTREE
        jne     .not_btreedir
        jmp     .btreedir
  .not_btreedir:
DEBUGF 1,"NOT IMPLEMENTED: DIR FORMAT\n"
        jmp     .error

  .shortdir:
  .shortdir.check_parent:
        ; parent inode number in shortform directories is always implicit, check this case
        mov     eax, [esi]
        and     eax, 0x00ffffff
        cmp     eax, '..'
        je      .shortdir.parent2
        cmp     eax, '../'
        je      .shortdir.parent3
        jmp     .shortdir.common
  .shortdir.parent3:
        inc     esi
  .shortdir.parent2:
        add     esi, 2
        add     ebx, xfs_inode.di_u
        stdcall xfs_get_inode_number_sf, dword[ebx + xfs_dir2_sf_hdr.count], dword[ebx + xfs_dir2_sf_hdr.parent + 4], dword[ebx + xfs_dir2_sf_hdr.parent]
;DEBUGF 1,"found inode: 0x%x%x\n",edx,eax
        jmp     .quit

        ; not a parent inode?
        ; search in the list, all the other files are stored uniformly

  .shortdir.common:
        mov     eax, 4
        movzx   edx, word[ebx + xfs_inode.di_u + xfs_dir2_sf_hdr.count] ; read count (byte) and i8count (byte) at once
        test    dl, dl          ; is count zero?
        jnz     @f
        shr     edx, 8          ; use i8count
        add     eax, eax        ; inode_num size
    @@:
        lea     edi, [ebx + xfs_inode.di_u + xfs_dir2_sf_hdr.parent + eax]

  .next_name:
        movzx   ecx, byte[edi + xfs_dir2_sf_entry.namelen]
        add     edi, xfs_dir2_sf_entry.name
        mov     esi, [esp + 4]
;DEBUGF 1,"esi: %s\n",esi
;DEBUGF 1,"edi: %s\n",edi
        repe cmpsb
        jne     @f
        cmp     byte[esi], 0            ; HINT: use adc here?
        je      .found
        cmp     byte[esi], '/'
        je      .found_inc
    @@:
        add     edi, ecx
        add     edi, eax
        dec     edx
        jnz     .next_name
        movi    eax, ERROR_FILE_NOT_FOUND
        jmp     .error
  .found_inc:           ; increment esi to skip '/' symbol
                        ; this means esi always points to valid file name or zero terminator byte
        inc     esi
  .found:
        stdcall xfs_get_inode_number_sf, dword[ebx + xfs_inode.di_u + xfs_dir2_sf_hdr.count], [edi + 4], [edi]
;DEBUGF 1,"found inode: 0x%x%x\n",edx,eax
        jmp     .quit

  .blockdir:
        lea     eax, [ebx + xfs_inode.di_u + xfs_bmbt_rec.l0]
        stdcall xfs_extent_unpack, eax
        stdcall xfs_read_dirblock, dword[ebp + XFS.extent.br_startblock + 0], dword[ebp + XFS.extent.br_startblock + 4], [ebp + XFS.cur_dirblock]
        test    eax, eax
        jnz     .error
;DEBUGF 1,"dirblock signature: %s\n",[ebp+XFS.cur_dirblock]
        mov     ebx, [ebp + XFS.cur_dirblock]
        mov     eax, [ebp + XFS.dirblocksize]
        mov     eax, [ebx + eax - sizeof.xfs_dir2_block_tail + xfs_dir2_block_tail.count]
        ; note that we don't subtract xfs_dir2_block_tail.stale here,
        ; since we need the number of leaf entries rather than file number
        bswap   eax
        add     ebx, [ebp + XFS.dirblocksize]
;        mov     ecx, sizeof.xfs_dir2_leaf_entry
        imul    ecx, eax, sizeof.xfs_dir2_leaf_entry
        sub     ebx, sizeof.xfs_dir2_block_tail
        sub     ebx, ecx
        shr     ecx, 3
        push    ecx     ; for xfs_get_inode_by_hash
        push    ebx     ; for xfs_get_inode_by_hash

        mov     edi, esi
        xor     eax, eax
        mov     ecx, 4096       ; MAX_PATH_LEN
        repne scasb
        movi    eax, ERROR_FS_FAIL
        jne     .error
        neg     ecx
        add     ecx, 4096       ; MAX_PATH_LEN
        dec     ecx
        mov     edx, ecx
;DEBUGF 1,"strlen total  : %d\n",edx
        mov     edi, esi
        mov     eax, '/'
        mov     ecx, edx
        repne scasb
        jne     @f
        inc     ecx
    @@:
        neg     ecx
        add     ecx, edx
;DEBUGF 1,"strlen current: %d\n",ecx
        stdcall xfs_hashname, esi, ecx
        add     esi, ecx
        cmp     byte[esi], '/'
        jne     @f
        inc     esi
    @@:
;DEBUGF 1,"hashed: 0x%x\n",eax
;        bswap   eax
        stdcall xfs_get_addr_by_hash
        bswap   eax
;DEBUGF 1,"got address: 0x%x\n",eax
        cmp     eax, -1
        jne     @f
        movi    eax, ERROR_FILE_NOT_FOUND
        mov     ebx, -1
        jmp     .error
    @@:
        shl     eax, 3
        mov     ebx, [ebp + XFS.cur_dirblock]
        add     ebx, eax
        mov     edx, [ebx + 0]
        mov     eax, [ebx + 4]
        bswap   edx
        bswap   eax
;DEBUGF 1,"found inode: 0x%x%x\n",edx,eax
        jmp     .quit

  .leafdir:
;DEBUGF 1,"dirblock signature: %s\n",[ebp+XFS.cur_dirblock]
        lea     eax, [ebx + xfs_inode.di_u + xfs_bmbt_rec.l0]
        mov     edx, [ebx + xfs_inode.di_core.di_nextents]
        bswap   edx
        stdcall xfs_extent_list_read_dirblock, eax, [ebp + XFS.dir2_leaf_offset_blocks], 0, edx, -1, -1
;DEBUGF 1,"RETVALUE: %d %d\n",edx,eax
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        jz      .error

        mov     ebx, [ebp + XFS.cur_dirblock]
        movzx   eax, [ebx + xfs_dir2_leaf.hdr.count]
        ; note that we don't subtract xfs_dir2_leaf.hdr.stale here,
        ; since we need the number of leaf entries rather than file number
        xchg    al, ah
        add     ebx, xfs_dir2_leaf.ents
;        imul    ecx, eax, sizeof.xfs_dir2_leaf_entry
;        shr     ecx, 3
        push    eax     ; for xfs_get_addr_by_hash: len
        push    ebx     ; for xfs_get_addr_by_hash: base

        mov     edi, esi
        xor     eax, eax
        mov     ecx, 4096       ; MAX_PATH_LEN
        repne scasb
        movi    eax, ERROR_FS_FAIL
        jne     .error
        neg     ecx
        add     ecx, 4096
        dec     ecx
        mov     edx, ecx
;DEBUGF 1,"strlen total  : %d\n",edx
        mov     edi, esi
        mov     eax, '/'
        mov     ecx, edx
        repne scasb
        jne     @f
        inc     ecx
    @@:
        neg     ecx
        add     ecx, edx
;DEBUGF 1,"strlen current: %d\n",ecx
        stdcall xfs_hashname, esi, ecx
        add     esi, ecx
        cmp     byte[esi], '/'
        jne     @f
        inc     esi
    @@:
;DEBUGF 1,"hashed: 0x%x\n",eax
        stdcall xfs_get_addr_by_hash
        bswap   eax
;DEBUGF 1,"got address: 0x%x\n",eax
        cmp     eax, -1
        jne     @f
        movi    eax, ERROR_FILE_NOT_FOUND
        mov     ebx, -1
        jmp     .error
    @@:

        mov     ebx, [ebp + XFS.cur_inode_save]
        push    esi edi
        xor     edi, edi
        mov     esi, eax
        shld    edi, esi, 3     ; get offset
        shl     esi, 3          ; 2^3 = 8 byte align
        mov     edx, esi
        mov     ecx, [ebp + XFS.dirblklog]
        add     ecx, [ebp + XFS.blocklog]
        mov     eax, 1
        shl     eax, cl
        dec     eax
        and     edx, eax
        push    edx
        shrd    esi, edi, cl
        shr     edi, cl
        lea     eax, [ebx + xfs_inode.di_u + xfs_bmbt_rec.l0]
        mov     edx, [ebx + xfs_inode.di_core.di_nextents]
        bswap   edx
        stdcall xfs_extent_list_read_dirblock, eax, esi, edi, edx, [ebp + XFS.dir2_leaf_offset_blocks], 0
;DEBUGF 1,"RETVALUE: %d %d\n",edx,eax
        pop     edx
        pop     edi esi
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        jz      .error

        mov     ebx, [ebp + XFS.cur_dirblock]
        add     ebx, edx
        mov     edx, [ebx + 0]
        mov     eax, [ebx + 4]
        bswap   edx
        bswap   eax
;DEBUGF 1,"found inode: 0x%x%x\n",edx,eax
        jmp     .quit

  .nodedir:
;DEBUGF 1,"lookupdir: node\n"
        mov     [ebp + XFS.cur_inode_save], ebx

        mov     edi, esi
        xor     eax, eax
        mov     ecx, 4096       ; MAX_PATH_LEN
        repne scasb
        movi    eax, ERROR_FS_FAIL
        jne     .error
        neg     ecx
        add     ecx, 4096       ; MAX_PATH_LEN
        dec     ecx
        mov     edx, ecx
;DEBUGF 1,"strlen total  : %d\n",edx
        mov     edi, esi
        mov     eax, '/'
        mov     ecx, edx
        repne scasb
        jne     @f
        inc     ecx
    @@:
        neg     ecx
        add     ecx, edx
;DEBUGF 1,"strlen current: %d\n",ecx
        stdcall xfs_hashname, esi, ecx
        add     esi, ecx
        cmp     byte[esi], '/'
        jne     @f
        inc     esi
    @@:
;DEBUGF 1,"hashed: 0x%x\n",eax
        push    edi edx
        mov     edi, eax
        mov     [ebp + XFS.entries_read], 0
        lea     eax, [ebx + xfs_inode.di_u + xfs_bmbt_rec.l0]
        mov     edx, [ebx + xfs_inode.di_core.di_nextents]
        bswap   edx
        stdcall xfs_dir2_lookupdir_node, eax, edx, [ebp + XFS.dir2_leaf_offset_blocks], edi
        pop     edx edi
        test    eax, eax
        jnz     .error
        bswap   ecx
;DEBUGF 1,"got address: 0x%x\n",ecx

        mov     ebx, [ebp + XFS.cur_inode_save]
        push    esi edi
        xor     edi, edi
        mov     esi, ecx
        shld    edi, esi, 3     ; get offset
        shl     esi, 3          ; 8 byte align
        mov     edx, esi
        mov     ecx, [ebp + XFS.dirblklog]
        add     ecx, [ebp + XFS.blocklog]
        mov     eax, 1
        shl     eax, cl
        dec     eax
        and     edx, eax
        push    edx
        shrd    esi, edi, cl
        shr     edi, cl
        lea     eax, [ebx + xfs_inode.di_u + xfs_bmbt_rec.l0]
        mov     edx, [ebx + xfs_inode.di_core.di_nextents]
        bswap   edx
        stdcall xfs_extent_list_read_dirblock, eax, esi, edi, edx, [ebp + XFS.dir2_leaf_offset_blocks], 0
;DEBUGF 1,"RETVALUE: %d %d\n",edx,eax
        pop     edx
        pop     edi esi
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        jz      .error

        mov     ebx, [ebp + XFS.cur_dirblock]
        add     ebx, edx
        mov     edx, [ebx + 0]
        mov     eax, [ebx + 4]
        bswap   edx
        bswap   eax
;DEBUGF 1,"found inode: 0x%x%x\n",edx,eax
        jmp     .quit

  .btreedir:
DEBUGF 1,"lookupdir: btree\n"
        mov     [ebp + XFS.cur_inode_save], ebx

        push    ebx edx
        mov     eax, [ebx + xfs_inode.di_core.di_nextents]
        bswap   eax
        mov     [ebp + XFS.ro_nextents], eax
        mov     eax, [ebp + XFS.inodesize]
        sub     eax, xfs_inode.di_u
        sub     eax, sizeof.xfs_bmdr_block
        shr     eax, 4  ; FIXME forkoff
;DEBUGF 1,"maxnumresc: %d\n",eax
        mov     edx, dword[ebx + xfs_inode.di_u + sizeof.xfs_bmdr_block + sizeof.xfs_bmbt_key*eax + 0]
        mov     eax, dword[ebx + xfs_inode.di_u + sizeof.xfs_bmdr_block + sizeof.xfs_bmbt_key*eax + 4]
        bswap   eax
        bswap   edx
        mov     ebx, [ebp + XFS.cur_block]
;DEBUGF 1,"read_block: %x %x ",edx,eax
        stdcall xfs_read_block
        pop     edx ebx
        test    eax, eax
        jnz     .error
;DEBUGF 1,"ok\n"
        mov     ebx, [ebp + XFS.cur_block]

        mov     edi, esi
        xor     eax, eax
        mov     ecx, 4096       ; MAX_PATH_LEN
        repne scasb
        movi    eax, ERROR_FS_FAIL
        jne     .error
        neg     ecx
        add     ecx, 4096
        dec     ecx
        mov     edx, ecx
DEBUGF 1,"strlen total  : %d\n",edx
        mov     edi, esi
        mov     eax, '/'
        mov     ecx, edx
        repne scasb
        jne     @f
        inc     ecx
    @@:
        neg     ecx
        add     ecx, edx
DEBUGF 1,"strlen current: %d\n",ecx
        stdcall xfs_hashname, esi, ecx
        add     esi, ecx
        cmp     byte[esi], '/'
        jne     @f
        inc     esi
    @@:
DEBUGF 1,"hashed: 0x%x\n",eax
        push    edi edx
        mov     edi, eax
        mov     [ebp + XFS.entries_read], 0
        lea     eax, [ebx + sizeof.xfs_bmbt_block]
        mov     edx, [ebp + XFS.ro_nextents]
;push eax
;mov eax, [ebp + XFS.dir2_leaf_offset_blocks]
;DEBUGF 1,": 0x%x %d\n",eax,eax
;pop eax
        stdcall xfs_dir2_lookupdir_node, eax, edx, [ebp + XFS.dir2_leaf_offset_blocks], edi
        pop     edx edi
        test    eax, eax
        jnz     .error
        bswap   ecx
DEBUGF 1,"got address: 0x%x\n",ecx

        mov     ebx, [ebp + XFS.cur_block]
        push    esi edi
        xor     edi, edi
        mov     esi, ecx
        shld    edi, esi, 3  ; get offset
        shl     esi, 3
        mov     edx, esi
        mov     ecx, [ebp + XFS.dirblklog]
        add     ecx, [ebp + XFS.blocklog]
        mov     eax, 1
        shl     eax, cl
        dec     eax
        and     edx, eax
        push    edx
        shrd    esi, edi, cl
        shr     edi, cl
        lea     eax, [ebx + sizeof.xfs_bmbt_block]
        mov     edx, [ebp + XFS.ro_nextents]
        stdcall xfs_extent_list_read_dirblock, eax, esi, edi, edx, [ebp + XFS.dir2_leaf_offset_blocks], 0
;DEBUGF 1,"RETVALUE: %d %d\n",edx,eax
        pop     edx
        pop     edi esi
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        jz      .error

        mov     ebx, [ebp + XFS.cur_dirblock]
        add     ebx, edx
        mov     edx, [ebx + 0]
        mov     eax, [ebx + 4]
        bswap   edx
        bswap   eax
DEBUGF 1,"found inode: 0x%x%x\n",edx,eax
        jmp     .quit

  .quit:
        ret     12
  .error:
        xor     eax, eax
        mov     edx, eax
        ret     12


;----------------------------------------------------------------
; push name
; call xfs_get_inode
; test eax, eax
;----------------------------------------------------------------
xfs_get_inode:
        ; call xfs_get_inode_short until file is found / error returned

;DEBUGF 1,"getting inode of: %s\n",[esp+4]
        push    ebx esi edi

        ; start from the root inode

        mov     edx, dword[ebp + XFS.rootino + 4]       ; hi
        mov     eax, dword[ebp + XFS.rootino + 0]       ; lo
        mov     esi, [esp + 16] ; name

  .next_dir:
        cmp     byte[esi], 0
        je      .found

;DEBUGF 1,"next_level: |%s|\n",esi
        stdcall xfs_get_inode_short, esi, eax, edx
        test    edx, edx
        jnz     @f
        test    eax, eax
        jz      .error
    @@:
        jmp     .next_dir       ; file name found, go to next directory level

  .found:

  .quit:
        pop     edi esi ebx
        ret     4
  .error:
        pop     edi esi ebx
        xor     eax, eax
        mov     edx, eax
        ret     4


;----------------------------------------------------------------
; xfs_ReadFolder - XFS implementation of reading a folder
; in:  ebp = pointer to XFS structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
xfs_ReadFolder:

        ; to read folder
        ; 1. lock partition
        ; 2. find inode number
        ; 3. read this inode
        ; 4. get bdfe's
        ; 5. unlock partition

        ; 1.
        call    xfs_lock
        push    ecx edx esi edi

        ; 2.
        push    ebx esi edi
        add     esi, [esp + 32]         ; directory name
;DEBUGF 1,"xfs_ReadFolder: |%s|\n",esi
        stdcall xfs_get_inode, esi
        pop     edi esi ebx
        mov     ecx, edx
        or      ecx, eax
        jnz     @f
        movi    eax, ERROR_FILE_NOT_FOUND
    @@:

        ; 3.
        stdcall xfs_read_inode, eax, edx, [ebp + XFS.cur_inode]
        test    eax, eax
        movi    eax, ERROR_FS_FAIL
        jnz     .error

        ; 4.
        mov     eax, [ebx + 8]          ; encoding
        and     eax, 1
        stdcall xfs_dir_get_bdfes, [ebx + 4], [ebx + 12], [ebx + 16], edx, eax
        test    eax, eax
        jnz     .error

  .quit:
;DEBUGF 1,"\n\n"
        pop     edi esi edx ecx
        ; 5.
        call    xfs_unlock
        xor     eax, eax
        ret
  .error:
;DEBUGF 1,"\n\n"
        pop     edi esi edx ecx
        push    eax
        call    xfs_unlock
        pop     eax
        ret


;----------------------------------------------------------------
; push inode_num_hi
; push inode_num_lo
; push [count]
; call xfs_get_inode_number_sf
;----------------------------------------------------------------
xfs_get_inode_number_sf:

        ; inode numbers in short form directories may be 4 or 8 bytes long
        ; determine the length in run time and read inode number at given address

        cmp     byte[esp + 4 + xfs_dir2_sf_hdr.i8count], 0      ; i8count == 0 means 4 byte per inode number
        je      .i4bytes
  .i8bytes:
        mov     edx, [esp + 12] ; hi
        mov     eax, [esp + 8]  ; lo
        bswap   edx             ; big endian
        bswap   eax
        ret     12
  .i4bytes:
        xor     edx, edx        ; no hi
        mov     eax, [esp + 12] ; hi = lo
        bswap   eax             ; big endian
        ret     12


;----------------------------------------------------------------
; push dest
; push src
; call xfs_get_inode_info
;----------------------------------------------------------------
xfs_get_inode_info:

        ; get access time and other file properties
        ; useful for browsing directories
        ; called for each dir entry

;DEBUGF 1,"get_inode_info\n"
        xor     eax, eax
        mov     edx, [esp + 4]
        movzx   ecx, word[edx + xfs_inode.di_core.di_mode]
        xchg    cl, ch
;DEBUGF 1,"di_mode: %x\n",ecx
        test    ecx, S_IFDIR    ; directory?
        jz      @f
        mov     eax, 0x10       ; set directory flag
    @@:

        mov     edi, [esp + 8]
        mov     [edi + 0], eax
        mov     eax, dword[edx + xfs_inode.di_core.di_size + 0] ; hi
        bswap   eax
        mov     dword[edi + 36], eax    ; file size hi
;DEBUGF 1,"file_size hi: %d\n",eax
        mov     eax, dword[edx + xfs_inode.di_core.di_size + 4] ; lo
        bswap   eax
        mov     dword[edi + 32], eax    ; file size lo
;DEBUGF 1,"file_size lo: %d\n",eax

        add     edi, 8
        mov     eax, [edx + xfs_inode.di_core.di_ctime.t_sec]
        bswap   eax
        push    edx
        xor     edx, edx
        add     eax, 3054539008                                     ;(369 * 365 + 89) * 24 * 3600
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec
        pop     edx

        mov     eax, [edx + xfs_inode.di_core.di_atime.t_sec]
        bswap   eax
        push    edx
        xor     edx, edx
        add     eax, 3054539008                                     ;(369 * 365 + 89) * 24 * 3600
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec
        pop     edx

        mov     eax, [edx + xfs_inode.di_core.di_mtime.t_sec]
        bswap   eax
        push    edx
        xor     edx, edx
        add     eax, 3054539008                                     ;(369 * 365 + 89) * 24 * 3600
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec
        pop     edx

  .quit:
        xor     eax, eax
        ret     8
  .error:
        movi    eax, ERROR_FS_FAIL
        ret     8


;----------------------------------------------------------------
; push extent_data
; call xfs_extent_unpack
;----------------------------------------------------------------
xfs_extent_unpack:

        ; extents come as packet 128bit bitfields
        ; lets unpack them to access internal fields
        ; write result to the XFS.extent structure

        push    eax ebx ecx edx
        mov     ebx, [esp + 20]

        xor     eax, eax
        mov     edx, [ebx + 0]
        bswap   edx
        test    edx, 0x80000000         ; mask, see documentation
        setnz   al
        mov     [ebp + XFS.extent.br_state], eax

        and     edx, 0x7fffffff         ; mask
        mov     eax, [ebx + 4]
        bswap   eax
        shrd    eax, edx, 9
        shr     edx, 9
        mov     dword[ebp + XFS.extent.br_startoff + 0], eax
        mov     dword[ebp + XFS.extent.br_startoff + 4], edx

        mov     edx, [ebx + 4]
        mov     eax, [ebx + 8]
        mov     ecx, [ebx + 12]
        bswap   edx
        bswap   eax
        bswap   ecx
        and     edx, 0x000001ff         ; mask
        shrd    ecx, eax, 21
        shrd    eax, edx, 21
        mov     dword[ebp + XFS.extent.br_startblock + 0], ecx
        mov     dword[ebp + XFS.extent.br_startblock + 4], eax

        mov     eax, [ebx + 12]
        bswap   eax
        and     eax, 0x001fffff         ; mask
        mov     [ebp + XFS.extent.br_blockcount], eax

        pop     edx ecx ebx eax
;DEBUGF 1,"extent.br_startoff  : %d %d\n",[ebp+XFS.extent.br_startoff+4],[ebp+XFS.extent.br_startoff+0]
;DEBUGF 1,"extent.br_startblock: %d %d\n",[ebp+XFS.extent.br_startblock+4],[ebp+XFS.extent.br_startblock+0]
;DEBUGF 1,"extent.br_blockcount: %d\n",[ebp+XFS.extent.br_blockcount]
;DEBUGF 1,"extent.br_state     : %d\n",[ebp+XFS.extent.br_state]
        ret     4


;----------------------------------------------------------------
; push namelen
; push name
; call xfs_hashname
;----------------------------------------------------------------
xfs_hashname:   ; xfs_da_hashname

        ; simple hash function
        ; never fails)

        push    ecx esi
        xor     eax, eax
        mov     esi, [esp + 12] ; name
        mov     ecx, [esp + 16] ; namelen
;mov esi, '.'
;mov ecx, 1
;DEBUGF 1,"hashname: %d %s\n",ecx,esi

    @@:
        rol     eax, 7
        xor     al, [esi]
        add     esi, 1
        loop    @b

        pop     esi ecx
        ret     8


;----------------------------------------------------------------
; push  len
; push  base
; eax -- hash value
; call xfs_get_addr_by_hash
;----------------------------------------------------------------
xfs_get_addr_by_hash:

        ; look for the directory entry offset by its file name hash
        ; allows fast file search for block, leaf and node directories
        ; binary (ternary) search

;DEBUGF 1,"get_addr_by_hash\n"
        push    ebx esi
        mov     ebx, [esp + 12] ; left
        mov     edx, [esp + 16] ; len
  .next:
        mov     ecx, edx
;        jecxz   .error
        test    ecx, ecx
        jz      .error
        shr     ecx, 1
        mov     esi, [ebx + ecx*8 + xfs_dir2_leaf_entry.hashval]
        bswap   esi
;DEBUGF 1,"cmp 0x%x",esi
        cmp     eax, esi
        jb      .below
        ja      .above
        mov     eax, [ebx + ecx*8 + xfs_dir2_leaf_entry.address]
        pop     esi ebx
        ret     8
  .below:
;DEBUGF 1,"b\n"
        mov     edx, ecx
        jmp     .next
  .above:
;DEBUGF 1,"a\n"
        lea     ebx, [ebx + ecx*8 + 8]
        sub     edx, ecx
        dec     edx
        jmp     .next
  .error:
        mov     eax, -1
        pop     esi ebx
        ret     8


;----------------------------------------------------------------
; xfs_GetFileInfo - XFS implementation of getting file info
; in:  ebp = pointer to XFS structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
xfs_GetFileInfo:

        ; lock partition
        ; get inode number by file name
        ; read inode
        ; get info
        ; unlock partition

        push    ecx edx esi edi
        call    xfs_lock

        add     esi, [esp + 20]         ; name
;DEBUGF 1,"xfs_GetFileInfo: |%s|\n",esi
        stdcall xfs_get_inode, esi
        mov     ecx, edx
        or      ecx, eax
        jnz     @f
        movi    eax, ERROR_FILE_NOT_FOUND
        jmp     .error
    @@:
        stdcall xfs_read_inode, eax, edx, [ebp + XFS.cur_inode]
        test    eax, eax
        movi    eax, ERROR_FS_FAIL
        jnz     .error

        stdcall xfs_get_inode_info, edx, [ebx + 16]

  .quit:
        call    xfs_unlock
        pop     edi esi edx ecx
        xor     eax, eax
;DEBUGF 1,"quit\n\n"
        ret
  .error:
        call    xfs_unlock
        pop     edi esi edx ecx
;DEBUGF 1,"error\n\n"
        ret


;----------------------------------------------------------------
; xfs_Read - XFS implementation of reading a file
; in:  ebp = pointer to XFS structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
xfs_Read:
        push    ebx ecx edx esi edi
        call    xfs_lock

        add     esi, [esp + 24]
;DEBUGF 1,"xfs_Read: %d %d |%s|\n",[ebx+4],[ebx+12],esi
        stdcall xfs_get_inode, esi
        mov     ecx, edx
        or      ecx, eax
        jnz     @f
        movi    eax, ERROR_FILE_NOT_FOUND
        jmp     .error
    @@:
        stdcall xfs_read_inode, eax, edx, [ebp + XFS.cur_inode]
        test    eax, eax
        movi    eax, ERROR_FS_FAIL
        jnz     .error
        mov     [ebp + XFS.cur_inode_save], edx

        cmp     byte[edx + xfs_inode.di_core.di_format], XFS_DINODE_FMT_EXTENTS
        jne     .not_extent_list
        jmp     .extent_list
  .not_extent_list:
        cmp     byte[edx + xfs_inode.di_core.di_format], XFS_DINODE_FMT_BTREE
        jne     .not_btree
        jmp     .btree
  .not_btree:
DEBUGF 1,"XFS: NOT IMPLEMENTED: FILE FORMAT\n"
        movi    eax, ERROR_FS_FAIL
        jmp     .error
  .extent_list:
        mov     ecx, [ebx + 12]         ; bytes to read
        mov     edi, [ebx + 16]         ; buffer for data
        mov     esi, [ebx + 8]          ; offset_hi
        mov     ebx, [ebx + 4]          ; offset_lo

        mov     eax, dword[edx + xfs_inode.di_core.di_size + 4] ; lo
        bswap   eax
        mov     dword[ebp + XFS.bytes_left_in_file + 0], eax    ; lo
        mov     eax, dword[edx + xfs_inode.di_core.di_size + 0] ; hi
        bswap   eax
        mov     dword[ebp + XFS.bytes_left_in_file + 4], eax    ; hi

        mov     eax, [edx + xfs_inode.di_core.di_nextents]
        bswap   eax
        mov     [ebp + XFS.left_extents], eax

        mov     dword[ebp + XFS.bytes_read], 0          ; actually read bytes

        xor     eax, eax                ; extent offset in list
  .extent_list.next_extent:
;DEBUGF 1,"extent_list.next_extent, eax: 0x%x\n",eax
;DEBUGF 1,"bytes_to_read: %d\n",ecx
;DEBUGF 1,"cur file offset: %d %d\n",esi,ebx
;DEBUGF 1,"esp: 0x%x\n",esp
        cmp     [ebp + XFS.left_extents], 0
        jne     @f
        test    ecx, ecx
        jz      .quit
        movi    eax, ERROR_END_OF_FILE
        jmp     .error
    @@:
        push    eax
        lea     eax, [edx + xfs_inode.di_u + eax + xfs_bmbt_rec.l0]
        stdcall xfs_extent_unpack, eax
        pop     eax
        dec     [ebp + XFS.left_extents]
        add     eax, sizeof.xfs_bmbt_rec
        push    eax ebx ecx edx esi
        mov     ecx, [ebp + XFS.blocklog]
        shrd    ebx, esi, cl
        shr     esi, cl
        cmp     esi, dword[ebp + XFS.extent.br_startoff + 4]
        jb      .extent_list.to_hole          ; handle sparse files
        ja      @f
        cmp     ebx, dword[ebp + XFS.extent.br_startoff + 0]
        jb      .extent_list.to_hole          ; handle sparse files
        je      .extent_list.to_extent        ; read from the start of current extent
    @@:
        xor     edx, edx
        mov     eax, [ebp + XFS.extent.br_blockcount]
        add     eax, dword[ebp + XFS.extent.br_startoff + 0]
        adc     edx, dword[ebp + XFS.extent.br_startoff + 4]
;DEBUGF 1,"br_startoff: %d %d\n",edx,eax
        cmp     esi, edx
        ja      .extent_list.skip_extent
        jb      .extent_list.to_extent
        cmp     ebx, eax
        jae     .extent_list.skip_extent
        jmp     .extent_list.to_extent
  .extent_list.to_hole:
;DEBUGF 1,"extent_list.to_hole\n"
        pop     esi edx ecx ebx eax
        jmp     .extent_list.read_hole
  .extent_list.to_extent:
;DEBUGF 1,"extent_list.to_extent\n"
        pop     esi edx ecx ebx eax
        jmp     .extent_list.read_extent
  .extent_list.skip_extent:
;DEBUGF 1,"extent_list.skip_extent\n"
        pop     esi edx ecx ebx eax
        jmp     .extent_list.next_extent

  .extent_list.read_hole:
;DEBUGF 1,"hole: offt: 0x%x%x ",esi,ebx
        push    eax edx
        mov     eax, dword[ebp + XFS.extent.br_startoff + 0]
        mov     edx, dword[ebp + XFS.extent.br_startoff + 4]
        push    esi ebx
        mov     ebx, ecx
        sub     eax, ebx        ; get hole_size, it is 64 bit
        sbb     edx, 0          ; now edx:eax contains the size of hole
;DEBUGF 1,"size: 0x%x%x\n",edx,eax
        jnz     @f              ; if hole size >= 2^32, write bytes_to_read zero bytes
        cmp     eax, ecx        ; if hole size >= bytes_to_read, write bytes_to_read zeros
        jae     @f
        mov     ecx, eax        ; if hole is < than bytes_to_read, write hole size zeros
    @@:
        sub     ebx, ecx        ; bytes_to_read - hole_size = left_to_read
        add     dword[esp + 0], ecx     ; update pushed file offset
        adc     dword[esp + 4], 0
        xor     eax, eax        ; hole is made of zeros
        rep stosb
        mov     ecx, ebx
        pop     ebx esi

        test    ecx, ecx        ; all requested bytes are read?
        pop     edx eax
        jz      .quit
        jmp     .extent_list.read_extent        ; continue from the start of unpacked extent

  .extent_list.read_extent:
;DEBUGF 1,"extent_list.read_extent\n"
        push    eax ebx ecx edx esi
        mov     eax, ebx
        mov     edx, esi
        mov     ecx, [ebp + XFS.blocklog]
        shrd    eax, edx, cl
        shr     edx, cl
        sub     eax, dword[ebp + XFS.extent.br_startoff + 0]    ; skip esi:ebx ?
        sbb     edx, dword[ebp + XFS.extent.br_startoff + 4]
        sub     [ebp + XFS.extent.br_blockcount], eax
        add     dword[ebp + XFS.extent.br_startblock + 0], eax
        adc     dword[ebp + XFS.extent.br_startblock + 4], 0
  .extent_list.read_extent.next_block:
;DEBUGF 1,"extent_list.read_extent.next_block\n"
        cmp     [ebp + XFS.extent.br_blockcount], 0     ; out of blocks in current extent?
        jne     @f
        pop     esi edx ecx ebx eax
        jmp     .extent_list.next_extent                ; go to next extent
    @@:
        mov     eax, dword[ebp + XFS.extent.br_startblock + 0]
        mov     edx, dword[ebp + XFS.extent.br_startblock + 4]
        push    ebx
        mov     ebx, [ebp + XFS.cur_block]
;DEBUGF 1,"read block: 0x%x%x\n",edx,eax
        stdcall xfs_read_block
        test    eax, eax
        pop     ebx
        jz      @f
        pop     esi edx ecx ebx eax
        movi    eax, ERROR_FS_FAIL
        jmp     .error
    @@:
        dec     [ebp + XFS.extent.br_blockcount]
        add     dword[ebp + XFS.extent.br_startblock + 0], 1
        adc     dword[ebp + XFS.extent.br_startblock + 4], 0
        mov     esi, [ebp + XFS.cur_block]
        mov     ecx, [ebp + XFS.blocklog]
        mov     eax, 1
        shl     eax, cl
        dec     eax             ; get blocklog mask
        and     eax, ebx        ; offset in current block
        add     esi, eax
        neg     eax
        add     eax, [ebp + XFS.blocksize]
        mov     ecx, [esp + 8]  ; pushed ecx, bytes_to_read
        cmp     ecx, eax        ; is current block enough?
        jbe     @f              ; if so, read bytes_to_read bytes
        mov     ecx, eax        ; otherwise read the block up to the end
    @@:
        sub     [esp + 8], ecx          ; left_to_read
        add     [esp + 12], ecx         ; update current file offset, pushed ebx
        sub     dword[ebp + XFS.bytes_left_in_file + 0], ecx
        sbb     dword[ebp + XFS.bytes_left_in_file + 4], 0
        jnc     @f
        add     dword[ebp + XFS.bytes_left_in_file + 0], ecx
        mov     ecx, dword[ebp + XFS.bytes_left_in_file + 0]
        mov     dword[ebp + XFS.bytes_left_in_file + 0], 0
        mov     dword[ebp + XFS.bytes_left_in_file + 4], 0
    @@:
        add     [ebp + XFS.bytes_read], ecx
        adc     [esp + 0], dword 0      ; pushed esi
;DEBUGF 1,"read data: %d\n",ecx
        rep movsb
        mov     ecx, [esp + 8]
;DEBUGF 1,"left_to_read: %d\n",ecx
        xor     ebx, ebx
        test    ecx, ecx
        jz      @f
        cmp     dword[ebp + XFS.bytes_left_in_file + 4], 0
        jne     .extent_list.read_extent.next_block
        cmp     dword[ebp + XFS.bytes_left_in_file + 0], 0
        jne     .extent_list.read_extent.next_block
    @@:
        pop     esi edx ecx ebx eax
        jmp     .quit

  .btree:
        mov     ecx, [ebx + 12]         ; bytes to read
        mov     [ebp + XFS.bytes_to_read], ecx
        mov     edi, [ebx + 16]         ; buffer for data
        mov     esi, [ebx + 8]          ; offset_hi
        mov     ebx, [ebx + 4]          ; offset_lo
        mov     dword[ebp + XFS.file_offset + 0], ebx
        mov     dword[ebp + XFS.file_offset + 4], esi
        mov     [ebp + XFS.buffer_pos], edi

        mov     eax, dword[edx + xfs_inode.di_core.di_size + 4] ; lo
        bswap   eax
        mov     dword[ebp + XFS.bytes_left_in_file + 0], eax    ; lo
        mov     eax, dword[edx + xfs_inode.di_core.di_size + 0] ; hi
        bswap   eax
        mov     dword[ebp + XFS.bytes_left_in_file + 4], eax    ; hi

        mov     eax, [edx + xfs_inode.di_core.di_nextents]
        bswap   eax
        mov     [ebp + XFS.left_extents], eax

        mov     dword[ebp + XFS.bytes_read], 0          ; actually read bytes

        push    ebx ecx edx esi edi
        mov     [ebp + XFS.eof], 0
        mov     eax, dword[ebp + XFS.file_offset + 0]
        mov     edx, dword[ebp + XFS.file_offset + 4]
        add     eax, [ebp + XFS.bytes_to_read]
        adc     edx, 0
        sub     eax, dword[ebp + XFS.bytes_left_in_file + 0]
        sbb     edx, dword[ebp + XFS.bytes_left_in_file + 4]
        jc      @f      ; file_offset + bytes_to_read < file_size
        jz      @f      ; file_offset + bytes_to_read = file_size
        mov     [ebp + XFS.eof], 1
        cmp     edx, 0
        jne     .error.eof
        sub     dword[ebp + XFS.bytes_to_read], eax
        jc      .error.eof
        jz      .error.eof
    @@:
        stdcall xfs_btree_read, 0, 0, 1
        pop     edi esi edx ecx ebx
        test    eax, eax
        jnz     .error
        cmp     [ebp + XFS.eof], 1
        jne     .quit
        jmp     .error.eof


  .quit:
        call    xfs_unlock
        pop     edi esi edx ecx ebx
        xor     eax, eax
        mov     ebx, [ebp + XFS.bytes_read]
;DEBUGF 1,"quit: %d\n\n",ebx
        ret
  .error.eof:
        movi    eax, ERROR_END_OF_FILE
  .error:
;DEBUGF 1,"error\n\n"
        call    xfs_unlock
        pop     edi esi edx ecx ebx
        mov     ebx, [ebp + XFS.bytes_read]
        ret


;----------------------------------------------------------------
; push  max_offset_hi
; push  max_offset_lo
; push  nextents
; push  block_number_hi
; push  block_number_lo
; push  extent_list
; -1 / read block number
;----------------------------------------------------------------
xfs_extent_list_read_dirblock:  ; skips holes
;DEBUGF 1,"xfs_extent_list_read_dirblock\n"
        push    ebx esi edi
;mov eax, [esp+28]
;DEBUGF 1,"nextents: %d\n",eax
;mov eax, [esp+20]
;mov edx, [esp+24]
;DEBUGF 1,"block_number: 0x%x%x\n",edx,eax
;mov eax, [esp+32]
;mov edx, [esp+36]
;DEBUGF 1,"max_addr    : 0x%x%x\n",edx,eax
        mov     ebx, [esp + 16]
        mov     esi, [esp + 20]
        mov     edi, [esp + 24]
;        mov     ecx, [esp + 28] ; nextents
  .next_extent:
;DEBUGF 1,"next_extent\n"
        dec     dword[esp + 28]
        js      .error
        stdcall xfs_extent_unpack, ebx
        add     ebx, sizeof.xfs_bmbt_rec        ; next extent
        mov     edx, dword[ebp + XFS.extent.br_startoff + 4]
        mov     eax, dword[ebp + XFS.extent.br_startoff + 0]
        cmp     edx, [esp + 36] ; max_offset_hi
        ja      .error
        jb      @f
        cmp     eax, [esp + 32] ; max_offset_lo
        jae     .error
    @@:
        cmp     edi, edx
        jb      .hole
        ja      .check_count
        cmp     esi, eax
        jb      .hole
        ja      .check_count
        jmp     .read_block
  .hole:
;DEBUGF 1,"hole\n"
        mov     esi, eax
        mov     edi, edx
        jmp     .read_block
  .check_count:
;DEBUGF 1,"check_count\n"
        add     eax, [ebp + XFS.extent.br_blockcount]
        adc     edx, 0
        cmp     edi, edx
        ja      .next_extent
        jb      .read_block
        cmp     esi, eax
        jae     .next_extent
;        jmp     .read_block
  .read_block:
;DEBUGF 1,"read_block\n"
        push    esi edi
        sub     esi, dword[ebp + XFS.extent.br_startoff + 0]
        sbb     edi, dword[ebp + XFS.extent.br_startoff + 4]
        add     esi, dword[ebp + XFS.extent.br_startblock + 0]
        adc     edi, dword[ebp + XFS.extent.br_startblock + 4]
        stdcall xfs_read_dirblock, esi, edi, [ebp + XFS.cur_dirblock]
        pop     edx eax
  .quit:
;DEBUGF 1,"xfs_extent_list_read_dirblock: quit\n"
        pop     edi esi ebx
        ret     24
  .error:
;DEBUGF 1,"xfs_extent_list_read_dirblock: error\n"
        xor     eax, eax
        dec     eax
        mov     edx, eax
        pop     edi esi ebx
        ret     24


;----------------------------------------------------------------
; push  dirblock_num
; push  nextents
; push  extent_list
;----------------------------------------------------------------
xfs_dir2_node_get_numfiles:

        ; unfortunately, we need to set 'total entries' field
        ; this often requires additional effort, since there is no such a number in most directory ondisk formats

;DEBUGF 1,"xfs_dir2_node_get_numfiles\n"
        push    ebx ecx edx esi edi

        mov     eax, [esp + 24]
        mov     edx, [esp + 28]
        mov     esi, [esp + 32]
        stdcall xfs_extent_list_read_dirblock, eax, esi, 0, edx, -1, -1
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        jnz     @f
        movi    eax, ERROR_FS_FAIL
        jmp     .error
    @@:
        mov     ebx, [ebp + XFS.cur_dirblock]
        cmp     word[ebx + xfs_da_intnode.hdr.info.magic], XFS_DA_NODE_MAGIC
        je      .node
        cmp     word[ebx + xfs_da_intnode.hdr.info.magic], XFS_DIR2_LEAFN_MAGIC
        je      .leaf
        mov     eax, ERROR_FS_FAIL
        jmp     .error

  .node:
;DEBUGF 1,".node\n"
        mov     edi, [ebx + xfs_da_intnode.hdr.info.forw]
        bswap   edi
        mov     eax, [esp + 24]
        mov     edx, [esp + 28]
        mov     esi, [ebx + xfs_da_intnode.btree.before]
        bswap   esi
        stdcall xfs_dir2_node_get_numfiles, eax, edx, esi
        test    eax, eax
        jnz     .error
        jmp     .common
        
  .leaf:
;DEBUGF 1,".leaf\n"
        movzx   ecx, word[ebx + xfs_dir2_leaf.hdr.count]
        xchg    cl, ch
        movzx   eax, word[ebx + xfs_dir2_leaf.hdr.stale]
        xchg    al, ah
        sub     ecx, eax
        add     [ebp + XFS.entries_read], ecx
        mov     edi, [ebx + xfs_dir2_leaf.hdr.info.forw]
        bswap   edi
        jmp     .common

  .common:
        test    edi, edi
        jz      .quit
        mov     esi, edi
        mov     eax, [esp + 24]
        mov     edx, [esp + 28]
        stdcall xfs_dir2_node_get_numfiles, eax, edx, esi
        test    eax, eax
        jnz     .error
        jmp     .quit

  .quit:
;DEBUGF 1,".quit\n"
        pop     edi esi edx ecx ebx
        xor     eax, eax
        ret     12
  .error:
;DEBUGF 1,".error\n"
        pop     edi esi edx ecx ebx
        movi    eax, ERROR_FS_FAIL
        ret     12


;----------------------------------------------------------------
; push  hash
; push  dirblock_num
; push  nextents
; push  extent_list
;----------------------------------------------------------------
xfs_dir2_lookupdir_node:
DEBUGF 1,"xfs_dir2_lookupdir_node\n"
        push    ebx edx esi edi

        mov     eax, [esp + 20]
        mov     edx, [esp + 24]
        mov     esi, [esp + 28]
DEBUGF 1,"read dirblock: 0x%x %d\n",esi,esi
        stdcall xfs_extent_list_read_dirblock, eax, esi, 0, edx, -1, -1
DEBUGF 1,"dirblock read: 0x%x%x\n",edx,eax
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        jnz     @f
        movi    eax, ERROR_FS_FAIL
        jmp     .error
    @@:
DEBUGF 1,"checkpoint #1\n"
        mov     ebx, [ebp + XFS.cur_dirblock]
        cmp     word[ebx + xfs_da_intnode.hdr.info.magic], XFS_DA_NODE_MAGIC
        je      .node
        cmp     word[ebx + xfs_da_intnode.hdr.info.magic], XFS_DIR2_LEAFN_MAGIC
        je      .leaf
        mov     eax, ERROR_FS_FAIL
DEBUGF 1,"checkpoint #2\n"
        jmp     .error

  .node:
DEBUGF 1,".node\n"
        mov     edi, [esp + 32] ; hash
        movzx   ecx, word[ebx + xfs_da_intnode.hdr.count]
        xchg    cl, ch
        mov     [ebp + XFS.left_leaves], ecx
        xor     ecx, ecx
  .node.next_leaf:
        mov     esi, [ebx + xfs_da_intnode.btree + ecx*sizeof.xfs_da_node_entry + xfs_da_node_entry.hashval]
        bswap   esi
        cmp     edi, esi
        jbe     .node.leaf_found
        inc     ecx
        cmp     ecx, [ebp + XFS.left_leaves]
        jne     .node.next_leaf
        mov     eax, ERROR_FILE_NOT_FOUND
        jmp     .error
    @@:
  .node.leaf_found:
        mov     eax, [esp + 20]
        mov     edx, [esp + 24]
        mov     esi, [ebx + xfs_da_intnode.btree + ecx*sizeof.xfs_da_node_entry + xfs_da_node_entry.before]
        bswap   esi
        stdcall xfs_dir2_lookupdir_node, eax, edx, esi, edi
        test    eax, eax
        jz      .quit
        movi    eax, ERROR_FILE_NOT_FOUND
        jmp     .error

  .leaf:
DEBUGF 1,".leaf\n"
        movzx   ecx, [ebx + xfs_dir2_leaf.hdr.count]
        xchg    cl, ch
        lea     esi, [ebx + xfs_dir2_leaf.ents]
        mov     eax, [esp + 32]
        stdcall xfs_get_addr_by_hash, esi, ecx
        cmp     eax, -1
        je      .error
        mov     ecx, eax
        jmp     .quit

  .quit:
DEBUGF 1,".quit\n"
        pop     edi esi edx ebx
        xor     eax, eax
        ret     16
  .error:
DEBUGF 1,".error\n"
        pop     edi esi edx ebx
        ret     16


;----------------------------------------------------------------
; push  dirblock_num
; push  nextents
; push  extent_list
;----------------------------------------------------------------
xfs_dir2_btree_get_numfiles:
;DEBUGF 1,"xfs_dir2_node_get_numfiles\n"
        push    ebx ecx edx esi edi

        mov     eax, [esp + 24]
        mov     edx, [esp + 28]
        mov     esi, [esp + 32]
        stdcall xfs_extent_list_read_dirblock, eax, esi, 0, edx, -1, -1
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        jnz     @f
        movi    eax, ERROR_FS_FAIL
        jmp     .error
    @@:
        mov     ebx, [ebp + XFS.cur_dirblock]
        cmp     word[ebx + xfs_da_intnode.hdr.info.magic], XFS_DA_NODE_MAGIC
        je      .node
        cmp     word[ebx + xfs_da_intnode.hdr.info.magic], XFS_DIR2_LEAFN_MAGIC
        je      .leaf
        mov     eax, ERROR_FS_FAIL
        jmp     .error

  .node:
;DEBUGF 1,".node\n"
        mov     edi, [ebx + xfs_da_intnode.hdr.info.forw]
        bswap   edi
        mov     eax, [esp + 24]
        mov     edx, [esp + 28]
        mov     esi, [ebx + xfs_da_intnode.btree.before]
        bswap   esi
        stdcall xfs_dir2_node_get_numfiles, eax, edx, esi
        test    eax, eax
        jnz     .error
        jmp     .common
        
  .leaf:
;DEBUGF 1,".leaf\n"
        movzx   ecx, word[ebx + xfs_dir2_leaf.hdr.count]
        xchg    cl, ch
        movzx   eax, word[ebx + xfs_dir2_leaf.hdr.stale]
        xchg    al, ah
        sub     ecx, eax
        add     [ebp + XFS.entries_read], ecx
        mov     edi, [ebx + xfs_dir2_leaf.hdr.info.forw]
        bswap   edi
        jmp     .common

  .common:
        test    edi, edi
        jz      .quit
        mov     esi, edi
        mov     eax, [esp + 24]
        mov     edx, [esp + 28]
        stdcall xfs_dir2_node_get_numfiles, eax, edx, esi
        test    eax, eax
        jnz     .error
        jmp     .quit

  .quit:
;DEBUGF 1,".quit\n"
        pop     edi esi edx ecx ebx
        xor     eax, eax
        ret     12
  .error:
;DEBUGF 1,".error\n"
        pop     edi esi edx ecx ebx
        movi    eax, ERROR_FS_FAIL
        ret     12


;----------------------------------------------------------------
; push  is_root
; push  block_hi
; push  block_lo
;----------------------------------------------------------------
xfs_btree_read:
        push    ebx ecx edx esi edi
        cmp     dword[esp + 32], 1      ; is root?
        je      .root
        jmp     .not_root
  .root:
DEBUGF 1,".root\n"
        mov     ebx, [ebp + XFS.cur_inode_save]
        add     ebx, xfs_inode.di_u
        movzx   edx, [ebx + xfs_bmdr_block.bb_numrecs]
        xchg    dl, dh
        dec     edx
        add     ebx, sizeof.xfs_bmdr_block
        xor     eax, eax
        dec     eax
 .root.next_key:
DEBUGF 1,".root.next_key\n"
        cmp     [ebp + XFS.bytes_to_read], 0
        je      .quit
        inc     eax
        cmp     eax, edx        ; out of keys?
        ja      .root.key_found ; there is no length field, so try the last key
        lea     edi, [ebx + sizeof.xfs_bmbt_key*eax + 0]
        lea     esi, [ebx + sizeof.xfs_bmbt_key*eax + 4]
        bswap   edi
        bswap   esi
        mov     ecx, [ebp + XFS.blocklog]
        shld    edi, esi, cl
        shl     esi, cl
        cmp     edi, dword[ebp + XFS.file_offset + 4]
        ja      .root.prev_or_hole
        jb      .root.next_key
        cmp     esi, dword[ebp + XFS.file_offset + 0]
        ja      .root.prev_or_hole
        jb      .root.next_key
        jmp     .root.key_found
  .root.prev_or_hole:
DEBUGF 1,".root.prev_or_hole\n"
        test    eax, eax
        jz      .root.hole
        dec     eax
        jmp     .root.key_found
  .root.hole:
DEBUGF 1,".root.hole\n"
        push    eax edx esi edi
        mov     ecx, [ebp + XFS.blocklog]
        shld    edi, esi, cl
        shl     esi, cl
        sub     esi, dword[ebp + XFS.file_offset + 0]
        sbb     edi, dword[ebp + XFS.file_offset + 4]
        mov     ecx, [ebp + XFS.bytes_to_read]
        cmp     edi, 0  ; hole size >= 2^32
        jne     @f
        cmp     ecx, esi
        jbe     @f
        mov     ecx, esi
    @@:
        add     dword[ebp + XFS.file_offset + 0], ecx
        adc     dword[ebp + XFS.file_offset + 4], 0
        sub     [ebp + XFS.bytes_to_read], ecx
        xor     eax, eax
        mov     edi, [ebp + XFS.buffer_pos]
        rep stosb
        mov     [ebp + XFS.buffer_pos], edi
        pop     edi esi edx eax
        jmp     .root.next_key
  .root.key_found:
DEBUGF 1,".root.key_found\n"
        mov     edx, [ebp + XFS.cur_inode_save]
        mov     eax, [ebp + XFS.inodesize]
        sub     eax, xfs_inode.di_u
        cmp     [edx + xfs_inode.di_core.di_forkoff], 0
        je      @f
        movzx   eax, [edx + xfs_inode.di_core.di_forkoff]
        shl     eax, XFS_DIR2_DATA_ALIGN_LOG    ; 3
    @@:
        sub     eax, sizeof.xfs_bmdr_block
        shr     eax, 4  ;log2(sizeof.xfs_bmbt_key + sizeof.xfs_bmdr_ptr)
        mov     edx, [ebx + sizeof.xfs_bmbt_key*eax + 0]        ; hi
        mov     eax, [ebx + sizeof.xfs_bmbt_key*eax + 4]        ; hi
        bswap   edx
        bswap   eax
        stdcall xfs_btree_read, eax, edx, 0
        test    eax, eax
        jnz     .error
        jmp     .root.next_key

  .not_root:
DEBUGF 1,".root.not_root\n"
        mov     eax, [esp + 24] ; block_lo
        mov     edx, [esp + 28] ; block_hi
        mov     ebx, [ebp + XFS.cur_block]
        stdcall xfs_read_block
        test    eax, eax
        jnz     .error
        mov     ebx, [ebp + XFS.cur_block]

        cmp     [ebx + xfs_bmbt_block.bb_magic], XFS_BMAP_MAGIC
        jne     .error
        cmp     [ebx + xfs_bmbt_block.bb_level], 0      ; leaf?
        je      .leaf
        jmp     .node

  .node:
;        mov     eax, [ebp + XFS.blocksize]
;        sub     eax, sizeof.xfs_bmbt_block
;        shr     eax, 4  ; maxnumrecs
        mov     eax, dword[ebp + XFS.file_offset + 0]   ; lo
        mov     edx, dword[ebp + XFS.file_offset + 4]   ; hi
        movzx   edx, [ebx + xfs_bmbt_block.bb_numrecs]
        xchg    dl, dh
        dec     edx
        add     ebx, sizeof.xfs_bmbt_block
        xor     eax, eax
        dec     eax
  .node.next_key:
        push    eax ecx edx esi edi
        mov     eax, [esp + 44] ; block_lo
        mov     edx, [esp + 48] ; block_hi
        mov     ebx, [ebp + XFS.cur_block]
        stdcall xfs_read_block
        test    eax, eax
        jnz     .error
        mov     ebx, [ebp + XFS.cur_block]
        add     ebx, sizeof.xfs_bmbt_block
        pop     edi esi edx ecx eax
        cmp     [ebp + XFS.bytes_to_read], 0
        je      .quit
        inc     eax
        cmp     eax, edx        ; out of keys?
        ja      .node.key_found ; there is no length field, so try the last key
        lea     edi, [ebx + sizeof.xfs_bmbt_key*eax + 0]
        lea     esi, [ebx + sizeof.xfs_bmbt_key*eax + 4]
        bswap   edi
        bswap   esi
        mov     ecx, [ebp + XFS.blocklog]
        shld    edi, esi, cl
        shl     esi, cl
        cmp     edi, dword[ebp + XFS.file_offset + 4]
        ja      .node.prev_or_hole
        jb      .node.next_key
        cmp     esi, dword[ebp + XFS.file_offset + 0]
        ja      .node.prev_or_hole
        jb      .node.next_key
        jmp     .node.key_found
  .node.prev_or_hole:
        test    eax, eax
        jz      .node.hole
        dec     eax
        jmp     .node.key_found
  .node.hole:
        push    eax edx esi edi
        mov     ecx, [ebp + XFS.blocklog]
        shld    edi, esi, cl
        shl     esi, cl
        sub     esi, dword[ebp + XFS.file_offset + 0]
        sbb     edi, dword[ebp + XFS.file_offset + 4]
        mov     ecx, [ebp + XFS.bytes_to_read]
        cmp     edi, 0  ; hole size >= 2^32
        jne     @f
        cmp     ecx, esi
        jbe     @f
        mov     ecx, esi
    @@:
        add     dword[ebp + XFS.file_offset + 0], ecx
        adc     dword[ebp + XFS.file_offset + 4], 0
        sub     [ebp + XFS.bytes_to_read], ecx
        xor     eax, eax
        mov     edi, [ebp + XFS.buffer_pos]
        rep stosb
        mov     [ebp + XFS.buffer_pos], edi
        pop     edi esi edx eax
        jmp     .node.next_key
  .node.key_found:
        mov     edx, [ebp + XFS.cur_inode_save]
        mov     eax, [ebp + XFS.inodesize]
        sub     eax, xfs_inode.di_u
        cmp     [edx + xfs_inode.di_core.di_forkoff], 0
        je      @f
        movzx   eax, [edx + xfs_inode.di_core.di_forkoff]
        shl     eax, XFS_DIR2_DATA_ALIGN_LOG    ; 3
    @@:
        sub     eax, sizeof.xfs_bmdr_block
        shr     eax, 4  ;log2(sizeof.xfs_bmbt_key + sizeof.xfs_bmdr_ptr)
        mov     edx, [ebx + sizeof.xfs_bmbt_key*eax + 0]        ; hi
        mov     eax, [ebx + sizeof.xfs_bmbt_key*eax + 4]        ; hi
        bswap   edx
        bswap   eax
        stdcall xfs_btree_read, eax, edx, 0
        test    eax, eax
        jnz     .error
        jmp     .node.next_key
        jmp     .quit

  .leaf:
        
        jmp     .quit

  .error:
        pop     edi esi edx ecx ebx
        movi    eax, ERROR_FS_FAIL
        ret     4
  .quit:
        pop     edi esi edx ecx ebx
        xor     eax, eax
        ret     4


;----------------------------------------------------------------
; push  nextents
; push  extent_list
; push  file_offset_hi
; push  file_offset_lo
;----------------------------------------------------------------
;xfs_extent_list_read:
;        push    ebx 0 edx esi edi       ; zero means actually_read_bytes
;
;  .quit:
;        pop     edi esi edx ecx ebx
;        xor     eax, eax
;        ret     24
;  .error:
;        pop     edi esi edx ecx ebx
;        ret     24
