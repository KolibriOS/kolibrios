;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; NTFS external functions
;   in:
; ebx -> parameter structure of sysfunc 70
; ebp -> NTFS structure
; esi -> path string in UTF-8
;   out:
; eax, ebx = return values for sysfunc 70
iglobal
align 4
ntfs_user_functions:
        dd      ntfs_free
        dd      (ntfs_user_functions_end - ntfs_user_functions - 8) / 4
        dd      ntfs_ReadFile
        dd      ntfs_ReadFolder
        dd      ntfs_CreateFile
        dd      ntfs_WriteFile
        dd      ntfs_SetFileEnd
        dd      ntfs_GetFileInfo
        dd      ntfs_SetFileInfo
        dd      0
        dd      ntfs_Delete
        dd      ntfs_CreateFolder
ntfs_user_functions_end:
endg

; Basic concepts:
; File is a FileRecord in the $MFT.
; $MFT is a file, that consists of FileRecords and starts with FileRecord of itself.
; FileRecord (FILE) consists of a header and attributes.
; Attribute consists of a header and a body.
; Attribute's body can be inside (resident) or outside of FileRecord.
; File's data is a body of $Data (80h) attribute.
; FileRecords is a data of the $MFT file.
; Directory is a file, that consists of index nodes.
; Resident index node is always located in a body of $IndexRoot (90h) attribute.
; Body of $IndexAllocation (A0h) attribute is always non resident
;  and consists of IndexRecords.
; IndexRecord (INDX) consists of a header and an index node.
; Index node consists of a header and indexes.
; Index consists of a header and a copy of indexed attribute's body.
; Directories index $Filename (30h) attribute of all existing files.
; $IndexRoot and $IndexAllocation attributes of a directory has a name — $I30.

; Offsets:
    ; record header
magic = 0
updateSequenceOffset = 4
updateSequenceSize = 6
    ; FileRecord header
reuseCounter = 16
hardLinkCounter = 12h
attributeOffset = 14h
recordFlags = 16h
recordRealSize = 18h
recordAllocatedSize = 1ch
baseRecordReference = 20h       ; for auxiliary records
baseRecordReuse = 26h
newAttributeID = 28h
    ; attribute header
attributeType = 0
sizeWithHeader = 4
nonResidentFlag = 8
nameLength = 9
nameOffset = 10
attributeFlags = 12
attributeID = 14
    ; resident attribute header
sizeWithoutHeader = 10h
attributeOffset = 14h
indexedFlag = 16h
    ; non resident attribute header
firstVCN = 10h
lastVCN = 18h
dataRunsOffset = 20h
attributeAllocatedSize = 28h
attributeRealSize = 30h
initialDataSize = 38h
    ; $IndexRoot
indexedAttributesType = 0
collationRule = 4
indexRecordSize = 8
indexRecordSizeClus = 12        ; in sectors if less than one cluster
rootNode = 16
    ; IndexRecord header
recordVCN = 16
recordNode = 18h
    ; node header
indexOffset = 0
nodeRealSize = 4
nodeAllocatedSize = 8
nonLeafFlag = 12
    ; $Filename index
fileRecordReference = 0
fileReferenceReuse = 6
indexAllocatedSize = 8
indexRawSize = 10
indexFlags = 12
directoryRecordReference = 16
directoryReferenceReuse = 16h
fileCreated = 18h
fileModified = 20h
recordModified = 28h
fileAccessed = 30h
fileAllocatedSize = 38h
fileRealSize = 40h
fileFlags = 48h
fileNameLength = 50h
namespace = 51h
fileName = 52h

struct NTFS PARTITION
Lock                MUTEX   ; Currently operations with one partition
; can not be executed in parallel since the legacy code is not ready.
sectors_per_cluster dd  ?
mft_cluster         dd  ?   ; location
mftmirr_cluster     dd  ?   ; location
frs_size            dd  ?   ; in bytes
frs_buffer          dd  ?   ; MFT fileRecord buffer
mft_retrieval_end   dd  ?
mftSize             dd  ?   ; in sectors
cur_index_size      dd  ?   ; in sectors
cur_index_buf       dd  ?   ; index node buffer
secondIndexBuffer   dd  ?
BitmapBuffer        dd  ?
BitmapTotalSize     dd  ?   ; bytes reserved
BitmapSize          dd  ?   ; bytes readen
BitmapLocation      dd  ?   ; starting sector
BitmapStart         dd  ?   ; first byte after area, reserved for MFT
mftBitmapBuffer     dd  ?   ; one cluster
mftBitmapSize       dd  ?   ; bytes readen
mftBitmapLocation   dd  ?   ; starting sector

attr_size           dq  ?
attr_offs           dd  ?
attr_list           dd  ?
attr_iBaseRecord    dd  ?
cur_attr            dd  ?   ; attribute type
cur_iRecord         dd  ?   ; number of fileRecord in MFT
cur_offs            dd  ?   ; attribute VCN in sectors
cur_size            dd  ?   ; max sectors to read
cur_buf             dd  ?
cur_read            dd  ?   ; bytes readen
cur_tail            dd  ?
cur_subnode_size    dd  ?
LastRead            dd  ?   ; last readen block of sectors
mftLastRead         dd  ?
rootLastRead        dd  ?
nodeLastRead        dd  ?
indexRoot           dd  ?
indexPointer        dd  ?
newRecord           dd  ?
fileDataStart       dd  ?   ; starting cluster
fileDataSize        dd  ?   ; in clusters
fileDataBuffer      dd  ?
fileRealSize        dd  ?   ; in bytes
fragmentCount       db  ?
bCanContinue        db  ?
bFolder             db  ?
bWriteAttr          db  ?   ; Warning: Don't forget to turn off!!!

mft_retrieval       rb  512
align0  rb  1024-NTFS.align0
attrlist_buf        rb  1024
attrlist_mft_buf    rb  1024
bitmap_buf          rb  1024
ends

ntfs_test_bootsec:
; in: ebx -> buffer, edx = size of partition
; out: CF=1 -> invalid
; 1. Name=='NTFS    '
        cmp     dword [ebx+3], 'NTFS'
        jnz     .no
        cmp     dword [ebx+7], '    '
        jnz     .no
; 2. Number of bytes per sector is the same as for physical device
; (that is, 0x200 for hard disk)
        cmp     word [ebx+11], 0x200
        jnz     .no
; 3. Number of sectors per cluster must be power of 2
        movzx   eax, byte [ebx+13]
        dec     eax
        js      .no
        test    al, [ebx+13]
        jnz     .no
; 4. FAT parameters must be zero
        cmp     word [ebx+14], 0
        jnz     .no
        cmp     dword [ebx+16], 0
        jnz     .no
        cmp     byte [ebx+20], 0
        jnz     .no
        cmp     word [ebx+22], 0
        jnz     .no
        cmp     dword [ebx+32], 0
        jnz     .no
; 5. Number of sectors <= partition size
        cmp     dword [ebx+0x2C], 0
        ja      .no
        cmp     [ebx+0x28], edx
        ja      .no
; 6. $MFT and $MFTMirr clusters must be within partition
        cmp     dword [ebx+0x34], 0
        ja      .no
        push    edx
        movzx   eax, byte [ebx+13]
        mul     dword [ebx+0x30]
        test    edx, edx
        pop     edx
        jnz     .no
        cmp     eax, edx
        ja      .no
        cmp     dword [ebx+0x3C], 0
        ja      .no
        push    edx
        movzx   eax, byte [ebx+13]
        mul     dword [ebx+0x38]
        test    edx, edx
        pop     edx
        jnz     .no
        cmp     eax, edx
        ja      .no
; 7. Clusters per FRS must be either power of 2 or between -31 and -9
        movsx   eax, byte [ebx+0x40]
        cmp     al, -31
        jl      .no
        cmp     al, -9
        jle     @f
        dec     eax
        js      .no
        test    [ebx+0x40], al
        jnz     .no
@@:         ; 8. Same for clusters per IndexAllocationBuffer
        movsx   eax, byte [ebx+0x44]
        cmp     al, -31
        jl      .no
        cmp     al, -9
        jle     @f
        dec     eax
        js      .no
        test    [ebx+0x44], al
        jnz     .no
@@:         ; OK, this is correct NTFS bootsector
        clc
        ret
.no:        ; No, this bootsector isn't NTFS
        stc
        ret

; Mount if it's a valid NTFS partition.
ntfs_create_partition:
;   in:
; ebp -> PARTITION structure
; ebx -> boot sector
; ebx+512 -> buffer
;   out:
; eax -> NTFS structure, 0 = not NTFS
        cmp     dword [esi+DISK.MediaInfo.SectorSize], 512
        jnz     .nope
        mov     edx, dword [ebp+PARTITION.Length]
        cmp     dword [esp+4], 0
        jz      .boot_read_ok
        add     ebx, 512
        lea     eax, [edx-1]
        call    fs_read32_sys
        test    eax, eax
        jnz     @f
        call    ntfs_test_bootsec
        jnc     .ntfs_setup
@@:
        mov     eax, edx
        shr     eax, 1
        call    fs_read32_sys
        test    eax, eax
        jnz     .nope
.boot_read_ok:
        call    ntfs_test_bootsec
        jnc     .ntfs_setup
.nope:
        xor     eax, eax
        jmp     .exit

.ntfs_setup:    ; By given bootsector, initialize some NTFS variables
        stdcall kernel_alloc, 1000h
        test    eax, eax
        jz      .exit
        mov     ecx, dword [ebp+PARTITION.FirstSector]
        mov     dword [eax+NTFS.FirstSector], ecx
        mov     ecx, dword [ebp+PARTITION.FirstSector+4]
        mov     dword [eax+NTFS.FirstSector+4], ecx
        mov     ecx, [ebp+PARTITION.Disk]
        mov     [eax+NTFS.Disk], ecx
        mov     [eax+NTFS.FSUserFunctions], ntfs_user_functions
        mov     [eax+NTFS.bWriteAttr], 0

        push    ebx ebp esi
        mov     ebp, eax
        lea     ecx, [ebp+NTFS.Lock]
        call    mutex_init
        movzx   eax, byte [ebx+13]
        mov     [ebp+NTFS.sectors_per_cluster], eax
        mov     eax, [ebx+0x28]
        mov     dword [ebp+NTFS.Length], eax
        and     dword [ebp+NTFS.Length+4], 0
        mov     eax, [ebx+0x30]
        mov     [ebp+NTFS.mft_cluster], eax
        mov     eax, [ebx+0x38]
        mov     [ebp+NTFS.mftmirr_cluster], eax
        movsx   eax, byte [ebx+0x40]
        test    eax, eax
        js      @f
        mul     [ebp+NTFS.sectors_per_cluster]
        shl     eax, 9
        jmp     .1

@@:
        neg     eax
        mov     ecx, eax
        mov     eax, 1
        shl     eax, cl
.1:
        mov     [ebp+NTFS.frs_size], eax
        stdcall kernel_alloc, eax
        test    eax, eax
        jz      .fail_free
        mov     [ebp+NTFS.frs_buffer], eax
; read $MFT disposition
        mov     eax, [ebp+NTFS.mft_cluster]
        mul     [ebp+NTFS.sectors_per_cluster]
        mov     ecx, [ebp+NTFS.frs_size]
        shr     ecx, 9
        mov     ebx, [ebp+NTFS.frs_buffer]
        call    fs_read64_sys
        test    eax, eax
        jnz     .usemirr
        cmp     dword [ebx], 'FILE'
        jnz     .usemirr
        call    ntfs_restore_usa_frs
        jnc     .mftok
.usemirr:
        mov     eax, [ebp+NTFS.mftmirr_cluster]
        mul     [ebp+NTFS.sectors_per_cluster]
        mov     ecx, [ebp+NTFS.frs_size]
        shr     ecx, 9
        mov     ebx, [ebp+NTFS.frs_buffer]
        call    fs_read64_sys
        test    eax, eax
        jnz     .fail_free_frs
        cmp     dword [ebx], 'FILE'
        jnz     .fail_free_frs
        call    ntfs_restore_usa_frs
        jc      .fail_free_frs
.mftok:     ; prepare $MFT retrieval information
; search for unnamed non-resident $DATA attribute
        movzx   eax, word [ebx+attributeOffset]
        add     eax, ebx
.scandata:
        cmp     dword [eax], -1
        jz      .fail_free_frs
        cmp     dword [eax], 0x80
        jnz     @f
        cmp     byte [eax+nameLength], 0
        jz      .founddata
@@:
        add     eax, [eax+sizeWithHeader]
        jmp     .scandata

.founddata:
        cmp     byte [eax+nonResidentFlag], 0
        jz      .fail_free_frs
        movzx   esi, word [eax+dataRunsOffset]
        add     esi, eax
        mov     edx, [eax+attributeAllocatedSize+4]
        mov     eax, [eax+attributeAllocatedSize]
        shrd    eax, edx, 9
        mov     [ebp+NTFS.mftSize], eax
        sub     esp, 10h
        lea     ecx, [ebp+NTFS.mft_retrieval]
        xor     edx, edx
.scanmcb:   ; load descriptions of fragments
        call    ntfs_decode_mcb_entry
        jnc     .scanmcbend
        mov     eax, [esp]      ; block length
        mov     [ecx], eax
        add     edx, [esp+8]    ; block addr
        mov     [ecx+4], edx
        add     ecx, 8
        jmp     .scanmcb

.scanmcbend:
        add     esp, 10h
        lea     eax, [ebp+NTFS.attrlist_buf]
        cmp     eax, ecx
        jc      @f
        mov     eax, ecx
@@:
        mov     [ebp+NTFS.mft_retrieval_end], eax
; allocate index buffers
        stdcall kernel_alloc, 2000h
        test    eax, eax
        jz      .fail_free_frs
        mov     [ebp+NTFS.cur_index_buf], eax
        add     eax, 1000h
        mov     [ebp+NTFS.secondIndexBuffer], eax
        mov     [ebp+NTFS.cur_index_size], 8
; reserve adress space for bitmap buffer and load some part of bitmap
        mov     eax, dword [ebp+NTFS.Length]
        xor     edx, edx
        div     [ebp+NTFS.sectors_per_cluster]
        shr     eax, 3
        mov     [ebp+NTFS.BitmapTotalSize], eax
        add     eax, 7FFFh
        and     eax, not 7FFFh
        push    eax
        call    alloc_kernel_space
        test    eax, eax
        jz      .failFreeIndex
        mov     [ebp+NTFS.BitmapBuffer], eax
        mov     [ebp+NTFS.cur_buf], eax
        mov     eax, [ebp+NTFS.BitmapTotalSize]
        add     eax, [ebp+NTFS.mft_cluster]
        shr     eax, 3+2        ; reserve 1/8 of partition for $MFT
        shl     eax, 2
        mov     [ebp+NTFS.BitmapStart], eax
        shr     eax, 15
        inc     eax
        shl     eax, 3
        push    eax
        push    eax
        shl     eax, 3
        mov     [ebp+NTFS.cur_size], eax
        call    alloc_pages
        test    eax, eax
        pop     ecx
        jz      .failFreeBitmap
        add     eax, 3
        mov     ebx, [ebp+NTFS.BitmapBuffer]
        call    commit_pages
        mov     [ebp+NTFS.cur_iRecord], 6
        mov     [ebp+NTFS.cur_attr], 0x80
        mov     [ebp+NTFS.cur_offs], 0
        call    ntfs_read_attr
        jc      .failFreeBitmap
        mov     eax, [ebp+NTFS.cur_read]
        mov     [ebp+NTFS.BitmapSize], eax
        mov     eax, [ebp+NTFS.LastRead]
        mov     [ebp+NTFS.BitmapLocation], eax
; read MFT $BITMAP attribute
        mov     eax, [ebp+NTFS.sectors_per_cluster]
        mov     [ebp+NTFS.cur_size], eax
        shl     eax, 9
        stdcall kernel_alloc, eax
        test    eax, eax
        jz      .failFreeBitmap
        mov     [ebp+NTFS.mftBitmapBuffer], eax
        mov     [ebp+NTFS.cur_buf], eax
        mov     [ebp+NTFS.cur_iRecord], 0
        mov     [ebp+NTFS.cur_attr], 0xB0
        mov     [ebp+NTFS.cur_offs], 0
        call    ntfs_read_attr
        mov     eax, [ebp+NTFS.cur_read]
        cmp     eax, 4
        jc      .failFreeBitmapMFT
        mov     ecx, [ebp+NTFS.attr_offs]
        cmp     byte [ecx+nonResidentFlag], 1
        jnz     .failFreeBitmapMFT
        mov     [ebp+NTFS.mftBitmapSize], eax
        mov     eax, [ebp+NTFS.LastRead]
        mov     [ebp+NTFS.mftBitmapLocation], eax

        mov     eax, ebp
.pop_exit:
        pop     esi ebp ebx
.exit:
        cmp     dword [esp+4], 0
        jz      @f
        sub     ebx, 512
@@:
        ret

.failFreeBitmapMFT:
        stdcall kernel_free, [ebp+NTFS.mftBitmapBuffer]
.failFreeBitmap:
        stdcall kernel_free, [ebp+NTFS.BitmapBuffer]
.failFreeIndex:
        mov     eax, [ebp+NTFS.cur_index_buf]
        cmp     eax, [ebp+NTFS.secondIndexBuffer]
        jc      @f
        mov     eax, [ebp+NTFS.secondIndexBuffer]
@@:
        stdcall kernel_free, eax
.fail_free_frs:
        stdcall kernel_free, [ebp+NTFS.frs_buffer]
.fail_free:
        stdcall kernel_free, ebp
        xor     eax, eax
        jmp     .pop_exit

ntfs_free:
        push    ebx
        mov     ebx, eax
        stdcall kernel_free, [ebx+NTFS.frs_buffer]
        stdcall kernel_free, [ebx+NTFS.mftBitmapBuffer]
        stdcall kernel_free, [ebx+NTFS.BitmapBuffer]
        mov     eax, [ebx+NTFS.cur_index_buf]
        cmp     eax, [ebx+NTFS.secondIndexBuffer]
        jc      @f
        mov     eax, [ebx+NTFS.secondIndexBuffer]
@@:
        stdcall kernel_free, eax
        stdcall kernel_free, ebx
        pop     ebx
        ret

ntfs_lock:
        lea     ecx, [ebp+NTFS.Lock]
        jmp     mutex_lock

ntfs_unlock:
        lea     ecx, [ebp+NTFS.Lock]
        jmp     mutex_unlock

ntfs_read_attr:
; [ebp+NTFS.bWriteAttr]=1 -> write attribute
;   in:
; [ebp+NTFS.cur_iRecord] = number of fileRecord
; [ebp+NTFS.cur_attr] = attribute type
; [ebp+NTFS.cur_offs] = attribute VCN in sectors
; [ebp+NTFS.cur_buf] -> buffer for data
; [ebp+NTFS.cur_size] = max sectors to read
;   out:
; [ebp+NTFS.cur_read] = bytes readen
; CF=1 -> failed, eax = disk error code, eax=0 -> something with FS
        xor     eax, eax
        pushad
        and     [ebp+NTFS.cur_read], 0
        cmp     [ebp+NTFS.cur_iRecord], 0
        jnz     .nomft
        cmp     [ebp+NTFS.cur_attr], 0x80
        jnz     .nomft
; precalculated part of $Mft $DATA
        mov     eax, [ebp+NTFS.cur_offs]
        xor     edx, edx
        div     [ebp+NTFS.sectors_per_cluster]
        mov     ebx, edx
        mov     [ebp+NTFS.fragmentCount], 0
; eax = VCN, ebx = offset in sectors from beginning of cluster
        lea     esi, [ebp+NTFS.mft_retrieval]
        sub     esi, 8
.mftscan:
        add     esi, 8
        cmp     esi, [ebp+NTFS.mft_retrieval_end]
        jz      .nomft
        mov     ecx, [esi+4]
        sub     eax, [esi]
        jnc     .mftscan
        add     ecx, eax
        add     ecx, [esi]
        neg     eax
        mul     [ebp+NTFS.sectors_per_cluster]
        xchg    eax, ecx
        mul     [ebp+NTFS.sectors_per_cluster]
        sub     ecx, ebx
        add     eax, ebx
        mov     ebx, [ebp+NTFS.cur_buf]
        cmp     ecx, [ebp+NTFS.cur_size]
        jb      @f
        mov     ecx, [ebp+NTFS.cur_size]
@@:
        mov     [ebp+NTFS.LastRead], eax
        mov     edi, ecx
        call    fs_read64_sys
        test    eax, eax
        jnz     .errret
        sub     [ebp+NTFS.cur_size], edi
        add     [ebp+NTFS.cur_offs], edi
        shl     edi, 9
        add     [ebp+NTFS.cur_read], edi
        add     [ebp+NTFS.cur_buf], edi
        inc     [ebp+NTFS.fragmentCount]
        xor     eax, eax
        xor     ebx, ebx
        cmp     [ebp+NTFS.cur_size], eax
        jz      @f
        jmp     .mftscan

.errret2_pop:
        xor     eax, eax
.errret_pop:
        pop     ecx
        pop     ecx
.errret:
        mov     [esp+28], eax
        stc
@@:
        popad
        ret

.nomft:
; 1. Read file record.
; N.B. This will do recursive call of read_attr for $MFT::$Data.
        mov     eax, [ebp+NTFS.cur_iRecord]
        and     [ebp+NTFS.attr_list], 0
        or      dword [ebp+NTFS.attr_size+4], -1
        or      [ebp+NTFS.attr_iBaseRecord], -1
        call    ntfs_read_file_record
        jc      .errret
; 2. Find required attribute.
        mov     eax, [ebp+NTFS.frs_buffer]
; a) For auxiliary records, read base record.
; If base record is present, base iRecord may be 0 (for $Mft),
; but SequenceNumber is nonzero.
        cmp     word [eax+baseRecordReuse], 0
        jz      @f
        mov     eax, [eax+baseRecordReference]
.beginfindattr:
        call    ntfs_read_file_record
        jc      .errret
        jmp     @f

.newAttribute:
        pushad
        and     [ebp+NTFS.cur_read], 0
@@:
; b) Scan for required attribute and for $ATTR_LIST
        mov     eax, [ebp+NTFS.frs_buffer]
        movzx   ecx, word [eax+attributeOffset]
        add     eax, ecx
        mov     ecx, [ebp+NTFS.cur_attr]
        and     [ebp+NTFS.attr_offs], 0
.scanattr:
        cmp     dword [eax], -1
        jz      .scandone
        cmp     dword [eax], ecx
        jz      .okattr
        cmp     [ebp+NTFS.attr_iBaseRecord], -1
        jnz     .scancont
        cmp     dword [eax], 0x20       ; $ATTR_LIST
        jnz     .scancont
        mov     [ebp+NTFS.attr_list], eax
        jmp     .scancont

.okattr:
; ignore named $DATA attributes (aka NTFS streams)
        cmp     ecx, 0x80
        jnz     @f
        cmp     byte [eax+nameLength], 0
        jnz     .scancont
@@:
        mov     [ebp+NTFS.attr_offs], eax
.scancont:
        add     eax, [eax+sizeWithHeader]
        jmp     .scanattr

.continue:
        pushad
        and     [ebp+NTFS.cur_read], 0
.scandone:
; c) Check for required offset and length
        mov     ecx, [ebp+NTFS.attr_offs]
        jecxz   .noattr
        push    [ebp+NTFS.cur_size]
        push    [ebp+NTFS.cur_read]
        call    .doreadattr
        pop     edx
        pop     ecx
        jc      .ret
        cmp     [ebp+NTFS.bCanContinue], 0
        jz      .ret
        sub     edx, [ebp+NTFS.cur_read]
        neg     edx
        shr     edx, 9
        sub     ecx, edx
        mov     [ebp+NTFS.cur_size], ecx
        jz      .ret
.noattr:
        cmp     [ebp+NTFS.cur_attr], 0x20
        jz      @f
        mov     ecx, [ebp+NTFS.attr_list]
        test    ecx, ecx
        jnz     .lookattr
        and     dword [esp+28], 0
        cmp     [ebp+NTFS.attr_offs], 1     ; define CF
.ret:
        popad
        ret

.lookattr:
; required attribute or required offset was not found in base record;
; it may be present in auxiliary records;
; scan $ATTR_LIST
        mov     eax, [ebp+NTFS.attr_iBaseRecord]
        cmp     eax, -1
        jz      @f
        call    ntfs_read_file_record
        jc      .errret
        or      [ebp+NTFS.attr_iBaseRecord], -1
@@:
        push    [ebp+NTFS.cur_offs]
        push    [ebp+NTFS.cur_size]
        push    [ebp+NTFS.cur_read]
        push    [ebp+NTFS.cur_buf]
        push    dword [ebp+NTFS.attr_size]
        push    dword [ebp+NTFS.attr_size+4]
        or      dword [ebp+NTFS.attr_size+4], -1
        and     [ebp+NTFS.cur_offs], 0
        mov     [ebp+NTFS.cur_size], 2
        and     [ebp+NTFS.cur_read], 0
        lea     eax, [ebp+NTFS.attrlist_buf]
        cmp     [ebp+NTFS.cur_iRecord], 0
        jnz     @f
        lea     eax, [ebp+NTFS.attrlist_mft_buf]
@@:
        mov     [ebp+NTFS.cur_buf], eax
        push    eax
        call    .doreadattr
        pop     esi
        mov     edx, 1
        pop     dword [ebp+NTFS.attr_size+4]
        pop     dword [ebp+NTFS.attr_size]
        mov     ecx, [ebp+NTFS.cur_read]
        pop     [ebp+NTFS.cur_buf]
        pop     [ebp+NTFS.cur_read]
        pop     [ebp+NTFS.cur_size]
        pop     [ebp+NTFS.cur_offs]
        jc      .errret
        or      edi, -1
        lea     ecx, [ecx+esi-1Ah]
.scanliststart:
        push    ecx
        mov     eax, [ebp+NTFS.cur_attr]
.scanlist:
        cmp     esi, [esp]
        jae     .scanlistdone
        cmp     eax, [esi]
        jz      @f
.scanlistcont:
        movzx   ecx, word [esi+4]
        add     esi, ecx
        jmp     .scanlist

@@:
; ignore named $DATA attributes (aka NTFS streams)
        cmp     eax, 0x80
        jnz     @f
        cmp     byte [esi+6], 0
        jnz     .scanlistcont
@@:
        push    eax
        mov     eax, [esi+8]
        test    eax, eax
        jnz     .testf
        cmp     dword [ebp+NTFS.attr_size+4], -1
        jnz     .testfz
; if attribute is in auxiliary records, its size is defined only in first
        mov     eax, [esi+10h]
        call    ntfs_read_file_record
        jc      .errret_pop
        mov     eax, [ebp+NTFS.frs_buffer]
        movzx   ecx, word [eax+14h]
        add     eax, ecx
        mov     ecx, [ebp+NTFS.cur_attr]
@@:
        cmp     dword [eax], -1
        jz      .errret2_pop
        cmp     dword [eax], ecx
        jz      @f
.l1:
        add     eax, [eax+4]
        jmp     @b

@@:
        cmp     eax, 0x80
        jnz     @f
        cmp     byte [eax+9], 0
        jnz     .l1
@@:
        cmp     byte [eax+8], 0
        jnz     .sdnores
        mov     eax, [eax+10h]
        mov     dword [ebp+NTFS.attr_size], eax
        and     dword [ebp+NTFS.attr_size+4], 0
        jmp     .testfz

.sdnores:
        mov     ecx, [eax+30h]
        mov     dword [ebp+NTFS.attr_size], ecx
        mov     ecx, [eax+34h]
        mov     dword [ebp+NTFS.attr_size+4], ecx
.testfz:
        xor     eax, eax
.testf:
        imul    eax, [ebp+NTFS.sectors_per_cluster]
        cmp     eax, [ebp+NTFS.cur_offs]
        pop     eax
        ja      @f
        mov     edi, [esi+10h]  ; keep previous iRecord
        jmp     .scanlistcont

@@:
        pop     ecx
.scanlistfound:
        cmp     edi, -1
        jz      .ret
        mov     eax, [ebp+NTFS.cur_iRecord]
        mov     [ebp+NTFS.attr_iBaseRecord], eax
        mov     eax, edi
        jmp     .beginfindattr

.scanlistdone:
        pop     ecx
        sub     ecx, ebp
        sub     ecx, NTFS.attrlist_buf-1Ah
        cmp     [ebp+NTFS.cur_iRecord], 0
        jnz     @f
        sub     ecx, NTFS.attrlist_mft_buf-NTFS.attrlist_buf
@@:
        cmp     ecx, 0x400
        jnz     .scanlistfound
        inc     edx
        push    esi edi
        lea     esi, [ebp+NTFS.attrlist_buf+0x200]
        lea     edi, [ebp+NTFS.attrlist_buf]
        cmp     [ebp+NTFS.cur_iRecord], 0
        jnz     @f
        lea     esi, [ebp+NTFS.attrlist_mft_buf+0x200]
        lea     edi, [ebp+NTFS.attrlist_mft_buf]
@@:
        mov     ecx, 0x200/4
        rep movsd
        mov     eax, edi
        pop     edi esi
        sub     esi, 0x200
        push    [ebp+NTFS.cur_offs]
        push    [ebp+NTFS.cur_size]
        push    [ebp+NTFS.cur_read]
        push    [ebp+NTFS.cur_buf]
        push    dword [ebp+NTFS.attr_size]
        push    dword [ebp+NTFS.attr_size+4]
        or      dword [ebp+NTFS.attr_size+4], -1
        mov     [ebp+NTFS.cur_offs], edx
        mov     [ebp+NTFS.cur_size], 1
        and     [ebp+NTFS.cur_read], 0
        mov     [ebp+NTFS.cur_buf], eax
        mov     ecx, [ebp+NTFS.attr_list]
        push    esi edx edi
        call    .doreadattr
        pop     edi edx esi
        mov     ecx, [ebp+NTFS.cur_read]
        pop     dword [ebp+NTFS.attr_size+4]
        pop     dword [ebp+NTFS.attr_size]
        pop     [ebp+NTFS.cur_buf]
        pop     [ebp+NTFS.cur_read]
        pop     [ebp+NTFS.cur_size]
        pop     [ebp+NTFS.cur_offs]
        jc      .errret
        lea     ecx, [ecx+ebp+NTFS.attrlist_buf+0x200-0x1A]
        cmp     [ebp+NTFS.cur_iRecord], 0
        jnz     .scanliststart
        add     ecx, NTFS.attrlist_mft_buf-NTFS.attrlist_buf
        jmp     .scanliststart

.doreadattr:
        mov     [ebp+NTFS.bCanContinue], 0
        cmp     byte [ecx+nonResidentFlag], 0
        jnz     .nonresident
        mov     eax, [ecx+sizeWithoutHeader]
        mov     esi, eax
        mov     edx, [ebp+NTFS.cur_offs]
        shr     eax, 9
        cmp     eax, edx
        jb      .okret
        shl     edx, 9
        sub     esi, edx
        movzx   eax, word [ecx+attributeOffset]
        add     edx, eax
        add     edx, ecx        ; edx -> data
        mov     eax, [ebp+NTFS.cur_size]
        cmp     eax, (0xFFFFFFFF shr 9)+1
        jbe     @f
        mov     eax, (0xFFFFFFFF shr 9)+1
@@:
        shl     eax, 9
        cmp     eax, esi
        jbe     @f
        mov     eax, esi
@@:
; eax = length, edx -> data
        mov     [ebp+NTFS.cur_read], eax
        mov     ecx, eax
        mov     eax, edx
        mov     ebx, [ebp+NTFS.cur_buf]
        call    memmove
        and     [ebp+NTFS.cur_size], 0      ; CF=0
        ret

.nonresident:
; Not all auxiliary records contain correct FileSize info
        mov     eax, dword [ebp+NTFS.attr_size]
        mov     edx, dword [ebp+NTFS.attr_size+4]
        cmp     edx, -1
        jnz     @f
        mov     eax, [ecx+attributeRealSize]
        mov     edx, [ecx+attributeRealSize+4]
        mov     dword [ebp+NTFS.attr_size], eax
        mov     dword [ebp+NTFS.attr_size+4], edx
@@:
        add     eax, 0x1FF
        adc     edx, 0
        shrd    eax, edx, 9
        sub     eax, [ebp+NTFS.cur_offs]
        ja      @f
; return with nothing read
        and     [ebp+NTFS.cur_size], 0
.okret:
        clc
        ret

@@:
; reduce read length
        and     [ebp+NTFS.cur_tail], 0
        cmp     [ebp+NTFS.cur_size], eax
        jb      @f
        mov     [ebp+NTFS.cur_size], eax
        mov     eax, dword [ebp+NTFS.attr_size]
        and     eax, 0x1FF
        mov     [ebp+NTFS.cur_tail], eax
@@:
        mov     eax, [ebp+NTFS.cur_offs]
        xor     edx, edx
        div     [ebp+NTFS.sectors_per_cluster]
        sub     eax, [ecx+firstVCN]
        jb      .okret
        mov     ebx, edx
; eax = starting cluster, ebx = sector in the cluster
        cmp     [ebp+NTFS.cur_attr], 0x80
        jnz     .sys
        cmp     [ebp+NTFS.cur_iRecord], 0
        jz      .sys
        push    fs_read64_app
        cmp     [ebp+NTFS.bWriteAttr], 1
        jnz     @f
        mov     dword[esp], fs_write64_app
        jmp     @f

.sys:
        push    fs_read64_sys
@@:
        sub     esp, 10h
        movzx   esi, word [ecx+dataRunsOffset]
        add     esi, ecx
        xor     edi, edi
        mov     [ebp+NTFS.fragmentCount], 0
.readloop:
        call    ntfs_decode_mcb_entry
        jnc     .break
        add     edi, [esp+8]
        sub     eax, [esp]
        jae     .readloop
        mov     ecx, edi
        add     ecx, eax
        add     ecx, [esp]
        neg     eax
        mul     [ebp+NTFS.sectors_per_cluster]
        xchg    eax, ecx
        mul     [ebp+NTFS.sectors_per_cluster]
        sub     ecx, ebx
        add     eax, ebx
        mov     ebx, [ebp+NTFS.cur_buf]
        cmp     ecx, [ebp+NTFS.cur_size]
        jb      @f
        mov     ecx, [ebp+NTFS.cur_size]
@@:
        mov     [ebp+NTFS.LastRead], eax
        push    ecx
        call    dword[esp+14h]
        pop     ecx
        test    eax, eax
        jnz     .errread2
        sub     [ebp+NTFS.cur_size], ecx
        add     [ebp+NTFS.cur_offs], ecx
        shl     ecx, 9
        add     [ebp+NTFS.cur_read], ecx
        add     [ebp+NTFS.cur_buf], ecx
        inc     [ebp+NTFS.fragmentCount]
        xor     eax, eax
        xor     ebx, ebx
        cmp     [ebp+NTFS.cur_size], 0
        jnz     .readloop
        add     esp, 14h
        mov     eax, [ebp+NTFS.cur_tail]
        test    eax, eax
        jz      @f
        sub     eax, 0x200
        add     [ebp+NTFS.cur_read], eax
@@:
        clc
        ret

.errread2:
        add     esp, 14h
        stc
        ret

.break:
        add     esp, 14h        ; CF=0
        mov     [ebp+NTFS.bCanContinue], 1
        ret

ntfs_read_file_record:
; in: eax = iRecord
; out: [ebp+NTFS.frs_buffer] -> file record
; CF=1 -> failed, eax = disk error code, eax=0 -> something with FS
    ; Read attr $DATA of $Mft, starting from eax*[ebp+NTFS.frs_size]
        push    ecx edx
        mov     ecx, [ebp+NTFS.frs_size]
        mul     ecx
        shrd    eax, edx, 9
        shr     edx, 9
        jnz     .errret
        push    [ebp+NTFS.attr_iBaseRecord]
        push    [ebp+NTFS.attr_offs]
        push    [ebp+NTFS.attr_list]
        push    dword [ebp+NTFS.attr_size+4]
        push    dword [ebp+NTFS.attr_size]
        push    [ebp+NTFS.cur_iRecord]
        push    [ebp+NTFS.cur_attr]
        push    [ebp+NTFS.cur_offs]
        push    [ebp+NTFS.cur_size]
        push    [ebp+NTFS.cur_buf]
        push    [ebp+NTFS.cur_read]
        mov     [ebp+NTFS.cur_attr], 0x80   ; $DATA
        and     [ebp+NTFS.cur_iRecord], 0   ; $Mft
        mov     [ebp+NTFS.cur_offs], eax
        shr     ecx, 9
        mov     [ebp+NTFS.cur_size], ecx
        mov     eax, [ebp+NTFS.frs_buffer]
        mov     [ebp+NTFS.cur_buf], eax
        call    ntfs_read_attr
        mov     edx, [ebp+NTFS.cur_read]
        pop     [ebp+NTFS.cur_read]
        pop     [ebp+NTFS.cur_buf]
        pop     [ebp+NTFS.cur_size]
        pop     [ebp+NTFS.cur_offs]
        pop     [ebp+NTFS.cur_attr]
        pop     [ebp+NTFS.cur_iRecord]
        pop     dword [ebp+NTFS.attr_size]
        pop     dword [ebp+NTFS.attr_size+4]
        pop     [ebp+NTFS.attr_list]
        pop     [ebp+NTFS.attr_offs]
        pop     [ebp+NTFS.attr_iBaseRecord]
        jc      .ret
        cmp     edx, [ebp+NTFS.frs_size]
        jnz     .errret
        mov     eax, [ebp+NTFS.LastRead]
        mov     [ebp+NTFS.mftLastRead], eax
        mov     eax, [ebp+NTFS.frs_buffer]
        cmp     dword [eax], 'FILE'
        jnz     .errret
        push    ebx
        mov     ebx, eax
        call    ntfs_restore_usa_frs
        pop     ebx
        jc      .errret
.ret:
        pop     edx ecx
        ret

.errret:
        pop     edx ecx
        xor     eax, eax
        stc
        ret

ntfs_restore_usa_frs:
        mov     eax, [ebp+NTFS.frs_size]
ntfs_restore_usa:
;   in:
; ebx -> record
; eax = size in bytes
        pushad
        shr     eax, 9
        mov     ecx, eax
        inc     eax
        cmp     [ebx+updateSequenceSize], ax
        jnz     .err
        movzx   eax, word [ebx+updateSequenceOffset]
        lea     esi, [eax+ebx]
        lodsw
        mov     edx, eax
        lea     edi, [ebx+0x1FE]
@@:
        cmp     [edi], dx
        jnz     .err
        lodsw
        stosw
        add     edi, 0x1FE
        loop    @b
        popad
        clc
        ret

.err:
        popad
        stc
        ret

ntfs_decode_mcb_entry:
;   in:
; esi -> MCB entry
; esp -> buffer (16 bytes)
;   out:
; esi -> next MCB entry
; esp -> data run size
; esp+8 -> cluster (delta)
; CF=0 -> MCB end
        push    eax ecx edi
        lea     edi, [esp+16]
        xor     eax, eax
        lodsb
        test    al, al
        jz      .end
        mov     ecx, eax
        and     ecx, 0xF
        cmp     ecx, 8
        ja      .end
        push    ecx
        rep movsb
        pop     ecx
        sub     ecx, 8
        neg     ecx
        cmp     byte [esi-1], 80h
        jae     .end
        push    eax
        xor     eax, eax
        rep stosb
        pop     ecx
        shr     ecx, 4
        cmp     ecx, 8
        ja      .end
        push    ecx
        rep movsb
        pop     ecx
        sub     ecx, 8
        neg     ecx
        cmp     byte [esi-1], 80h
        cmc
        sbb     eax, eax
        rep stosb
        stc
.end:
        pop     edi ecx eax
        ret

ntfs_find_lfn:
; in: esi -> path string in UTF-8
;   out:
; [ebp+NTFS.cur_iRecord] = target fileRecord
; eax -> target index in the node
; [ebp+NTFS.LastRead] = target node location
; [ebp+NTFS.indexPointer] -> index, that points the target subnode
; [ebp+NTFS.nodeLastRead] = branch node location
; [ebp+NTFS.indexRoot] -> attribute
; [ebp+NTFS.rootLastRead] = directory fileRecord location
; [ebp+NTFS.cur_size] = index record size in sectors
; [ebp+NTFS.cur_subnode_size] = index record size in clusters or sectors
; CF=1 -> file not found, eax=0 -> error
        mov     [ebp+NTFS.cur_iRecord], 5   ; start from root directory
.doit2:
        mov     [ebp+NTFS.cur_attr], 0x90   ; $INDEX_ROOT
        and     [ebp+NTFS.cur_offs], 0
        mov     eax, [ebp+NTFS.cur_index_size]
        mov     [ebp+NTFS.cur_size], eax
        mov     eax, [ebp+NTFS.cur_index_buf]
        mov     [ebp+NTFS.cur_buf], eax
        call    ntfs_read_attr
        mov     eax, 0
        jc      .ret
        cmp     [ebp+NTFS.cur_read], 0x20
        jc      .ret
        push    esi
        pushad
        mov     esi, [ebp+NTFS.cur_index_buf]
        mov     eax, [esi+indexRecordSize]
        shr     eax, 9
        cmp     [ebp+NTFS.cur_index_size], eax
        jc      .realloc
        mov     [ebp+NTFS.cur_size], eax
        mov     al, [esi+indexRecordSizeClus]
        mov     [ebp+NTFS.cur_subnode_size], eax
        add     esi, rootNode
        mov     eax, [esi+nodeRealSize]
        add     eax, rootNode
        cmp     [ebp+NTFS.cur_read], eax
        jc      .err
        mov     eax, [ebp+NTFS.mftLastRead]
        mov     [ebp+NTFS.rootLastRead], eax
        mov     eax, [ebp+NTFS.attr_offs]
        mov     [ebp+NTFS.indexRoot], eax
.scanloop:  ; esi -> current index node
        add     esi, [esi+indexOffset]
.scanloopint:
        push    esi
        test    byte [esi+indexFlags], 2
        jnz     .subnode
        movzx   ecx, byte [esi+fileNameLength]
        lea     edi, [esi+fileName]
        mov     esi, [esp+8]
@@:
        call    utf8to16
        cmp     ax, '/'
        jz      .subnode
        call    utf16toUpper
        push    eax
        mov     ax, [edi]
        call    utf16toUpper
        cmp     [esp], ax
        pop     eax
        jc      .subnode
        jnz     .scanloopcont
        add     edi, 2
        loop    @b
        call    utf8to16
        cmp     ax, '/'
        jz      .found
        test    ax, ax
        jz      .found
.scanloopcont:
        pop     esi
        movzx   eax, word [esi+indexAllocatedSize]
        add     esi, eax
        jmp     .scanloopint

.realloc:
        mov     edi, eax
        mov     eax, [esi+indexRecordSize]
        shl     eax, 1
        stdcall kernel_alloc, eax
        test    eax, eax
        jz      .err
        mov     edx, [ebp+NTFS.cur_index_buf]
        cmp     edx, [ebp+NTFS.secondIndexBuffer]
        jc      @f
        mov     edx, [ebp+NTFS.secondIndexBuffer]
@@:
        mov     [ebp+NTFS.cur_index_buf], eax
        add     eax, [esi+indexRecordSize]
        mov     [ebp+NTFS.secondIndexBuffer], eax
        mov     [ebp+NTFS.cur_index_size], edi
        stdcall kernel_free, edx
        popad
        pop     eax
        jmp     .doit2

.notfound:
        mov     [esp+28], esi
.err:
        popad
        stc
.ret2:
        pop     esi
.ret:
        ret

.subnode:
        pop     esi
        test    byte [esi+indexFlags], 1
        jz      .notfound
        mov     eax, [ebp+NTFS.LastRead]
        mov     [ebp+NTFS.nodeLastRead], eax
        mov     [ebp+NTFS.indexPointer], esi
        movzx   eax, word [esi+indexAllocatedSize]
        mov     eax, [esi+eax-8]
        mov     edx, [ebp+NTFS.cur_size]
        push    edx
        cmp     edx, [ebp+NTFS.cur_subnode_size]
        jz      @f
        mul     [ebp+NTFS.sectors_per_cluster]
@@:
        mov     esi, [ebp+NTFS.cur_index_buf]
        xchg    [ebp+NTFS.secondIndexBuffer], esi
        mov     [ebp+NTFS.cur_index_buf], esi
        mov     [ebp+NTFS.cur_buf], esi
        mov     [ebp+NTFS.cur_attr], 0xA0   ; $INDEX_ALLOCATION
        mov     [ebp+NTFS.cur_offs], eax
        call    ntfs_read_attr.newAttribute
        pop     eax
        mov     [ebp+NTFS.cur_size], eax
        shl     eax, 9
        cmp     [ebp+NTFS.cur_read], eax
        jnz     .err
        cmp     dword [esi], 'INDX'
        jnz     .err
        mov     ebx, esi
        call    ntfs_restore_usa
        jc      .err
        add     esi, recordNode
        jmp     .scanloop

.found:
        mov     [esp+8], esi
        pop     eax
        mov     [esp+28], eax
        mov     eax, [eax+fileRecordReference]
        mov     [ebp+NTFS.cur_iRecord], eax
        popad
        cmp     byte [esi-1], 0
        jz      .ret2
        pop     eax
        jmp     .doit2

;----------------------------------------------------------------
ntfs_ReadFile:
        call    ntfs_lock
        call    ntfs_find_lfn
        jc      ntfsNotFound
        mov     [ebp+NTFS.cur_attr], 0x80   ; $DATA
        and     [ebp+NTFS.cur_offs], 0
        and     [ebp+NTFS.cur_size], 0
        call    ntfs_read_attr
        jc      ntfsDenied
        xor     eax, eax
        push    eax
        cmp     dword [ebx+8], 0x200
        jnc     .eof
        mov     ecx, [ebx+12]
        mov     edx, [ebx+16]
        mov     eax, [ebx+4]
        test    eax, 0x1FF
        jz      .alignedstart
        push    edx
        mov     edx, [ebx+8]
        shrd    eax, edx, 9
        pop     edx
        mov     [ebp+NTFS.cur_offs], eax
        mov     [ebp+NTFS.cur_size], 1
        lea     eax, [ebp+NTFS.bitmap_buf]
        mov     [ebp+NTFS.cur_buf], eax
        call    ntfs_read_attr.continue
        mov     eax, [ebx+4]
        and     eax, 0x1FF
        lea     esi, [ebp+NTFS.bitmap_buf+eax]
        sub     eax, [ebp+NTFS.cur_read]
        jae     .eof
        neg     eax
        push    ecx
        cmp     ecx, eax
        jb      @f
        mov     ecx, eax
@@:
        mov     [esp+4], ecx
        mov     edi, edx
        rep movsb
        mov     edx, edi
        pop     ecx
        sub     ecx, [esp]
        jz      .retok
        cmp     [ebp+NTFS.cur_read], 0x200
        jnz     .eof
.alignedstart:
        mov     eax, [ebx+4]
        push    edx
        mov     edx, [ebx+8]
        add     eax, 511
        adc     edx, 0
        shrd    eax, edx, 9
        pop     edx
        mov     [ebp+NTFS.cur_offs], eax
        mov     [ebp+NTFS.cur_buf], edx
        mov     eax, ecx
        shr     eax, 9
        mov     [ebp+NTFS.cur_size], eax
        add     eax, [ebp+NTFS.cur_offs]
        push    eax
        call    ntfs_read_attr.continue
        pop     [ebp+NTFS.cur_offs]
        mov     eax, [ebp+NTFS.cur_read]
        add     [esp], eax
        mov     eax, ecx
        and     eax, not 0x1FF
        cmp     [ebp+NTFS.cur_read], eax
        jnz     .eof
        and     ecx, 0x1FF
        jz      .retok
        add     edx, [ebp+NTFS.cur_read]
        mov     [ebp+NTFS.cur_size], 1
        lea     eax, [ebp+NTFS.bitmap_buf]
        mov     [ebp+NTFS.cur_buf], eax
        call    ntfs_read_attr.continue
        cmp     [ebp+NTFS.cur_read], ecx
        jb      @f
        mov     [ebp+NTFS.cur_read], ecx
@@:
        xchg    ecx, [ebp+NTFS.cur_read]
        push    ecx
        mov     edi, edx
        lea     esi, [ebp+NTFS.bitmap_buf]
        add     [esp+4], ecx
        rep movsb
        pop     ecx
        cmp     ecx, [ebp+NTFS.cur_read]
        jnz     .eof
.retok:
        pushd   0
.ret:
        call    ntfs_unlock
        pop     eax ebx
        ret

.eof:
        push    ERROR_END_OF_FILE
        jmp     .ret

;----------------------------------------------------------------
ntfs_ReadFolder:
        call    ntfs_lock
        mov     [ebp+NTFS.cur_iRecord], 5   ; root directory
        cmp     byte [esi], 0
        jz      @f
        call    ntfs_find_lfn
        jc      ntfsNotFound
@@:
        mov     [ebp+NTFS.cur_attr], 0x10   ; $STANDARD_INFORMATION
        and     [ebp+NTFS.cur_offs], 0
        mov     [ebp+NTFS.cur_size], 1
        lea     eax, [ebp+NTFS.bitmap_buf]
        mov     [ebp+NTFS.cur_buf], eax
        call    ntfs_read_attr
        jc      ntfsFail
        mov     [ebp+NTFS.cur_attr], 0x90   ; $INDEX_ROOT
.doit:
        mov     eax, [ebp+NTFS.cur_index_size]
        mov     [ebp+NTFS.cur_size], eax
        mov     eax, [ebp+NTFS.cur_index_buf]
        mov     [ebp+NTFS.cur_buf], eax
        call    ntfs_read_attr.newAttribute
        jc      ntfsFail
        cmp     [ebp+NTFS.cur_read], 0x20
        jc      ntfsFail
        mov     esi, [ebp+NTFS.cur_index_buf]
        mov     eax, [esi+indexRecordSize]
        shr     eax, 9
        cmp     [ebp+NTFS.cur_index_size], eax
        jc      .realloc
        mov     [ebp+NTFS.cur_subnode_size], eax
        add     esi, rootNode
        mov     eax, [esi+nodeRealSize]
        add     eax, rootNode
        cmp     [ebp+NTFS.cur_read], eax
        jc      ntfsFail
        mov     edi, [ebx+16]
        mov     ecx, [ebx+12]
        pushd   [ebx]
        pushd   [ebx+8]     ; read ANSI/UNICODE name
        push    edi
        mov     edx, esp
        mov     ebx, [ebx+4]
; init header
        xor     eax, eax
        mov     [edi+8], eax
        mov     [edi+4], eax
        inc     eax
        mov     [edi], eax      ; version
        add     edi, 32
; edi -> BDFE, esi -> current index data, ebx = first wanted block,
; ecx = number of blocks to read
; edx -> parameters block: dd <output>, dd <flags>
        cmp     [ebp+NTFS.cur_iRecord], 5
        jz      .skip_specials
; dot and dotdot entries
        push    esi
        xor     esi, esi
        call    .add_special_entry
        inc     esi
        call    .add_special_entry
        pop     esi
.skip_specials:
; at first, dump index root
        add     esi, [esi+indexOffset]
.dump_root:
        test    byte [esi+indexFlags], 2
        jnz     .dump_root_done
        call    .add_entry
        movzx   eax, word [esi+indexAllocatedSize]
        add     esi, eax
        jmp     .dump_root

.realloc:
        mov     edi, eax
        mov     eax, [esi+indexRecordSize]
        shl     eax, 1
        stdcall kernel_alloc, eax
        test    eax, eax
        jz      ntfsFail
        mov     edx, [ebp+NTFS.cur_index_buf]
        cmp     edx, [ebp+NTFS.secondIndexBuffer]
        jc      @f
        mov     edx, [ebp+NTFS.secondIndexBuffer]
@@:
        mov     [ebp+NTFS.cur_index_buf], eax
        add     eax, [esi+indexRecordSize]
        mov     [ebp+NTFS.secondIndexBuffer], eax
        mov     [ebp+NTFS.cur_index_size], edi
        stdcall kernel_free, edx
        jmp     .doit

.dump_root_done:
; now dump all subnodes
        push    ecx edi
        lea     edi, [ebp+NTFS.bitmap_buf]
        mov     [ebp+NTFS.cur_buf], edi
        mov     ecx, 0x400/4
        xor     eax, eax
        rep stosd
        mov     [ebp+NTFS.cur_attr], 0xB0   ; $BITMAP
        and     [ebp+NTFS.cur_offs], 0
        mov     [ebp+NTFS.cur_size], 2
        call    ntfs_read_attr.newAttribute
        pop     edi ecx
        push    0       ; save offset in $BITMAP attribute
        and     [ebp+NTFS.cur_offs], 0
.dumploop:
        mov     [ebp+NTFS.cur_attr], 0xA0
        mov     eax, [ebp+NTFS.cur_subnode_size]
        mov     [ebp+NTFS.cur_size], eax
        mov     esi, [ebp+NTFS.cur_index_buf]
        mov     [ebp+NTFS.cur_buf], esi
        mov     eax, [ebp+NTFS.cur_offs]
        push    eax
        imul    eax, [ebp+NTFS.cur_subnode_size]
        mov     [ebp+NTFS.cur_offs], eax
        call    ntfs_read_attr.newAttribute
        pop     [ebp+NTFS.cur_offs]
        mov     eax, [ebp+NTFS.cur_subnode_size]
        shl     eax, 9
        cmp     [ebp+NTFS.cur_read], eax
        jnz     .done
        push    eax
        mov     eax, [ebp+NTFS.cur_offs]
        and     eax, 0x400*8-1
        bt      dword [ebp+NTFS.bitmap_buf], eax
        pop     eax
        jnc     .dump_subnode_done
        cmp     dword [esi], 'INDX'
        jnz     .dump_subnode_done
        push    ebx
        mov     ebx, esi
        call    ntfs_restore_usa
        pop     ebx
        jc      .dump_subnode_done
        add     esi, recordNode
        add     esi, [esi+indexOffset]
.dump_subnode:
        test    byte [esi+indexFlags], 2
        jnz     .dump_subnode_done
        call    .add_entry
        movzx   eax, word [esi+indexAllocatedSize]
        add     esi, eax
        jmp     .dump_subnode

.dump_subnode_done:
        inc     [ebp+NTFS.cur_offs]
        test    [ebp+NTFS.cur_offs], 0x400*8-1
        jnz     .dumploop
        mov     [ebp+NTFS.cur_attr], 0xB0
        push    ecx edi
        lea     edi, [ebp+NTFS.bitmap_buf]
        mov     [ebp+NTFS.cur_buf], edi
        mov     ecx, 0x400/4
        xor     eax, eax
        rep stosd
        pop     edi ecx
        pop     eax
        push    [ebp+NTFS.cur_offs]
        inc     eax
        mov     [ebp+NTFS.cur_offs], eax
        mov     [ebp+NTFS.cur_size], 2
        push    eax
        call    ntfs_read_attr.newAttribute
        pop     eax
        pop     [ebp+NTFS.cur_offs]
        push    eax
        jmp     .dumploop

.done:
        pop     eax
        pop     eax
        mov     ebx, [eax+4]
        pop     eax
        pop     eax
        test    eax, eax
        jz      .ret
        xor     eax, eax
        dec     ecx
        js      @f
        mov     al, ERROR_END_OF_FILE
@@:
        push    eax
        call    ntfs_unlock
        pop     eax
        ret

.add_special_entry:
        mov     eax, [edx]
        inc     dword [eax+8]   ; new file found
        dec     ebx
        jns     .ret
        dec     ecx
        js      .ret
        inc     dword [eax+4]   ; new file block copied
        mov     eax, [edx+4]
        mov     [edi+4], eax
        mov     eax, 0x10
        stosd
        scasd
        push    ebx ecx edx
        mov     eax, dword [ebp+NTFS.bitmap_buf]
        mov     edx, dword [ebp+NTFS.bitmap_buf+4]
        call    ntfs_datetime_to_bdfe
        mov     eax, dword [ebp+NTFS.bitmap_buf+0x18]
        mov     edx, dword [ebp+NTFS.bitmap_buf+0x1C]
        call    ntfs_datetime_to_bdfe
        mov     eax, dword [ebp+NTFS.bitmap_buf+8]
        mov     edx, dword [ebp+NTFS.bitmap_buf+0xC]
        call    ntfs_datetime_to_bdfe
        pop     edx ecx ebx
        xor     eax, eax
        stosd
        stosd
        mov     al, '.'
        push    edi ecx
        lea     ecx, [esi+1]
        cmp     dword[edi-36], 2
        jz      .utf16sp
        rep stosb
        mov     byte [edi], 0
        pop     ecx edi
        cmp     dword[edi-36], 3
        jz      @f
        add     edi, 264
        ret

.utf16sp:
        rep stosw
        mov     word [edi], 0
        pop     ecx edi
@@:
        add     edi, 520
.ret:
        ret

.add_entry:
; do not return DOS 8.3 names
        cmp     byte [esi+namespace], 2
        jz      .ret
; do not return system files
        cmp     dword[esi+fileRecordReference], 16
        jb      .ret
        cmp     byte [esi+fileNameLength], 0
        jz      .ret
        mov     eax, [edx]
        inc     dword [eax+8]   ; new file found
        dec     ebx
        jns     .ret
        dec     ecx
        js      .ret
        inc     dword [eax+4]   ; new file block copied
        mov     eax, [edx+4]    ; flags
        call    ntfs_direntry_to_bdfe
        push    ecx esi edi
        movzx   ecx, byte [esi+fileNameLength]
        add     esi, fileName
        cmp     dword[edi-36], 2
        jz      .utf16
        cmp     dword[edi-36], 3
        jz      .utf8
@@:
        lodsw
        call    uni2ansi_char
        stosb
        loop    @b
        mov     byte [edi], 0
        pop     edi esi ecx
        add     edi, 264
        ret

.utf8:
        push    ecx
        mov     cx, 519
@@:
        lodsw
        call    UTF16to8
        js      @f
        dec     dword[esp]
        jnz     @b
@@:
        mov     byte [edi], 0
        pop     edi
@@:
        pop     edi esi ecx
        add     edi, 520
        ret

.utf16:
        rep movsw
        mov     word [edi], 0
        jmp     @b

ntfs_direntry_to_bdfe:
        mov     [edi+4], eax    ; ANSI/UNICODE name
        mov     eax, [esi+fileFlags]
        test    eax, 0x10000000
        jz      @f
        and     eax, not 0x10000000
        or      al, 0x10
@@:
        stosd
        scasd
        push    ebx ecx edx
        mov     eax, [esi+fileCreated]
        mov     edx, [esi+fileCreated+4]
        call    ntfs_datetime_to_bdfe
        mov     eax, [esi+fileAccessed]
        mov     edx, [esi+fileAccessed+4]
        call    ntfs_datetime_to_bdfe
        mov     eax, [esi+fileModified]
        mov     edx, [esi+fileModified+4]
        call    ntfs_datetime_to_bdfe
        pop     edx ecx ebx
        mov     eax, [esi+fileRealSize]
        stosd
        mov     eax, [esi+fileRealSize+4]
        stosd
        ret

ntfs_datetime_to_bdfe:
; in: edx:eax = seconds since 01.01.1601 x10000000
; edi -> data block
; out: edi = edi+8
        sub     eax, 3365781504
        sbb     edx, 29389701
        mov     ecx, 10000000
        cmp     edx, ecx
        jc      @f
        xor     edx, edx
@@:
        div     ecx
        jmp     fsTime2bdfe

;----------------------------------------------------------------
ntfs_GetFileInfo:
        mov     edi, [ebx+16]
        cmp     byte [esi], 0
        jz      .volume
        call    ntfs_lock
        call    ntfs_find_lfn
        jnc     .found
        test    eax, eax
        jz      ntfsFail
        jmp     ntfsNotFound

.found:
        mov     esi, eax
        xor     eax, eax
        call    ntfs_direntry_to_bdfe
.end:
        call    ntfs_unlock
        xor     eax, eax
@@:
        ret

.volume:
        mov     eax, dword [ebp+NTFS.Length]
        mov     edx, dword [ebp+NTFS.Length+4]
        shld    edx, eax, 9
        shl     eax, 9
        mov     [edi+36], edx
        mov     [edi+32], eax
        mov     eax, [ebx+8]
        mov     byte [edi], 8
        mov     [edi+4], eax
        test    eax, eax
        jz      @b
        call    ntfs_lock
        add     edi, 40
        mov     [ebp+NTFS.cur_buf], edi
        mov     [ebp+NTFS.cur_iRecord], 3
        mov     [ebp+NTFS.cur_attr], 0x60
        mov     [ebp+NTFS.cur_offs], 0
        mov     [ebp+NTFS.cur_size], 1
        call    ntfs_read_attr
        jc      ntfsFail
        mov     ecx, [ebp+NTFS.cur_read]
        mov     [edi+ecx], ax
        cmp     byte [ebx+8], 2
        jz      .end
        shr     ecx, 1
        jz      .end
        mov     esi, edi
        cmp     byte [ebx+8], 3
        jnz     @f
        shl     ecx, 1
        call    UTF16to8_string
        mov     byte [edi], 0
        jmp     .end

@@:
        lodsw
        call    uni2ansi_char
        stosb
        loop    @b
        mov     byte [edi], 0
        jmp     .end

;----------------------------------------------------------------
ntfs_CreateFolder:
        mov     [ebp+NTFS.bFolder], 1
        jmp     @f

ntfs_CreateFile:
        mov     [ebp+NTFS.bFolder], 0
@@: ; 1. Search file
        call    ntfs_lock
        call    ntfs_find_lfn
        jc      .notFound
; found, rewrite
        cmp     [ebp+NTFS.cur_iRecord], 16
        jc      ntfsDenied
        cmp     [ebp+NTFS.bFolder], 1
        jz      .folder
        test    byte [eax+fileFlags], 1
        jnz     ntfsDenied
        cmp     [ebp+NTFS.fragmentCount], 1
        jnz     ntfsUnsupported     ; record fragmented
; edit directory node
        mov     edi, [ebp+NTFS.cur_index_buf]
        cmp     dword [edi], 'INDX'
        jz      @f
        mov     esi, [ebp+NTFS.frs_buffer]
        mov     ecx, [esi+recordRealSize]
        shr     ecx, 2
        rep movsd
        mov     esi, [ebp+NTFS.attr_offs]
        mov     cl, [esi+attributeOffset]
        sub     esi, [ebp+NTFS.frs_buffer]
        add     eax, ecx
        add     eax, esi
@@:
        mov     edi, eax
        mov     eax, [ebx+12]
        mov     [edi+fileRealSize], eax
        mov     dword [edi+fileRealSize+4], 0
        push    ebx eax
        call    ntfsGetTime
        mov     [edi+fileModified], eax
        mov     [edi+fileModified+4], edx
        mov     [edi+recordModified], eax
        mov     [edi+recordModified+4], edx
        mov     [edi+fileAccessed], eax
        mov     [edi+fileAccessed+4], edx
        pop     edx ebx
        mov     eax, [ebp+NTFS.LastRead]
        mov     [ebp+NTFS.nodeLastRead], eax
        mov     [ebp+NTFS.cur_attr], 0x80
        mov     [ebp+NTFS.cur_offs], 0
        mov     [ebp+NTFS.cur_size], 0
        call    ntfs_read_attr
        jc      ntfsFail
        mov     esi, edi
        mov     edi, [ebp+NTFS.frs_buffer]
        cmp     word [edi+baseRecordReuse], 0
        jnz     ntfsUnsupported     ; auxiliary record
        mov     al, [edi+attributeOffset]
        add     edi, eax
        mov     al, [edi+attributeOffset]
        add     edi, eax
        mov     ecx, 6
        add     esi, fileModified
        add     edi, 8
        rep movsd
        mov     eax, edx
        xor     edx, edx
        mov     ecx, [ebp+NTFS.attr_offs]
        cmp     word [ecx+attributeFlags], 0
        jnz     ntfsUnsupported
        push    ebx
        cmp     byte [ecx+nonResidentFlag], 0
        jz      @f
        cmp     [ecx+attributeRealSize+4], edx
        jnz     @f
        cmp     [ecx+attributeRealSize], eax
        jz      ntfs_WriteFile.writeNode
@@:
        jmp     ntfs_WriteFile.resizeAttribute

.folder:
        bt      dword [eax+fileFlags], 28
        jnc     ntfsDenied
        push    0
        jmp     ntfsOut

.notFound:  ; create
        test    eax, eax
        jz      ntfsFail
        cmp     [ebp+NTFS.fragmentCount], 1
        jnz     ntfsUnsupported     ; record fragmented
; 2. Prepare directory record
        mov     edi, esi
        mov     edx, eax
        xor     ecx, ecx
@@:         ; count characters
        call    utf8to16
        cmp     ax, '/'
        jz      ntfsNotFound    ; path folder not found
        inc     ecx
        test    ax, ax
        jnz     @b
        dec     ecx
        push    ecx     ; name length in chars
        push    edi
        shl     ecx, 1
        add     ecx, fileName+7
        and     ecx, not 7
        mov     edi, [ebp+NTFS.cur_index_buf]
        mov     eax, [ebx+12]
        mov     [ebp+NTFS.fileRealSize], eax
        mov     eax, [ebx+16]
        mov     [ebp+NTFS.fileDataBuffer], eax
        push    ecx     ; index length
        mov     eax, edx
        mov     edx, ecx
        cmp     dword [edi], 'INDX'
        jz      .indexRecord
        mov     esi, [ebp+NTFS.frs_buffer]  ; indexRoot
        mov     ecx, [esi+recordRealSize]
        add     edx, ecx
        cmp     [esi+recordAllocatedSize], edx
        jc      .growTree
        mov     [esi+recordRealSize], edx
        shr     ecx, 2
        rep movsd
        mov     edi, [ebp+NTFS.indexRoot]
        sub     edi, [ebp+NTFS.frs_buffer]
        add     edi, [ebp+NTFS.cur_index_buf]
        mov     esi, [esp]
        add     [edi+sizeWithHeader], esi
        add     [edi+sizeWithoutHeader], esi
        mov     cl, [edi+attributeOffset]
        add     edi, ecx
        add     [edi+rootNode+nodeRealSize], esi
        add     [edi+rootNode+nodeAllocatedSize], esi
        sub     eax, [ebp+NTFS.cur_index_buf]
        add     eax, edi
        mov     edi, [ebp+NTFS.cur_index_buf]
        jmp     .common

.growTree:  ; create indexRecord
        mov     edi, [ebp+NTFS.cur_index_buf]
        mov     ecx, 10
        xor     eax, eax
        rep stosd
        mov     esi, [ebp+NTFS.indexRoot]
        mov     al, [esi+attributeOffset]
        add     esi, eax
        rdtsc
        stosw
        mov     eax, [esi+indexRecordSize]
        cmp     eax, [ebp+NTFS.frs_size]
        jc      .errorPop3
        shr     eax, 9
        inc     eax
        mov     edi, [ebp+NTFS.cur_index_buf]
        mov     dword[edi], 'INDX'
        mov     byte [edi+updateSequenceOffset], 28h
        mov     [edi+updateSequenceSize], al
        add     edi, recordNode
        shl     eax, 1
        add     eax, 28h-recordNode+7
        and     eax, not 7
        mov     [edi+indexOffset], eax
        mov     ecx, [esi+indexRecordSize]
        sub     ecx, recordNode
        mov     [edi+nodeAllocatedSize], ecx
        add     esi, rootNode
        push    esi
        mov     ecx, [esi+nodeRealSize]
        sub     ecx, [esi+indexOffset]
        add     eax, ecx
        mov     [edi+nodeRealSize], eax
        mov     eax, [esi+nonLeafFlag]
        mov     [edi+nonLeafFlag], eax
        shr     ecx, 2
        add     esi, [esi+indexOffset]
        add     edi, [edi+indexOffset]
        rep movsd       ; copy root indexes
; clear root node
        mov     cl, 10
        mov     edi, [esp]
        xor     eax, eax
        rep stosd
        pop     edi
        mov     byte [edi+indexOffset], 16
        mov     byte [edi+nodeRealSize], 28h
        mov     byte [edi+nodeAllocatedSize], 28h
        mov     byte [edi+nonLeafFlag], 1
        mov     byte [edi+16+indexAllocatedSize], 18h
        mov     byte [edi+16+indexFlags], 3
        mov     esi, [ebp+NTFS.indexRoot]
        add     edi, 28h
        mov     eax, edi
        sub     eax, esi
        mov     word [esi+sizeWithoutHeader], 38h
        xchg    [esi+sizeWithHeader], eax
        add     esi, eax
        mov     [ebp+NTFS.attr_offs], edi
        cmp     byte [esi], 0xA0
        jnz     @f
        cmp     dword [esi+attributeAllocatedSize], 0
        jz      @f
        mov     eax, [ebp+NTFS.frs_buffer]
        mov     ecx, eax
        add     ecx, [eax+recordRealSize]
        sub     ecx, esi
        shr     ecx, 2
        rep movsd
        sub     edi, eax
        mov     [eax+recordRealSize], edi
        call    .ntfsNodeAlloc
        jc      ntfsErrorPop3
        mov     eax, [ebp+NTFS.newRecord]
        mov     edi, [ebp+NTFS.cur_index_buf]
        mov     [edi+recordVCN], eax
        mov     edi, [ebp+NTFS.attr_offs]
        mov     [edi-8], eax
        jmp     .refresh

@@:
        mov     cl, 32
        xor     eax, eax
        rep stosd
        mov     eax, [ebp+NTFS.cur_subnode_size]
        cmp     eax, [ebp+NTFS.cur_size]
        jnz     @f
        cmp     [ebp+NTFS.sectors_per_cluster], 1
        jz      @f
        mov     al, 1
@@:
        mov     [ebp+NTFS.fileDataSize], eax
        mov     edi, [ebp+NTFS.BitmapStart]
        call    ntfsSpaceAlloc
        movi    eax, ERROR_DISK_FULL
        jc      ntfsErrorPop3
; create $IndexAllocation
        mov     edi, [ebp+NTFS.attr_offs]
        mov     byte [edi+attributeType], 0xA0
        mov     byte [edi+nonResidentFlag], 1
        mov     byte [edi+nameLength], 4
        mov     byte [edi+nameOffset], 40h
        mov     byte [edi+dataRunsOffset], 48h
        mov     byte [edi+sizeWithHeader], 50h
        mov     eax, [ebp+NTFS.fileDataSize]
        dec     eax
        mov     [edi+lastVCN], eax
        inc     eax
        mul     [ebp+NTFS.sectors_per_cluster]
        shl     eax, 9
        mov     [edi+attributeAllocatedSize], eax
        mov     [edi+attributeRealSize], eax
        mov     [edi+initialDataSize], eax
        mov     dword[edi+40h], 490024h     ; unicode $I30
        mov     dword[edi+40h+4], 300033h
        push    edi
        mov     esi, edi
        add     edi, 48h
        call    createMcbEntry
        mov     esi, [ebp+NTFS.frs_buffer]
        pop     edi
        mov     al, [esi+newAttributeID]
        mov     [edi+attributeID], al
        add     edi, 50h
        inc     eax
; create $Bitmap
        mov     [edi+attributeID], al
        inc     eax
        mov     [esi+newAttributeID], al
        mov     byte [edi+attributeType], 0xB0
        mov     byte [edi+nameLength], 4
        mov     byte [edi+nameOffset], 18h
        mov     byte [edi+attributeOffset], 20h
        mov     byte [edi+sizeWithoutHeader], 8
        mov     byte [edi+sizeWithHeader], 28h
        mov     dword[edi+18h], 490024h     ; unicode $I30
        mov     dword[edi+18h+4], 300033h
        mov     byte [edi+20h], 1
        mov     dword[edi+28h], -1
        add     edi, 30h
        sub     edi, esi
        mov     [esi+recordRealSize], edi
        mov     eax, [ebp+NTFS.fileDataStart]
        mul     [ebp+NTFS.sectors_per_cluster]
        mov     edx, eax
        jmp     @f

.refresh:
        mov     [ebp+NTFS.cur_size], 0
        call    ntfs_read_attr.continue
        movi    eax, ERROR_FS_FAIL
        jc      ntfsErrorPop3
        mov     edx, [ebp+NTFS.LastRead]
@@:
        mov     ebx, [ebp+NTFS.cur_index_buf]
        call    writeRecord
        mov     ebx, [ebp+NTFS.frs_buffer]
        mov     edx, [ebp+NTFS.rootLastRead]
        call    writeRecord
        mov     esi, [esp+4]
        call    ntfs_find_lfn.doit2
        test    eax, eax
        jz      .errorPop3
        mov     edi, [ebp+NTFS.cur_index_buf]
        mov     edx, [esp]
.indexRecord:
        add     edi, recordNode
        add     edx, [edi+nodeRealSize]
        cmp     [edi+nodeAllocatedSize], edx
        jc      .arborizeTree
        mov     [edi+nodeRealSize], edx
        jmp     .common

.errorPop3:
        add     esp, 12
        jmp     ntfsUnsupported

.ntfsNodeAlloc:
; in: [ebp+NTFS.attr_offs] -> $IndexAllocation
;   out:
; [ebp+NTFS.newRecord] = node VCN
; [ebp+NTFS.cur_offs]
; CF=1 -> eax = error code
        mov     esi, [ebp+NTFS.attr_offs]
        add     esi, [esi+sizeWithHeader]
        cmp     byte [esi], 0xB0
        jnz     .ret
        movzx   ecx, word [esi+sizeWithoutHeader]
        shr     ecx, 2
        movzx   edi, byte [esi+attributeOffset]
        add     edi, esi
        mov     edx, edi
        or      eax, -1
        repz scasd
        jnz     @f
        cmp     [edi], eax
        jnz     .ret
; extend folder $Bitmap
        add     word [esi+sizeWithHeader], 8
        add     word [esi+sizeWithoutHeader], 8
        mov     esi, [ebp+NTFS.frs_buffer]
        mov     eax, [esi+recordRealSize]
        add     eax, 8
        cmp     [esi+recordAllocatedSize], eax
        jc      .ret
        mov     [esi+recordRealSize], eax
        xor     eax, eax
        stosd
        mov     [edi], eax
        mov     [edi+8], eax
        dec     eax
        mov     [edi+4], eax
@@:
        sub     edi, 4
        mov     eax, [edi]
        not     eax
        bsf     eax, eax
        bts     [edi], eax
        sub     edi, edx
        shl     edi, 3
        add     eax, edi
        mul     [ebp+NTFS.cur_subnode_size]
        mov     [ebp+NTFS.newRecord], eax
        mov     ecx, [ebp+NTFS.cur_size]
        cmp     ecx, [ebp+NTFS.cur_subnode_size]
        jz      @f
        mul     [ebp+NTFS.sectors_per_cluster]
@@:
        mov     [ebp+NTFS.cur_offs], eax
        add     eax, ecx
        shl     eax, 9
        mov     esi, [ebp+NTFS.attr_offs]
        cmp     [esi+attributeAllocatedSize], eax
        jnc     @f
        xor     edx, edx
        jmp     resizeAttribute

.ret:
        movi    eax, ERROR_UNSUPPORTED_FS
        stc
@@:
        ret

.arborizeTree:      ; find median index
        mov     ecx, [edi+nodeRealSize]
        sub     ecx, [edi+indexOffset]
        shr     ecx, 1
        add     edi, [edi+indexOffset]
        xor     eax, eax
@@:
        add     edi, eax
        mov     ax, [edi+indexAllocatedSize]
        sub     ecx, eax
        jnc     @b
        add     eax, 8
        mov     esi, [ebp+NTFS.secondIndexBuffer]
        cmp     dword [esi], 'INDX'
        jz      @f
; move index to the root node
        mov     esi, [ebp+NTFS.frs_buffer]
        mov     ecx, eax
        add     ecx, 8
        add     ecx, [esi+recordRealSize]
        cmp     [esi+recordAllocatedSize], ecx
        jc      .growTree
        push    edi eax
        call    .ntfsNodeAlloc
        jc      ntfsErrorPop5
        pop     eax
        mov     edi, [ebp+NTFS.indexRoot]
        add     [ebp+NTFS.attr_offs], eax
        add     [edi+sizeWithHeader], eax
        add     [edi+sizeWithoutHeader], eax
        movzx   ecx, byte [edi+attributeOffset]
        add     ecx, edi
        add     [ecx+rootNode+nodeRealSize], eax
        add     [ecx+rootNode+nodeAllocatedSize], eax
        add     ecx, [ebp+NTFS.indexPointer]
        sub     ecx, [ebp+NTFS.secondIndexBuffer]
        mov     esi, [ebp+NTFS.frs_buffer]
        add     [esi+recordRealSize], eax
        add     esi, [esi+recordRealSize]
        mov     edi, esi
        sub     esi, eax
        neg     ecx
        add     ecx, esi
        shr     ecx, 2
        sub     esi, 4
        sub     edi, 4
        std
        rep movsd   ; make space
        mov     [edi], ecx
        mov     edi, esi
        add     edi, 4
        mov     esi, [esp]
        add     word [esi+indexAllocatedSize], 8
        mov     byte [esi+indexFlags], 1
        mov     ecx, eax
        sub     ecx, 8
        shr     ecx, 2
        cld
        rep movsd   ; insert index
        mov     eax, [ebp+NTFS.newRecord]
        stosd
        jmp     .splitNode

.growBranch:    ; move node and replace it with empty one
        mov     esi, [ebp+NTFS.cur_index_buf]
        mov     edi, [ebp+NTFS.secondIndexBuffer]
        mov     eax, [esi+recordVCN]
        mov     [edi+recordVCN], eax
        add     edi, recordNode
        mov     eax, [edi+indexOffset]
        add     eax, 18h
        mov     [edi+nodeRealSize], eax
        add     edi, [edi+indexOffset]
        mov     ecx, 6
        xor     eax, eax
        mov     [ebp+NTFS.indexPointer], edi
        push    edi
        rep stosd
        pop     edi
        mov     eax, [ebp+NTFS.newRecord]
        mov     byte [edi+indexAllocatedSize], 18h
        mov     byte [edi+indexFlags], 3
        mov     [edi+16], eax
        mov     [esi+recordVCN], eax
        mov     eax, [ebp+NTFS.LastRead]
        mov     [ebp+NTFS.nodeLastRead], eax
        push    [ebp+NTFS.cur_size]
        mov     [ebp+NTFS.cur_size], 0
        call    ntfs_read_attr.continue
        pop     [ebp+NTFS.cur_size]
        movi    eax, ERROR_FS_FAIL
        jc      ntfsErrorPop5
        pop     eax edi
@@:         ; move index to the branch node
        push    edi eax
        call    .ntfsNodeAlloc
        jc      ntfsErrorPop5
        mov     eax, [esp]
        mov     esi, [ebp+NTFS.secondIndexBuffer]
        add     esi, recordNode
        mov     ecx, [esi+nodeRealSize]
        add     eax, ecx
        cmp     [esi+nodeAllocatedSize], eax
        jc      .growBranch
        mov     [esi+nodeRealSize], eax
        lea     edi, [esi+eax-4]
        add     esi, ecx
        mov     ecx, esi
        sub     ecx, [ebp+NTFS.indexPointer]
        shr     ecx, 2
        sub     esi, 4
        std
        rep movsd   ; make space
        mov     [edi], ecx
        pop     ecx
        sub     ecx, 8
        shr     ecx, 2
        mov     edi, esi
        add     edi, 4
        mov     esi, [esp]
        add     word [esi+indexAllocatedSize], 8
        mov     byte [esi+indexFlags], 1
        cld
        rep movsd   ; insert index
        mov     eax, [ebp+NTFS.newRecord]
        stosd
        mov     ebx, [ebp+NTFS.secondIndexBuffer]
        mov     edx, [ebp+NTFS.nodeLastRead]
        push    esi
        call    writeRecord
        pop     esi
.splitNode:
        mov     edi, [ebp+NTFS.cur_index_buf]
        mov     eax, edi
        add     eax, recordNode
        add     eax, [edi+recordNode+nodeRealSize]
        sub     eax, esi
        push    eax
        mov     ecx, [edi+recordNode+indexOffset]
        add     eax, ecx
        add     ecx, recordNode
        shr     ecx, 2
        push    esi
        mov     esi, edi
        mov     edi, [ebp+NTFS.secondIndexBuffer]
        rep movsd
        pop     esi
        pop     ecx
        shr     ecx, 2
        rep movsd
        mov     edi, [ebp+NTFS.secondIndexBuffer]
        mov     [edi+recordNode+nodeRealSize], eax
        pop     edi
        mov     cl, 4
        xor     eax, eax
        mov     esi, edi
        rep stosd
        mov     byte [esi+indexAllocatedSize], 16
        mov     byte [esi+indexFlags], 2
        mov     esi, [ebp+NTFS.cur_index_buf]
        mov     eax, [ebp+NTFS.newRecord]
        mov     [esi+recordVCN], eax
        add     esi, recordNode
        sub     edi, esi
        mov     [esi+nodeRealSize], edi
        mov     ebx, [ebp+NTFS.secondIndexBuffer]
        mov     edx, [ebp+NTFS.LastRead]
        call    writeRecord
        jmp     .refresh

.common:
        add     edi, edx
        sub     edi, 4
        mov     esi, edi
        sub     esi, [esp]
        mov     ecx, esi
        sub     ecx, eax    ; eax = pointer in the record
        shr     ecx, 2
        inc     ecx
        std
        rep movsd           ; move forward, make space
        mov     ecx, [esp]
        shr     ecx, 2
        xor     eax, eax
        rep stosd
        cld
        add     edi, 4
        call    ntfsGetTime
        mov     [edi+fileCreated], eax
        mov     [edi+fileCreated+4], edx
        mov     [edi+fileModified], eax
        mov     [edi+fileModified+4], edx
        mov     [edi+recordModified], eax
        mov     [edi+recordModified+4], edx
        mov     [edi+fileAccessed], eax
        mov     [edi+fileAccessed+4], edx
        pop     ecx
        pop     esi
        mov     [edi+indexAllocatedSize], cx    ; fill index with data
        mov     eax, [esp]
        shl     eax, 1
        add     eax, 42h
        mov     [edi+indexRawSize], ax
        mov     eax, [ebp+NTFS.cur_iRecord]
        mov     [edi+directoryRecordReference], eax
        mov     eax, [ebp+NTFS.frs_buffer]
        mov     eax, [eax+reuseCounter]
        mov     [edi+directoryReferenceReuse], ax
        mov     eax, [ebp+NTFS.frs_size]
        shr     eax, 8
        add     ecx, 30h+48h+8+18h+8
        add     ecx, eax
        mov     eax, [ebp+NTFS.fileRealSize]
        add     ecx, eax
        mov     [edi+fileRealSize], eax
        cmp     [ebp+NTFS.frs_size], ecx
        jc      @f
        xor     eax, eax
@@:
        mov     ecx, [ebp+NTFS.sectors_per_cluster]
        shl     ecx, 9
        add     eax, ecx
        dec     eax
        xor     edx, edx
        div     ecx
        mov     [ebp+NTFS.fileDataSize], eax
        mul     ecx
        mov     [edi+fileAllocatedSize], eax
        pop     ecx
        mov     [ebp+NTFS.indexPointer], edi
        mov     [edi+fileNameLength], cl
        add     edi, fileName
@@:         ; record filename
        call    utf8to16
        stosw
        loop    @b
        mov     eax, [ebp+NTFS.LastRead]
        mov     [ebp+NTFS.nodeLastRead], eax
        cmp     [ebp+NTFS.bFolder], 0
        jz      @f
        mov     edi, [ebp+NTFS.indexPointer]
        bts     dword [edi+fileFlags], 28
        jmp     .mftBitmap

@@: ; 3. File data
        cmp     [ebp+NTFS.fileDataSize], 0
        jz      .mftBitmap
        mov     edi, [ebp+NTFS.BitmapStart]
        call    ntfsSpaceAlloc
        jc      ntfsDiskFull
        mov     eax, [ebp+NTFS.fileDataStart]
        mul     [ebp+NTFS.sectors_per_cluster]
        mov     ecx, [ebp+NTFS.fileRealSize]
        add     ecx, 511
        shr     ecx, 9
        mov     ebx, [ebp+NTFS.fileDataBuffer]
        call    fs_write64_app
        test    eax, eax
        jnz     ntfsDevice
    ; 4. MFT record
.mftBitmap: ; search for free record
        mov     edi, [ebp+NTFS.mftBitmapBuffer]
        mov     ecx, [ebp+NTFS.mftBitmapSize]
        mov     al, -1
        add     edi, 3
        sub     ecx, 3
        repz scasb
        dec     edi
        movzx   eax, byte [edi]
        not     al
        bsf     ecx, eax
        jz      .extendBitmapMFT    ; no free records
        bts     [edi], ecx
; get record location
        sub     edi, [ebp+NTFS.mftBitmapBuffer]
        shl     edi, 3
        add     edi, ecx
        mov     [ebp+NTFS.newRecord], edi
        mov     eax, [ebp+NTFS.frs_size]
        shr     eax, 9
        mul     edi
        mov     [ebp+NTFS.cur_iRecord], 0
        mov     [ebp+NTFS.cur_attr], 0x80
        mov     [ebp+NTFS.cur_offs], eax
        push    eax
        mov     [ebp+NTFS.cur_size], 0
        mov     eax, [ebp+NTFS.frs_buffer]
        mov     [ebp+NTFS.cur_buf], eax
        call    ntfs_read_attr
        pop     eax
        jc      ntfsFail
        cmp     eax, [ebp+NTFS.mftSize]
        jnc     .extendMFT
        jmp     .mftRecord

.extendBitmapMFT:
        mov     eax, [ebp+NTFS.sectors_per_cluster]
        mov     [ebp+NTFS.cur_offs], eax
        shl     eax, 9
        cmp     [ebp+NTFS.mftBitmapSize], eax
        jnc     ntfsUnsupported
        mov     [ebp+NTFS.cur_iRecord], 0
        mov     [ebp+NTFS.cur_attr], 0xB0
        mov     [ebp+NTFS.cur_size], 0
        call    ntfs_read_attr
        jc      ntfsFail
        mov     eax, [ebp+NTFS.mft_cluster]
        mul     [ebp+NTFS.sectors_per_cluster]
        cmp     eax, [ebp+NTFS.LastRead]
        jnz     ntfsUnsupported     ; auxiliary record
        mov     edi, [ebp+NTFS.mftBitmapBuffer]
        mov     ecx, [ebp+NTFS.mftBitmapSize]
        add     edi, ecx
        mov     eax, ecx
        mov     edx, [ebp+NTFS.attr_offs]
        add     ecx, 8
        mov     [edx+attributeRealSize], ecx
        mov     [edx+initialDataSize], ecx
        shl     eax, 3
        mov     [ebp+NTFS.newRecord], eax
        mov     dword [edi], 1
        mov     dword [edi+4], 0
        mov     [ebp+NTFS.cur_attr], 0x80
        mov     [ebp+NTFS.cur_offs], 0
        call    ntfs_read_attr.newAttribute
        jc      ntfsFail
        mov     [ebp+NTFS.mftBitmapSize], ecx
.extendMFT:
        mov     eax, [ebp+NTFS.mft_cluster]
        mul     [ebp+NTFS.sectors_per_cluster]
        cmp     eax, [ebp+NTFS.LastRead]
        jnz     ntfsUnsupported     ; auxiliary record
        mov     ecx, [ebp+NTFS.attr_offs]
        mov     eax, [ecx+attributeRealSize]
        mov     edx, [ecx+attributeRealSize+4]
        xor     ax, ax
        add     eax, 10000h
        adc     edx, 0
        push    [ebp+NTFS.fileDataStart]
        push    [ebp+NTFS.fileDataSize]
        call    resizeAttribute
        jc      ntfsErrorPop2
        mov     ebx, [ebp+NTFS.frs_buffer]
        mov     edx, [ebp+NTFS.LastRead]
        call    writeRecord     ; $MFT
        mov     eax, [ebp+NTFS.mftmirr_cluster]
        mul     [ebp+NTFS.sectors_per_cluster]
        mov     ecx, [ebp+NTFS.frs_size]
        shr     ecx, 9
        call    fs_write64_sys  ; $MFTMirr
; update $MFT retrieval information
        mov     edi, [ebp+NTFS.mft_retrieval_end]
        mov     eax, [edi-4]
        add     eax, [edi-8]
        mov     edx, [ebp+NTFS.fileDataSize]
        cmp     eax, [ebp+NTFS.fileDataStart]
        jnz     .newFragment
        add     [edi-8], edx
        jmp     @f
.newFragment:
        lea     eax, [ebp+NTFS.attrlist_buf]
        cmp     eax, edi
        jz      @f
        mov     [edi], edx
        mov     eax, [ebp+NTFS.fileDataStart]
        mov     [edi+4], eax
        add     [ebp+NTFS.mft_retrieval_end], 8
@@:
        mov     eax, [ebp+NTFS.fileDataSize]
        mul     [ebp+NTFS.sectors_per_cluster]
        add     [ebp+NTFS.mftSize], eax
        call    ntfsSpaceClean
        pop     [ebp+NTFS.fileDataSize]
        pop     [ebp+NTFS.fileDataStart]
.mftRecord:
        mov     ecx, [ebp+NTFS.frs_size]
        shr     ecx, 2
        mov     edi, [ebp+NTFS.frs_buffer]
        xor     eax, eax
        rep stosd
        mov     esi, [ebp+NTFS.indexPointer]
        mov     eax, [ebp+NTFS.newRecord]
        mov     [esi+fileRecordReference], eax
        rdtsc
        mov     [esi+fileReferenceReuse], ax
        mov     edi, [ebp+NTFS.frs_buffer]
; record header
        mov     [edi+reuseCounter], ax
        mov     [edi+2ah], ax
        mov     eax, [ebp+NTFS.frs_size]
        mov     [edi+recordAllocatedSize], eax
        shr     eax, 9
        inc     eax
        mov     [edi+updateSequenceSize], al
        shl     eax, 1
        add     eax, 2ah+7
        and     eax, not 7
        mov     dword[edi], 'FILE'
        mov     byte [edi+updateSequenceOffset], 2ah
        mov     byte [edi+hardLinkCounter], 1
        mov     byte [edi+newAttributeID], 3
        mov     [edi+attributeOffset], al
        add     edi, eax
; $StandardInformation
        mov     byte [edi+attributeType], 10h
        mov     byte [edi+sizeWithHeader], 48h
        mov     byte [edi+sizeWithoutHeader], 30h
        mov     byte [edi+attributeOffset], 18h
        mov     cl, 8
        add     esi, fileCreated
        add     edi, 18h
        rep movsd
        add     edi, 16
        mov     esi, [ebp+NTFS.indexPointer]
; $FileName
        mov     byte [edi+attributeType], 30h
        mov     byte [edi+attributeID], 1
        mov     byte [edi+attributeOffset], 18h
        mov     byte [edi+indexedFlag], 1
        mov     cx, [esi+indexRawSize]
        mov     [edi+sizeWithoutHeader], ecx
        mov     cx, [esi+indexAllocatedSize]
        add     ecx, 8
        mov     [edi+sizeWithHeader], ecx
        add     edi, 18h
        add     esi, 16
        sub     ecx, 18h
        shr     ecx, 2
        rep movsd
        mov     byte [edi+sizeWithHeader], 50h
        mov     byte [edi+attributeID], 2
        cmp     [ebp+NTFS.bFolder], 1
        jz      .indexRoot
; $Data
        mov     byte [edi+attributeType], 80h
        mov     eax, [ebp+NTFS.fileDataSize]
        test    eax, eax
        jz      .resident
        mov     esi, [ebp+NTFS.indexPointer]
        dec     eax
        mov     [edi+lastVCN], eax
        mov     byte [edi+nonResidentFlag], 1
        mov     byte [edi+dataRunsOffset], 40h
        mov     eax, [esi+fileAllocatedSize]
        mov     [edi+attributeAllocatedSize], eax
        mov     eax, [esi+fileRealSize]
        mov     [edi+attributeRealSize], eax
        mov     [edi+initialDataSize], eax
        push    edi
        mov     esi, edi
        add     edi, 40h
        call    createMcbEntry
        inc     edi
        jmp     @f

.resident:
        mov     ecx, [ebp+NTFS.fileRealSize]
        mov     [edi+sizeWithoutHeader], ecx
        mov     byte [edi+attributeOffset], 18h
        push    edi
        mov     esi, [ebp+NTFS.fileDataBuffer]
        add     edi, 18h
        rep movsb
@@:
        mov     eax, edi
        pop     edi
        sub     eax, edi
        add     eax, 7
        and     eax, not 7
        mov     [edi+sizeWithHeader], eax
        add     edi, eax
        mov     al, 1
        jmp     .end

.indexRoot:
        mov     byte [edi+attributeType], 90h
        mov     byte [edi+nameLength], 4
        mov     byte [edi+nameOffset], 18h
        mov     byte [edi+sizeWithoutHeader], 30h
        mov     byte [edi+attributeOffset], 20h
        mov     dword[edi+18h], 490024h     ; unicode $I30
        mov     dword[edi+18h+4], 300033h
        mov     byte [edi+20h+indexedAttributesType], 30h
        mov     byte [edi+20h+collationRule], 1
        mov     eax, [ebp+NTFS.sectors_per_cluster]
        mov     dl, 1
        shl     eax, 8
@@:
        shl     eax, 1
        shl     edx, 1
        cmp     eax, [ebp+NTFS.frs_size]
        jc      @b
        shr     edx, 1
        mov     [edi+20h+indexRecordSize], eax
        mov     [edi+20h+indexRecordSizeClus], dl
        mov     byte [edi+30h+indexOffset], 16
        mov     byte [edi+30h+nodeRealSize], 32
        mov     byte [edi+30h+nodeAllocatedSize], 32
        mov     byte [edi+40h+indexAllocatedSize], 16
        mov     byte [edi+40h+indexFlags], 2
        add     edi, 50h
        mov     al, 3
.end:
        mov     ebx, [ebp+NTFS.frs_buffer]
        mov     dword [edi], -1
        mov     dword [edi+4], 0
        add     edi, 8
        sub     edi, ebx
        mov     [ebx+recordFlags], al
        mov     [ebx+recordRealSize], edi
        mov     edx, [ebp+NTFS.LastRead]
        call    writeRecord
; write MFT bitmap
        mov     eax, [ebp+NTFS.newRecord]
        shr     eax, 3+9
        mov     ebx, eax
        shl     ebx, 9
        add     eax, [ebp+NTFS.mftBitmapLocation]
        add     ebx, [ebp+NTFS.mftBitmapBuffer]
        mov     ecx, 1
        xor     edx, edx
        call    fs_write64_sys
; 5. Write directory node
        mov     ebx, [ebp+NTFS.cur_index_buf]
        mov     edx, [ebp+NTFS.nodeLastRead]
        call    writeRecord
        mov     ebx, [ebp+NTFS.fileRealSize]
ntfsDone:
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
        call    ntfs_unlock
        xor     eax, eax
        ret

writeRecord:
; make updateSequence and write to disk
;   in:
; ebx -> record
; edx = partition sector
        mov     esi, ebx
        mov     edi, ebx
        movzx   ecx, word [esi+updateSequenceOffset]
        add     edi, ecx
        mov     ax, [edi]
        inc     ax
        stosw
        mov     cx, [esi+updateSequenceSize]
        dec     ecx
        push    ecx
@@:
        add     esi, 510
        movsw
        mov     [esi-2], ax
        loop    @b
        mov     eax, edx
        xor     edx, edx
        pop     ecx
        jmp     fs_write64_sys

createMcbEntry:
;   in:
; [ebp+NTFS.fileDataStart] = position value
; [ebp+NTFS.fileDataSize] = size value
; edi -> destination
; esi -> attribute header
        mov     eax, [ebp+NTFS.fileDataStart]
        xor     edx, edx
        shl     eax, 1
        jnc     @f
        not     eax
@@:
        inc     edx
        shr     eax, 8
        jnz     @b
        mov     eax, [ebp+NTFS.fileDataSize]
        shl     eax, 1
        xor     ecx, ecx
@@:
        inc     ecx
        shr     eax, 8
        jnz     @b
        lea     eax, [edi+edx+1]
        add     eax, ecx
        sub     eax, esi
        sub     eax, [esi+sizeWithHeader]
        jc      @f
        add     word [esi+sizeWithHeader], 8    ; extend attribute
        mov     esi, [ebp+NTFS.frs_buffer]
        mov     eax, [esi+recordRealSize]
        add     eax, 8
        cmp     [esi+recordAllocatedSize], eax
        jc      .end    ; no space in the record
        mov     [esi+recordRealSize], eax
        push    ecx edi
        add     esi, eax
        mov     ecx, esi
        sub     ecx, edi
        sub     ecx, 8
        shr     ecx, 2
        mov     edi, esi
        sub     edi, 4
        sub     esi, 12
        std
        rep movsd
        cld
        pop     edi ecx
@@:
        mov     eax, edx
        shl     eax, 4
        add     eax, ecx
        stosb
        lea     esi, [ebp+NTFS.fileDataSize]
        rep movsb
        lea     esi, [ebp+NTFS.fileDataStart]
        mov     ecx, edx
        rep movsb
        mov     [edi], cl
.end:
        ret

resizeAttribute:
;   in:
; [ebp+NTFS.frs_buffer] -> file record
; [ebp+NTFS.attr_offs] -> attribute
; edx:eax = new size
;   out:
; [ebp+NTFS.fileDataSize] = clusters added (positive)
; [ebp+NTFS.fileDataStart] = added block
; CF=1 -> eax = error code
        mov     esi, [ebp+NTFS.attr_offs]
        mov     dword [ebp+NTFS.attr_size], eax
        mov     dword [ebp+NTFS.attr_size+4], edx
        cmp     byte [esi+nonResidentFlag], 0
        jz      .resident
        mov     ecx, [ebp+NTFS.sectors_per_cluster]
        shl     ecx, 9
        mov     [esi+attributeRealSize], eax
        mov     [esi+attributeRealSize+4], edx
        mov     [esi+initialDataSize], eax
        mov     [esi+initialDataSize+4], edx
        sub     eax, 1
        sbb     edx, 0
        jc      .makeResident
        div     ecx
        mov     edi, eax
        inc     eax
        mul     ecx
        mov     [esi+attributeAllocatedSize], eax
        mov     [esi+attributeAllocatedSize+4], edx
        mov     ecx, [esi+lastVCN]
        mov     [esi+lastVCN], edi
        movzx   eax, byte [esi+dataRunsOffset]
        sub     edi, ecx
        mov     [ebp+NTFS.fileDataSize], edi
        jz      .done
        jc      .shrinkAttribute
; extend attribute
        xor     edi, edi
        add     esi, eax
        push    edi edi edi edi
@@:
        mov     edx, eax
        mov     eax, esi
        add     edi, [esp+8]
        call    ntfs_decode_mcb_entry
        jc      @b
        mov     [esp+4], edx
        mov     [esp+12], edi
        add     edi, [esp]
        push    edi
        shr     edi, 5
        shl     edi, 2
        push    eax
        cmp     edi, [ebp+NTFS.BitmapStart]
        jnc     @f
        cmp     [ebp+NTFS.cur_iRecord], 0
        jz      @f
        mov     edi, [ebp+NTFS.BitmapStart]
@@:
        call    ntfsSpaceAlloc
        jc      .err1
        mov     eax, [ebp+NTFS.fileDataStart]
        pop     edi
        pop     edx
        cmp     edx, eax
        jnz     .newEntry
        pop     edx
        pop     edi
        pop     [ebp+NTFS.fileDataStart]
        mov     [esp], eax
        push    [ebp+NTFS.fileDataSize]
        add     [ebp+NTFS.fileDataSize], edx
        jmp     @f

.newEntry:
        add     esp, 12
        pop     edx
        push    eax
        push    [ebp+NTFS.fileDataSize]
        sub     eax, edx
        mov     [ebp+NTFS.fileDataStart], eax
@@:
        mov     esi, [ebp+NTFS.attr_offs]
        call    createMcbEntry
        pop     [ebp+NTFS.fileDataSize]
        pop     [ebp+NTFS.fileDataStart]
        movi    eax, ERROR_UNSUPPORTED_FS
.done:
        ret

.err1:
        add     esp, 24
        stc
.err2:
        movi    eax, ERROR_DISK_FULL
        ret

.err3:
        movi    eax, ERROR_FS_FAIL
        add     esp, 20
        stc
        ret

.shrinkAttribute:
        add     ecx, edi
        inc     ecx
        add     esi, eax
        xor     edi, edi
        sub     esp, 20
@@:
        mov     [esp+16], esi
        call    ntfs_decode_mcb_entry
        jnc     .err3
        add     edi, [esp+8]
        sub     ecx, [esp]
        jnc     @b
        mov     ebx, ecx
        add     ecx, [esp]
        mov     eax, [esp+8]
        mov     [ebp+NTFS.fileDataSize], ecx
        mov     [ebp+NTFS.fileDataStart], eax
        push    edi
        add     edi, ecx
        neg     ebx
        call    ntfsSpaceFree
        pop     edi
        jc      .end
@@:
        call    ntfs_decode_mcb_entry
        jnc     .end
        cmp     dword[esp+8], 0
        jz      @b
        add     edi, [esp+8]
        mov     ebx, [esp]
        call    ntfsSpaceFree
        jnc     @b
.end:
        add     esp, 16
        pop     edi
        cmp     [ebp+NTFS.fileDataSize], 0
        jz      @f
        mov     esi, [ebp+NTFS.attr_offs]
        call    createMcbEntry
        mov     [ebp+NTFS.fileDataSize], 0
@@:
        ret

.resident:
        test    edx, edx
        jnz     .nonResident
        cmp     eax, 8000h
        jnc     .nonResident
        add     ax, [esi+attributeOffset]
        sub     eax, [esi+sizeWithHeader]
        jc      @f
        mov     edi, [ebp+NTFS.frs_buffer]
        mov     ecx, eax
        add     ecx, [edi+recordRealSize]
        cmp     [edi+recordAllocatedSize], ecx
        jc      .nonResident
        add     eax, 7
        and     eax, not 7
        add     [edi+recordRealSize], eax
        add     edi, [edi+recordRealSize]
        add     [esi+sizeWithHeader], eax
        add     esi, [esi+sizeWithHeader]
        mov     ecx, edi
        sub     ecx, esi
        shr     ecx, 2
        sub     edi, 4
        mov     esi, edi
        sub     esi, eax
        std
        rep movsd
        mov     ecx, eax
        shr     ecx, 2
        xor     eax, eax
        rep stosd
        cld
        mov     esi, [ebp+NTFS.attr_offs]
@@:
        mov     eax, dword [ebp+NTFS.attr_size]
        mov     [esi+sizeWithoutHeader], eax
        mov     [ebp+NTFS.fileDataSize], 0
        clc
        ret

.nonResident:   ; convert resident to non-resident
        mov     eax, dword [ebp+NTFS.attr_size]
        sub     eax, 1
        sbb     edx, 0
        mov     ecx, [ebp+NTFS.sectors_per_cluster]
        shl     ecx, 9
        div     ecx
        inc     eax
        mov     [ebp+NTFS.fileDataSize], eax
        mov     edi, [ebp+NTFS.BitmapStart]
        push    ecx
        call    ntfsSpaceAlloc
        pop     ecx
        jc      .err2
        mov     esi, [ebp+NTFS.attr_offs]
        xor     eax, eax
        xor     edx, edx
@@:
        add     eax, ecx
        inc     edx
        cmp     eax, [esi+sizeWithoutHeader]
        jc      @b
        push    edx
        push    eax
        stdcall kernel_alloc, eax
        mov     ecx, [esp]
        shr     ecx, 2
        mov     edi, eax
        mov     ebx, eax
        xor     eax, eax
        rep stosd
        mov     al, [esi+attributeOffset]
        mov     ecx, [esi+sizeWithoutHeader]
        add     esi, eax
        mov     edi, ebx
        rep movsb
        mov     eax, [ebp+NTFS.fileDataStart]
        mul     [ebp+NTFS.sectors_per_cluster]
        pop     ecx
        shr     ecx, 9
        call    fs_write64_app
        stdcall kernel_free, ebx
        mov     esi, [ebp+NTFS.attr_offs]
        add     esi, [esi+sizeWithHeader]
        mov     ecx, [ebp+NTFS.frs_buffer]
        add     ecx, [ecx+recordRealSize]
        sub     ecx, esi
        shr     ecx, 2
        lea     edi, [ebp+NTFS.bitmap_buf]
        push    ecx
        rep movsd
        mov     edi, [ebp+NTFS.attr_offs]
        add     edi, 16
        mov     cl, 6
        xor     eax, eax
        rep stosd
        mov     edi, [ebp+NTFS.attr_offs]
        mov     eax, [ebp+NTFS.fileDataSize]
        dec     eax
        mov     [edi+lastVCN], eax
        inc     eax
        mov     ecx, [ebp+NTFS.sectors_per_cluster]
        shl     ecx, 9
        mul     ecx
        mov     byte [edi+sizeWithHeader], 50h
        mov     byte [edi+nonResidentFlag], 1
        mov     byte [edi+dataRunsOffset], 40h
        mov     [edi+attributeAllocatedSize], eax
        mov     [edi+attributeAllocatedSize+4], edx
        mov     eax, dword [ebp+NTFS.attr_size]
        mov     edx, dword [ebp+NTFS.attr_size+4]
        mov     [edi+attributeRealSize], eax
        mov     [edi+attributeRealSize+4], edx
        mov     [edi+initialDataSize], eax
        mov     [edi+initialDataSize+4], edx
        mov     esi, edi
        add     edi, 40h
        call    createMcbEntry
        mov     eax, edi
        mov     edi, [ebp+NTFS.attr_offs]
        sub     eax, edi
        add     eax, 8
        and     eax, not 7
        mov     [edi+sizeWithHeader], eax
        pop     ecx
        lea     esi, [ebp+NTFS.bitmap_buf]
        add     edi, eax
        rep movsd
        mov     esi, [ebp+NTFS.frs_buffer]
        sub     edi, esi
        mov     [esi+recordRealSize], edi
        pop     edx
        sub     [ebp+NTFS.fileDataSize], edx
        add     [ebp+NTFS.fileDataStart], edx
        ret

.makeResident:  ; convert non-resident to empty resident
        movzx   eax, byte [esi+dataRunsOffset]
        mov     byte [esi+nonResidentFlag], 0
        mov     dword [esi+sizeWithoutHeader], 0
        mov     dword [esi+attributeOffset], 18h
        add     esi, eax
        xor     edi, edi
        sub     esp, 16
@@:
        call    ntfs_decode_mcb_entry
        jnc     @f
        cmp     dword[esp+8], 0
        jz      @b
        add     edi, [esp+8]
        mov     ebx, [esp]
        call    ntfsSpaceFree
        jnc     @b
@@:
        add     esp, 16
        mov     [ebp+NTFS.fileDataSize], 0
        ret

ntfsSpaceClean:
; clean up to 16 Mb of disk space
;   in:
; [ebp+NTFS.fileDataStart] = block to clean
; [ebp+NTFS.fileDataSize] = block size
        mov     eax, [ebp+NTFS.fileDataSize]
        test    eax, eax
        jz      @f
        mul     [ebp+NTFS.sectors_per_cluster]
        cmp     eax, 8001h
        jnc     @f
        push    eax
        shl     eax, 9
        stdcall kernel_alloc, eax
        pop     ecx
        test    eax, eax
        jz      @f
        push    ecx
        shl     ecx, 7
        mov     edi, eax
        mov     ebx, eax
        xor     eax, eax
        rep stosd
        mov     eax, [ebp+NTFS.fileDataStart]
        mul     [ebp+NTFS.sectors_per_cluster]
        mov     [ebp+NTFS.LastRead], eax
        pop     ecx
        call    fs_write64_app
        stdcall kernel_free, ebx
@@:
        ret

ntfsSpaceAlloc:
; allocate disk space
;   in:
; edi = offset in bitmap to start search from
; [ebp+NTFS.fileDataSize] = block size in clusters
;   out:
; [ebp+NTFS.fileDataStart] = allocated block starting cluster
; CF=1 -> disk full
        mov     ecx, [ebp+NTFS.BitmapBuffer]
        add     edi, ecx
        add     ecx, [ebp+NTFS.BitmapSize]
        sub     ecx, edi
        ja      @f
        push    eax
        call    bitmapBuffering
        pop     eax
        shl     ecx, 2
@@:
        shr     ecx, 2
        push    ecx
        mov     eax, [ebp+NTFS.fileDataSize]
        shr     eax, 5
        jz      .small
        mov     ebx, eax    ; bitmap dwords
.start:
        mov     ecx, [ebp+NTFS.BitmapBuffer]
        add     ecx, [ebp+NTFS.BitmapSize]
        sub     ecx, edi
        shr     ecx, 2
@@:
        xor     eax, eax
        repnz scasd         ; search for empty dword
        jz      @f
        call    bitmapBuffering
        jmp     @b
@@:
        cmp     ecx, ebx
        jnc     @f
        call    bitmapBuffering
        jmp     @b
@@:
        sub     edi, 4
        mov     ecx, ebx
        mov     esi, edi
        xor     eax, eax
        repz scasd          ; check following dwords
        jnz     .start
        sub     esi, 4
        mov     eax, [esi]
        xor     edx, edx
        bsr     edx, eax
        inc     edx
        push    edx         ; starting bit
        push    esi         ; starting dword
        add     esi, 4
        neg     edx
        add     edx, 32
        mov     eax, [ebp+NTFS.fileDataSize]
        sub     eax, edx
        mov     edx, eax
        shr     eax, 5
        shl     eax, 2
        add     esi, eax
        mov     eax, [esi]
        bsf     ecx, eax    ; check last dword
        jz      .done
        and     edx, 31
        cmp     ecx, edx
        jnc     .done
        add     esp, 8
        jmp     .start

@@:
        sub     edi, 4
        call    bitmapBuffering
        push    ecx
.small:     ; less than 32 clusters
        pop     ecx
        or      eax, -1
        repz scasd
        jecxz   @b
        push    ecx
        mov     eax, [edi-4]
        not     eax
@@:
        bsf     ecx, eax    ; first 0
        jz      .small
        not     eax
        shr     eax, cl
        shl     eax, cl
        bsf     edx, eax    ; next 1
        jz      @f
        sub     edx, ecx
        cmp     edx, [ebp+NTFS.fileDataSize]
        jnc     .got        ; fits inside
        bsf     ecx, eax
        not     eax
        shr     eax, cl
        shl     eax, cl
        jmp     @b

@@:         ; next dword
        mov     eax, [edi]
        bsf     edx, eax
        jz      .got        ; empty
        add     edx, 32
        sub     edx, ecx
        cmp     edx, [ebp+NTFS.fileDataSize]
        jc      .small
.got:
        sub     edi, 4
        push    ecx         ; starting bit
        push    edi         ; starting dword
.done:      ; mark space
        pop     edi ecx
        cmp     ecx, 32
        jc      @f
        xor     ecx, ecx
        add     edi, 4
@@:
        push    ecx edi
        or      eax, -1
        shr     eax, cl
        shl     eax, cl
        neg     ecx
        add     ecx, 32
        sub     ecx, [ebp+NTFS.fileDataSize]
        jc      @f
        shl     eax, cl     ; fits inside dword
        shr     eax, cl
        or      [edi], eax
        jmp     .end

@@:
        or      [edi], eax
        neg     ecx
        push    ecx
        shr     ecx, 5
        add     edi, 4
        or      eax, -1
        rep stosd
        pop     ecx
        and     ecx, 31
        shr     eax, cl
        shl     eax, cl
        not     eax
        or      [edi], eax
.end:
        pop     eax
        pop     ecx
        sub     eax, [ebp+NTFS.BitmapBuffer]
        shl     eax, 3
        add     eax, ecx
        pop     ecx
        mov     ecx, [ebp+NTFS.fileDataSize]
        mov     [ebp+NTFS.fileDataStart], eax
        add     ecx, eax
        add     ecx, 4095
        shr     ecx, 3+9
        shr     eax, 3+9
        sub     ecx, eax
        mov     ebx, eax
        shl     ebx, 9
        add     eax, [ebp+NTFS.BitmapLocation]
        add     ebx, [ebp+NTFS.BitmapBuffer]
        xor     edx, edx
        jmp     fs_write64_app

ntfsSpaceFree:
; free disk space
;   in:
; edi = starting cluster
; ebx = size in clusters
        mov     eax, edi
        add     eax, ebx
        shr     eax, 3
        cmp     eax, [ebp+NTFS.BitmapSize]
        jc      @f
        add     eax, [ebp+NTFS.BitmapBuffer]
        push    edi
        mov     edi, eax
        call    bitmapBuffering
        pop     edi
@@:
        push    edi
        mov     ecx, edi
        shr     edi, 5
        shl     edi, 2
        add     edi, [ebp+NTFS.BitmapBuffer]
        and     ecx, 31
        xor     eax, eax
        dec     eax
        shr     eax, cl
        shl     eax, cl
        neg     ecx
        add     ecx, 32
        sub     ecx, ebx
        jc      @f
        shl     eax, cl     ; fits inside dword
        shr     eax, cl
        not     eax
        and     [edi], eax
        jmp     .writeBitmap

@@:
        not     eax
        and     [edi], eax
        neg     ecx
        push    ecx
        shr     ecx, 5
        add     edi, 4
        xor     eax, eax
        rep stosd
        pop     ecx
        and     ecx, 31
        dec     eax
        shr     eax, cl
        shl     eax, cl
        and     [edi], eax
.writeBitmap:
        pop     eax
        mov     edi, eax
        lea     ecx, [eax+ebx+4095]
        shr     eax, 3+9
        shr     ecx, 3+9
        sub     ecx, eax
        mov     ebx, eax
        shl     ebx, 9
        add     eax, [ebp+NTFS.BitmapLocation]
        add     ebx, [ebp+NTFS.BitmapBuffer]
        xor     edx, edx
        jmp     fs_write64_app

bitmapBuffering:
; Extend BitmapBuffer and read next 32kb of bitmap
; Warning: $Bitmap fragmentation is not foreseen
; in: edi -> position in bitmap buffer
; out: ecx = number of buffered dwords left
        push    ebx
        mov     eax, [ebp+NTFS.BitmapTotalSize]
        cmp     eax, [ebp+NTFS.BitmapSize]
        jz      .end
        stdcall alloc_pages, 8
        test    eax, eax
        jz      .end
        add     eax, 3
        mov     ebx, [ebp+NTFS.BitmapBuffer]
        add     ebx, [ebp+NTFS.BitmapSize]
        push    ebx
        mov     ecx, 8
        call    commit_pages
        mov     eax, [ebp+NTFS.BitmapSize]
        shr     eax, 9
        add     eax, [ebp+NTFS.BitmapLocation]
        pop     ebx
        mov     ecx, 64
        xor     edx, edx
        call    fs_read64_app
        test    eax, eax
        jnz     .err
        mov     eax, [ebp+NTFS.BitmapSize]
        add     eax, 8000h
        cmp     [ebp+NTFS.BitmapTotalSize], eax
        jnc     @f
        mov     eax, [ebp+NTFS.BitmapTotalSize]
@@:
        mov     [ebp+NTFS.BitmapSize], eax
        pop     ebx
        mov     ecx, [ebp+NTFS.BitmapBuffer]
        add     ecx, eax
        sub     ecx, edi
        jbe     bitmapBuffering
        shr     ecx, 2
        ret

.err:
        mov     eax, [ebp+NTFS.BitmapBuffer]
        add     eax, [ebp+NTFS.BitmapSize]
        mov     ecx, 8
        call    release_pages
.end:
        add     esp, 12     ; ret
        stc
        ret

;----------------------------------------------------------------
ntfs_WriteFile:
        call    ntfs_lock
        call    ntfs_find_lfn
        jc      ntfsNotFound
        cmp     [ebp+NTFS.cur_iRecord], 16
        jc      ntfsDenied
        test    dword [eax+fileFlags], 10000001h
        jnz     ntfsDenied
        cmp     [ebp+NTFS.fragmentCount], 1
        jnz     ntfsUnsupported     ; record fragmented
; edit directory node
        mov     edi, [ebp+NTFS.cur_index_buf]
        cmp     dword [edi], 'INDX'
        jz      @f
        mov     esi, [ebp+NTFS.frs_buffer]
        mov     ecx, [esi+recordRealSize]
        shr     ecx, 2
        rep movsd
        mov     esi, [ebp+NTFS.attr_offs]
        mov     cl, [esi+attributeOffset]
        sub     esi, [ebp+NTFS.frs_buffer]
        add     eax, ecx
        add     eax, esi
@@:
        mov     edi, eax
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        add     eax, [ebx+12]
        adc     edx, 0

        cmp     edx, [edi+fileRealSize+4]
        jc      @f
        jnz     .resize
        cmp     [edi+fileRealSize], eax
        jnc     @f
.resize:
        mov     [edi+fileRealSize], eax
        mov     [edi+fileRealSize+4], edx
@@:
        push    edx eax ebx
        call    ntfsGetTime
        mov     [edi+fileModified], eax
        mov     [edi+fileModified+4], edx
        mov     [edi+recordModified], eax
        mov     [edi+recordModified+4], edx
        mov     [edi+fileAccessed], eax
        mov     [edi+fileAccessed+4], edx
        pop     ebx ecx edx
        mov     eax, [ebp+NTFS.LastRead]
        mov     [ebp+NTFS.nodeLastRead], eax
        mov     [ebp+NTFS.cur_attr], 0x80
        mov     [ebp+NTFS.cur_offs], 0
        mov     [ebp+NTFS.cur_size], 0
        call    ntfs_read_attr
        jc      ntfsFail
        mov     esi, edi
        mov     edi, [ebp+NTFS.frs_buffer]
        cmp     word [edi+baseRecordReuse], 0
        jnz     ntfsUnsupported     ; auxiliary record
        mov     al, [edi+attributeOffset]
        add     edi, eax
        mov     al, [edi+attributeOffset]
        add     edi, eax
        mov     eax, ecx
        mov     ecx, 6
        add     esi, fileModified
        add     edi, 8
        rep movsd
        mov     ecx, [ebp+NTFS.attr_offs]
        cmp     word [ecx+attributeFlags], 0
        jnz     ntfsUnsupported
        push    ebx
        cmp     byte [ecx+nonResidentFlag], 0
        jz      .resizeAttribute
        cmp     edx, [ecx+attributeRealSize+4]
        jc      .writeNode
        jnz     .resizeAttribute
        cmp     [ecx+attributeRealSize], eax
        jnc     .writeNode
.resizeAttribute:
        call    resizeAttribute
        jc      ntfsErrorPop
        mov     ecx, [ebp+NTFS.attr_offs]
        cmp     byte [ecx+nonResidentFlag], 1
        jz      @f
        mov     ebx, [esp]
        movzx   edi, byte [ecx+attributeOffset]
        add     edi, ecx
        add     edi, [ebx+4]
        mov     ecx, [ebx+12]
        mov     esi, [ebx+16]
        rep movsb
@@:
        mov     ebx, [ebp+NTFS.frs_buffer]
        mov     edx, [ebp+NTFS.mftLastRead]
        call    writeRecord     ; file
        call    ntfs_restore_usa_frs
.writeNode:
        mov     ebx, [ebp+NTFS.cur_index_buf]
        mov     edx, [ebp+NTFS.nodeLastRead]
        call    writeRecord     ; directory
        pop     ebx
        mov     ecx, [ebp+NTFS.attr_offs]
        cmp     byte [ecx+nonResidentFlag], 0
        jz      .done
        mov     ecx, [ebx+12]
        test    ecx, ecx
        jz      .done
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        mov     esi, [ebx+16]
        shrd    eax, edx, 9
        test    dword[ebx+4], 1FFh
        jz      .aligned
        mov     [ebp+NTFS.cur_offs], eax
        mov     [ebp+NTFS.cur_size], 1
        lea     edi, [ebp+NTFS.bitmap_buf]
        mov     [ebp+NTFS.cur_buf], edi
        call    ntfs_read_attr.continue
        jc      ntfsDevice
        mov     eax, [ebx+4]
        and     eax, 1FFh
        add     edi, eax
        sub     eax, [ebp+NTFS.cur_read]
        neg     eax
        push    ecx
        cmp     ecx, eax
        jb      @f
        mov     ecx, eax
@@:
        sub     [esp], ecx
        rep movsb
        push    ebx
        mov     eax, [ebp+NTFS.LastRead]
        lea     ebx, [ebp+NTFS.bitmap_buf]
        mov     ecx, 1
        xor     edx, edx
        call    fs_write64_app
        pop     ebx
        pop     ecx
        test    ecx, ecx
        jz      .done
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        shrd    eax, edx, 9
        inc     eax
.aligned:
        push    ecx
        shr     ecx, 9
        mov     [ebp+NTFS.cur_offs], eax
        mov     [ebp+NTFS.cur_size], ecx
        mov     [ebp+NTFS.cur_buf], esi
        add     eax, ecx
        push    eax
        mov     [ebp+NTFS.bWriteAttr], 1
        call    ntfs_read_attr.continue
        mov     [ebp+NTFS.bWriteAttr], 0
        pop     [ebp+NTFS.cur_offs]
        pop     ecx
        jc      ntfsDevice
        and     ecx, 1FFh
        jz      .done
        add     esi, [ebp+NTFS.cur_read]
        mov     [ebp+NTFS.cur_size], 1
        lea     edi, [ebp+NTFS.bitmap_buf]
        mov     [ebp+NTFS.cur_buf], edi
        call    ntfs_read_attr.continue
        jc      ntfsDevice
        rep movsb
        push    ebx
        mov     eax, [ebp+NTFS.LastRead]
        lea     ebx, [ebp+NTFS.bitmap_buf]
        mov     ecx, 1
        xor     edx, edx
        call    fs_write64_app
        pop     ebx
.done:
        mov     ebx, [ebx+12]
        jmp     ntfsDone

;----------------------------------------------------------------
ntfs_Delete:
        call    ntfs_lock
        call    ntfs_find_lfn
        jc      ntfsNotFound
        cmp     [ebp+NTFS.cur_iRecord], 16
        jc      ntfsDenied
        test    byte [eax+fileFlags], 1
        jnz     ntfsDenied
        cmp     [ebp+NTFS.fragmentCount], 1
        jnz     ntfsUnsupported     ; record fragmented
        mov     ebx, [eax+directoryRecordReference]
        mov     [ebp+NTFS.newRecord], ebx
        mov     bx, [eax+fileReferenceReuse]
        mov     [ebp+NTFS.indexPointer], esi
        mov     eax, [ebp+NTFS.cur_iRecord]
        shr     eax, 3
        cmp     eax, [ebp+NTFS.mftBitmapSize]
        jnc     ntfsUnsupported
; examine file record
        mov     [ebp+NTFS.cur_attr], 0x80   ; file?
        mov     [ebp+NTFS.cur_offs], 0
        mov     [ebp+NTFS.cur_size], 0
        call    ntfs_read_attr
        jnc     @f
        xor     eax, eax
        push    ebx eax eax eax eax
        mov     [esp+12], esp
        push    eax
        mov     ebx, esp
        mov     [ebp+NTFS.cur_attr], 0x90   ; folder?
        call    ntfs_ReadFolder.doit
        mov     edx, [esp+12]
        add     esp, 20
        pop     ebx
        test    eax, eax
        jnz     .ret
        cmp     edx, 2
        jnz     ntfsDenied      ; folder is not empty
        mov     [ebp+NTFS.cur_attr], 0xA0
        mov     [ebp+NTFS.cur_offs], 0
        mov     [ebp+NTFS.cur_size], 0
        call    ntfs_read_attr.newAttribute
        jc      .deleteFileRecord
@@:
        mov     esi, [ebp+NTFS.frs_buffer]
        cmp     word [esi+baseRecordReuse], 0
        jnz     ntfsUnsupported     ; auxiliary record
        cmp     word [esi+reuseCounter], bx
        jnz     .backToIndex        ; broken index
        test    byte [esi+recordFlags], 1
        jz      .writeBitmapMFT     ; record deleted
        cmp     byte [esi+hardLinkCounter], 3
        jnc     ntfsUnsupported
        mov     esi, [ebp+NTFS.attr_offs]
        cmp     byte [esi+nonResidentFlag], 0
        jz      .deleteFileRecord
        movzx   eax, byte [esi+dataRunsOffset]
        add     esi, eax
        xor     edi, edi
        sub     esp, 16
@@:
        call    ntfs_decode_mcb_entry
        jnc     @f
        cmp     dword[esp+8], 0
        jz      @b
        add     edi, [esp+8]
        mov     ebx, [esp]
        call    ntfsSpaceFree
        jnc     @b
@@:
        add     esp, 16
.deleteFileRecord:
        mov     ebx, [ebp+NTFS.frs_buffer]
        mov     byte [ebx+recordFlags], 0
        mov     edx, [ebp+NTFS.mftLastRead]
        call    writeRecord
.writeBitmapMFT:
        mov     eax, [ebp+NTFS.cur_iRecord]
        mov     ecx, eax
        shr     eax, 3
        and     ecx, 7
        mov     edi, [ebp+NTFS.mftBitmapBuffer]
        btr     [edi+eax], ecx
        shr     eax, 9
        mov     ebx, eax
        shl     ebx, 9
        add     eax, [ebp+NTFS.mftBitmapLocation]
        add     ebx, edi
        mov     ecx, 1
        xor     edx, edx
        call    fs_write64_sys
.backToIndex:
        mov     eax, [ebp+NTFS.newRecord]
        mov     [ebp+NTFS.cur_iRecord], eax
        mov     esi, [ebp+NTFS.indexPointer]
        call    ntfs_find_lfn.doit2
        jc      ntfsFail
        mov     ebx, [ebp+NTFS.secondIndexBuffer]
        mov     byte [ebx], 0
        mov     ebx, [ebp+NTFS.LastRead]
        mov     [ebp+NTFS.nodeLastRead], ebx
        xor     ebx, ebx
        test    byte [eax+indexFlags], 1
        jz      .deleteIndex    ; no subnode
        mov     edi, eax
        call    .findSubindex
        jc      ntfsFail
        movzx   edx, word [edi+indexAllocatedSize]
        test    esi, esi
        jz      @f
        sub     edx, eax
        sub     edx, 8
@@:
        mov     eax, edi
        mov     ebx, esi
        jmp     @f

.deleteIndex:
        movzx   edx, word [eax+indexAllocatedSize]
        mov     ecx, [eax+fileRecordReference]
        cmp     [eax+edx+fileRecordReference], ecx
        jnz     @f
        add     dx, [eax+edx+indexAllocatedSize]
@@:
        mov     edi, [ebp+NTFS.cur_index_buf]
        cmp     dword [edi], 'INDX'
        jz      .indexRecord
        sub     eax, edi
        mov     edi, [ebp+NTFS.indexRoot]
        sub     [edi+sizeWithHeader], edx
        sub     [edi+sizeWithoutHeader], edx
        movzx   ecx, byte [edi+attributeOffset]
        add     edi, ecx
        add     eax, edi
        sub     [edi+rootNode+nodeRealSize], edx
        sub     [edi+rootNode+nodeAllocatedSize], edx
        mov     edi, [ebp+NTFS.frs_buffer]
        sub     [edi+recordRealSize], edx
        mov     ecx, [edi+recordRealSize]
        cmp     [edi+recordAllocatedSize], ecx
        jmp     @f

.indexRecord:
        add     edi, recordNode
        sub     [edi+nodeRealSize], edx
        mov     ecx, [edi+nodeRealSize]
        cmp     [edi+nodeAllocatedSize], ecx
@@:
        jc      ntfsUnsupported
        add     ecx, edi
        sub     ecx, eax
        mov     esi, eax
        add     esi, edx
        mov     edi, eax
        test    edx, edx
        jns     @f
        neg     edx
        add     edx, ecx
        sub     edx, 4
        add     esi, edx
        add     edi, edx
        std
@@:
        jz      @f
        shr     ecx, 2
        rep movsd
        cld
@@:
        test    ebx, ebx
        jz      .done
; copy index from the subnode to replace deleted pointing index
        movzx   ecx, word [ebx+indexAllocatedSize]
        mov     edx, ecx
        test    byte [ebx+indexFlags], 1
        jz      @f
        sub     ecx, 8
        movzx   edi, word [ebx+edx+indexAllocatedSize]
        add     edi, edx
        mov     esi, [ebx+ecx]
        mov     [ebx+edi-8], esi
        mov     [ebx+indexAllocatedSize], cx
@@:
        shr     ecx, 2
        mov     esi, ebx
        mov     edi, eax
        rep movsd
        add     word [eax+indexAllocatedSize], 8
        mov     byte [eax+indexFlags], 1
        mov     edi, [ebp+NTFS.secondIndexBuffer]
        mov     eax, ebx
        xor     ebx, ebx
        jmp     .indexRecord

.done:
        mov     ebx, [ebp+NTFS.frs_buffer]
        mov     edx, [ebp+NTFS.rootLastRead]
        call    writeRecord
        mov     ebx, [ebp+NTFS.cur_index_buf]
        cmp     dword [ebx], 'INDX'
        jnz     @f
        mov     edx, [ebp+NTFS.nodeLastRead]
        call    writeRecord
@@:
        mov     ebx, [ebp+NTFS.secondIndexBuffer]
        cmp     byte [ebx], 0
        jz      ntfsDone
        mov     edx, [ebp+NTFS.LastRead]
        call    writeRecord
        jmp     ntfsDone

.findSubindex:
; in: eax -> index
;   out:
; CF=1 -> error
; esi=0 -> subnode deleted
; esi -> replacement index
; eax = index effective size
        movzx   edx, word [eax+indexAllocatedSize]
        mov     eax, [eax+edx-8]
        mov     edx, [ebp+NTFS.cur_size]
        push    edx
        cmp     edx, [ebp+NTFS.cur_subnode_size]
        jz      @f
        mul     [ebp+NTFS.sectors_per_cluster]
@@:
        mov     [ebp+NTFS.cur_attr], 0xA0
        mov     [ebp+NTFS.cur_offs], eax
        push    eax
        mov     ebx, [ebp+NTFS.secondIndexBuffer]
        mov     esi, ebx
        mov     [ebp+NTFS.cur_buf], ebx
        call    ntfs_read_attr.newAttribute
        pop     [ebp+NTFS.cur_offs]
        pop     eax
        jc      .ret
        cmp     dword [esi], 'INDX'
        stc
        jnz     .ret
        mov     [ebp+NTFS.cur_size], eax
        shl     eax, 9
        call    ntfs_restore_usa
        jc      .ret
        add     esi, recordNode
        add     esi, [esi+indexOffset]
        test    byte [esi+indexFlags], 2
        jnz     .emptyNode
        cmp     [ebp+NTFS.fragmentCount], 1
        stc
        jnz     .ret    ; record fragmented
        xor     eax, eax
@@:
        add     esi, eax
        mov     ax, [esi+indexAllocatedSize]
        test    byte [esi+eax+indexFlags], 2
        jz      @b
        test    byte [esi+indexFlags], 1
        jz      .ret
        add     eax, esi
        push    esi
        push    [ebp+NTFS.cur_offs]
        call    .findSubindex
        pop     [ebp+NTFS.cur_offs]
        pop     edx
        jc      .ret
        test    esi, esi
        jnz     .ret
        mov     esi, edx
        mov     ebx, [ebp+NTFS.secondIndexBuffer]
        mov     [ebp+NTFS.cur_buf], ebx
        push    [ebp+NTFS.cur_size]
        call    ntfs_read_attr.continue
        pop     eax
        jc      .ret
        shl     eax, 9
        call    ntfs_restore_usa
        jc      .ret
        movzx   eax, word [esi+indexAllocatedSize]
        sub     eax, 8
.ret:
        ret

.emptyNode:
        test    byte [esi+indexFlags], 1
        jz      @f
        mov     eax, esi
        push    [ebp+NTFS.cur_offs]
        call    .findSubindex
        pop     [ebp+NTFS.cur_offs]
        jc      .ret
        test    esi, esi
        jnz     .ret
@@:         ; delete node
        mov     esi, [ebp+NTFS.attr_offs]
        add     esi, [esi+sizeWithHeader]
        cmp     byte [esi], 0xB0
        stc
        jnz     .ret
        movzx   eax, byte [esi+attributeOffset]
        add     esi, eax
        mov     eax, [ebp+NTFS.cur_offs]
        xor     edx, edx
        div     [ebp+NTFS.cur_size]
        mov     edx, eax
        shr     eax, 3
        and     edx, 7
        btr     [esi+eax], edx
        mov     esi, [ebp+NTFS.secondIndexBuffer]
        mov     byte [esi], 0
        xor     esi, esi
        ret

;----------------------------------------------------------------
ntfs_SetFileEnd:
        call    ntfs_lock
        call    ntfs_find_lfn
        jc      ntfsNotFound
        cmp     [ebp+NTFS.cur_iRecord], 16
        jc      ntfsDenied
        test    dword [eax+fileFlags], 10000001h
        jnz     ntfsDenied
        cmp     [ebp+NTFS.fragmentCount], 1
        jnz     ntfsUnsupported     ; record fragmented
; edit directory node
        mov     edi, [ebp+NTFS.cur_index_buf]
        cmp     dword [edi], 'INDX'
        jz      @f
        mov     esi, [ebp+NTFS.frs_buffer]
        mov     ecx, [esi+recordRealSize]
        shr     ecx, 2
        rep movsd
        mov     esi, [ebp+NTFS.attr_offs]
        mov     cl, [esi+attributeOffset]
        sub     esi, [ebp+NTFS.frs_buffer]
        add     eax, ecx
        add     eax, esi
@@:
        mov     edi, eax
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        mov     [edi+fileRealSize], eax
        mov     [edi+fileRealSize+4], edx
        push    edx eax ebx
        call    ntfsGetTime
        mov     [edi+fileModified], eax
        mov     [edi+fileModified+4], edx
        mov     [edi+recordModified], eax
        mov     [edi+recordModified+4], edx
        mov     [edi+fileAccessed], eax
        mov     [edi+fileAccessed+4], edx
        pop     ebx ecx edx
        mov     eax, [ebp+NTFS.LastRead]
        mov     [ebp+NTFS.nodeLastRead], eax
        mov     [ebp+NTFS.cur_attr], 0x80
        mov     [ebp+NTFS.cur_offs], 0
        mov     [ebp+NTFS.cur_size], 0
        call    ntfs_read_attr
        jc      ntfsFail
        mov     esi, edi
        mov     edi, [ebp+NTFS.frs_buffer]
        cmp     word [edi+baseRecordReuse], 0
        jnz     ntfsUnsupported     ; auxiliary record
        mov     al, [edi+attributeOffset]
        add     edi, eax
        mov     al, [edi+attributeOffset]
        add     edi, eax
        mov     eax, ecx
        mov     ecx, 6
        add     esi, fileModified
        add     edi, 8
        rep movsd
        mov     ecx, [ebp+NTFS.attr_offs]
        cmp     word [ecx+attributeFlags], 0
        jnz     ntfsUnsupported
        cmp     byte [ecx+nonResidentFlag], 0
        jz      .resizeAttribute
        cmp     [ecx+attributeRealSize+4], edx
        jnz     .resizeAttribute
        cmp     [ecx+attributeRealSize], eax
        jnc     .resizeAttribute
        mov     eax, [ecx+attributeRealSize]
        mov     ecx, [ebp+NTFS.sectors_per_cluster]
        mov     [ebp+NTFS.cur_size], ecx
        shl     ecx, 9
        div     ecx
        test    edx, edx
        jz      .aligned
        push    edx
        push    ecx
        mul     [ebp+NTFS.sectors_per_cluster]
        mov     [ebp+NTFS.cur_offs], eax
        stdcall kernel_alloc, ecx
        pop     ecx
        pop     edi
        mov     esi, eax
        sub     ecx, edi
        add     edi, eax
        mov     [ebp+NTFS.cur_buf], eax
        call    ntfs_read_attr.continue
        jc      @f
        xor     eax, eax
        rep stosb
        push    ebx
        mov     eax, [ebp+NTFS.LastRead]
        mov     ebx, esi
        mov     ecx, [ebp+NTFS.sectors_per_cluster]
        xor     edx, edx
        call    fs_write64_app
        pop     ebx
@@:
        stdcall kernel_free, esi
.aligned:
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
.resizeAttribute:
        call    resizeAttribute
        jc      ntfsError
        mov     ebx, [ebp+NTFS.frs_buffer]
        mov     edx, [ebp+NTFS.mftLastRead]
        call    writeRecord     ; file
        mov     ebx, [ebp+NTFS.cur_index_buf]
        mov     edx, [ebp+NTFS.nodeLastRead]
        call    writeRecord     ; directory
        call    ntfsSpaceClean
        jmp     ntfsDone

ntfsGetTime:
        call    fsGetTime
        jmp     @f

ntfsCalculateTime:
; in: esi -> data block
; out: edx:eax = seconds since 01.01.1601 x10000000
        call    fsCalculateTime
@@:
        mov     edx, 10000000
        mul     edx
        add     eax, 3365781504
        adc     edx, 29389701
        ret

;----------------------------------------------------------------
ntfs_SetFileInfo:
        call    ntfs_lock
        call    ntfs_find_lfn
        jnc     @f
        test    eax, eax
        jz      ntfsFail
        jmp     ntfsNotFound

@@:
        cmp     [ebp+NTFS.fragmentCount], 1
        jnz     ntfsUnsupported     ; record fragmented
        mov     esi, [ebp+NTFS.cur_index_buf]
        cmp     dword [esi], 'INDX'
        jz      @f
        sub     eax, esi
        mov     esi, [ebp+NTFS.indexRoot]
        movzx   edx, byte [esi+attributeOffset]
        add     eax, esi
        add     eax, edx
@@:
        mov     esi, [ebx+16]
        mov     edi, eax
        mov     eax, [esi]
        and     eax, 27h
        and     byte [edi+fileFlags], -28h
        or      [edi+fileFlags], al
        add     esi, 8
        call    ntfsCalculateTime
        mov     [edi+fileCreated], eax
        mov     [edi+fileCreated+4], edx
        add     esi, 8
        call    ntfsCalculateTime
        mov     [edi+fileAccessed], eax
        mov     [edi+fileAccessed+4], edx
        add     esi, 8
        call    ntfsCalculateTime
        mov     [edi+fileModified], eax
        mov     [edi+fileModified+4], edx
        mov     ebx, [ebp+NTFS.cur_index_buf]
        cmp     dword [ebx], 'INDX'
        jz      @f
        mov     ebx, [ebp+NTFS.frs_buffer]
@@:
        mov     edx, [ebp+NTFS.LastRead]
        call    writeRecord
        jmp     ntfsDone

ntfsUnsupported:
        push    ERROR_UNSUPPORTED_FS
        jmp     ntfsOut
ntfsDevice:
        push    ERROR_DEVICE
        jmp     ntfsOut
ntfsNotFound:
        push    ERROR_FILE_NOT_FOUND
        jmp     ntfsOut
ntfsDenied:
        push    ERROR_ACCESS_DENIED
        jmp     ntfsOut
ntfsFail:
        push    ERROR_FS_FAIL
        jmp     ntfsOut
ntfsDiskFull:
        push    ERROR_DISK_FULL
        jmp     ntfsOut
ntfsErrorPop5:
        pop     ebx
        pop     ebx
ntfsErrorPop3:
        pop     ebx
ntfsErrorPop2:
        pop     ebx
ntfsErrorPop:
        pop     ebx
ntfsError:
        push    eax
ntfsOut:
        call    ntfs_unlock
        xor     ebx, ebx
        pop     eax
        ret
