;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; NTFS driver

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
updateSequenceOffset = 4
updateSequenceSize = 6
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
sizeWithoutHeader = 16
indexedFlag = 16h
    ; non resident attribute header
lastVCN = 18h
dataRunsOffset = 20h
attributeAllocatedSize = 28h
attributeRealSize = 30h
initialDataSize = 38h
    ; $IndexRoot
collationRule = 4
indexRecordSize = 8
indexRecordSizeClus = 12
    ; node header
indexOffset = 0
nodeRealSize = 4
nodeAllocatedSize = 8
    ; $Filename index
fileRecordReference = 0
fileReferenceReuse = 6
indexAllocatedSize = 8
indexRawSize = 10
indexFlags = 12
directoryRecordReference = 16
directoryReferenceReuse = 16h
fileAllocatedSize = 38h
fileRealSize = 40h
fileFlags = 48h
fileNameLength = 50h

struct NTFS PARTITION
Lock                MUTEX   ?   ; Currently operations with one partition
; can not be executed in parallel since the legacy code is not ready.
sectors_per_cluster     dd  ?
mft_cluster             dd  ?   ; location
mftmirr_cluster         dd  ?   ; location
frs_size                dd  ?   ; in bytes
frs_buffer              dd  ?   ; MFT fileRecord buffer
mft_retrieval           dd  ?
mft_retrieval_size      dd  ?
mft_retrieval_alloc     dd  ?
mft_retrieval_end       dd  ?
cur_index_size          dd  ?   ; in sectors
cur_index_buf           dd  ?   ; index node buffer
BitmapBuffer            dd  ?
BitmapTotalSize         dd  ?   ; bytes reserved
BitmapSize              dd  ?   ; bytes readen
BitmapLocation          dd  ?   ; starting sector
BitmapStart             dd  ?   ; first byte after area, reserved for MFT
mftBitmapBuffer         dd  ?   ; one cluster
mftBitmapSize           dd  ?   ; bytes readen
mftBitmapLocation       dd  ?   ; starting sector

ntfs_cur_attr           dd  ?   ; attribute type
ntfs_cur_iRecord        dd  ?   ; number of fileRecord in MFT
ntfs_cur_offs           dd  ?   ; attribute VCN in sectors
ntfs_cur_size           dd  ?   ; max sectors to read
ntfs_cur_buf            dd  ?
ntfs_cur_read           dd  ?   ; bytes readen
ntfsLastRead            dd  ?   ; last readen block of sectors
newMftRecord            dd  ?   ; number of fileRecord in MFT
fileDataStart           dd  ?   ; starting cluster
fileDataSize            dd  ?   ; in clusters
fileRealSize            dd  ?   ; in bytes
indexOffset             dd  ?
nodeLastRead            dd  ?
ntfs_bCanContinue       db  ?
ntfsFolder              db  ?
ntfsWriteAttr           db  ?   ; Warning: Don't forget to turn off!!!
ntfsFragmentCount       db  ?

cur_subnode_size        dd  ?
ntfs_attr_iRecord       dd  ?
ntfs_attr_iBaseRecord   dd  ?
ntfs_attr_offs          dd  ?
ntfs_attr_list          dd  ?
ntfs_attr_size          dq  ?
ntfs_cur_tail           dd  ?

ntfs_attrlist_buf       rb  0x400
ntfs_attrlist_mft_buf   rb  0x400
ntfs_bitmap_buf         rb  0x400
ends

; NTFS external functions
;   in:
; ebx -> parameter structure of sysfunc 70
; ebp -> NTFS structure
; [esi]+[esp+4] = name
;   out:
; eax, ebx = return values for sysfunc 70
iglobal
align 4
ntfs_user_functions:
        dd      ntfs_free
        dd      (ntfs_user_functions_end - ntfs_user_functions - 4) / 4
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

ntfs_create_partition:
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
; By given bootsector, initialize some NTFS variables
.ntfs_setup:
        movi    eax, sizeof.NTFS
        call    malloc
        test    eax, eax
        jz      .exit
        mov     ecx, dword [ebp+PARTITION.FirstSector]
        mov     dword [eax+NTFS.FirstSector], ecx
        mov     ecx, dword [ebp+PARTITION.FirstSector+4]
        mov     dword [eax+NTFS.FirstSector+4], ecx
        mov     ecx, [ebp+PARTITION.Disk]
        mov     [eax+NTFS.Disk], ecx
        mov     [eax+NTFS.FSUserFunctions], ntfs_user_functions
        mov     [eax+NTFS.ntfsWriteAttr], 0

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
        call    ntfs_read_frs_sector
        test    eax, eax
        jnz     .usemirr
        cmp     dword [ebx], 'FILE'
        jnz     .usemirr
        call    ntfs_restore_usa_frs
        jnc     .mftok
.usemirr:
        mov     eax, [ebp+NTFS.mftmirr_cluster]
        mul     [ebp+NTFS.sectors_per_cluster]
        call    ntfs_read_frs_sector
        test    eax, eax
        jnz     .fail_free_frs
        cmp     dword [ebx], 'FILE'
        jnz     .fail_free_frs
        call    ntfs_restore_usa_frs
        jc      .fail_free_frs
.mftok:
; read $MFT table retrieval information
; start with one page, increase if not enough (when MFT too fragmented)
        push    ebx
        stdcall kernel_alloc, 0x1000
        pop     ebx
        test    eax, eax
        jz      .fail_free_frs
        mov     [ebp+NTFS.mft_retrieval], eax
        and     [ebp+NTFS.mft_retrieval_size], 0
        mov     [ebp+NTFS.mft_retrieval_alloc], 0x1000/8
; $MFT base record must contain unnamed non-resident $DATA attribute
        movzx   eax, word [ebx+14h]
        add     eax, ebx
.scandata:
        cmp     dword [eax], -1
        jz      .fail_free_mft
        cmp     dword [eax], 0x80
        jnz     @f
        cmp     byte [eax+9], 0
        jz      .founddata
@@:
        add     eax, [eax+4]
        jmp     .scandata
.founddata:
        cmp     byte [eax+8], 0
        jz      .fail_free_mft
; load first portion of $DATA attribute retrieval information
        mov     edx, [eax+0x18]
        mov     [ebp+NTFS.mft_retrieval_end], edx
        mov     esi, eax
        movzx   eax, word [eax+0x20]
        add     esi, eax
        sub     esp, 10h
.scanmcb:
        call    ntfs_decode_mcb_entry
        jnc     .scanmcbend
        call    .get_mft_retrieval_ptr
        mov     edx, [esp]      ; block length
        mov     [eax], edx
        mov     edx, [esp+8]    ; block addr (relative)
        mov     [eax+4], edx
        inc     [ebp+NTFS.mft_retrieval_size]
        jmp     .scanmcb
.scanmcbend:
        add     esp, 10h
; there may be other portions of $DATA attribute in auxiliary records;
; if they will be needed, they will be loaded later
        mov     [ebp+NTFS.cur_index_size], 0x1000/0x200
        stdcall kernel_alloc, 0x1000
        test    eax, eax
        jz      .fail_free_mft
        mov     [ebp+NTFS.cur_index_buf], eax
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
        mov     [ebp+NTFS.ntfs_cur_buf], eax
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
        mov     [ebp+NTFS.ntfs_cur_size], eax
        call    alloc_pages
        test    eax, eax
        pop     ecx
        jz      .failFreeBitmap
        add     eax, 3
        mov     ebx, [ebp+NTFS.BitmapBuffer]
        call    commit_pages
        mov     [ebp+NTFS.ntfs_cur_iRecord], 6
        mov     [ebp+NTFS.ntfs_cur_attr], 0x80
        mov     [ebp+NTFS.ntfs_cur_offs], 0
        call    ntfs_read_attr
        jc      .failFreeBitmap
        mov     eax, [ebp+NTFS.ntfs_cur_read]
        mov     [ebp+NTFS.BitmapSize], eax
        mov     eax, [ebp+NTFS.ntfsLastRead]
        mov     [ebp+NTFS.BitmapLocation], eax
; read MFT $BITMAP attribute
        mov     eax, [ebp+NTFS.sectors_per_cluster]
        mov     [ebp+NTFS.ntfs_cur_size], eax
        shl     eax, 9
        stdcall kernel_alloc, eax
        test    eax, eax
        jz      .failFreeBitmap
        mov     [ebp+NTFS.mftBitmapBuffer], eax
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        mov     [ebp+NTFS.ntfs_cur_iRecord], 0
        mov     [ebp+NTFS.ntfs_cur_attr], 0xB0
        mov     [ebp+NTFS.ntfs_cur_offs], 0
        call    ntfs_read_attr
        mov     eax, [ebp+NTFS.ntfs_cur_read]
        cmp     eax, 4
        jc      .failFreeBitmapMFT
        mov     [ebp+NTFS.mftBitmapSize], eax
        mov     eax, [ebp+NTFS.ntfsLastRead]
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
        stdcall kernel_free, [ebx+NTFS.mftBitmapBuffer]
.failFreeBitmap:
        stdcall kernel_free, [ebx+NTFS.BitmapBuffer]
.failFreeIndex:
        stdcall kernel_free, [ebp+NTFS.cur_index_buf]
.fail_free_mft:
        stdcall kernel_free, [ebp+NTFS.mft_retrieval]
.fail_free_frs:
        stdcall kernel_free, [ebp+NTFS.frs_buffer]
.fail_free:
        mov     eax, ebp
        call    free
        xor     eax, eax
        jmp     .pop_exit

.get_mft_retrieval_ptr:
        pushad
        mov     eax, [ebp+NTFS.mft_retrieval_size]
        cmp     eax, [ebp+NTFS.mft_retrieval_alloc]
        jnz     .ok
        add     eax, 0x1000/8
        mov     [ebp+NTFS.mft_retrieval_alloc], eax
        shl     eax, 3
        stdcall kernel_alloc, eax
        test    eax, eax
        jnz     @f
        popad
        add     esp, 14h
        jmp     .fail_free_mft
@@:
        mov     esi, [ebp+NTFS.mft_retrieval]
        mov     edi, eax
        mov     ecx, [ebp+NTFS.mft_retrieval_size]
        add     ecx, ecx
        rep movsd
        push    [ebp+NTFS.mft_retrieval]
        mov     [ebp+NTFS.mft_retrieval], eax
        call    kernel_free
        mov     eax, [ebp+NTFS.mft_retrieval_size]
.ok:
        shl     eax, 3
        add     eax, [ebp+NTFS.mft_retrieval]
        mov     [esp+28], eax
        popad
        ret

ntfs_free:
        push    ebx
        mov     ebx, eax
        stdcall kernel_free, [ebx+NTFS.frs_buffer]
        stdcall kernel_free, [ebx+NTFS.mft_retrieval]
        stdcall kernel_free, [ebx+NTFS.cur_index_buf]
        stdcall kernel_free, [ebx+NTFS.mftBitmapBuffer]
        stdcall kernel_free, [ebx+NTFS.BitmapBuffer]
        mov     eax, ebx
        pop     ebx
        jmp     free

ntfs_lock:
        lea     ecx, [ebp+NTFS.Lock]
        jmp     mutex_lock

ntfs_unlock:
        lea     ecx, [ebp+NTFS.Lock]
        jmp     mutex_unlock

ntfs_read_frs_sector:
        push    ecx
        mov     ebx, [ebp+NTFS.frs_buffer]
        push    ebx
        mov     ecx, [ebp+NTFS.frs_size]
        shr     ecx, 9
        push    ecx
        mov     ecx, eax
@@:
        mov     eax, ecx
        call    fs_read32_sys
        test    eax, eax
        jnz     .fail
        add     ebx, 0x200
        inc     ecx
        dec     dword [esp]
        jnz     @b
        pop     eax
.fail:
        pop     ebx
        pop     ecx
        ret

ntfs_read_attr:
; [ebp+NTFS.ntfsWriteAttr]=1 -> write attribute
;   in:
; [ebp+NTFS.ntfs_cur_iRecord] = number of fileRecord
; [ebp+NTFS.ntfs_cur_attr] = attribute type
; [ebp+NTFS.ntfs_cur_offs] = attribute VCN in sectors
; [ebp+NTFS.ntfs_cur_buf] -> buffer for data
; [ebp+NTFS.ntfs_cur_size] = max sectors to read
;   out:
; [ebp+NTFS.ntfs_cur_read] = bytes readen
; CF=1 -> failed, eax = disk error code, eax=0 -> something with FS
        xor     eax, eax
        pushad
        and     [ebp+NTFS.ntfs_cur_read], 0
        cmp     [ebp+NTFS.ntfs_cur_iRecord], 0
        jnz     .nomft
        cmp     [ebp+NTFS.ntfs_cur_attr], 0x80
        jnz     .nomft
        mov     eax, [ebp+NTFS.mft_retrieval_end]
        inc     eax
        mul     [ebp+NTFS.sectors_per_cluster]
        cmp     eax, [ebp+NTFS.ntfs_cur_offs]
        jbe     .nomft
; precalculated part of $Mft $DATA
        mov     esi, [ebp+NTFS.mft_retrieval]
        mov     eax, [ebp+NTFS.ntfs_cur_offs]
        xor     edx, edx
        div     [ebp+NTFS.sectors_per_cluster]
; eax = VCN, edx = offset in sectors from beginning of cluster
        xor     ecx, ecx        ; ecx will contain LCN
.mftscan:
        add     ecx, [esi+4]
        sub     eax, [esi]
        jb      @f
        add     esi, 8
        push    eax
        mov     eax, [ebp+NTFS.mft_retrieval_end]
        shl     eax, 3
        add     eax, [ebp+NTFS.mft_retrieval]
        cmp     eax, esi
        pop     eax
        jnz     .mftscan
        jmp     .nomft
@@:
        push    ecx
        add     ecx, eax
        add     ecx, [esi]
        push    eax
        push    edx
        mov     eax, [ebp+NTFS.sectors_per_cluster]
        mul     ecx
; eax = sector on partition
        pop     edx
        add     eax, edx
        mov     ebx, [ebp+NTFS.ntfs_cur_buf]
        pop     ecx
        neg     ecx
        imul    ecx, [ebp+NTFS.sectors_per_cluster]
        sub     ecx, edx
        mov     [ebp+NTFS.ntfsLastRead], eax
        cmp     ecx, [ebp+NTFS.ntfs_cur_size]
        jb      @f
        mov     ecx, [ebp+NTFS.ntfs_cur_size]
@@:
; ecx = number of sequential sectors to read
        push    eax
        call    fs_read32_sys
        pop     edx
        test    eax, eax
        jnz     .errread
        add     [ebp+NTFS.ntfs_cur_read], 0x200
        dec     [ebp+NTFS.ntfs_cur_size]
        inc     [ebp+NTFS.ntfs_cur_offs]
        add     ebx, 0x200
        mov     [ebp+NTFS.ntfs_cur_buf], ebx
        lea     eax, [edx+1]
        loop    @b
        pop     ecx
        xor     eax, eax
        xor     edx, edx
        cmp     [ebp+NTFS.ntfs_cur_size], eax
        jz      @f
        add     esi, 8
        push    eax
        mov     eax, [ebp+NTFS.mft_retrieval_end]
        shl     eax, 3
        add     eax, [ebp+NTFS.mft_retrieval]
        cmp     eax, esi
        pop     eax
        jz      .nomft
        jmp     .mftscan
@@:
        popad
        ret
.errread:
        pop     ecx
.errret:
        mov     [esp+28], eax
        stc
        popad
        ret
.nomft:
; 1. Read file record.
; N.B. This will do recursive call of read_attr for $MFT::$Data.
        mov     eax, [ebp+NTFS.ntfs_cur_iRecord]
        mov     [ebp+NTFS.ntfs_attr_iRecord], eax
        and     [ebp+NTFS.ntfs_attr_list], 0
        or      dword [ebp+NTFS.ntfs_attr_size], -1
        or      dword [ebp+NTFS.ntfs_attr_size+4], -1
        or      [ebp+NTFS.ntfs_attr_iBaseRecord], -1
        call    ntfs_read_file_record
        jc      .errret
; 2. Find required attribute.
        mov     eax, [ebp+NTFS.frs_buffer]
; a) For auxiliary records, read base record
; N.B. If base record is present,
;      base iRecord may be 0 (for $Mft), but SequenceNumber is nonzero
        cmp     dword [eax+24h], 0
        jz      @f
        mov     eax, [eax+20h]
;        test    eax, eax
;        jz      @f
.beginfindattr:
        mov     [ebp+NTFS.ntfs_attr_iRecord], eax
        call    ntfs_read_file_record
        jc      .errret
@@:
; b) Scan for required attribute and for $ATTR_LIST
        mov     eax, [ebp+NTFS.frs_buffer]
        movzx   ecx, word [eax+14h]
        add     eax, ecx
        mov     ecx, [ebp+NTFS.ntfs_cur_attr]
        and     [ebp+NTFS.ntfs_attr_offs], 0
.scanattr:
        cmp     dword [eax], -1
        jz      .scandone
        cmp     dword [eax], ecx
        jz      .okattr
        cmp     [ebp+NTFS.ntfs_attr_iBaseRecord], -1
        jnz     .scancont
        cmp     dword [eax], 0x20       ; $ATTR_LIST
        jnz     .scancont
        mov     [ebp+NTFS.ntfs_attr_list], eax
        jmp     .scancont
.okattr:
; ignore named $DATA attributes (aka NTFS streams)
        cmp     ecx, 0x80
        jnz     @f
        cmp     byte [eax+9], 0
        jnz     .scancont
@@:
        mov     [ebp+NTFS.ntfs_attr_offs], eax
.scancont:
        add     eax, [eax+4]
        jmp     .scanattr
.continue:
        pushad
        and     [ebp+NTFS.ntfs_cur_read], 0
.scandone:
; c) Check for required offset and length
        mov     ecx, [ebp+NTFS.ntfs_attr_offs]
        jecxz   .noattr
        push    [ebp+NTFS.ntfs_cur_size]
        push    [ebp+NTFS.ntfs_cur_read]
        call    .doreadattr
        pop     edx
        pop     ecx
        jc      @f
        cmp     [ebp+NTFS.ntfs_bCanContinue], 0
        jz      @f
        sub     edx, [ebp+NTFS.ntfs_cur_read]
        neg     edx
        shr     edx, 9
        sub     ecx, edx
        mov     [ebp+NTFS.ntfs_cur_size], ecx
        jnz     .not_in_cur
@@:
        popad
        ret
.noattr:
.not_in_cur:
        cmp     [ebp+NTFS.ntfs_cur_attr], 0x20
        jz      @f
        mov     ecx, [ebp+NTFS.ntfs_attr_list]
        test    ecx, ecx
        jnz     .lookattr
.ret_is_attr:
        and     dword [esp+28], 0
        cmp     [ebp+NTFS.ntfs_attr_offs], 1     ; CF set <=> ntfs_attr_offs == 0
        popad
        ret
.lookattr:
; required attribute or required offset was not found in base record;
; it may be present in auxiliary records;
; scan $ATTR_LIST
        mov     eax, [ebp+NTFS.ntfs_attr_iBaseRecord]
        cmp     eax, -1
        jz      @f
        call    ntfs_read_file_record
        jc      .errret
        or      [ebp+NTFS.ntfs_attr_iBaseRecord], -1
@@:
        push    [ebp+NTFS.ntfs_cur_offs]
        push    [ebp+NTFS.ntfs_cur_size]
        push    [ebp+NTFS.ntfs_cur_read]
        push    [ebp+NTFS.ntfs_cur_buf]
        push    dword [ebp+NTFS.ntfs_attr_size]
        push    dword [ebp+NTFS.ntfs_attr_size+4]
        or      dword [ebp+NTFS.ntfs_attr_size], -1
        or      dword [ebp+NTFS.ntfs_attr_size+4], -1
        and     [ebp+NTFS.ntfs_cur_offs], 0
        mov     [ebp+NTFS.ntfs_cur_size], 2
        and     [ebp+NTFS.ntfs_cur_read], 0
        lea     eax, [ebp+NTFS.ntfs_attrlist_buf]
        cmp     [ebp+NTFS.ntfs_cur_iRecord], 0
        jnz     @f
        lea     eax, [ebp+NTFS.ntfs_attrlist_mft_buf]
@@:
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        push    eax
        call    .doreadattr
        pop     esi
        mov     edx, 1
        pop     dword [ebp+NTFS.ntfs_attr_size+4]
        pop     dword [ebp+NTFS.ntfs_attr_size]
        mov     ecx, [ebp+NTFS.ntfs_cur_read]
        pop     [ebp+NTFS.ntfs_cur_buf]
        pop     [ebp+NTFS.ntfs_cur_read]
        pop     [ebp+NTFS.ntfs_cur_size]
        pop     [ebp+NTFS.ntfs_cur_offs]
        jc      .errret
        or      edi, -1
        lea     ecx, [ecx+esi-1Ah]
.scanliststart:
        push    ecx
        mov     eax, [ebp+NTFS.ntfs_cur_attr]
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
        mov     eax, dword [ebp+NTFS.ntfs_attr_size]
        and     eax, dword [ebp+NTFS.ntfs_attr_size+4]
        cmp     eax, -1
        jnz     .testfz
; if attribute is in auxiliary records, its size is defined only in first
        mov     eax, [esi+10h]
        call    ntfs_read_file_record
        jnc     @f
.errret_pop:
        pop     ecx ecx
        jmp     .errret
.errret2_pop:
        xor     eax, eax
        jmp     .errret_pop
@@:
        mov     eax, [ebp+NTFS.frs_buffer]
        movzx   ecx, word [eax+14h]
        add     eax, ecx
        mov     ecx, [ebp+NTFS.ntfs_cur_attr]
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
        mov     dword [ebp+NTFS.ntfs_attr_size], eax
        and     dword [ebp+NTFS.ntfs_attr_size+4], 0
        jmp     .testfz
.sdnores:
        mov     ecx, [eax+30h]
        mov     dword [ebp+NTFS.ntfs_attr_size], ecx
        mov     ecx, [eax+34h]
        mov     dword [ebp+NTFS.ntfs_attr_size+4], ecx
.testfz:
        xor     eax, eax
.testf:
        imul    eax, [ebp+NTFS.sectors_per_cluster]
        cmp     eax, [ebp+NTFS.ntfs_cur_offs]
        pop     eax
        ja      @f
        mov     edi, [esi+10h]  ; keep previous iRecord
        jmp     .scanlistcont
@@:
        pop     ecx
.scanlistfound:
        cmp     edi, -1
        jnz     @f
        popad
        ret
@@:
        mov     eax, [ebp+NTFS.ntfs_cur_iRecord]
        mov     [ebp+NTFS.ntfs_attr_iBaseRecord], eax
        mov     eax, edi
        jmp     .beginfindattr
.scanlistdone:
        pop     ecx
        sub     ecx, ebp
        sub     ecx, NTFS.ntfs_attrlist_buf-1Ah
        cmp     [ebp+NTFS.ntfs_cur_iRecord], 0
        jnz     @f
        sub     ecx, NTFS.ntfs_attrlist_mft_buf-NTFS.ntfs_attrlist_buf
@@:
        cmp     ecx, 0x400
        jnz     .scanlistfound
        inc     edx
        push    esi edi
        lea     esi, [ebp+NTFS.ntfs_attrlist_buf+0x200]
        lea     edi, [ebp+NTFS.ntfs_attrlist_buf]
        cmp     [ebp+NTFS.ntfs_cur_iRecord], 0
        jnz     @f
        lea     esi, [ebp+NTFS.ntfs_attrlist_mft_buf+0x200]
        lea     edi, [ebp+NTFS.ntfs_attrlist_mft_buf]
@@:
        mov     ecx, 0x200/4
        rep movsd
        mov     eax, edi
        pop     edi esi
        sub     esi, 0x200
        push    [ebp+NTFS.ntfs_cur_offs]
        push    [ebp+NTFS.ntfs_cur_size]
        push    [ebp+NTFS.ntfs_cur_read]
        push    [ebp+NTFS.ntfs_cur_buf]
        push    dword [ebp+NTFS.ntfs_attr_size]
        push    dword [ebp+NTFS.ntfs_attr_size+4]
        or      dword [ebp+NTFS.ntfs_attr_size], -1
        or      dword [ebp+NTFS.ntfs_attr_size+4], -1
        mov     [ebp+NTFS.ntfs_cur_offs], edx
        mov     [ebp+NTFS.ntfs_cur_size], 1
        and     [ebp+NTFS.ntfs_cur_read], 0
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        mov     ecx, [ebp+NTFS.ntfs_attr_list]
        push    esi edx edi
        call    .doreadattr
        pop     edi edx esi
        mov     ecx, [ebp+NTFS.ntfs_cur_read]
        pop     dword [ebp+NTFS.ntfs_attr_size+4]
        pop     dword [ebp+NTFS.ntfs_attr_size]
        pop     [ebp+NTFS.ntfs_cur_buf]
        pop     [ebp+NTFS.ntfs_cur_read]
        pop     [ebp+NTFS.ntfs_cur_size]
        pop     [ebp+NTFS.ntfs_cur_offs]
        jc      .errret
        lea     ecx, [ecx+ebp+NTFS.ntfs_attrlist_buf+0x200-0x1A]
        cmp     [ebp+NTFS.ntfs_cur_iRecord], 0
        jnz     .scanliststart
        add     ecx, NTFS.ntfs_attrlist_mft_buf-NTFS.ntfs_attrlist_buf
        jmp     .scanliststart

.doreadattr:
        mov     [ebp+NTFS.ntfs_bCanContinue], 0
        cmp     byte [ecx+8], 0
        jnz     .nonresident
        mov     eax, [ecx+10h]  ; length
        mov     esi, eax
        mov     edx, [ebp+NTFS.ntfs_cur_offs]
        shr     eax, 9
        cmp     eax, edx
        jb      .okret
        shl     edx, 9
        sub     esi, edx
        movzx   eax, word [ecx+14h]
        add     edx, eax
        add     edx, ecx        ; edx -> data
        mov     eax, [ebp+NTFS.ntfs_cur_size]
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
        mov     [ebp+NTFS.ntfs_cur_read], eax
        mov     ecx, eax
        mov     eax, edx
        mov     ebx, [ebp+NTFS.ntfs_cur_buf]
        call    memmove
        and     [ebp+NTFS.ntfs_cur_size], 0      ; CF=0
        ret
.nonresident:
; Not all auxiliary records contain correct FileSize info
        mov     eax, dword [ebp+NTFS.ntfs_attr_size]
        mov     edx, dword [ebp+NTFS.ntfs_attr_size+4]
        push    eax
        and     eax, edx
        cmp     eax, -1
        pop     eax
        jnz     @f
        mov     eax, [ecx+30h]  ; FileSize
        mov     edx, [ecx+34h]
        mov     dword [ebp+NTFS.ntfs_attr_size], eax
        mov     dword [ebp+NTFS.ntfs_attr_size+4], edx
@@:
        add     eax, 0x1FF
        adc     edx, 0
        shrd    eax, edx, 9
        sub     eax, [ebp+NTFS.ntfs_cur_offs]
        ja      @f
; return with nothing read
        and     [ebp+NTFS.ntfs_cur_size], 0
.okret:
        clc
        ret
@@:
; reduce read length
        and     [ebp+NTFS.ntfs_cur_tail], 0
        cmp     [ebp+NTFS.ntfs_cur_size], eax
        jb      @f
        mov     [ebp+NTFS.ntfs_cur_size], eax
        mov     eax, dword [ebp+NTFS.ntfs_attr_size]
        and     eax, 0x1FF
        mov     [ebp+NTFS.ntfs_cur_tail], eax
@@:
        cmp     [ebp+NTFS.ntfs_cur_size], 0
        jz      .okret
        mov     eax, [ebp+NTFS.ntfs_cur_offs]
        xor     edx, edx
        div     [ebp+NTFS.sectors_per_cluster]
        sub     eax, [ecx+10h]  ; first_vbo
        jb      .okret
; eax = cluster, edx = starting sector
        cmp     [ebp+NTFS.ntfs_cur_attr], 0x80
        jnz     .sys
        cmp     [ebp+NTFS.ntfs_cur_iRecord], 0
        jz      .sys
        push    fs_read64_app
        cmp     [ebp+NTFS.ntfsWriteAttr], 1
        jnz     @f
        mov     dword[esp], fs_write64_app
        jmp     @f
.sys:
        push    fs_read64_sys
@@:
        sub     esp, 10h
        movzx   esi, word [ecx+20h]     ; mcb_info_ofs
        add     esi, ecx
        xor     edi, edi
        mov     [ebp+NTFS.ntfsFragmentCount], 0
.readloop:
        call    ntfs_decode_mcb_entry
        jnc     .break
        add     edi, [esp+8]
        sub     eax, [esp]
        jae     .readloop
        push    ecx
        push    eax
        add     eax, [esp+8]
        add     eax, edi
        imul    eax, [ebp+NTFS.sectors_per_cluster]
        add     eax, edx
        pop     ecx
        neg     ecx
        imul    ecx, [ebp+NTFS.sectors_per_cluster]
        sub     ecx, edx
        cmp     ecx, [ebp+NTFS.ntfs_cur_size]
        jb      @f
        mov     ecx, [ebp+NTFS.ntfs_cur_size]
@@:
        mov     ebx, [ebp+NTFS.ntfs_cur_buf]
        mov     [ebp+NTFS.ntfsLastRead], eax
        push    ecx
        xor     edx, edx
        call    dword[esp+18h]
        pop     ecx
        test    eax, eax
        jnz     .errread2
        sub     [ebp+NTFS.ntfs_cur_size], ecx
        add     [ebp+NTFS.ntfs_cur_offs], ecx
        shl     ecx, 9
        add     [ebp+NTFS.ntfs_cur_read], ecx
        add     [ebp+NTFS.ntfs_cur_buf], ecx
        inc     [ebp+NTFS.ntfsFragmentCount]
        pop     ecx
        xor     eax, eax
        xor     edx, edx
        cmp     [ebp+NTFS.ntfs_cur_size], 0
        jnz     .readloop
        add     esp, 14h
        mov     eax, [ebp+NTFS.ntfs_cur_tail]
        test    eax, eax
        jz      @f
        sub     eax, 0x200
        add     [ebp+NTFS.ntfs_cur_read], eax
@@:
        clc
        ret
.errread2:
        pop     ecx
        add     esp, 14h
        stc
        ret
.break:
        add     esp, 14h        ; CF=0
        mov     [ebp+NTFS.ntfs_bCanContinue], 1
        ret

ntfs_read_file_record:
; in: eax = iRecord
; out: [ebp+NTFS.frs_buffer] = record data
; CF=1 -> failed, eax = disk error code, eax=0 -> something with FS
    ; Read attr $DATA of $Mft, starting from eax*[ebp+NTFS.frs_size]
        push    ecx edx
        mov     ecx, [ebp+NTFS.frs_size]
        mul     ecx
        shrd    eax, edx, 9
        shr     edx, 9
        jnz     .errret
        push    [ebp+NTFS.ntfs_attr_iRecord]
        push    [ebp+NTFS.ntfs_attr_iBaseRecord]
        push    [ebp+NTFS.ntfs_attr_offs]
        push    [ebp+NTFS.ntfs_attr_list]
        push    dword [ebp+NTFS.ntfs_attr_size+4]
        push    dword [ebp+NTFS.ntfs_attr_size]
        push    [ebp+NTFS.ntfs_cur_iRecord]
        push    [ebp+NTFS.ntfs_cur_attr]
        push    [ebp+NTFS.ntfs_cur_offs]
        push    [ebp+NTFS.ntfs_cur_size]
        push    [ebp+NTFS.ntfs_cur_buf]
        push    [ebp+NTFS.ntfs_cur_read]
        mov     [ebp+NTFS.ntfs_cur_attr], 0x80   ; $DATA
        and     [ebp+NTFS.ntfs_cur_iRecord], 0   ; $Mft
        mov     [ebp+NTFS.ntfs_cur_offs], eax
        shr     ecx, 9
        mov     [ebp+NTFS.ntfs_cur_size], ecx
        mov     eax, [ebp+NTFS.frs_buffer]
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        call    ntfs_read_attr
        mov     edx, [ebp+NTFS.ntfs_cur_read]
        pop     [ebp+NTFS.ntfs_cur_read]
        pop     [ebp+NTFS.ntfs_cur_buf]
        pop     [ebp+NTFS.ntfs_cur_size]
        pop     [ebp+NTFS.ntfs_cur_offs]
        pop     [ebp+NTFS.ntfs_cur_attr]
        pop     [ebp+NTFS.ntfs_cur_iRecord]
        pop     dword [ebp+NTFS.ntfs_attr_size]
        pop     dword [ebp+NTFS.ntfs_attr_size+4]
        pop     [ebp+NTFS.ntfs_attr_list]
        pop     [ebp+NTFS.ntfs_attr_offs]
        pop     [ebp+NTFS.ntfs_attr_iBaseRecord]
        pop     [ebp+NTFS.ntfs_attr_iRecord]
        jc      .ret
        cmp     edx, [ebp+NTFS.frs_size]
        jnz     .errret
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
        pushad
        shr     eax, 9
        mov     ecx, eax
        inc     eax
        cmp     [ebx+6], ax
        jnz     .err
        movzx   eax, word [ebx+4]
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
; esi -> mcb entry
; esp -> buffer (16 bytes)
;   out:
; esi -> next mcb entry
; esp -> data run size
; esp+8 -> cluster (delta)
; CF=0 -> mcb end
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

unichar_toupper:
        push    eax
        call    uni2ansi_char
        cmp     al, '_'
        jz      .unk
        add     esp, 4
        call    char_toupper
        jmp     ansi2uni_char
.unk:
        pop     eax
        ret

ntfs_find_lfn:
; in: [esi]+[esp+4] = name
;   out:
; [ebp+NTFS.ntfs_cur_iRecord] = number of MFT fileRecord
; eax -> index in the parent index node
; CF=1 -> file not found, eax=0 -> error
        mov     [ebp+NTFS.ntfs_cur_iRecord], 5   ; start parse from root cluster
.doit2:
        mov     [ebp+NTFS.ntfs_cur_attr], 0x90   ; $INDEX_ROOT
        and     [ebp+NTFS.ntfs_cur_offs], 0
        mov     eax, [ebp+NTFS.cur_index_size]
        mov     [ebp+NTFS.ntfs_cur_size], eax
        mov     eax, [ebp+NTFS.cur_index_buf]
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        call    ntfs_read_attr
        mov     eax, 0
        jnc     @f
.ret:
        ret     4
@@:
        cmp     [ebp+NTFS.ntfs_cur_read], 0x20
        jc      .ret
        pushad
        mov     esi, [ebp+NTFS.cur_index_buf]
        mov     eax, [esi+14h]
        add     eax, 10h
        cmp     [ebp+NTFS.ntfs_cur_read], eax
        jae     .readok1
        add     eax, 1FFh
        shr     eax, 9
        cmp     eax, [ebp+NTFS.cur_index_size]
        ja      @f
.stc_ret:
        popad
        stc
        ret     4
@@:
; reallocate
        push    eax
        stdcall kernel_free, [ebp+NTFS.cur_index_buf]
        pop     eax
        mov     [ebp+NTFS.cur_index_size], eax
        stdcall kernel_alloc, eax
        test    eax, eax
        jnz     @f
        and     [ebp+NTFS.cur_index_size], 0
        and     [ebp+NTFS.cur_index_buf], 0
        jmp     .stc_ret
@@:
        mov     [ebp+NTFS.cur_index_buf], eax
        popad
        jmp     .doit2
.readok1:
        mov     edx, [esi+8]    ; subnode_size
        shr     edx, 9
        cmp     edx, [ebp+NTFS.cur_index_size]
        jbe     .ok2
        push    esi edx
        stdcall kernel_alloc, edx
        pop     edx esi
        test    eax, eax
        jz      .stc_ret
        mov     edi, eax
        mov     ecx, [ebp+NTFS.cur_index_size]
        shl     ecx, 9-2
        rep movsd
        mov     esi, eax
        mov     [ebp+NTFS.cur_index_size], edx
        push    esi edx
        stdcall kernel_free, [ebp+NTFS.cur_index_buf]
        pop     edx esi
        mov     [ebp+NTFS.cur_index_buf], esi
.ok2:
        add     esi, 10h
        mov     edi, [esp+4]
; edi -> name, esi -> current index data, edx = subnode size
.scanloop:
        add     esi, [esi]
.scanloopint:
        test    byte [esi+0Ch], 2
        jnz     .subnode
        push    esi
        add     esi, 0x52
        movzx   ecx, byte [esi-2]
        push    edi
@@:
        lodsw
        call    unichar_toupper
        push    eax
        mov     al, [edi]
        inc     edi
        cmp     al, '/'
        jz      .slash
        call    char_toupper
        call    ansi2uni_char
        cmp     ax, [esp]
        pop     eax
        loopz   @b
        jz      .found
        pop     edi
        pop     esi
        jb      .subnode
.scanloopcont:
        movzx   eax, word [esi+8]
        add     esi, eax
        jmp     .scanloopint
.slash:
        pop     eax
        pop     edi
        pop     esi
.subnode:
        test    byte [esi+0Ch], 1
        jz      .notfound
        movzx   eax, word [esi+8]
        mov     eax, [esi+eax-8]
        imul    eax, [ebp+NTFS.sectors_per_cluster]
        mov     [ebp+NTFS.ntfs_cur_offs], eax
        mov     [ebp+NTFS.ntfs_cur_attr], 0xA0   ; $INDEX_ALLOCATION
        mov     [ebp+NTFS.ntfs_cur_size], edx
        mov     eax, [ebp+NTFS.cur_index_buf]
        mov     esi, eax
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        push    edx
        call    ntfs_read_attr
        pop     edx
        mov     eax, edx
        shl     eax, 9
        cmp     [ebp+NTFS.ntfs_cur_read], eax
        jnz     .err
        cmp     dword [esi], 'INDX'
        jnz     .err
        mov     [ebp+NTFS.ntfs_cur_buf], esi
        mov     ebx, esi
        call    ntfs_restore_usa
        jc      .err
        add     esi, 0x18
        jmp     .scanloop
.notfound:
        mov     [esp+1Ch], esi
.err:
        popad
        stc
        ret     4
.found:
        cmp     byte [edi], 0
        jz      .done
        cmp     byte [edi], '/'
        jz      .next
        pop     edi
        pop     esi
        jmp     .scanloopcont
.done:
.next:
        pop     esi
        pop     esi
        mov     eax, [esi]
        mov     [ebp+NTFS.ntfs_cur_iRecord], eax
        mov     [esp+1Ch], esi
        mov     [esp+4], edi
        popad
        inc     esi
        cmp     byte [esi-1], 0
        jnz     .doit2
        cmp     dword [esp+4], 0
        jz      @f
        mov     esi, [esp+4]
        mov     dword [esp+4], 0
        jmp     .doit2
@@:
        ret     4

;----------------------------------------------------------------
ntfs_ReadFile:
        cmp     byte [esi], 0
        jnz     @f
        or      ebx, -1
        movi    eax, ERROR_ACCESS_DENIED
        ret
@@:
        call    ntfs_lock
        stdcall ntfs_find_lfn, [esp+4]
        jnc     .found
        call    ntfs_unlock
        or      ebx, -1
        movi    eax, ERROR_FILE_NOT_FOUND
        ret
.found:
        mov     [ebp+NTFS.ntfs_cur_attr], 0x80   ; $DATA
        and     [ebp+NTFS.ntfs_cur_offs], 0
        and     [ebp+NTFS.ntfs_cur_size], 0
        call    ntfs_read_attr
        jnc     @f
        call    ntfs_unlock
        or      ebx, -1
        movi    eax, ERROR_ACCESS_DENIED
        ret
@@:
        pushad
        and     dword [esp+10h], 0
        xor     eax, eax
        cmp     dword [ebx+8], 0x200
        jb      @f
.eof0:
        popad
        xor     ebx, ebx
.eof:
        push    ERROR_END_OF_FILE
        call    ntfs_unlock
        pop     eax
        ret
@@:
        mov     ecx, [ebx+12]
        mov     edx, [ebx+16]
        mov     eax, [ebx+4]
        test    eax, 0x1FF
        jz      .alignedstart
        push    edx
        mov     edx, [ebx+8]
        shrd    eax, edx, 9
        pop     edx
        mov     [ebp+NTFS.ntfs_cur_offs], eax
        mov     [ebp+NTFS.ntfs_cur_size], 1
        lea     eax, [ebp+NTFS.ntfs_bitmap_buf]
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        call    ntfs_read_attr.continue
        mov     eax, [ebx+4]
        and     eax, 0x1FF
        lea     esi, [ebp+NTFS.ntfs_bitmap_buf+eax]
        sub     eax, [ebp+NTFS.ntfs_cur_read]
        jae     .eof0
        neg     eax
        push    ecx
        cmp     ecx, eax
        jb      @f
        mov     ecx, eax
@@:
        mov     [esp+10h+4], ecx
        mov     edi, edx
        rep movsb
        mov     edx, edi
        pop     ecx
        sub     ecx, [esp+10h]
        jnz     @f
.retok:
        popad
        call    ntfs_unlock
        xor     eax, eax
        ret
@@:
        cmp     [ebp+NTFS.ntfs_cur_read], 0x200
        jz      .alignedstart
.eof_ebx:
        popad
        jmp     .eof
.alignedstart:
        mov     eax, [ebx+4]
        push    edx
        mov     edx, [ebx+8]
        add     eax, 511
        adc     edx, 0
        shrd    eax, edx, 9
        pop     edx
        mov     [ebp+NTFS.ntfs_cur_offs], eax
        mov     [ebp+NTFS.ntfs_cur_buf], edx
        mov     eax, ecx
        shr     eax, 9
        mov     [ebp+NTFS.ntfs_cur_size], eax
        add     eax, [ebp+NTFS.ntfs_cur_offs]
        push    eax
        call    ntfs_read_attr.continue
        pop     [ebp+NTFS.ntfs_cur_offs]
        mov     eax, [ebp+NTFS.ntfs_cur_read]
        add     [esp+10h], eax
        mov     eax, ecx
        and     eax, not 0x1FF
        cmp     [ebp+NTFS.ntfs_cur_read], eax
        jnz     .eof_ebx
        and     ecx, 0x1FF
        jz      .retok
        add     edx, [ebp+NTFS.ntfs_cur_read]
        mov     [ebp+NTFS.ntfs_cur_size], 1
        lea     eax, [ebp+NTFS.ntfs_bitmap_buf]
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        call    ntfs_read_attr.continue
        cmp     [ebp+NTFS.ntfs_cur_read], ecx
        jb      @f
        mov     [ebp+NTFS.ntfs_cur_read], ecx
@@:
        xchg    ecx, [ebp+NTFS.ntfs_cur_read]
        push    ecx
        mov     edi, edx
        lea     esi, [ebp+NTFS.ntfs_bitmap_buf]
        add     [esp+10h+4], ecx
        rep movsb
        pop     ecx
        xor     eax, eax
        cmp     ecx, [ebp+NTFS.ntfs_cur_read]
        jz      @f
        mov     al, ERROR_END_OF_FILE
@@:
        mov     [esp+1Ch], eax
        call    ntfs_unlock
        popad
        ret

;----------------------------------------------------------------
ntfs_ReadFolder:
        call    ntfs_lock
        mov     eax, 5          ; root cluster
        cmp     byte [esi], 0
        jz      .doit
        stdcall ntfs_find_lfn, [esp+4]
        jnc     .doit2
.notfound:
        or      ebx, -1
        push    ERROR_FILE_NOT_FOUND
.pop_ret:
        call    ntfs_unlock
        pop     eax
        ret
.doit:
        mov     [ebp+NTFS.ntfs_cur_iRecord], eax
.doit2:
        mov     [ebp+NTFS.ntfs_cur_attr], 0x10   ; $STANDARD_INFORMATION
        and     [ebp+NTFS.ntfs_cur_offs], 0
        mov     [ebp+NTFS.ntfs_cur_size], 1
        lea     eax, [ebp+NTFS.ntfs_bitmap_buf]
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        call    ntfs_read_attr
        jc      .notfound
        mov     [ebp+NTFS.ntfs_cur_attr], 0x90   ; $INDEX_ROOT
        and     [ebp+NTFS.ntfs_cur_offs], 0
        mov     eax, [ebp+NTFS.cur_index_size]
        mov     [ebp+NTFS.ntfs_cur_size], eax
        mov     eax, [ebp+NTFS.cur_index_buf]
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        call    ntfs_read_attr
        jnc     .ok
        test    eax, eax
        jz      .notfound
        or      ebx, -1
        push    ERROR_DEVICE
        jmp     .pop_ret
.ok:
        cmp     [ebp+NTFS.ntfs_cur_read], 0x20
        jae     @f
        or      ebx, -1
.fserr:
        push    ERROR_FAT_TABLE
        jmp     .pop_ret
@@:
        pushad
        mov     esi, [ebp+NTFS.cur_index_buf]
        mov     eax, [esi+14h]
        add     eax, 10h
        cmp     [ebp+NTFS.ntfs_cur_read], eax
        jae     .readok1
        add     eax, 1FFh
        shr     eax, 9
        cmp     eax, [ebp+NTFS.cur_index_size]
        ja      @f
        popad
        jmp     .fserr
@@:
; reallocate
        push    eax
        stdcall kernel_free, [ebp+NTFS.cur_index_buf]
        pop     eax
        mov     [ebp+NTFS.cur_index_size], eax
        stdcall kernel_alloc, eax
        test    eax, eax
        jnz     @f
        and     [ebp+NTFS.cur_index_size], 0
        and     [ebp+NTFS.cur_index_buf], 0
.nomem:
        call    ntfs_unlock
        popad
        or      ebx, -1
        movi    eax, ERROR_OUT_OF_MEMORY
        ret
@@:
        mov     [ebp+NTFS.cur_index_buf], eax
        popad
        jmp     .doit2
.readok1:
        mov     edx, [esi+8]    ; subnode_size
        shr     edx, 9
        mov     [ebp+NTFS.cur_subnode_size], edx
        cmp     edx, [ebp+NTFS.cur_index_size]
        jbe     .ok2
        push    esi edx
        stdcall kernel_alloc, edx
        pop     edx esi
        test    eax, eax
        jz      .nomem
        mov     edi, eax
        mov     ecx, [ebp+NTFS.cur_index_size]
        shl     ecx, 9-2
        rep movsd
        mov     esi, eax
        mov     [ebp+NTFS.cur_index_size], edx
        stdcall kernel_free, [ebp+NTFS.cur_index_buf]
        mov     [ebp+NTFS.cur_index_buf], esi
.ok2:
        add     esi, 10h
        mov     edx, [ebx+16]
        push    dword [ebx+8]   ; read ANSI/UNICODE name
; init header
        mov     edi, edx
        mov     ecx, 32/4
        xor     eax, eax
        rep stosd
        mov     byte [edx], 1   ; version
        mov     ecx, [ebx+12]
        mov     ebx, [ebx+4]
        push    edx
        mov     edx, esp
; edi -> BDFE, esi -> current index data, ebx = first wanted block,
; ecx = number of blocks to read
; edx -> parameters block: dd <output>, dd <flags>
        cmp     [ebp+NTFS.ntfs_cur_iRecord], 5
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
        add     esi, [esi]
.dump_root:
        test    byte [esi+0Ch], 2
        jnz     .dump_root_done
        call    .add_entry
        movzx   eax, word [esi+8]
        add     esi, eax
        jmp     .dump_root
.dump_root_done:
; now dump all subnodes
        push    ecx edi
        lea     edi, [ebp+NTFS.ntfs_bitmap_buf]
        mov     [ebp+NTFS.ntfs_cur_buf], edi
        mov     ecx, 0x400/4
        xor     eax, eax
        rep stosd
        mov     [ebp+NTFS.ntfs_cur_attr], 0xB0   ; $BITMAP
        and     [ebp+NTFS.ntfs_cur_offs], 0
        mov     [ebp+NTFS.ntfs_cur_size], 2
        call    ntfs_read_attr
        pop     edi ecx
        push    0       ; save offset in $BITMAP attribute
        and     [ebp+NTFS.ntfs_cur_offs], 0
.dumploop:
        mov     [ebp+NTFS.ntfs_cur_attr], 0xA0
        mov     eax, [ebp+NTFS.cur_subnode_size]
        mov     [ebp+NTFS.ntfs_cur_size], eax
        mov     eax, [ebp+NTFS.cur_index_buf]
        mov     esi, eax
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        push    [ebp+NTFS.ntfs_cur_offs]
        mov     eax, [ebp+NTFS.ntfs_cur_offs]
        imul    eax, [ebp+NTFS.cur_subnode_size]
        mov     [ebp+NTFS.ntfs_cur_offs], eax
        call    ntfs_read_attr
        pop     [ebp+NTFS.ntfs_cur_offs]
        mov     eax, [ebp+NTFS.cur_subnode_size]
        shl     eax, 9
        cmp     [ebp+NTFS.ntfs_cur_read], eax
        jnz     .done
        push    eax
        mov     eax, [ebp+NTFS.ntfs_cur_offs]
        and     eax, 0x400*8-1
        bt      dword [ebp+NTFS.ntfs_bitmap_buf], eax
        pop     eax
        jnc     .dump_subnode_done
        cmp     dword [esi], 'INDX'
        jnz     .dump_subnode_done
        push    ebx
        mov     ebx, esi
        call    ntfs_restore_usa
        pop     ebx
        jc      .dump_subnode_done
        add     esi, 0x18
        add     esi, [esi]
.dump_subnode:
        test    byte [esi+0Ch], 2
        jnz     .dump_subnode_done
        call    .add_entry
        movzx   eax, word [esi+8]
        add     esi, eax
        jmp     .dump_subnode
.dump_subnode_done:
        inc     [ebp+NTFS.ntfs_cur_offs]
        test    [ebp+NTFS.ntfs_cur_offs], 0x400*8-1
        jnz     .dumploop
        mov     [ebp+NTFS.ntfs_cur_attr], 0xB0
        push    ecx edi
        lea     edi, [ebp+NTFS.ntfs_bitmap_buf]
        mov     [ebp+NTFS.ntfs_cur_buf], edi
        mov     ecx, 0x400/4
        xor     eax, eax
        rep stosd
        pop     edi ecx
        pop     eax
        push    [ebp+NTFS.ntfs_cur_offs]
        inc     eax
        mov     [ebp+NTFS.ntfs_cur_offs], eax
        mov     [ebp+NTFS.ntfs_cur_size], 2
        push    eax
        call    ntfs_read_attr
        pop     eax
        pop     [ebp+NTFS.ntfs_cur_offs]
        push    eax
        jmp     .dumploop
.done:
        pop     eax
        pop     edx
        mov     ebx, [edx+4]
        pop     edx
        xor     eax, eax
        dec     ecx
        js      @f
        mov     al, ERROR_END_OF_FILE
@@:
        mov     [esp+1Ch], eax
        mov     [esp+10h], ebx
        call    ntfs_unlock
        popad
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
;        mov     eax, dword [ntfs_bitmap_buf+0x20]
;        or      al, 0x10
        mov     eax, 0x10
        stosd
        scasd
        push    edx
        mov     eax, dword [ebp+NTFS.ntfs_bitmap_buf]
        mov     edx, dword [ebp+NTFS.ntfs_bitmap_buf+4]
        call    ntfs_datetime_to_bdfe
        mov     eax, dword [ebp+NTFS.ntfs_bitmap_buf+0x18]
        mov     edx, dword [ebp+NTFS.ntfs_bitmap_buf+0x1C]
        call    ntfs_datetime_to_bdfe
        mov     eax, dword [ebp+NTFS.ntfs_bitmap_buf+8]
        mov     edx, dword [ebp+NTFS.ntfs_bitmap_buf+0xC]
        call    ntfs_datetime_to_bdfe
        pop     edx
        xor     eax, eax
        stosd
        stosd
        mov     al, '.'
        push    edi ecx
        lea     ecx, [esi+1]
        test    byte [edi-0x24], 1
        jz      @f
        rep stosw
        pop     ecx
        xor     eax, eax
        stosw
        pop     edi
        add     edi, 520
        ret
@@:
        rep stosb
        pop     ecx
        xor     eax, eax
        stosb
        pop     edi
        add     edi, 264
.ret:
        ret

.add_entry:
; do not return DOS 8.3 names
        cmp     byte [esi+0x51], 2
        jz      .ret
; do not return system files
; ... note that there will be no bad effects if system files also were reported ...
        cmp     dword [esi], 0x10
        jb      .ret
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
        movzx   ecx, byte [esi+0x50]
        add     esi, 0x52
        test    byte [edi-0x24], 1
        jz      .ansi
        shr     ecx, 1
        rep movsd
        adc     ecx, ecx
        rep movsw
        and     word [edi], 0
        pop     edi
        add     edi, 520
        pop     esi ecx
        ret
.ansi:
        jecxz   .skip
@@:
        lodsw
        call    uni2ansi_char
        stosb
        loop    @b
.skip:
        xor     al, al
        stosb
        pop     edi
        add     edi, 264
        pop     esi ecx
        ret

ntfs_direntry_to_bdfe:
        mov     [edi+4], eax    ; ANSI/UNICODE name
        mov     eax, [esi+48h]
        test    eax, 0x10000000
        jz      @f
        and     eax, not 0x10000000
        or      al, 0x10
@@:
        stosd
        scasd
        push    edx
        mov     eax, [esi+0x18]
        mov     edx, [esi+0x1C]
        call    ntfs_datetime_to_bdfe
        mov     eax, [esi+0x30]
        mov     edx, [esi+0x34]
        call    ntfs_datetime_to_bdfe
        mov     eax, [esi+0x20]
        mov     edx, [esi+0x24]
        call    ntfs_datetime_to_bdfe
        pop     edx
        mov     eax, [esi+0x40]
        stosd
        mov     eax, [esi+0x44]
        stosd
        ret

iglobal
_24             dd      24
_60             dd      60
_10000000       dd      10000000
days400year     dd      365*400+100-4+1
days100year     dd      365*100+25-1
days4year       dd      365*4+1
days1year       dd      365
months  dd  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
months2 dd  31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
_400            dd      400
_100            dd      100
endg

ntfs_datetime_to_bdfe:
; edx:eax = number of 100-nanosecond intervals since January 1, 1601, in UTC
        push    eax
        mov     eax, edx
        xor     edx, edx
        div     [_10000000]
        xchg    eax, [esp]
        div     [_10000000]
        pop     edx
    .sec:
; edx:eax = number of seconds since January 1, 1601
        push    eax
        mov     eax, edx
        xor     edx, edx
        div     [_60]
        xchg    eax, [esp]
        div     [_60]
        mov     [edi], dl
        pop     edx
; edx:eax = number of minutes
        div     [_60]
        mov     [edi+1], dl
; eax = number of hours (note that 2^64/(10^7*60*60) < 2^32)
        xor     edx, edx
        div     [_24]
        mov     [edi+2], dl
        mov     [edi+3], byte 0
; eax = number of days since January 1, 1601
        xor     edx, edx
        div     [days400year]
        imul    eax, 400
        add     eax, 1601
        mov     [edi+6], ax
        mov     eax, edx
        xor     edx, edx
        div     [days100year]
        cmp     al, 4
        jnz     @f
        dec     eax
        add     edx, [days100year]
@@:
        imul    eax, 100
        add     [edi+6], ax
        mov     eax, edx
        xor     edx, edx
        div     [days4year]
        shl     eax, 2
        add     [edi+6], ax
        mov     eax, edx
        xor     edx, edx
        div     [days1year]
        cmp     al, 4
        jnz     @f
        dec     eax
        add     edx, [days1year]
@@:
        add     [edi+6], ax
        push    esi edx
        mov     esi, months
        movzx   eax, word [edi+6]
        test    al, 3
        jnz     .noleap
        xor     edx, edx
        push    eax
        div     [_400]
        pop     eax
        test    edx, edx
        jz      .leap
        xor     edx, edx
        div     [_100]
        test    edx, edx
        jz      .noleap
.leap:
        mov     esi, months2
.noleap:
        pop     edx
        xor     eax, eax
        inc     eax
@@:
        sub     edx, [esi]
        jb      @f
        add     esi, 4
        inc     eax
        jmp     @b
@@:
        add     edx, [esi]
        pop     esi
        inc     edx
        mov     [edi+4], dl
        mov     [edi+5], al
        add     edi, 8
        ret

;----------------------------------------------------------------
ntfs_CreateFolder:
        mov     [ebp+NTFS.ntfsFolder], 1
        jmp     @f

ntfs_CreateFile:
        mov     [ebp+NTFS.ntfsFolder], 0
@@:
        cmp     byte [esi], 0
        jnz     @f
        xor     ebx, ebx
        movi    eax, ERROR_ACCESS_DENIED
        ret
@@: ; 1. Search file
        call    ntfs_lock
        stdcall ntfs_find_lfn, [esp+4]
        jnc     .found
        cmp     [ebp+NTFS.ntfsFragmentCount], 1
        jnz     ntfsUnsupported     ; record fragmented
        test    eax, eax
        jz      ntfsFail
        jmp     .notFound

.found:     ; rewrite
        cmp     [ebp+NTFS.ntfs_cur_iRecord], 16
        jc      ntfsDenied
        cmp     [ebp+NTFS.ntfsFolder], 1
        jz      ntfsDenied
        mov     [ebp+NTFS.ntfs_cur_attr], 0x80
        mov     [ebp+NTFS.ntfs_cur_offs], 0
        mov     [ebp+NTFS.ntfs_cur_size], 0
        call    ntfs_read_attr
        jc      ntfsDenied
        mov     eax, [ebp+NTFS.frs_buffer]
        cmp     word [eax+baseRecordReuse], 0
        jnz     ntfsUnsupported     ; auxiliary record
        cmp     byte [eax+hardLinkCounter], 1
        jnz     ntfsUnsupported     ; file copying required
        mov     ecx, [ebp+NTFS.ntfs_attr_offs]
        cmp     byte [ecx+nonResidentFlag], 1
        jnz     ntfsUnsupported     ; resident $DATA
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        add     eax, [ebx+12]
        adc     edx, 0
        cmp     edx, [ecx+attributeRealSize+4]
        jnz     ntfsUnsupported
        cmp     [ecx+attributeRealSize], eax
        jnz     ntfsUnsupported
        jmp     ntfs_WriteFile.write

.notFound:  ; create; check path folders
        cmp     dword [esp+4], 0
        jnz     ntfsNotFound
        cmp     byte [esi], 0
        jz      ntfsNotFound
; 2. Prepare directory record
        mov     ecx, esi
@@:         ; count characters
        inc     ecx
        cmp     byte [ecx], '/'
        jz      ntfsNotFound
        cmp     byte [ecx], 0
        jnz     @b
        sub     ecx, esi
        push    ecx
        lea     ecx, [ecx*2+52h]    ; precalculate index length
        add     ecx, 7              ; align 8
        and     ecx, not 7
        mov     edi, [ebp+NTFS.cur_index_buf]
        push    esi
        push    ecx
        cmp     dword [edi], 'INDX'
        jz      .indexRecord
        mov     esi, [ebp+NTFS.frs_buffer]  ; indexRoot
        mov     edx, [esi+recordRealSize]
        add     edx, ecx
        cmp     [esi+recordAllocatedSize], edx
        jnc     @f
        add     esp, 12
        jmp     ntfsUnsupported     ; indexAllocation required
@@:         ; index fits in the indexRoot
        mov     [esi+recordRealSize], edx
        mov     ecx, edx
        shr     ecx, 2
        rep movsd
        mov     edi, [ebp+NTFS.ntfs_attr_offs]
        sub     edi, [ebp+NTFS.frs_buffer]
        add     edi, [ebp+NTFS.cur_index_buf]
        mov     esi, [esp]
        add     [edi+sizeWithHeader], esi
        add     [edi+sizeWithoutHeader], esi
        mov     cl, [edi+attributeOffset]
        add     edi, ecx
        add     [edi+16+nodeRealSize], esi
        add     [edi+16+nodeAllocatedSize], esi
        sub     eax, [ebp+NTFS.cur_index_buf]
        add     eax, edi
        mov     edi, [ebp+NTFS.cur_index_buf]
        add     edi, edx
        sub     edi, 4
        jmp     .common

.indexRecord:
        mov     edx, [edi+28]
        add     edx, ecx
        cmp     [edi+32], edx
        jnc     @f
        add     esp, 12
        jmp     ntfsUnsupported     ; new node required
@@:         ; index fits in the node
        mov     [edi+28], edx
        lea     edi, [edi+edx+24-4]
.common:
        mov     esi, edi
        sub     esi, [esp]
        mov     ecx, esi
        sub     ecx, eax    ; eax = pointer in the node
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
        pop     eax
        pop     esi
        mov     [edi+indexAllocatedSize], ax     ; fill index with data
        mov     eax, [esp]
        lea     eax, [eax*2+42h]
        mov     [edi+indexRawSize], ax
        mov     eax, [ebp+NTFS.ntfs_attr_iRecord]
        mov     [edi+directoryRecordReference], eax
        mov     eax, [ebp+NTFS.frs_buffer]
        mov     eax, [eax+reuseCounter]
        mov     [edi+directoryReferenceReuse], ax
        mov     eax, [ebx+12]
        mov     [ebp+NTFS.fileRealSize], eax
        mov     [edi+fileRealSize], eax
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
        mov     [ebp+NTFS.indexOffset], edi
        mov     [edi+fileNameLength], cl
        add     edi, 52h
@@:         ; record filename
        lodsb
        call    ansi2uni_char
        stosw
        dec     ecx
        jnz     @b
        mov     eax, [ebp+NTFS.ntfsLastRead]
        mov     [ebp+NTFS.nodeLastRead], eax
        cmp     [ebp+NTFS.ntfsFolder], 0
        jz      @f
        mov     edi, [ebp+NTFS.indexOffset]
        mov     byte [edi+fileFlags+3], 16
        jmp     .mftBitmap

@@: ; 3. File data
        cmp     [ebp+NTFS.fileRealSize], 0
        jz      .mftBitmap
; One piece free space bitmap search engine
        mov     edi, [ebp+NTFS.BitmapBuffer]
        add     edi, [ebp+NTFS.BitmapStart]
        mov     eax, [ebp+NTFS.fileDataSize]
        shr     eax, 5
        jz      .small
        push    eax         ; bitmap dwords
        add     edi, 4
.start:
        mov     ecx, [ebp+NTFS.BitmapSize]
        add     ecx, [ebp+NTFS.BitmapBuffer]
        sub     ecx, edi
        shr     ecx, 2
@@:
        xor     eax, eax
        repnz scasd         ; search for empty dword
        jz      @f
        call    bitmapBuffering
        jmp     @b
@@:
        cmp     ecx, [esp]
        jnc     @f
        call    bitmapBuffering
        jmp     @b
@@:
        sub     edi, 4
        mov     ecx, [esp]
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
        bsf     ecx, eax    ; last dword
        jz      .done
        and     edx, 31
        cmp     ecx, edx
        jnc     .done
        add     esp, 8
        jmp     .start

.small:     ; less than 32 clusters
        mov     ecx, [ebp+NTFS.BitmapSize]
        sub     ecx, [ebp+NTFS.BitmapStart]
        shr     ecx, 2
.smStart:
        mov     eax, -1
        repz scasd          ; search for zero bits
        push    ecx
        test    ecx, ecx
        jnz     @f
        call    bitmapBuffering
        pop     eax
        jmp     .smStart
@@:
        sub     edi, 4
        mov     eax, [edi]
        not     eax
@@:
        bsf     ecx, eax    ; first 0
        jz      .again
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
        mov     eax, [edi+4]
        bsf     edx, eax
        jz      .got        ; empty
        add     edx, 32
        sub     edx, ecx
        cmp     edx, [ebp+NTFS.fileDataSize]
        jnc     .got        ; share between dwords
.again:
        add     edi, 4
        pop     ecx
        jmp     .smStart

.got:
        push    ecx         ; starting bit
        push    edi         ; starting dword
.done:      ; mark space
        mov     ecx, [esp+4]
        cmp     ecx, 32
        jc      @f
        xor     ecx, ecx
        add     dword [esp], 4
        mov     [esp+4], ecx
@@:
        mov     edi, [esp]
        xor     eax, eax
        dec     eax
        shr     eax, cl
        shl     eax, cl
        neg     ecx
        add     ecx, 32
        sub     ecx, [ebp+NTFS.fileDataSize]
        jc      @f
        shl     eax, cl     ; fits inside dword
        shr     eax, cl
        or      [edi], eax
        jmp     .writeData

@@:
        or      [edi], eax
        neg     ecx
        push    ecx
        shr     ecx, 5
        add     edi, 4
        xor     eax, eax
        dec     eax
        rep stosd
        pop     ecx
        and     ecx, 31
        shr     eax, cl
        shl     eax, cl
        not     eax
        or      [edi], eax
.writeData:
        pop     edx
        sub     edx, [ebp+NTFS.BitmapBuffer]
        shl     edx, 3
        pop     eax
        add     eax, edx
        pop     edx
        mov     [ebp+NTFS.fileDataStart], eax
        mul     [ebp+NTFS.sectors_per_cluster]
        mov     ecx, [ebp+NTFS.fileRealSize]
        add     ecx, 511
        shr     ecx, 9
        mov     ebx, [ebx+16]
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
        jz      ntfsUnsupported     ; no free records
        bts     [edi], ecx
; get record location
        sub     edi, [ebp+NTFS.mftBitmapBuffer]
        shl     edi, 3
        add     edi, ecx
        mov     [ebp+NTFS.newMftRecord], edi
        mov     eax, [ebp+NTFS.frs_size]
        shr     eax, 9
        mul     edi
        mov     [ebp+NTFS.ntfs_cur_iRecord], 0
        mov     [ebp+NTFS.ntfs_cur_attr], 0x80
        mov     [ebp+NTFS.ntfs_cur_offs], eax
        mov     [ebp+NTFS.ntfs_cur_size], 1
        mov     eax, [ebp+NTFS.frs_buffer]
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        call    ntfs_read_attr
        cmp     [ebp+NTFS.ntfs_cur_read], 0
        jnz     .mftRecord
; extend MFT $DATA
        mov     eax, [ebp+NTFS.mft_cluster]
        mul     [ebp+NTFS.sectors_per_cluster]
        cmp     eax, [ebp+NTFS.ntfsLastRead]
        jnz     ntfsUnsupported     ; auxiliary record
        mov     edi, [ebp+NTFS.ntfs_attr_offs]
        mov     ebx, [ebp+NTFS.sectors_per_cluster]
        shl     ebx, 9+3
        add     dword [edi+lastVCN], 8
        add     [edi+attributeAllocatedSize], ebx
        adc     byte [edi+attributeAllocatedSize+4], 0
        add     [edi+attributeRealSize], ebx
        adc     byte [edi+attributeRealSize+4], 0
        add     [edi+initialDataSize], ebx
        adc     byte [edi+initialDataSize+4], 0
        movzx   eax, byte [edi+dataRunsOffset]
        add     edi, eax
        mov     al, [edi]
        inc     edi
        shl     eax, 4
        shr     al, 4
        mov     cl, 4
        sub     cl, al
        shl     cl, 3
        add     ah, al
        shr     eax, 8
        cmp     byte [edi+eax], 0
        jnz     ntfsUnsupported     ; $MFT fragmented
        mov     al, 8
        mov     edx, [edi]
        rol     eax, cl
        rol     edx, cl
        add     eax, edx
        jc      ntfsUnsupported
        ror     eax, cl
        shr     edx, cl
        mov     [edi], eax
        add     edx, [ebp+NTFS.mft_cluster]
        mov     esi, edx
        mov     ecx, edx
        and     ecx, 7
        shr     edx, 3
        add     edx, [ebp+NTFS.BitmapBuffer]
        mov     ax, [edx]
        shr     ax, cl
        test    al, al
        jnz     ntfsUnsupported
        dec     al
        xchg    [edx], al
        mov     [edx+1], al
        stdcall kernel_alloc, ebx
        test    eax, eax
        jz      ntfsNoMemory
        mov     ecx, ebx
        shr     ecx, 2
        mov     edi, eax
        push    ebx
        mov     ebx, eax
        xor     eax, eax
        rep stosd
        mov     eax, esi
        mul     [ebp+NTFS.sectors_per_cluster]
        pop     ecx
        shr     ecx, 9
        call    fs_write64_sys  ; clear new records
        stdcall kernel_free, ebx
        mov     eax, esi
        shr     eax, 3+9
        mov     ebx, eax
        shl     ebx, 9
        add     ebx, [ebp+NTFS.BitmapBuffer]
        add     eax, [ebp+NTFS.BitmapLocation]
        mov     ecx, 1
        xor     edx, edx
        call    fs_write64_app  ; partition bitmap
        test    eax, eax
        jnz     ntfsDevice
        mov     eax, [ebp+NTFS.frs_buffer]
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        call    writeRecord     ; $MFT
        test    eax, eax
        jnz     ntfsDevice
        mov     eax, [ebp+NTFS.mftmirr_cluster]
        mul     [ebp+NTFS.sectors_per_cluster]
        mov     ebx, [ebp+NTFS.frs_buffer]
        movzx   ecx, word [ebx+updateSequenceSize]
        dec     ecx
        call    fs_write64_sys  ; $MFTMirr
        test    eax, eax
        jnz     ntfsDevice
        mov     eax, [ebp+NTFS.ntfs_cur_offs]
        add     [ebp+NTFS.ntfsLastRead], eax
.mftRecord:
        mov     esi, [ebp+NTFS.indexOffset]
        mov     edi, [ebp+NTFS.frs_buffer]
        xor     eax, eax
        movzx   ecx, word [esi+indexAllocatedSize]
        add     ecx, 8+30h+48h+50h+8
        push    ecx
        shr     ecx, 2
        rep stosd
        mov     edi, [ebp+NTFS.frs_buffer]
; record header
        mov     dword[edi], 'FILE'
        mov     byte [edi+updateSequenceOffset], 2ah
        mov     byte [edi+updateSequenceSize], 3
        mov     byte [edi+hardLinkCounter], 1
        mov     byte [edi+attributeOffset], 30h
        pop     dword[edi+recordRealSize]
        mov     word [edi+recordAllocatedSize], 1024
        mov     byte [edi+newAttributeID], 3
        rdtsc
        mov     [edi+2ah], ax
        add     edi, 30h
; $StandardInformation
        mov     byte [edi+attributeType], 10h
        mov     byte [edi+sizeWithHeader], 48h
        mov     byte [edi+sizeWithoutHeader], 30h
        mov     byte [edi+attributeOffset], 18h
        add     edi, 48h
; $FileName
        mov     byte [edi+attributeType], 30h
        mov     byte [edi+attributeID], 1
        mov     cx, [esi+indexRawSize]
        mov     [edi+sizeWithoutHeader], ecx
        mov     cx, [esi+indexAllocatedSize]
        add     ecx, 8
        mov     [edi+sizeWithHeader], ecx
        mov     byte [edi+attributeOffset], 18h
        mov     byte [edi+indexedFlag], 1
        add     edi, 18h
        add     esi, 16
        sub     ecx, 18h
        shr     ecx, 2
        rep movsd
        cmp     [ebp+NTFS.ntfsFolder], 0
        jnz     @f
; $Data
        mov     byte [edi+attributeType], 80h
        cmp     [ebp+NTFS.fileRealSize], 0
        jz      .zeroSize
        mov     esi, [ebp+NTFS.indexOffset]
        mov     byte [edi+nonResidentFlag], 1
        mov     byte [edi+dataRunsOffset], 40h
        mov     eax, [esi+fileAllocatedSize]
        mov     [edi+attributeAllocatedSize], eax
        mov     eax, [esi+fileRealSize]
        mov     [edi+attributeRealSize], eax
        mov     [edi+initialDataSize], eax
        mov     byte [edi+40h], 44h
        mov     eax, [ebp+NTFS.fileDataSize]
        mov     [edi+41h], eax
        dec     eax
        mov     [edi+lastVCN], eax
        mov     eax, [ebp+NTFS.fileDataStart]
        mov     [edi+45h], eax
        mov     al, 1
        jmp     .writeMftRecord

.zeroSize:
        mov     byte [edi+attributeOffset], 18h
        mov     al, 1
        jmp     .writeMftRecord

@@: ; $IndexRoot
        mov     byte [edi+attributeType], 90h
        mov     byte [edi+nameLength], 4
        mov     byte [edi+nameOffset], 18h
        mov     byte [edi+sizeWithoutHeader], 30h
        mov     byte [edi+attributeOffset], 20h
        mov     dword[edi+18h], 490024h     ; unicode $I30
        mov     dword[edi+18h+4], 300033h
        mov     byte [edi+20h+attributeType], 30h
        mov     byte [edi+20h+collationRule], 1
        mov     eax, [ebp+NTFS.sectors_per_cluster]
        shl     eax, 9
        mov     [edi+20h+indexRecordSize], eax
        mov     byte [edi+20h+indexRecordSizeClus], 1
        mov     byte [edi+30h+indexOffset], 16
        mov     byte [edi+30h+nodeRealSize], 32
        mov     byte [edi+30h+nodeAllocatedSize], 32
        mov     byte [edi+40h+indexAllocatedSize], 16
        mov     byte [edi+40h+indexFlags], 2
        mov     al, 3
.writeMftRecord:
        mov     byte [edi+sizeWithHeader], 50h
        mov     byte [edi+attributeID], 2
        mov     dword[edi+50h], -1      ; $End
        mov     edi, [ebp+NTFS.frs_buffer]
        mov     [edi+recordFlags], al
        mov     [ebp+NTFS.ntfs_cur_buf], edi
        call    writeRecord
        test    eax, eax
        jnz     ntfsDevice
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
; write MFT bitmap
        mov     eax, [ebp+NTFS.newMftRecord]
        shr     eax, 3+9
        mov     ebx, eax
        shl     ebx, 9
        add     eax, [ebp+NTFS.mftBitmapLocation]
        add     ebx, [ebp+NTFS.mftBitmapBuffer]
        mov     ecx, 1
        xor     edx, edx
        call    fs_write64_sys
        test    eax, eax
        jnz     ntfsDevice
; 5. Write partition bitmap
        cmp     [ebp+NTFS.ntfsFolder], 0
        jnz     @f
        cmp     [ebp+NTFS.fileRealSize], 0
        jz      @f
        mov     ecx, [ebp+NTFS.fileDataStart]
        mov     eax, ecx
        add     ecx, [ebp+NTFS.fileDataSize]
        add     ecx, 4095
        shr     ecx, 3+9
        shr     eax, 3+9
        sub     ecx, eax
        mov     ebx, eax
        shl     ebx, 9
        add     eax, [ebp+NTFS.BitmapLocation]
        add     ebx, [ebp+NTFS.BitmapBuffer]
        xor     edx, edx
        call    fs_write64_app
        test    eax, eax
        jnz     ntfsDevice
@@:
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
        mov     edi, [ebp+NTFS.indexOffset]
        mov     eax, [ebp+NTFS.newMftRecord]
        mov     [edi+fileRecordReference], eax
; 6. Write directory node
        mov     eax, [ebp+NTFS.nodeLastRead]
        mov     [ebp+NTFS.ntfsLastRead], eax
        mov     eax, [ebp+NTFS.cur_index_buf]
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        call    writeRecord
        test    eax, eax
        jnz     ntfsDevice
        mov     ebx, [ebp+NTFS.fileRealSize]
ntfsDone:
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
        call    ntfs_unlock
        xor     eax, eax
        ret

writeRecord:
;   in:
; [ebp+NTFS.ntfs_cur_buf] = record
; [ebp+NTFS.ntfsLastRead] = partition sector
    ; making updateSequence
        mov     esi, [ebp+NTFS.ntfs_cur_buf]
        mov     edi, esi
        movzx   ecx, word [esi+updateSequenceOffset]
        add     edi, ecx
        mov     ax, [edi]
        add     edi, 2
        mov     cx, [esi+updateSequenceSize]
        dec     ecx
        push    ecx
@@:
        add     esi, 510
        movsw
        mov     [esi-2], ax
        dec     ecx
        jnz     @b
; writing to disk
        mov     eax, [ebp+NTFS.ntfsLastRead]
        mov     ebx, [ebp+NTFS.ntfs_cur_buf]
        pop     ecx
        xor     edx, edx
        jmp     fs_write64_sys

bitmapBuffering:
; Extend BitmapBuffer and read next 32kb of bitmap
; Warning: $Bitmap fragmentation is not foreseen
; if edi -> position in bitmap buffer,
; then ecx = number of buffered dwords left
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
        add     [ebp+NTFS.BitmapSize], 8000h
        mov     eax, [ebp+NTFS.BitmapTotalSize]
        cmp     eax, [ebp+NTFS.BitmapSize]
        jnc     @f
        mov     [ebp+NTFS.BitmapSize], eax
@@:
        mov     ecx, [ebp+NTFS.BitmapSize]
        add     ecx, [ebp+NTFS.BitmapBuffer]
        sub     ecx, edi
        shr     ecx, 2
        pop     ebx
        ret

.err:
        mov     eax, [ebp+NTFS.BitmapBuffer]
        add     eax, [ebp+NTFS.BitmapSize]
        mov     ecx, 8
        call    release_pages
.end:
        add     esp, 12     ; double ret
        push    ERROR_DISK_FULL
        jmp     ntfsOut

;----------------------------------------------------------------
ntfs_WriteFile:
        cmp     byte [esi], 0
        jnz     @f
        xor     ebx, ebx
        movi    eax, ERROR_ACCESS_DENIED
        ret
@@:
        call    ntfs_lock
        stdcall ntfs_find_lfn, [esp+4]
        jc      ntfsNotFound
        cmp     [ebp+NTFS.ntfs_cur_iRecord], 16
        jc      ntfsDenied
        mov     [ebp+NTFS.ntfs_cur_attr], 0x80
        mov     [ebp+NTFS.ntfs_cur_offs], 0
        mov     [ebp+NTFS.ntfs_cur_size], 0
        call    ntfs_read_attr
        jc      ntfsDenied
        mov     eax, [ebp+NTFS.frs_buffer]
        cmp     word [eax+baseRecordReuse], 0
        jnz     ntfsUnsupported     ; auxiliary record
        cmp     byte [eax+hardLinkCounter], 1
        jnz     ntfsUnsupported     ; file copying required
        mov     ecx, [ebp+NTFS.ntfs_attr_offs]
        cmp     byte [ecx+nonResidentFlag], 1
        jnz     ntfsUnsupported     ; resident $DATA
        cmp     word [ecx+attributeFlags], 0
        jnz     ntfsUnsupported
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        add     eax, [ebx+12]
        adc     edx, 0
        cmp     edx, [ecx+attributeRealSize+4]
        jc      .write
        jnz     ntfsUnsupported     ; end of file
        cmp     [ecx+attributeRealSize], eax
        jc      ntfsUnsupported
.write:
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        mov     ecx, [ebx+12]
        mov     esi, [ebx+16]
        shrd    eax, edx, 9
        test    dword[ebx+4], 1FFh
        jz      .aligned
        mov     [ebp+NTFS.ntfs_cur_offs], eax
        mov     [ebp+NTFS.ntfs_cur_size], 1
        lea     edi, [ebp+NTFS.ntfs_bitmap_buf]
        mov     [ebp+NTFS.ntfs_cur_buf], edi
        call    ntfs_read_attr.continue
        jc      ntfsDevice
        mov     eax, [ebx+4]
        and     eax, 1FFh
        add     edi, eax
        sub     eax, [ebp+NTFS.ntfs_cur_read]
        neg     eax
        push    ecx
        cmp     ecx, eax
        jb      @f
        mov     ecx, eax
@@:
        sub     [esp], ecx
        rep movsb
        push    ebx
        mov     eax, [ebp+NTFS.ntfsLastRead]
        lea     ebx, [ebp+NTFS.ntfs_bitmap_buf]
        mov     ecx, 1
        xor     edx, edx
        call    fs_write64_app
        pop     ebx
        pop     ecx
        test    eax, eax
        jnz     ntfsDevice
        test    ecx, ecx
        jz      @f
        mov     eax, [ebx+4]
        mov     edx, [ebx+8]
        shrd    eax, edx, 9
        inc     eax
.aligned:
        push    ecx
        shr     ecx, 9
        mov     [ebp+NTFS.ntfs_cur_offs], eax
        mov     [ebp+NTFS.ntfs_cur_size], ecx
        mov     [ebp+NTFS.ntfs_cur_buf], esi
        add     eax, ecx
        push    eax
        mov     [ebp+NTFS.ntfsWriteAttr], 1
        call    ntfs_read_attr.continue
        mov     [ebp+NTFS.ntfsWriteAttr], 0
        pop     [ebp+NTFS.ntfs_cur_offs]
        pop     ecx
        jc      ntfsDevice
        and     ecx, 1FFh
        jz      @f
        add     esi, [ebp+NTFS.ntfs_cur_read]
        mov     [ebp+NTFS.ntfs_cur_size], 1
        lea     edi, [ebp+NTFS.ntfs_bitmap_buf]
        mov     [ebp+NTFS.ntfs_cur_buf], edi
        call    ntfs_read_attr.continue
        jc      ntfsDevice
        rep movsb
        push    ebx
        mov     eax, [ebp+NTFS.ntfsLastRead]
        lea     ebx, [ebp+NTFS.ntfs_bitmap_buf]
        mov     ecx, 1
        xor     edx, edx
        call    fs_write64_app
        pop     ebx
        test    eax, eax
        jnz     ntfsDevice
@@:
        mov     ebx, [ebx+12]
        jmp     ntfsDone

;----------------------------------------------------------------
ntfs_Delete:
        cmp     byte [esi], 0
        jnz     @f
        xor     ebx, ebx
        movi    eax, ERROR_ACCESS_DENIED
        ret
@@:
        call    ntfs_lock
        stdcall ntfs_find_lfn, [esp+4]
        jc      ntfsNotFound
        cmp     [ebp+NTFS.ntfs_cur_iRecord], 16
        jc      ntfsDenied
        cmp     [ebp+NTFS.ntfsFragmentCount], 1
        jnz     ntfsUnsupported     ; record fragmented
        test    byte [eax+indexFlags], 1
        jnz     ntfsUnsupported     ; index has a subnode
        mov     edx, [ebp+NTFS.ntfs_cur_iRecord]
        shr     edx, 3
        cmp     edx, [ebp+NTFS.mftBitmapSize]
        jnc     ntfsUnsupported
; delete index from the node
        movzx   edx, word [eax+indexAllocatedSize]
        mov     edi, [ebp+NTFS.cur_index_buf]
        cmp     dword [edi], 'INDX'
        jz      .indexRecord
        mov     esi, [ebp+NTFS.frs_buffer]  ; indexRoot
        mov     ecx, [esi+recordRealSize]
        shr     ecx, 2
        rep movsd
        mov     esi, [ebp+NTFS.cur_index_buf]
        mov     edi, [ebp+NTFS.ntfs_attr_offs]
        sub     edi, [ebp+NTFS.frs_buffer]
        add     edi, esi
        sub     [edi+sizeWithHeader], edx
        sub     [edi+sizeWithoutHeader], edx
        mov     cl, [edi+attributeOffset]
        add     edi, ecx
        sub     [edi+16+nodeRealSize], edx
        sub     [edi+16+nodeAllocatedSize], edx
        sub     eax, esi
        add     eax, edi
        sub     [esi+recordRealSize], edx
        mov     ecx, [esi+recordRealSize]
        jmp     @f

.indexRecord:
        sub     [edi+28], edx
        mov     ecx, [edi+28]
        add     ecx, 24
@@:
        add     ecx, [ebp+NTFS.cur_index_buf]
        sub     ecx, eax
        shr     ecx, 2
        mov     esi, eax
        add     esi, edx
        mov     edi, eax
        rep movsd
        mov     eax, [ebp+NTFS.ntfsLastRead]
        mov     [ebp+NTFS.nodeLastRead], eax
; examine file record
        mov     [ebp+NTFS.ntfs_cur_attr], 0x80
        mov     [ebp+NTFS.ntfs_cur_offs], 0
        mov     [ebp+NTFS.ntfs_cur_size], 0
        call    ntfs_read_attr
        jc      .folder
        mov     esi, [ebp+NTFS.frs_buffer]
        cmp     word [esi+baseRecordReuse], 0
        jnz     ntfsUnsupported     ; auxiliary record
        cmp     byte [esi+hardLinkCounter], 2
        jnc     .writeFileRecord    ; delete hard link
        mov     esi, [ebp+NTFS.ntfs_attr_offs]
        cmp     byte [esi+nonResidentFlag], 0
        jz      .writeBitmapMFT
        movzx   eax, byte [esi+dataRunsOffset]
        add     esi, eax
        xor     edi, edi
        sub     esp, 16
.clearBitmap:       ; "delete" file data
        call    ntfs_decode_mcb_entry
        jnc     .mcbEnd
        cmp     dword[esp+8], 0
        jz      .clearBitmap
        add     edi, [esp+8]
        mov     ebx, [esp]
        mov     eax, edi
        add     eax, ebx
        shr     eax, 3
        inc     eax
        cmp     eax, [ebp+NTFS.BitmapSize]
        jc      .buffered
        add     eax, [ebp+NTFS.BitmapBuffer]
        add     esp, 16
        push    edi
        mov     edi, eax
@@:
        call    bitmapBuffering
        shl     ecx, 2
        js      @b
        pop     edi
        sub     esp, 16
.buffered:
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
        pop     edi
        mov     ecx, edi
        add     ecx, ebx
        add     ecx, 4095
        shr     ecx, 3+9
        mov     eax, edi
        shr     eax, 3+9
        sub     ecx, eax
        mov     ebx, eax
        shl     ebx, 9
        add     eax, [ebp+NTFS.BitmapLocation]
        add     ebx, [ebp+NTFS.BitmapBuffer]
        xor     edx, edx
        call    fs_write64_app
        jmp     .clearBitmap

.mcbEnd:
        add     esp, 16
        jmp     .writeBitmapMFT

.folder:    ; empty?
        lea     esi, [ebp+NTFS.ntfs_bitmap_buf]
        mov     [ebp+NTFS.ntfs_cur_buf], esi
        mov     [ebp+NTFS.ntfs_cur_attr], 0x90
        mov     [ebp+NTFS.ntfs_cur_offs], 0
        mov     [ebp+NTFS.ntfs_cur_size], 1
        call    ntfs_read_attr
        cmp     [ebp+NTFS.ntfs_cur_read], 48
        jnz     ntfsDenied
        test    byte [esi+32+indexFlags], 1
        jnz     ntfsDenied
.writeBitmapMFT:    ; "delete" file record
        mov     eax, [ebp+NTFS.ntfs_cur_iRecord]
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
        mov     esi, [ebp+NTFS.frs_buffer]
        mov     byte [esi+recordFlags], 0
.writeFileRecord:
        dec     byte [esi+hardLinkCounter]
        mov     [ebp+NTFS.ntfs_cur_buf], esi
        call    writeRecord
; write directory node
        mov     eax, [ebp+NTFS.nodeLastRead]
        mov     [ebp+NTFS.ntfsLastRead], eax
        mov     eax, [ebp+NTFS.cur_index_buf]
        mov     [ebp+NTFS.ntfs_cur_buf], eax
        call    writeRecord
        test    eax, eax
        jz      ntfsDone
        jmp     ntfsDevice

;----------------------------------------------------------------
ntfs_SetFileEnd:
ntfs_SetFileInfo:
        movi    eax, ERROR_UNSUPPORTED_FS
        ret

;----------------------------------------------------------------
ntfs_GetFileInfo:
        cmp     byte [esi], 0
        jnz     @f
        movi    eax, ERROR_UNSUPPORTED_FS
        ret
@@:
        call    ntfs_lock
        stdcall ntfs_find_lfn, [esp+4]
        jnc     .found
        test    eax, eax
        jz      ntfsFail
        jmp     ntfsNotFound
.found:
        push    esi edi
        mov     esi, eax
        mov     edi, [ebx+16]
        xor     eax, eax
        call    ntfs_direntry_to_bdfe
        pop     edi esi
        call    ntfs_unlock
        xor     eax, eax
        ret

ntfsUnsupported:
        push    ERROR_UNSUPPORTED_FS
ntfsOut:
        call    ntfs_unlock
        xor     ebx, ebx
        pop     eax
        ret
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
ntfsNoMemory:
        push    ERROR_OUT_OF_MEMORY
        jmp     ntfsOut
