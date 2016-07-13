;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2016. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; EXT external functions
;   in:
; ebx -> parameter structure of sysfunc 70
; ebp -> EXTFS structure
; [esi]+[[esp+4]] = name
;   out:
; eax, ebx = return values for sysfunc 70
iglobal
align 4
ext_user_functions:
        dd      ext_free
        dd      (ext_user_functions_end - ext_user_functions - 4) / 4
        dd      ext_ReadFile
        dd      ext_ReadFolder
        dd      ext_CreateFile
        dd      ext_WriteFile
        dd      ext_SetFileEnd
        dd      ext_GetFileInfo
        dd      ext_SetFileInfo
        dd      0
        dd      ext_Delete
        dd      ext_CreateFolder
ext_user_functions_end:
endg

struct DIRENTRY
inodeNumber     dd  ?
entryLength     dw  ?
nameLength      db  ?
fileType        db  ?
name            db  ?   ; rb [nameLength]
ends

struct INODE
accessMode      dw  ?
UID             dw  ?
fileSize        dd  ?
accessedTime    dd  ?
inodeModified   dd  ?
dataModified    dd  ?
deletedTime     dd  ?
GID             dw  ?
linksCount      dw  ?
sectorsUsed     dd  ?
featureFlags    dd  ?
reserved        dd  ?
blockNumbers    rd  12
addressBlock    dd  ?
doubleAddress   dd  ?
tripleAddress   dd  ?
generation      dd  ?
ACL             dd  ?
fileSizeHigh    dd  ?
ends

struct BGDESCR  ; block group descriptor
blockBitmap         dd  ?
inodeBitmap         dd  ?
inodeTable          dd  ?
blocksFree          dw  ?
inodesFree          dw  ?
directoriesCount    dw  ?
reserved            rb  14
ends

struct SUPERBLOCK
inodesTotal         dd  ?
blocksTotal         dd  ?
blocksReserved      dd  ?
blocksFree          dd  ?
inodesFree          dd  ?
firstGroupBlock     dd  ?
sectorsPerBlockLog  dd  ?   ; shift for 1024
fragmentSizeLog     dd  ?
blocksPerGroup      dd  ?
fragmentsPerGroup   dd  ?
inodesPerGroup      dd  ?
lastMountTime       dd  ?
lastWriteTime       dd  ?
mountCount          dw  ?
mountMax            dw  ?
magic               dw  ?
state               dw  ?
errorHandling       dw  ?
additionalVersion   dw  ?
lastCheck           dd  ?
checkInterval       dd  ?
creatorOS           dd  ?
dynamicVersionFlag  dd  ?
reservedUID         dw  ?
reservedGID         dw  ?
firstInode          dd  ?
inodeSize           dw  ?
thisBlockGroup      dw  ?
compatibleFlags     dd  ?
incompatibleFlags   dd  ?
RO_compatibleFlags  dd  ?
ends

; ext4 extent tree
struct NODEHEADER       ; tree node header
magic           dw  ?   ; 0xF30A
entriesFolow    dw  ?
entriesMax      dw  ?
currentDepth    dw  ?
generation      dd  ?
ends

struct INDEX    ; root/branch
fileBlock       dd  ?
nodeBlock       dd  ?
nodeBlockHigh   dw  ?
reserved        dw  ?
ends

struct EXTENT   ; leaf
fileBlock       dd  ?
blocksCount     dw  ?
fsBlockHigh     dw  ?
fsBlock         dd  ?
ends

ROOT_INODE = 2
PERMISSIONS = 110110110b
EXTENTS_USED = 80000h
TYPE_MASK = 0F000h
FLAG_FILE = 8000h
DIRECTORY = 4000h
DIR_FLAG_FILE = 1
DIR_DIRECTORY = 2
KOS_HIDDEN = 2
KOS_DIRECTORY = 10h
READ_ONLY = 1

; Implemented "incompatible" features:
; 2 = have file type in directory entry
; 40h = extents
; 200h = flexible block groups
INCOMPATIBLE_SUPPORT = 242h
; Read only support for "incompatible" features:
INCOMPATIBLE_READ_SUPPORT = 240h

; Implemented "read-only" features:
; 1 = sparse superblock
; 2 = 64-bit file size
READ_ONLY_SUPPORT = 3

struct EXTFS PARTITION
Lock                MUTEX
mountType           dd  ?
sectorsPerBlockLog  dd  ?   ; shift for 512
bytesPerBlock       dd  ?
sectorsPerBlock     dd  ?
dwordsPerBlock      dd  ?
dwordsPerBranch     dd  ?   ; dwordsPerBlock ^ 2
mainBlockBuffer     dd  ?
tempBlockBuffer     dd  ?
align 512
superblock          SUPERBLOCK
align 1024
rootInodeBuffer     INODE
align 1024
mainInodeBuffer     INODE
align 1024
tempInodeBuffer     INODE
ends

; mount if it's a valid EXT partition
ext2_create_partition:
;   in:
; ebp -> PARTITION structure
; ebx -> boot sector
; ebx+512 -> buffer
;   out:
; eax -> EXTFS structure, 0 = not EXT
        push    ebx
        cmp     dword [esi+DISK.MediaInfo.SectorSize], 512
        jnz     .fail
        mov     eax, 2
        add     ebx, 512
        call    fs_read32_sys
        test    eax, eax
        jnz     .fail
        cmp     [ebx+SUPERBLOCK.magic], 0xEF53
        jne     .fail
        cmp     [ebx+SUPERBLOCK.state], 1
        jne     .fail
        test    [ebx+SUPERBLOCK.incompatibleFlags], not INCOMPATIBLE_SUPPORT
        jnz     .fail
        cmp     [ebx+SUPERBLOCK.sectorsPerBlockLog], 6  ; 64KB
        ja      .fail
        cmp     [ebx+SUPERBLOCK.inodeSize], 1024
        ja      .fail
        cmp     [ebx+SUPERBLOCK.blocksPerGroup], 0
        je      .fail
        cmp     [ebx+SUPERBLOCK.inodesPerGroup], 0
        je      .fail
        stdcall kernel_alloc, 1000h
        test    eax, eax
        jz      .fail
        mov     ecx, dword [ebp+PARTITION.FirstSector]
        mov     dword [eax+EXTFS.FirstSector], ecx
        mov     ecx, dword [ebp+PARTITION.FirstSector+4]
        mov     dword [eax+EXTFS.FirstSector+4], ecx
        mov     ecx, dword [ebp+PARTITION.Length]
        mov     dword [eax+EXTFS.Length], ecx
        mov     ecx, dword [ebp+PARTITION.Length+4]
        mov     dword [eax+EXTFS.Length+4], ecx
        mov     ecx, [ebp+PARTITION.Disk]
        mov     [eax+EXTFS.Disk], ecx
        mov     [eax+EXTFS.FSUserFunctions], ext_user_functions

        push    ebp esi edi
        mov     ebp, eax
        lea     ecx, [eax+EXTFS.Lock]
        call    mutex_init
        mov     esi, ebx
        lea     edi, [ebp+EXTFS.superblock]
        mov     ecx, 512/4
        rep movsd   ; copy superblock
        mov     ecx, [ebx+SUPERBLOCK.sectorsPerBlockLog]
        inc     ecx
        mov     [ebp+EXTFS.sectorsPerBlockLog], ecx
        mov     eax, 1
        shl     eax, cl
        mov     [ebp+EXTFS.sectorsPerBlock], eax
        shl     eax, 9
        mov     [ebp+EXTFS.bytesPerBlock], eax
        shl     eax, 1
        push    eax
        shr     eax, 3
        mov     [ebp+EXTFS.dwordsPerBlock], eax
        mul     eax
        mov     [ebp+EXTFS.dwordsPerBranch], eax
        call    kernel_alloc
        test    eax, eax
        jz      .error
        mov     [ebp+EXTFS.mainBlockBuffer], eax
        add     eax, [ebp+EXTFS.bytesPerBlock]
        mov     [ebp+EXTFS.tempBlockBuffer], eax
        mov     [ebp+EXTFS.mountType], 0
        test    [ebx+SUPERBLOCK.RO_compatibleFlags], not READ_ONLY_SUPPORT
        jnz     .read_only
        test    [ebx+SUPERBLOCK.incompatibleFlags], INCOMPATIBLE_READ_SUPPORT
        jz      @f
.read_only:
        or      [ebp+EXTFS.mountType], READ_ONLY
@@: ; read root inode
        lea     ebx, [ebp+EXTFS.rootInodeBuffer]
        mov     eax, ROOT_INODE
        call    readInode
        test    eax, eax
        jnz     @f
        mov     eax, ebp
        pop     edi esi ebp ebx
        ret

@@:
        stdcall kernel_free, [ebp+EXTFS.mainBlockBuffer]
.error:
        stdcall kernel_free, ebp
        pop     edi esi ebp
.fail:
        pop     ebx
        xor     eax, eax
        ret

; unmount EXT partition
ext_free:
; in: eax -> EXTFS structure
        push    eax
        stdcall kernel_free, [eax+EXTFS.mainBlockBuffer]
        call    kernel_free
        ret

extfsWriteBlock:
        push    fs_write64_sys
        jmp     @f
;   in:
; eax = block number
; ebx -> buffer
extfsReadBlock:
        push    fs_read64_sys
@@:
        push    ecx edx
        mov     ecx, [ebp+EXTFS.sectorsPerBlock]
        mul     ecx
        call    dword[esp+8]
        pop     edx ecx
        add     esp, 4
        test    eax, eax
        jz      @f
        movi    eax, ERROR_DEVICE
        stc
@@:
        ret

extfsReadDescriptor:
; in: eax = block group number
;   out:
; [ebp+EXTFS.tempBlockBuffer] -> relevant block
; eax -> block group descriptor, 0 = error
        push    edx ebx
        shl     eax, 5
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        add     eax, [ebp+EXTFS.superblock.firstGroupBlock]
        inc     eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .fail
        mov     eax, ebx
        add     eax, edx
@@:
        pop     ebx edx
        ret

.fail:
        xor     eax, eax
        stc
        jmp     @b

extfsWriteDescriptor:
;   in:
; eax = block group number
; [ebp+EXTFS.tempBlockBuffer] -> relevant block
        push    edx ebx
        shl     eax, 5
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        add     eax, [ebp+EXTFS.superblock.firstGroupBlock]
        inc     eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsWriteBlock
        pop     ebx edx
        ret

extfsResourceFree:
;   in:
; ecx=0 -> block, ecx=1 -> inode
; eax = block/inode number
        push    ebx edx
        sub     eax, [ebp+EXTFS.superblock.firstGroupBlock]
        xor     edx, edx
        div     [ebp+EXTFS.superblock.blocksPerGroup]
        push    eax edx
        call    extfsReadDescriptor
        jc      .fail
        inc     [eax+BGDESCR.blocksFree+ecx*2]
        mov     eax, [eax+BGDESCR.blockBitmap+ecx*4]
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        mov     edx, eax
        call    extfsReadBlock
        jc      .fail
        pop     eax
        push    edx
        mov     edx, eax
        and     edx, 31
        shr     eax, 5
        shl     eax, 2
        add     eax, [ebp+EXTFS.mainBlockBuffer]
        btr     [eax], edx
        pop     eax
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsWriteBlock
        jc      @f
        inc     [ebp+EXTFS.superblock.blocksFree+ecx*4]
        pop     eax
        call    extfsWriteDescriptor
.ret:
        pop     edx ebx
        ret

.fail:
        pop     eax
@@:
        pop     eax
        movi    eax, ERROR_DEVICE
        jmp     .ret

freeDoublyIndirectBlock:
; in: eax = doubly-indirect block number
; out: eax=1 -> finished
        test    eax, eax
        jz      .complete
        push    eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        pop     eax
        jc      .ret
        xor     ecx, ecx
        call    extfsResourceFree
        mov     edx, ebx
        add     edx, [ebp+EXTFS.bytesPerBlock]
@@:
        mov     eax, [ebx]
        test    eax, eax
        jz      .complete
        call    extfsResourceFree
        add     ebx, 4
        cmp     ebx, edx
        jb      @b
.ret:
        xor     eax, eax
        ret

.complete:
        inc     eax
        ret

inodeBlockAlloc:
; in: esi -> inode
; out: ebx = block number
; TODO: fix to have correct preference.
        mov     eax, ROOT_INODE
        call    extfsBlockAlloc
        jc      @f
        mov     eax, [ebp+EXTFS.sectorsPerBlock]
        add     [esi+INODE.sectorsUsed], eax
        xor     eax, eax
@@:
        ret

extfsBlockAlloc:    ; also erases
; in: eax = inode number
; out: ebx = block number
        xor     ebx, ebx
        call    extfsResourceAlloc
        jc      @f
        push    ebx ecx edi
        mov     ecx, [ebp+EXTFS.dwordsPerBlock]
        mov     edi, [ebp+EXTFS.tempBlockBuffer]
        mov     ebx, edi
        xor     eax, eax
        rep stosd
        pop     edi ecx
        mov     eax, [esp]
        call    extfsWriteBlock
        pop     ebx
@@:
        ret

extfsResourceAlloc:
;   in:
; eax = inode number
; ebx=0 -> block, ebx=1 -> inode
;   out:
; ebx = block/inode number
        push    ecx edx esi edi
        dec     eax
        xor     edx, edx
        div     [ebp+EXTFS.superblock.inodesPerGroup]
        push    eax eax
        mov     esi, .forward   ; search forward, then backward
.test_block_group:
        call    extfsReadDescriptor
        jc      .fail
        dec     [eax+BGDESCR.blocksFree+ebx*2]
        mov     eax, [eax+BGDESCR.blockBitmap+ebx*4]
        push    ebx
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        mov     edx, eax
        mov     edi, ebx
        call    extfsReadBlock
        pop     ebx
        jc      .fail
        mov     ecx, [ebp+EXTFS.superblock.blocksPerGroup+ebx*8]
        or      eax, -1
        shr     ecx, 5
        jz      .next
        repz scasd
        jz      .next
        sub     edi, 4
        mov     eax, [edi]
        not     eax
        bsf     eax, eax
        bts     [edi], eax
        sub     edi, [ebp+EXTFS.mainBlockBuffer]
        shl     edi, 3
        add     eax, edi
        mov     ecx, eax
        mov     eax, edx
        push    ebx
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsWriteBlock
        pop     ebx
        jc      .fail
        mov     eax, [esp]
        mul     [ebp+EXTFS.superblock.blocksPerGroup+ebx*8]
        add     eax, ecx
        dec     [ebp+EXTFS.superblock.blocksFree+ebx*4]
        mov     ebx, eax
        pop     eax
        add     esp, 4
        call    extfsWriteDescriptor
@@:
        pop     edi esi edx ecx
        ret

.fail:
        pop     eax eax
        movi    eax, ERROR_DEVICE
        jmp     @b

.next:
        jmp     esi

.forward:
        inc     dword[esp]
        mov     eax, [esp]
        mul     [ebp+EXTFS.superblock.blocksPerGroup+ebx*8]
        neg     ebx
        cmp     eax, [ebp+EXTFS.superblock.blocksTotal+ebx*4]
        ja      @f
        neg     ebx
        mov     eax, [esp]
        jmp     .test_block_group

@@:
        neg     ebx
        mov     eax, [esp+4]
        mov     [esp], eax
        mov     esi, .backward
.backward:
        sub     dword[esp], 1
        jc      .fail
        mov     eax, [esp]
        jmp     .test_block_group

extfsGetFileBlock:
;   in:
; ecx = file block number
; esi -> inode
;   out:
; ecx = block number
        test    [esi+INODE.featureFlags], EXTENTS_USED
        jz      .listTreeSearch
        pushad
        add     esi, INODE.blockNumbers
.extentTreeSearch:
        cmp     word [esi+NODEHEADER.magic], 0xF30A
        jne     .fail
        movzx   ebx, [esi+NODEHEADER.entriesFolow]
        add     esi, sizeof.NODEHEADER
        cmp     word [esi-sizeof.NODEHEADER+NODEHEADER.currentDepth], 0
        je      .leaf_block
        test    ebx, ebx
        jz      .fail   ; empty
@@:
        cmp     ebx, 1
        je      .end_search_index
        cmp     ecx, [esi+INDEX.fileBlock]
        jb      .fail
        cmp     ecx, [esi+sizeof.INDEX+INDEX.fileBlock]
        jb      .end_search_index
        add     esi, sizeof.INDEX
        dec     ebx
        jmp     @b

.end_search_index:
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        mov     eax, [esi+INDEX.nodeBlock]
        call    extfsReadBlock
        jc      .fail
        mov     esi, ebx
        jmp     .extentTreeSearch

.leaf_block:
        test    ebx, ebx
        jz      .fail
        mov     edx, [esi+EXTENT.fileBlock]
        cmp     ecx, edx
        jb      .fail
        movzx   edi, [esi+EXTENT.blocksCount]
        add     edx, edi
        cmp     ecx, edx
        jb      .end_search_extent
        add     esi, sizeof.EXTENT
        dec     ebx
        jmp     .leaf_block

.end_search_extent:
        sub     ecx, [esi+EXTENT.fileBlock]
        add     ecx, [esi+EXTENT.fsBlock]
        mov     PUSHAD_ECX, ecx
        popad
        xor     eax, eax
        ret

.fail:
        popad
        movi    eax, ERROR_FS_FAIL
        stc
        ret

.get_indirect_block:
        push    edx ebx
        mov     eax, [esi+INODE.addressBlock]
        test    eax, eax
        jz      .fail3
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      @f
        mov     ecx, [ebx+ecx*4]
@@:
        pop     ebx edx
        ret

.get_direct_block:
        mov     ecx, [esi+INODE.blockNumbers+ecx*4]
        xor     eax, eax
        ret

.listTreeSearch:
        cmp     ecx, 12
        jb      .get_direct_block
        sub     ecx, 12
        cmp     ecx, [ebp+EXTFS.dwordsPerBlock]
        jb      .get_indirect_block
        sub     ecx, [ebp+EXTFS.dwordsPerBlock]
        cmp     ecx, [ebp+EXTFS.dwordsPerBranch]
        jb      .get_double_indirect_block
; triply-indirect blocks
        sub     ecx, [ebp+EXTFS.dwordsPerBranch]
        push    edx ebx
        mov     eax, [esi+INODE.tripleAddress]
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .fail2
        xor     edx, edx
        mov     eax, ecx
        div     [ebp+EXTFS.dwordsPerBranch]
; eax = number in triply-indirect block, edx = number in branch
        mov     eax, [ebx+eax*4]
        test    eax, eax
        jz      .fail3
        call    extfsReadBlock
        jc      .fail2
        mov     eax, edx
        jmp     @f

.fail3:
        pop     ebx edx
        movi    eax, ERROR_FS_FAIL
        stc
        ret

.get_double_indirect_block:
        push    edx ebx
        mov     eax, [esi+INODE.doubleAddress]
        test    eax, eax
        jz      .fail3
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .fail2
        mov     eax, ecx
@@:
        xor     edx, edx
        div     [ebp+EXTFS.dwordsPerBlock]
; eax = number in doubly-indirect block, edx = number in indirect block
        mov     eax, [ebx+eax*4]
        test    eax, eax
        jz      .fail3
        call    extfsReadBlock
        jc      .fail2
        mov     ecx, [ebx+edx*4]
.fail2:
        pop     ebx edx
        ret

extfsSetFileBlock:
;   in:
; ecx = file block number
; edi = block number
; esi -> inode
        push    ebx ecx edx
        cmp     ecx, 12
        jb      .direct_block
        sub     ecx, 12
        cmp     ecx, [ebp+EXTFS.dwordsPerBlock]
        jb      .indirect_block
        sub     ecx, [ebp+EXTFS.dwordsPerBlock]
        cmp     ecx, [ebp+EXTFS.dwordsPerBranch]
        jb      .double_indirect_block
; triple indirect blocks
        sub     ecx, [ebp+EXTFS.dwordsPerBranch]
        mov     eax, [esi+INODE.tripleAddress]
        test    eax, eax
        jnz     @f
        call    inodeBlockAlloc
        jc      .ret
        mov     [esi+INODE.tripleAddress], ebx
        mov     eax, ebx
@@:
        push    eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .fail_alloc_4
        xor     edx, edx
        mov     eax, ecx
        div     [ebp+EXTFS.dwordsPerBranch]
; eax = number in triply-indirect block, edx = number in branch
        lea     ecx, [ebx+eax*4]
        mov     eax, [ebx+eax*4]
        test    eax, eax
        jnz     @f
        call    inodeBlockAlloc
        jc      .fail_alloc_4
        mov     [ecx], ebx
        mov     eax, [esp]
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsWriteBlock
        jc      .fail_alloc_4
        mov     eax, [ecx]
@@:
        mov     [esp], eax
        call    extfsReadBlock
        jc      .fail_alloc_4
        mov     eax, edx
        jmp     @f

.double_indirect_block:
        mov     eax, [esi+INODE.doubleAddress]
        test    eax, eax
        jnz     .double_indirect_present
        call    inodeBlockAlloc
        jc      .ret
        mov     [esi+INODE.doubleAddress], ebx
        mov     eax, ebx
.double_indirect_present:
        push    eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .fail_alloc_4
        mov     eax, ecx
@@:
        xor     edx, edx
        div     [ebp+EXTFS.dwordsPerBlock]
; eax = number in doubly-indirect block, edx = number in indirect block
        lea     ecx, [ebx+edx*4]
        push    ecx
        lea     ecx, [ebx+eax*4]
        cmp     dword[ecx], 0
        jne     @f
        call    inodeBlockAlloc
        jc      .fail_alloc_8
        mov     [ecx], ebx
        mov     eax, [esp+4]
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsWriteBlock
        jc      .fail_alloc_8
@@:
        mov     eax, [ecx]
        push    eax
        call    extfsReadBlock
        jc      .fail_alloc_12
        pop     eax ecx edx
        mov     [ecx], edi
        call    extfsWriteBlock
        jmp     .ret

.indirect_block:
        mov     eax, [esi+INODE.addressBlock]
        test    eax, eax
        jnz     @f
        call    inodeBlockAlloc
        jc      .ret
        mov     [esi+INODE.addressBlock], ebx
        mov     eax, ebx
@@:
        push    eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .fail_alloc_4
        mov     [ebx+ecx*4], edi
        pop     eax
        call    extfsWriteBlock
        jmp     .ret

.direct_block:
        mov     [esi+INODE.blockNumbers+ecx*4], edi
        xor     eax, eax
.ret:
        pop     edx ecx ebx
        ret

.fail_alloc_12:
        pop     ebx
.fail_alloc_8:
        pop     ebx
.fail_alloc_4:
        pop     ebx
        jmp     .ret

extfsEraseFileBlock:    ; also allocates
;   in:
; edx = inode number
; eax = file block number
; [ebp+EXTFS.tempInodeBuffer] = inode
        push    ebx ecx edx edi esi
        mov     edi, eax
        mov     ecx, eax
        lea     esi, [ebp+EXTFS.tempInodeBuffer]
        call    extfsGetFileBlock
        jc      @f
        test    ecx, ecx
        jz      .allocate
        mov     edx, ecx
        mov     ecx, [ebp+EXTFS.bytesPerBlock]
        mov     edi, [ebp+EXTFS.tempBlockBuffer]
        xor     eax, eax
        rep stosb
        mov     eax, edx
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsWriteBlock
        jmp     @f

.allocate:
        mov     eax, edx
        call    extfsBlockAlloc
        jc      @f
        mov     ecx, edi
        mov     edi, ebx
        lea     esi, [ebp+EXTFS.tempInodeBuffer]
        call    extfsSetFileBlock
        jc      @f
        mov     eax, [ebp+EXTFS.sectorsPerBlock]
        add     [esi+INODE.sectorsUsed], eax
        xor     eax, eax
@@:
        pop     esi edi edx ecx ebx
        ret

extfsFreeFileBlock:
;   in:
; eax = file block number
; [ebp+EXTFS.tempInodeBuffer] = inode
        push    ebx ecx edi esi
        mov     edi, eax
        mov     ecx, eax
        lea     esi, [ebp+EXTFS.tempInodeBuffer]
        call    extfsGetFileBlock
        jc      @f
        test    ecx, ecx
        jz      @f
        mov     eax, ecx
        xor     ecx, ecx
        call    extfsResourceFree
        mov     ecx, edi
        xor     edi, edi
        lea     esi, [ebp+EXTFS.tempInodeBuffer]
        call    extfsSetFileBlock
        mov     eax, [ebp+EXTFS.sectorsPerBlock]
        sub     [esi+INODE.sectorsUsed], eax
        xor     eax, eax
@@:
        pop     esi edi ecx ebx
        ret

extfsReadFileBlock:
;   in:
; eax = file block number
; [ebp+EXTFS.tempInodeBuffer] = inode
;   out:
; [ebp+EXTFS.mainBlockBuffer] -> block
        push    ebx ecx edx esi
        mov     ecx, eax
        lea     esi, [ebp+EXTFS.tempInodeBuffer]
        call    extfsGetFileBlock
        jc      .ret
        test    ecx, ecx
        jz      @f
        mov     eax, ecx
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsReadBlock
.ret:
        pop     esi edx ecx ebx
        ret

@@:
        movi    eax, ERROR_FS_FAIL
        stc
        jmp     .ret

extfsWriteFileBlock:
;   in:
; eax = file block number
; [ebp+EXTFS.tempInodeBuffer] = inode
; [ebp+EXTFS.mainBlockBuffer] -> block to write
        push    ebx ecx edx esi
        mov     ecx, eax
        lea     esi, [ebp+EXTFS.tempInodeBuffer]
        call    extfsGetFileBlock
        jc      @f
        test    ecx, ecx
        jz      @b
        mov     eax, ecx
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsWriteBlock
@@:
        pop     esi edx ecx ebx
        ret

getInodeLocation:
; in: eax = inode number
;   out:
; ebx = inode sector
; edx = offset in sector
        dec     eax
        xor     edx, edx
        div     [ebp+EXTFS.superblock.inodesPerGroup]
        mov     ecx, edx
        shl     eax, 5
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        add     eax, [ebp+EXTFS.superblock.firstGroupBlock]
        inc     eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      @f
        add     ebx, edx
        mov     ebx, [ebx+BGDESCR.inodeTable]
        mov     eax, ecx
        mov     ecx, [ebp+EXTFS.sectorsPerBlockLog]
        shl     ebx, cl
        mul     [ebp+EXTFS.superblock.inodeSize]
        mov     edx, eax
        shr     eax, 9
        and     edx, 511
        add     ebx, eax
        xor     eax, eax
@@:
        ret

writeInode:
;   in:
; eax = inode number
; ebx -> inode data
        push    edx edi esi ecx ebx eax
        mov     edi, ebx
        call    fsGetTime
        add     eax, 978307200
        mov     [edi+INODE.inodeModified], eax
        pop     eax
        call    getInodeLocation
        jc      .ret
        mov     eax, ebx
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        mov     ecx, eax
        call    fs_read32_sys
        test    eax, eax
        jnz     @f
        mov     eax, ecx
        mov     esi, edi
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        mov     edi, edx
        add     edi, ebx
        rep movsb
        call    fs_write32_sys
.ret:
        pop     ebx ecx esi edi edx
        ret

@@:
        movi    eax, ERROR_DEVICE
        stc
        jmp     .ret

readInode:
;   in:
; eax = inode number
; ebx -> inode buffer
        push    edx edi esi ecx ebx
        mov     edi, ebx
        call    getInodeLocation
        jc      @f
        mov     eax, ebx
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    fs_read32_sys
        test    eax, eax
        jnz     @b
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        mov     esi, edx
        add     esi, ebx
        rep movsb
        xor     eax, eax
@@:
        pop     ebx ecx esi edi edx
        ret

extfsExtendFile:
;   in:
; eax = inode number
; ecx = new size
        push    ebx ecx edx esi edi eax
        lea     ebx, [ebp+EXTFS.tempInodeBuffer]
        call    readInode
        jc      .ret
        cmp     [ebx+INODE.fileSize], ecx
        jnc     .ret
        mov     eax, [ebx+INODE.fileSize]
        push    eax
        sub     ecx, eax
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        test    edx, edx
        jz      .start_aligned
        mov     esi, [ebp+EXTFS.bytesPerBlock]
        sub     esi, edx
        cmp     esi, ecx
        jbe     @f
        mov     esi, ecx
@@: ; clear esi trailing bytes in block number eax
        push    eax
        call    extfsReadFileBlock
        pop     edi
        jc      .error_inode_size
        push    edi ecx
        xor     eax, eax
        mov     ecx, esi
        mov     edi, ebx
        add     edi, edx
        rep stosb
        pop     ecx eax
        call    extfsWriteFileBlock
        jc      .error_inode_size
        add     [esp], esi
        sub     ecx, esi
        jz      .write_inode
.start_aligned:
        cmp     ecx, [ebp+EXTFS.bytesPerBlock]
        jb      @f
        mov     eax, [esp]
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        mov     edx, [esp+4]
        call    extfsEraseFileBlock
        jc      .error_inode_size
        mov     eax, [ebp+EXTFS.bytesPerBlock]
        sub     ecx, eax
        add     [esp], eax
        jmp     .start_aligned

@@: ; handle the remaining bytes
        test    ecx, ecx
        jz      .write_inode
        mov     eax, [esp]
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        mov     edx, [esp+4]
        call    extfsEraseFileBlock
        jc      .error_inode_size
        add     [esp], ecx
.write_inode:
        xor     eax, eax
.error_inode_size:
        lea     ebx, [ebp+EXTFS.tempInodeBuffer]
        pop     [ebx+INODE.fileSize]
        push    eax
        mov     eax, [esp+4]
        call    writeInode
        pop     ebx
        jc      .ret
        xchg    eax, ebx
        cmp     ebx, eax    ; set CF
.ret:
        pop     edi edi esi edx ecx ebx
        ret

extfsTruncateFile:
;   in:
; eax = inode number
; ecx = new size
        push    ebx ecx edx esi edi eax
        lea     ebx, [ebp+EXTFS.tempInodeBuffer]
        call    readInode
        jc      .ret
        cmp     ecx, [ebx+INODE.fileSize]
        jnc     .ret
        mov     eax, [ebx+INODE.fileSize]
        push    eax
        sub     ecx, eax
        not     ecx
        inc     ecx
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        test    edx, edx
        jz      .start_aligned
        mov     esi, edx
        cmp     esi, ecx
        jbe     @f
; if the size to truncate is smaller than the unaligned bytes
; we're going to clear neccessary bytes from the EOF
        push    eax
        call    extfsReadFileBlock
        pop     edi
        jc      .error_inode_size
        push    edi ecx
        mov     edi, [ebp+EXTFS.mainBlockBuffer]
        sub     edx, ecx
        add     edi, edx
        xor     eax, eax
        rep stosb
        pop     ecx eax
        call    extfsWriteFileBlock
        jc      .error_inode_size
        sub     [esp], ecx
        jmp     .write_inode

@@:
        call    extfsFreeFileBlock
        sub     [esp], esi
        sub     ecx, esi
        jz      .write_inode
.start_aligned:
        cmp     ecx, [ebp+EXTFS.bytesPerBlock]
        jb      @f
        mov     eax, [esp]
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        dec     eax
        call    extfsFreeFileBlock
        mov     eax, [ebp+EXTFS.bytesPerBlock]
        sub     ecx, eax
        sub     [esp], eax
        jmp     .start_aligned

@@: ; handle the remaining bytes
        test    ecx, ecx
        jz      .write_inode
        mov     eax, [esp]
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        dec     eax
        push    eax
        call    extfsReadFileBlock
        pop     edi
        jc      .error_inode_size
        push    edi ecx
        mov     edi, [ebp+EXTFS.mainBlockBuffer]
        mov     edx, [ebp+EXTFS.bytesPerBlock]
        sub     edx, ecx
        add     edi, edx
        xor     eax, eax
        rep stosb
        pop     ecx eax
        call    extfsWriteFileBlock
        jc      .error_inode_size
        sub     [esp], ecx
.write_inode:
        xor     eax, eax
.error_inode_size:
        lea     ebx, [ebp+EXTFS.tempInodeBuffer]
        pop     [ebx+INODE.fileSize]
        push    eax
        mov     eax, [esp+4]
        call    writeInode
        pop     ebx
        jc      .ret
        xchg    eax, ebx
        cmp     ebx, eax    ; set CF
.ret:
        pop     edi edi esi edx ecx ebx
        ret

linkInode:
;   in:
; eax = inode on which to link
; ebx = inode to link
; esi -> name
;  dl = file type
        push    esi edi ebx ecx eax edx
        call    strlen
        add     ecx, 8  ; directory entry size
        push    esi ebx ecx
        xor     ecx, ecx
        lea     esi, [ebp+EXTFS.tempInodeBuffer]
        mov     ebx, esi
        call    readInode
        jc      .error_inode_read
        mov     ecx, [ebp+EXTFS.sectorsPerBlockLog]
        mov     eax, [esi+INODE.sectorsUsed]
        shr     eax, cl
        xor     ecx, ecx
        push    eax     ; maximum file block number
        push    ecx     ; current file block number
.searchBlock:
        call    extfsGetFileBlock
        jc      .error_get_inode_block
        test    ecx, ecx
        jz      .alloc_block
        push    ecx
        mov     eax, ecx
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .error_block_read
        mov     ecx, [esp+8]
        mov     edi, [ebp+EXTFS.tempBlockBuffer]
        mov     edx, edi
        add     edx, [ebp+EXTFS.bytesPerBlock]
.searchSpace:
        movzx   eax, [edi+DIRENTRY.entryLength]
        test    eax, eax
        jz      .zeroLength
        cmp     [edi+DIRENTRY.inodeNumber], 0
        je      .unusedEntry
        movzx   ebx, [edi+DIRENTRY.nameLength]
        add     ebx, 8+3
        and     ebx, -4
        sub     eax, ebx
        add     edi, ebx
        cmp     eax, ecx
        jb      .nextEntry
        sub     edi, ebx
        mov     [edi+DIRENTRY.entryLength], bx
        add     edi, ebx
        mov     [edi+DIRENTRY.entryLength], ax
        jmp     .found

.unusedEntry:
        cmp     eax, ecx
        jge     .found
.nextEntry:
        add     edi, eax
        cmp     edi, edx
        jb      .searchSpace
        jmp     .nextBlock

.zeroLength:
        mov     [edi+DIRENTRY.entryLength], cx
        mov     eax, edx
        sub     eax, edi
        cmp     eax, ecx
        jge     .found
        mov     [edi+DIRENTRY.inodeNumber], 0
        mov     [edi+DIRENTRY.entryLength], ax
; this block wasn't linking to the next one, so write it, and use the next block
        pop     eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsWriteBlock
        jc      .error_get_inode_block
        inc     dword[esp]
        mov     ecx, [esp]
        call    extfsGetFileBlock
        jc      .error_get_inode_block
        test    ecx, ecx
        jz      .alloc_block
        push    ecx
        jmp     .prepare_block

.nextBlock:
        add     esp, 4
        inc     dword[esp]
        mov     ecx, [esp]
        cmp     ecx, [esp+4]
        jbe     .searchBlock
.alloc_block:
        mov     eax, [esp+12]
        call    extfsBlockAlloc
        jc      .error_get_inode_block
        mov     ecx, [esp]
        mov     edi, ebx
        call    extfsSetFileBlock
        jc      .error_get_inode_block
        mov     eax, [ebp+EXTFS.bytesPerBlock]
        add     [esi+INODE.fileSize], eax
        mov     eax, [ebp+EXTFS.sectorsPerBlock]
        add     [esi+INODE.sectorsUsed], eax
        mov     eax, [esp+24]
        mov     ebx, esi
        call    writeInode
        jc      .error_get_inode_block
        push    edi     ; save the block we just allocated
.prepare_block:
        mov     eax, [esp]
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .error_block_read
        mov     edi, ebx
        mov     eax, [ebp+EXTFS.bytesPerBlock]
        mov     [edi+DIRENTRY.entryLength], ax
.found:
        pop     edx ecx ecx ecx ebx esi
        push    ebx
        mov     [edi], ebx  ; save inode
        mov     eax, [esp+4]
        cmp     [ebp+EXTFS.superblock.dynamicVersionFlag], 0
        je      .name
        mov     [edi+DIRENTRY.fileType], al
.name:
        sub     ecx, 8
        mov     [edi+DIRENTRY.nameLength], cl
        add     edi, 8
        rep movsb
        mov     eax, edx
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsWriteBlock
        jc      .error_block_write
        mov     eax, [esp]
        lea     ebx, [ebp+EXTFS.tempInodeBuffer]
        call    readInode
        jc      .error_block_write
        pop     eax
        inc     [ebx+INODE.linksCount]
        call    writeInode
        jc      @f
        xor     eax, eax
@@:
        pop     edx ecx ecx ebx edi esi
        ret

.error_block_read:
        pop     ebx
.error_get_inode_block:
        pop     ebx ebx
.error_inode_read:
        pop     ebx ebx
.error_block_write:
        pop     ebx
        jmp     @b

unlinkInode:
;   in:
; eax = inode from which to unlink
; ebx = inode to unlink
;   out:
; eax = current number of links to inode, -1 = error
        push    edx esi edi ebx
        lea     ebx, [ebp+EXTFS.tempInodeBuffer]
        call    readInode
        jc      .fail
        push    eax
        lea     esi, [ebp+EXTFS.tempInodeBuffer]
.loop:
        mov     ecx, [esp]
        call    extfsGetFileBlock
        jc      .fail_loop
        test    ecx, ecx
        jz      .fail_loop
        mov     eax, ecx
        mov     edi, ecx
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .fail_loop
.first_dir_entry:   ; edi -> block
        mov     eax, [esp+4]
        cmp     [ebx+DIRENTRY.inodeNumber], eax
        jne     @f
        mov     [ebx+DIRENTRY.inodeNumber], 0
        mov     word [ebx+DIRENTRY.nameLength], 0   ; fileType = 0
        jmp     .write_block

@@:
        mov     edx, ebx
        add     edx, [ebp+EXTFS.bytesPerBlock]
        push    edx
        mov     edx, ebx
        movzx   ecx, [ebx+DIRENTRY.entryLength]
        add     ebx, ecx
.dir_entry:
        cmp     [ebx+DIRENTRY.inodeNumber], eax
        jne     @f
        mov     cx, [ebx+DIRENTRY.entryLength]
        add     [edx+DIRENTRY.entryLength], cx
        pop     eax
        jmp     .write_block

@@:
        mov     edx, ebx
        movzx   ecx, [ebx+DIRENTRY.entryLength]
        test    ecx, ecx
        jz      .fail_inode
        add     ebx, ecx
        cmp     ebx, [esp]
        jb      .dir_entry
        pop     ecx
        inc     dword[esp]
        jmp     .loop

.fail_inode:
        pop     eax
.fail_loop:
        pop     eax
.fail:
        or      eax, -1
        jmp     @f

.write_block:
        pop     eax
        mov     eax, edi
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsWriteBlock
        jc      .fail
        mov     eax, [esp]
        lea     ebx, [ebp+EXTFS.tempInodeBuffer]
        call    readInode
        jc      .fail
        dec     word [ebx+INODE.linksCount]
        mov     eax, [esp]
        call    writeInode
        jc      .fail
        movzx   eax, word [ebx+INODE.linksCount]
@@:
        pop     ebx edi esi edx
        ret

findInode_parent:
; in: esi -> path
;   out:
; edi -> file name
; esi = inode
        push    esi
        xor     edi, edi
.loop:
        cmp     byte [esi], '/'
        jne     @f
        mov     edi, esi
        inc     esi
        jmp     .loop

@@:
        inc     esi
        cmp     byte [esi-1], 0
        jne     .loop
        cmp     edi, 0
        jne     @f
; parent is root
        pop     edi
        dec     esi
        jmp     .get_inode

@@: ; parent is folder
        mov     byte [edi], 0
        inc     edi
        pop     esi
.get_inode:
        push    ebx edx
        stdcall findInode, 0
        pop     edx ebx
        ret

findInode:
; in: [esi]+[[esp+4]] = name
;   out:
; [ebp+EXTFS.mainInodeBuffer] = inode
; esi = inode number
;  dl = first byte of file/folder name
        lea     edx, [ebp+EXTFS.rootInodeBuffer]
        cmp     [edx+INODE.sectorsUsed], 0
        je      .not_found
        cmp     byte [esi], 0
        jne     .next_path_part
; root
        push    edi ecx
        lea     esi, [ebp+EXTFS.rootInodeBuffer]
        lea     edi, [ebp+EXTFS.mainInodeBuffer]
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        rep movsb
        pop     ecx edi
        xor     eax, eax
        xor     dl, dl
        mov     esi, ROOT_INODE
        ret     4

.next_path_part:
        push    [edx+INODE.sectorsUsed]
        xor     ecx, ecx
.folder_block_cycle:
        push    ecx
        xchg    esi, edx
        call    extfsGetFileBlock
        jc      .error_get_block
        xchg    esi, edx
        mov     eax, ecx
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsReadBlock
        jc      .error_get_block
        push    esi edx
        sub     esp, 256
        mov     edx, ebx
        add     edx, [ebp+EXTFS.bytesPerBlock]
.start_rec:
        cmp     [ebx+DIRENTRY.inodeNumber], 0
        jz      .next_rec
        mov     edi, esp
        push    esi
        movzx   ecx, [ebx+DIRENTRY.nameLength]
        lea     esi, [ebx+DIRENTRY.name]
        call    utf8_to_cp866
        mov     ecx, edi
        lea     edi, [esp+4]
        sub     ecx, edi    ; number of bytes in resulting string
        mov     esi, [esp]
@@: ; edi -> converted string in stack, ecx = size, esi -> original file path
        jecxz   .test_find
        dec     ecx
        lodsb
        call    char_toupper
        mov     ah, [edi]
        inc     edi
        xchg    al, ah
        call    char_toupper
        cmp     al, ah
        je      @b
@@: ; doesn't match
        pop     esi
.next_rec:
        movzx   eax, [ebx+DIRENTRY.entryLength]
        add     ebx, eax
        cmp     ebx, edx
        jb      .start_rec
        push    eax
        jmp     @f

.test_find:
        cmp     byte [esi], 0
        je      @f
        cmp     byte [esi], '/'
        jne     @b
        inc     esi
@@:
        add     esp, 256+4
        pop     edx edi ecx
; ebx -> matched directory entry, esi -> name without parent, or not changed
        cmp     edi, esi
        je      .next_folder_block
        cmp     byte [esi], 0
        jnz     @f
        cmp     dword[esp+8], 0
        je      .get_inode_ret
        mov     esi, [esp+8]
        mov     dword[esp+8], 0
@@:
        mov     eax, [ebx+DIRENTRY.inodeNumber]
        lea     ebx, [ebp+EXTFS.mainInodeBuffer]
        call    readInode
        jc      .error_get_inode
        movzx   eax, [ebx+INODE.accessMode]
        and     eax, TYPE_MASK
        cmp     eax, DIRECTORY
        jne     .not_found      ; path folder is a file
        pop     ecx
        mov     edx, ebx
        jmp     .next_path_part

.next_folder_block:
        pop     eax
        sub     eax, [ebp+EXTFS.sectorsPerBlock]
        jle     .not_found
        push    eax
        inc     ecx
        jmp     .folder_block_cycle

.get_inode_ret:
        pop     eax
        mov     dl, [ebx+DIRENTRY.name]
        mov     eax, [ebx+DIRENTRY.inodeNumber]
        lea     ebx, [ebp+EXTFS.mainInodeBuffer]
        mov     esi, eax
        call    readInode
        ret     4

.not_found:
        movi    eax, ERROR_FILE_NOT_FOUND
        stc
        ret     4

.error_get_block:
        pop     ebx
.error_get_inode:
        pop     ebx
        ret     4

writeSuperblock:
        push    ebx
        mov     eax, 2
        lea     ebx, [ebp+EXTFS.superblock]
        call    fs_write32_sys
        pop     ebx
        ret

extfsWritingInit:
        movi    eax, ERROR_ACCESS_DENIED
        cmp     byte [esi], 0
        jz      @f
        movi    eax, ERROR_UNSUPPORTED_FS
        test    [ebp+EXTFS.mountType], READ_ONLY
        jnz     @f
ext_lock:
        lea     ecx, [ebp+EXTFS.Lock]
        jmp     mutex_lock

@@:
        pop     ebx
        xor     ebx, ebx
        ret

ext_unlock:
        lea     ecx, [ebp+EXTFS.Lock]
        jmp     mutex_unlock

;----------------------------------------------------------------
ext_ReadFolder:
        call    ext_lock
        cmp     byte [esi], 0
        jz      .root_folder
        push    ebx
        stdcall findInode, [esp+4+4]
        pop     ebx
        jc      .error_ret
        lea     esi, [ebp+EXTFS.mainInodeBuffer]
        test    [esi+INODE.accessMode], DIRECTORY
        jz      .error_not_found
        jmp     @f

.root_folder:
        lea     esi, [ebp+EXTFS.rootInodeBuffer]
        lea     edi, [ebp+EXTFS.mainInodeBuffer]
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        shr     ecx, 2
        push    edi
        rep movsd
        pop     esi
@@:
        cmp     [esi+INODE.fileSize], 0
        je      .error_empty_dir
        mov     edx, [ebx+16]
        push    edx         ; [edi+28] result buffer
        push    0           ; [edi+24] end of the current block in folder
        pushd   [ebx+12]    ; [edi+20] files to read
        pushd   [ebx+4]     ; [edi+16] first wanted file
        pushd   [ebx+8]     ; [edi+12] flags
        push    0           ; [edi+8]  read files
        push    0           ; [edi+4]  files in folder
        push    0           ; [edi]    current block index
        mov     edi, esp    ; edi -> local variables
        add     edx, 32
        xor     ecx, ecx
        call    extfsGetFileBlock
        jc      .error_get_block
        mov     eax, ecx
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsReadBlock
        jc      .error_get_block
        mov     eax, ebx
        add     eax, [ebp+EXTFS.bytesPerBlock]
        mov     [edi+24], eax
        mov     ecx, [edi+16]
.find_wanted_start:
        jecxz   .find_wanted_end
.find_wanted_cycle:
        cmp     [ebx+DIRENTRY.inodeNumber], 0
        jz      @f
        inc     dword [edi+4]
        dec     ecx
@@:
        movzx   eax, [ebx+DIRENTRY.entryLength]
        cmp     eax, 12     ; minimum entry length
        jb      .error_bad_len
        test    eax, 3      ; length must be aligned
        jnz     .error_bad_len
        sub     [esi+INODE.fileSize], eax
        add     ebx, eax
        cmp     ebx, [edi+24]
        jb      .find_wanted_start
        push    .find_wanted_start
.end_block: ; read next block
        cmp     [esi+INODE.fileSize], 0
        jle     .end_dir
        inc     dword [edi]
        push    ecx
        mov     ecx, [edi]
        call    extfsGetFileBlock
        jc      .error_get_block
        mov     eax, ecx
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsReadBlock
        jc      .error_get_block
        pop     ecx
        mov     eax, ebx
        add     eax, [ebp+EXTFS.bytesPerBlock]
        mov     [edi+24], eax
        ret

.wanted_end:
        loop    .find_wanted_cycle
.find_wanted_end:
        mov     ecx, [edi+20]
.wanted_start:
        jecxz   .wanted_end
        cmp     [ebx+DIRENTRY.inodeNumber], 0
        jz      .empty_rec
        inc     dword [edi+8]
        inc     dword [edi+4]
        push    ebx edi ecx esi edx
        mov     edi, edx
        xor     eax, eax
        mov     ecx, 40 / 4
        rep stosd
        mov     eax, [ebx+DIRENTRY.inodeNumber]
        lea     ebx, [ebp+EXTFS.tempInodeBuffer]
        call    readInode
        jc      .error_read_subinode
        mov     esi, ebx
        lea     edi, [edx+8]
        mov     eax, [ebx+INODE.inodeModified]
        sub     eax, 978307200  ; 01.01.1970-01.01.2001 = (365*31+8)*24*60*60
        call    fsTime2bdfe

        mov     eax, [esi+INODE.accessedTime]
        sub     eax, 978307200
        call    fsTime2bdfe

        mov     eax, [esi+INODE.dataModified]
        sub     eax, 978307200
        call    fsTime2bdfe
        pop     edx
        or      dword [edx], KOS_DIRECTORY
        test    [esi+INODE.accessMode], DIRECTORY
        jnz     @f
        xor     dword [edx], KOS_DIRECTORY  ; mark as file
        mov     eax, [esi+INODE.fileSize]
        stosd
        mov     eax, [esi+INODE.fileSizeHigh]
        stosd
@@:
        mov     esi, [esp+12]
        movzx   ecx, [esi+DIRENTRY.nameLength]
        lea     edi, [edx+40]
        lea     esi, [esi+DIRENTRY.name]
        call    utf8_to_cp866
        and     byte [edi], 0
        pop     esi ecx edi ebx
        cmp     byte [edx+40], '.'
        jne     @f
        or      dword [edx], KOS_HIDDEN
@@:
        add     edx, 40+264 ; go to the next record
        dec     ecx
.empty_rec:
        movzx   eax, [ebx+DIRENTRY.entryLength]
        cmp     eax, 12
        jb      .error_bad_len
        test    eax, 3
        jnz     .error_bad_len
        sub     [esi+INODE.fileSize], eax
        add     ebx, eax
        cmp     ebx, [edi+24]
        jb      .wanted_start
        push    .wanted_start
        jmp     .end_block

.end_dir:
        call    ext_unlock
        mov     edx, [edi+28]
        mov     ebx, [edi+8]
        mov     ecx, [edi+4]
        mov     dword [edx], 1  ; version
        mov     [edx+4], ebx
        mov     [edx+8], ecx
        lea     esp, [edi+32]
        mov     ecx, 20/4
        lea     edi, [edx+12]
        xor     eax, eax
        rep stosd
        ret

.error_bad_len:
        movi    eax, ERROR_FS_FAIL
.error_read_subinode:
.error_get_block:
        lea     esp, [edi+32]
.error_ret:
        or      ebx, -1
        push    eax
        call    ext_unlock
        pop     eax
        ret

.error_empty_dir:
        movi    eax, ERROR_FS_FAIL
        jmp     .error_ret

.error_not_found:
        movi    eax, ERROR_FILE_NOT_FOUND
        jmp     .error_ret

;----------------------------------------------------------------
ext_ReadFile:
        call    ext_lock
        push    ERROR_ACCESS_DENIED
        cmp     byte [esi], 0
        jz      .error  ; root
        mov     [esp], ebx
        stdcall findInode, [esp+4+4]
        pop     ebx
        jc      .error_eax
        push    ERROR_ACCESS_DENIED
        lea     esi, [ebp+EXTFS.mainInodeBuffer]
        mov     ax, [esi+INODE.accessMode]
        and     ax, TYPE_MASK
        cmp     ax, FLAG_FILE
        jnz     .error  ; not a file
        pop     eax
        mov     edi, [ebx+16]
        mov     ecx, [ebx+12]
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        push    ERROR_END_OF_FILE
        cmp     [esi+INODE.fileSizeHigh], edx
        ja      @f
        jb      .error
        cmp     [esi+INODE.fileSize], eax
        jna     .error
@@:
        add     esp, 4
        add     eax, ecx
        adc     edx, 0
        cmp     [esi+INODE.fileSizeHigh], edx
        ja      .read_till_requested
        jb      .read_whole_file
        cmp     [esi+INODE.fileSize], eax
        jae     .read_till_requested
.read_whole_file:
        push    1   ; read till the end of file
        mov     ecx, [esi+INODE.fileSize]
        sub     ecx, [ebx+4]
        jmp     @f

.read_till_requested:
        push    0   ; read as much as requested
@@: ; ecx = bytes to read, edi -> buffer
        push    ecx
; read part of the first block
        mov     edx, [ebx+8]
        mov     eax, [ebx+4]
        div     [ebp+EXTFS.bytesPerBlock]
        push    eax
        push    ecx
        mov     ecx, eax
        call    extfsGetFileBlock
        jc      .error_at_first_block
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        mov     eax, ecx
        call    extfsReadBlock
        jc      .error_at_first_block
        pop     ecx
        add     ebx, edx
        neg     edx
        add     edx, [ebp+EXTFS.bytesPerBlock]
        cmp     ecx, edx
        jbe     .only_one_block
        mov     eax, ecx
        sub     eax, edx    ; bytes to read
        mov     ecx, edx
        push    esi
        mov     esi, ebx
        rep movsb
        pop     esi
        mov     ebx, edi
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        mov     edi, eax
@@:
        test    edi, edi
        jz      .finish_block
        inc     dword [esp]
        mov     ecx, [esp]
        call    extfsGetFileBlock
        jc      .error_at_read_cycle
        mov     eax, ecx
        call    extfsReadBlock
        jc      .error_at_read_cycle
        add     ebx, [ebp+EXTFS.bytesPerBlock]
        dec     edi
        jmp     @b

.finish_block:  ; edx = number of bytes in the last block
        test    edx, edx
        jz      .end_read
        pop     ecx     ; block counter
        inc     ecx
        call    extfsGetFileBlock
        jc      .error_at_finish_block
        mov     edi, ebx
        mov     eax, ecx
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsReadBlock
        jc      .error_at_finish_block
        push    eax
        mov     ecx, edx
.only_one_block:
        mov     esi, ebx
        rep movsb
.end_read:
        call    ext_unlock
        pop     eax ebx eax
        test    eax, eax
        jz      @f
        movi    eax, ERROR_END_OF_FILE
@@:
        ret

.error_at_first_block:
        pop     ebx
.error_at_read_cycle:
        pop     ebx
.error_at_finish_block:
        pop     ebx ebx
.error_eax:
        push    eax
.error:
        call    ext_unlock
        xor     ebx, ebx
        pop     eax
        ret

;----------------------------------------------------------------
ext_GetFileInfo:
        call    ext_lock
        mov     edx, [ebx+16]
        cmp     byte [esi], 0
        jz      .is_root
        push    edx
        stdcall findInode, [esp+4+4]
        mov     ebx, edx
        pop     edx
        lea     esi, [ebp+EXTFS.mainInodeBuffer]
        jnc     @f
        push    eax
        call    ext_unlock
        pop     eax
        ret

.is_root:
        xor     ebx, ebx
        lea     esi, [ebp+EXTFS.rootInodeBuffer]
@@:
        xor     eax, eax
        mov     edi, edx
        mov     ecx, 40/4
        rep stosd
        cmp     bl, '.'
        jne     @f
        or      dword [edx], KOS_HIDDEN
@@:
        or      dword [edx], KOS_DIRECTORY
        test    [esi+INODE.accessMode], DIRECTORY
        jnz     @f
        xor     dword [edx], KOS_DIRECTORY  ; mark as file
        mov     eax, [esi+INODE.fileSize]
        mov     ebx, [esi+INODE.fileSizeHigh]
        mov     dword [edx+32], eax
        mov     dword [edx+36], ebx
@@:
        lea     edi, [edx+8]
        mov     eax, [esi+INODE.inodeModified]
        sub     eax, 978307200  ; 01.01.1970-01.01.2001 = (365*31+8)*24*60*60
        call    fsTime2bdfe

        mov     eax, [esi+INODE.accessedTime]
        sub     eax, 978307200
        call    fsTime2bdfe

        mov     eax, [esi+INODE.dataModified]
        sub     eax, 978307200
        call    fsTime2bdfe
        call    ext_unlock
        xor     eax, eax
        ret

;----------------------------------------------------------------
ext_SetFileInfo:
        call    extfsWritingInit
        pushd   [ebx+16]
        stdcall findInode, [esp+4+4]
        pop     edx
        jc      @f
        push    esi     ; inode number
        lea     esi, [edx+16]
        lea     edi, [ebp+EXTFS.mainInodeBuffer]
        call    fsCalculateTime
        add     eax, 978307200  ; 01.01.1970-01.01.2001 = (365*31+8)*24*60*60
        mov     [edi+INODE.accessedTime], eax

        add     esi, 8
        call    fsCalculateTime
        add     eax, 978307200
        mov     [edi+INODE.dataModified], eax
        mov     ebx, edi
        pop     eax
        call    writeInode
@@:
        push    eax
        jc      @f
        call    writeSuperblock
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
@@:
        call    ext_unlock
        pop     eax
        ret

;----------------------------------------------------------------
ext_Delete:
        call    extfsWritingInit
        push    esi
        stdcall findInode, [esp+4+4]
        mov     ebx, esi
        pop     esi
        push    eax
        jc      .ret
        pop     eax
        lea     edx, [ebp+EXTFS.mainInodeBuffer]
        movzx   edx, [edx+INODE.accessMode]
        and     edx, TYPE_MASK
        cmp     edx, DIRECTORY
        jne     .file
        push    esi ebx edx 0
        lea     esi, [ebp+EXTFS.mainInodeBuffer]
.checkDirectory:
        mov     ecx, [esp]
        call    extfsGetFileBlock
        jc      .not_empty_eax
        test    ecx, ecx
        jz      .empty
        mov     eax, ecx
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .not_empty_eax
        mov     edx, ebx
        add     edx, [ebp+EXTFS.bytesPerBlock]
        movzx   ecx, [ebx+DIRENTRY.entryLength]
        add     ebx, ecx
.dir_entry:
        cmp     byte [ebx+DIRENTRY.nameLength], 1
        jne     @f
        cmp     byte [ebx+DIRENTRY.name], '.'
        jne     .not_empty
@@:
        cmp     byte [ebx+DIRENTRY.nameLength], 2
        jne     .not_empty
        cmp     word [ebx+DIRENTRY.name], '..'
        jne     .not_empty
        movzx   ecx, [ebx+DIRENTRY.entryLength]
        add     ebx, ecx
        cmp     ebx, edx
        jb      .dir_entry
        inc     dword[esp]
        jmp     .checkDirectory

.empty:
        pop     edx edx ebx esi
.file:
        call    findInode_parent
        jc      .error
        mov     eax, esi
; save file/folder's and parent's inode
        push    ebx eax
        cmp     edx, DIRECTORY
        jne     @f
; Unlink '.'
        mov     eax, [esp+4]
        call    unlinkInode
        cmp     eax, -1
        je      .error_stack8
; Unlink '..'
        mov     eax, [esp+4]
        mov     ebx, [esp]
        call    unlinkInode
        cmp     eax, -1
        je      .error_stack8
@@:
        pop     eax
        mov     ebx, [esp]
        call    unlinkInode
        cmp     eax, -1
        je      .error_stack4
        test    eax, eax
        jz      @f
; has hardlinks
        xor     eax, eax
        mov     [esp], eax
        jmp     .disk_sync

@@:
        mov     eax, [esp]
        lea     ebx, [ebp+EXTFS.mainInodeBuffer]
        call    readInode
        jc      .error_stack4_eax
; free file's data
        lea     esi, [ebp+EXTFS.mainInodeBuffer]
        xor     ecx, ecx
@@:
        push    ecx
        call    extfsGetFileBlock
        jc      .error_stack8_eax
        mov     eax, ecx
        test    eax, eax
        jz      @f
        xor     ecx, ecx
        call    extfsResourceFree
        pop     ecx
        inc     ecx
        jmp     @b

@@: ; free indirect blocks
        pop     ecx
        push    edx
        lea     edi, [ebp+EXTFS.mainInodeBuffer]
        mov     eax, [edi+INODE.addressBlock]
        test    eax, eax
        jz      .success
        xor     ecx, ecx
        call    extfsResourceFree
        mov     eax, [edi+INODE.doubleAddress]
        call    freeDoublyIndirectBlock
        cmp     eax, 1
        je      .success
        mov     eax, [edi+INODE.tripleAddress]
        test    eax, eax
        jz      .success
        push    eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        pop     ecx
        jc      .error_stack8_eax
        mov     eax, ecx
        xor     ecx, ecx
        call    extfsResourceFree
        mov     edx, ebx
        add     edx, [ebp+EXTFS.bytesPerBlock]
@@:
        mov     eax, [ebx]
        test    eax, eax
        jz      .success
        push    ebx edx
        call    freeDoublyIndirectBlock
        pop     edx ebx
        cmp     eax, 1
        je      .success
        add     ebx, 4
        cmp     ebx, edx
        jb      @b
.success:   ; clear the inode, and add deletion time
        xor     eax, eax
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        rep stosb
        lea     edi, [ebp+EXTFS.mainInodeBuffer]
        call    fsGetTime
        pop     edx
        add     eax, 978307200
        mov     [edi+INODE.deletedTime], eax
        mov     eax, [esp]
        mov     ebx, edi
        call    writeInode
        jc      .error_stack4_eax
        cmp     edx, DIRECTORY
        jne     @f
        mov     eax, [esp]
        dec     eax
        xor     edx, edx
        div     [ebp+EXTFS.superblock.inodesPerGroup]
        push    eax
        call    extfsReadDescriptor
        jc      .error_stack8
        dec     [eax+BGDESCR.directoriesCount]
        pop     eax
        call    extfsWriteDescriptor
@@: ; free inode
        pop     eax
        dec     eax
        xor     ecx, ecx
        inc     ecx
        call    extfsResourceFree
        push    eax
.disk_sync:
        call    writeSuperblock
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
.ret:
        call    ext_unlock
        xor     ebx, ebx
        pop     eax
        ret

.not_empty:
        pop     eax
.error_stack8:
        pop     eax
.error_stack4:
        pop     eax
        push    ERROR_ACCESS_DENIED
        jmp     .disk_sync

.not_empty_eax:
        pop     ebx
.error_stack8_eax:
        pop     ebx
.error_stack4_eax:
        pop     ebx
.error:
        push    eax
        jmp     .disk_sync

;----------------------------------------------------------------
ext_CreateFolder:
        call    extfsWritingInit
        push    esi
        stdcall findInode, [esp+4+4]
        pop     esi
        jnc     .success    ; exist
        call    findInode_parent
        jc      .error
        mov     eax, esi
        xor     ebx, ebx
        inc     ebx
        call    extfsResourceAlloc
        jc      .error
        inc     ebx
        push    ebx esi edi
        xor     al, al
        lea     edi, [ebp+EXTFS.tempInodeBuffer]
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        rep stosb
        lea     edi, [ebp+EXTFS.tempInodeBuffer]
        call    fsGetTime
        add     eax, 978307200
        mov     [edi+INODE.accessedTime], eax
        mov     [edi+INODE.dataModified], eax
        mov     ebx, edi
        pop     edi esi edx
; edx = allocated inode number, edi -> filename, esi = parent inode number
        mov     [ebx+INODE.accessMode], DIRECTORY or PERMISSIONS
        mov     eax, edx
        call    writeInode
        jc      .error
; link to self
        push    edx esi
        mov     eax, edx
        mov     ebx, eax
        mov     dl, DIR_DIRECTORY
        mov     esi, self_link
        call    linkInode
        pop     esi edx
        jc      .error
; link to parent
        push    edx esi
        mov     eax, ebx
        mov     ebx, esi
        mov     dl, DIR_DIRECTORY
        mov     esi, parent_link
        call    linkInode
        pop     esi edx
        jc      .error
; link parent to child
        mov     eax, esi
        mov     ebx, edx
        mov     esi, edi
        mov     dl, DIR_DIRECTORY
        call    linkInode
        jc      .error
        mov     eax, ebx
        dec     eax
        xor     edx, edx
        div     [ebp+EXTFS.superblock.inodesPerGroup]
        mov     edx, eax
        call    extfsReadDescriptor
        jc      @f
        inc     [eax+BGDESCR.directoriesCount]
        mov     eax, edx
        call    extfsWriteDescriptor
.success:
.error:
        push    eax
        call    writeSuperblock
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
        call    ext_unlock
        pop     eax
        ret

@@:
        movi    eax, ERROR_DEVICE
        jmp     .error

self_link   db ".", 0
parent_link db "..", 0

;----------------------------------------------------------------
ext_CreateFile:
        call    extfsWritingInit
        push    ebx esi
        stdcall findInode, [esp+8+4]
        mov     esi, [esp]
        jc      @f
        call    ext_unlock
        stdcall ext_Delete, [esp+8+4]
        mov     [esp], eax
        call    ext_lock
        pop     eax
        test    eax, eax
        jnz     .error
        mov     esi, [esp]
@@:
        call    findInode_parent
        jc      .error
        mov     eax, esi
        xor     ebx, ebx
        inc     ebx
        call    extfsResourceAlloc
        jc      .error
        inc     ebx
        push    ebx esi edi
        xor     al, al
        lea     edi, [ebp+EXTFS.tempInodeBuffer]
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        rep stosb
        lea     edi, [ebp+EXTFS.tempInodeBuffer]
        call    fsGetTime
        add     eax, 978307200
        mov     [edi+INODE.accessedTime], eax
        mov     [edi+INODE.dataModified], eax
        mov     ebx, edi
        pop     edi esi edx
; edx = allocated inode number, edi -> filename, esi = parent inode number
        mov     [ebx+INODE.accessMode], FLAG_FILE or PERMISSIONS
        mov     eax, edx
        call    writeInode
        jc      .error
; link parent to child
        mov     eax, esi
        mov     ebx, edx
        mov     esi, edi
        mov     dl, DIR_FLAG_FILE
        call    linkInode
        jc      .error
        pop     esi ebx
        call    ext_unlock
        jmp     ext_WriteFile

.error:
        push    eax
        call    ext_unlock
        pop     eax ebx ebx
        xor     ebx, ebx
        ret

;----------------------------------------------------------------
ext_WriteFile:
        call    extfsWritingInit
        push    0 ebx
        stdcall findInode, [esp+8+4]
        jc      .error
        lea     edx, [ebp+EXTFS.mainInodeBuffer]
        movi    eax, ERROR_ACCESS_DENIED
        test    [edx+INODE.accessMode], FLAG_FILE
        jz      .error  ; not a file
        mov     ebx, [esp]
        push    esi     ; inode number
        mov     eax, esi
        mov     ecx, [ebx+4]
        call    extfsExtendFile
        jc      .error2
        mov     ecx, [ebx+12]
        mov     esi, [ebx+16]
        mov     eax, [edx+INODE.fileSize]
        push    eax
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        test    edx, edx
        jz      .start_aligned
        mov     ebx, [ebp+EXTFS.bytesPerBlock]
        sub     ebx, edx
        cmp     ebx, ecx
        jbe     @f
        mov     ebx, ecx
@@:
        push    eax
        call    extfsReadFileBlock
        pop     edi
        jc      .error_inode_size
        mov     eax, edi
        push    ecx
        mov     ecx, ebx
        mov     edi, ebx
        add     edi, edx
        rep movsb
        pop     ecx
        call    extfsWriteFileBlock
        jc      .error_inode_size
        add     [esp], ebx
        sub     ecx, ebx
        jz      .write_inode
.start_aligned:
        cmp     ecx, [ebp+EXTFS.bytesPerBlock]
        jb      @f
        mov     eax, [esp]
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        mov     edx, [esp+4]
        push    eax
        call    extfsEraseFileBlock
        pop     edi
        jc      .error_inode_size
        mov     eax, edi
        push    ecx
        mov     ecx, [ebp+EXTFS.bytesPerBlock]
        mov     edi, [ebp+EXTFS.mainBlockBuffer]
        rep movsb
        pop     ecx
        call    extfsWriteFileBlock
        jc      .error_inode_size
        mov     eax, [ebp+EXTFS.bytesPerBlock]
        sub     ecx, eax
        add     [esp], eax
        jmp     .start_aligned
@@:         ; Handle the remaining bytes.
        test    ecx, ecx
        jz      .write_inode
        mov     eax, [esp]
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        push    eax
        call    extfsReadFileBlock
        pop     eax
        jnc     @f
        mov     edx, [esp+4]
        push    eax
        call    extfsEraseFileBlock
        pop     edi
        jc      .error_inode_size
        mov     eax, edi
@@:
        push    ecx
        mov     edi, [ebp+EXTFS.mainBlockBuffer]
        rep movsb
        pop     ecx
        call    extfsWriteFileBlock
        jc      .error_inode_size
        add     [esp], ecx
        xor     ecx, ecx
.error_inode_size:
        mov     [esp+12], eax
.write_inode:
        lea     ebx, [ebp+EXTFS.tempInodeBuffer]
        pop     [ebx+INODE.fileSize]
        pop     eax
        call    writeInode
        pop     ebx
        mov     ebx, [ebx+12]
        sub     ebx, ecx
        test    eax, eax
        jz      @f
        mov     [esp], eax
@@:
        call    writeSuperblock
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
@@:
        call    ext_unlock
        pop     eax
        ret

.error2:
        pop     ebx
.error:
        pop     ebx ebx
        push    eax
        jmp     @b

;----------------------------------------------------------------
ext_SetFileEnd:
        call    extfsWritingInit
        pushd   [ebx+4]
        stdcall findInode, [esp+4+4]
        pop     ecx
        jc      @f
        lea     edx, [ebp+EXTFS.mainInodeBuffer]
        movi    eax, ERROR_ACCESS_DENIED
        cmp     [edx+INODE.accessMode], FLAG_FILE
        jnz     @f      ; not a file
        mov     eax, esi
        call    extfsExtendFile
        jc      @f
        mov     eax, esi
        call    extfsTruncateFile
        jc      @f
        mov     eax, esi
        lea     ebx, [ebp+EXTFS.tempInodeBuffer]
        call    writeInode
@@:
        push    eax
        call    writeSuperblock
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
        call    ext_unlock
        pop     eax
        ret
