;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2014. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 4850 $


; from stat.h
; distinguish file types
S_IFMT      = 0170000o  ; These bits determine file type.
S_IFDIR     = 0040000o  ; Directory.
S_IFCHR     = 0020000o  ; Character device.
S_IFBLK     = 0060000o  ; Block device.
S_IFREG     = 0100000o  ; Regular file.
S_IFIFO     = 0010000o  ; FIFO.
S_IFLNK     = 0120000o  ; Symbolic link.
S_IFSOCK    = 0140000o  ; Socket.
; end stat.h


; XFS null constant: empty fields must be all ones, not zeros!
XFS_NULL                = -1


; static sector numbers
XFS_SECT_SB             = 0
XFS_SECT_AGF            = 1
XFS_SECT_AGI            = 2
XFS_SECT_AGFL           = 3


; signatures of file system structures
; 'string' numbers are treated by fasm as big endian
XFS_SB_MAGIC            = 'XFSB'
XFS_AGF_MAGIC           = 'XAGF'
XFS_AGI_MAGIC           = 'XAGI'
XFS_ABTB_MAGIC          = 'ABTB'
XFS_ABTC_MAGIC          = 'ABTC'
XFS_IBT_MAGIC           = 'IABT'
XFS_DINODE_MAGIC        = 'IN'
XFS_BMAP_MAGIC          = 'BMAP'
XFS_DA_NODE_MAGIC       = 0xbefe        ; those are little endian here
XFS_ATTR_LEAF_MAGIC     = 0xeefb        ; but big endian in docs
XFS_DIR2_LEAF1_MAGIC    = 0xf1d2        ; pay attention!
XFS_DIR2_LEAFN_MAGIC    = 0xffd2        ;
XFS_DIR2_BLOCK_MAGIC    = 'XD2B'
XFS_DIR2_DATA_MAGIC     = 'XD2D'
XFS_DIR2_FREE_MAGIC     = 'XD2F'
XFS_DQUOT_MAGIC         = 'DQ'


; bitfield lengths for packed extent
; MSB to LSB / left to right
BMBT_EXNTFLAG_BITLEN    =  1
BMBT_STARTOFF_BITLEN    = 54
BMBT_STARTBLOCK_BITLEN  = 52
BMBT_BLOCKCOUNT_BITLEN  = 21


; those constants are taken from linux source (xfs_dir2_leaf.h)
; they are magic infile offsets for directories
XFS_DIR2_DATA_ALIGN_LOG = 3     ; i.e., 8 bytes
XFS_DIR2_LEAF_SPACE     = 1
XFS_DIR2_SPACE_SIZE     = (1 SHL (32 + XFS_DIR2_DATA_ALIGN_LOG))
XFS_DIR2_LEAF_OFFSET    = (XFS_DIR2_LEAF_SPACE * XFS_DIR2_SPACE_SIZE)
XFS_DIR2_FREE_SPACE     = 2
XFS_DIR2_SPACE_SIZE     = (1 SHL (32 + XFS_DIR2_DATA_ALIGN_LOG))
XFS_DIR2_FREE_OFFSET    = (XFS_DIR2_FREE_SPACE * XFS_DIR2_SPACE_SIZE)


; data section magic constants for directories (xfs_dir2_data.h)
XFS_DIR2_DATA_FD_COUNT  = 3
XFS_DIR2_DATA_FREE_TAG  = 0xffff


; valid inode formats
; enum xfs_dinode_fmt (xfs_dinode.h)
XFS_DINODE_FMT_DEV      = 0     ; xfs_dev_t
XFS_DINODE_FMT_LOCAL    = 1     ; one inode is enough (shortdir)
XFS_DINODE_FMT_EXTENTS  = 2     ; one or more extents (leafdir, nodedir, regular files)
XFS_DINODE_FMT_BTREE    = 3     ; highly fragmented files or really huge directories
XFS_DINODE_FMT_UUID     = 4     ; uuid_t


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
XFS_DIFLAG_IMMUTABLE_BIT    =  3        ; inode is immutable
XFS_DIFLAG_APPEND_BIT       =  4        ; inode is append-only
XFS_DIFLAG_SYNC_BIT         =  5        ; inode is written synchronously
XFS_DIFLAG_NOATIME_BIT      =  6        ; do not update atime
XFS_DIFLAG_NODUMP_BIT       =  7        ; do not dump
XFS_DIFLAG_RTINHERIT_BIT    =  8        ; create with realtime bit set
XFS_DIFLAG_PROJINHERIT_BIT  =  9        ; create with parents projid
XFS_DIFLAG_NOSYMLINKS_BIT   = 10        ; disallow symlink creation
XFS_DIFLAG_EXTSIZE_BIT      = 11        ; inode extent size allocator hint
XFS_DIFLAG_EXTSZINHERIT_BIT = 12        ; inherit inode extent size
XFS_DIFLAG_NODEFRAG_BIT     = 13        ; do not reorganize/defragment
XFS_DIFLAG_FILESTREAM_BIT   = 14        ; use filestream allocator
XFS_DIFLAG_REALTIME         = (1 SHL XFS_DIFLAG_REALTIME_BIT)
XFS_DIFLAG_PREALLOC         = (1 SHL XFS_DIFLAG_PREALLOC_BIT)
XFS_DIFLAG_NEWRTBM          = (1 SHL XFS_DIFLAG_NEWRTBM_BIT)
XFS_DIFLAG_IMMUTABLE        = (1 SHL XFS_DIFLAG_IMMUTABLE_BIT)
XFS_DIFLAG_APPEND           = (1 SHL XFS_DIFLAG_APPEND_BIT)
XFS_DIFLAG_SYNC             = (1 SHL XFS_DIFLAG_SYNC_BIT)
XFS_DIFLAG_NOATIME          = (1 SHL XFS_DIFLAG_NOATIME_BIT)
XFS_DIFLAG_NODUMP           = (1 SHL XFS_DIFLAG_NODUMP_BIT)
XFS_DIFLAG_RTINHERIT        = (1 SHL XFS_DIFLAG_RTINHERIT_BIT)
XFS_DIFLAG_PROJINHERIT      = (1 SHL XFS_DIFLAG_PROJINHERIT_BIT)
XFS_DIFLAG_NOSYMLINKS       = (1 SHL XFS_DIFLAG_NOSYMLINKS_BIT)
XFS_DIFLAG_EXTSIZE          = (1 SHL XFS_DIFLAG_EXTSIZE_BIT)
XFS_DIFLAG_EXTSZINHERIT     = (1 SHL XFS_DIFLAG_EXTSZINHERIT_BIT)
XFS_DIFLAG_NODEFRAG         = (1 SHL XFS_DIFLAG_NODEFRAG_BIT)
XFS_DIFLAG_FILESTREAM       = (1 SHL XFS_DIFLAG_FILESTREAM_BIT)


; superblock _ondisk_ structure (xfs_sb.h)
; this is _not_ the partition structure
; for XFS partition structure see XFS below
struct xfs_sb
        sb_magicnum     dd ?    ; signature, must be XFS_SB_MAGIC
        sb_blocksize    dd ?    ; block is the minimal file system unit, in bytes
        sb_dblocks      dq ?    ; number of data blocks
        sb_rblocks      dq ?    ; number of realtime blocks (not supported yet!)
        sb_rextents     dq ?    ; number of realtime extents (not supported yet!)
        sb_uuid         rb 16   ; file system unique identifier
        sb_logstart     dq ?    ; starting block of log (for internal journal; journals on separate devices are not supported!)
        sb_rootino      dq ?    ; root inode number
        sb_rbmino       dq ?    ; bitmap inode for realtime extents (ignored)
        sb_rsumino      dq ?    ; summary inode for rt bitmap (ignored)
        sb_rextsize     dd ?    ; realtime extent size, blocks
        sb_agblocks     dd ?    ; size of an allocation group (the last one may be smaller!)
        sb_agcount      dd ?    ; number of allocation groups
        sb_rbmblocks    dd ?    ; number of rt bitmap blocks
        sb_logblocks    dd ?    ; number of log blocks
        sb_versionnum   dw ?    ; header version == XFS_SB_VERSION
        sb_sectsize     dw ?    ; volume sector size in bytes (only 512B sectors are supported)
        sb_inodesize    dw ?    ; inode size, bytes
        sb_inopblock    dw ?    ; inodes per block
        sb_fname        rb 12   ; inodes per block (aka label)
        sb_blocklog     db ?    ; log2 of sb_blocksize
        sb_sectlog      db ?    ; log2 of sb_blocksize
        sb_inodelog     db ?    ; log2 of sb_inodesize
        sb_inopblog     db ?    ; log2 of sb_inopblock
        sb_agblklog     db ?    ; log2 of sb_agblocks (rounded up!)
        sb_rextslog     db ?    ; log2 of sb_rextents
        sb_inprogress   db ?    ; mkfs is in progress, don't mount
        sb_imax_pct     db ?    ; max % of fs for inode space
        ; statistics
        sb_icount       dq ?    ; allocated inodes
        sb_ifree        dq ?    ; free inodes
        sb_fdblocks     dq ?    ; free data blocks
        sb_frextents    dq ?    ; free realtime extents

        sb_uquotino     dq ?    ; user quota inode
        sb_gquotino     dq ?    ; group quota inode
        sb_qflags       dw ?    ; quota flags
        sb_flags        db ?    ; misc. flags
        sb_shared_vn    db ?    ; shared version number
        sb_inoalignmt   dd ?    ; inode chunk alignment, fsblocks
        sb_unit         dd ?    ; stripe or raid unit
        sb_width        dd ?    ; stripe or raid width
        sb_dirblklog    db ?    ; log2 of dir block size (fsbs)
        sb_logsectlog   db ?    ; log2 of the log sector size
        sb_logsectsize  dw ?    ; sector size for the log, bytes
        sb_logsunit     dd ?    ; stripe unit size for the log
        sb_features2    dd ?    ; additional feature bits
ends


; allocation group inode (xfs_ag.h)
struct xfs_agi
        agi_magicnum    dd ?    ; magic number == XFS_AGI_MAGIC
        agi_versionnum  dd ?    ; header version == XFS_AGI_VERSION
        agi_seqno       dd ?    ; sequence number starting from 0
        agi_length      dd ?    ; size in blocks of a.g.
        agi_count       dd ?    ; count of allocated inodes
        agi_root        dd ?    ; root of inode btree
        agi_level       dd ?    ; levels in inode btree
        agi_freecount   dd ?    ; number of free inodes
        agi_newino      dd ?    ; new inode just allocated
        agi_dirino      dd ?    ; last directory inode chunk
        agi_unlinked    rd XFS_AGI_UNLINKED_BUCKETS     ; Hash table of inodes which have been unlinked but are still being referenced
ends


; superblock structure of b+tree node/leaf (same structure, bb_level matters)
struct xfs_btree_sblock
        bb_magic        dd ?
        bb_level        dw ?    ; distinguishes nodeds and leaves
        bb_numrecs      dw ?
        bb_leftsib      dd ?
        bb_rightsib     dd ?
ends


; record of b+tree inode
struct xfs_inobt_rec
        ir_startino     dd ?
        ir_freecount    dd ?
        ir_free         dq ?
ends


; structure to store create, access and modification time in inode core
struct xfs_timestamp
        t_sec           dd ?
        t_nsec          dd ?    ; nanoseconds
ends


; inode core structure: basic information about file
struct xfs_dinode_core
        di_magic        dw ?            ; inode magic = XFS_DINODE_MAGIC
        di_mode         dw ?            ; mode and type of file
        di_version      db ?            ; inode version
        di_format       db ?            ; format of di_c data
        di_onlink       dw ?            ; old number of links to file
        di_uid          dd ?            ; owner's user id
        di_gid          dd ?            ; owner's group id
        di_nlink        dd ?            ; number of links to file
        di_projid       dw ?            ; owner's project id
        di_pad          rb 8            ; unused, zeroed space
        di_flushiter    dw ?            ; incremented on flush
        di_atime        xfs_timestamp   ; time last accessed
        di_mtime        xfs_timestamp   ; time last modified
        di_ctime        xfs_timestamp   ; time created/inode modified
        di_size         dq ?            ; number of bytes in file
        di_nblocks      dq ?            ; number of direct & btree blocks used
        di_extsize      dd ?            ; basic/minimum extent size for file
        di_nextents     dd ?            ; number of extents in data fork
        di_anextents    dw ?            ; number of extents in attribute fork
        di_forkoff      db ?            ; attr fork offs, <<3 for 64b align
        di_aformat      db ?            ; format of attr fork's data
        di_dmevmask     dd ?            ; DMIG event mask
        di_dmstate      dw ?            ; DMIG state info
        di_flags        dw ?            ; random flags, XFS_DIFLAG_...
        di_gen          dd ?            ; generation number
ends


; shortform dir header
struct xfs_dir2_sf_hdr
        count           db ?            ; the number of directory entries, used only if each inode number fits 4 bytes; zero otherwise
        i8count         db ?            ; the number of directory entries, used only when count is zero
        parent          dq ?            ; parent inode number: xfs_dir2_inou_t (4 or 8 bytes)
ends         


; shortform dir entry
struct xfs_dir2_sf_entry
        namelen         db ?            ; actual name length (ASCII)
        offset          rb 2            ; saved offset
        name            db ?            ; name, variable size
;       inumber         dq ?            ; xfs_dir2_inou_t
ends


; active entry in a data block
; aligned to 8 bytes
; tag appears as the last 2 bytes
struct xfs_dir2_data_entry
        inumber         dq ?            ; inode number
        namelen         db ?            ; name length
        name            db ?            ; name bytes, no null
;       tag             dw ?            ; starting offset of us
ends


; unused entry in a data block
; aligned to 8 bytes
; tag appears as the last 2 bytes
struct xfs_dir2_data_unused
        freetag         dw ?            ; XFS_DIR2_DATA_FREE_TAG
        length          dw ?            ; total free length
;        tag             dw ?           ; starting offset of us
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
        magic           dd ?                    ; XFS_DIR2_DATA_MAGIC or XFS_DIR2_BLOCK_MAGIC
        bestfree        xfs_dir2_data_free
        bestfree2       xfs_dir2_data_free
        bestfree3       xfs_dir2_data_free
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
        hdr             xfs_dir2_data_hdr
        u               xfs_dir2_data_union
;        leaf            xfs_dir2_leaf_entry
;        tail            xfs_dir2_block_tail
ends


; 
struct xfs_dir2_data
        hdr             xfs_dir2_data_hdr       ; magic XFS_DIR2_DATA_MAGIC
        u               xfs_dir2_data_union
ends


; 
struct xfs_da_blkinfo
        forw            dd ?            ; previous block in list
        back            dd ?            ; following block in list
        magic           dw ?            ; validity check on block
        pad             dw ?            ; unused
ends


; leaf block header
struct xfs_dir2_leaf_hdr
        info            xfs_da_blkinfo  ; header for da routines
        count           dw ?            ; count of entries
        stale           dw ?            ; count of stale entries
ends


; leaf block tail
struct xfs_dir2_leaf_tail
        bestcount       dd ?
ends


; leaf block
; bests and tail are at the end of the block for single-leaf only
; (magic = XFS_DIR2_LEAF1_MAGIC not XFS_DIR2_LEAFN_MAGIC)
struct xfs_dir2_leaf
        hdr             xfs_dir2_leaf_hdr       ; leaf header
        ents            xfs_dir2_leaf_entry     ; entries
;       bests           dw ?                    ; best free counts
;       tail            xfs_dir2_leaf_tail      ; leaf tail
ends


; header of 'free' block part
struct xfs_dir2_free_hdr
        magic           dd ?    ; XFS_DIR2_FREE_MAGIC
        firstdb         dd ?    ; db of first entry
        nvalid          dd ?    ; count of valid entries
        nused           dd ?    ; count of used entries
ends


; 'free' part of directiry block
struct xfs_dir2_free
        hdr             xfs_dir2_free_hdr       ; block header
        bests           dw ?                    ; best free counts
                                                ; unused entries are -1 (XFS_NULL)
ends


; b+tree node header
struct xfs_da_node_hdr
        info            xfs_da_blkinfo
        count           dw ?
        level           dw ?
ends


; b+tree node
struct xfs_da_node_entry
        hashval         dd ?            ; hash value for this descendant
        before          dd ?            ; Btree block before this key
ends


; 
struct xfs_da_intnode
        hdr             xfs_da_node_hdr
        btree           xfs_da_node_entry
ends


; packet extent
struct xfs_bmbt_rec
        l0              dq ?
        l1              dq ?
ends


; unpacked extent
struct xfs_bmbt_irec
        br_startoff     dq ?            ; starting file offset
        br_startblock   dq ?            ; starting block number
        br_blockcount   dd ?            ; number of blocks
        br_state        dd ?            ; extent state
ends


; bmap root header, on-disk form only
struct xfs_bmdr_block
        bb_level        dw ?            ; 0 is a leaf
        bb_numrecs      dw ?            ; current number of data records
ends


; key structure for non-leaf levels of the tree
struct xfs_bmbt_key
        br_startoff     dq ?            ; starting file offset
ends


sizeof.xfs_bmbt_ptr = 8                 ; workaround
sizeof.xfs_bmdr_ptr = 8                 ; workaround


; long form header: bmap btrees
; xfs_btree_lblock is xfs_bmbt_block (xfs_btree.h)
struct xfs_bmbt_block
        bb_magic        dd ?            ; magic number for block type
        bb_level        dw ?            ; 0 is a leaf
        bb_numrecs      dw ?            ; current number of data records
        bb_leftsib      dq ?            ; left sibling block or NULLDFSBNO
        bb_rightsib     dq ?            ; right sibling block or NULLDFSBNO
ends


; high level inode structure
struct xfs_inode
        di_core                 xfs_dinode_core ; main info, aka core
        di_next_unlinked        dd ?            ; unlinked but still used inode (if any, XFS_NULL otherwise)
        di_u                    db ?            ; data fork inode part
;        di_a                    db ?           ; data attribute
ends


; internal data for every XFS partition
; this _is_ XFS partition structure
; most fields are unpacked or bswap'ed values from the superblock, so see xfs_sb structure above
struct XFS PARTITION
        Lock                    MUTEX ?         ; access mutex
        blocksize               dd ?
        sectsize                dd ?
        dirblocksize            dd ?
        rootino                 dq ?
        cur_block               dd ?
        cur_inode               dd ?
        cur_sect                dd ?
        cur_dirblock            dd ?
        tmp_inode               dd ?
        versionnum              dd ?
        features2               dd ?
        inodesize               dd ?
        inopblock               dd ?
        blocklog                dd ?
        sectlog                 dd ?
        inodelog                dd ?
        inopblog                dd ?
        agblklog                dd ?
        blockmsectlog           dd ?
        inodetoblocklog         dd ?
        dirblklog               dd ?
        sectpblock              dd ?
        agblocks                dd ?
        ; helpers, temporary vars, etc
        agblockmask             dq ?
        extent                  xfs_bmbt_irec
        left_extents            dd ?
        left_leaves             dd ?
        bytes_to_read           dd ?
        bytes_read              dd ?
        entries_read            dd ?
        file_offset             dq ?
        max_dirblockaddr        dd ?
        next_block_num          dq ?
        dir2_leaf_offset_blocks dd ?
        dir2_free_offset_blocks dd ?
        cur_inode_save          dd ?
        bytes_left_in_file      dq ?
        ro_nextents             dd ?
        bb_ptrs                 dd ?
        maxnumrecs              dd ?
        buffer_pos              dd ?
        eof                     dd ?
ends
