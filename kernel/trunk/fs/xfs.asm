;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2022. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License   ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


include 'xfs.inc'

macro omit_frame_pointer_prologue procname,flag,parmbytes,localbytes,reglist {
  local loc
  loc = (localbytes+3) and (not 3)
  if localbytes
        sub     esp, loc
  end if
  irps reg, reglist \{ push reg \}
  counter = 0
  irps reg, reglist \{counter = counter+1 \}
  parmbase@proc equ esp+counter*4+loc+4
  localbase@proc equ esp
}

macro omit_frame_pointer_epilogue procname,flag,parmbytes,localbytes,reglist {
  local loc
  loc = (localbytes+3) and (not 3)
  irps reg, reglist \{ reverse pop reg \}
  if localbytes
        lea     esp, [esp+loc]
  end if
  if flag and 10000b
        retn
  else
        retn    parmbytes
  end if
}

prologue@proc equ omit_frame_pointer_prologue
epilogue@proc equ omit_frame_pointer_epilogue

macro movbe reg, arg {
 if CPUID_MOVBE eq Y
        movbe   reg, arg
 else
        mov     reg, arg
  if reg in <eax,ebx,ecx,edx,esi,edi,ebp,esp>
        bswap   reg
  else if ax eq reg
        xchg    al, ah
  else if bx eq reg
        xchg    bl, bh
  else if cx eq reg
        xchg    cl, ch
  else if dx eq reg
        xchg    dl, dh
  else
   err
  end if
 end if
}

;
; This file contains XFS related code.
; For more information on XFS check links and source below.
;
; 1. https://xfs.wiki.kernel.org/
;
; 2. XFS Algorithms & Data Structures:
;    git://git.kernel.org/pub/scm/fs/xfs/xfs-documentation.git
;    https://mirrors.edge.kernel.org/pub/linux/utils/fs/xfs/docs/xfs_filesystem_structure.pdf
;
; 3. Linux source at https://www.kernel.org/
;    git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
;    /fs/xfs
;

iglobal
align 4
xfs._.user_functions:
        dd      xfs._.free
        dd      (xfs._.user_functions_end-xfs._.user_functions-8)/4
        dd      xfs_Read
        dd      xfs_ReadFolder
        dd      0;xfs_Rewrite
        dd      0;xfs_Write
        dd      0;xfs_SetFileEnd
        dd      xfs_GetFileInfo
xfs._.user_functions_end:
endg

; test partition type (valid XFS one?)
; alloc and fill XFS (see xfs.inc) structure
; this function is called for each partition
; return 0 (not XFS or invalid) / pointer to partition structure
proc xfs_create_partition uses ebx esi edi
        ; check XFS signature
        cmp     [ebx+xfs_sb.sb_magicnum], XFS_SB_MAGIC
        jnz     .error_nofree
        ; test for supported feature flags and version in sb_versionnum
        movzx   eax, [ebx+xfs_sb.sb_versionnum]
        xchg    al, ah
        ; allow only known and supported features
        ; return error otherwise
        test    eax, NOT XFS_SB_VERSION_SUPPORTED
        jnz     .error_nofree
        ; version < 4 obsolete, not supported
        ; version = 4,5 supported
        ; version > 5 unknown
        and     al, XFS_SB_VERSION_NUMBITS
        cmp     al, 4
        jb      .error_nofree
        cmp     al, 5
        ja      .error_nofree
        ; if MOREBITS bit is set, additional feature flags are in sb_features2
        test    eax, XFS_SB_VERSION_MOREBITSBIT
        jz      @f
        movbe   eax, [ebx+xfs_sb.sb_features2]
        test    eax, NOT XFS_SB_VERSION2_SUPPORTED
        jnz     .error_nofree
@@:
        movbe   eax, [ebx+xfs_sb.sb_features_incompat]
        test    eax, NOT XFS_SB_FEAT_INCOMPAT_SUPPORTED
        jnz     .error_nofree
        ; all presented features are either supported or don't affect reading
        movi    eax, sizeof.XFS
        call    malloc
        mov     edi, eax
        test    eax, eax
        jz      .error

        ; standard partition initialization, common for all file systems
        mov     eax, dword[ebp+PARTITION.FirstSector+DQ.lo]
        mov     dword[edi+XFS.FirstSector+DQ.lo], eax
        mov     eax, dword[ebp+PARTITION.FirstSector+DQ.hi]
        mov     dword[edi+XFS.FirstSector+DQ.hi], eax
        mov     eax, dword[ebp+PARTITION.Length+DQ.lo]
        mov     dword[edi+XFS.Length+DQ.lo], eax
        mov     eax, dword[ebp+PARTITION.Length+DQ.hi]
        mov     dword[edi+XFS.Length+DQ.hi], eax
        mov     eax, [ebp+PARTITION.Disk]
        mov     [edi+XFS.Disk], eax
        mov     [edi+XFS.FSUserFunctions], xfs._.user_functions
        ; here we initialize only one mutex (for the entire partition)
        ; XFS potentially allows parallel r/w access to different AGs, keep it in mind
        lea     ecx, [edi+XFS.Lock]
        call    mutex_init

;        movzx   eax, [ebx+xfs_sb.sb_sectsize]
;        xchg    al, ah
        mov     eax, [eax+DISK.MediaInfo.SectorSize]
        mov     [edi+XFS.sectsize], eax

        movbe   eax, [ebx+xfs_sb.sb_blocksize]
        mov     [edi+XFS.blocksize], eax

        movzx   eax, [ebx+xfs_sb.sb_versionnum]
        xchg    al, ah
        mov     [edi+XFS.versionnum], eax
        and     eax, XFS_SB_VERSION_NUMBITS
        mov     [edi+XFS.version], eax

        mov     [edi+XFS.conv_time_to_kos_epoch], xfs._.conv_time_to_kos_epoch
        mov     [edi+XFS.nextents_offset], xfs_inode.di_core.di_nextents

        movbe   eax, [ebx+xfs_sb.sb_features2]
        mov     [edi+XFS.features2], eax
        cmp     [edi+XFS.version], 5
        jz      .v5
.v4:
        mov     [edi+XFS.inode_core_size], sizeof.xfs_dinode_core
        test    eax, XFS_SB_VERSION2_FTYPE
        setnz   al
        movzx   eax, al
        mov     [edi+XFS.ftype_size], eax
        mov     [edi+XFS.dir_block_magic], XFS_DIR2_BLOCK_MAGIC
        mov     [edi+XFS.dir_data_magic], XFS_DIR2_DATA_MAGIC
        mov     [edi+XFS.dir_leaf1_magic], XFS_DIR2_LEAF1_MAGIC
        mov     [edi+XFS.dir_leafn_magic], XFS_DIR2_LEAFN_MAGIC
        mov     [edi+XFS.da_node_magic], XFS_DA_NODE_MAGIC
        mov     [edi+XFS.bmap_magic], XFS_BMAP_MAGIC
        mov     [edi+XFS.dirx_leaf_ents_offset], xfs_dir2_leaf.ents
        mov     [edi+XFS.dirx_leaf_hdr_count_offset], xfs_dir2_leaf_hdr.count
        mov     [edi+XFS.dir_block_size], sizeof.xfs_dir2_data_hdr
        mov     [edi+XFS.bmbt_block_size], sizeof.xfs_bmbt_block
        mov     [edi+XFS.da_blkinfo_size], sizeof.xfs_da_blkinfo
        jmp     .vcommon
.v5:
        mov     [edi+XFS.inode_core_size], sizeof.xfs_dinode3_core
        movbe   eax, [ebx+xfs_sb.sb_features_incompat]
        mov     [edi+XFS.features_incompat], eax
        test    eax, XFS_SB_FEAT_INCOMPAT_FTYPE
        setnz   al
        movzx   eax, al
        mov     [edi+XFS.ftype_size], eax
        mov     [edi+XFS.dir_block_magic], XFS_DIR3_BLOCK_MAGIC
        mov     [edi+XFS.dir_data_magic], XFS_DIR3_DATA_MAGIC
        mov     [edi+XFS.dir_leaf1_magic], XFS_DIR3_LEAF1_MAGIC
        mov     [edi+XFS.dir_leafn_magic], XFS_DIR3_LEAFN_MAGIC
        mov     [edi+XFS.da_node_magic], XFS_DA3_NODE_MAGIC
        mov     [edi+XFS.bmap_magic], XFS_BMAP3_MAGIC
        mov     [edi+XFS.dirx_leaf_ents_offset], xfs_dir3_leaf.ents
        mov     [edi+XFS.dirx_leaf_hdr_count_offset], xfs_dir3_leaf_hdr.count
        mov     [edi+XFS.dir_block_size], sizeof.xfs_dir3_data_hdr
        mov     [edi+XFS.bmbt_block_size], sizeof.xfs_bmbt3_block
        mov     [edi+XFS.da_blkinfo_size], sizeof.xfs_da3_blkinfo
        test    [edi+XFS.features_incompat], XFS_SB_FEAT_INCOMPAT_BIGTIME
        jz      @f      ; no bigtime
        mov     [edi+XFS.conv_time_to_kos_epoch], xfs._.conv_bigtime_to_kos_epoch
@@:
        test    [edi+XFS.features_incompat], XFS_SB_FEAT_INCOMPAT_NREXT64
        jz      @f      ; no bigtime
        mov     [edi+XFS.nextents_offset], xfs_inode.di_core.di_big_nextents.lo_be
@@:
.vcommon:

        movzx   eax, [ebx+xfs_sb.sb_inodesize]
        xchg    al, ah
        mov     [edi+XFS.inodesize], eax

        movzx   eax, [ebx+xfs_sb.sb_inopblock]
        xchg    al, ah
        mov     [edi+XFS.inopblock], eax

        movzx   eax, [ebx+xfs_sb.sb_blocklog]
        mov     [edi+XFS.blocklog], eax

;        movzx   eax, [ebx+xfs_sb.sb_sectlog]
        mov     eax, [edi+XFS.sectsize]
        bsf     eax, eax
        mov     [edi+XFS.sectlog], eax

        movzx   eax, [ebx+xfs_sb.sb_inodelog]
        mov     [edi+XFS.inodelog], eax

        movzx   eax, [ebx+xfs_sb.sb_inopblog]
        mov     [edi+XFS.inopblog], eax

        movzx   ecx, [ebx+xfs_sb.sb_dirblklog]
        mov     [edi+XFS.dirblklog], ecx
        movi    eax, 1
        shl     eax, cl
        mov     [edi+XFS.blkpdirblk], eax

        movbe   eax, [ebx+xfs_sb.sb_rootino.hi]
        mov     [edi+XFS.rootino.lo], eax
        movbe   eax, [ebx+xfs_sb.sb_rootino.lo]
        mov     [edi+XFS.rootino.hi], eax

        mov     eax, [edi+XFS.blocksize]
        mov     ecx, [edi+XFS.dirblklog]
        shl     eax, cl
        mov     [edi+XFS.dirblocksize], eax           ; blocks are for files, dirblocks are for directories

        ; sector is always smaller than block
        ; so precalculate shift order to allow faster sector_num->block_num conversion
        mov     ecx, [edi+XFS.blocklog]
        sub     ecx, [edi+XFS.sectlog]
        mov     [edi+XFS.sectpblog], ecx

        mov     eax, 1
        shl     eax, cl
        mov     [edi+XFS.sectpblock], eax

        movbe   eax, [ebx+xfs_sb.sb_agblocks]
        mov     [edi+XFS.agblocks], eax

        movzx   ecx, [ebx+xfs_sb.sb_agblklog]
        mov     [edi+XFS.agblklog], ecx

        ; get the mask for block numbers
        ; block numbers are AG relative!
        ; bitfield length may vary between partitions
        mov     eax, 1
        xor     edx, edx
        shld    edx, eax, cl
        shl     eax, cl
        sub     eax, 1
        sbb     edx, 0
        mov     [edi+XFS.agblockmask.lo], eax
        mov     [edi+XFS.agblockmask.hi], edx

        ; calculate magic offsets for directories
        mov     ecx, [edi+XFS.blocklog]
        mov     eax, XFS_DIR2_LEAF_OFFSET AND 0xffffffff        ; lo
        mov     edx, XFS_DIR2_LEAF_OFFSET SHR 32                ; hi
        shrd    eax, edx, cl
        shr     edx, cl
        mov     [edi+XFS.dir2_leaf_offset_blocks.lo], eax
        mov     [edi+XFS.dir2_leaf_offset_blocks.hi], edx

        mov     ecx, [edi+XFS.blocklog]
        mov     eax, XFS_DIR2_FREE_OFFSET AND 0xffffffff        ; lo
        mov     edx, XFS_DIR2_FREE_OFFSET SHR 32                ; hi
        shrd    eax, edx, cl
        shr     edx, cl
        mov     [edi+XFS.dir2_free_offset_blocks.lo], eax
        mov     [edi+XFS.dir2_free_offset_blocks.hi], edx


        ; allocate memory for temp block, dirblock, inode, etc
        mov     eax, [edi+XFS.blocksize]
        call    malloc
        mov     [edi+XFS.cur_block], eax
        test    eax, eax
        jz      .error

        mov     eax, [edi+XFS.blocksize]
        call    malloc
        mov     [edi+XFS.cur_block_data], eax
        test    eax, eax
        jz      .error

        ; we do need XFS.blocksize bytes for single inode
        ; minimal file system structure is block, inodes are packed in blocks
        mov     eax, [edi+XFS.blocksize]
        call    malloc
        mov     [edi+XFS.cur_inode], eax
        test    eax, eax
        jz      .error

        mov     eax, [edi+XFS.blocksize]
        call    malloc
        test    eax, eax
        jz      .error
        mov     [edi+XFS.tmp_inode], eax

        ; current sector
        ; only for sector sized structures like AGF
        ; inodes usually fit this size, but not always!
        ; therefore never store inode here
        mov     eax, [edi+XFS.sectsize]
        call    malloc
        mov     [edi+XFS.cur_sect], eax
        test    eax, eax
        jz      .error

        mov     eax, [edi+XFS.dirblocksize]
        call    malloc
        mov     [edi+XFS.cur_dirblock], eax
        test    eax, eax
        jz      .error

.quit:
        ; return pointer to allocated XFS partition structure
        mov     eax, edi
        ret
.error:
        mov     eax, edi
        call    xfs._.free
.error_nofree:
        xor     eax, eax
        ret
endp


; lock partition access mutex
xfs._.lock:
        lea     ecx, [ebp+XFS.Lock]
        jmp     mutex_lock


; unlock partition access mutex
xfs._.unlock:
        lea     ecx, [ebp+XFS.Lock]
        jmp     mutex_unlock


; free all the allocated memory
; called on partition destroy
; or during failed initialization from xfs_create_partition
xfs._.free:
        test    eax, eax
        jz      .done
        push    ebx
        mov     ebx, eax


        ; freeing order must correspond the order of
        ; allocation in xfs_create_partition
        mov     eax, [ebx+XFS.cur_block]
        test    eax, eax
        jz      .done
        call    free

        mov     eax, [ebx+XFS.cur_block_data]
        test    eax, eax
        jz      .done
        call    free

        mov     eax, [ebx+XFS.cur_inode]
        test    eax, eax
        jz      .done
        call    free

        mov     eax, [ebx+XFS.tmp_inode]
        test    eax, eax
        jz      .done
        call    free

        mov     eax, [ebx+XFS.cur_sect]
        test    eax, eax
        jz      .done
        call    free

        mov     eax, [ebx+XFS.cur_dirblock]
        test    eax, eax
        jz      .done
        call    free


        mov     eax, ebx
        call    free
        pop     ebx
.done:
        ret


;---------------------------------------------------------------
; block number
; eax -- inode_lo
; edx -- inode_hi
; ebx -- buffer
;---------------------------------------------------------------
proc xfs._.read_block
        movi    ecx, 1
        call    xfs._.read_blocks
        ret
endp


proc xfs._.blkrel2sectabs uses esi
        push    edx eax

        ; XFS block numbers are AG relative
        ; they come in bitfield form of concatenated AG and block numbers
        ; to get absolute block number for fs_read64_sys we should
        ; 1. get AG number and multiply it by the AG size in blocks
        ; 2. extract and add AG relative block number

        ; 1.
        mov     ecx, [ebp+XFS.agblklog]
        shrd    eax, edx, cl
        shr     edx, cl
        mul     [ebp+XFS.agblocks]
        ; 2.
        pop     ecx esi
        and     ecx, [ebp+XFS.agblockmask.lo]
        and     esi, [ebp+XFS.agblockmask.hi]
        add     eax, ecx
        adc     edx, esi

        mov     ecx, [ebp+XFS.sectpblog]
        shld    edx, eax, cl
        shl     eax, cl
        ret
endp


;---------------------------------------------------------------
; start block number
; edx:eax -- block
; ebx -- buffer
; ecx -- count
;---------------------------------------------------------------
proc xfs._.read_blocks
        push    ecx
        call    xfs._.blkrel2sectabs
        pop     ecx
        imul    ecx, [ebp+XFS.sectpblock]
        call    fs_read64_sys
        test    eax, eax
        ret
endp


proc xfs._.read_dirblock uses ebx, _startblock:qword, _buffer
        mov     eax, dword[_startblock+DQ.lo]
        mov     edx, dword[_startblock+DQ.hi]
        mov     ebx, [_buffer]
        mov     ecx, [ebp+XFS.blkpdirblk]
        call    xfs._.read_blocks
        ret
endp


;---------------------------------------------------------------
; test eax, eax
;---------------------------------------------------------------
proc xfs_read_inode uses ebx, _inode_lo, _inode_hi, _buffer
        mov     eax, [_inode_lo]
        mov     edx, [_inode_hi]
        mov     ebx, [_buffer]
        ; inodes are packed into blocks
        ; 1. calculate block number
        ; 2. read the block
        ; 3. add inode offset to block base address
        ; 1.
        mov     ecx, [ebp+XFS.inopblog]
        shrd    eax, edx, cl
        shr     edx, cl
        ; 2.
        call    xfs._.read_block
        jnz     .error
        ; inode numbers should be first extracted from bitfields by mask

        mov     eax, [_inode_lo]
        mov     edx, 1
        mov     ecx, [ebp+XFS.inopblog]
        shl     edx, cl
        dec     edx             ; get inode number mask
        and     eax, edx        ; apply mask
        mov     ecx, [ebp+XFS.inodelog]
        shl     eax, cl
        add     ebx, eax
        xor     eax, eax

        cmp     [ebx+xfs_inode.di_core.di_magic], XFS_DINODE_MAGIC
        jz      .quit
        movi    eax, ERROR_FS_FAIL
.quit:
        mov     edx, ebx
.error:
        ret
endp


; skip ecx first entries
proc xfs._.dir_sf_skip _count
        mov     ecx, [_count]
.next:
        dec     ecx
        js      .quit
        dec     [ebp+XFS.entries_left_in_dir]
        js      .quit
.self:
        bts     [ebp+XFS.dir_sf_self_done], 0
        jc      .parent
        jmp     .next
.parent:
        bts     [ebp+XFS.dir_sf_parent_done], 0
        jc      .common
        jmp     .next
.common:
        movzx   eax, [esi+xfs_dir2_sf_entry.namelen]
        add     esi, xfs_dir2_sf_entry.name
        add     esi, eax
        add     esi, [ebp+XFS.ftype_size]
        add     esi, [ebp+XFS.shortform_inodelen]
        jmp     .next
.quit:
        ret
endp


proc xfs._.dir_sf_read uses edi, _count
locals
        _dst dd ?
endl
.next:
        dec     [_count]
        js      .quit
        dec     [ebp+XFS.entries_left_in_dir]
        js      .quit
        mov     [_dst], edx
.self:
        bts     [ebp+XFS.dir_sf_self_done], 0
        jc      .parent
        lea     edi, [edx+bdfe.name]
        mov     dword[edi], '.'
        stdcall xfs_get_inode_info, [ebp+XFS.cur_inode], edx
        jmp     .common
.parent:
        bts     [ebp+XFS.dir_sf_parent_done], 0
        jc      .not_special
        lea     edi, [edx+bdfe.name]         ; get file name offset
        mov     dword[edi], '..'        ; terminator included
        mov     edi, edx
        lea     edx, [ebx+xfs_dir2_sf.hdr.parent]
        call    xfs._.get_inode_number_sf
        stdcall xfs_read_inode, eax, edx, [ebp+XFS.tmp_inode]
        test    eax, eax
        jnz     .error
        stdcall xfs_get_inode_info, edx, edi
        jmp     .common
.not_special:
        movzx   ecx, [esi+xfs_dir2_sf_entry.namelen]
        add     esi, xfs_dir2_sf_entry.name
        lea     edi, [edx+bdfe.name]
        stdcall xfs._.copy_filename
        add     esi, [ebp+XFS.ftype_size]
        mov     edi, edx
        mov     edx, esi
        call    xfs._.get_inode_number_sf
        stdcall xfs_read_inode, eax, edx, [ebp+XFS.tmp_inode]
        test    eax, eax
        jnz     .error
        stdcall xfs_get_inode_info, edx, edi
        add     esi, [ebp+XFS.shortform_inodelen]
.common:
        mov     edx, [_dst]
        mov     eax, [ebp+XFS.bdfe_nameenc]
        mov     [edx+bdfe.nameenc], eax
        add     edx, [ebp+XFS.bdfe_len]
        inc     [ebp+XFS.entries_read]
        jmp     .next
.quit:
        xor     eax, eax
.error:
        ret
endp


proc xfs._.readdir_sf uses esi, _src, _dst
        mov     ebx, [_src]
        mov     edx, [_dst]
        mov     [ebp+XFS.dir_sf_self_done], 0
        mov     [ebp+XFS.dir_sf_parent_done], 0
        mov     [ebp+XFS.entries_read], 0
        movzx   eax, [ebx+xfs_dir2_sf.hdr.count]
        ; '..' and '.' are implicit
        add     eax, 2
        mov     [ebp+XFS.entries_left_in_dir], eax
        mov     [edx+bdfe_hdr.total_cnt], eax
        ; inode numbers are often saved as 4 bytes (iff they fit)
        ; compute the length of inode numbers
        ; 8 iff i8count != 0, 4 otherwise
        cmp     [ebx+xfs_dir2_sf.hdr.i8count], 0
        setnz   al
        lea     eax, [eax*4+4]
        mov     [ebp+XFS.shortform_inodelen], eax
        add     edx, sizeof.bdfe_hdr
        lea     esi, [ebx+xfs_dir2_sf.hdr.parent+eax]
        stdcall xfs._.dir_sf_skip, [ebp+XFS.entries_to_skip]
        stdcall xfs._.dir_sf_read, [ebp+XFS.requested_cnt]
        ret
endp


proc xfs._.readdir_block _literal_area, _out_buf
        mov     ebx, [_literal_area]
        mov     [ebp+XFS.entries_read], 0
        mov     eax, ebx
        mov     ebx, [ebp+XFS.cur_dirblock]
        stdcall xfs._.extent_unpack, eax
        stdcall xfs._.read_dirblock, [ebp+XFS.extent.br_startblock.lo], [ebp+XFS.extent.br_startblock.hi], ebx
        mov     edx, [_out_buf]
        jnz     .error
        mov     eax, [ebp+XFS.dir_block_magic]
        cmp     [ebx+xfs_dir2_block.hdr.magic], eax
        movi    eax, ERROR_FS_FAIL
        jnz     .error
        mov     eax, [ebp+XFS.dirblocksize]
        movbe   ecx, [ebx+eax-sizeof.xfs_dir2_block_tail+xfs_dir2_block_tail.stale]
        movbe   eax, [ebx+eax-sizeof.xfs_dir2_block_tail+xfs_dir2_block_tail.count]
        sub     eax, ecx        ; actual number of entries = count - stale
        mov     [ebp+XFS.entries_left_in_dir], eax
        mov     [edx+bdfe_hdr.total_cnt], eax

        add     ebx, [ebp+XFS.dir_block_size]
        add     edx, sizeof.bdfe_hdr
        mov     [_out_buf], edx
        lea     edi, [_out_buf]
.next:
        movi    eax, ERROR_SUCCESS
        cmp     [ebp+XFS.requested_cnt], 0
        jz      .quit
        cmp     [ebp+XFS.entries_left_in_dir], 0
        jz      .quit
        stdcall xfs._.dir_entry_skip_read, edi
        jz      .next
.error:
.quit:
        ret
endp


proc xfs._.readdir_leaf_node uses esi, _inode_data, _out_buf
        mov     ebx, [_inode_data]
        mov     edx, [_out_buf]
        mov     [ebp+XFS.cur_inode_save], ebx
        mov     [ebp+XFS.entries_read], 0
        mov     eax, ebx
        add     eax, [ebp+XFS.inode_core_size]
        mov     edx, [ebp+XFS.nextents_offset]
        movbe   edx, [ebx+edx]
        mov     ecx, [ebp+XFS.dir2_leaf_offset_blocks.lo]
        mov     [ebp+XFS.offset_begin.lo], ecx
        mov     ecx, [ebp+XFS.dir2_leaf_offset_blocks.hi]
        mov     [ebp+XFS.offset_begin.hi], ecx
        mov     ecx, [ebp+XFS.dir2_free_offset_blocks.lo]
        mov     [ebp+XFS.offset_end.lo], ecx
        mov     ecx, [ebp+XFS.dir2_free_offset_blocks.hi]
        mov     [ebp+XFS.offset_end.hi], ecx
        stdcall xfs._.walk_extent_list, edx, eax, xfs._.extent_iterate_dirblocks, xfs._.leafn_calc_entries, 0
        jnz     .error
        mov     eax, [ebp+XFS.entries_read]
        mov     edx, [_out_buf]
        mov     [edx+bdfe_hdr.total_cnt], eax
        mov     [ebp+XFS.entries_left_in_dir], eax
        add     [_out_buf], sizeof.bdfe_hdr
        mov     [ebp+XFS.entries_read], 0
        mov     edx, [ebp+XFS.nextents_offset]
        movbe   edx, [ebx+edx]
        mov     eax, ebx
        add     eax, [ebp+XFS.inode_core_size]
        lea     ecx, [_out_buf]
        push    ecx
        mov     [ebp+XFS.offset_begin.lo], 0
        mov     [ebp+XFS.offset_begin.hi], 0
        mov     ecx, [ebp+XFS.dir2_leaf_offset_blocks.lo]
        mov     [ebp+XFS.offset_end.lo], ecx
        mov     ecx, [ebp+XFS.dir2_leaf_offset_blocks.hi]
        mov     [ebp+XFS.offset_end.hi], ecx
        pop     ecx
        stdcall xfs._.walk_extent_list, edx, eax, xfs._.extent_iterate_dirblocks, xfs._.dir_btree_skip_read, ecx
;        jnz     .error
.error:
.quit:
        ret
endp


proc xfs._.dir_entry_skip_read uses esi edi, _arg
        cmp     [ebx+xfs_dir2_data_union.unused.freetag], XFS_NULL
        jnz     @f
        movzx   eax, [ebx+xfs_dir2_data_union.unused.length]
        xchg    al, ah
        add     ebx, eax
        jmp     .quit
@@:
        cmp     [ebp+XFS.entries_to_skip], 0
        jz      .read
.skip:
        dec     [ebp+XFS.entries_to_skip]
        movzx   ecx, [ebx+xfs_dir2_data_union.xentry.namelen]
        lea     ebx, [ebx+xfs_dir2_data_union.xentry.name+ecx+2]
        add     ebx, [ebp+XFS.ftype_size]
        jmp     .common
.read:
        dec     [ebp+XFS.requested_cnt]
        inc     [ebp+XFS.entries_read]
        mov     edi, [_arg]
        mov     edi, [edi]
        movbe   edx, [ebx+xfs_dir2_data_union.xentry.inumber.lo]
        movbe   eax, [ebx+xfs_dir2_data_union.xentry.inumber.hi]
        stdcall xfs_read_inode, eax, edx, [ebp+XFS.tmp_inode]
        stdcall xfs_get_inode_info, edx, edi
        jnz     .error
        mov     edx, [_arg]
        mov     edx, [edx]
        mov     ecx, [ebp+XFS.bdfe_nameenc]
        mov     [edx+bdfe.nameenc], ecx
        lea     edi, [edx+bdfe.name]
        movzx   ecx, [ebx+xfs_dir2_data_union.xentry.namelen]
        lea     esi, [ebx+xfs_dir2_data_union.xentry.name]
        stdcall xfs._.copy_filename
        lea     ebx, [esi+2]  ; skip 'tag'
        add     ebx, [ebp+XFS.ftype_size]
        mov     eax, [_arg]
        mov     edx, [eax]
        add     edx, [ebp+XFS.bdfe_len]
        mov     [eax], edx
.common:
        sub     ebx, [ebp+XFS.cur_dirblock]
        add     ebx, 7          ; xfs_dir2_data_entries are aligned to 8 bytes
        and     ebx, not 7
        add     ebx, [ebp+XFS.cur_dirblock]
        dec     [ebp+XFS.entries_left_in_dir]
.quit:
        movi    eax, ERROR_SUCCESS
        cmp     esp, esp
.error:
        ret
endp


proc xfs._.dir_btree_skip_read uses ebx ecx edx esi edi, _cur_dirblock, _offset_lo, _offset_hi, _arg
        mov     ebx, [_cur_dirblock]
        mov     eax, [ebp+XFS.dir_data_magic]
        cmp     [ebx+xfs_dir2_block.hdr.magic], eax
        movi    eax, ERROR_FS_FAIL
        jnz     .error
        mov     eax, ebx
        add     eax, [ebp+XFS.dirblocksize]
        mov     [ebp+XFS.max_dirblockaddr], eax
        add     ebx, [ebp+XFS.dir_block_size]
.next:
        movi    eax, ERROR_SUCCESS
        cmp     [ebp+XFS.requested_cnt], 0
        jz      .quit
        cmp     [ebp+XFS.entries_left_in_dir], 0
        jz      .quit
        cmp     ebx, [ebp+XFS.max_dirblockaddr]
        jz      .quit
        stdcall xfs._.dir_entry_skip_read, [_arg]
        jz      .next
.error:
.quit:
        ret
endp


proc xfs._.readdir_btree uses esi, _inode_data, _out_buf
        mov     [ebp+XFS.cur_inode_save], ebx
        mov     [ebp+XFS.entries_read], 0
        mov     eax, [ebp+XFS.inodesize]
        sub     eax, [ebp+XFS.inode_core_size]
        movzx   ecx, [ebx+xfs_inode.di_core.di_forkoff]
        jecxz   @f
        shl     ecx, 3
        mov     eax, ecx
@@:
        mov     edx, ebx
        add     edx, [ebp+XFS.inode_core_size]
        mov     ecx, [ebp+XFS.dir2_leaf_offset_blocks.lo]
        mov     [ebp+XFS.offset_begin.lo], ecx
        mov     ecx, [ebp+XFS.dir2_leaf_offset_blocks.hi]
        mov     [ebp+XFS.offset_begin.hi], ecx
        mov     ecx, [ebp+XFS.dir2_free_offset_blocks.lo]
        mov     [ebp+XFS.offset_end.lo], ecx
        mov     ecx, [ebp+XFS.dir2_free_offset_blocks.hi]
        mov     [ebp+XFS.offset_end.hi], ecx
        stdcall xfs._.walk_btree, edx, eax, xfs._.extent_iterate_dirblocks, xfs._.leafn_calc_entries, 0, 1
        mov     eax, [ebp+XFS.entries_read]
        mov     edx, [_out_buf]
        mov     [edx+bdfe_hdr.total_cnt], eax
        mov     [ebp+XFS.entries_left_in_dir], eax
        mov     [ebp+XFS.entries_read], 0
        add     [_out_buf], sizeof.bdfe_hdr
        mov     eax, [ebp+XFS.inodesize]
        sub     eax, [ebp+XFS.inode_core_size]
        movzx   ecx, [ebx+xfs_inode.di_core.di_forkoff]
        jecxz   @f
        shl     ecx, 3
        mov     eax, ecx
@@:
        mov     edx, ebx
        add     edx, [ebp+XFS.inode_core_size]
        mov     [ebp+XFS.offset_begin.lo], 0
        mov     [ebp+XFS.offset_begin.hi], 0
        mov     ecx, [ebp+XFS.dir2_leaf_offset_blocks.lo]
        mov     [ebp+XFS.offset_end.lo], ecx
        mov     ecx, [ebp+XFS.dir2_leaf_offset_blocks.hi]
        mov     [ebp+XFS.offset_end.hi], ecx
        mov     ecx, [_out_buf]
        push    ecx
        mov     ecx, esp
        stdcall xfs._.walk_btree, edx, eax, xfs._.extent_iterate_dirblocks, xfs._.dir_btree_skip_read, ecx, 1
        pop     ecx
.error:
.quit:
        ret
endp


proc xfs._.copy_filename uses eax
        mov     eax, [ebp+XFS.bdfe_nameenc]
        cmp     eax, 3
        jz      .utf8
        cmp     eax, 2
        jz      .utf16
.cp866:
        call    unicode.utf8.decode
        call    unicode.cp866.encode
        stosb
        test    ecx, ecx
        jnz     .cp866
        mov     byte[edi], 0
        jmp     .done
.utf16:
        call    unicode.utf8.decode
        call    unicode.utf16.encode
        stosw
        shr     eax, 16
        jz      @f
        stosw
@@:
        test    ecx, ecx
        jnz     .utf16
        mov     word[edi], 0
        jmp     .done
.utf8:
        rep movsb
        mov     byte[edi], 0
.done:
        ret
endp

;----------------------------------------------------------------
; src              ; inode
; dst              ; bdfe
; start_number     ; from 0
;----------------------------------------------------------------
proc xfs._.readdir uses ebx esi edi, _start_number, _entries_to_read, _dst, _src, _encoding
        mov     ecx, [_start_number]
        mov     [ebp+XFS.entries_to_skip], ecx
        mov     eax, [_entries_to_read]
        mov     [ebp+XFS.requested_cnt], eax
        mov     eax, [_encoding]
        mov     [ebp+XFS.bdfe_nameenc], eax
        mov     ecx, 304
        cmp     eax, 1  ; CP866
        jbe     @f
        mov     ecx, 560
@@:
        mov     [ebp+XFS.bdfe_len], ecx
        mov     edx, [_dst]
        mov     [ebp+XFS.bdfe_buf], edx
        mov     ebx, [_src]
        mov     [ebp+XFS.cur_inode_save], ebx

        mov     [edx+bdfe_hdr.version], 1
        mov     [edx+bdfe_hdr.zeroed+0x00], 0
        mov     [edx+bdfe_hdr.zeroed+0x04], 0
        mov     [edx+bdfe_hdr.zeroed+0x08], 0
        mov     [edx+bdfe_hdr.zeroed+0x0c], 0
        mov     [edx+bdfe_hdr.zeroed+0x10], 0

        movzx   eax, [ebx+xfs_inode.di_core.di_format]
        ; switch directory ondisk format and jump to corresponding label
        cmp     eax, XFS_DINODE_FMT_LOCAL
        jnz     @f
        add     ebx, [ebp+XFS.inode_core_size]
        stdcall xfs._.readdir_sf, ebx, [_dst]
        test    eax, eax
        jnz     .error
        jmp     .quit
@@:
        cmp     eax, XFS_DINODE_FMT_BTREE
        jnz     @f
        stdcall xfs._.readdir_btree, ebx, [_dst]
        jmp     .quit
@@:
        cmp     eax, XFS_DINODE_FMT_EXTENTS
        movi    eax, ERROR_FS_FAIL
        jnz     .error
        call    xfs._.get_last_dirblock
        test    eax, eax
        jnz     @f
        add     ebx, [ebp+XFS.inode_core_size]
        stdcall xfs._.readdir_block, ebx, [_dst]
        jmp     .quit
@@:
        stdcall xfs._.readdir_leaf_node, ebx, [_dst]
        jmp     .quit
.quit:
        mov     edx, [_dst]
        mov     ebx, [ebp+XFS.entries_read]
        mov     [edx+bdfe_hdr.read_cnt], ebx
        xor     eax, eax
.error:
        ret
endp


; returns edx:eax inode or 0
proc xfs._.lookup_sf _name, _len
        add     ebx, [ebp+XFS.inode_core_size]
        mov     esi, [_name]
        mov     ecx, [_len]
        cmp     ecx, 2
        ja      .common
        jz      .check_parent
.check_self:
        cmp     byte[esi], '.'
        jnz     .common
        mov     eax, [ebp+XFS.inode_self.lo]
        mov     edx, [ebp+XFS.inode_self.hi]
        jmp     .quit
.check_parent:
        cmp     word[esi], '..'
        jnz     .common
        lea     edx, [ebx+xfs_dir2_sf.hdr.parent]
        call    xfs._.get_inode_number_sf
        jmp     .quit
.common:
        movzx   edx, [ebx+xfs_dir2_sf.hdr.count]
        movi    eax, 0
        cmp     [ebx+xfs_dir2_sf.hdr.i8count], 0
        setnz   al
        lea     eax, [eax*4+4]
        lea     edi, [ebx+xfs_dir2_sf.hdr.parent+eax]
.next_name:
        dec     edx
        jns     @f
        movi    eax, ERROR_FILE_NOT_FOUND
        jmp     .error
@@:
        movzx   ecx, [edi+xfs_dir2_sf_entry.namelen]
        add     edi, xfs_dir2_sf_entry.name
        mov     esi, [_name]
        cmp     ecx, [_len]
        jnz     @f
        repz cmpsb
        jz      .found
@@:
        add     edi, [ebp+XFS.ftype_size]
        add     edi, ecx
        add     edi, eax
        jmp     .next_name
.found:
        add     edi, [ebp+XFS.ftype_size]
        mov     edx, edi
        call    xfs._.get_inode_number_sf
.quit:
        cmp     esp, esp
.error:
        ret
endp


proc xfs._.lookup_block uses esi, _name, _len
        add     ebx, [ebp+XFS.inode_core_size]
        mov     eax, ebx
        mov     ebx, [ebp+XFS.cur_dirblock]
        stdcall xfs._.extent_unpack, eax
        stdcall xfs._.read_dirblock, [ebp+XFS.extent.br_startblock.lo], \
                [ebp+XFS.extent.br_startblock.hi], ebx
        jnz     .error
        mov     eax, [ebp+XFS.dir_block_magic]
        cmp     [ebx+xfs_dir2_block.hdr.magic], eax
        movi    eax, ERROR_FS_FAIL
        jnz     .error
        stdcall xfs_hashname, [_name+4], [_len]
        add     ebx, [ebp+XFS.dirblocksize]
        movbe   ecx, [ebx-sizeof.xfs_dir2_block_tail+xfs_dir2_block_tail.count]
        lea     edx, [ecx*sizeof.xfs_dir2_leaf_entry+sizeof.xfs_dir2_block_tail]
        sub     ebx, edx
        stdcall xfs._.get_addr_by_hash, ebx, ecx
        jnz     .error
        mov     ebx, [ebp+XFS.cur_dirblock]
        movbe   edx, [ebx+eax*XFS_DIR2_DATA_ALIGN+xfs_dir2_data_entry.inumber.lo]
        movbe   eax, [ebx+eax*XFS_DIR2_DATA_ALIGN+xfs_dir2_data_entry.inumber.hi]
.quit:
.error:
        ret
endp


proc xfs._.get_inode_by_addr uses ebx esi edi, _inode_buf
        xor     edx, edx
        shld    edx, eax, XFS_DIR2_DATA_ALIGN_LOG
        shl     eax, XFS_DIR2_DATA_ALIGN_LOG
        mov     esi, [ebp+XFS.dirblocksize]
        dec     esi
        and     esi, eax
        mov     ecx, [ebp+XFS.blocklog]
        add     ecx, [ebp+XFS.dirblklog]
        shrd    eax, edx, cl
        shr     edx, cl
        mov     ecx, [ebp+XFS.dirblklog]
        shld    edx, eax, cl
        shl     eax, cl
        mov     ebx, [_inode_buf]
        cmp     [ebx+xfs_inode.di_core.di_format], XFS_DINODE_FMT_EXTENTS
        jz      .extents
        cmp     [ebx+xfs_inode.di_core.di_format], XFS_DINODE_FMT_BTREE
        jz      .btree
        jmp     .error
.extents:
        mov     ecx, [ebp+XFS.nextents_offset]
        movbe   ecx, [ebx+ecx]
        add     ebx, [ebp+XFS.inode_core_size]
        mov     [ebp+XFS.offset_begin.lo], eax
        mov     [ebp+XFS.offset_begin.hi], edx
        stdcall xfs._.extent_list.seek, ecx
        stdcall xfs._.read_dirblock, [ebp+XFS.extent.br_startblock.lo], [ebp+XFS.extent.br_startblock.hi], [ebp+XFS.cur_dirblock]
        jnz     .error
        jmp     .common
.btree:
        movzx   ecx, [ebx+xfs_inode.di_core.di_forkoff]
        shl     ecx, 3
        test    ecx, ecx
        jnz     @f
        mov     ecx, [ebp+XFS.inodesize]
        sub     ecx, [ebp+XFS.inode_core_size]
@@:
        add     ebx, [ebp+XFS.inode_core_size]
        stdcall xfs._.btree_read_block, ebx, ecx, eax, edx, [ebp+XFS.cur_dirblock]
.common:
        mov     ebx, [ebp+XFS.cur_dirblock]
        mov     eax, [ebp+XFS.dir_data_magic]
        cmp     [ebx+xfs_dir2_block.hdr.magic], eax
        movi    eax, ERROR_FS_FAIL
        jnz     .error
        movbe   edx, [ebx+esi+xfs_dir2_data_entry.inumber.lo]
        movbe   eax, [ebx+esi+xfs_dir2_data_entry.inumber.hi]
.error:
.quit:
        ret
endp


proc xfs._.lookup_leaf uses ebx esi edi, _name, _len
        mov     ecx, [ebp+XFS.nextents_offset]
        movbe   ecx, [ebx+ecx]
        add     ebx, [ebp+XFS.inode_core_size]
        mov     eax, [ebp+XFS.dir2_leaf_offset_blocks.lo]
        mov     [ebp+XFS.offset_begin.lo], ecx
        mov     eax, [ebp+XFS.dir2_leaf_offset_blocks.hi]
        mov     [ebp+XFS.offset_begin.hi], ecx
        stdcall xfs._.extent_list.seek, ecx
        stdcall xfs._.read_dirblock, [ebp+XFS.extent.br_startblock.lo], \
                [ebp+XFS.extent.br_startblock.hi], [ebp+XFS.cur_dirblock]
        jnz     .error
        mov     ebx, [ebp+XFS.cur_dirblock]
        movzx   eax, [ebp+XFS.dir_leaf1_magic]
        cmp     [ebx+xfs_dir2_leaf.hdr.info.magic], ax
        movi    eax, ERROR_FS_FAIL
        jnz     .error
        stdcall xfs_hashname, [_name+4], [_len]
        cmp     [ebp+XFS.version], 5
        jz      .v5
.v4:
        movzx   ecx, [ebx+xfs_dir2_leaf.hdr.count]
        xchg    cl, ch
        add     ebx, xfs_dir2_leaf.ents
        jmp     .vcommon
.v5:
        movzx   ecx, [ebx+xfs_dir3_leaf.hdr.count]
        xchg    cl, ch
        add     ebx, xfs_dir3_leaf.ents
.vcommon:
        stdcall xfs._.get_addr_by_hash, ebx, ecx
        jnz     .error
        stdcall xfs._.get_inode_by_addr, [ebp+XFS.cur_inode_save]
.quit:
.error:
        ret
endp


proc xfs._.lookup_node uses ebx esi edi, _name, _len
locals
        .hash dd ?
endl
        mov     [ebp+XFS.cur_inode_save], ebx
        stdcall xfs_hashname, [_name+4], [_len]
        mov     [.hash], eax
        mov     edx, [ebp+XFS.nextents_offset]
        movbe   edx, [ebx+edx]
        mov     esi, [ebp+XFS.dir2_leaf_offset_blocks.lo]
.begin:
        mov     ebx, [ebp+XFS.cur_inode_save]
        mov     eax, ebx
        add     eax, [ebp+XFS.inode_core_size]
        mov     edx, [ebp+XFS.nextents_offset]
        movbe   edx, [ebx+edx]
        mov     ebx, eax
        mov     [ebp+XFS.offset_begin.lo], esi
        mov     [ebp+XFS.offset_begin.hi], 0
        stdcall xfs._.extent_list.seek, edx
        stdcall xfs._.read_dirblock, [ebp+XFS.extent.br_startblock.lo], [ebp+XFS.extent.br_startblock.hi], [ebp+XFS.cur_dirblock]
        jnz     .error
        mov     ebx, [ebp+XFS.cur_dirblock]
        movzx   eax, [ebp+XFS.da_node_magic]
        cmp     [ebx+xfs_da_intnode.hdr.info.magic], ax
        jz      .node
        movzx   eax, [ebp+XFS.dir_leafn_magic]
        cmp     [ebx+xfs_dir2_leaf.hdr.info.magic], ax
        jz      .leaf
        movi    eax, ERROR_FS_FAIL
        jmp     .error
.node:
        cmp     [ebp+XFS.version], 5
        jz      .node.v5
.node.v4:
        lea     eax, [ebx+sizeof.xfs_da_intnode]
        movzx   edx, [ebx+xfs_da_intnode.hdr.count]
        jmp     .node.vcommon
.node.v5:
        lea     eax, [ebx+sizeof.xfs_da3_intnode]
        movzx   edx, [ebx+xfs_da3_intnode.hdr.count]
.node.vcommon:
        xchg    dl, dh
        stdcall xfs._.get_before_by_hashval, eax, edx, [.hash]
        jnz     .error
        mov     esi, eax
        jmp     .begin
.leaf:
        cmp     [ebp+XFS.version], 5
        jz      .leaf.v5
.leaf.v4:
        movzx   ecx, [ebx+xfs_dir2_leaf.hdr.count]
        xchg    cl, ch
        add     ebx, xfs_dir2_leaf.ents
        jmp     .leaf.vcommon
.leaf.v5:
        movzx   ecx, [ebx+xfs_dir3_leaf.hdr.count]
        xchg    cl, ch
        add     ebx, xfs_dir3_leaf.ents
.leaf.vcommon:
        mov     eax, [.hash]
        stdcall xfs._.get_addr_by_hash, ebx, ecx
        jnz     .error
        stdcall xfs._.get_inode_by_addr, [ebp+XFS.cur_inode_save]
.quit:
        cmp     esp, esp
        ret
.error:
        test    esp, esp
        ret
endp


proc xfs._.lookup_btree uses ebx esi edi, _name, _len
locals
        .hash dd ?
endl
        mov     [ebp+XFS.cur_inode_save], ebx
        stdcall xfs_hashname, [_name+4], [_len]
        mov     [.hash], eax
        mov     edx, [ebp+XFS.dir2_leaf_offset_blocks.hi]
        mov     eax, [ebp+XFS.dir2_leaf_offset_blocks.lo]
        jmp     .next_level.first
.next_level:
        lea     eax, [ebx+sizeof.xfs_da_intnode]
        movzx   edx, [ebx+xfs_da_intnode.hdr.count]
        xchg    dl, dh
        stdcall xfs._.get_before_by_hashval, eax, edx, [.hash]
        jnz     .error
        xor     edx, edx
.next_level.first:
        mov     ebx, [ebp+XFS.cur_inode_save]
        movzx   ecx, [ebx+xfs_inode.di_core.di_forkoff]
        shl     ecx, 3
        test    ecx, ecx
        jnz     @f
        mov     ecx, [ebp+XFS.inodesize]
        sub     ecx, [ebp+XFS.inode_core_size]
@@:
        add     ebx, [ebp+XFS.inode_core_size]
        stdcall xfs._.btree_read_block, ebx, ecx, eax, edx, [ebp+XFS.cur_dirblock]
        mov     ebx, [ebp+XFS.cur_dirblock]
        movzx   eax, [ebp+XFS.da_node_magic]
        cmp     [ebx+xfs_da_blkinfo.magic], ax
        jz      .next_level
        movzx   eax, [ebp+XFS.dir_leafn_magic]
        cmp     [ebx+xfs_da_blkinfo.magic], ax
        jz      .leafn
        movzx   eax, [ebp+XFS.dir_leaf1_magic]
        cmp     [ebx+xfs_da_blkinfo.magic], ax
        jnz     .error
        mov     eax, [.hash]
        mov     ecx, [ebp+XFS.dirx_leaf_hdr_count_offset]
        movzx   ecx, word[ebx+ecx]
        xchg    cl, ch
        add     ebx, [ebp+XFS.dirx_leaf_ents_offset]
        stdcall xfs._.get_addr_by_hash, ebx, ecx
        jnz     .error
        mov     ebx, [ebp+XFS.cur_dirblock]
        jmp     .got_addr
.leafn:
        mov     ecx, [ebp+XFS.dirx_leaf_hdr_count_offset]
        movzx   ecx, word[ebx+ecx]
        xchg    cl, ch
        add     ebx, [ebp+XFS.dirx_leaf_ents_offset]
        mov     eax, [.hash]
        stdcall xfs._.get_addr_by_hash, ebx, ecx
        jnz     .error
        mov     ebx, [ebp+XFS.cur_block]
.got_addr:
        stdcall xfs._.get_inode_by_addr, [ebp+XFS.cur_inode_save]
.quit:
        cmp     esp, esp
        ret
.error:
        test    esp, esp
        ret
endp


; search for the _name in _inode dir
; called for each /path/component/to/my/file
; out:
; ZF/zf   = ok/fail
; edx:eax = inode/garbage:error
proc xfs._.get_inode_short uses esi, _inode:qword, _len, _name
        mov     esi, [_name]
        mov     eax, dword[_inode+DQ.lo]
        mov     edx, dword[_inode+DQ.hi]
        stdcall xfs_read_inode, eax, edx, [ebp+XFS.cur_inode]
        test    eax, eax
        movi    eax, ERROR_FS_FAIL
        jnz     .error
        ; switch directory ondisk format
        mov     ebx, edx
        mov     [ebp+XFS.cur_inode_save], ebx
        movzx   eax, [ebx+xfs_inode.di_core.di_format]
        cmp     eax, XFS_DINODE_FMT_LOCAL
        mov     edi, xfs._.lookup_sf
        jz      .lookup
        cmp     eax, XFS_DINODE_FMT_BTREE
        mov     edi, xfs._.lookup_btree
        jz      .lookup
        cmp     eax, XFS_DINODE_FMT_EXTENTS
        jnz     .error
        call    xfs._.get_last_dirblock
        test    eax, eax
        mov     edi, xfs._.lookup_block
        jz      .lookup
        cmp     edx, [ebp+XFS.dir2_free_offset_blocks.hi]
        mov     edi, xfs._.lookup_node
        ja      .lookup
        cmp     eax, [ebp+XFS.dir2_free_offset_blocks.lo]
        jae     .lookup
        mov     edi, xfs._.lookup_leaf
.lookup:
        stdcall edi, [_name+4], [_len]
.error:
        ret
endp


; ZF/zf   = ok/fail
; edx:eax = inode/garbage:error
proc xfs_get_inode uses ebx esi edi, _name
        ; call *._.get_inode_short until file is found / error returned
        ; start from the root inode
        mov     eax, [ebp+XFS.rootino.lo]
        mov     edx, [ebp+XFS.rootino.hi]
        mov     esi, [_name]
.next_dir:
@@:
        cmp     byte[esi], '/'
        jnz     @f
        inc     esi
        jmp     @b
@@:
        cmp     byte[esi], 0
        jz      .found
        push    esi
        inc     esi
@@:
        cmp     byte[esi], 0
        jz      @f
        cmp     byte[esi], '/'
        jz      @f
        inc     esi
        jmp     @b
@@:
        mov     ecx, esi
        sub     ecx, [esp]
        mov     [ebp+XFS.inode_self.lo], eax
        mov     [ebp+XFS.inode_self.hi], edx
        stdcall xfs._.get_inode_short, eax, edx, ecx      ; esi pushed above
        jz      .next_dir
.error:
.found:
        ret
endp


; in:  ebp = pointer to XFS structure
; in:  esi
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
; out: [edx] -- f70.1 out structure
proc xfs_ReadFolder uses esi edi
        call    xfs._.lock
        stdcall xfs_get_inode, esi
        jnz     .error
        stdcall xfs_read_inode, eax, edx, [ebp+XFS.cur_inode]
        test    eax, eax
        jnz     .error
        stdcall xfs._.readdir, [ebx+f70s1arg.start_idx], [ebx+f70s1arg.count], [ebx+f70s1arg.buf], edx, [ebx+f70s1arg.encoding]
        test    eax, eax
        jnz     .error
        mov     edx, [ebx+f70s1arg.buf]
        mov     ecx, [ebx+f70s1arg.count]
        cmp     [edx+bdfe_hdr.read_cnt], ecx
        jz      .quit
        movi    eax, ERROR_END_OF_FILE
.quit:
        mov     ebx, [edx+bdfe_hdr.read_cnt]

.error:
        push    eax
        call    xfs._.unlock
        pop     eax
        ret
endp


; edx -- pointer to inode number in big endian
; ZF -- must be set at exit
proc xfs._.get_inode_number_sf
        cmp     [ebx+xfs_dir2_sf.hdr.i8count], 0
        jz      .i4bytes
.i8bytes:
        movbe   eax, [edx+DQ.hi]
        movbe   edx, [edx+DQ.lo]
        ret
.i4bytes:
        movbe   eax, [edx+DQ.lo]
        xor     edx, edx
        ret
endp

proc xfs._.conv_time_to_kos_epoch
        movbe   eax, [ecx+DQ.hi_be]
        call    fsTime2bdfe
        ret
endp

proc xfs._.conv_bigtime_to_kos_epoch
NANOSEC_PER_SEC = 1_000_000_000
BIGTIME_TO_UNIX_OFFSET = 0x80000000     ; int32 min
UNIXTIME_TO_KOS_OFFSET = (365*31+8)*24*60*60  ; 01.01.1970--01.01.2001
BIGTIME_TO_KOS_OFFSET = BIGTIME_TO_UNIX_OFFSET + UNIXTIME_TO_KOS_OFFSET
BIGTIME_TO_KOS_OFFSET_NS = BIGTIME_TO_KOS_OFFSET * NANOSEC_PER_SEC
        movbe   edx, [ecx+DQ.hi_be]
        movbe   eax, [ecx+DQ.lo_be]
        sub     eax, BIGTIME_TO_KOS_OFFSET_NS AND 0xffffffff
        sbb     edx, BIGTIME_TO_KOS_OFFSET_NS SHR 32
        jnc     .after_kos_epoch_begin
        xor     eax, eax        ; set to very begin of kolibrios epoch
        xor     edx, edx
        jmp     .time_to_bdfe
.after_kos_epoch_begin:
        cmp     edx, NANOSEC_PER_SEC
        jb      .time_to_bdfe
        mov     edx, NANOSEC_PER_SEC - 1
        mov     eax, -1         ; very end of kolibrios epoch
.time_to_bdfe:
        mov     ecx, NANOSEC_PER_SEC
        div     ecx
        call    fsTime2bdfe
        ret
endp

proc xfs_get_inode_info uses ebx esi edi, _src, _dst
        ; get access time and other file properties
        ; useful for browsing directories
        ; called for each dir entry
        xor     eax, eax
        mov     esi, [_src]
        movzx   ecx, [esi+xfs_inode.di_core.di_mode]
        xchg    cl, ch
        test    ecx, S_IFDIR
        jz      @f
        movi    eax, 0x10       ; set directory flag
@@:
        mov     edi, [_dst]
        mov     [edi+bdfe.attr], eax
        movbe   edx, [esi+xfs_inode.di_core.di_size.hi_be]
        movbe   eax, [esi+xfs_inode.di_core.di_size.lo_be]
        mov     [edi+bdfe.size.hi], edx
        mov     [edi+bdfe.size.lo], eax

        add     edi, bdfe.ctime
        lea     ecx, [esi+xfs_inode.di_core.di_ctime]
        call    [ebp+XFS.conv_time_to_kos_epoch]
        lea     ecx, [esi+xfs_inode.di_core.di_atime]
        call    [ebp+XFS.conv_time_to_kos_epoch]
        lea     ecx, [esi+xfs_inode.di_core.di_mtime]
        call    [ebp+XFS.conv_time_to_kos_epoch]

        movi    eax, ERROR_SUCCESS
        cmp     esp, esp
        ret
endp


proc xfs._.extent_unpack uses eax ebx ecx edx, _extent_data
        ; extents come as packed 128bit bitfields
        ; unpack them to access internal fields
        ; write result to the XFS.extent structure
        mov     ebx, [_extent_data]

        xor     eax, eax
        movbe   edx, [ebx+0]
        test    edx, 0x80000000         ; mask, see documentation
        setnz   al
        mov     [ebp+XFS.extent.br_state], eax

        and     edx, 0x7fffffff         ; mask
        movbe   eax, [ebx+4]
        shrd    eax, edx, 9
        shr     edx, 9
        mov     [ebp+XFS.extent.br_startoff.lo], eax
        mov     [ebp+XFS.extent.br_startoff.hi], edx

        movbe   edx, [ebx+4]
        movbe   eax, [ebx+8]
        movbe   ecx, [ebx+12]
        and     edx, 0x000001ff         ; mask
        shrd    ecx, eax, 21
        shrd    eax, edx, 21
        mov     [ebp+XFS.extent.br_startblock.lo], ecx
        mov     [ebp+XFS.extent.br_startblock.hi], eax

        movbe   eax, [ebx+12]
        and     eax, 0x001fffff         ; mask
        mov     [ebp+XFS.extent.br_blockcount], eax
        ret
endp


proc xfs_hashname uses ecx esi, _name, _len
        xor     eax, eax
        mov     esi, [_name]
        mov     ecx, [_len]
@@:
        rol     eax, 7
        xor     al, [esi]
        add     esi, 1
        dec     ecx
        jnz     @b
        ret
endp


; eax -- hash value
proc xfs._.get_addr_by_hash uses ebx esi, _base, _len
        ; look for the directory entry offset by its file name hash
        ; allows fast file search for block, leaf and node directories
        ; binary (ternary) search
        mov     ebx, [_base]
        mov     edx, [_len]
.next:
        mov     ecx, edx
;        jecxz   .error
        test    ecx, ecx
        jz      .not_found
        shr     ecx, 1
        movbe   esi, [ebx+ecx*sizeof.xfs_dir2_leaf_entry+xfs_dir2_leaf_entry.hashval]
        cmp     eax, esi
        jb      .below
        ja      .above
        movbe   eax, [ebx+ecx*sizeof.xfs_dir2_leaf_entry+xfs_dir2_leaf_entry.address]
        ret
.below:
        mov     edx, ecx
        jmp     .next
.above:
        lea     ebx, [ebx+(ecx+1)*sizeof.xfs_dir2_leaf_entry]
        sub     edx, ecx
        dec     edx
        jmp     .next
.not_found:
        movi    eax, ERROR_FILE_NOT_FOUND
        test    esp, esp
        ret
endp


;----------------------------------------------------------------
; xfs_GetFileInfo: XFS implementation of getting file info
; in:  ebp = pointer to XFS structure
; in:  esi = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
proc xfs_GetFileInfo uses ecx edx esi edi
        call    xfs._.lock
        stdcall xfs_get_inode, esi
        jnz     .error
        stdcall xfs_read_inode, eax, edx, [ebp+XFS.cur_inode]
        test    eax, eax
        movi    eax, ERROR_FS_FAIL
        jnz     .error
        stdcall xfs_get_inode_info, edx, [ebx+f70s5arg.buf]
.quit:
        call    xfs._.unlock
        xor     eax, eax
        ret
.error:
        push    eax
        call    xfs._.unlock
        pop     eax
        ret
endp


proc xfs._.file.read_extent uses ebx ecx edx, _callback, _callback_data
        mov     eax, [ebp+XFS.file_offset.lo]
        mov     edx, [ebp+XFS.file_offset.hi]
        mov     esi, [ebp+XFS.extent.br_startoff.lo]
        mov     edi, [ebp+XFS.extent.br_startoff.hi]
        mov     ecx, [ebp+XFS.blocklog]
        shld    edi, esi, cl
        shl     esi, cl
        cmp     edx, edi
        jb      .hole
        ja      .try_head
        cmp     eax, esi
        ja      .try_head
        jz      .try_match
.hole:
        sub     esi, eax
        sbb     edi, edx
        movi    ecx, -1
        test    edi, edi
        jnz     @f
        mov     ecx, esi
@@:
        cmp     ecx, [ebp+XFS.bytes_to_read]
        jbe     @f
        mov     ecx, [ebp+XFS.bytes_to_read]
@@:
        mov     edi, [ebp+XFS.file_buffer]
        xor     eax, eax
        sub     [ebp+XFS.bytes_to_read], ecx
        sub     [ebp+XFS.bytes_left_in_file.lo], ecx
        sbb     [ebp+XFS.bytes_left_in_file.hi], 0
        add     [ebp+XFS.bytes_read], ecx
        add     [ebp+XFS.file_buffer], ecx
        add     [ebp+XFS.file_offset.lo], ecx
        adc     [ebp+XFS.file_offset.hi], 0
        rep stosb
        cmp     [ebp+XFS.bytes_to_read], 0
        jz      .quit
        jmp     .try_match
.try_head:
        mov     eax, [ebp+XFS.file_offset.lo]
        mov     ecx, [ebp+XFS.blocksize]
        dec     ecx
        test    eax, ecx
        jz      .try_match
.head:
        mov     eax, [ebp+XFS.extent.br_startblock.lo]
        mov     edx, [ebp+XFS.extent.br_startblock.hi]
        mov     ebx, [ebp+XFS.cur_block_data]
        stdcall xfs._.read_block
        mov     esi, [ebp+XFS.cur_block_data]
        mov     edi, [ebp+XFS.file_buffer]
        mov     eax, [ebp+XFS.file_offset.lo]
        mov     ecx, [ebp+XFS.blocksize]
        dec     ecx
        and     eax, ecx
        add     esi, eax
        inc     ecx
        sub     ecx, eax
        cmp     ecx, [ebp+XFS.bytes_to_read]
        jbe     @f
        mov     ecx, [ebp+XFS.bytes_to_read]
@@:
        sub     [ebp+XFS.bytes_to_read], ecx
        sub     [ebp+XFS.bytes_left_in_file.lo], ecx
        sbb     [ebp+XFS.bytes_left_in_file.hi], 0
        add     [ebp+XFS.bytes_read], ecx
        add     [ebp+XFS.file_buffer], ecx
        add     [ebp+XFS.file_offset.lo], ecx
        adc     [ebp+XFS.file_offset.hi], 0
        rep movsb
        add     [ebp+XFS.extent.br_startoff.lo], 1
        adc     [ebp+XFS.extent.br_startoff.hi], 0
        add     [ebp+XFS.extent.br_startblock.lo], 1
        adc     [ebp+XFS.extent.br_startblock.hi], 0
        dec     [ebp+XFS.extent.br_blockcount]
;        cmp     [ebp+XFS.bytes_to_read], 0
        jz      .quit
.try_match:
        mov     eax, [ebp+XFS.bytes_to_read]
        test    eax, eax
        jz      .quit
        cmp     eax, [ebp+XFS.blocksize]
        jb      .tail
        mov     ecx, [ebp+XFS.blocklog]
        shr     eax, cl
        cmp     eax, [ebp+XFS.extent.br_blockcount]
        jbe     @f
        mov     eax, [ebp+XFS.extent.br_blockcount]
@@:
        mov     ecx, eax
        mov     eax, [ebp+XFS.extent.br_startblock.lo]
        mov     edx, [ebp+XFS.extent.br_startblock.hi]
        mov     ebx, [ebp+XFS.file_buffer]
        push    ecx
        stdcall xfs._.read_blocks
        pop     eax
        add     [ebp+XFS.extent.br_startoff.lo], eax
        adc     [ebp+XFS.extent.br_startoff.hi], 0
        add     [ebp+XFS.extent.br_startblock.lo], eax
        adc     [ebp+XFS.extent.br_startblock.hi], 0
        sub     [ebp+XFS.extent.br_blockcount], eax
        imul    eax, [ebp+XFS.blocksize]
        sub     [ebp+XFS.bytes_to_read], eax
        sub     [ebp+XFS.bytes_left_in_file.lo], eax
        sbb     [ebp+XFS.bytes_left_in_file.hi], 0
        add     [ebp+XFS.bytes_read], eax
        add     [ebp+XFS.file_buffer], eax
        add     [ebp+XFS.file_offset.lo], eax
        adc     [ebp+XFS.file_offset.hi], 0
;        cmp     [ebp+XFS.bytes_to_read], 0
        cmp     [ebp+XFS.extent.br_blockcount], 0
        jz      .quit
.tail:
        mov     eax, [ebp+XFS.extent.br_startblock.lo]
        mov     edx, [ebp+XFS.extent.br_startblock.hi]
        mov     ebx, [ebp+XFS.cur_block_data]
        stdcall xfs._.read_block
        mov     ecx, [ebp+XFS.bytes_to_read]
        cmp     [ebp+XFS.bytes_left_in_file.hi], 0
        jnz     @f
        cmp     ecx, [ebp+XFS.bytes_left_in_file.lo]
        jbe     @f
        mov     ecx, [ebp+XFS.bytes_left_in_file.lo]
@@:
        mov     esi, [ebp+XFS.cur_block_data]
        mov     edi, [ebp+XFS.file_buffer]
        mov     eax, ecx
        rep movsb
        add     [ebp+XFS.bytes_read], eax
        sub     [ebp+XFS.bytes_to_read], eax
        sub     [ebp+XFS.bytes_left_in_file.lo], eax
        sbb     [ebp+XFS.bytes_left_in_file.hi], 0
        add     [ebp+XFS.file_buffer], eax
        add     [ebp+XFS.file_offset.lo], eax
        adc     [ebp+XFS.file_offset.hi], 0
        add     [ebp+XFS.extent.br_startoff.lo], 1
        adc     [ebp+XFS.extent.br_startoff.hi], 0
        add     [ebp+XFS.extent.br_startblock.lo], 1
        adc     [ebp+XFS.extent.br_startblock.hi], 0
        dec     [ebp+XFS.extent.br_blockcount]
.quit:
        mov     esi, [ebp+XFS.extent.br_startoff.lo]
        mov     edi, [ebp+XFS.extent.br_startoff.hi]
        movi    eax, ERROR_SUCCESS
        cmp     esp, esp
        ret
endp


;----------------------------------------------------------------
; in:  ebp = pointer to XFS structure
; in:  esi = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
proc xfs_Read uses ecx edx esi edi
locals
        .offset_begin DQ ?
        .offset_end   DQ ?
endl
        call    xfs._.lock
        mov     [ebp+XFS.bytes_read], 0
        mov     eax, [ebx+f70s0arg.count]
        mov     [ebp+XFS.bytes_to_read], eax
        test    eax, eax
        jz      .quit
        mov     eax, [ebx+f70s0arg.buf]
        mov     [ebp+XFS.file_buffer], eax
        mov     eax, [ebx+f70s0arg.offset.hi]
        mov     [ebp+XFS.file_offset.hi], eax
        mov     eax, [ebx+f70s0arg.offset.lo]
        mov     [ebp+XFS.file_offset.lo], eax

        stdcall xfs_get_inode, esi
        jnz     .error
        stdcall xfs_read_inode, eax, edx, [ebp+XFS.cur_inode]
        test    eax, eax
        movi    eax, ERROR_FS_FAIL
        jnz     .error
        mov     [ebp+XFS.cur_inode_save], edx
        mov     ebx, edx
        ; precompute .offset_begin
        mov     esi, [ebp+XFS.file_offset.lo]
        mov     edi, [ebp+XFS.file_offset.hi]
        mov     ecx, [ebp+XFS.blocklog]
        shrd    esi, edi, cl
        shr     edi, cl
        mov     [.offset_begin.lo], esi
        mov     [.offset_begin.hi], edi
        ; precompute .offset_end
        mov     esi, [ebp+XFS.file_offset.lo]
        mov     edi, [ebp+XFS.file_offset.hi]
        add     esi, [ebp+XFS.bytes_to_read]
        adc     edi, 0
        mov     ecx, [ebp+XFS.blocksize]
        dec     ecx
        add     esi, ecx
        adc     edi, 0
        mov     ecx, [ebp+XFS.blocklog]
        shrd    esi, edi, cl
        shr     edi, cl
        mov     [.offset_end.lo], esi
        mov     [.offset_end.hi], edi

        movbe   ecx, [ebx+xfs_inode.di_core.di_size.hi]
        movbe   edx, [ebx+xfs_inode.di_core.di_size.lo]
        mov     [ebp+XFS.bytes_left_in_file.lo], ecx
        mov     [ebp+XFS.bytes_left_in_file.hi], edx

        sub     ecx, [ebp+XFS.file_offset.lo]
        sbb     edx, [ebp+XFS.file_offset.hi]
        movi    eax, ERROR_END_OF_FILE
        jb      .error
        mov     [ebp+XFS.eof], 0
        test    edx, edx
        jnz     @f
        cmp     ecx, [ebp+XFS.bytes_to_read]
        jae     @f
        mov     [ebp+XFS.eof], ERROR_END_OF_FILE
        mov     [ebp+XFS.bytes_to_read], ecx
@@:

        cmp     [ebx+xfs_inode.di_core.di_format], XFS_DINODE_FMT_BTREE
        jz      .btree
.extent_list:
        mov     eax, ebx
        add     eax, [ebp+XFS.inode_core_size]
        mov     edx, [ebp+XFS.nextents_offset]
        movbe   edx, [ebx+edx]
        mov     ecx, [.offset_begin.lo]
        mov     [ebp+XFS.offset_begin.lo], ecx
        mov     ecx, [.offset_begin.hi]
        mov     [ebp+XFS.offset_begin.hi], ecx
        mov     ecx, [.offset_end.lo]
        mov     [ebp+XFS.offset_end.lo], ecx
        mov     ecx, [.offset_end.hi]
        mov     [ebp+XFS.offset_end.hi], ecx
        stdcall xfs._.walk_extent_list, edx, eax, xfs._.file.read_extent, 0, 0
        jnz     .error
        jmp     .hole_check
.btree:
        mov     eax, [ebp+XFS.inodesize]
        sub     eax, [ebp+XFS.inode_core_size]
        movzx   ecx, [ebx+xfs_inode.di_core.di_forkoff]
        jecxz   @f
        shl     ecx, 3
        mov     eax, ecx
@@:
        mov     edx, ebx
        add     edx, [ebp+XFS.inode_core_size]
        mov     ecx, [.offset_begin.lo]
        mov     [ebp+XFS.offset_begin.lo], ecx
        mov     ecx, [.offset_begin.hi]
        mov     [ebp+XFS.offset_begin.hi], ecx
        mov     ecx, [.offset_end.lo]
        mov     [ebp+XFS.offset_end.lo], ecx
        mov     ecx, [.offset_end.hi]
        mov     [ebp+XFS.offset_end.hi], ecx
        stdcall xfs._.walk_btree, edx, eax, xfs._.file.read_extent, 0, 0, 1
.hole_check:
        cmp     [ebp+XFS.bytes_left_in_file.hi], 0
        jnz     @f
        cmp     [ebp+XFS.bytes_left_in_file.lo], 0
        jz      .hole_done
@@:
        cmp     [ebp+XFS.bytes_to_read], 0
        jz      .hole_done
        mov     ebx, [ebp+XFS.cur_inode_save]
        movbe   edx, [ebx+xfs_inode.di_core.di_size.lo]
        movbe   eax, [ebx+xfs_inode.di_core.di_size.hi]
        sub     eax, [ebp+XFS.file_offset.lo]
        sbb     edx, [ebp+XFS.file_offset.hi]
        jc      .hole_done
        mov     ecx, [ebp+XFS.bytes_to_read]
        test    edx, edx
        jnz     .hole_read
        cmp     eax, [ebp+XFS.bytes_to_read]
        jae     .hole_read
        mov     ecx, eax
        jmp     .hole_read
.hole_read:
        sub     [ebp+XFS.bytes_to_read], ecx
        add     [ebp+XFS.bytes_read], ecx
        mov     edi, [ebp+XFS.file_buffer]
        xor     eax, eax
        rep stosb
.hole_done:
.quit:
        mov     eax, [ebp+XFS.eof]
.error:
        push    eax
        call    xfs._.unlock
        pop     eax
        mov     ebx, [ebp+XFS.bytes_read]
        ret
endp


proc xfs._.leafn_calc_entries uses ebx ecx edx esi edi, _cur_dirblock, _offset_lo, _offset_hi, _arg
        mov     edx, [_cur_dirblock]
        movzx   eax, [ebp+XFS.da_node_magic]
        cmp     [edx+xfs_dir2_leaf.hdr.info.magic], ax
        jz      .quit
        cmp     [ebp+XFS.version], 5
        jnz     @f
        add     edx, xfs_dir3_leaf.hdr.count-xfs_dir2_leaf.hdr.count
@@:
        movzx   eax, [edx+xfs_dir2_leaf.hdr.count]
        movzx   ecx, [edx+xfs_dir2_leaf.hdr.stale]
        xchg    al, ah
        xchg    cl, ch
        sub     eax, ecx
        add     [ebp+XFS.entries_read], eax
.quit:
        movi    eax, ERROR_SUCCESS
        cmp     esp, esp
        ret
endp


proc xfs._.get_before_by_hashval uses ebx edx esi edi, _base, _count, _hash
        mov     edi, [_hash]
        mov     edx, [_count]
        xor     ecx, ecx
.node.next:
        movbe   eax, [ebx+xfs_da_intnode.btree+ecx*sizeof.xfs_da_node_entry+xfs_da_node_entry.hashval]
        cmp     [ebp+XFS.version], 5
        jnz     @f
        movbe   eax, [ebx+xfs_da3_intnode.btree+ecx*sizeof.xfs_da_node_entry+xfs_da_node_entry.hashval]
@@:
        cmp     eax, edi
        jae     .node.leaf_found
        inc     ecx
        cmp     ecx, edx
        jnz     .node.next
        movi    eax, ERROR_FILE_NOT_FOUND
        test    esp, esp
        jmp     .error
.node.leaf_found:
        movbe   eax, [ebx+xfs_da_intnode.btree+ecx*sizeof.xfs_da_node_entry+xfs_da_node_entry.before]
        cmp     [ebp+XFS.version], 5
        jnz     @f
        movbe   eax, [ebx+xfs_da3_intnode.btree+ecx*sizeof.xfs_da_node_entry+xfs_da_node_entry.before]
@@:
        jmp     .quit
.error:
        test    esp, esp
        ret
.quit:
        cmp     esp, esp
        ret
endp


proc xfs._.long_btree.seek uses ebx esi edi, _ptr, _size
        mov     ebx, [_ptr]
        mov     esi, [_size]
        sub     esi, sizeof.xfs_bmdr_block
        shr     esi, 4
        shl     esi, 3
        movzx   eax, [ebx+xfs_bmdr_block.bb_level]
        movzx   ecx, [ebx+xfs_bmdr_block.bb_numrecs]
        xchg    cl, ch
        add     ebx, sizeof.xfs_bmdr_block
        jmp     .common
.not_root:
        mov     esi, [ebp+XFS.blocksize]
        sub     esi, [ebp+XFS.bmbt_block_size]
        shr     esi, 4
        shl     esi, 3
        movzx   eax, [ebx+xfs_bmbt_block.bb_level]
        movzx   ecx, [ebx+xfs_bmbt_block.bb_numrecs]
        xchg    cl, ch
        add     ebx, [ebp+XFS.bmbt_block_size]
.common:
        test    eax, eax
        jz      .leaf
.node:
.next_rec:
        dec     ecx
        js      .error
        movbe   eax, [ebx+ecx*sizeof.xfs_bmbt_key+xfs_bmbt_key.br_startoff.lo]
        cmp     [ebp+XFS.offset_begin.hi], eax
        ja      .node_found
        jb      .next_rec
        movbe   eax, [ebx+ecx*sizeof.xfs_bmbt_key+xfs_bmbt_key.br_startoff.hi]
        cmp     [ebp+XFS.offset_begin.lo], eax
        jae     .node_found
        jmp     .next_rec
.node_found:
        add     ebx, esi
        movbe   edx, [ebx+ecx*sizeof.xfs_bmbt_ptr+xfs_bmbt_ptr.lo]
        movbe   eax, [ebx+ecx*sizeof.xfs_bmbt_ptr+xfs_bmbt_ptr.hi]
        mov     ebx, [ebp+XFS.cur_block]
        stdcall xfs._.read_block
        test    eax, eax
        jnz     .error
        mov     ebx, [ebp+XFS.cur_block]
        jmp     .not_root
.leaf:
        jmp     .quit
.error:
.quit:
        ret
endp


proc xfs._.walk_btree uses ebx esi edi, _ptr, _size, _callback_extent, _callback_block, _callback_data, _is_root
        stdcall xfs._.long_btree.seek, [_ptr+4], [_size]
        mov     [_is_root], 0
.begin:
        mov     ebx, [ebp+XFS.cur_block]
        mov     eax, [ebp+XFS.bmap_magic]
        cmp     [ebx+xfs_bmbt_block.bb_magic], eax
        movi    eax, ERROR_FS_FAIL
        jnz     .error
        movzx   ecx, [ebx+xfs_bmbt_block.bb_numrecs]
        xchg    cl, ch
        add     ebx, [ebp+XFS.bmbt_block_size]
        stdcall xfs._.walk_extent_list, ecx, ebx, [_callback_extent+8], [_callback_block+4], [_callback_data]
        jnz     .error
        mov     esi, [ebp+XFS.offset_begin.lo]
        mov     edi, [ebp+XFS.offset_begin.hi]
        cmp     edi, [ebp+XFS.offset_end.hi]
        ja      .quit
        cmp     esi, [ebp+XFS.offset_end.lo]
        jae     .quit
        sub     ebx, [ebp+XFS.bmbt_block_size]
        movbe   edx, [ebx+xfs_bmbt_block.bb_rightsib.lo]
        movbe   eax, [ebx+xfs_bmbt_block.bb_rightsib.hi]
        mov     ecx, eax
        and     ecx, edx
        inc     ecx
        jz      .quit
        mov     ebx, [ebp+XFS.cur_block]
        stdcall xfs._.read_block
        jnz     .error
        jmp     .begin
.error:
.quit:
        ret
endp


proc xfs._.btree_read_block uses ebx esi edi, _tree, _size, _block_lo, _block_hi, _buf
        mov     eax, [_block_lo]
        mov     [ebp+XFS.offset_begin.lo], eax
        mov     eax, [_block_hi]
        mov     [ebp+XFS.offset_begin.hi], eax
        stdcall xfs._.long_btree.seek, [_tree+4], [_size]
        jnz     .error
        mov     ebx, [ebp+XFS.cur_block]
        mov     eax, [ebp+XFS.bmap_magic]
        cmp     [ebx+xfs_bmbt_block.bb_magic], eax
        jnz     .error
        movzx   ecx, [ebx+xfs_bmbt_block.bb_numrecs]
        xchg    cl, ch
        add     ebx, [ebp+XFS.bmbt_block_size]
        mov     eax, [_block_lo]
        mov     [ebp+XFS.offset_begin.lo], eax
        mov     eax, [_block_hi]
        mov     [ebp+XFS.offset_begin.hi], eax
        stdcall xfs._.extent_list.seek, ecx
        stdcall xfs._.read_dirblock, [ebp+XFS.extent.br_startblock.lo], \
                [ebp+XFS.extent.br_startblock.hi], [_buf]
.error:
.quit:
        ret
endp


proc xfs._.extent_list.seek uses esi, _count
        sub     ebx, sizeof.xfs_bmbt_rec
        inc     [_count]
.find_low:
        add     ebx, sizeof.xfs_bmbt_rec
        dec     [_count]
        jz      .quit
        stdcall xfs._.extent_unpack, ebx
        mov     eax, [ebp+XFS.extent.br_startoff.lo]
        mov     edx, [ebp+XFS.extent.br_startoff.hi]
        mov     esi, [ebp+XFS.extent.br_blockcount]
        add     eax, esi
        adc     edx, 0

        cmp     edx, [ebp+XFS.offset_begin.hi]
        ja      .low_found
        jb      .find_low
        cmp     eax, [ebp+XFS.offset_begin.lo]
        ja      .low_found
        jmp     .find_low
.low_found:
        add     ebx, sizeof.xfs_bmbt_rec

        mov     eax, [ebp+XFS.offset_begin.lo]
        mov     edx, [ebp+XFS.offset_begin.hi]
        mov     esi, eax
        sub     esi, [ebp+XFS.extent.br_startoff.lo]
        jbe     .quit
        ; same br_blockcount for block and dirblock?
        mov     [ebp+XFS.extent.br_startoff.lo], eax
        mov     [ebp+XFS.extent.br_startoff.hi], edx
        sub     [ebp+XFS.extent.br_blockcount], esi
        add     [ebp+XFS.extent.br_startblock.lo], esi
        adc     [ebp+XFS.extent.br_startblock.hi], 0
        jmp     .quit
.quit:
        mov     eax, [_count]
        ret
endp


proc xfs._.extent_iterate_dirblocks _callback, _callback_data
.check_high:
        cmp     edi, [ebp+XFS.offset_end.hi]
        ja      .quit
        jb      .read_dirblock
        cmp     esi, [ebp+XFS.offset_end.lo]
        jae     .quit
.read_dirblock:
        stdcall xfs._.read_dirblock, [ebp+XFS.extent.br_startblock.lo], [ebp+XFS.extent.br_startblock.hi], [ebp+XFS.cur_dirblock]
        mov     edx, [ebp+XFS.cur_dirblock]
        mov     eax, [_callback]
        stdcall eax, edx, esi, edi, [_callback_data]
        test    eax, eax
        jnz     .error
        mov     eax, [ebp+XFS.blkpdirblk]
        add     esi, eax
        adc     edi, 0
        add     [ebp+XFS.extent.br_startblock.lo], eax
        adc     [ebp+XFS.extent.br_startblock.hi], 0
        sub     [ebp+XFS.extent.br_blockcount], eax
        jnz     .check_high
.error:
.quit:
        ret
endp


proc xfs._.walk_extent_list uses ebx esi edi, _count, _ptr, _callback_extent, _callback_block, _callback_data
        mov     ebx, [_ptr]
        stdcall xfs._.extent_list.seek, [_count]
        mov     [_count], eax
        dec     [_count]
        js      .quit
        jmp     .next_extent.decoded
.next_extent:
        stdcall xfs._.extent_unpack, ebx
        add     ebx, sizeof.xfs_bmbt_rec
.next_extent.decoded:
        mov     eax, [ebp+XFS.extent.br_blockcount]
        add     [ebp+XFS.offset_begin.lo], eax
        adc     [ebp+XFS.offset_begin.hi], 0
        mov     esi, [ebp+XFS.extent.br_startoff.lo]
        mov     edi, [ebp+XFS.extent.br_startoff.hi]
        stdcall [_callback_extent+8], [_callback_block+4], [_callback_data]
        jnz     .error
        cmp     edi, [ebp+XFS.offset_end.hi]
        ja      .quit
        jb      @f
        cmp     esi, [ebp+XFS.offset_end.lo]
        jae     .quit
@@:
        dec     [_count]
        js      .quit
        jmp     .next_extent
.quit:
        movi    eax, ERROR_SUCCESS
.error:
        test    eax, eax
        ret
endp


proc xfs._.get_last_dirblock uses ecx
        mov     eax, [ebp+XFS.nextents_offset]
        movbe   eax, [ebx+eax]
assert (sizeof.xfs_bmbt_rec AND (sizeof.xfs_bmbt_rec - 1)) = 0
        shl     eax, BSF sizeof.xfs_bmbt_rec
        add     eax, [ebp+XFS.inode_core_size]
        lea     eax, [ebx+eax-sizeof.xfs_bmbt_rec]
        stdcall xfs._.extent_unpack, eax
        xor     edx, edx
        mov     eax, [ebp+XFS.extent.br_blockcount]
        mov     ecx, [ebp+XFS.dirblklog]
        shr     eax, cl
        dec     eax
        add     eax, [ebp+XFS.extent.br_startoff.lo]
        adc     edx, [ebp+XFS.extent.br_startoff.hi]
        ret
endp


restore prologue@proc,epilogue@proc
restore movbe
