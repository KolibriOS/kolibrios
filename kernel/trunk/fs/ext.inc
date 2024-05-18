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
; esi -> path string in UTF-8
;   out:
; eax, ebx = return values for sysfunc 70
iglobal
align 4
ext_user_functions:
        dd      ext_free
        dd      (ext_user_functions_end - ext_user_functions - 8) / 4
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
        dd      ext_Rename
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
UUID                rb  16
volumeLabel         rb  16
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
bytesPerBlock       dd  ?
sectorsPerBlock     dd  ?
dwordsPerBlock      dd  ?
dwordsPerBranch     dd  ?   ; dwordsPerBlock ^ 2
mainBlockBuffer     dd  ?
tempBlockBuffer     dd  ?
descriptorTable     dd  ?
descriptorTableEnd  dd  ?
align0  rb  200h-EXTFS.align0
superblock          SUPERBLOCK
align1  rb  400h-EXTFS.align1
rootInodeBuffer     INODE
align2  rb  600h-EXTFS.align2
inodeBuffer         INODE
align3  rb  800h-EXTFS.align3
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
        ja      .fail
        test    [ebx+SUPERBLOCK.incompatibleFlags], not INCOMPATIBLE_SUPPORT
        jnz     .fail
        cmp     [ebx+SUPERBLOCK.sectorsPerBlockLog], 6  ; 64KB
        ja      .fail
        cmp     [ebx+SUPERBLOCK.inodeSize], 512
        ja      .fail
        cmp     [ebx+SUPERBLOCK.blocksPerGroup], 0
        je      .fail
        cmp     [ebx+SUPERBLOCK.inodesPerGroup], 0
        je      .fail
        movi    eax, sizeof.EXTFS
        call    malloc
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
@@:
        mov     eax, [ebx+SUPERBLOCK.inodesTotal]
        dec     eax
        xor     edx, edx
        div     [ebx+SUPERBLOCK.inodesPerGroup]
        inc     eax
        shl     eax, 5
        push    eax eax
        call    kernel_alloc
        pop     ecx
        test    eax, eax
        jz      .error2
        mov     [ebp+EXTFS.descriptorTable], eax
        mov     ebx, eax
        add     eax, ecx
        mov     [ebp+EXTFS.descriptorTableEnd], eax
        mov     eax, [ebp+EXTFS.superblock.firstGroupBlock]
        inc     eax
        mul     [ebp+EXTFS.sectorsPerBlock]
        dec     ecx
        shr     ecx, 9
        inc     ecx
        call    fs_read64_sys
        test    eax, eax
        jnz     @f
        mov     al, ROOT_INODE
        lea     ebx, [ebp+EXTFS.rootInodeBuffer]
        call    readInode
        test    eax, eax
        jnz     @f
        mov     eax, ebp
        pop     edi esi ebp ebx
        ret

@@:
        stdcall kernel_free, [ebp+EXTFS.descriptorTable]
.error2:
        stdcall kernel_free, [ebp+EXTFS.mainBlockBuffer]
.error:
        mov     eax, ebp
        call    free
        pop     edi esi ebp
.fail:
        pop     ebx
        xor     eax, eax
        ret

; unmount EXT partition
ext_free:
; in: eax -> EXTFS structure
        push    eax [eax+EXTFS.mainBlockBuffer]
        stdcall kernel_free, [eax+EXTFS.descriptorTable]
        call    kernel_free
        pop     eax
        jmp     free

extfsWriteBlock:
        push    fs_write64_sys
        jmp     @f
; in: eax = block number, ebx -> buffer
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

extfsWriteDescriptor:
; in: ebx = block group descriptor
        mov     eax, [ebp+EXTFS.superblock.firstGroupBlock]
        inc     eax
        mul     [ebp+EXTFS.sectorsPerBlock]
        sub     ebx, [ebp+EXTFS.descriptorTable]
        shr     ebx, 9
        add     eax, ebx
        shl     ebx, 9
        add     ebx, [ebp+EXTFS.descriptorTable]
        call    fs_write32_sys
        ret

extfsExtentFree:
; in: eax = first block number, ecx = extent size
        push    ebx edx edi
        sub     eax, [ebp+EXTFS.superblock.firstGroupBlock]
        xor     edx, edx
        mov     ebx, [ebp+EXTFS.superblock.blocksPerGroup]
        div     ebx
        sub     ebx, edx
        sub     ebx, ecx
        jc      .ret
        push    edx
        mov     ebx, [ebp+EXTFS.descriptorTable]
        shl     eax, 5
        add     ebx, eax
        mov     eax, ecx
        mul     [ebp+EXTFS.sectorsPerBlock]
        add     [ebx+BGDESCR.blocksFree], cx
        add     [ebp+EXTFS.superblock.blocksFree], ecx
        sub     [ebp+EXTFS.inodeBuffer.sectorsUsed], eax
        push    [ebx+BGDESCR.blockBitmap]
        call    extfsWriteDescriptor
        pop     eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        mov     edx, eax
        call    extfsReadBlock
        pop     eax
        jc      .ret
        push    ebx edx
        mov     edi, eax
        shr     edi, 5
        shl     edi, 2
        add     edi, ebx
        mov     edx, ecx
        and     eax, 31
        jz      .aligned
        mov     ecx, 32
        sub     ecx, eax
        sub     edx, ecx
        jnc     @f
        add     ecx, edx
        xor     edx, edx
@@:
        or      ebx, -1
        shl     ebx, cl
        not     ebx
        mov     ecx, eax
        shl     ebx, cl
        not     ebx
        and     [edi], ebx
        add     edi, 4
        xor     eax, eax
.aligned:
        mov     ecx, edx
        shr     ecx, 5
        rep stosd
        and     edx, 31
        jz      @f
        mov     ecx, edx
        not     eax
        shl     eax, cl
        and     [edi], eax
@@:
        pop     eax ebx
        call    extfsWriteBlock
.ret:
        pop     edi edx ebx
        xor     eax, eax
        ret

extfsInodeAlloc:
; in: eax = parent inode number
; out: ebx = allocated inode number
        push    ecx edx edi
        dec     eax
        xor     edx, edx
        div     [ebp+EXTFS.superblock.inodesPerGroup]
        mov     ebx, [ebp+EXTFS.descriptorTable]
        shl     eax, 5
        add     ebx, eax
        push    ebx
.test_block_group:
        push    ebx
        cmp     [ebx+BGDESCR.blocksFree], 0
        jz      .next
        cmp     [ebx+BGDESCR.inodesFree], 0
        jz      .next
        dec     [ebx+BGDESCR.inodesFree]
        dec     [ebp+EXTFS.superblock.inodesFree]
        push    [ebx+BGDESCR.inodeBitmap]
        call    extfsWriteDescriptor
        pop     eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        mov     edx, eax
        mov     edi, ebx
        call    extfsReadBlock
        jc      .fail
        mov     ecx, [ebp+EXTFS.superblock.inodesPerGroup]
        or      eax, -1
        shr     ecx, 5
        repz scasd
        jz      .next
        sub     edi, 4
        mov     eax, [edi]
        not     eax
        bsf     eax, eax
        bts     [edi], eax
        sub     edi, [ebp+EXTFS.tempBlockBuffer]
        shl     edi, 3
        add     eax, edi
        mov     ecx, eax
        mov     eax, edx
        call    extfsWriteBlock
        pop     eax
        sub     eax, [ebp+EXTFS.descriptorTable]
        shr     eax, 5
        mul     [ebp+EXTFS.superblock.inodesPerGroup]
        lea     ebx, [eax+ecx+1]
        xor     eax, eax
.ret:
        pop     edi edi edx ecx
        ret

.next:      ; search forward, then backward
        pop     ebx
        cmp     ebx, [esp]
        jc      .backward
        add     ebx, 32
        cmp     ebx, [ebp+EXTFS.descriptorTableEnd]
        jc      .test_block_group
        mov     ebx, [esp]
.backward:
        sub     ebx, 32
        cmp     ebx, [ebp+EXTFS.descriptorTable]
        jnc     .test_block_group
        movi    eax, ERROR_DISK_FULL
        push    eax
.fail:
        pop     edi
        jmp     .ret

extfsExtentAlloc:
; in: eax = parent inode number, ecx = blocks max
; out: ebx = first block number, ecx = blocks allocated
        push    edx esi edi ecx
        dec     eax
        xor     edx, edx
        div     [ebp+EXTFS.superblock.inodesPerGroup]
        mov     ebx, [ebp+EXTFS.descriptorTable]
        shl     eax, 5
        add     ebx, eax
        push    ebx
.test_block_group:
        push    ebx
        cmp     [ebx+BGDESCR.blocksFree], 0
        jz      .next
        mov     eax, [ebx+BGDESCR.blockBitmap]
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        mov     edx, eax
        mov     edi, ebx
        call    extfsReadBlock
        jc      .fail
        mov     ecx, [ebp+EXTFS.superblock.blocksPerGroup]
        shr     ecx, 5
        or      eax, -1
        repz scasd
        jz      .next
        mov     esi, edi
        sub     esi, 4
        push    edx ecx
        mov     eax, [esi]
        not     eax
        bsf     ecx, eax
        not     eax
        shr     eax, cl
        shl     eax, cl
        mov     ebx, 32
        bsf     ebx, eax
        sub     ebx, ecx
        mov     eax, [esp+16]
        cmp     ebx, eax
        jc      @f
        mov     ebx, eax
@@:
        or      eax, -1
        cmp     ebx, 32
        jz      @f
        xchg    ebx, ecx
        shl     eax, cl
        not     eax
        xchg    ebx, ecx
        shl     eax, cl
@@:
        or      [esi], eax
        sub     esi, [ebp+EXTFS.tempBlockBuffer]
        shl     esi, 3
        add     esi, ecx
        mov     eax, [esp+16]
        sub     eax, ebx
        mov     [esp+16], ebx
        add     ebx, ecx
        pop     ecx
        test    eax, eax
        jz      .done
        cmp     ebx, 32
        jnz     .done
        jecxz   .done
        mov     ebx, eax
        shr     eax, 5
        inc     eax
        and     ebx, 31
        cmp     ecx, eax
        jnc     @f
        mov     eax, ecx
        mov     bl, 32
@@:
        mov     ecx, eax
        shl     eax, 5
        add     [esp+12], eax
        xor     eax, eax
        push    edi
        repz scasd
        jz      @f
        mov     eax, [edi-4]
        bsf     eax, eax
        xchg    eax, ebx
        test    ecx, ecx
        jnz     @f
        cmp     ebx, eax
        jc      @f
        mov     ebx, eax
@@:
        inc     ecx
        shl     ecx, 5
        sub     ecx, ebx
        sub     [esp+16], ecx
        mov     ecx, edi
        pop     edi
        sub     ecx, edi
        shr     ecx, 2
        dec     ecx
        or      eax, -1
        rep stosd
        mov     ecx, ebx
        jecxz   .done
        neg     ecx
        add     ecx, 32
        shr     eax, cl
        or      [edi], eax
.done:
        pop     eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsWriteBlock
        mov     ebx, [esp]
        mov     ecx, [esp+8]
        sub     [ebx+BGDESCR.blocksFree], cx
        jnc     @f
        mov     [ebx+BGDESCR.blocksFree], 0
@@:
        sub     [ebp+EXTFS.superblock.blocksFree], ecx
        call    extfsWriteDescriptor
        pop     eax ebx
        sub     eax, [ebp+EXTFS.descriptorTable]
        shr     eax, 5
        mul     [ebp+EXTFS.superblock.blocksPerGroup]
        mov     ebx, eax
        add     ebx, esi
        add     ebx, [ebp+EXTFS.superblock.firstGroupBlock]
        pop     ecx
        mov     eax, ecx
        mul     [ebp+EXTFS.sectorsPerBlock]
        add     [ebp+EXTFS.inodeBuffer.sectorsUsed], eax
        xor     eax, eax
.ret:
        pop     edi esi edx
        ret

.next:      ; search forward, then backward
        pop     ebx
        cmp     ebx, [esp]
        jc      .backward
        add     ebx, 32
        cmp     ebx, [ebp+EXTFS.descriptorTableEnd]
        jc      .test_block_group
        mov     ebx, [esp]
.backward:
        sub     ebx, 32
        cmp     ebx, [ebp+EXTFS.descriptorTable]
        jnc     .test_block_group
        movi    eax, ERROR_DISK_FULL
        push    eax
.fail:
        add     esp, 12
        xor     ecx, ecx
        stc
        jmp     .ret

extfsGetExtent:
; in: ecx = starting file block
; out: eax = first block number, ecx = extent size
        push    ebx edx esi
        lea     esi, [ebp+EXTFS.inodeBuffer]
        test    [esi+INODE.featureFlags], EXTENTS_USED
        jz      .listTreeSearch
        add     esi, INODE.blockNumbers
.extentTreeSearch:
        cmp     word [esi+NODEHEADER.magic], 0xF30A
        jne     .fail
        movzx   ebx, [esi+NODEHEADER.entriesFolow]
        add     esi, sizeof.NODEHEADER
        test    ebx, ebx
        jz      .noBlock
        cmp     word [esi-sizeof.NODEHEADER+NODEHEADER.currentDepth], 0
        je      .leaf_block
        dec     ebx
        jz      .end_search_index
@@:
        cmp     ecx, [esi+sizeof.INDEX+INDEX.fileBlock]
        jb      .end_search_index
        add     esi, sizeof.INDEX
        dec     ebx
        jnz     @b
.end_search_index:
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        mov     eax, [esi+INDEX.nodeBlock]
        call    extfsReadBlock
        jc      .fail2
        mov     esi, ebx
        jmp     .extentTreeSearch

.fail:
        movi    eax, ERROR_FS_FAIL
        jmp     .fail2

.leaf_block:
        movzx   edx, [esi+EXTENT.blocksCount]
        add     edx, [esi+EXTENT.fileBlock]
        sub     edx, ecx
        ja      .end_search_extent
        add     esi, sizeof.EXTENT
        dec     ebx
        jnz     .leaf_block
.noBlock:
        movi    eax, ERROR_END_OF_FILE
.fail2:
        pop     esi edx ebx
        stc
        ret

.end_search_extent:
        sub     ecx, [esi+EXTENT.fileBlock]
        jc      .fail
        add     ecx, [esi+EXTENT.fsBlock]
        mov     eax, ecx
        mov     ecx, edx
        pop     esi edx ebx
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
        mov     eax, [esi+INODE.tripleAddress]
        test    eax, eax
        jz      .noBlock
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .fail2
        xor     edx, edx
        mov     eax, ecx
        div     [ebp+EXTFS.dwordsPerBranch]
; eax = number in triply-indirect block, edx = number in branch
        mov     eax, [ebx+eax*4]
        test    eax, eax
        jz      .noBlock
        call    extfsReadBlock
        jc      .fail2
        mov     eax, edx
        jmp     @f

.get_direct_block:
        mov     edx, ecx
        mov     cl, 12
        lea     ebx, [esi+INODE.blockNumbers]
        jmp     .calculateExtent

.get_indirect_block:
        mov     eax, [esi+INODE.addressBlock]
        test    eax, eax
        jz      .noBlock
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .fail2
        mov     edx, ecx
        mov     ecx, [ebp+EXTFS.dwordsPerBlock]
        jmp     .calculateExtent

.get_double_indirect_block:
        mov     eax, [esi+INODE.doubleAddress]
        test    eax, eax
        jz      .noBlock
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .fail2
        mov     eax, ecx
@@:
        xor     edx, edx
        mov     ecx, [ebp+EXTFS.dwordsPerBlock]
        div     ecx
; eax = number in doubly-indirect block, edx = number in indirect block
        mov     eax, [ebx+eax*4]
        test    eax, eax
        jz      .noBlock
        call    extfsReadBlock
        jc      .fail2
.calculateExtent:
        lea     esi, [ebx+edx*4]
        lodsd
        test    eax, eax
        jz      .noBlock
        mov     ebx, eax
        sub     ecx, edx
        xor     edx, edx
@@:
        inc     edx
        dec     ecx
        jz      @f
        lodsd
        sub     eax, ebx
        sub     eax, edx
        jz      @b
@@:
        mov     eax, ebx
        mov     ecx, edx
        pop     esi edx ebx
        clc
        ret

getInodeLocation:
; in: eax = inode number
; out: eax = inode sector, edx = offset in sector
        dec     eax
        xor     edx, edx
        div     [ebp+EXTFS.superblock.inodesPerGroup]
        shl     eax, 5
        add     eax, [ebp+EXTFS.descriptorTable]
        mov     ebx, [eax+BGDESCR.inodeTable]
        imul    ebx, [ebp+EXTFS.sectorsPerBlock]
        movzx   eax, [ebp+EXTFS.superblock.inodeSize]
        mul     edx
        mov     edx, eax
        shr     eax, 9
        and     edx, 511
        add     eax, ebx
        ret

writeInode:
; in: eax = inode number, ebx -> inode data
        push    edx edi esi ecx ebx eax
        mov     edi, ebx
        call    fsGetTime
        add     eax, 978307200
        mov     [edi+INODE.inodeModified], eax
        pop     eax
        cmp     eax, ROOT_INODE
        jnz     @f
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        mov     esi, edi
        lea     edi, [ebp+EXTFS.rootInodeBuffer]
        rep movsb
@@:
        call    getInodeLocation
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        mov     ecx, eax
        call    fs_read32_sys
        test    eax, eax
        jnz     @f
        mov     eax, ecx
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        mov     edi, edx
        add     edi, ebx
        mov     esi, [esp]
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
; in: eax = inode number, ebx -> inode buffer
        push    edx edi esi ecx ebx
        mov     edi, ebx
        call    getInodeLocation
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    fs_read32_sys
        test    eax, eax
        jnz     @b
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        mov     esi, edx
        add     esi, ebx
        rep movsb
        xor     eax, eax
        pop     ebx ecx esi edi edx
        ret

indirectBlockAlloc:
;   in:
; edi -> indirect block number
; ebx = starting extent block
; ecx = extent size
; edx = starting file block
        mov     eax, [edi]
        test    eax, eax
        jz      .newBlock
        push    edi ebx ecx
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .err2
        lea     edi, [ebx+edx*4]
        test    edx, edx
        jz      @f
        cmp     dword[edi-4], 0
        jnz     @f
        pop     ecx ebx edi
.err:
        mov     al, ERROR_FS_FAIL
        stc
        ret

.err2:
        pop     ecx ebx edi
        ret

.newBlock:
        test    edx, edx
        jnz     .err
        mov     [edi], ebx
        inc     ebx
        dec     ecx
        push    edi ebx ecx
        mov     ecx, [ebp+EXTFS.dwordsPerBlock]
        mov     edi, [ebp+EXTFS.tempBlockBuffer]
        push    edi
        rep stosd
        pop     edi
@@:
        mov     ecx, [ebp+EXTFS.dwordsPerBlock]
        sub     ecx, edx
        pop     ebx eax
        sub     ebx, ecx
        jnc     @f
        add     ecx, ebx
        xor     ebx, ebx
@@:
        jecxz   .done
        add     edx, ecx
@@:
        stosd
        inc     eax
        loop    @b
.done:
        pop     edi
        push    eax ebx
        mov     eax, [edi]
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsWriteBlock
        pop     ecx ebx
        ret

doublyIndirectBlockAlloc:
;   in:
; edi -> indirect block number
; edx = starting file block
; ebx = starting extent block
; ecx = extent size
; [esp+4] = rest of size
; [esp+8] = parent inode number
        mov     eax, [edi]
        test    eax, eax
        jz      .newBlock
        push    edi ecx ebx
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsReadBlock
        jc      .err2
        mov     eax, edx
        xor     edx, edx
        mov     ecx, [ebp+EXTFS.dwordsPerBlock]
        div     ecx
        lea     edi, [ebx+eax*4]
        pop     ebx
        test    eax, eax
        jz      @f
        cmp     dword[edi-4], 0
        jnz     @f
        pop     ecx edi
.err:
        mov     al, ERROR_FS_FAIL
        stc
        ret

.err2:
        pop     ebx ecx edi
        ret

.newBlock:
        test    edx, edx
        jnz     .err
        mov     [edi], ebx
        inc     ebx
        dec     ecx
        inc     dword[esp+4]
        push    edi ecx
        mov     ecx, [ebp+EXTFS.dwordsPerBlock]
        mov     edi, [ebp+EXTFS.mainBlockBuffer]
        push    ecx edi
        rep stosd
        pop     edi ecx
@@:
        sub     ecx, eax
        xchg    [esp], ecx
.loop:
        cmp     dword[edi], 0
        jnz     @f
        inc     dword[esp+12]
@@:
        jecxz   .extentAlloc
        call    indirectBlockAlloc
        jc      .end
        cmp     edx, [ebp+EXTFS.dwordsPerBlock]
        jnz     @b
        add     edi, 4
        xor     edx, edx
        dec     dword[esp]
        jnz     .loop
.end:
        pop     edi edi
        push    ebx eax
        mov     eax, [edi]
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsWriteBlock
        pop     ebx
        add     eax, ebx
        xor     ebx, ebx
        cmp     ebx, eax
        pop     ebx
        ret

.extentAlloc:
        mov     ecx, [esp+12]
        xor     eax, eax
        jecxz   .end
        mov     eax, [esp+16]
        call    extfsExtentAlloc
        jc      .end
        sub     [esp+12], ecx
        jmp     @b

extfsExtendFile:
;   in:
; [ebp+EXTFS.inodeBuffer] = inode
; ecx = inode number
; edx:eax = new size
        push    ebx esi edi ecx
        lea     esi, [ebp+EXTFS.inodeBuffer]
        mov     ebx, [esi+INODE.fileSize]
        mov     ecx, [esi+INODE.fileSizeHigh]
        cmp     ebx, eax
        sbb     ecx, edx
        jnc     .ret
        mov     ecx, [esi+INODE.fileSizeHigh]
        mov     [esi+INODE.fileSize], eax
        mov     [esi+INODE.fileSizeHigh], edx
        sub     eax, 1
        sbb     edx, 0
        div     [ebp+EXTFS.bytesPerBlock]
        inc     eax
        xchg    eax, ebx
        mov     edx, ecx
        sub     eax, 1
        sbb     edx, 0
        jc      @f
        div     [ebp+EXTFS.bytesPerBlock]
@@:
        inc     eax
        sub     ebx, eax
        jz      .ret
        push    ebx
        mov     edx, eax
@@:
        mov     ecx, [esp]
        mov     eax, [esp+4]
        test    ecx, ecx
        jz      .done
        call    extfsExtentAlloc
        jc      .errDone
        sub     [esp], ecx
        cmp     edx, 12
        jc      .directBlocks
        sub     edx, 12
        cmp     edx, [ebp+EXTFS.dwordsPerBlock]
        jc      .indirectBlocks
        sub     edx, [ebp+EXTFS.dwordsPerBlock]
        cmp     edx, [ebp+EXTFS.dwordsPerBranch]
        jc      .doublyIndirectBlock
        sub     edx, [ebp+EXTFS.dwordsPerBranch]
        jmp     .triplyIndirectBlock

.newExtent:
        jmp     @b

.directBlocks:
        lea     edi, [esi+INODE.blockNumbers+edx*4]
        test    edx, edx
        jz      @f
        cmp     dword[edi-4], 0
        jz      .errDone
@@:
        mov     eax, ebx
        mov     ebx, ecx
        mov     ecx, 12
        sub     ecx, edx
        sub     ebx, ecx
        jnc     @f
        add     ecx, ebx
        xor     ebx, ebx
@@:
        add     edx, ecx
@@:
        stosd
        inc     eax
        loop    @b
        mov     ecx, ebx
        mov     ebx, eax
        jecxz   .newExtent
        xor     edx, edx
.indirectBlocks:
        lea     edi, [esi+INODE.addressBlock]
        cmp     dword[edi], 0
        jnz     @f
        inc     dword[esp]
@@:
        call    indirectBlockAlloc
        jc      .errDone
        add     edx, 12
        jecxz   .newExtent
        xor     edx, edx
.doublyIndirectBlock:
        lea     edi, [esi+INODE.doubleAddress]
        call    doublyIndirectBlockAlloc
        jc      .errDone
        mov     edx, [ebp+EXTFS.dwordsPerBranch]
        add     edx, [ebp+EXTFS.dwordsPerBlock]
        add     edx, 12
        jecxz   .newExtent
        xor     edx, edx
.triplyIndirectBlock:
        push    ecx ebx edx
        stdcall kernel_alloc, [ebp+EXTFS.bytesPerBlock]
        pop     edx
        mov     esi, eax
        mov     eax, [ebp+EXTFS.inodeBuffer.tripleAddress]
        test    eax, eax
        jz      .newBlock
        mov     ebx, esi
        call    extfsReadBlock
        pop     ebx ecx
        jc      .errFree
        mov     eax, edx
        xor     edx, edx
        div     [ebp+EXTFS.dwordsPerBranch]
        lea     edi, [esi+eax*4]
        test    eax, eax
        jz      @f
        cmp     dword[edi-4], 0
        jnz     @f
        mov     al, ERROR_FS_FAIL
.errFree:
        push    ecx eax
        stdcall kernel_free, esi
        pop     eax ecx
.errDone:
        imul    ecx, [ebp+EXTFS.sectorsPerBlock]
        sub     [ebp+EXTFS.inodeBuffer.sectorsUsed], ecx
        pop     ebx
        imul    ebx, [ebp+EXTFS.sectorsPerBlock]
        add     ebx, ecx
        shl     ebx, 9
        sub     [ebp+EXTFS.inodeBuffer.fileSize], ebx
        stc
        jmp     .ret

.newBlock:
        pop     ebx ecx
        mov     al, ERROR_FS_FAIL
        test    edx, edx
        jnz     .errFree
        mov     [ebp+EXTFS.inodeBuffer.tripleAddress], ebx
        inc     ebx
        dec     ecx
        inc     dword[esp]
        push    ecx
        mov     ecx, [ebp+EXTFS.dwordsPerBlock]
        mov     edi, esi
        xor     eax, eax
        rep stosd
        mov     edi, esi
        pop     ecx
@@:
        jecxz   .extentAlloc
        call    doublyIndirectBlockAlloc
        jc      .errSave
        add     edi, 4
        jmp     @b

.extentAlloc:
        mov     ecx, [esp]
        mov     eax, [esp+4]
        jecxz   @f
        call    extfsExtentAlloc
        jc      .errSave
        sub     [esp], ecx
        jmp     @b

@@:
        mov     eax, [ebp+EXTFS.inodeBuffer.tripleAddress]
        mov     ebx, esi
        call    extfsWriteBlock
        stdcall kernel_free, esi
.done:
        xor     eax, eax
        pop     edi
.ret:
        pop     edi edi esi ebx
        ret

.errSave:
        push    eax
        mov     eax, [ebp+EXTFS.inodeBuffer.tripleAddress]
        mov     ebx, esi
        call    extfsWriteBlock
        pop     eax
        jmp     .errFree

freeBlockList:
; in: edi -> list of blocks, edx = amount of blocks
; out: ebx=0 -> end of list
        mov     ebx, [edi]
        test    ebx, ebx
        jz      .ret
        xor     eax, eax
        xor     ecx, ecx
@@:
        stosd
        inc     ecx
        dec     edx
        jz      @f
        mov     eax, [edi]
        sub     eax, ebx
        sub     eax, ecx
        jz      @b
@@:
        mov     eax, ebx
        call    extfsExtentFree
        test    edx, edx
        jnz     freeBlockList
.ret:
        ret

freeIndirectBlock:
; in: edi -> indirect block number, edx = starting block
; out: edi = edi+4, eax=0 -> end
        pushd   ecx 0 edi edx
        mov     eax, [edi]
        test    eax, eax
        jz      .ret
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsReadBlock
        jc      .ret
        lea     edi, [ebx+edx*4]
        neg     edx
        add     edx, [ebp+EXTFS.dwordsPerBlock]
        call    freeBlockList
        test    ebx, ebx
        jz      @f
        inc     dword[esp+8]
@@:
        pop     edx edi
        mov     eax, [edi]
        test    edx, edx
        jnz     @f
        xor     ecx, ecx
        inc     ecx
        call    extfsExtentFree
        stosd
        jmp     .done

@@:
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsWriteBlock
        add     edi, 4
.done:
        pop     eax ecx
        xor     edx, edx
        ret

.ret:
        pop     edi edi edx ecx
        ret

freeDoublyIndirectBlock:
; in: edi -> doubly-indirect block number, edx = starting block
; out: edi = edi+4, eax=-1 -> done, eax=0 -> end
        mov     eax, [edi]
        test    eax, eax
        jz      .ret
        push    ecx eax edx
        stdcall kernel_alloc, [ebp+EXTFS.bytesPerBlock]
        mov     ebx, eax
        pop     edx eax
        pushd   0 ebx edx
        call    extfsReadBlock
        jc      .err
        mov     eax, edx
        xor     edx, edx
        mov     ecx, [ebp+EXTFS.dwordsPerBlock]
        div     ecx
        sub     ecx, eax
        push    edi
        lea     edi, [ebx+eax*4]
@@:
        call    freeIndirectBlock
        test    eax, eax
        jz      .end
        dec     ecx
        jnz     @b
        dec     dword[esp+12]
.end:
        pop     edi edx
        mov     eax, [edi]
        test    edx, edx
        jnz     @f
        xor     ecx, ecx
        inc     ecx
        call    extfsExtentFree
        stosd
        jmp     .done

@@:
        mov     ebx, [esp]
        call    extfsWriteBlock
        add     edi, 4
        jmp     .done

.err:
        mov     [esp+8], eax
        pop     eax
.done:
        call    kernel_free
        pop     eax ecx
.ret:
        xor     edx, edx
        ret

extfsTruncateFile:
; in: edx:eax = new size, [ebp+EXTFS.inodeBuffer] = inode
        lea     esi, [ebp+EXTFS.inodeBuffer]
        mov     ecx, edx
        cmp     eax, [esi+INODE.fileSize]
        sbb     ecx, [esi+INODE.fileSizeHigh]
        jnc     .ret
        mov     [esi+INODE.fileSize], eax
        mov     [esi+INODE.fileSizeHigh], edx
        sub     eax, 1
        sbb     edx, 0
        jc      @f
        div     [ebp+EXTFS.bytesPerBlock]
@@:
        inc     eax
        mov     edx, eax
        cmp     edx, 12
        jc      .directBlocks
        sub     edx, 12
        cmp     edx, [ebp+EXTFS.dwordsPerBlock]
        jc      .indirectBlocks
        sub     edx, [ebp+EXTFS.dwordsPerBlock]
        cmp     edx, [ebp+EXTFS.dwordsPerBranch]
        jc      .doublyIndirectBlock
        sub     edx, [ebp+EXTFS.dwordsPerBranch]
        jmp     .triplyIndirectBlock

.directBlocks:
        lea     edi, [esi+INODE.blockNumbers+edx*4]
        neg     edx
        add     edx, 12
        call    freeBlockList
        test    ebx, ebx
        jz      .ret
.indirectBlocks:
        lea     edi, [esi+INODE.addressBlock]
        call    freeIndirectBlock
        test    eax, eax
        jz      .ret
.doublyIndirectBlock:
        lea     edi, [esi+INODE.doubleAddress]
        call    freeDoublyIndirectBlock
        test    eax, eax
        jz      .ret
.triplyIndirectBlock:
        mov     eax, [esi+INODE.tripleAddress]
        test    eax, eax
        jz      .ret
        push    eax edx
        stdcall kernel_alloc, [ebp+EXTFS.bytesPerBlock]
        mov     ebx, eax
        pop     edx eax
        push    ebx eax edx
        call    extfsReadBlock
        jc      .err
        mov     eax, edx
        xor     edx, edx
        div     [ebp+EXTFS.dwordsPerBranch]
        mov     ecx, [ebp+EXTFS.dwordsPerBlock]
        sub     ecx, eax
        lea     edi, [ebx+eax*4]
@@:
        call    freeDoublyIndirectBlock
        test    eax, eax
        jz      .end
        dec     ecx
        jnz     @b
.end:
        pop     edx eax
        test    edx, edx
        jnz     @f
        xor     ecx, ecx
        inc     ecx
        call    extfsExtentFree
        mov     [esi+INODE.tripleAddress], eax
        jmp     .done

@@:
        mov     ebx, [esp]
        call    extfsWriteBlock
        jmp     .done

.err:
        pop     eax eax
.done:
        call    kernel_free
.ret:
        ret

linkInode:
;   in:
; eax = inode on which to link
; ebx = inode to link
; esi -> name in UTF-8
;  dl = file type
        push    esi edi ebx ecx eax edx
        call    strlen
        push    esi ebx ecx
        lea     esi, [ebp+EXTFS.inodeBuffer]
        mov     ebx, esi
        call    readInode
        jc      .error_inode_read
        mov     eax, [esi+INODE.fileSize]
        xor     edx, edx
        div     [ebp+EXTFS.bytesPerBlock]
        xor     ecx, ecx
.searchBlock:
        push    eax     ; blocks total
        push    ecx     ; current file block number
        cmp     eax, ecx
        jz      .alloc_block
        call    extfsGetExtent
        jc      .error_get_inode_block
        push    eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .error_block_read
        mov     ecx, [esp+12]
        add     ecx, 8  ; directory entry size
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
        pop     ecx
@@:
        pop     ecx eax
        inc     ecx
        jmp     .searchBlock

.zeroLength:
        mov     eax, edx
        sub     eax, edi
        mov     [edi+DIRENTRY.entryLength], ax
        cmp     eax, ecx
        jge     .found
        mov     [edi+DIRENTRY.inodeNumber], 0
        pop     eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsWriteBlock
        jmp     @b

.alloc_block:
        mov     eax, [esi+INODE.fileSize]
        add     eax, [ebp+EXTFS.bytesPerBlock]
        xor     edx, edx
        mov     ecx, [esp+24]
        call    extfsExtendFile
        jc      .error_get_inode_block
        mov     eax, [esp+24]
        mov     ebx, esi
        call    writeInode
        jc      .error_get_inode_block
        mov     ecx, [esp]
        call    extfsGetExtent
        jc      .error_get_inode_block
        push    eax
        mov     edi, [ebp+EXTFS.tempBlockBuffer]
        mov     eax, [ebp+EXTFS.bytesPerBlock]
        mov     [edi+DIRENTRY.entryLength], ax
.found:
        pop     edx ecx ecx ecx ebx esi
        mov     [edi+DIRENTRY.inodeNumber], ebx
        mov     word [edi+DIRENTRY.nameLength], cx
        sub     eax, 8
        cmp     ecx, eax
        adc     ecx, 0
        test    [ebp+EXTFS.superblock.incompatibleFlags], 2
        jz      @f
        mov     eax, [esp]
        mov     [edi+DIRENTRY.fileType], al
@@:
        add     edi, 8
        rep movsb
        mov     eax, edx
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsWriteBlock
@@:
        pop     edx ecx ecx ebx edi esi
        ret

.error_block_read:
        pop     ebx
.error_get_inode_block:
        pop     ebx ebx
.error_inode_read:
        pop     ebx ebx ebx
        jmp     @b

unlinkInode:
; in: eax = directory inode number, esi = inode to unlink
        push    edi
        lea     ebx, [ebp+EXTFS.inodeBuffer]
        call    readInode
        jc      .ret
        xor     ecx, ecx
.loop:
        push    ecx
        call    extfsGetExtent
        jc      .fail_loop
        mov     edi, eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .fail_loop
.first_dir_entry:   ; edi -> block
        cmp     [ebx+DIRENTRY.inodeNumber], esi
        jne     @f
        mov     [ebx+DIRENTRY.inodeNumber], 0
        mov     word [ebx+DIRENTRY.nameLength], 0   ; fileType = 0
        jmp     .write_block

.fail:
        pop     edi
        movi    eax, ERROR_FS_FAIL
        stc
.fail_loop:
        pop     edi
        jmp     .ret

.next:
        pop     ecx ecx
        inc     ecx
        jmp     .loop

@@:
        mov     edx, ebx
        add     edx, [ebp+EXTFS.bytesPerBlock]
        push    edx
@@:
        movzx   ecx, [ebx+DIRENTRY.entryLength]
        jecxz   .fail
        mov     edx, ebx
        add     ebx, ecx
        cmp     ebx, [esp]
        jnc     .next
        cmp     [ebx+DIRENTRY.inodeNumber], esi
        jnz     @b
        mov     cx, [ebx+DIRENTRY.entryLength]
        add     [edx+DIRENTRY.entryLength], cx
        pop     eax
.write_block:
        pop     eax
        mov     eax, edi
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsWriteBlock
.ret:
        pop     edi
        ret

findInode:
; in: esi -> path string in UTF-8
;   out:
; edi -> file name in UTF-8
; esi = last inode number
; [ebp+EXTFS.inodeBuffer] = last inode
; ecx = parent inode number
; CF=1 -> file not found, edi=0 -> error
        push    esi
        lea     esi, [ebp+EXTFS.rootInodeBuffer]
        lea     edi, [ebp+EXTFS.inodeBuffer]
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        mov     edx, esi
        rep movsb
        pop     esi
        pushd   ebx 0 ROOT_INODE
        mov     edi, esi
        cmp     [edx+INODE.fileSize], 0
        jz      .not_found
        cmp     byte [esi], 0
        jnz     .next_path_part
        xor     eax, eax
        pop     esi ecx ebx
        ret

@@:
        pop     esi esi
.error:
        pop     esi ecx ebx
        xor     edi, edi
        stc
        ret

.next_path_part:
        push    [edx+INODE.fileSize]
        xor     ecx, ecx
.folder_block_cycle:
        push    ecx
        call    extfsGetExtent
        jc      @b
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    extfsReadBlock
        jc      @b
        push    esi edx
        mov     edx, ebx
        add     edx, [ebp+EXTFS.bytesPerBlock]
.start_rec:
        cmp     [ebx+DIRENTRY.inodeNumber], 0
        jz      .next_rec
        push    esi
        movzx   ecx, [ebx+DIRENTRY.nameLength]
        lea     edi, [ebx+DIRENTRY.name]
        repz cmpsb
        jz      .test_find
@@: ; doesn't match
        pop     esi
.next_rec:
        movzx   ecx, [ebx+DIRENTRY.entryLength]
        jecxz   .stop
        add     ebx, ecx
        cmp     ebx, edx
        jb      .start_rec
        jmp     .stop

.test_find:
        cmp     byte [esi], 0
        je      @f
        cmp     byte [esi], '/'
        jne     @b
        inc     esi
@@:
        pop     edx
.stop:
        pop     edx edi ecx eax
; ebx -> matched directory entry, esi -> name without parent, or not changed
        cmp     edi, esi
        jnz     @f
        sub     eax, [ebp+EXTFS.bytesPerBlock]
        jle     .not_found
        push    eax
        inc     ecx
        jmp     .folder_block_cycle

@@:
        pop     eax
        mov     [esp], eax
        mov     eax, [ebx+DIRENTRY.inodeNumber]
        lea     ebx, [ebp+EXTFS.inodeBuffer]
        push    eax
        call    readInode
        jc      .error
        cmp     byte [esi], 0
        je      .ret
        mov     edx, ebx
        movzx   eax, [ebx+INODE.accessMode]
        and     eax, TYPE_MASK
        cmp     eax, DIRECTORY
        jz      .next_path_part
        xor     edi, edi    ; path folder is a file
        jmp     @f

.not_found:
        mov     esi, edi
        call    strlen
        mov     al, '/'
        repnz scasb
        mov     edi, esi
        jnz     @f
        xor     edi, edi    ; path folder not found
@@:
        movi    eax, ERROR_FILE_NOT_FOUND
        stc
.ret:
        pop     esi ecx ebx
        ret

writeSuperblock:
        push    ebx
        mov     eax, 2
        lea     ebx, [ebp+EXTFS.superblock]
        call    fs_write32_sys
        pop     ebx
        ret

extfsWritingInit:
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
        call    findInode
        jc      .error_ret
        lea     esi, [ebp+EXTFS.inodeBuffer]
        test    [esi+INODE.accessMode], FLAG_FILE
        jnz     .error_not_found
        jmp     @f

.root_folder:
        lea     esi, [ebp+EXTFS.rootInodeBuffer]
        lea     edi, [ebp+EXTFS.inodeBuffer]
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
        call    extfsGetExtent
        jc      .error_get_block
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
        call    extfsGetExtent
        jc      .error_get_block
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
        push    ebx edi ecx esi edx edi
        pushd   [edi+12]
        mov     edi, edx
        xor     eax, eax
        mov     ecx, 40 / 4
        rep stosd
        popd    [edx+4] edi
        mov     eax, [ebx+DIRENTRY.inodeNumber]
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
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
        test    [esi+INODE.accessMode], FLAG_FILE
        jz      @f
        xor     dword [edx], KOS_DIRECTORY  ; mark as file
        mov     eax, [esi+INODE.fileSize]
        stosd
        mov     eax, [esi+INODE.fileSizeHigh]
        stosd
@@:
        mov     esi, [esp+12]
        movzx   ecx, [esi+DIRENTRY.nameLength]
        lea     esi, [esi+DIRENTRY.name]
        cmp     byte [esi], '.'
        jnz     @f
        or      byte [edx], KOS_HIDDEN
@@:
        lea     edi, [edx+40]
        cmp     byte [edx+4], 3
        jz      .utf8
        add     ecx, esi
        cmp     byte [edx+4], 2
        jz      .utf16
@@:
        call    utf8to16
        call    uni2ansi_char
        stosb
        cmp     esi, ecx
        jc      @b
        and     byte [edi], 0
        add     edx, 40+264
@@:
        pop     esi ecx edi ebx
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

.utf8:
        rep movsb
        mov     byte [edi], 0
        add     edx, 40+520
        jmp     @b

.utf16:
        call    utf8to16
        stosw
        cmp     esi, ecx
        jc      .utf16
        and     word [edi], 0
        add     edx, 40+520
        jmp     @b

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
        xor     ebx, ebx
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
        call    findInode
        pushd   0 eax
        jc      .ret
        lea     esi, [ebp+EXTFS.inodeBuffer]
        mov     byte [esp], ERROR_ACCESS_DENIED
        test    [esi+INODE.accessMode], FLAG_FILE
        jz      .ret    ; not a file
        mov     byte [esp], ERROR_END_OF_FILE
        mov     eax, [esi+INODE.fileSize]
        mov     edx, [esi+INODE.fileSizeHigh]
        sub     eax, [ebx+4]
        sbb     edx, [ebx+8]
        jc      .ret
        mov     ecx, [ebx+12]
        sub     eax, ecx
        sbb     edx, 0
        jc      @f
        xor     eax, eax
        mov     [esp], eax
@@:
        add     ecx, eax
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        mov     edi, [ebx+16]
        div     [ebp+EXTFS.bytesPerBlock]
        test    edx, edx
        jz      .aligned
.piece:
        push    eax ecx
        mov     esi, edx
        mov     ecx, eax
        call    extfsGetExtent
        jc      .errorGet
        mov     ecx, [ebp+EXTFS.sectorsPerBlock]
        mul     ecx
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    fs_read64_sys
        test    eax, eax
        jnz     .errorRead
        pop     eax
        mov     ecx, [ebp+EXTFS.bytesPerBlock]
        sub     ecx, esi
        sub     eax, ecx
        jnc     @f
        add     ecx, eax
        xor     eax, eax
@@:
        add     esi, ebx
        add     [esp+8], ecx
        rep movsb
        mov     ecx, eax
        pop     eax
        inc     eax
        xor     edx, edx
        jecxz   .ret
.aligned:
        xchg    eax, ecx
        div     [ebp+EXTFS.bytesPerBlock]
        push    edx
        mov     edx, eax
.writeExtent:
        test    edx, edx
        jz      .end
        push    ecx
        call    extfsGetExtent
        jc      .errorGet
        sub     edx, ecx
        jnc     @f
        add     ecx, edx
        xor     edx, edx
@@:
        add     [esp], ecx
        imul    ecx, [ebp+EXTFS.sectorsPerBlock]
        mov     ebx, edi
        push    edx ecx
        mul     [ebp+EXTFS.sectorsPerBlock]
        call    fs_read64_sys
        pop     ecx edx
        test    eax, eax
        jnz     .errorRead
        shl     ecx, 9
        add     edi, ecx
        add     [esp+12], ecx
        pop     ecx
        jmp     .writeExtent

.end:
        mov     eax, ecx
        pop     ecx
        jecxz   .ret
        jmp     .piece

.errorRead:
        movi    eax, ERROR_DEVICE
.errorGet:
        pop     ebx ebx
        mov     [esp], eax
.ret:
        call    ext_unlock
        pop     eax ebx
        ret

;----------------------------------------------------------------
ext_GetFileInfo:
        cmp     byte [esi], 0
        jz      .volume
        call    ext_lock
        call    findInode
        jc      .ret
        lea     esi, [ebp+EXTFS.inodeBuffer]
        mov     edx, [ebx+16]
        mov     bl, [edi]
        xor     eax, eax
        mov     edi, edx
        mov     ecx, 40/4
        rep stosd
        cmp     bl, '.'
        jne     @f
        or      dword [edx], KOS_HIDDEN
@@:
        or      dword [edx], KOS_DIRECTORY
        test    [esi+INODE.accessMode], FLAG_FILE
        jz      @f
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
        xor     eax, eax
.ret:
        push    eax
        call    ext_unlock
        pop     eax
@@:
        ret

.volume:
        mov     eax, dword[ebp+EXTFS.Length]
        mov     edx, dword[ebp+EXTFS.Length+4]
        mov     edi, [ebx+16]
        shld    edx, eax, 9
        shl     eax, 9
        mov     [edi+36], edx
        mov     [edi+32], eax
        mov     eax, [ebx+8]
        mov     byte [edi], 8
        mov     [edi+4], eax
        test    eax, eax
        jz      @b
        lea     esi, [ebp+EXTFS.superblock.volumeLabel]
        mov     ecx, 16
        add     edi, 40
        cmp     eax, 3
        jz      .utf8
        add     ecx, esi
        cmp     eax, 2
        jz      .utf16
@@:
        call    utf8to16
        call    uni2ansi_char
        stosb
        cmp     esi, ecx
        jc      @b
        jmp     @f

.utf8:
        rep movsb
        jmp     @f

.utf16:
        call    utf8to16
        stosw
        cmp     esi, ecx
        jc      .utf16
@@:
        xor     eax, eax
        mov     [edi], ax
        ret

;----------------------------------------------------------------
ext_SetFileInfo:
        call    extfsWritingInit
        call    findInode
        jc      @f
        push    esi
        mov     esi, [ebx+16]
        add     esi, 16
        lea     edi, [ebp+EXTFS.inodeBuffer]
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
        call    findInode
        jc      .error
        push    ecx
        movzx   edi, [ebp+EXTFS.inodeBuffer.accessMode]
        and     edi, TYPE_MASK
        cmp     edi, DIRECTORY
        jne     .file
        xor     ecx, ecx
.checkDirectory:
        push    ecx
        call    extfsGetExtent
        jc      .empty
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        call    extfsReadBlock
        jc      .error8
        mov     edx, ebx
        add     edx, [ebp+EXTFS.bytesPerBlock]
.dir_entry:
        movzx   ecx, [ebx+DIRENTRY.nameLength]
        mov     ax, word [ebx+DIRENTRY.name]
        jecxz   @f
        cmp     al, '.'
        jnz     .not_empty
        dec     ecx
        jz      @f
        cmp     al, ah
        jnz     .not_empty
        dec     ecx
        jnz     .not_empty
@@:
        mov     cx, [ebx+DIRENTRY.entryLength]
        jecxz   @f
        add     ebx, ecx
        cmp     ebx, edx
        jb      .dir_entry
@@:
        pop     ecx
        inc     ecx
        jmp     .checkDirectory

.not_empty:
        pop     eax eax
        push    ERROR_ACCESS_DENIED
        jmp     .ret

.error8:
        pop     ebx
.error4:
        pop     ebx
.error:
        push    eax
        jmp     .ret

.empty:
        pop     ecx ecx
        cmp     eax, ERROR_END_OF_FILE
        jnz     .error
        push    ecx
.file:
        mov     eax, ecx
        call    unlinkInode
        jc      .error4
        pop     eax
        lea     ebx, [ebp+EXTFS.inodeBuffer]
        cmp     edi, DIRECTORY
        jnz     @f
        dec     [ebx+INODE.linksCount]
        call    writeInode
@@:
        mov     eax, esi
        call    readInode
        jc      .error
        dec     [ebx+INODE.linksCount]
        jz      @f
        cmp     edi, DIRECTORY
        jnz     .hardlinks
@@:
        push    esi edi
        xor     eax, eax
        xor     edx, edx
        call    extfsTruncateFile   ; free file's data
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        lea     edi, [ebp+EXTFS.inodeBuffer]
        xor     eax, eax
        push    edi
        rep stosb
        call    fsGetTime
        pop     ebx edi esi
        add     eax, 978307200
        mov     [ebx+INODE.deletedTime], eax
        mov     eax, esi
        dec     eax
        xor     edx, edx
        div     [ebp+EXTFS.superblock.inodesPerGroup]
        push    edx
        mov     ebx, [ebp+EXTFS.descriptorTable]
        shl     eax, 5
        add     ebx, eax
        cmp     edi, DIRECTORY
        jnz     @f
        dec     [ebx+BGDESCR.directoriesCount]
@@:
        inc     [ebx+BGDESCR.inodesFree]
        push    [ebx+BGDESCR.inodeBitmap]
        call    extfsWriteDescriptor
        pop     eax
        mov     ebx, [ebp+EXTFS.tempBlockBuffer]
        mov     ecx, eax
        call    extfsReadBlock
        pop     edx
        jc      .error
        mov     eax, edx
        and     edx, 31
        shr     eax, 5
        shl     eax, 2
        add     eax, ebx
        btr     [eax], edx
        mov     eax, ecx
        call    extfsWriteBlock
        inc     [ebp+EXTFS.superblock.inodesFree]
.hardlinks:
        mov     eax, esi
        lea     ebx, [ebp+EXTFS.inodeBuffer]
        call    writeInode
        push    eax
        call    writeSuperblock
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
.ret:
        call    ext_unlock
        xor     ebx, ebx
        pop     eax
        ret

;----------------------------------------------------------------
ext_CreateFolder:
        call    extfsWritingInit
        call    findInode
        jnc     .success    ; exist
        test    edi, edi
        jz      .error
        mov     eax, esi
        call    extfsInodeAlloc
        jc      .error
        push    ebx esi edi
        lea     edi, [ebp+EXTFS.inodeBuffer]
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        xor     eax, eax
        rep stosb
        call    fsGetTime
        add     eax, 978307200
        lea     ebx, [ebp+EXTFS.inodeBuffer]
        mov     [ebx+INODE.accessedTime], eax
        mov     [ebx+INODE.dataModified], eax
        pop     edi esi edx
; edx = allocated inode number, edi -> filename, esi = parent inode number
        mov     [ebx+INODE.accessMode], DIRECTORY or 511
        mov     byte [ebx+INODE.linksCount], 2
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
        push    esi
        mov     eax, esi
        mov     ebx, edx
        mov     esi, edi
        mov     dl, DIR_DIRECTORY
        call    linkInode
        pop     edx
        jc      .error
        push    ebx
        lea     ebx, [ebp+EXTFS.inodeBuffer]
        inc     [ebx+INODE.linksCount]
        mov     eax, edx
        call    writeInode
        pop     ebx
        jc      .error
        mov     eax, ebx
        dec     eax
        xor     edx, edx
        div     [ebp+EXTFS.superblock.inodesPerGroup]
        mov     ebx, [ebp+EXTFS.descriptorTable]
        shl     eax, 5
        add     ebx, eax
        inc     [ebx+BGDESCR.directoriesCount]
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

self_link   db ".", 0
parent_link db "..", 0

;----------------------------------------------------------------
ext_CreateFile:
        call    extfsWritingInit
        pushd   0 0 ebx
        call    findInode
        jnc     .exist
        test    edi, edi
        jz      .error
        mov     eax, esi
        call    extfsInodeAlloc
        jc      .error
        push    ebx ebx esi edi
        lea     edi, [ebp+EXTFS.inodeBuffer]
        movzx   ecx, [ebp+EXTFS.superblock.inodeSize]
        xor     eax, eax
        rep stosb
        call    fsGetTime
        add     eax, 978307200
        lea     ebx, [ebp+EXTFS.inodeBuffer]
        mov     [ebx+INODE.accessedTime], eax
        mov     [ebx+INODE.dataModified], eax
        pop     edi esi edx
; edx = allocated inode number, edi -> filename, esi = parent inode number
        mov     [ebx+INODE.accessMode], FLAG_FILE or 110110110b
        mov     byte [ebx+INODE.linksCount], 1
        mov     eax, edx
        call    writeInode
        jc      .error2
; link parent to child
        mov     eax, esi
        mov     ebx, edx
        mov     esi, edi
        mov     dl, DIR_FLAG_FILE
        call    linkInode
        jc      .error2
        mov     eax, ebx
        lea     ebx, [ebp+EXTFS.inodeBuffer]
        call    readInode
        jc      .error2
        pop     esi ebx
        mov     eax, [ebx+12]
        xor     edx, edx
        jmp     ext_WriteFile.start

.exist:
        movi    eax, ERROR_ACCESS_DENIED
        test    [ebp+EXTFS.inodeBuffer.accessMode], FLAG_FILE
        jz      .error  ; not a file
        pop     ebx
        mov     eax, [ebx+12]
        xor     edx, edx
        push    eax edx ebx esi
        call    extfsTruncateFile
        pop     esi ebx edx eax
        jmp     ext_WriteFile.start

.error2:
        pop     ebx
.error:
        push    eax
        call    ext_unlock
        pop     eax ebx ebx ebx
        ret

;----------------------------------------------------------------
ext_WriteFile:
        call    extfsWritingInit
        call    findInode
        pushd   0 eax
        jc      .ret
        mov     byte [esp], ERROR_ACCESS_DENIED
        test    [ebp+EXTFS.inodeBuffer.accessMode], FLAG_FILE
        jz      .ret    ; not a file
        mov     byte [esp], 0
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        add     eax, [ebx+12]
        adc     edx, 0
.start:
        push    esi
        mov     ecx, esi
        call    extfsExtendFile
        jc      .errorExtend
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        mov     ecx, [ebx+12]
        mov     esi, [ebx+16]
.write:
        jecxz   .zero
        div     [ebp+EXTFS.bytesPerBlock]
        test    edx, edx
        jz      .aligned
.piece:
        mov     ebx, ecx
        mov     edi, edx
        mov     ecx, eax
        push    eax
        call    extfsGetExtent
        jc      .errorGet
        mov     ecx, [ebp+EXTFS.sectorsPerBlock]
        mul     ecx
        push    ecx eax ebx
        mov     ebx, [ebp+EXTFS.mainBlockBuffer]
        call    fs_read64_sys
        test    eax, eax
        jnz     .errorDevice
        pop     eax
        mov     ecx, [ebp+EXTFS.bytesPerBlock]
        sub     ecx, edi
        sub     eax, ecx
        jnc     @f
        add     ecx, eax
        xor     eax, eax
@@:
        add     edi, ebx
        add     [esp+20], ecx
        rep movsb
        mov     edi, eax
        pop     eax ecx
        xor     edx, edx
        call    fs_write64_sys
        mov     ecx, edi
        pop     eax
        inc     eax
        xor     edx, edx
.zero:
        jecxz   .done
.aligned:
        xchg    eax, ecx
        div     [ebp+EXTFS.bytesPerBlock]
        push    edx
        mov     edx, eax
.writeExtent:
        test    edx, edx
        jz      .end
        push    ecx
        call    extfsGetExtent
        jc      .errorGet2
        sub     edx, ecx
        jnc     @f
        add     ecx, edx
        xor     edx, edx
@@:
        add     [esp], ecx
        imul    ecx, [ebp+EXTFS.sectorsPerBlock]
        mov     ebx, esi
        push    edx ecx
        mul     [ebp+EXTFS.sectorsPerBlock]
        call    fs_write64_sys
        test    eax, eax
        jnz     .errorDevice
        pop     ebx edx ecx
        shl     ebx, 9
        add     esi, ebx
        add     [esp+12], ebx
        jmp     .writeExtent

.end:
        mov     eax, ecx
        pop     ecx
        jecxz   .done
        jmp     .piece

.errorDevice:
        pop     eax eax
        movi    eax, ERROR_DEVICE
.errorGet2:
        pop     ebx
.errorGet:
        pop     ebx
.errorExtend:
        mov     [esp+4], eax
.done:
        lea     ebx, [ebp+EXTFS.inodeBuffer]
        pop     eax
        call    writeInode
        add     [esp], eax
        call    writeSuperblock
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
.ret:
        call    ext_unlock
        pop     eax ebx
        ret

.erase:
        push    eax eax edi
        mov     eax, ebx
        jmp     .write

;----------------------------------------------------------------
ext_SetFileEnd:
        call    extfsWritingInit
        call    findInode
        jc      .error2
        lea     edi, [ebp+EXTFS.inodeBuffer]
        movi    eax, ERROR_ACCESS_DENIED
        test    [edi+INODE.accessMode], FLAG_FILE
        jz      .error2 ; not a file
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        mov     ebx, [edi+INODE.fileSize]
        mov     ecx, [edi+INODE.fileSizeHigh]
        push    esi ecx
        cmp     ebx, eax
        sbb     ecx, edx
        mov     ecx, esi
        jnc     @f
        call    extfsExtendFile
        pop     esi
        jc      .error
        mov     eax, [edi+INODE.fileSize]
        mov     edx, [edi+INODE.fileSizeHigh]
        sub     eax, ebx
        sbb     edx, esi
        jnz     .done
        cmp     eax, 1000001h
        jnc     .done
        push    eax
        stdcall kernel_alloc, eax
        pop     ecx
        test    eax, eax
        jz      .error
        push    ecx
        add     ecx, 3
        shr     ecx, 2
        mov     edx, esi
        mov     esi, eax
        mov     edi, eax
        xor     eax, eax
        rep stosd
        pop     ecx edi
        push    esi
        call    ext_WriteFile.erase
        call    kernel_free
        xor     eax, eax
        ret

@@:
        call    extfsTruncateFile
        pop     eax
.done:
        xor     eax, eax
.error:
        xchg    eax, [esp]
        lea     ebx, [ebp+EXTFS.inodeBuffer]
        call    writeInode
        add     [esp], eax
        call    writeSuperblock
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
        pop     eax
.error2:
        push    eax
        call    ext_unlock
        pop     eax
        ret

;----------------------------------------------------------------
ext_Rename:
        call    extfsWritingInit
        push    esi
        mov     esi, edi
        call    findInode
        jnc     .error
        test    edi, edi
        jz      .error
        xchg    [esp], esi
        push    edi
        call    findInode
        pop     edi
        jc      .error
        xor     edx, edx
        inc     edx
        test    [ebp+EXTFS.inodeBuffer.accessMode], DIRECTORY
        jz      @f
        inc     edx
@@:
        mov     eax, ecx
        push    ecx edx
        call    unlinkInode
        pop     edx ecx
        jc      .error
        cmp     edx, 1
        jz      @f
        lea     ebx, [ebp+EXTFS.inodeBuffer]
        dec     [ebx+INODE.linksCount]
        mov     eax, ecx
        call    writeInode
        jc      .error
@@:
        mov     ebx, esi
        mov     esi, edi
        pop     eax
        push    eax edx
        call    linkInode
        pop     edx
        jc      .error
        pop     eax
        cmp     edx, 1
        jz      @f
        lea     ebx, [ebp+EXTFS.inodeBuffer]
        inc     [ebx+INODE.linksCount]
        call    writeInode
@@:
        call    writeSuperblock
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
        call    ext_unlock
        xor     eax, eax
        ret

.error:
        push    eax
        call    ext_unlock
        pop     eax ebx
        ret
