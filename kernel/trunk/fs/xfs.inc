;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2022. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License   ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; file types from stat.h
S_IFMT      = 0170000o  ; These bits determine file type
S_IFDIR     = 0040000o  ; Directory
S_IFCHR     = 0020000o  ; Character device
S_IFBLK     = 0060000o  ; Block device
S_IFREG     = 0100000o  ; Regular file
S_IFIFO     = 0010000o  ; FIFO
S_IFLNK     = 0120000o  ; Symbolic link
S_IFSOCK    = 0140000o  ; Socket

; XFS null constant: empty fields must be all ones, not zeros!
XFS_NULL                = -1

XFS_SECT_SB             = 0

; signatures of file system structures
XFS_AGF_MAGIC           = 'XAGF'
XFS_DINODE_MAGIC        = 'IN'
XFS_BMAP_MAGIC          = 'BMAP'
XFS_DA_NODE_MAGIC       = 0xbefe
XFS_DIR2_LEAF1_MAGIC    = 0xf1d2
XFS_DIR2_LEAFN_MAGIC    = 0xffd2
XFS_DIR2_BLOCK_MAGIC    = 'XD2B'
XFS_DIR2_DATA_MAGIC     = 'XD2D'

XFS_BMAP3_MAGIC         = 'BMA3'
XFS_DA3_NODE_MAGIC      = 0xbe3e        ; non-leaf blocks
XFS_DIR3_LEAF1_MAGIC    = 0xf13d        ; v3 dir single blks
XFS_DIR3_LEAFN_MAGIC    = 0xff3d        ; v3 dir multi blks
XFS_DIR3_BLOCK_MAGIC    = 'XDB3'        ; single block dirs
XFS_DIR3_DATA_MAGIC     = 'XDD3'        ; multiblock dirs

XFS_SB_MAGIC               = 'XFSB'
XFS_SB_VERSION_NUMBITS     = 0x000f
XFS_SB_VERSION_ALLFBITS    = 0xfff0
XFS_SB_VERSION_REALFBITS   = 0x0ff0
XFS_SB_VERSION_ATTRBIT     = 0x0010
XFS_SB_VERSION_NLINKBIT    = 0x0020
XFS_SB_VERSION_QUOTABIT    = 0x0040
XFS_SB_VERSION_ALIGNBIT    = 0x0080
XFS_SB_VERSION_DALIGNBIT   = 0x0100
XFS_SB_VERSION_SHAREDBIT   = 0x0200
XFS_SB_VERSION_LOGV2BIT    = 0x0400
XFS_SB_VERSION_SECTORBIT   = 0x0800
XFS_SB_VERSION_EXTFLGBIT   = 0x1000
XFS_SB_VERSION_DIRV2BIT    = 0x2000
XFS_SB_VERSION_BORGBIT     = 0x4000     ; ASCII only case-insensitive
XFS_SB_VERSION_MOREBITSBIT = 0x8000

XFS_SB_VERSION_SUPPORTED = XFS_SB_VERSION_NUMBITS     OR \
                           XFS_SB_VERSION_ATTRBIT     OR \
                           XFS_SB_VERSION_NLINKBIT    OR \
                           XFS_SB_VERSION_QUOTABIT    OR \
                           XFS_SB_VERSION_ALIGNBIT    OR \
                           XFS_SB_VERSION_LOGV2BIT    OR \
                           XFS_SB_VERSION_SECTORBIT   OR \
                           XFS_SB_VERSION_EXTFLGBIT   OR \
                           XFS_SB_VERSION_DIRV2BIT    OR \
                           XFS_SB_VERSION_MOREBITSBIT

XFS_SB_VERSION2_RESERVED1BIT   = 0x00000001
XFS_SB_VERSION2_LAZYSBCOUNTBIT = 0x00000002     ; Superblk counters
XFS_SB_VERSION2_RESERVED4BIT   = 0x00000004
XFS_SB_VERSION2_ATTR2BIT       = 0x00000008     ; Inline attr rework
XFS_SB_VERSION2_PARENTBIT      = 0x00000010     ; parent pointers in xattr
XFS_SB_VERSION2_PROJID32BIT    = 0x00000080     ; 32 bit project id
XFS_SB_VERSION2_CRCBIT         = 0x00000100     ; metadata CRCs
XFS_SB_VERSION2_FTYPE          = 0x00000200     ; inode type in dir

XFS_SB_VERSION2_SUPPORTED = XFS_SB_VERSION2_LAZYSBCOUNTBIT OR \
                            XFS_SB_VERSION2_ATTR2BIT       OR \
                            XFS_SB_VERSION2_PARENTBIT      OR \
                            XFS_SB_VERSION2_PROJID32BIT    OR \
                            XFS_SB_VERSION2_CRCBIT         OR \
                            XFS_SB_VERSION2_FTYPE

XFS_SB_FEAT_INCOMPAT_FTYPE     = 1      ; filetype in dirent
XFS_SB_FEAT_INCOMPAT_SPINODES  = 2      ; sparse inode chunks
XFS_SB_FEAT_INCOMPAT_META_UUID = 4      ; metadata UUID
XFS_SB_FEAT_INCOMPAT_BIGTIME   = 8      ; large timestamps
XFS_SB_FEAT_INCOMPAT_SUPPORTED = XFS_SB_FEAT_INCOMPAT_FTYPE    OR \
                                 XFS_SB_FEAT_INCOMPAT_SPINODES OR \
                                 XFS_SB_FEAT_INCOMPAT_META_UUID OR \
                                 XFS_SB_FEAT_INCOMPAT_BIGTIME

; bitfield lengths for packed extent
; MSB to LSB / left to right
BMBT_EXNTFLAG_BITLEN    =  1
BMBT_STARTOFF_BITLEN    = 54
BMBT_STARTBLOCK_BITLEN  = 52
BMBT_BLOCKCOUNT_BITLEN  = 21

XFS_DIR2_DATA_ALIGN_LOG = 3
XFS_DIR2_DATA_ALIGN     = 1 SHL XFS_DIR2_DATA_ALIGN_LOG
XFS_DIR2_SPACE_SIZE     = (1 SHL (32 + XFS_DIR2_DATA_ALIGN_LOG))
XFS_DIR2_DATA_OFFSET    = 0*XFS_DIR2_SPACE_SIZE
XFS_DIR2_LEAF_OFFSET    = 1*XFS_DIR2_SPACE_SIZE
XFS_DIR2_FREE_OFFSET    = 2*XFS_DIR2_SPACE_SIZE

; data section magic constants for directories (xfs_dir2_data.h)
XFS_DIR2_DATA_FD_COUNT  = 3

; valid inode formats
; enum xfs_dinode_fmt (xfs_dinode.h)
XFS_DINODE_FMT_DEV      = 0
XFS_DINODE_FMT_LOCAL    = 1
XFS_DINODE_FMT_EXTENTS  = 2
XFS_DINODE_FMT_BTREE    = 3
XFS_DINODE_FMT_UUID     = 4

; size of the unlinked inode hash table in the agi
XFS_AGI_UNLINKED_BUCKETS = 64

; possible extent states
; enum xfs_exntst_t (xfs_bmap_btree.h)
XFS_EXT_NORM            = 0
XFS_EXT_UNWRITTEN       = 1
XFS_EXT_DMAPI_OFFLINE   = 2
XFS_EXT_INVALID         = 3

; values for inode core flags / di_flags (xfs_dinode.h)
XFS_DIFLAG_REALTIME_BIT     =  0        ; file's blocks come from rt area
XFS_DIFLAG_PREALLOC_BIT     =  1        ; file space has been preallocated
XFS_DIFLAG_NEWRTBM_BIT      =  2        ; for rtbitmap inode, new format
XFS_DIFLAG_NODUMP_BIT       =  7        ; do not dump
XFS_DIFLAG_REALTIME         = (1 SHL XFS_DIFLAG_REALTIME_BIT)
XFS_DIFLAG_PREALLOC         = (1 SHL XFS_DIFLAG_PREALLOC_BIT)
XFS_DIFLAG_NEWRTBM          = (1 SHL XFS_DIFLAG_NEWRTBM_BIT)
XFS_DIFLAG_NODUMP           = (1 SHL XFS_DIFLAG_NODUMP_BIT)

; superblock _ondisk_ structure (xfs_sb.h)
; this is _not_ the partition structure
; for XFS partition structure see XFS below
struct xfs_sb
        sb_magicnum              dd ?   ; signature, must be XFS_SB_MAGIC
        sb_blocksize             dd ?   ; block is the minimal file system unit, in bytes
        sb_dblocks               DQ ?   ; number of data blocks
        sb_rblocks               DQ ?   ; number of realtime blocks
        sb_rextents              DQ ?   ; number of realtime extents
        sb_uuid                  rb 16  ; file system unique identifier
        sb_logstart              DQ ?   ; starting block of log (for internal journal)
        sb_rootino               DQ ?   ; root inode number
        sb_rbmino                DQ ?   ; bitmap inode for realtime extents
        sb_rsumino               DQ ?   ; summary inode for rt bitmap
        sb_rextsize              dd ?   ; realtime extent size, blocks
        sb_agblocks              dd ?   ; size of an allocation group (the last one may be smaller!)
        sb_agcount               dd ?   ; number of allocation groups
        sb_rbmblocks             dd ?   ; number of rt bitmap blocks
        sb_logblocks             dd ?   ; number of log blocks
        sb_versionnum            dw ?   ; header version
        sb_sectsize              dw ?   ; volume sector size in bytes
        sb_inodesize             dw ?   ; inode size, bytes
        sb_inopblock             dw ?   ; inodes per block
        sb_fname                 rb 12  ; inodes per block (aka label)
        sb_blocklog              db ?   ; log2 of sb_blocksize
        sb_sectlog               db ?   ; log2 of sb_blocksize
        sb_inodelog              db ?   ; log2 of sb_inodesize
        sb_inopblog              db ?   ; log2 of sb_inopblock
        sb_agblklog              db ?   ; log2 of sb_agblocks (rounded up!)
        sb_rextslog              db ?   ; log2 of sb_rextents
        sb_inprogress            db ?   ; mkfs is in progress, don't mount
        sb_imax_pct              db ?   ; max % of fs for inode space
        ; statistics
        sb_icount                DQ ?   ; allocated inodes
        sb_ifree                 DQ ?   ; free inodes
        sb_fdblocks              DQ ?   ; free data blocks
        sb_frextents             DQ ?   ; free realtime extents

        sb_uquotino              DQ ?   ; user quota inode
        sb_gquotino              DQ ?   ; group quota inode
        sb_qflags                dw ?   ; quota flags
        sb_flags                 db ?   ; misc. flags
        sb_shared_vn             db ?   ; shared version number
        sb_inoalignmt            dd ?   ; inode chunk alignment, fsblocks
        sb_unit                  dd ?   ; stripe or raid unit
        sb_width                 dd ?   ; stripe or raid width
        sb_dirblklog             db ?   ; log2 of dir block size (fsbs)
        sb_logsectlog            db ?   ; log2 of the log sector size
        sb_logsectsize           dw ?   ; sector size for the log, bytes
        sb_logsunit              dd ?   ; stripe unit size for the log
        sb_features2             dd ?   ; additional feature bits
        sb_bad_features2         dd ?
        sb_features_compat       dd ?
        sb_features_ro_compat    dd ?
        sb_features_incompat     dd ?
        sb_features_log_incompat dd ?
        sb_crc                   dd ?   ; superblock crc
        sb_spino_align           dd ?   ; sparse inode chunk alignment
        sb_pquotino              DQ ?   ; project quota inode
        sb_lsn                   DQ ?   ; last write sequence
        sb_meta_uuid             rb 16  ; metadata file system unique id
ends

; inode core structure: basic information about file
struct xfs_dinode_core
        di_magic         dw ?           ; inode magic = XFS_DINODE_MAGIC
        di_mode          dw ?           ; mode and type of file
        di_version       db ?           ; inode version
        di_format        db ?           ; format of di_c data
        di_onlink        dw ?           ; old number of links to file
        di_uid           dd ?           ; owner's user id
        di_gid           dd ?           ; owner's group id
        di_nlink         dd ?           ; number of links to file
        di_projid        dw ?           ; owner's project id
        di_pad           rb 8           ; unused, zeroed space
        di_flushiter     dw ?           ; incremented on flush
        di_atime         DQ             ; time last accessed
        di_mtime         DQ               ; time last modified
        di_ctime         DQ             ; time created/inode modified
        di_size          DQ ?           ; number of bytes in file
        di_nblocks       DQ ?           ; number of direct & btree blocks used
        di_extsize       dd ?           ; basic/minimum extent size for file
        di_nextents      dd ?           ; number of extents in data fork
        di_anextents     dw ?           ; number of extents in attribute fork
        di_forkoff       db ?           ; attr fork offs, <<3 for 64b align
        di_aformat       db ?           ; format of attr fork's data
        di_dmevmask      dd ?           ; DMIG event mask
        di_dmstate       dw ?           ; DMIG state info
        di_flags         dw ?           ; random flags, XFS_DIFLAG_...
        di_gen           dd ?           ; generation number
        di_next_unlinked dd ?           ; unlinked but still used inode (if any, XFS_NULL otherwise)
ends

struct xfs_dinode3_core xfs_dinode_core
        di_crc           dd ?           ; CRC of the inode
        di_changecount   DQ ?           ; number of attribute changes
        di_lsn           DQ ?           ; flush sequence
        di_flags2        DQ ?           ; more random flags
        di_cowextsize    dd ?           ; basic cow extent size for file
        di_pad2          rb 12          ; more padding for future expansion

        ; fields only written to during inode creation
        di_crtime        DQ             ; time created
        di_ino           DQ ?           ; inode number
        di_uuid          rb 16          ; UUID of the filesystem
ends


struct xfs_dir2_sf_hdr
        count           db ?
        i8count         db ?
        parent          DQ ?            ; parent inode number, 4 or 8 bytes
ends         

struct xfs_dir2_sf_entry
        namelen         db ?            ; actual name length (ASCII)
        offset          rb 2            ; saved offset
        name            db ?            ; name, variable size
        inumber         DQ ?            ; 4 or 8 bytes
ends

struct xfs_dir2_sf
        hdr     xfs_dir2_sf_hdr
        entries xfs_dir2_sf_entry
ends

; active entry in a data block
; aligned to 8 bytes
; tag appears as the last 2 bytes
struct xfs_dir2_data_entry
        inumber         DQ ?
        namelen         db ?
        name            db ?            ; name bytes array without terminator
;       tag             dw ?            ; starting offset of the entry
ends

; unused entry in a data block
struct xfs_dir2_data_unused
        freetag         dw ?            ; XFS_DIR2_DATA_FREE_TAG aka XFS_NULL
        length          dw ?            ; total free length
;        tag             dw ?           ; starting offset of the entry
ends

; generic data entry
struct xfs_dir2_data_union
  union
        xentry          xfs_dir2_data_entry
        unused          xfs_dir2_data_unused
  ends
ends

; describe a free area in the data block
; the freespace will be formatted as a xfs_dir2_data_unused_t
struct xfs_dir2_data_free
        offset          dw ?            ; start of freespace
        length          dw ?            ; length of freespace
ends

; header for the data blocks
; always at the beginning of a directory-sized block
; the code knows that XFS_DIR2_DATA_FD_COUNT is 3
struct xfs_dir2_data_hdr
        magic           dd ?            ; XFS_DIR2_DATA_MAGIC or XFS_DIR2_BLOCK_MAGIC
        bestfree        xfs_dir2_data_free
        bestfree2       xfs_dir2_data_free
        bestfree3       xfs_dir2_data_free
ends

struct xfs_dir3_data_hdr xfs_dir2_data_hdr
        magic3  dd ?
        crc     dd ?
        blkno   DQ ?
        lsn     DQ ?
        uuid    rb 16
        owner   DQ ?
ends

; leaf block entry
struct xfs_dir2_leaf_entry
        hashval         dd ?            ; hash value of name
        address         dd ?            ; address of data entry
ends

; the tail of directory block
struct xfs_dir2_block_tail
        count           dd ?            ; count of leaf entries
        stale           dd ?            ; count of stale leaf entries
ends

; generic single-block structure, for xfs_db
struct xfs_dir2_block
        hdr             xfs_dir2_data_hdr       ; magic XFS_DIR2_DATA_MAGIC
        u               xfs_dir2_data_union
;        leaf            xfs_dir2_leaf_entry
;        tail            xfs_dir2_block_tail
ends

struct xfs_da_blkinfo
        forw            dd ?            ; previous block in list
        back            dd ?            ; following block in list
        magic           dw ?            ; validity check on block
        pad             dw ?            ; unused
ends

struct xfs_dir2_leaf_hdr
        info            xfs_da_blkinfo
        count           dw ?
        stale           dw ?
ends

struct xfs_da3_blkinfo xfs_da_blkinfo
        crc   dd ?      ; CRC of block
        blkno DQ ?      ; first block of the buffer
        lsn   DQ ?      ; sequence number of last write
        uuid  rb 16     ; filesystem we belong to
        owner DQ ?      ; inode that owns the block
ends

struct xfs_dir3_leaf_hdr
        info            xfs_da3_blkinfo
        count           dw ?
        stale           dw ?
        pad             dd ?
ends

; bests and tail are at the end of the block for single-leaf only
; (magic = XFS_DIR2_LEAF1_MAGIC not XFS_DIR2_LEAFN_MAGIC)
struct xfs_dir2_leaf
        hdr             xfs_dir2_leaf_hdr
        ents            xfs_dir2_leaf_entry
;       bests           dw ?
;       tail            xfs_dir2_leaf_tail
ends

struct xfs_dir3_leaf
        hdr             xfs_dir3_leaf_hdr
        ents            xfs_dir2_leaf_entry
;       bests
;       tail
ends

struct xfs_dir2_free_hdr
        magic           dd ?    ; XFS_DIR2_FREE_MAGIC
        firstdb         dd ?    ; db of first entry
        nvalid          dd ?    ; count of valid entries
        nused           dd ?    ; count of used entries
ends

struct xfs_da_node_hdr
        info            xfs_da_blkinfo
        count           dw ?
        level           dw ?
ends

struct xfs_da_node_entry
        hashval         dd ?            ; hash value for this descendant
        before          dd ?            ; Btree block before this key
ends

struct xfs_da_intnode
        hdr             xfs_da_node_hdr
        btree           xfs_da_node_entry
ends

struct xfs_da3_node_hdr
        info            xfs_da3_blkinfo
        count           dw ?
        level           dw ?
        pad             dd ?
ends

struct xfs_da3_intnode
        hdr             xfs_da3_node_hdr
        btree           xfs_da_node_entry
ends

; packet extent
struct xfs_bmbt_rec
        l0              DQ ?
        l1              DQ ?
ends

; unpacked extent
struct xfs_bmbt_irec
        br_startoff     DQ ?
        br_startblock   DQ ?
        br_blockcount   dd ?
        br_state        dd ?
ends

struct xfs_dir2_bmx
        bmx     xfs_bmbt_rec
ends

; bmap root header
struct xfs_bmdr_block
        bb_level        dw ?            ; 0 is a leaf
        bb_numrecs      dw ?            ; current number of data records
ends

; key structure for non-leaf levels of the tree
struct xfs_bmbt_key
        br_startoff     DQ ?            ; starting file offset
ends

struct xfs_bmdr_ptr DQ
ends

struct xfs_bmbt_ptr DQ
ends

; long form header: bmap btrees
; xfs_btree_lblock is xfs_bmbt_block (xfs_btree.h)
struct xfs_bmbt_block
        bb_magic        dd ?            ; magic number for block type
        bb_level        dw ?            ; 0 is a leaf
        bb_numrecs      dw ?            ; current number of data records
        bb_leftsib      DQ ?            ; left sibling block or NULLDFSBNO
        bb_rightsib     DQ ?            ; right sibling block or NULLDFSBNO
ends

struct xfs_bmbt3_block xfs_bmbt_block
        bb_blkno        DQ ?
        bb_lsn          DQ ?
        bb_uuid         rb 16
        bb_owner        DQ ?
        bb_crc          dd ?
        bb_pad          dd ?
ends

struct xfs_inode
        di_core                 xfs_dinode_core ; main info, aka core
        union
                di_u            db ?            ; data fork inode part
                dir2_sf         xfs_dir2_sf
                bmx             xfs_dir2_bmx
        ends
;        di_a                    db ?           ; data attribute
ends

struct xfs_agf
        agf_magicnum    dd ?    ; magic number == XFS_AGF_MAGIC
        agf_versionnum  dd ?    ; header version == XFS_AGF_VERSION
        agf_seqno       dd ?    ; sequence # starting from 0
        agf_length      dd ?    ; size in blocks of AG
        agf_roots       dd ?
        agf_levels      dd ?
        agf_flfirst     dd ?
        agf_fllast      dd ?
        agf_flcount     dd ?
        agf_freeblks    dd ?    ; free blocks in AG
ends

; internal data for every XFS partition
; this _is_ XFS partition structure
; most fields are unpacked or bswap'ed values of the superblock, see xfs_sb structure above
struct XFS PARTITION
        Lock                    MUTEX ?         ; access mutex
        sectsize                dd ?
        blocksize               dd ?
        dirblocksize            dd ?
        inodesize               dd ?
        rootino                 DQ ?
        versionnum              dd ?
        version                 dd ?
        features2               dd ?
        inopblock               dd ?
        blkpdirblk              dd ?
        blocklog                dd ?
        sectlog                 dd ?
        inodelog                dd ?
        inopblog                dd ?
        agblklog                dd ?
        sectpblog               dd ?
        dirblklog               dd ?    ; in fsblocks
        sectpblock              dd ?
        agblocks                dd ?
        dir2_leaf_offset_blocks DQ ?
        dir2_free_offset_blocks DQ ?
        agblockmask             DQ ?
        inode_core_size         dd ?
        features_incompat       dd ?
        ftype_size              dd ?
        dir_block_magic         dd ?
        dir_data_magic          dd ?
        dir_leaf1_magic         dw ?
        dir_leafn_magic         dw ?
        da_node_magic           dw ?
        bmap_magic              dd ?
        bmbt_block_size         dd ?
        dir_block_size          dd ?
        dir_data_size           dd ?
        dir_leaf1_size          dd ?
        dir_leafn_size          dd ?
        da_node_size            dd ?
        da_blkinfo_size         dd ?
        conv_time_to_kos_epoch  dd ?
        ; helpers, temporary vars, etc
        ; should go to file descriptor and local vars?
        cur_block               dd ?
        cur_block_data          dd ?
        cur_inode               dd ?
        cur_sect                dd ?
        cur_dirblock            dd ?
        tmp_inode               dd ?
        extent                  xfs_bmbt_irec
        bytes_to_read           dd ?
        bytes_read              dd ?
        bytes_left_in_file      DQ ?
        file_offset             DQ ?
        file_buffer             dd ?
        entries_read            dd ?
        requested_cnt           dd ?
        dir_sf_self_done        dd ?
        dir_sf_parent_done      dd ?
        entries_left_in_dir     dd ?
        entries_to_skip         dd ?
        max_dirblockaddr        dd ?
        cur_inode_save          dd ?
        shortform_inodelen      dd ?
        bdfe_nameenc            dd ?
        bdfe_len                dd ?
        bdfe_process            dd ?
        inode_self              DQ ?
        bdfe_buf                dd ?
        eof                     dd ?
        offset_begin            DQ ?
        offset_end              DQ ?
ends

struct f70s0arg
        sf       dd ?
        offset   DQ ?
        count    dd ?
        buf      dd ?
        zero     db ?
        path     dd ?
ends

struct f70s1arg
        sf        dd ?
        start_idx dd ?
        encoding  dd ?
        count     dd ?
        buf       dd ?
        zero      db ?
        path      dd ?
ends

struct f70s5arg
        sf     dd ?
               dd ?
        xflags dd ?     ; name flags is already used
               dd ?
        buf    dd ?
        zero   db ?
        path   dd ?
ends

struct bdfe_hdr
        version   dd ?
        read_cnt  dd ?
        total_cnt dd ?
        zeroed    rd 5
ends

struct bdfe
        attr     dd ?
        nameenc  dd ?
        ctime    dd ?
        cdate    dd ?
        atime    dd ?
        adate    dd ?
        mtime    dd ?
        mdate    dd ?
        size     DQ ?
        name     db ?
ends
