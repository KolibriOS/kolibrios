;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2021-2022. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; exFAT external functions
;   in:
; ebx -> parameter structure of sysfunc 70
; ebp -> exFAT structure
; esi -> path string in UTF-8
;   out:
; eax, ebx = return values for sysfunc 70
iglobal
align 4
exFAT_user_functions:
        dd      exFAT_free
        dd      (exFAT_user_functions_end - exFAT_user_functions - 4) / 4
        dd      exFAT_ReadFile
        dd      exFAT_ReadFolder
        dd      0 ;exFAT_CreateFile
        dd      0 ;exFAT_Write
        dd      0 ;exFAT_SetFileEnd
        dd      exFAT_GetFileInfo
        dd      exFAT_SetFileInfo
        dd      0
        dd      0 ;exFAT_Delete
        dd      0 ;exFAT_CreateFolder
        dd      0 ;exFAT_Rename
exFAT_user_functions_end:
endg

struct exFAT PARTITION
fat_change          db  ?   ; 1=fat has changed
createOption        db  ?
                    rb  2
Lock                MUTEX   ; currently operations with one partition
; can not be executed in parallel since the legacy code is not ready
fat_in_cache        dd  ?
FAT_START           dd  ?   ; start of fat table
ROOT_START          dd  ?   ; start of rootdir
SECTORS_PER_CLUSTER dd  ?
BYTES_PER_SECTOR    dd  ?   ; Note: if BPS <> 512 need lots of changes
SECTORS_PER_FAT     dd  ?
NUMBER_OF_FATS      dd  ?
CLUSTER_HEAP_START  dd  ?
CLUSTER_COUNT       dd  ?
DATA_START          dd  ?   ; start of data area (=first cluster 2)
LAST_CLUSTER        dd  ?   ; last availabe cluster
fatRESERVED         dd  ?
fatBAD              dd  ?
fatEND              dd  ?
fatMASK             dd  ?
fatStartScan        dd  ?
cluster_tmp         dd  ?   ; used by analyze_directory and analyze_directory_to_write
ROOT_CLUSTER        dd  ?   ; first rootdir cluster
fat_cache_ptr       dd  ?
points_to_BDFE      dd  ?
secondary_dir_entry dd  ?
longname_sec1       dd  ?   ; used by analyze_directory to save 2 previous
longname_sec2       dd  ?   ; directory sectors for delete long filename
LFN_reserve_place   dd  ?
path_in_UTF8        dd  ?
General_Sec_Flags   dd  ?
valid_data_length   dd  ?
RAX_high            dd  ?
RCX_high            dd  ?
RDX_high            dd  ?
RDI_high            dd  ?
current_hash        dd  ?
hash_flag           dd  ?
need_hash           dd  ?
buffer_curr_sector  dd  ?
buff_file_dirsect   dd  ?
buff_file_dir_pos   dd  ?
fname_extdir_offset dd  ?
volumeLabel         rb  12
; The next nine areas (32*17) should be arranged sequentially.
; Do not change their location!!!
file_dir_entry      rb  32 ; Entry Type 0x85
str_ext_dir_entry   rb  32 ; Entry Type 0xC0
fname_ext_dir_entry rb  32*17 ; Entry Type 0xC1 * 17 Entries
buffer              rb  512
ends

; these labels are located before the main function to make
; most of jumps to these be short
exFAT_create_partition.free_return0:
        mov     eax, ebp
        call    free
        pop     ebp
; DEBUGF  1, "K : exFAT_create_partition.free_return0 EAX: %x\n", eax
exFAT_create_partition.return0:
        xor     eax, eax
; DEBUGF  1, "K : exFAT_create_partition.return0 EAX: %x\n", eax
        ret

; Mount if it's a valid exFAT partition.
exFAT_create_partition:
; DEBUGF  1, "K : exFAT_create_partition EAX: %x\n", eax
;   in:
; ebp -> PARTITION structure
; ebx -> boot sector
; ebx+512 -> buffer
;   out:
; eax -> exFAT structure, 0 = not exFAT
        cmp     dword [esi+DISK.MediaInfo.SectorSize], 512
        jnz     .return0
; DEBUGF  1, "K : exFAT DISK.MediaInfo.SectorSize=512\n"
; bootsector must have been successfully read
        cmp     dword [esp+4], 0
        jnz     .return0
; DEBUGF  1, "K : exFAT bootsector successfully read\n"
; offset +510 = bootsector signature must be correct
        cmp     word [ebx+0x1fe], 0xaa55
        jnz     .return0
; DEBUGF  1, "K : exFAT bootsector signature correct\n"
; offset +109 = sectors per cluster must be nonzero
        cmp     byte [ebx+0x6d], 0
        jz      .return0
; DEBUGF  1, "K : exFAT sectors per cluster = nonzero\n"
; offset +108 = bytes per sector must be 0x200
; (LEGACY! In the future, increase support to 4096)
; the value for exFAT is a power of 2
        cmp     byte [ebx+0x6c], 9
        jnz     .return0
; DEBUGF  1, "K : exFAT bytes per sector = 512\n"
; Test name = 'EXFAT   '
        cmp     dword [ebx+3], 'EXFA'
        jnz     .return0
        cmp     dword [ebx+7], 'T   '
        jnz     .return0
; DEBUGF  1, "K : exFAT Test name EXFAT = OK \n"
        movi    eax, sizeof.exFAT
        call    malloc
        test    eax, eax
        jz      .return0
; DEBUGF  1, "K : exFAT malloc sizeof.exFAT = OK \n"
        mov     ecx, dword [ebp+PARTITION.FirstSector]
        mov     dword [eax+exFAT.FirstSector], ecx
; DEBUGF  1, "K : exFAT PARTITION.FirstSector ECX: %x\n", ecx
        mov     ecx, dword [ebp+PARTITION.FirstSector+4]
        mov     dword [eax+exFAT.FirstSector+4], ecx
; DEBUGF  1, "K : exFAT PARTITION.FirstSector+4 ECX: %x\n", ecx
        mov     ecx, dword [ebp+PARTITION.Length]
        mov     dword [eax+exFAT.Length], ecx
; DEBUGF  1, "K : exFAT PARTITION.Length ECX: %x\n", ecx
        mov     ecx, dword [ebp+PARTITION.Length+4]
        mov     dword [eax+exFAT.Length+4], ecx
; DEBUGF  1, "K : exFAT PARTITION.Length+4 ECX: %x\n", ecx
        mov     ecx, [ebp+PARTITION.Disk]
        mov     [eax+exFAT.Disk], ecx
; DEBUGF  1, "K : exFAT PARTITION.Disk ECX: %x\n", ecx
        mov     [eax+exFAT.FSUserFunctions], exFAT_user_functions
        or      [eax+exFAT.fat_in_cache], -1
        mov     [eax+exFAT.fat_change], 0

        push    ebp
        mov     ebp, eax
        lea     ecx, [ebp+exFAT.Lock]
        call    mutex_init
; offset +80 = sectors reserved
        mov     eax, [ebx+0x50]
        mov     [ebp+exFAT.FAT_START], eax
; DEBUGF  1, "K : exFAT.FAT_START EAX: %x\n", eax
; offset +109 = sectors per cluster. This is power of 2; Minimal value is 1;
; 2^0 =1 sector (512 Bytes) and maximum 32 MB cluster size in bytes
        movzx   ecx, byte [ebx+0x6d]
        mov     eax, 1
        shl     eax, cl
        mov     [ebp+exFAT.SECTORS_PER_CLUSTER], eax
; DEBUGF  1, "K : exFAT.SECTORS_PER_CLUSTER EAX: %x\n", eax
; offset +108 = bytes per sector. This is power of 2; Minimal value is 9;
; 2^9 =512 Bytes and maximum 2^12 =4096 Bytes
        movzx   ecx, byte [ebx+0x6c]     ; bytes per sector
        mov     eax, 1
        shl     eax, cl
        mov     [ebp+exFAT.BYTES_PER_SECTOR], eax
; DEBUGF  1, "K : exFAT.BYTES_PER_SECTOR EAX: %x\n", eax
;------------------------------------------------------------------------------
;        movzx   eax, word [ebx+0x11]    ; count of rootdir entries (=0 fat32)
;        shl     eax, 5                  ; mul 32
;        dec     ecx
;        add     eax, ecx                ; round up if not equal count
;        inc     ecx                     ; bytes per sector
;        xor     edx, edx
;        div     ecx
;        mov     [ebp+FAT.ROOT_SECTORS], eax     ; count of rootdir sectors
;------------------------------------------------------------------------------
; offset +84 = Size of FAT in sectors
        mov     eax, [ebx+0x54]         ; sectors per fat
        mov     [ebp+exFAT.SECTORS_PER_FAT], eax
; DEBUGF  1, "K : exFAT.SECTORS_PER_FAT EAX: %x\n", eax
;------------------------------------------------------------------------------
; offset +88 = Starting sector of cluster heap
        mov     eax, [ebx+0x58]         ; Cluster offset
        mov     [ebp+exFAT.CLUSTER_HEAP_START], eax
; DEBUGF  1, "K : exFAT.CLUSTER_HEAP_START EAX: %x\n", eax
;------------------------------------------------------------------------------
; offset +92 = Number of clusters
        mov     eax, [ebx+0x5c]         ; Cluster count
        mov     [ebp+exFAT.CLUSTER_COUNT], eax
; DEBUGF  1, "K : exFAT.CLUSTER_COUNT EAX: %x\n", eax
;------------------------------------------------------------------------------
; offset +110 = Either 1 or 2; if TexFAT is supported then it will be 2
        movzx   eax, byte [ebx+0x6e]    ; number of fats
        mov     [ebp+exFAT.NUMBER_OF_FATS], eax
; DEBUGF  1, "K : exFAT.NUMBER_OF_FATS EAX: %x\n", eax
        mul     [ebp+exFAT.SECTORS_PER_FAT]
; DEBUGF  1, "K : exFAT.SECTORS_PER_FAT mul EAX: %x\n", eax
;        test    edx, edx
;        jnz     .free_return0
        add     eax, [ebp+exFAT.FAT_START]
        jc      .free_return0

;        mov     [ebp+FAT.ROOT_START], eax       ; rootdir = fat_start + fat_size * fat_count
;        add     eax, [ebp+FAT.ROOT_SECTORS]     ; rootdir sectors should be 0 on fat32
;        jc      .free_return0
        mov     [ebp+exFAT.DATA_START], eax
; DEBUGF  1, "K : exFAT.DATA_START EAX: %x\n", eax
;------------------------------------------------------------------------------
; offset +72 = Total number of Sectors
        mov     eax, [ebx+0x48+4]         ; total sector count high part
        test    eax, eax
        jnz     .free_return0           ; 32-BIT LIMIT - MODIFY LATER WITY RASP !!!
; DEBUGF  1, "K : exFAT Total number of Sectors+4 EAX: %x\n", eax
        mov     eax, [ebx+0x48]         ; total sector count low part
; DEBUGF  1, "K : exFAT Total number of Sectors EAX: %x\n", eax
        cmp     dword [ebp+exFAT.Length+4], 0
        jnz     @f
; DEBUGF  1, "K : exFAT.Length+4 = 0\n"
        cmp     eax, dword [ebp+exFAT.Length]
        ja      .free_return0
; DEBUGF  1, "K : exFAT.Length >= Total number of Sectors\n"
@@:
        mov     dword [ebp+exFAT.Length], eax
        and     dword [ebp+exFAT.Length+4], 0
        sub     eax, [ebp+exFAT.DATA_START]       ; eax = count of data sectors
        jc      .free_return0
; DEBUGF  1, "K : EAX - exFAT.DATA_START EAX: %x\n", eax
        xor     edx, edx
        div     [ebp+exFAT.SECTORS_PER_CLUSTER]
        inc     eax
        mov     [ebp+exFAT.LAST_CLUSTER], eax
; DEBUGF  1, "K : exFAT.LAST_CLUSTER EAX: %x\n", eax
        dec     eax                     ; cluster count
        jz      .free_return0
; DEBUGF  1, "K : exFAT.LAST_CLUSTER >= 2 EAX: %x\n",eax
        mov     [ebp+exFAT.fatStartScan], 2
;        cmp     eax, 0xfff5
;        jb      .fat16
;------------------------------------------------------------------------------
;.fat32:
;        pusha
;        lea     esi, [ebx+71]
;        lea     edi, [ebp+exFAT.volumeLabel]
;        movsd
;        movsd
;        movsd
;        popa
;------------------------------------------------------------------------------
; offset +96 = First cluster of root directory 
        mov     eax, [ebx+0x60]         ; rootdir cluster
        mov     [ebp+exFAT.ROOT_CLUSTER], eax
; DEBUGF  1, "K : exFAT.ROOT_CLUSTER EAX: %x\n", eax
        dec     eax
        dec     eax
        imul    eax, [ebp+exFAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+exFAT.CLUSTER_HEAP_START]
        mov     [ebp+exFAT.ROOT_START], eax
; DEBUGF  1, "K : exFAT.ROOT_START EAX: %x\n", eax
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;        movzx   eax, word [ebx+0x30]
;        mov     [ebp+FAT.ADR_FSINFO], eax
;------------------------------------------------------------------------------
;        push    ebx
;        add     ebx, 512
;        call    fs_read32_sys
;        test    eax, eax
;        jnz     @f
;        mov     eax, [ebx+0x1ec]
;        cmp     eax, -1
;        jz      @f
;        mov     [ebp+exFAT.fatStartScan], eax
;@@:
;        pop     ebx
;------------------------------------------------------------------------------
        mov     [ebp+exFAT.fatRESERVED], 0x0FFFFFF6
        mov     [ebp+exFAT.fatBAD], 0x0FFFFFF7
        mov     [ebp+exFAT.fatEND], 0x0FFFFFF8
        mov     [ebp+exFAT.fatMASK], 0x0FFFFFFF
;------------------------------------------------------------------------------
;        mov     al, 32
;.fat_not_12_finalize:
;        mov     [ebp+FAT.fs_type], al
;------------------------------------------------------------------------------
; For FAT16 and FAT32, allocate 512 bytes for FAT cache.
        mov     eax, 512
        call    malloc
        test    eax, eax
        jz      .free_return0
        mov     [ebp+exFAT.fat_cache_ptr], eax
; DEBUGF  1, "K : malloc exFAT.fat_cache_ptr EAX: %x\n", eax

        mov     eax, ebp
        pop     ebp
        ret
;------------------------------------------------------------------------------
exFAT_free:
        push    eax
        mov     eax, [eax+exFAT.fat_cache_ptr]
; DEBUGF  1, "K : exFAT_free exFAT.fat_cache_ptr EAX: %x\n", eax
        call    free
        pop     eax
        jmp     free
;------------------------------------------------------------------------------
exFAT_lock:
; DEBUGF  1, "K : exFAT_lock \n"
        lea     ecx, [ebp+exFAT.Lock]
        jmp     mutex_lock

exFAT_unlock:
; DEBUGF  1, "K : exFAT_unlock \n"
        lea     ecx, [ebp+exFAT.Lock]
        jmp     mutex_unlock
;------------------------------------------------------------------------------
exFAT_get_name:
        cmp     byte [edi], 0
        jz      .no
; DEBUGF  1, "K : exFAT_get_name EDI:%x [EDI]:%x\n", edi, [edi]
;        push    ebp
;        mov     ebp,[esp+12+8+4+4+7*4+262*2+4+4]
; DEBUGF  1, "K : exFAT_get_name START Input FS EBP:%x\n", ebp
;        pop     ebp
; in: edi -> FAT entry, esi -> buffer for UTF-16 name
; out: CF=1 -> no valid entry
        cmp     byte [edi], 0x85 ; File/Folder Directory Entry of ExFAT
        jz      .file_directory_entry
        cmp     byte [edi], 0xC0 ; Stream Extension Directory Entry of ExFAT
        jz      .stream_extension_directory_entry
        cmp     byte [edi], 0xC1 ; File Name Extension Directory Entry of ExFAT
        jz      .longname
        push    edi esi
        xchg    esi, edi
; DEBUGF  1, "K : exFAT Volume label dword [ESI]: %x\n", [esi]
        cmp     byte [esi], 0x83 ; Indicates that the Volume label exists
        jnz     .no_label
.label:
; DEBUGF  1, "K : exFAT_get_name.label \n"
        add     esi, 2
        lea     edi, [ebp+exFAT.volumeLabel]
        
        push    ecx
        mov     ecx, 12
        call    UTF16to8_string
        pop     ecx
        
;        push    edi
;        lea     edi, [ebp+exFAT.volumeLabel]
; DEBUGF  1, "K : exFAT Volume label: %s\n", edi
;        pop     edi
.no_label:
; DEBUGF  1, "K : exFAT_get_name.no_label \n"
        pop     esi edi
.no:
; DEBUGF  1, "K : exFAT_get_name.no \n"
        stc
        ret
;--------------------------------------
.file_directory_entry:
; DEBUGF  1, "K : exFAT_get_name 0x85\n"
        mov     eax, [ebp+exFAT.buffer_curr_sector]
        mov     [ebp+exFAT.buff_file_dirsect], eax
        mov     [ebp+exFAT.buff_file_dir_pos], edi

        lea     eax, [ebp+exFAT.fname_ext_dir_entry]
        mov     [ebp+exFAT.fname_extdir_offset], eax

        xor     eax, eax
        mov     [ebp+exFAT.hash_flag], eax ; dword 0
        mov     al, byte [edi+1]  ; Number of Secondary directory entries
        dec     eax
        mov     [ebp+exFAT.secondary_dir_entry], eax
; DEBUGF  1, "K : exFAT_get_name 0x85 SDE: %x\n", eax
        lea     esi, [ebp+exFAT.file_dir_entry]
; DEBUGF  1, "K : exFAT.file_dir_entry ESI: %x [ESI]: %x\n", esi, [esi]
        jmp     @f
;--------------------------------------
.stream_extension_directory_entry:
; DEBUGF  1, "K : exFAT_get_name 0xC0\n"
; DEBUGF  1, "K : exFAT SEDE need_hash :%x\n", [ebp+exFAT.need_hash]
        mov     eax, [ebp+exFAT.need_hash]
        test    eax, eax
        jz      .stream_extension_directory_entry_1 ; @f
        movzx   eax, word [edi+4] ; hash of the file name
; DEBUGF  1, "K : exFAT hash 1 :%x\n", eax
; DEBUGF  1, "K : exFAT hash 2 :%x\n", [ebp+exFAT.current_hash]
        cmp     eax, [ebp+exFAT.current_hash]
        je      .stream_extension_directory_entry_1 ; @f
        xor     eax, eax
        inc     eax
        mov     [ebp+exFAT.hash_flag], eax ; dword 1
; DEBUGF  1, "K : exFAT hashes don't match! \n"
.stream_extension_directory_entry_1:
        lea     esi, [ebp+exFAT.str_ext_dir_entry]
; DEBUGF  1, "K : exFAT.str_ext_dir_entry ESI: %x [ESI]: %x\n", esi, [esi]
@@:
        push    edi
        xchg    esi, edi
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        pop     edi
;        lea     esi, [esp+20]

        mov     esi, [ebp+exFAT.LFN_reserve_place]
        mov     word [esi+260*2], 0     ; force null-terminating for orphans
        jmp     .no
;--------------------------------------
.longname:
; DEBUGF  1, "K : exFAT_get_name 0xC1\n"
;        push    ebp
;        mov     ebp,[esp+12+8+4+4+7*4+262*2+4+4]
; DEBUGF  1, "K : exFAT_get_name.longname 0 Input FS EBP:%x\n", ebp
;        pop     ebp
; DEBUGF  1, "K : exFAT_get_name.longname \n"
; DEBUGF  1, "K : exFAT_get_name.longname EDI:%x [EDI]:%x ESI:%x [ESI]:%x EBP:%x NAME:%s\n", edi, [edi], esi, [esi], ebp, ebp
; copy name (15 chars in UTF-16)
;        mov     word [esi+260*2], 0     ; force null-terminating for orphans

;        push    ebp
;        mov     ebp,[esp+12+8+4+4+7*4+262*2+4+4]
; DEBUGF  1, "K : exFAT_get_name.longname Input FS EBP:%x\n", ebp
;        pop     ebp
 
        mov     eax, [ebp+exFAT.hash_flag]
        test    eax, eax
        jnz     .no
; DEBUGF  1, "K : exFAT_get_name.longname hash match! \n"
; copy File Name Extension Directory Entry [0xC1] for calculate SetChecksum Field
        lea     eax, [ebp+exFAT.fname_ext_dir_entry+32*17]
        cmp     eax, [ebp+exFAT.fname_extdir_offset]
        je      .no

        push    edi esi
        xchg    esi, edi
        mov     edi, [ebp+exFAT.fname_extdir_offset]
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        mov     [ebp+exFAT.fname_extdir_offset], edi
        pop     esi edi
; copy name        
        push    edi esi
        xchg    esi, edi
        add     esi, 2

        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        movsw
; force null-terminating for incomplete name
        xor     eax, eax
        stosw
        pop     esi edi
        
;        push    ebp
;        mov     ebp,[esp+12+8+4+4+7*4+262*2+4+4]
; DEBUGF  1, "K : exFAT_get_name.longname Output FS EBP:%x\n", ebp
;        pop     ebp

        mov     eax, [ebp+exFAT.secondary_dir_entry]
        dec     eax
        mov     [ebp+exFAT.secondary_dir_entry], eax
        jz      @f
        add     esi, 30
; DEBUGF  1, "K : exFAT_get_name 0xC1 CONT\n"
        jmp     .no
;        test    ax, ax
;        jnz     .no ; if this is not first entry, more processing required
@@:
;         mov     esi, [ebp+exFAT.LFN_reserve_place]
; DEBUGF  1, "K : exFAT_get_name.longname END \n"
; DEBUGF  1, "K : exFAT_get_name 0xC1 END\n"
        ret
;------------------------------------------------------------------------------
exFAT_entry_to_bdfe:
; DEBUGF  1, "K : exFAT_ReadFolder exFAT_entry_to_bdfe \n"
; convert FAT entry at edi to BDFE (block of data of folder entry) at esi, advance esi
        mov     eax, [ebp-4]
        mov     [esi+4], eax    ; cp866/UNICODE name
exFAT_entry_to_bdfe2:
;        movzx   eax, byte [edi+11]
        movzx   eax, byte [edi+4]
        mov     [esi], eax      ; attributes

;        movzx   eax, word [edi+14]
        movzx   eax, word [edi+8]
        call    fat_time_to_bdfe
        mov     [esi+8], eax    ; creation time

;        movzx   eax, word [edi+16]
        movzx   eax, word [edi+8+2]
        call    fat_date_to_bdfe
        mov     [esi+12], eax   ; creation date

;        and     dword [esi+16], 0       ; last access time is not supported on FAT
        movzx   eax, word [edi+16]
        call    fat_time_to_bdfe
        mov     [esi+16], eax    ; last access time

;        movzx   eax, word [edi+18]
        movzx   eax, word [edi+16+2]
        call    fat_date_to_bdfe
        mov     [esi+20], eax   ; last access date

;        movzx   eax, word [edi+22]
        movzx   eax, word [edi+12]
        call    fat_time_to_bdfe
        mov     [esi+24], eax   ; last write time

;        movzx   eax, word [edi+24]
        movzx   eax, word [edi+12+2]
        call    fat_date_to_bdfe
        mov     [esi+28], eax   ; last write date

        mov     al, [esi]
        test    al, 10000b
        jz      .file_size
        xor     eax, eax
        mov     [esi+32], eax   ; file size (low dword)
        mov     [esi+36], eax   ; file size (high dword)
        jmp     @f
.file_size:
        mov     eax, [edi+32+8]
        mov     [esi+32], eax   ; file size (low dword)
        mov     eax, [edi+32+8+4]
        mov     [esi+36], eax   ; file size (high dword)
@@:
        test    ebp, ebp
        jz      .ret
.copy_path:
        add     esi, 40
        push    edi esi
        mov     edi, esi
        mov     esi, ebp
        cmp     byte [ebp-4], 2
        jz      .utf16
        cmp     byte [ebp-4], 3
        jz      .utf8
@@:
        lodsw
        call    uni2ansi_char
        stosb
        test    al, al
        jnz     @b
        pop     esi edi
        add     esi, 264
.ret:
; DEBUGF  1, "K : exFAT_entry_to_bdfe +264 ESI:%x\n", esi
        ret

.utf8:
        push    ecx
        mov     ecx, 519
        call    UTF16to8_string
        pop     ecx
        jmp     @f

.utf16:
        lodsw
        stosw
        test    eax, eax
        jnz     .utf16
@@:
        pop     esi edi
        add     esi, 520
; DEBUGF  1, "K : exFAT_entry_to_bdfe +520 ESI:%x\n", esi
        ret
;------------------------------------------------------------------------------
exFAT_bdfe_to_fat_entry:
; convert BDFE at edx to FAT entry at edi
; destroys eax
; attributes byte
;        test    byte [edi+11], 8        ; volume label?
;        jnz     @f
        mov     al, [edx]
;        movzx   eax, al
; DEBUGF  1, "K : bdfe file attributes: %x\n", eax
        and     al, 0x27
;        and     byte [edi+11], 0x10
;        or      byte [edi+11], al
        and     byte [edi+4], 0x10
        or      byte [edi+4], al
;        movzx   eax, byte [edi+4]
; DEBUGF  1, "K : exFAT file attributes: %x\n", eax
;@@:
        mov     eax, [edx+8]
; DEBUGF  1, "K : bdfe creation time: %x\n", eax
        call    bdfe_to_fat_time
;        mov     [edi+14], ax            ; creation time
        mov     [edi+8], ax            ; creation time
; DEBUGF  1, "K : exFAT creation time: %x\n", ax

        mov     eax, [edx+12]
; DEBUGF  1, "K : bdfe creation date: %x\n", eax
        call    bdfe_to_fat_date
;        mov     [edi+16], ax            ; creation date
        mov     [edi+8+2], ax            ; creation date
; DEBUGF  1, "K : exFAT creation date: %x\n", ax

        mov     eax, [edx+16]
; DEBUGF  1, "K : bdfe access time: %x\n", eax
        call    bdfe_to_fat_time
        mov     [edi+16], ax            ; last access time
; DEBUGF  1, "K : exFAT access time: %x\n", ax

        mov     eax, [edx+20]
; DEBUGF  1, "K : bdfe access date: %x\n", eax
        call    bdfe_to_fat_date
;        mov     [edi+18], ax            ; last access date
        mov     [edi+16+2], ax            ; last access date
; DEBUGF  1, "K : exFAT access date: %x\n", ax

        mov     eax, [edx+24]
; DEBUGF  1, "K : bdfe write time: %x\n", eax
        call    bdfe_to_fat_time
;        mov     [edi+22], ax            ; last write time
        mov     [edi+12], ax            ; last write time
; DEBUGF  1, "K : exFAT write time: %x\n", ax

        mov     eax, [edx+28]
; DEBUGF  1, "K : bdfe write date: %x\n", eax
        call    bdfe_to_fat_date
;        mov     [edi+24], ax            ; last write date
        mov     [edi+12+2], ax            ; last write date
; DEBUGF  1, "K : exFAT write date: %x\n", ax
        ret
;------------------------------------------------------------------------------
exFAT_get_FAT:
; DEBUGF  1, "K : exFAT_get_FAT \n"
; in: eax = cluster
; out: eax = next cluster, CF=1 -> error
        push    ebx esi
;        cmp     [ebp+FAT.fs_type], 12
;        je      .FAT12
;        cmp     [ebp+FAT.fs_type], 16
;        je      @f
;        add     eax, eax
;@@:
;        add     eax, eax
        shl     eax, 2
        mov     esi, 511
        and     esi, eax
        shr     eax, 9
        add     eax, [ebp+exFAT.FAT_START]
        mov     ebx, [ebp+exFAT.fat_cache_ptr]
        cmp     eax, [ebp+exFAT.fat_in_cache]
        je      .inCache
        cmp     [ebp+exFAT.fat_change], 0
        je      @f
;        call    write_fat_sector
@@:
        mov     [ebp+exFAT.fat_in_cache], eax
        call    fs_read32_sys
        test    eax, eax
        jnz     .error
.inCache:
; DEBUGF  1, "K : exFAT_get_FAT.inCache \n"
        mov     eax, [ebx+esi]
        and     eax, [ebp+exFAT.fatMASK]
.ret:
        pop     esi ebx
        ret

.error:
; DEBUGF  1, "K : exFAT_get_FAT.error \n"
        stc
        jmp     .ret
;------------------------------------------------------------------------------
exFAT_hd_find_lfn:
; DEBUGF  1, "K : exFAT_hd_find_lfn path ESI: %s\n", esi
; in: esi -> path string in UTF-8
; out: CF=1 - file not found, eax=error code
;      else CF=0 and edi->direntry, eax=sector
        push    esi edi
        push    0
        push    0
        push    exFAT_notroot_first ; 0 ; fat1x_root_first
        push    exFAT_notroot_next ; 0 ; fat1x_root_next
        xor     eax, eax
        mov     [ebp+exFAT.General_Sec_Flags], eax
        mov     dword [ebp+exFAT.valid_data_length], 0xffffffff ; for ROOT
        mov     eax, [ebp+exFAT.ROOT_CLUSTER]
;        mov     [ebp+exFAT.secondary_dir_entry], dword 1
;        cmp     [ebp+FAT.fs_type], 32
;        jz      .fat32
        jmp     @f ; .fat32
.loop:
        and     [ebp+exFAT.longname_sec1], 0
        and     [ebp+exFAT.longname_sec2], 0

;        push    ebp
;        mov     ebp,[esp+12+8+4+4+7*4]
; DEBUGF  1, "K : exFAT_find_lfn Input FS EBP:%x\n", ebp
;        pop     ebp

        call    exFAT_find_lfn

;        push    ebp
;        mov     ebp,[esp+12+8+4+4+7*4]
; DEBUGF  1, "K : exFAT_find_lfn Output FS EBP:%x\n", ebp
;        pop     ebp

        jc      .notfound
; DEBUGF  1, "K : exFAT_hd_find_lfn [ESI]: %x\n", [esi]
        cmp     byte [esi], 0
        jz      .found
;        test    byte [edi+11], 10h
        lea     eax, [ebp+exFAT.file_dir_entry]
; DEBUGF  1, "K : exFAT_hd_find_lfn exFAT.file_dir_entry [EAX]: %x\n", [eax]
        test    byte [eax+4], 10000b
        jz      .notfound
        and     dword [esp+12], 0
; this entry’s first cluster number
;        mov     eax, [edi+20-2]
;        mov     ax, [edi+26]    ; cluster
        lea     eax, [ebp+exFAT.str_ext_dir_entry]

        push    eax
        movzx   eax, byte [eax+1]
        mov     [ebp+exFAT.General_Sec_Flags], eax
; DEBUGF  1, "K : exFAT General_Sec_Flags %x\n", eax
        mov     eax, [esp]
        mov     eax, [eax+8] ; LOW dword of Valid data length - WARNING!!! late rewrite
        mov     [ebp+exFAT.valid_data_length], eax
; DEBUGF  1, "K : exFAT.valid_data_length 1 %x\n", eax
        pop     eax

        mov     eax, [eax+20]    ; cluster
;.fat32:
@@:
; DEBUGF  1, "K : exFAT_hd_find_lfn exFAT cluster EAX: %x\n", eax
        mov     [esp+8], eax
;        mov     dword [esp+4], exFAT_notroot_first ; fat_notroot_first
;        mov     dword [esp], exFAT_notroot_next ; fat_notroot_next
        jmp     .loop

.notfound:
; DEBUGF  1, "K : exFAT_hd_find_lfn.notfound \n"
        add     esp, 16
        pop     edi esi
        stc
        ret

.found:
; DEBUGF  1, "K : exFAT_hd_find_lfn.found \n"
        lea     eax, [esp+8]
        cmp     dword [eax], 0
        jz      .root
        call    exFAT_get_sector
        jmp     .cmn

.root:
; DEBUGF  1, "K : exFAT_hd_find_lfn.found.root \n"
        mov     eax, [eax+4]
        add     eax, [ebp+exFAT.ROOT_START]
.cmn:
; DEBUGF  1, "K : exFAT_hd_find_lfn.found.cmn \n"
        add     esp, 20         ; CF=0
        pop     esi
        ret
;------------------------------------------------------------------------------
exFAT_find_lfn:
; DEBUGF  1, "K : exFAT_find_lfn \n"
;   in:
; esi -> path in UTF-8
; parameters in the stack
;   out:
; esi -> next name in the path
; edi -> direntry
; CF=1 -> file not found, eax = error code
        xor     eax, eax
        inc     eax
        mov     [ebp+exFAT.secondary_dir_entry], eax ; dword 1
        mov     [ebp+exFAT.need_hash], eax ; dword 1
        lea     eax, [esp+12]
        call    dword [eax-4] ; exFAT_notroot_first
        jc      .reterr
        sub     esp, 262*2      ; reserve place for LFN
;        lea     eax, [esp]
        mov     eax, esp
        mov     [ebp+exFAT.LFN_reserve_place], eax
        mov     [ebp+exFAT.path_in_UTF8], esi
; DEBUGF  1, "K : exFAT_find_lfn Path: %s\n", esi
; DEBUGF  1, "K : exFAT Path: %s\n", esi
; DEBUGF  1, "K : exFAT Path1: %x %x %x\n", [esi], [esi+4], [esi+8]
        push    esi edi
;        lea     edi, [esp+8]
        mov     edi, eax
align 4
@@:
; in: esi -> UTF-8 char (increasing)
; out: ax = UTF-16 char
        call    utf8to16
        call    utf16toUpper
        stosw
        test    ax, ax
        jz      @f
        cmp     ax, word 0x002f ; "/"
        jne     @b
@@:
;        mov     [edi-2], dword 0
        mov     esi, [ebp+exFAT.LFN_reserve_place]
; DEBUGF  1, "K : exFAT Path2: %x %x %x\n", [esi], [esi+4], [esi+8]
        push    ebx ecx
        mov     ecx, edi
        sub     ecx, esi
        sub     ecx, 2 ; correction for zero or "/"
; exFAT_hash_calculate
;   in:
; esi -> NameUTF16
; ecx -> NameUTF16 length
; out: ax = hash
        xor     eax, eax
        xor     ebx, ebx
;--------------------------------------
align 4
.start:
; DEBUGF 1, "Hash start EAX:%x ECX:%x\n", eax, ecx
        mov     bx, ax
; (Hash&1) ? 0x8000 : 0)
        and     ax, 0x1
        jz      .else

        mov     ax, 0x8000
        jmp     @f
;--------------------------------------
.else:
        xor     ax, ax
;--------------------------------------
@@:
; DEBUGF 1, "(Hash&1) EAX:%x\n", eax
; (Hash>>1)
        shr     bx, 1
; DEBUGF 1, "(Hash>>1) EBX:%x\n", ebx
        add     ax, bx
; DEBUGF 1, "+ (Hash>>1)) EAX:%x\n", eax
        movzx   bx, byte [esi]
        add     ax, bx
; DEBUGF 1, "+ (UInt16)Buffer[Index] EAX:%x\n", eax        
        inc     esi
        dec     ecx
        jnz     .start
;--------------------------------------
        pop     ecx ebx
        mov     [ebp+exFAT.current_hash], eax
; DEBUGF  1, "K : exFAT current hash :%x\n", eax        
        pop     edi esi
.l1:
;        push    esi
;        lea     esi, [esp+4]
;        mov     esi, [ebp+exFAT.LFN_reserve_place]
; DEBUGF  1, "K : exFAT_find_lfn.exFAT_get_name \n"

;        push    ebp
;        mov     ebp,[esp+12+8+4+4+7*4+262*2+4]
; DEBUGF  1, "K : exFAT_get_name Input FS EBP:%x\n", ebp
;        pop     ebp
; DEBUGF  1, "K : exFAT FL need_hash :%x\n", [ebp+exFAT.need_hash]
        call    exFAT_get_name
;        mov     [ebp+exFAT.LFN_reserve_place], esi
;        pop     esi

;        push    ebp
;        mov     ebp,[esp+12+8+4+4+7*4+262*2+4]
; DEBUGF  1, "K : exFAT_get_name Output FS EBP:%x\n", ebp
;        pop     ebp

        jc      .no

;        push    eax
        xor     eax, eax
        cmp     [ebp+exFAT.secondary_dir_entry], eax
;        pop     eax
        jnz     .no

        push    edi esi
        mov     esi, [ebp+exFAT.path_in_UTF8]
        lea     edi, [esp+8]
@@:
        call    utf8to16
        call    utf16toUpper
        mov     edx, eax
        mov     ax, [edi]
        call    utf16toUpper
        cmp     ax, dx
        jnz     .done
        add     edi, 2
        test    ax, ax
        jnz     @b
        dec     esi
        pop     eax edi
.found:
; DEBUGF  1, "K : exFAT_find_lfn.found \n"
        add     esp, 262*2
; if this is LFN entry, advance to true entry
;        cmp     byte [edi+11], 0xF
;        jnz     @f
        xor     eax, eax
        cmp     [ebp+exFAT.secondary_dir_entry], eax
        jz      @f
        lea     eax, [esp+12]
        call    dword[eax-8] ; exFAT_notroot_next
        jc      .reterr
@@:
; DEBUGF  1, "K : exFAT_find_lfn.OK \n"
        xor     eax, eax
        ret

.done:
; DEBUGF  1, "K : exFAT_find_lfn.done \n"
        cmp     dx, '/'
        jnz     @f
        test    ax, ax
        jnz     @f
        mov     [esp], esi
@@:
        pop     esi edi
        jz      .found
.no:
; DEBUGF  1, "K : exFAT_find_lfn.no \n"
        lea     eax, [esp+262*2+12]
; DEBUGF  1, "K : exFAT General_Sec_Flags %x\n", [ebp+exFAT.General_Sec_Flags]
; DEBUGF  1, "K : exFAT.valid_data_length 2 %x\n", [ebp+exFAT.valid_data_length]
        cmp     [ebp+exFAT.valid_data_length], 0
        jbe     @f

        call    dword[eax-8] ; exFAT_notroot_next
        jnc     .l1
@@:
        add     esp, 262*2
.reterr:
; DEBUGF  1, "K : exFAT_find_lfn.reterr \n"
        stc
        ret
;------------------------------------------------------------------------------
exFAT_ReadFile:
; DEBUGF  1, "K : exFAT_ReadFile \n"
; DEBUGF  1, "K : exFAT F70 +00: %x\n", [ebx]
; DEBUGF  1, "K : exFAT F70 +04: %x\n", [ebx+4]
; DEBUGF  1, "K : exFAT F70 +08: %x\n", [ebx+8]
; DEBUGF  1, "K : exFAT F70 +12: %x\n", [ebx+12]
; DEBUGF  1, "K : exFAT F70 +16: %x\n", [ebx+16]
; DEBUGF  1, "K : exFAT F70 +20: %x\n", [ebx+20]
; DEBUGF  1, "K : exFAT Path: %s\n", esi
        
;        push    eax
;        pushfd
;        pop     eax
; DEBUGF  1, "K : eFlags:%x\n",eax
;        pop     eax
; DEBUGF  1, "K : EAX:%x EBX:%x ECX:%x EDX:%x\n", eax, ebx, ecx, edx
; DEBUGF  1, "K : EBP:%x ESI:%x EDI:%x\n", ebp, esi, edi
;   in:
; ebx -> parameter structure of sysfunc 70
; ebp -> exFAT structure
; esi -> path string in UTF-8
;   out:
; eax, ebx = return values for sysfunc 70
        call    exFAT_lock
        xor     eax, eax
        mov     [ebp+exFAT.need_hash], eax ; dword 0
        mov     [ebp+exFAT.hash_flag], eax ; dword 0
        call    exFAT_hd_find_lfn
        jc      .notFound
;        test    byte [edi+11], 0x10     ; do not allow read directories
;        jnz     .noaccess
        lea     eax, [ebp+exFAT.file_dir_entry]
        test    byte [eax+4], 10000b  ; do not allow read directories
        jnz     .noaccess
; Rewrite code to work with more than 4 GB files !!!
;        cmp     dword [ebx+8], 0
;        jnz     .endOfFile

        mov     edx, [ebx+8]    ; file offset high
; DEBUGF  1, "K : exFAT_ReadFile Hdword file offset EDX:%x\n", edx
        mov     [ebp+exFAT.RDX_high], edx
        mov     edx, [ebx+4]    ; file offset low
; DEBUGF  1, "K : exFAT_ReadFile Ldword file offset EAX:%x\n", edx

        mov     ecx, [ebx+12]   ; size
        mov     ebx, [ebx+16]   ; buffer
        push    ebx
        push    0
        test    ecx, ecx ; read size 0?
        jz      .done

;        mov     eax, [edi+28] ; real file size
        lea     eax, [ebp+exFAT.str_ext_dir_entry]
; DEBUGF  1, "K : exFAT 0xC0 +00: %x\n", [eax]
; DEBUGF  1, "K : exFAT 0xC0 +04: %x\n", [eax+4]
; DEBUGF  1, "K : exFAT 0xC0 +08: %x\n", [eax+8]
; DEBUGF  1, "K : exFAT 0xC0 +12: %x\n", [eax+12]
; DEBUGF  1, "K : exFAT 0xC0 +16: %x\n", [eax+16]
; DEBUGF  1, "K : exFAT 0xC0 +20: %x\n", [eax+20]
; DEBUGF  1, "K : exFAT 0xC0 +24: %x\n", [eax+24]
; DEBUGF  1, "K : exFAT 0xC0 +28: %x\n", [eax+28]
        push    eax
        movzx   eax, byte [eax+1]
        mov     [ebp+exFAT.General_Sec_Flags], eax
        pop     eax

        push    eax
        mov     eax, [eax+12]    ; high dword of  real file size
        mov     [ebp+exFAT.RAX_high], eax
; DEBUGF  1, "K : exFAT_ReadFile Hdword file size EAX:%x\n", eax
        pop     eax

        mov     eax, [eax+8]    ; low dword of  real file size
; DEBUGF  1, "K : exFAT_ReadFile Ldword file size EAX:%x\n", eax

;        sub     eax, edx ; low dword file size - file offset low = rest of file
;        jb      .fileEnd
        sub     eax, edx ; low dword file size - file offset low = rest of file
        push    eax
        mov     eax, [ebp+exFAT.RDX_high]
        sbb     [ebp+exFAT.RAX_high], eax 
        pop     eax
        jb      .fileEnd
; DEBUGF  1, "K : exFAT_ReadFile Hdword rest of file RAX:%x\n", [ebp+exFAT.RAX_high]
; DEBUGF  1, "K : exFAT_ReadFile Ldword rest of file EAX:%x\n", eax

        push    eax
        mov     eax, [ebp+exFAT.RAX_high]
        test    eax, eax
        pop     eax
        jnz     @f

        cmp     eax, ecx ; rest of file - requested size
        jae     @f

; DEBUGF  1, "K : exFAT_ReadFile 6=EOF EAX:%x ECX:%x EDX:%x\n", eax, ecx, edx
        mov     ecx, eax
        mov     byte [esp], 6 ; 6 = end of file, EOF
@@:
        lea     eax, [ebp+exFAT.str_ext_dir_entry]
        mov     eax, [eax+20]    ; cluster
; DEBUGF  1, "K : exFAT EAX:%x EBX:%x ECX:%x EDX:%x\n", eax, ebx, ecx, edx
; now eax=cluster, ebx=buffer for data, ecx=count, edx=position
        mov     edi, [ebp+exFAT.SECTORS_PER_CLUSTER]
        shl     edi, 9  ; bytes per cluster
@@:
        cmp     eax, 2
        jb      .fileEnd
.continue_1:
        cmp     eax, [ebp+exFAT.fatRESERVED]
        jae     .fileEnd

        push    eax
        xor     eax, eax
        sub     edx, edi ; file_offset_low - bytes per cluster
        sbb     [ebp+exFAT.RDX_high], eax
        pop     eax
        jc      @f

;        push    edi
;        lea     edi, [ebp+exFAT.file_dir_entry]
; Check - General Secondary Flags
; Bit 0 : Allocation possible
;         0 – No cluster allocated; 1 – cluster allocation is possible
; Bit 1 : No FAT chain
;         0 – Yes ; The clusters of this file/directory are NOT contiguous
;         1 – No; The Contiguous Cluster are allocated to this file/directory;
;         This improves the File read performance
; Bits 2 – 7 : Reserved
;        test    byte [edi+1], 11b
;        pop     edi
        test    byte [ebp+exFAT.General_Sec_Flags], 10b
        jz      .get_FAT_1
        inc     eax
        jmp     .continue_1
.get_FAT_1:

        call    exFAT_get_FAT
        jc      .noaccess2

        jmp     @b

.notFound:
; DEBUGF  1, "K : exFAT_ReadFile.notFound: \n"
        push    eax
        jmp     .ret

.noaccess:
; DEBUGF  1, "K : exFAT_ReadFile.noaccess \n"
        push    ERROR_ACCESS_DENIED
        jmp     .ret

.endOfFile:
; DEBUGF  1, "K : exFAT_ReadFile.endOfFile \n"
        push    ERROR_END_OF_FILE
.ret:
        call    exFAT_unlock
        pop     eax
        xor     ebx, ebx
; DEBUGF  1, "K : exFAT_ReadFile Return EBX:%x\n", ebx
        ret

@@:
; DEBUGF  1, "K : exFAT_ReadFile CONTINUE cluster EAX:%x\n", eax
        mov     esi, eax
        dec     eax
        dec     eax

        push    ebx edx
        xor     edx, edx
; DEBUGF  1, "K : exFAT_ReadFile IMUL in EDX:%x EAX:%x\n", EDX, eax
        imul    eax, [ebp+exFAT.SECTORS_PER_CLUSTER] ; Sector edx:eax
; DEBUGF  1, "K : exFAT_ReadFile IMUL out EDX:%x EAX:%x\n", EDX, eax
        xor     ebx, ebx
        add     eax, [ebp+exFAT.CLUSTER_HEAP_START]
        adc     edx, ebx
        mov     [ebp+exFAT.RAX_high], edx
        pop     edx ebx
; DEBUGF  1, "K : exFAT_ReadFile start sector RAX_H:%x EAX:%x RDX_H:%x EDX:%x EDI:%x\n", [ebp+exFAT.RAX_high], eax, [ebp+exFAT.RDX_high], edx, edi
        push    eax
        xor     eax, eax
        add     edx, edi ; file offset low + bytes per cluster
        adc     [ebp+exFAT.RDX_high], eax
        pop     eax

        test    edx, edx
        jz      .alignedCluster

        mov     edi, edx ; file offset low
        push    eax
        mov     eax, [ebp+exFAT.RDX_high]
        mov     [ebp+exFAT.RDI_high], eax
        pop     eax

;        shr     edi, 9
        push    ebx
        mov     ebx, [ebp+exFAT.RDI_high]
        shrd    edi, ebx, 5 ; /32
        shr     ebx, 5 ; /32
        shrd    edi, ebx, 4 ; /16
        shr     ebx, 4 ; /16
        mov     [ebp+exFAT.RDI_high], ebx
        pop     ebx

        add     eax, edi ; RFile_start_sector_low - file_sector_offset_low
        push    ebx
        mov     ebx, [ebp+exFAT.RDI_high]
        adc     [ebp+exFAT.RAX_high], ebx
        pop     ebx

        and     edx, 511
        and     dword [ebp+exFAT.RDX_high], 0

        cmp     ecx, 512
        jc      .sectorPiece
        test    edx, edx
        jz      .alignedSector
.sectorPiece:
; DEBUGF  1, "K : exFAT_ReadFile.sectorPiece \n"
        push    eax ebx ecx edx
        lea     ebx, [ebp+exFAT.buffer]
        mov     edx, [ebp+exFAT.RAX_high]
        xor     ecx, ecx
        inc     ecx
; DEBUGF  1, "K : exFAT fs_read64_app EDX:%x EAX:%x ECX:%x EBX:%x EBP:%x\n", edx, eax, ecx, ebx, ebp
;        call    fs_read32_app
        call    fs_read64_app
; DEBUGF  1, "K : exFAT fs_read64_app Output EAX:%x ECX:%x EBX:%x\n", eax, ecx, ebx
        test    eax, eax
;        lea     eax, [ebp+exFAT.buffer]
        mov     eax, ebx ; exFAT.buffer
        pop     edx ecx ebx
        jne     .noaccess3
; DEBUGF  1, "K : exFAT_ReadFile memmove(-1) EAX:%x EBX:%x ECX:%x EDX:%x\n", eax, ebx, ecx, edx
        add     eax, edx ; exFAT.buffer + offset within a sector
        push    ecx
        add     ecx, edx ; requested size + offset within a sector
        cmp     ecx, 512
        jbe     @f
        mov     ecx, 512
@@:
        sub     ecx, edx ; requested size - offset within a sector
; DEBUGF  1, "K : exFAT_ReadFile memmove EAX:%x EBX:%x ECX:%x\n", eax, ebx, ecx
; eax = from
; ebx = to
; ecx = no of bytes
        call    memmove
; DEBUGF  1, "K : exFAT_ReadFile memmove(1) EAX:%x EBX:%x ECX:%x\n", eax, ebx, ecx
        sub     [esp], ecx
        add     ebx, ecx
; DEBUGF  1, "K : exFAT_ReadFile memmove(2) EAX:%x EBX:%x ECX:%x\n", eax, ebx, ecx
        pop     ecx eax
        xor     edx, edx
;        inc     edi ; file_sector_offset_low
        push    eax
        xor     eax, eax
        add     edi, 1 ; you cannot use INC EDI for qword!!!
        adc     [ebp+exFAT.RDI_high], eax
        pop     eax
;        inc     eax ; RFile_start_sector_low
        push    ebx
        xor     ebx, ebx
        add     eax, 1 ; you cannot use INC EAX for qword!!!
        adc     [ebp+exFAT.RAX_high], ebx
        pop     ebx
        test    ecx, ecx
        jz      .done
.alignedSector:
; DEBUGF  1, "K : exFAT_ReadFile.alignedSector \n"
;        shl     edi, 9 ; RFile_start_sector_low * 512
        push    ebx
        mov     ebx, [ebp+exFAT.RDI_high]
        shld    ebx, edi, 5 ; *32
        shl     edi, 5 ; *32
        shld    ebx, edi, 4 ; *16
        shl     edi, 4 ; *16
        mov     [ebp+exFAT.RDI_high], ebx
        pop     ebx

        push    ebx
        xor     ebx, ebx
        mov     [ebp+exFAT.RCX_high], ebx
        add     ecx, edi ; requested size  + file_offset_low
        mov     ebx, [ebp+exFAT.RDI_high]
        adc     [ebp+exFAT.RCX_high], ebx
        pop     ebx

        xor     edi, edi
        mov     [ebp+exFAT.RDI_high], edi
        mov     edi, [ebp+exFAT.SECTORS_PER_CLUSTER]
        shl     edi, 9  ; bytes per cluster
.alignedCluster:
; DEBUGF  1, "K : exFAT_ReadFile.alignedCluster RAX_H:%x EAX:%x RDX_H:%x EDX:%x EDI:%x\n", [ebp+exFAT.RAX_high], eax, [ebp+exFAT.RDX_high], edx, edi
        cmp     ecx, 512
        jc      .sectorPiece
        mov     edx, [ebp+exFAT.RAX_high]
        mov     [ebp+exFAT.RDX_high], edx
        mov     edx, eax ; edx << RFile_start_sector_low

        xor     eax, eax
        mov     [ebp+exFAT.RAX_high], eax
        mov     eax, esi ; eax << cluster
@@:
        push    eax
        xor     eax, eax
        sub     ecx, edi ; requested size low - bytes per cluster
        sbb     [ebp+exFAT.RCX_high], eax
        pop     eax
        jbe     .readEnd
        
;        push    edi
;        lea     edi, [ebp+exFAT.file_dir_entry]
; Check - General Secondary Flags
; Bit 0 : Allocation possible
;         0 – No cluster allocated; 1 – cluster allocation is possible
; Bit 1 : No FAT chain
;         0 – Yes ; The clusters of this file/directory are NOT contiguous
;         1 – No; The Contiguous Cluster are allocated to this file/directory;
;         This improves the File read performance
; Bits 2 – 7 : Reserved
;        test    byte [edi+1], 11b
;        pop     edi
        test    byte [ebp+exFAT.General_Sec_Flags], 10b
        jz      .get_FAT
        inc     eax ; inc cluster
        jmp     .continue
.get_FAT:
; DEBUGF  1, "K : exFAT_ReadFile.get_FAT \n"
        call    exFAT_get_FAT
        jc      .noaccess4
        cmp     eax, 2
        jb      .fileEnd2
.continue:
; DEBUGF  1, "K : exFAT_ReadFile.continue \n"
        cmp     eax, [ebp+exFAT.fatRESERVED]
        jae     .fileEnd2

        inc     esi ; inc cluster
        cmp     eax, esi
        jz      @b
.fragmentEnd:
; DEBUGF  1, "K : exFAT_ReadFile.fragmentEnd \n"
        xchg    eax, esi
        dec     eax
        dec     eax

        push    ebx edx
        xor     edx, edx
        imul    eax, [ebp+exFAT.SECTORS_PER_CLUSTER] ; Sector edx:eax
        xor     ebx, ebx
        add     eax, [ebp+exFAT.CLUSTER_HEAP_START]
        adc     edx, ebx
        mov     [ebp+exFAT.RAX_high], edx
        pop     edx ebx
        
        push    dword [ebp+exFAT.RCX_high]
        push    ecx ; requested size low
        
        mov     ecx, [ebp+exFAT.RAX_high]
        mov     [ebp+exFAT.RCX_high], ecx
        mov     ecx, eax ; ecx << RFile_start_sector_low
        
        xor     eax, eax
        mov     [ebp+exFAT.RAX_high], eax
        mov     eax, esi ; eax << custer

        dec     eax
        dec     eax

        push    ebx edx
        xor     edx, edx
        imul    eax, [ebp+exFAT.SECTORS_PER_CLUSTER] ; Sector edx:eax
        xor     ebx, ebx
        add     eax, [ebp+exFAT.CLUSTER_HEAP_START]
        adc     edx, ebx
        mov     [ebp+exFAT.RAX_high], edx
        pop     edx ebx

        push    dword [ebp+exFAT.RAX_high]
        push    eax
.readFragment:
; DEBUGF  1, "K : exFAT_ReadFile.readFragment \n"
        push    eax
        mov     eax, [ebp+exFAT.RDX_high]
        sub     ecx, edx ; RFile_start_sector_low - RFile_start_sector_low
        sbb     [ebp+exFAT.RCX_high], eax
        pop     eax

        mov     eax, edx
;        xor     edx, edx
        mov     edx, [ebp+exFAT.RDX_high]

; DEBUGF  1, "K : exFAT fs_read64_app EDX:%x EAX:%x ECX:%x EBX:%x EBP:%x\n", edx, eax, ecx, ebx, ebp
        call    fs_read64_app
; DEBUGF  1, "K : exFAT fs_read64_app Output EAX:%x ECX:%x EBX:%x\n", eax, ecx, ebx
;        shl     ecx, 9
        push    ebx
        mov     ebx, [ebp+exFAT.RCX_high]
        shld    ebx, ecx, 5 ; *32
        shl     ecx, 5 ; *32
        shld    ebx, ecx, 4 ; *16
        shl     ecx, 4 ; *16
        mov     [ebp+exFAT.RCX_high], ebx
        pop     ebx

        add     ebx, ecx

        test    eax, eax
        pop     eax
        pop     dword [ebp+exFAT.RAX_high]
        jnz     .noaccess3
        pop     ecx
        pop     dword [ebp+exFAT.RCX_high]
        xor     edx, edx
        mov     [ebp+exFAT.RDX_high], edx
        jecxz   .done_1
        jmp     .alignedCluster
.done_1:
        jmp     .done

.readEnd:
; DEBUGF  1, "K : exFAT_ReadFile.readEnd \n"
        push    ebx
        add     ecx, edi ; requested size  + bytes per cluster
        mov     ebx, [ebp+exFAT.RDI_high]
        adc     [ebp+exFAT.RCX_high], ebx
        pop     ebx

        mov     edi, [ebp+exFAT.RCX_high]
        mov     [ebp+exFAT.RDI_high], edi
        mov     edi, ecx

        and     ecx, 511
        and     dword [ebp+exFAT.RCX_high], 0
;        shr     edi, 9
        push    ebx
        mov     ebx, [ebp+exFAT.RDI_high]
        shrd    edi, ebx, 5 ; /32
        shr     ebx, 5 ; /32
        shrd    edi, ebx, 4 ; /16
        shr     ebx, 4 ; /16
        mov     [ebp+exFAT.RDI_high], ebx
        pop     ebx

        dec     eax
        dec     eax

        push    ebx edx
        xor     edx, edx
        imul    eax, [ebp+exFAT.SECTORS_PER_CLUSTER] ; Sector edx:eax
        xor     ebx, ebx
        add     eax, [ebp+exFAT.CLUSTER_HEAP_START]
        adc     edx, ebx
        mov     [ebp+exFAT.RAX_high], edx
        pop     edx ebx

        add     eax, edi ; RFile_start_sector_low - file_sector_offset_low
        push    ebx
        mov     ebx, [ebp+exFAT.RDI_high]
        adc     [ebp+exFAT.RAX_high], ebx
        pop     ebx

        push    dword [ebp+exFAT.RCX_high]
        push    ecx

        push    dword [ebp+exFAT.RAX_high]
        push    eax

        mov     ecx, [ebp+exFAT.RAX_high]
        mov     [ebp+exFAT.RCX_high], ecx
        mov     ecx, eax
        jmp     .readFragment

.noaccess3:
; DEBUGF  1, "K : exFAT_ReadFile.noaccess3 \n"
        pop     eax
        pop     dword [ebp+exFAT.RAX_high]
.noaccess2:
; DEBUGF  1, "K : exFAT_ReadFile.noaccess2 \n"
        mov     byte [esp], ERROR_DEVICE
.done:
; DEBUGF  1, "K : exFAT_ReadFile.done \n"
        call    exFAT_unlock
        pop     eax edx
        sub     ebx, edx
; DEBUGF  1, "K : exFAT_ReadFile Return EBX:%x\n", ebx
           
;        push    eax
;        pushfd
;        pop     eax
; DEBUGF  1, "K : eFlags:%x\n",eax
;        pop     eax
; DEBUGF  1, "K : EAX:%x EBX:%x ECX:%x EDX:%x\n", eax, ebx, ecx, edx
; DEBUGF  1, "K : EBP:%x ESI:%x EDI:%x\n", ebp, esi, edi
        ret

.fileEnd:
; DEBUGF  1, "K : exFAT_ReadFile.fileEnd \n"
        mov     byte [esp], ERROR_END_OF_FILE
        jmp     .done

.noaccess4:
; DEBUGF  1, "K : exFAT_ReadFile.noaccess4 \n"
        mov     byte [esp], ERROR_DEVICE
        jmp     @f

.fileEnd2:
; DEBUGF  1, "K : exFAT_ReadFile.fileEnd2 \n"
        mov     byte [esp], ERROR_END_OF_FILE
@@:
        inc     esi
        xor     ecx, ecx
        mov     [ebp+exFAT.RCX_high], ecx
        jmp     .fragmentEnd
;------------------------------------------------------------------------------
exFAT_ReadFolder:
; DEBUGF  1, "K : exFAT_ReadFolder \n"
; DEBUGF  1, "K : exFAT F70 +00: %x\n", [ebx]
; DEBUGF  1, "K : exFAT F70 +04: %x\n", [ebx+4]
; DEBUGF  1, "K : exFAT F70 +08: %x\n", [ebx+8]
; DEBUGF  1, "K : exFAT F70 +12: %x\n", [ebx+12]
; DEBUGF  1, "K : exFAT F70 +16: %x\n", [ebx+16]
; DEBUGF  1, "K : exFAT F70 +20: %x\n", [ebx+20]
; DEBUGF  1, "K : exFAT Path: %s\n", esi
;        push    eax
;        pushfd
;        pop     eax
; DEBUGF  1, "K : eFlags:%x\n",eax
;        pop     eax
; DEBUGF  1, "K : EAX:%x EBX:%x ECX:%x EDX:%x\n", eax, ebx, ecx, edx
; DEBUGF  1, "K : EBP:%x ESI:%x EDI:%x\n", ebp, esi, edi
;   in:
; ebx -> parameter structure of sysfunc 70
; ebp -> exFAT structure
; esi -> path string in UTF-8
;   out:
; eax, ebx = return values for sysfunc 70
        call    exFAT_lock
        xor     eax, eax
        mov     [ebp+exFAT.need_hash], eax ; dword 0
        mov     [ebp+exFAT.hash_flag], eax ; dword 0
        mov     [ebp+exFAT.General_Sec_Flags], eax
; DEBUGF  1, "K : exFAT_ReadFolder General_Sec_Flags 1 %x\n", eax
        mov     eax, [ebp+exFAT.ROOT_CLUSTER]
; DEBUGF  1, "K : exFAT.ROOT_CLUSTER: %x\n", eax
        cmp     byte [esi], 0
        jz      .doit
        
;        push    ebp
;        mov     ebp,[esp+12+8+4+4]
; DEBUGF  1, "K : exFAT Input FS EBP:%x\n", ebp
;        pop     ebp

        call    exFAT_hd_find_lfn

;        push    ebp
;        mov     ebp,[esp+12+8+4+4]
; DEBUGF  1, "K : exFAT Output FS EBP:%x\n", ebp
;        pop     ebp

        jc      .error
;        jmp     .error
;        test    byte [edi+11], 0x10     ; do not allow read files
;        jz      .accessDenied
        xor     eax, eax
        mov     [ebp+exFAT.need_hash], eax ; dword 0
        mov     [ebp+exFAT.hash_flag], eax ; dword 0
        lea     eax, [ebp+exFAT.file_dir_entry]
        test    byte [eax+4], 10000b  ; do not allow read files
        jz      .accessDenied
;        mov     eax, [edi+20-2]
;        mov     ax, [edi+26]    ; eax=cluster
        lea     eax, [ebp+exFAT.str_ext_dir_entry]
        push    eax
        movzx   eax, byte [eax+1]
        mov     [ebp+exFAT.General_Sec_Flags], eax
; DEBUGF  1, "K : exFAT_ReadFolder General_Sec_Flags 2 %x\n", eax
        mov     eax, [esp]
        mov     eax, [eax+8] ; LOW dword of Valid data length - WARNING!!! late rewrite
        mov     [ebp+exFAT.valid_data_length], eax
        pop     eax
        mov     eax, [eax+20]    ; cluster
        jmp     .doit_1
.doit:
        mov     dword [ebp+exFAT.valid_data_length], 0xffffffff ; for ROOT
.doit_1:
; DEBUGF  1, "K : exFAT.valid_data_length %x\n", [ebp+exFAT.valid_data_length]
; DEBUGF  1, "K : exFAT_ReadFolder.doit \n"
        sub     esp, 262*2      ; reserve space for LFN
        push    dword [ebx+8]   ; cp866/UNICODE name
        mov     edx, [ebx+16]   ; pointer to buffer
; init header
        push    eax
        mov     edi, edx
        mov     ecx, 32/4
        xor     eax, eax
        rep stosd
        pop     eax
        mov     byte [edx], 1   ; version
;        mov     esi, edi        ; esi points to BDFE
        mov     [ebp+exFAT.points_to_BDFE], edi
; DEBUGF  1, "K : exFAT.points_to_BDFE start EDI: %x\n", edi
        mov     ecx, [ebx+12]   ; number of blocks to read
        mov     ebx, [ebx+4]    ; index of the first block
;------------------------------------------------------------------------------
; DEBUGF  1, "K : exFAT_ReadFolder 1 ECX: %x\n", ecx
        cmp     [ebp+exFAT.valid_data_length], 0xffffffff
        je      .num_read_blocks
        inc     dword [edx+8]   ; new file found
        test    ecx, ecx
        jz      .num_read_blocks
        test    ebx, ebx
        jnz     .dec_offset
; DEBUGF  1, "K : exFAT_ReadFolder create .. dir \n"
        inc     dword [edx+4]   ; new file block copied
        push    eax esi
        mov     esi, edi ; [ebp+exFAT.points_to_BDFE]
        mov     [esi], dword 0x10      ; attributes
        xor     eax, eax
        mov     [esi+8], eax    ; creation time
        mov     [esi+12], dword 0x010101 ; eax   ; creation date
        mov     [esi+16], eax    ; last access time
        mov     [esi+20], dword 0x020202 ;eax   ; last access date
        mov     [esi+24], eax   ; last write time
        mov     [esi+28], dword 0x010303 ; eax   ; last write date
        mov     [esi+32], eax   ; file size (low dword)
        mov     [esi+36], eax   ; file size (high dword)
        push    ebp
        lea     ebp, [esp+4+12]
; DEBUGF  1, "K : exFAT_ReadFolder 1 ESI: %x EBP: %x\n", esi, ebp
; DEBUGF  1, "K : exFAT_ReadFolder old file [EBP-4]: %x [EBP]: %x [EBP+4]: %x [EBP+8]: %x\n", [ebp-4], [ebp], [ebp+4], [ebp+8]
        mov     eax, [ebp-4]
        mov     [esi+4], eax    ; cp866/UNICODE name
        mov     [ebp], dword 0x002e002e ; imitate dir '..'
        xor     eax, eax
        mov     [ebp+4], eax
        call    exFAT_entry_to_bdfe2.copy_path
        pop     ebp
        mov     [ebp+exFAT.points_to_BDFE], esi
; DEBUGF  1, "K : exFAT_ReadFolder 2 ESI: %x EBP: %x\n", esi, ebp
        pop     esi eax
        dec     ecx
        jmp     .num_read_blocks
.dec_offset:
        dec     ebx
.num_read_blocks:
; DEBUGF  1, "K : exFAT_ReadFolder 2 ECX: %x\n", ecx
;------------------------------------------------------------------------------
        lea     esi, [esp+4]    ; buffer for UTF-16 name (space for LFN)
        mov     [ebp+exFAT.LFN_reserve_place], esi
;        push    eax
;        dec     eax
;        dec     eax
;        imul    eax, [ebp+exFAT.SECTORS_PER_CLUSTER]
;        add     eax, [ebp+exFAT.CLUSTER_HEAP_START]
; DEBUGF  1, "K : exFAT ROOT SECTOR: %x\n", eax
;        pop     eax
        mov     [ebp+exFAT.secondary_dir_entry], dword 1
.new_cluster:
; DEBUGF  1, "K : exFAT_ReadFolder.new_cluster \n"
        mov     [ebp+exFAT.cluster_tmp], eax
        test    eax, eax
;        jz      .notfound
        jnz     @f
        jmp     .notfound
@@:
        dec     eax
        dec     eax
        imul    eax, [ebp+exFAT.SECTORS_PER_CLUSTER]
        push    [ebp+exFAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+exFAT.CLUSTER_HEAP_START]
        push    ebx
.new_sector:
; DEBUGF  1, "K : exFAT_ReadFolder.new_sector \n"
        lea     ebx, [ebp+exFAT.buffer]
        mov     edi, ebx
        push    eax
; DEBUGF  1, "K : exFAT fs_read32_sys N1 EAX: %x\n", eax
        call    fs_read32_sys
        test    eax, eax
        pop     eax
        jnz     .notfound2
        add     ebx, 512
        push    eax
.l1:
; DEBUGF  1, "K : exFAT_ReadFolder.l1 \n"
;        push    esi
;        lea     esi, [esp+20]
; DEBUGF  1, "K : exFAT RD need_hash :%x\n", [ebp+exFAT.need_hash]
        call    exFAT_get_name
;        pop     esi
        jc      .l2
;        cmp     byte [edi], 0xC1 ; File Name Extension Directory Entry of ExFAT
;        jnz     .do_bdfe
; DEBUGF  1, "K : exFAT_ReadFolder CMP SDE\n"
        xor     eax, eax
        cmp     [ebp+exFAT.secondary_dir_entry], eax
        jz      .do_bdfe
;        add     edi, 0x20
; DEBUGF  1, "K : exFAT_ReadFolder.do_bdfe EDI: %x  EBX: %x\n", edi, ebx
; DEBUGF  1, "K : exFAT_ReadFolder.do_bdfe EDI:%x [EDI]:%x NAME:%s\n", edi, [edi], edi
        cmp     edi, ebx
        jb      .do_bdfe
; DEBUGF  1, "K : exFAT_ReadFolder.do_bdfe EDI after\n", edi, ebx
        pop     eax
        inc     eax
        dec     dword [esp+4]
        jnz     @f
        mov     eax, [ebp+exFAT.cluster_tmp]
        test    eax, eax
        jz      .done
; Check - General Secondary Flags
; Bit 0 : Allocation possible
;         0 – No cluster allocated; 1 – cluster allocation is possible
; Bit 1 : No FAT chain
;         0 – Yes ; The clusters of this file/directory are NOT contiguous
;         1 – No; The Contiguous Cluster are allocated to this file/directory;
;         This improves the File read performance
; Bits 2 – 7 : Reserved
        test    byte [ebp+exFAT.General_Sec_Flags], 10b
        jz      .get_FAT_1
        inc     eax
        jmp     .continue_1
.get_FAT_1:
; DEBUGF  1, "K : exFAT_ReadFolder N1 exFAT_get_FAT Input EAX: %x\n", eax
        call    exFAT_get_FAT
; DEBUGF  1, "K : exFAT_ReadFolder N1 exFAT_get_FAT Output EAX: %x\n", eax
        jc      .notfound2
        cmp     eax, 2
        jb      .done
.continue_1:
; DEBUGF  1, "K : exFAT_ReadFolder.continue_1\n"
        cmp     eax, [ebp+exFAT.fatRESERVED]
        jae     .done
        push    eax
        mov     eax, [ebp+exFAT.SECTORS_PER_CLUSTER]
        mov     [esp+8], eax
        pop     eax
        mov     [ebp+exFAT.cluster_tmp], eax
        dec     eax
        dec     eax
        imul    eax, [ebp+exFAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+exFAT.DATA_START]
@@:
; DEBUGF  1, "K : exFAT_ReadFolder.@@ \n"
        lea     ebx, [ebp+exFAT.buffer]
        mov     edi, ebx
        push    eax
; DEBUGF  1, "K : exFAT fs_read32_sys N2 EAX: %x\n", eax
        call    fs_read32_sys
        test    eax, eax
        pop     eax
        jnz     .notfound2
        add     ebx, 512
        push    eax
.do_bdfe:
; DEBUGF  1, "K : exFAT_ReadFolder.do_bdfe \n"
        inc     dword [edx+8]   ; new file found
        dec     dword [esp+4]
        jns     .l2
; DEBUGF  1, "K : exFAT_ReadFolder.do_bdfe ECX: %x\n", ecx
        dec     ecx
        js      .l2
; DEBUGF  1, "K : exFAT_ReadFolder.do_bdfe 2 \n"
        inc     dword [edx+4]   ; new file block copied
        push    esi edi
        mov     esi, [ebp+exFAT.points_to_BDFE]
        lea     edi, [ebp+exFAT.file_dir_entry]
        push    ebp
        lea     ebp, [esp+20+4+4]
; DEBUGF  1, "K : exFAT_ReadFolder ESI: %x EBP: %x\n", esi, ebp
; DEBUGF  1, "K : exFAT_ReadFolder.do_bdfe EDI:%x [EDI]:%x ESI:%x [ESI]:%x EBP:%x NAME:%s\n",  edi, [edi], esi, [esi], ebp, ebp
        call    exFAT_entry_to_bdfe
        pop     ebp
        mov     [ebp+exFAT.points_to_BDFE], esi
        pop     edi esi
.l2:
; DEBUGF  1, "K : exFAT_ReadFolder.l2 \n"
        add     edi, 0x20
; DEBUGF  1, "K : exFAT_ReadFolder.do_bdfe EDI: %x  EBX: %x\n", edi, ebx
        cmp     edi, ebx
        jb      .l1
        pop     eax
        inc     eax
        dec     dword [esp+4]
        jnz     .new_sector
        mov     eax, [ebp+exFAT.cluster_tmp]
        test    eax, eax
        jz      .done

        push    eax
        mov     eax, [ebp+exFAT.SECTORS_PER_CLUSTER]
        shl     eax, 9
        sub     [ebp+exFAT.valid_data_length], eax
        pop     eax
        jbe     .done
; Check - General Secondary Flags
; Bit 0 : Allocation possible
;         0 – No cluster allocated; 1 – cluster allocation is possible
; Bit 1 : No FAT chain
;         0 – Yes ; The clusters of this file/directory are NOT contiguous
;         1 – No; The Contiguous Cluster are allocated to this file/directory;
;         This improves the File read performance
; Bits 2 – 7 : Reserved
        test    byte [ebp+exFAT.General_Sec_Flags], 10b
        jz      .get_FAT
        inc     eax
        jmp     .continue
.get_FAT:
; DEBUGF  1, "K : exFAT_ReadFolder N2 exFAT_get_FAT Input EAX: %x\n", eax
        call    exFAT_get_FAT
; DEBUGF  1, "K : exFAT_ReadFolder N2 exFAT_get_FAT Output EAX: %x\n", eax
        jc      .notfound2
        cmp     eax, 2
        jb      .done
.continue:
; DEBUGF  1, "K : exFAT_ReadFolder.continue \n"
        cmp     eax, [ebp+exFAT.fatRESERVED]
        jae     .done
; DEBUGF  1, "K : exFAT_ReadFolder.continue after\n"
        push    eax
        mov     eax, [ebp+exFAT.SECTORS_PER_CLUSTER]
        mov     [esp+8], eax
        pop     eax
        pop     ebx
        add     esp, 4
        jmp     .new_cluster
;------------------------------------------------------------------------------
.notfound2:
; DEBUGF  1, "K : exFAT_ReadFolder.notfound2 \n"
        add     esp, 8
.notfound:
; DEBUGF  1, "K : exFAT_ReadFolder.notfound \n"
        add     esp, 262*2+4
; DEBUGF  1, "K : exFAT_ReadFolder.ERROR_DEVICE \n"
        push    ERROR_DEVICE
        jmp     @f
;------------------------------------------------------------------------------
.done:
; DEBUGF  1, "K : exFAT_ReadFolder.done \n"
; DEBUGF  1, "K : exFAT_ReadFolder TotalBloks: %x\n", [edx+8]
; DEBUGF  1, "K : exFAT_ReadFolder Read Bloks: %x\n", [edx+4]
        add     esp, 262*2+12
        pushd   0
; DEBUGF  1, "K : exFAT_ReadFolder.done ECX: %x\n", ecx
        dec     ecx
        js      @f
; DEBUGF  1, "K : exFAT_ReadFolder.ERROR_END_OF_FILE \n"
        mov     byte [esp], ERROR_END_OF_FILE
@@:
        mov     ebx, [edx+4]
;------------------------------------------------------------------------------
.ret:
; DEBUGF  1, "K : exFAT_ReadFolder.ret \n"
; DEBUGF  1, "K : exFAT_ReadFile Return ESI:%x\n", esi
        call    exFAT_unlock
        pop     eax
; DEBUGF  1, "K : exFAT_ReadFile Return EBX:%x\n", ebx
;        mov     esi, [ebp+exFAT.LFN_reserve_place]
;        push    eax
;        pushfd
;        pop     eax
; DEBUGF  1, "K : eFlags:%x\n",eax
;        pop     eax
; DEBUGF  1, "K : EAX:%x EBX:%x ECX:%x EDX:%x\n", eax, ebx, ecx, edx
; DEBUGF  1, "K : EBP:%x ESI:%x EDI:%x\n", ebp, esi, edi
        ret
;------------------------------------------------------------------------------
.error:
; DEBUGF  1, "K : exFAT_ReadFolder.error \n"
        push    eax
        xor     ebx, ebx
        jmp     .ret
;------------------------------------------------------------------------------
.accessDenied:
; DEBUGF  1, "K : exFAT_ReadFolder.ERROR_ACCESS_DENIED \n"
        push    ERROR_ACCESS_DENIED
        xor     ebx, ebx
        jmp     .ret
;------------------------------------------------------------------------------
exFAT_GetFileInfo:
; DEBUGF  1, "K : exFAT_GetFileInfo \n"
; DEBUGF  1, "K : exFAT F70 +00: %x\n", [ebx]
; DEBUGF  1, "K : exFAT F70 +04: %x\n", [ebx+4]
; DEBUGF  1, "K : exFAT F70 +08: %x\n", [ebx+8]
; DEBUGF  1, "K : exFAT F70 +12: %x\n", [ebx+12]
; DEBUGF  1, "K : exFAT F70 +16: %x\n", [ebx+16]
; DEBUGF  1, "K : exFAT F70 +20: %x\n", [ebx+20]
; DEBUGF  1, "K : exFAT Path: %s\n", esi
        cmp     byte [esi], 0
        jz      .volume
        call    exFAT_lock
        xor     eax, eax
        mov     [ebp+exFAT.need_hash], eax ; dword 0
        mov     [ebp+exFAT.hash_flag], eax ; dword 0
        call    exFAT_hd_find_lfn
        jc      @f
        lea     edi, [ebp+exFAT.file_dir_entry]
        push    ebp
        xor     ebp, ebp
        mov     esi, [ebx+16]
        mov     dword [esi+4], ebp
        call    exFAT_entry_to_bdfe2
        pop     ebp
        xor     eax, eax
@@:
        push    eax
        call    exFAT_unlock
        pop     eax
@@:
        ret

.volume:
; DEBUGF  1, "K : exFAT_GetFileInfo.volume \n"
        mov     eax, dword[ebp+exFAT.Length]
        mov     edx, dword[ebp+exFAT.Length+4]
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
        lea     esi, [ebp+exFAT.volumeLabel]
        mov     ecx, 11
@@:
        mov     byte [esi+ecx], 0
        dec     ecx
        jz      @f
        cmp     byte [esi+ecx], ' '
        jz      @b
@@:
        mov     cl, 12
        add     edi, 40
        cmp     eax, 2
        jz      @f
        rep movsb
        xor     eax, eax
        ret

@@:
        lodsb
        stosw
        loop    @b
        ret
;------------------------------------------------------------------------------
exFAT_SetFileInfo:
; DEBUGF  1, "K : exFAT_SetFileInfo \n"
; DEBUGF  1, "K : exFAT F70 +00: %x\n", [ebx]
; DEBUGF  1, "K : exFAT F70 +04: %x\n", [ebx+4]
; DEBUGF  1, "K : exFAT F70 +08: %x\n", [ebx+8]
; DEBUGF  1, "K : exFAT F70 +12: %x\n", [ebx+12]
; DEBUGF  1, "K : exFAT F70 +16: %x\n", [ebx+16]
; DEBUGF  1, "K : exFAT F70 +20: %x\n", [ebx+20]
; DEBUGF  1, "K : exFAT Path: %s\n", esi
        call    exFAT_lock
        xor     eax, eax
        mov     [ebp+exFAT.need_hash], eax ; dword 0
        mov     [ebp+exFAT.hash_flag], eax ; dword 0
        call    exFAT_hd_find_lfn
        jc      @f

        mov     edi, [ebp+exFAT.buff_file_dir_pos]
        cmp     eax, [ebp+exFAT.buff_file_dirsect]
        je      .continue

        mov     eax, [ebp+exFAT.buff_file_dirsect]
        mov     [ebp+exFAT.buffer_curr_sector], eax

        push    eax ebx
        lea     ebx, [ebp+exFAT.buffer]
        call    fs_read32_sys
        test    eax, eax
        jz      .continue_1 ; CF=0

        pop     ebx
        add     esp, 4
        jmp     @f

.continue_1:
        pop     ebx eax

.continue:
        push    eax
        mov     edx, [ebx+16] ; pointer to buffer with attributes (32 bytes)
        call    exFAT_bdfe_to_fat_entry
        pop     eax
; copy new File/Folder Directory Entry [0x85] for calculate SetChecksum field
        push    esi edi
        xchg    esi, edi
        lea     edi, [ebp+exFAT.file_dir_entry]
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        pop     edi esi

        push    eax
        call    calculate_SetChecksum_field
        mov     [edi+2], ax
        pop     eax
        
        lea     ebx, [ebp+exFAT.buffer]
        call    fs_write32_sys
        call    exFAT_update_disk
        xor     eax, eax
@@:
        push    eax
        call    exFAT_unlock
        pop     eax
        ret
;------------------------------------------------------------------------------
calculate_SetChecksum_field:
        push    ebx ecx edx esi edi
        lea     esi, [ebp+exFAT.file_dir_entry]
        mov     ecx, [ebp+exFAT.fname_extdir_offset]
        sub     ecx, esi
        mov     edx, esi
        mov     edi, esi
        add     edx, 2 ; (Index == 2)
        add     edi, 3 ; (Index == 3)
; exFAT_calculate_SetChecksum_field
;   in:
; esi -> file_dir_entry
; ecx -> NumberOfBytes
; out: ax = Checksum
        xor     eax, eax
        xor     ebx, ebx
;--------------------------------------
align 4
.start:
; DEBUGF 1, "Checksum start EAX:%x ECX:%x\n", eax, ecx
        cmp     esi, edx ; (Index == 2)
        je      .continue
        cmp     esi, edi ; (Index == 3)
        je      .continue
        mov     bx, ax
; ((Checksum&1) ? 0x8000 : 0)
        and     ax, 0x1
        jz      .else

        mov     ax, 0x8000
        jmp     @f
;--------------------------------------
.else:
        xor     ax, ax
;--------------------------------------
@@:
; DEBUGF 1, "(Checksum&1) EAX:%x\n", eax
; (Hash>>1)
        shr     bx, 1
; DEBUGF 1, "(Checksum>>1) EBX:%x\n", ebx
        add     ax, bx
; DEBUGF 1, "+ (Checksum>>1) EAX:%x\n", eax
        movzx   bx, byte [esi]
        add     ax, bx
; DEBUGF 1, "+ (UInt16)Entries[Index] EAX:%x\n", eax
.continue:       
        inc     esi
        dec     ecx
        jnz     .start
;--------------------------------------
        lea     ebx, [ebp+exFAT.file_dir_entry+2]
        mov     [ebx], ax
        pop     edi esi edx ecx ebx
        ret
;------------------------------------------------------------------------------
exFAT_update_disk:
        cmp     [ebp+exFAT.fat_change], 0
        jz      .noChange
;        cmp     [ebp+FAT.fs_type], 12
;        jz      .fat12
        call    exFAT_write_fat_sector
.noChange:
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
        ret
;------------------------------------------------------------------------------
exFAT_write_fat_sector:
        push    eax ebx ecx
        mov     [ebp+exFAT.fat_change], 0
        mov     eax, [ebp+exFAT.fat_in_cache]
        cmp     eax, -1
        jz      @f
        mov     ebx, [ebp+exFAT.fat_cache_ptr]
        mov     ecx, [ebp+exFAT.NUMBER_OF_FATS]
.write_next_fat:
        push    eax
        call    fs_write32_sys
        pop     eax
        add     eax, [ebp+exFAT.SECTORS_PER_FAT]
        dec     ecx
        jnz     .write_next_fat
@@:
        pop     ecx ebx eax
        ret
;------------------------------------------------------------------------------
exFAT_notroot_next:
; DEBUGF  1, "K : exFAT_notroot_next\n"
        push    ecx
        lea     ecx, [ebp+exFAT.buffer+0x200-0x20]
        cmp     edi, ecx
        jae     exFAT_notroot_next_sector
        add     edi, 0x20
@@:
; DEBUGF  1, "K : exFAT_notroot_next.ret\n"
        pop     ecx
        ret

;exFAT_notroot_next_write:
;        push    ecx
;        lea     ecx, [ebp+exFAT.buffer+0x200]
;        cmp     edi, ecx
;        jc      @b
;        push    eax
;        call    exFAT_notroot_end_write
;        pop     eax
exFAT_notroot_next_sector:
; DEBUGF  1, "K : exFAT_notroot_next_sector\n"
        push    [ebp+exFAT.longname_sec2]
        pop     [ebp+exFAT.longname_sec1]
        push    eax
; DEBUGF  1, "K : exFAT_notroot_next.exFAT_get_sector In EAX:%x\n", eax
        call    exFAT_get_sector
; DEBUGF  1, "K : exFAT_notroot_next.exFAT_get_sector Out EAX:%x\n", eax
        mov     [ebp+exFAT.longname_sec2], eax
        pop     eax
        mov     ecx, [eax+4]
        inc     ecx
        cmp     ecx, [ebp+exFAT.SECTORS_PER_CLUSTER]
        jae     exFAT_notroot_next_cluster
        mov     [eax+4], ecx
        jmp     @f

exFAT_notroot_next_err:
; DEBUGF  1, "K : exFAT_notroot_next_err\n"
;        dec     ecx
        pop     ecx
;        js      .1
        movi    eax, ERROR_FILE_NOT_FOUND
;.1:
        stc
        ret

exFAT_notroot_next_cluster:
; DEBUGF  1, "K : exFAT_notroot_next_cluster\n"
        push    eax
        mov     eax, [eax]

;        push    edi
;        lea     edi, [ebp+exFAT.str_ext_dir_entry]
; Check - General Secondary Flags
; Bit 0 : Allocation possible
;         0 – No cluster allocated; 1 – cluster allocation is possible
; Bit 1 : No FAT chain
;         0 – Yes ; The clusters of this file/directory are NOT contiguous
;         1 – No; The Contiguous Cluster are allocated to this file/directory;
;         This improves the File read performance
; Bits 2 – 7 : Reserved
;        push    eax
;        movzx   eax, byte [edi+1]
; DEBUGF  1, "K : exFAT_notroot_next_cluster GSF 1:%x\n", eax
;        movzx   eax, byte [ebp+exFAT.General_Sec_Flags]
; DEBUGF  1, "K : exFAT_notroot_next_cluster GSF 2:%x\n", eax
;        pop     eax
;        test    byte [edi+1], 10b ;11b
;        pop     edi
        test    byte [ebp+exFAT.General_Sec_Flags], 10b
        jz      .get_FAT
        inc     eax
        jmp     .continue
.get_FAT:
        call    exFAT_get_FAT
.continue:
        mov     ecx, eax
        pop     eax
        jc      exFAT_notroot_first.deverr
        cmp     ecx, 2
        jb      exFAT_notroot_next_err
        cmp     ecx, [ebp+exFAT.fatRESERVED]
        jae     exFAT_notroot_next_err
        mov     [eax], ecx
        and     dword [eax+4], 0
@@:
        pop     ecx
exFAT_notroot_first:
; DEBUGF  1, "K : exFAT_notroot_first\n"
; DEBUGF  1, "K : exFAT_notroot_first.exFAT_get_sector In EAX:%x\n", eax
        call    exFAT_get_sector
; DEBUGF  1, "K : exFAT_notroot_first.exFAT_get_sector Out EAX:%x\n", eax
        push    ebx
        lea     edi, [ebp+exFAT.buffer]
        mov     ebx, edi
        sub     [ebp+exFAT.valid_data_length], 512
        mov     [ebp+exFAT.buffer_curr_sector], eax
        call    fs_read32_sys
        pop     ebx
        test    eax, eax
        jz      .ret ; CF=0
        push    ecx
.deverr:
; DEBUGF  1, "K : exFAT_notroot_first.deverr\n"
        pop     ecx
        mov     eax, ERROR_DEVICE
        stc
.ret:
; DEBUGF  1, "K : exFAT_notroot_first.ret\n"
        ret

;fat_notroot_begin_write:
;        push    eax edi
;        call    fat_notroot_first
;        pop     edi eax
;        ret

;fat_notroot_end_write:
;        call    fat_get_sector
;        push    ebx
;        lea     ebx, [ebp+FAT.buffer]
;        call    fs_write32_sys
;        pop     ebx
;        ret
;--------------------------------------
exFAT_get_sector:
; DEBUGF  1, "K : exFAT_get_sector\n"
        push    ecx
        mov     ecx, [eax]
; DEBUGF  1, "K : exFAT_get_sector In [EAX]:%x [EAX+4]:%x\n", ecx, [eax+4]
        dec     ecx
        dec     ecx
        imul    ecx, [ebp+exFAT.SECTORS_PER_CLUSTER]
;        add     ecx, [ebp+exFAT.DATA_START]
        add     ecx, [ebp+exFAT.CLUSTER_HEAP_START]
        add     ecx, [eax+4]
        mov     eax, ecx
        pop     ecx
        ret
;------------------------------------------------------------------------------