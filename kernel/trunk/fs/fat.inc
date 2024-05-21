;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; FAT external functions
;   in:
; ebx -> parameter structure of sysfunc 70
; ebp -> FAT structure
; esi -> path string in UTF-8
;   out:
; eax, ebx = return values for sysfunc 70
iglobal
align 4
fat_user_functions:
        dd      fat_free
        dd      (fat_user_functions_end - fat_user_functions - 8) / 4
        dd      fat_Read
        dd      fat_ReadFolder
        dd      fat_CreateFile
        dd      fat_Write
        dd      fat_SetFileEnd
        dd      fat_GetFileInfo
        dd      fat_SetFileInfo
        dd      0
        dd      fat_Delete
        dd      fat_CreateFolder
        dd      fat_Rename
fat_user_functions_end:
endg

cache_max = 1919      ; max. is 1919*512+0x610000=0x6ffe00

PUSHAD_EAX equ [esp+28]
PUSHAD_ECX equ [esp+24]
PUSHAD_EDX equ [esp+20]
PUSHAD_EBX equ [esp+16]
PUSHAD_EBP equ [esp+8]
PUSHAD_ESI equ [esp+4]
PUSHAD_EDI equ [esp+0]

; Internal data for every FAT partition.
struct FAT PARTITION
fs_type             db  ?
fat_change          db  ?   ; 1=fat has changed
createOption        db  ?
                    rb  1
Lock                MUTEX   ; currently operations with one partition
; can not be executed in parallel since the legacy code is not ready
SECTORS_PER_FAT     dd  ?
NUMBER_OF_FATS      dd  ?
SECTORS_PER_CLUSTER dd  ?
BYTES_PER_SECTOR    dd  ?   ; Note: if BPS <> 512 need lots of changes
ROOT_CLUSTER        dd  ?   ; first rootdir cluster
FAT_START           dd  ?   ; start of fat table
ROOT_START          dd  ?   ; start of rootdir (only fat16)
ROOT_SECTORS        dd  ?   ; count of rootdir sectors (only fat16)
DATA_START          dd  ?   ; start of data area (=first cluster 2)
LAST_CLUSTER        dd  ?   ; last availabe cluster
ADR_FSINFO          dd  ?   ; used only by fat32
fatRESERVED         dd  ?
fatBAD              dd  ?
fatEND              dd  ?
fatMASK             dd  ?
fatStartScan        dd  ?
cluster_tmp         dd  ?   ; used by analyze_directory and analyze_directory_to_write
longname_sec1       dd  ?   ; used by analyze_directory to save 2 previous
longname_sec2       dd  ?   ; directory sectors for delete long filename
fat_in_cache        dd  ?
; For FAT16/FAT32, this points to 512-byte buffer for the current sector of FAT.
; For FAT12, the entire FAT structure is read
; and unpacked from 12bit per cluster to word per cluster.
; Note: work with unpacked copy of FAT12 means
; additional memory and additional code for packing/unpacking.
; I'm not sure that the economy justifies the cost, but anyway,
; there is how work was done before my edits, and I'm just keeping the principle.
fat_cache_ptr       dd  ?
fat12_unpacked_ptr  dd  ?
volumeLabel         rb  12
buffer              rb  512
fsinfo_buffer       rb  512
ends

uglobal
align 4
partition_count     dd  ?   ; partitions found by set_FAT32_variables
hd_error            dd  ?
hd_setup            dd  ?
hd_wait_timeout     dd  ?
cache_search_start  dd  ?   ; used by find_empty_slot
Sector512:      ; label for dev_hdcd.inc
buffer:
rb 512
endg

; these labels are located before the main function to make
; most of jumps to these be short
fat_create_partition.free_return0:
        mov     eax, ebp
        call    free
        pop     ebp
fat_create_partition.return0:
        xor     eax, eax
        ret

fat_create_partition:
        cmp     dword [esi+DISK.MediaInfo.SectorSize], 512
        jnz     .return0
; bootsector must have been successfully read
        cmp     dword [esp+4], 0
        jnz     .return0
; bootsector signature must be correct
        cmp     word [ebx+0x1fe], 0xaa55
        jnz     .return0
; sectors per cluster must be nonzero
        cmp     byte [ebx+0xd], 0
        jz      .return0
; bytes per sector must be 0x200
        cmp     word [ebx+0xb], 0x200
        jnz     .return0
; number of fats must be nonzero
        cmp     byte [ebx+0x10], 0
        jz      .return0
; The only reason to be invalid partition now is FAT12. Since the test for
; FAT size requires knowledge of some calculated values, which are also used
; in the normal operation, let's hope for the best and allocate data now; if
; it will prove wrong, just deallocate it.
        movi    eax, sizeof.FAT
        call    malloc
        test    eax, eax
        jz      .return0
        mov     ecx, dword [ebp+PARTITION.FirstSector]
        mov     dword [eax+FAT.FirstSector], ecx
        mov     ecx, dword [ebp+PARTITION.FirstSector+4]
        mov     dword [eax+FAT.FirstSector+4], ecx
        mov     ecx, dword [ebp+PARTITION.Length]
        mov     dword [eax+FAT.Length], ecx
        mov     ecx, dword [ebp+PARTITION.Length+4]
        mov     dword [eax+FAT.Length+4], ecx
        mov     ecx, [ebp+PARTITION.Disk]
        mov     [eax+FAT.Disk], ecx
        mov     [eax+FAT.FSUserFunctions], fat_user_functions
        or      [eax+FAT.fat_in_cache], -1
        mov     [eax+FAT.fat_change], 0
        push    ebp
        mov     ebp, eax
        lea     ecx, [ebp+FAT.Lock]
        call    mutex_init
        movzx   eax, word [ebx+0xe]     ; sectors reserved
        mov     [ebp+FAT.FAT_START], eax
        movzx   eax, byte [ebx+0xd]     ; sectors per cluster
        mov     [ebp+FAT.SECTORS_PER_CLUSTER], eax
        movzx   ecx, word [ebx+0xb]     ; bytes per sector
        mov     [ebp+FAT.BYTES_PER_SECTOR], ecx
        movzx   eax, word [ebx+0x11]    ; count of rootdir entries (=0 fat32)
        shl     eax, 5                  ; mul 32
        dec     ecx
        add     eax, ecx                ; round up if not equal count
        inc     ecx                     ; bytes per sector
        xor     edx, edx
        div     ecx
        mov     [ebp+FAT.ROOT_SECTORS], eax     ; count of rootdir sectors
        movzx   eax, word [ebx+0x16]    ; sectors per fat <65536
        test    eax, eax
        jnz     @f
        mov     eax, [ebx+0x24]         ; sectors per fat
@@:
        mov     [ebp+FAT.SECTORS_PER_FAT], eax
        movzx   eax, byte [ebx+0x10]    ; number of fats
        mov     [ebp+FAT.NUMBER_OF_FATS], eax
        mul     [ebp+FAT.SECTORS_PER_FAT]
        test    edx, edx
        jnz     .free_return0
        add     eax, [ebp+FAT.FAT_START]
        jc      .free_return0
        mov     [ebp+FAT.ROOT_START], eax       ; rootdir = fat_start + fat_size * fat_count
        add     eax, [ebp+FAT.ROOT_SECTORS]     ; rootdir sectors should be 0 on fat32
        jc      .free_return0
        mov     [ebp+FAT.DATA_START], eax       ; data area = rootdir + rootdir_size
        movzx   eax, word [ebx+0x13]    ; total sector count <65536
        test    eax, eax
        jnz     @f
        mov     eax, [ebx+0x20]         ; total sector count
@@:
        cmp     dword [ebp+FAT.Length+4], 0
        jnz     @f
        cmp     eax, dword [ebp+FAT.Length]
        ja      .free_return0
@@:
        mov     dword [ebp+FAT.Length], eax
        and     dword [ebp+FAT.Length+4], 0
        sub     eax, [ebp+FAT.DATA_START]       ; eax = count of data sectors
        jc      .free_return0
        xor     edx, edx
        div     [ebp+FAT.SECTORS_PER_CLUSTER]
        inc     eax
        mov     [ebp+FAT.LAST_CLUSTER], eax
        dec     eax                     ; cluster count
        jz      .free_return0
        mov     [ebp+FAT.fatStartScan], 2
        cmp     eax, 0xfff5
        jb      .fat16
.fat32:
        pusha
        lea     esi, [ebx+71]
        lea     edi, [ebp+FAT.volumeLabel]
        movsd
        movsd
        movsd
        popa
        mov     eax, [ebx+0x2c]         ; rootdir cluster
        mov     [ebp+FAT.ROOT_CLUSTER], eax
        movzx   eax, word [ebx+0x30]
        mov     [ebp+FAT.ADR_FSINFO], eax
        push    ebx
        add     ebx, 512
        call    fs_read32_sys
        test    eax, eax
        jnz     @f
        mov     eax, [ebx+0x1ec]
        cmp     eax, -1
        jz      @f
        mov     [ebp+FAT.fatStartScan], eax
@@:
        pop     ebx
        mov     [ebp+FAT.fatRESERVED], 0x0FFFFFF6
        mov     [ebp+FAT.fatBAD], 0x0FFFFFF7
        mov     [ebp+FAT.fatEND], 0x0FFFFFF8
        mov     [ebp+FAT.fatMASK], 0x0FFFFFFF
        mov     al, 32
.fat_not_12_finalize:
        mov     [ebp+FAT.fs_type], al
; For FAT16 and FAT32, allocate 512 bytes for FAT cache.
        mov     eax, 512
        call    malloc
        test    eax, eax
        jz      .free_return0
        mov     [ebp+FAT.fat_cache_ptr], eax
        mov     eax, ebp
        pop     ebp
        ret

.fat16:
        pusha
        lea     esi, [ebx+43]
        lea     edi, [ebp+FAT.volumeLabel]
        movsd
        movsd
        movsd
        popa
        cmp     eax, 0xff5
        jb      .fat12
        and     [ebp+FAT.ROOT_CLUSTER], 0
        mov     [ebp+FAT.fatRESERVED], 0x0000FFF6
        mov     [ebp+FAT.fatBAD], 0x0000FFF7
        mov     [ebp+FAT.fatEND], 0x0000FFF8
        mov     [ebp+FAT.fatMASK], 0x0000FFFF
        mov     al, 16
        jmp     .fat_not_12_finalize

.fat12:
        and     [ebp+FAT.ROOT_CLUSTER], 0
        mov     [ebp+FAT.fatRESERVED], 0xFF6
        mov     [ebp+FAT.fatBAD], 0xFF7
        mov     [ebp+FAT.fatEND], 0xFFF
        mov     [ebp+FAT.fatMASK], 0xFFF
        mov     al, 12
        mov     [ebp+FAT.fs_type], al
; For FAT12, allocate&read data for entire table:
; calculate A = ALIGN_UP(NUM_CLUSTERS, 8),
; allocate ALIGN_UP(A*3/2, 512) bytes for FAT table plus A*2 bytes for unpacked data.
        mov     eax, [ebp+FAT.LAST_CLUSTER]
        and     eax, not 7
        add     eax, 8
        mov     edx, eax
        lea     eax, [eax*3]
        add     eax, 512*2-1
        shr     eax, 10
        shl     eax, 9
        lea     eax, [eax+edx*2]
        call    malloc
        test    eax, eax
        jz      .free_return0
; Read ALIGN_UP(NUM_CLUSTERS*3/2, 512) bytes.
        push    ebx
        mov     [ebp+FAT.fat_cache_ptr], eax
        mov     edx, [ebp+FAT.LAST_CLUSTER]
        lea     edx, [(edx+1)*3 + 512*2-1]
        shr     edx, 10
        xchg    eax, ebx
        xor     eax, eax
.read_fat:
        push    eax
        add     eax, [ebp+FAT.FAT_START]
        call    fs_read32_sys
        test    eax, eax
        pop     eax
        jz      @f
        mov     eax, [ebp+FAT.fat_cache_ptr]
        call    free
        pop     ebx
        jmp     .free_return0

@@:
        add     ebx, 512
        inc     eax
        cmp     eax, edx
        jb      .read_fat
        mov     [ebp+FAT.fat12_unpacked_ptr], ebx
        pushad
        mov     esi, [ebp+FAT.fat_cache_ptr]
        mov     edi, [ebp+FAT.fat12_unpacked_ptr]
        mov     edx, [ebp+FAT.LAST_CLUSTER]
        and     edx, not 7
        lea     edx, [edi+(edx+8)*2]
        push    edx
@@:
        mov     eax, dword [esi]
        mov     ebx, dword [esi+4]
        mov     ecx, dword [esi+8]
        mov     edx, ecx
        shr     edx, 4
        shr     dx, 4
        xor     ch, ch
        shld    ecx, ebx, 20
        shr     cx, 4
        shld    ebx, eax, 12
        and     ebx, 0x0fffffff
        shr     bx, 4
        shl     eax, 4
        and     eax, 0x0fffffff
        shr     ax, 4
        mov     dword [edi], eax
        mov     dword [edi+4], ebx
        mov     dword [edi+8], ecx
        mov     dword [edi+12], edx
        add     edi, 16
        add     esi, 12
        cmp     edi, [esp]
        jnz     @b
        pop     eax
        popad
        mov     eax, ebp
        pop     ebx ebp
        ret

fat_free:
        push    eax
        mov     eax, [eax+FAT.fat_cache_ptr]
        call    free
        pop     eax
        jmp     free

iglobal
label fat_legal_chars byte
; 0 = not allowed
; 1 = allowed only in long names
; 3 = allowed
        times 32 db 0
;                 ! " # $ % & ' ( ) * + , - . /
        db      1,3,0,3,3,3,3,3,3,3,0,1,1,3,3,0
;               0 1 2 3 4 5 6 7 8 9 : ; < = > ?
        db      3,3,3,3,3,3,3,3,3,3,0,1,0,1,0,0
;               @ A B C D E F G H I J K L M N O
        db      3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
;               P Q R S T U V W X Y Z [ \ ] ^ _
        db      3,3,3,3,3,3,3,3,3,3,3,1,0,1,3,3
;               ` a b c d e f g h i j k l m n o
        db      3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
;               p q r s t u v w x y z { | } ~
        db      3,3,3,3,3,3,3,3,3,3,3,3,0,3,3,0
endg

fat_name_is_legal:
; in: esi -> UTF-8 name
; out: CF=1 -> legal
        push    esi
        xor     eax, eax
@@:
        lodsb
        test    al, al
        js      @b
        test    [fat_legal_chars+eax], 1
        jnz     @b
        test    al, al
        jnz     @f
        stc
@@:
        pop     esi
        ret

fat_next_short_name:
; in: edi->8+3 name
; out: name corrected, CF=1 -> error
        pushad
        mov     ecx, 8
        mov     al, '~'
        std
        push    edi
        add     edi, 7
        repnz scasb
        pop     edi
        cld
        jz      .tilde
; tilde is not found, insert "~1" at end
        add     edi, 6
        cmp     word [edi], '  '
        jnz     .insert_tilde
@@:
        dec     edi
        cmp     byte [edi], ' '
        jz      @b
        inc     edi
.insert_tilde:
        mov     word [edi], '~1'
        popad
        clc
        ret

.tilde:
        push    edi
        add     edi, 7
        xor     ecx, ecx
@@: ; after tilde may be only digits and trailing spaces
        cmp     byte [edi], '~'
        jz      .break
        cmp     byte [edi], ' '
        jz      .space
        cmp     byte [edi], '9'
        jnz     .found
        dec     edi
        jmp     @b

.space:
        dec     edi
        inc     ecx
        jmp     @b

.found:
        inc     byte [edi]
        add     dword [esp], 8
        jmp     .zerorest

.break:
        jecxz   .noplace
        inc     edi
        mov     al, '1'
@@:
        xchg    al, [edi]
        inc     edi
        cmp     al, ' '
        mov     al, '0'
        jnz     @b
.succ:
        pop     edi
        popad
        clc
        ret

.noplace:
        dec     edi
        cmp     edi, [esp]
        jz      .err
        add     dword [esp], 8
        mov     word [edi], '~1'
        inc     edi
        inc     edi
@@:
        mov     byte [edi], '0'
.zerorest:
        inc     edi
        cmp     edi, [esp]
        jb      @b
        pop     edi
        popad
        ret

.err:
        pop     edi
        popad
        stc
        ret

fat_gen_short_name:
;   in:
; esi -> UTF-8 name
; edi -> buffer (8+3=11 chars)
        pushad
        mov     eax, '    '
        push    edi
        stosd
        stosd
        stosd
        pop     edi
        xor     eax, eax
        movi    ebx, 8
        lea     ecx, [edi+8]
.loop:
        lodsb
        test    al, al
        js      .space
        jz      .done
        test    [fat_legal_chars+eax], 2
        jz      .space
        cmp     al, '.'
        jz      .dot
        dec     bl
        jns     .store
        inc     bl
.space:
        or      bh, 1
        jmp     .loop

.store:
        call    cp866toUpper
        stosb
        jmp     .loop

.dot:
        test    bh, 2
        jz      .firstdot
        pop     ebx
        add     ebx, edi
        sub     ebx, ecx
        push    ebx
        cmp     ebx, ecx
        jb      @f
        pop     ebx
        push    ecx
@@:
        cmp     edi, ecx
        jbe     .skip
@@:
        dec     edi
        mov     al, [edi]
        dec     ebx
        mov     [ebx], al
        mov     byte [edi], ' '
        cmp     edi, ecx
        ja      @b
.skip:
        mov     bh, 3
        jmp     @f

.firstdot:
        cmp     bl, 8
        jz      .space
        push    edi
        or      bh, 2
@@:
        mov     edi, ecx
        mov     bl, 3
        jmp     .loop

.done:
        test    bh, 2
        jz      @f
        pop     edi
@@:
        lea     edi, [ecx-8]
        test    bh, 1
        jz      @f
        call    fat_next_short_name
@@:
        popad
        ret

set_FAT:
; in: eax = cluster, edx = value to save
; out: edx = old value, CF=1 -> error
        push    eax ebx esi
        cmp     eax, 2
        jc      .ret
        cmp     [ebp+FAT.LAST_CLUSTER], eax
        jc      .ret
        cmp     [ebp+FAT.fs_type], 12
        je      .FAT12
        cmp     [ebp+FAT.fs_type], 16
        je      @f
        add     eax, eax
@@:
        add     eax, eax
        mov     esi, 511
        and     esi, eax
        shr     eax, 9
        add     eax, [ebp+FAT.FAT_START]
        mov     ebx, [ebp+FAT.fat_cache_ptr]
        cmp     eax, [ebp+FAT.fat_in_cache]
        je      .inCache
        cmp     [ebp+FAT.fat_change], 0
        je      @f
        call    write_fat_sector
@@:
        mov     [ebp+FAT.fat_in_cache], eax
        call    fs_read32_sys
        test    eax, eax
        jne     .error
.inCache:
        cmp     [ebp+FAT.fs_type], 16
        jne     .test32
        xchg    [ebx+esi], dx   ; save new value and get old value
        jmp     .write

.test32:
        mov     eax, [ebp+FAT.fatMASK]
        and     edx, eax
        xor     eax, -1         ; mask for high bits
        and     eax, [ebx+esi]  ; get high 4 bits
        or      eax, edx
        mov     edx, [ebx+esi]  ; get old value
        mov     [ebx+esi], eax  ; save new value
.write:
        mov     [ebp+FAT.fat_change], 1
        and     edx, [ebp+FAT.fatMASK]
.ret:
        pop     esi ebx eax
        ret

.error:
        stc
        jmp     .ret

.FAT12:
        test    edx, 0xF000
        jnz     .error
        mov     ebx, [ebp+FAT.fat12_unpacked_ptr]
        xchg    [ebx+eax*2], dx
        mov     [ebp+FAT.fat_change], 1
        jmp     .ret

get_FAT:
; in: eax = cluster
; out: eax = next cluster, CF=1 -> error
        push    ebx esi
        cmp     [ebp+FAT.fs_type], 12
        je      .FAT12
        cmp     [ebp+FAT.fs_type], 16
        je      @f
        add     eax, eax
@@:
        add     eax, eax
        mov     esi, 511
        and     esi, eax
        shr     eax, 9
        add     eax, [ebp+FAT.FAT_START]
        mov     ebx, [ebp+FAT.fat_cache_ptr]
        cmp     eax, [ebp+FAT.fat_in_cache]
        je      .inCache
        cmp     [ebp+FAT.fat_change], 0
        je      @f
        call    write_fat_sector
@@:
        mov     [ebp+FAT.fat_in_cache], eax
        call    fs_read32_sys
        test    eax, eax
        jnz     .error
.inCache:
        mov     eax, [ebx+esi]
        and     eax, [ebp+FAT.fatMASK]
.ret:
        pop     esi ebx
        ret

.error:
        stc
        jmp     .ret

.FAT12:
        mov     ebx, [ebp+FAT.fat12_unpacked_ptr]
        movzx   eax, word [ebx+eax*2]
        jmp     .ret

get_free_FAT:
; out: eax = number of first free cluster, CF=1 -> disk full
        push    ecx
        mov     ecx, [ebp+FAT.LAST_CLUSTER]
        mov     eax, [ebp+FAT.fatStartScan]
        cmp     [ebp+FAT.fs_type], 12
        jz      get_free_FAT12
        dec     ecx
        cmp     eax, 2
        jb      .reset
.test:
        cmp     eax, [ebp+FAT.LAST_CLUSTER]
        jbe     .inRange
.reset:
        mov     eax, 2
.inRange:
        push    eax
        call    get_FAT
        jc      @f
        test    eax, eax
        pop     eax
        je      .found
        inc     eax
        dec     ecx
        jnz     .test
.notFound:
        pop     ecx
        stc
        ret

@@:
        pop     eax
        jmp     .notFound

.found:
        lea     ecx, [eax+1]
        mov     [ebp+FAT.fatStartScan], ecx
        pop     ecx
        clc
        ret

get_free_FAT12:
        push    edx edi
        mov     edi, [ebp+FAT.fat12_unpacked_ptr]
        cmp     eax, 2
        jb      .reset
        cmp     eax, ecx
        jbe     @f
.reset:
        mov     eax, 2
@@:
        mov     edx, eax
        lea     edi, [edi+eax*2]
        sub     ecx, eax
        inc     ecx
        xor     eax, eax
        repnz scasw
        jz      .found
        cmp     edx, 2
        jz      .notfound
        mov     edi, [ebp+FAT.fat12_unpacked_ptr]
        lea     ecx, [edx-2]
        repnz scasw
        jnz     .notfound
.found:
        sub     edi, [ebp+FAT.fat12_unpacked_ptr]
        shr     edi, 1
        mov     [ebp+FAT.fatStartScan], edi
        lea     eax, [edi-1]
@@:
        pop     edi edx ecx
        ret

.notfound:
        stc
        jmp     @b

write_fat_sector:
        push    eax ebx ecx
        mov     [ebp+FAT.fat_change], 0
        mov     eax, [ebp+FAT.fat_in_cache]
        cmp     eax, -1
        jz      @f
        mov     ebx, [ebp+FAT.fat_cache_ptr]
        mov     ecx, [ebp+FAT.NUMBER_OF_FATS]
.write_next_fat:
        push    eax
        call    fs_write32_sys
        pop     eax
        add     eax, [ebp+FAT.SECTORS_PER_FAT]
        dec     ecx
        jnz     .write_next_fat
@@:
        pop     ecx ebx eax
        ret

get_date_for_file:
;   out in ax:
; bits 0-4 = day
; bits 5-8 = month
; bits 9-15 = count of years from 1980
        mov     al, 7
        call    fsReadCMOS
        ror     eax, 5
        mov     al, 8
        call    fsReadCMOS
        ror     eax, 4
        mov     al, 9
        call    fsReadCMOS
        add     ax, 20
        rol     eax, 9
        ret

get_time_for_file:
;   out in ax:
; bits 0-4 = second (the low bit is lost)
; bits 5-10 = minute
; bits 11-15 = hour
        mov     al, 0
        call    fsReadCMOS
        ror     eax, 6
        mov     al, 2
        call    fsReadCMOS
        ror     eax, 6
        mov     al, 4
        call    fsReadCMOS
        rol     eax, 11
        ret

add_disk_free_space:
; in: ecx = cluster count (signed)
        test    ecx, ecx
        je      .ret
        cmp     [ebp+FAT.fs_type], 32
        jne     .ret
        push    eax ebx
        mov     eax, [ebp+FAT.ADR_FSINFO]
        lea     ebx, [ebp+FAT.fsinfo_buffer]
        call    fs_read32_sys
        test    eax, eax
        jnz     @f
        cmp     dword [ebx+0x1fc], 0xaa550000   ; check sector id
        jne     @f
        add     [ebx+0x1e8], ecx
        push    [ebp+FAT.fatStartScan]
        pop     dword [ebx+0x1ec]
        mov     eax, [ebp+FAT.ADR_FSINFO]
        call    fs_write32_sys
@@:
        pop     ebx eax
.ret:
        ret

clear_cluster_chain:
; in: eax = first cluster
        push    eax ecx edx
        xor     ecx, ecx        ; cluster count
@@:
        cmp     eax, [ebp+FAT.LAST_CLUSTER]
        ja      @f
        cmp     eax, 2
        jb      @f
        cmp     eax, [ebp+FAT.ROOT_CLUSTER]
        jz      @f
        xor     edx, edx
        call    set_FAT
        jc      .ret
        inc     ecx
        mov     eax, edx
        jmp     @b

@@:
        call    add_disk_free_space
        clc
.ret:
        pop     edx ecx eax
        ret

update_disk:
        cmp     [ebp+FAT.fat_change], 0
        jz      .noChange
        cmp     [ebp+FAT.fs_type], 12
        jz      .fat12
        call    write_fat_sector
.noChange:
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
        ret

.fat12:
        mov     esi, [ebp+FAT.fat12_unpacked_ptr]
        mov     edi, [ebp+FAT.fat_cache_ptr]
        mov     edx, [ebp+FAT.LAST_CLUSTER]
        and     edx, not 7
        lea     edx, [esi+(edx+8)*2]
@@:
        mov     eax, dword [esi]
        mov     ebx, dword [esi+4]
        shl     ax, 4
        shl     eax, 4
        shl     bx, 4
        shr     ebx, 4
        shrd    eax, ebx, 8
        shr     ebx, 8
        mov     dword [edi], eax
        mov     word [edi+4], bx
        add     edi, 6
        add     esi, 8
        cmp     esi, edx
        jb      @b
        mov     esi, [ebp+FAT.NUMBER_OF_FATS]
        mov     edx, [ebp+FAT.LAST_CLUSTER]
        lea     edx, [(edx+1)*3 + 512*2-1]
        shr     edx, 10
        push    [ebp+FAT.FAT_START]
.write_fats:
        xor     eax, eax
        mov     ebx, [ebp+FAT.fat_cache_ptr]
.loop1:
        push    eax
        add     eax, [esp+4]
        call    fs_write32_sys
        test    eax, eax
        pop     eax
        jnz     @f
        add     ebx, 512
        inc     eax
        cmp     eax, edx
        jb      .loop1
        pop     eax
        add     eax, [ebp+FAT.SECTORS_PER_FAT]
        push    eax
        dec     esi
        jnz     .write_fats
@@:
        pop     eax
        mov     [ebp+FAT.fat_change], 0
        jmp     .noChange

fat_lock:
        lea     ecx, [ebp+FAT.Lock]
        jmp     mutex_lock

fat_unlock:
        lea     ecx, [ebp+FAT.Lock]
        jmp     mutex_unlock

fat_get_name:
; in: edi -> FAT entry, esi -> buffer for UTF-16 name
; out: CF=1 -> no valid entry
        cmp     byte [edi], 0
        jz      .no
        cmp     byte [edi], 0xE5
        jz      .no
        cmp     byte [edi+11], 0xF
        jz      .longname
        push    edi
        xchg    esi, edi
        test    byte [esi+11], 8
        jnz     .label
        pushd   ecx 8
        pop     ecx
@@:
        lodsb
        call    ansi2uni_char
        stosw
        loop    @b
        mov     cl, 8
@@:
        cmp     word [edi-2], ' '
        jnz     @f
        sub     edi, 2
        loop    @b
@@:
        mov     word [edi], '.'
        add     edi, 2
        mov     cl, 3
@@:
        lodsb
        call    ansi2uni_char
        stosw
        loop    @b
        mov     cl, 3
@@:
        cmp     word [edi-2], ' '
        jnz     @f
        sub     edi, 2
        loop    @b
        sub     edi, 2
@@:
        and     word [edi], 0   ; CF=0
        pop     ecx edi
        ret

.label:
        lea     edi, [ebp+FAT.volumeLabel]
        movsd
        movsd
        movsd
        pop     edi
.no:
        stc
        ret

.longname:
        mov     al, byte [edi]
        and     eax, 0x3F
        dec     eax
        cmp     al, 20
        jae     .no     ; ignore invalid entries
        mov     word [esi+260*2], 0     ; force null-terminating for orphans
        imul    eax, 13*2
        test    byte [edi], 0x40
        jz      @f
        mov     word [esi+eax+13*2], 0
@@: ; copy name (13 chars in UTF-16)
        push    edi
        inc     edi
        add     esi, eax
        xchg    esi, edi
        movsd
        movsd
        movsw
        add     esi, 3
        movsd
        movsd
        movsd
        add     esi, 2
        movsd
        pop     edi
        test    eax, eax
        jnz     .no ; if this is not first entry, more processing required
        ret

fat_find_lfn:
;   in:
; esi -> path in UTF-8
; parameters in the stack
;   out:
; esi -> next name in the path
; edi -> direntry
; CF=1 -> file not found, eax = error code
        lea     eax, [esp+12]
        call    dword [eax-4]
        jc      .reterr
        sub     esp, 262*2      ; reserve place for LFN
.l1:
        push    esi
        lea     esi, [esp+4]
        call    fat_get_name
        pop     esi
        jc      .no
        push    edi esi
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
        add     esp, 262*2
; if this is LFN entry, advance to true entry
        cmp     byte [edi+11], 0xF
        jnz     @f
        lea     eax, [esp+12]
        call    dword[eax-8]
        jc      .reterr
@@:
        xor     eax, eax
        ret

.done:
        cmp     dx, '/'
        jnz     @f
        test    ax, ax
        jnz     @f
        mov     [esp], esi
@@:
        pop     esi edi
        jz      .found
.no:
        lea     eax, [esp+262*2+12]
        call    dword[eax-8]
        jnc     .l1
        add     esp, 262*2
.reterr:
        stc
        ret

fat_time_to_bdfe:
; in: eax=FAT time
; out: eax=BDFE time
        push    ecx edx
        mov     ecx, eax
        mov     edx, eax
        shr     eax, 11
        shl     eax, 16 ; hours
        and     edx, 0x1F
        add     edx, edx
        mov     al, dl  ; seconds
        shr     ecx, 5
        and     ecx, 0x3F
        mov     ah, cl  ; minutes
        pop     edx ecx
        ret

fat_date_to_bdfe:
        push    ecx edx
        mov     ecx, eax
        mov     edx, eax
        shr     eax, 9
        add     ax, 1980
        shl     eax, 16 ; year
        and     edx, 0x1F
        mov     al, dl  ; day
        shr     ecx, 5
        and     ecx, 0xF
        mov     ah, cl  ; month
        pop     edx ecx
        ret

bdfe_to_fat_time:
        push    edx
        mov     edx, eax
        shr     eax, 16
        and     dh, 0x3F
        shl     eax, 6
        or      al, dh
        shr     dl, 1
        and     dl, 0x1F
        shl     eax, 5
        or      al, dl
        pop     edx
        ret

bdfe_to_fat_date:
        push    edx
        mov     edx, eax
        shr     eax, 16
        sub     ax, 1980
        and     dh, 0xF
        shl     eax, 4
        or      al, dh
        and     dl, 0x1F
        shl     eax, 5
        or      al, dl
        pop     edx
        ret

fat_entry_to_bdfe:
; convert FAT entry at edi to BDFE (block of data of folder entry) at esi, advance esi
        mov     eax, [ebp-4]
        mov     [esi+4], eax    ; cp866/UNICODE name
fat_entry_to_bdfe2:
        movzx   eax, byte [edi+11]
        mov     [esi], eax      ; attributes
        movzx   eax, word [edi+14]
        call    fat_time_to_bdfe
        mov     [esi+8], eax    ; creation time
        movzx   eax, word [edi+16]
        call    fat_date_to_bdfe
        mov     [esi+12], eax   ; creation date
        and     dword [esi+16], 0       ; last access time is not supported on FAT
        movzx   eax, word [edi+18]
        call    fat_date_to_bdfe
        mov     [esi+20], eax   ; last access date
        movzx   eax, word [edi+22]
        call    fat_time_to_bdfe
        mov     [esi+24], eax   ; last write time
        movzx   eax, word [edi+24]
        call    fat_date_to_bdfe
        mov     [esi+28], eax   ; last write date
        mov     eax, [edi+28]
        mov     [esi+32], eax   ; file size (low dword)
        xor     eax, eax
        mov     [esi+36], eax   ; file size (high dword)
        test    ebp, ebp
        jz      .ret
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
        ret

bdfe_to_fat_entry:
; convert BDFE at edx to FAT entry at edi
; destroys eax
; attributes byte
        test    byte [edi+11], 8        ; volume label?
        jnz     @f
        mov     al, [edx]
        and     al, 0x27
        and     byte [edi+11], 0x10
        or      byte [edi+11], al
@@:
        mov     eax, [edx+8]
        call    bdfe_to_fat_time
        mov     [edi+14], ax            ; creation time
        mov     eax, [edx+12]
        call    bdfe_to_fat_date
        mov     [edi+16], ax            ; creation date
        mov     eax, [edx+20]
        call    bdfe_to_fat_date
        mov     [edi+18], ax            ; last access date
        mov     eax, [edx+24]
        call    bdfe_to_fat_time
        mov     [edi+22], ax            ; last write time
        mov     eax, [edx+28]
        call    bdfe_to_fat_date
        mov     [edi+24], ax            ; last write date
        ret

hd_find_lfn:
; in: esi -> path string in UTF-8
; out: CF=1 - file not found, eax=error code
;      else CF=0 and edi->direntry, eax=sector
        push    esi edi
        push    0
        push    0
        push    fat1x_root_first
        push    fat1x_root_next
        mov     eax, [ebp+FAT.ROOT_CLUSTER]
        cmp     [ebp+FAT.fs_type], 32
        jz      .fat32
.loop:
        and     [ebp+FAT.longname_sec1], 0
        and     [ebp+FAT.longname_sec2], 0
        call    fat_find_lfn
        jc      .notfound
        cmp     byte [esi], 0
        jz      .found
        test    byte [edi+11], 10h
        jz      .notfound
        and     dword [esp+12], 0
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]    ; cluster
.fat32:
        mov     [esp+8], eax
        mov     dword [esp+4], fat_notroot_first
        mov     dword [esp], fat_notroot_next
        jmp     .loop

.notfound:
        add     esp, 16
        pop     edi esi
        stc
        ret

.found:
        lea     eax, [esp+8]
        cmp     dword [eax], 0
        jz      .root
        call    fat_get_sector
        jmp     .cmn

.root:
        mov     eax, [eax+4]
        add     eax, [ebp+FAT.ROOT_START]
.cmn:
        add     esp, 20         ; CF=0
        pop     esi
        ret

;----------------------------------------------------------------
fat_Read:
        call    fat_lock
        call    hd_find_lfn
        jc      .notFound
        test    byte [edi+11], 0x10     ; do not allow read directories
        jnz     .noaccess
        cmp     dword [ebx+8], 0
        jnz     .endOfFile
        mov     edx, [ebx+4]    ; file offset
        mov     ecx, [ebx+12]   ; size
        mov     ebx, [ebx+16]   ; buffer
        push    ebx
        push    0
        test    ecx, ecx
        jz      .done
        mov     eax, [edi+28]
        sub     eax, edx
        jb      .fileEnd
        cmp     eax, ecx
        jae     @f
        mov     ecx, eax
        mov     byte [esp], 6
@@:
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]
; now eax=cluster, ebx=buffer for data, ecx=count, edx=position
        mov     edi, [ebp+FAT.SECTORS_PER_CLUSTER]
        shl     edi, 9
@@:
        cmp     eax, 2
        jb      .fileEnd
        cmp     eax, [ebp+FAT.fatRESERVED]
        jae     .fileEnd
        sub     edx, edi
        jc      @f
        call    get_FAT
        jc      .noaccess2
        jmp     @b

.notFound:
        push    eax
        jmp     .ret

.noaccess:
        push    ERROR_ACCESS_DENIED
        jmp     .ret

.endOfFile:
        push    ERROR_END_OF_FILE
.ret:
        call    fat_unlock
        pop     eax
        xor     ebx, ebx
        ret

@@:
        mov     esi, eax
        dec     eax
        dec     eax
        imul    eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+FAT.DATA_START]
        add     edx, edi
        jz      .alignedCluster
        mov     edi, edx
        shr     edi, 9
        add     eax, edi
        and     edx, 511
        cmp     ecx, 512
        jc      .sectorPiece
        test    edx, edx
        jz      .alignedSector
.sectorPiece:
        push    eax ebx
        lea     ebx, [ebp+FAT.buffer]
        call    fs_read32_app
        test    eax, eax
        mov     eax, ebx
        pop     ebx
        jne     .noaccess3
        add     eax, edx
        push    ecx
        add     ecx, edx
        cmp     ecx, 512
        jbe     @f
        mov     ecx, 512
@@:
        sub     ecx, edx
        call    memmove
        sub     [esp], ecx
        add     ebx, ecx
        pop     ecx eax
        xor     edx, edx
        inc     edi
        inc     eax
        test    ecx, ecx
        jz      .done
.alignedSector:
        shl     edi, 9
        add     ecx, edi
        mov     edi, [ebp+FAT.SECTORS_PER_CLUSTER]
        shl     edi, 9
.alignedCluster:
        cmp     ecx, 512
        jc      .sectorPiece
        mov     edx, eax
        mov     eax, esi
@@:
        sub     ecx, edi
        jbe     .readEnd
        call    get_FAT
        jc      .noaccess4
        cmp     eax, 2
        jb      .fileEnd2
        cmp     eax, [ebp+FAT.fatRESERVED]
        jae     .fileEnd2
        inc     esi
        cmp     eax, esi
        jz      @b
.fragmentEnd:
        xchg    eax, esi
        dec     eax
        dec     eax
        imul    eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+FAT.DATA_START]
        push    ecx
        mov     ecx, eax
        mov     eax, esi
        dec     eax
        dec     eax
        imul    eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+FAT.DATA_START]
        push    eax
.readFragment:
        sub     ecx, edx
        mov     eax, edx
        xor     edx, edx
        call    fs_read64_app
        shl     ecx, 9
        add     ebx, ecx
        test    eax, eax
        pop     eax
        jnz     .noaccess3
        pop     ecx
        xor     edx, edx
        jecxz   .done
        jmp     .alignedCluster

.readEnd:
        add     ecx, edi
        mov     edi, ecx
        and     ecx, 511
        shr     edi, 9
        dec     eax
        dec     eax
        imul    eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+FAT.DATA_START]
        add     eax, edi
        push    ecx
        push    eax
        mov     ecx, eax
        jmp     .readFragment

.noaccess3:
        pop     eax
.noaccess2:
        mov     byte [esp], ERROR_DEVICE
.done:
        call    fat_unlock
        pop     eax edx
        sub     ebx, edx
        ret

.fileEnd:
        mov     byte [esp], ERROR_END_OF_FILE
        jmp     .done

.noaccess4:
        mov     byte [esp], ERROR_DEVICE
        jmp     @f

.fileEnd2:
        mov     byte [esp], ERROR_END_OF_FILE
@@:
        inc     esi
        xor     ecx, ecx
        jmp     .fragmentEnd

;----------------------------------------------------------------
fat_ReadFolder:
        call    fat_lock
        mov     eax, [ebp+FAT.ROOT_CLUSTER]
        cmp     byte [esi], 0
        jz      .doit
        call    hd_find_lfn
        jc      .error
        test    byte [edi+11], 0x10     ; do not allow read files
        jz      .accessDenied
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]    ; eax=cluster
.doit:
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
        mov     esi, edi        ; esi points to BDFE
        mov     ecx, [ebx+12]   ; number of blocks to read
        mov     ebx, [ebx+4]    ; index of the first block
.new_cluster:
        mov     [ebp+FAT.cluster_tmp], eax
        test    eax, eax
        jnz     @f
        cmp     [ebp+FAT.fs_type], 32
        jz      .notfound
        mov     eax, [ebp+FAT.ROOT_START]
        push    [ebp+FAT.ROOT_SECTORS]
        push    ebx
        jmp     .new_sector

@@:
        dec     eax
        dec     eax
        imul    eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        push    [ebp+FAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+FAT.DATA_START]
        push    ebx
.new_sector:
        lea     ebx, [ebp+FAT.buffer]
        mov     edi, ebx
        push    eax
        call    fs_read32_sys
        test    eax, eax
        pop     eax
        jnz     .notfound2
        add     ebx, 512
        push    eax
.l1:
        push    esi
        lea     esi, [esp+20]
        call    fat_get_name
        pop     esi
        jc      .l2
        cmp     byte [edi+11], 0xF
        jnz     .do_bdfe
        add     edi, 0x20
        cmp     edi, ebx
        jb      .do_bdfe
        pop     eax
        inc     eax
        dec     dword [esp+4]
        jnz     @f
        mov     eax, [ebp+FAT.cluster_tmp]
        test    eax, eax
        jz      .done
        call    get_FAT
        jc      .notfound2
        cmp     eax, 2
        jb      .done
        cmp     eax, [ebp+FAT.fatRESERVED]
        jae     .done
        push    eax
        mov     eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        mov     [esp+8], eax
        pop     eax
        mov     [ebp+FAT.cluster_tmp], eax
        dec     eax
        dec     eax
        imul    eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+FAT.DATA_START]
@@:
        lea     ebx, [ebp+FAT.buffer]
        mov     edi, ebx
        push    eax
        call    fs_read32_sys
        test    eax, eax
        pop     eax
        jnz     .notfound2
        add     ebx, 512
        push    eax
.do_bdfe:
        inc     dword [edx+8]   ; new file found
        dec     dword [esp+4]
        jns     .l2
        dec     ecx
        js      .l2
        inc     dword [edx+4]   ; new file block copied
        push    ebp
        lea     ebp, [esp+20]
        call    fat_entry_to_bdfe
        pop     ebp
.l2:
        add     edi, 0x20
        cmp     edi, ebx
        jb      .l1
        pop     eax
        inc     eax
        dec     dword [esp+4]
        jnz     .new_sector
        mov     eax, [ebp+FAT.cluster_tmp]
        test    eax, eax
        jz      .done
        call    get_FAT
        jc      .notfound2
        cmp     eax, 2
        jb      .done
        cmp     eax, [ebp+FAT.fatRESERVED]
        jae     .done
        push    eax
        mov     eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        mov     [esp+8], eax
        pop     eax
        pop     ebx
        add     esp, 4
        jmp     .new_cluster

.notfound2:
        add     esp, 8
.notfound:
        add     esp, 262*2+4
        push    ERROR_DEVICE
        jmp     @f

.done:
        add     esp, 262*2+12
        pushd   0
        dec     ecx
        js      @f
        mov     byte [esp], ERROR_END_OF_FILE
@@:
        mov     ebx, [edx+4]
.ret:
        call    fat_unlock
        pop     eax
        ret

.error:
        push    eax
        xor     ebx, ebx
        jmp     .ret

.accessDenied:
        push    ERROR_ACCESS_DENIED
        xor     ebx, ebx
        jmp     .ret

fat1x_root_next:
        push    ecx
        lea     ecx, [ebp+FAT.buffer+0x200-0x20]
        cmp     edi, ecx
        jae     fat1x_root_next_sector
        add     edi, 0x20
@@:
        pop     ecx
        ret

fat1x_root_next_write:
        push    ecx
        lea     ecx, [ebp+FAT.buffer+0x200]
        cmp     edi, ecx
        jc      @b
        call    fat1x_root_end_write
fat1x_root_next_sector:
        push    [ebp+FAT.longname_sec2]
        pop     [ebp+FAT.longname_sec1]
        mov     ecx, [eax+4]
        push    ecx
        add     ecx, [ebp+FAT.ROOT_START]
        mov     [ebp+FAT.longname_sec2], ecx
        pop     ecx
        inc     ecx
        mov     [eax+4], ecx
        cmp     ecx, [ebp+FAT.ROOT_SECTORS]
        jnc     fat_notroot_next_err
        pop     ecx
fat1x_root_first:
        mov     eax, [eax+4]
        add     eax, [ebp+FAT.ROOT_START]
        push    ebx
        lea     edi, [ebp+FAT.buffer]
        mov     ebx, edi
        call    fs_read32_sys
        pop     ebx
        test    eax, eax
        jz      @f
        movi    eax, ERROR_DEVICE
        stc
@@:
        ret

fat1x_root_begin_write:
        push    edi eax
        call    fat1x_root_first
        pop     eax edi
        ret

fat1x_root_end_write:
        pusha
        mov     eax, [eax+4]
        add     eax, [ebp+FAT.ROOT_START]
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_sys
        popa
        ret

fat_notroot_next:
        push    ecx
        lea     ecx, [ebp+FAT.buffer+0x200-0x20]
        cmp     edi, ecx
        jae     fat_notroot_next_sector
        add     edi, 0x20
@@:
        pop     ecx
        ret

fat_notroot_next_write:
        push    ecx
        lea     ecx, [ebp+FAT.buffer+0x200]
        cmp     edi, ecx
        jc      @b
        push    eax
        call    fat_notroot_end_write
        pop     eax
fat_notroot_next_sector:
        push    [ebp+FAT.longname_sec2]
        pop     [ebp+FAT.longname_sec1]
        push    eax
        call    fat_get_sector
        mov     [ebp+FAT.longname_sec2], eax
        pop     eax
        mov     ecx, [eax+4]
        inc     ecx
        cmp     ecx, [ebp+FAT.SECTORS_PER_CLUSTER]
        jae     fat_notroot_next_cluster
        mov     [eax+4], ecx
        jmp     @f

fat_notroot_next_err:
        pop     ecx
        movi    eax, ERROR_FILE_NOT_FOUND
fat1x_root_extend_dir:
        stc
        ret

fat_notroot_next_cluster:
        push    eax
        mov     eax, [eax]
        call    get_FAT
        mov     ecx, eax
        pop     eax
        jc      fat_notroot_first.deverr
        cmp     ecx, 2
        jb      fat_notroot_next_err
        cmp     ecx, [ebp+FAT.fatRESERVED]
        jae     fat_notroot_next_err
        mov     [eax], ecx
        and     dword [eax+4], 0
@@:
        pop     ecx
fat_notroot_first:
        call    fat_get_sector
        push    ebx
        lea     edi, [ebp+FAT.buffer]
        mov     ebx, edi
        call    fs_read32_sys
        pop     ebx
        test    eax, eax
        jz      .ret ; CF=0
        push    ecx
.deverr:
        pop     ecx
        mov     eax, ERROR_DEVICE
        stc
.ret:
        ret

fat_notroot_begin_write:
        push    eax edi
        call    fat_notroot_first
        pop     edi eax
        ret

fat_notroot_end_write:
        call    fat_get_sector
        push    ebx
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_sys
        pop     ebx
        ret

fat_notroot_extend_dir.writeerr:
        pop     edx
@@:
        pop     eax
        ret

fat_notroot_extend_dir:
        push    eax
        call    get_free_FAT
        jc      @b
        push    edx
        mov     edx, [ebp+FAT.fatEND]
        call    set_FAT
        jc      .writeerr
        mov     edx, eax
        mov     eax, [esp+4]
        mov     eax, [eax]
        push    edx
        call    set_FAT
        pop     edx
        jc      .writeerr
        push    ecx
        or      ecx, -1
        call    add_disk_free_space
        mov     ecx, 512/4
        lea     edi, [ebp+FAT.buffer]
        push    edi
        xor     eax, eax
        rep stosd
        pop     edi
        pop     ecx
        mov     eax, [esp+4]
        mov     [eax], edx
        and     dword [eax+4], 0
        pop     edx
        mov     eax, [eax]
        dec     eax
        dec     eax
        push    ebx ecx
        mov     ecx, [ebp+FAT.SECTORS_PER_CLUSTER]
        imul    eax, ecx
        add     eax, [ebp+FAT.DATA_START]
        mov     ebx, edi
@@:
        push    eax
        call    fs_write32_sys
        pop     eax
        inc     eax
        loop    @b
        pop     ecx ebx eax
        clc
        ret

fat_get_sector:
        push    ecx
        mov     ecx, [eax]
        dec     ecx
        dec     ecx
        imul    ecx, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     ecx, [ebp+FAT.DATA_START]
        add     ecx, [eax+4]
        mov     eax, ecx
        pop     ecx
        ret

;----------------------------------------------------------------
fat_CreateFolder:
        mov     [ebp+FAT.createOption], 0
        jmp     @f

fat_CreateFile:
        mov     [ebp+FAT.createOption], 1
@@:
        call    fat_lock
        mov     ecx, [ebx+12]
        mov     edx, [ebx+16]
.rename:
        pushad
        xor     edi, edi
        push    esi
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, '/'
        jnz     @b
        lea     edi, [esi-1]
        jmp     @b

@@:
        pop     esi
        test    edi, edi
        jnz     .noroot
        mov     edx, [ebp+FAT.ROOT_CLUSTER]
        cmp     [ebp+FAT.fs_type], 32
        jz      .pushnotroot
        xor     edx, edx
        push    edx
        push    fat1x_root_extend_dir
        push    fat1x_root_end_write
        push    fat1x_root_next_write
        push    fat1x_root_begin_write
        push    edx
        push    edx
        push    fat1x_root_first
        push    fat1x_root_next
        jmp     .common1

.retNotFound:
        movi    eax, ERROR_FILE_NOT_FOUND
        jmp     .ret1

.noAccess:
        movi    eax, ERROR_ACCESS_DENIED
.ret1:
        mov     [esp+28], eax
        call    fat_unlock
        popad
        xor     ebx, ebx
        ret

.full:
        movi    eax, ERROR_DISK_FULL
        jmp     .ret1

.noroot:
        cmp     byte [edi+1], 0
        jz      .noAccess
; check existence
        mov     byte [edi], 0
        push    edi
        call    hd_find_lfn
        pop     esi
        mov     byte [esi], '/'
        jc      .retNotFound
        inc     esi
        test    byte [edi+11], 0x10
        jz      .noAccess   ; file
        mov     edx, [edi+20-2]
        mov     dx, [edi+26]
        movi    eax, ERROR_FS_FAIL
        cmp     edx, 2
        jb      .ret1
.pushnotroot:
        push    edx
        push    fat_notroot_extend_dir
        push    fat_notroot_end_write
        push    fat_notroot_next_write
        push    fat_notroot_begin_write
        push    0
        push    edx
        push    fat_notroot_first
        push    fat_notroot_next
.common1:
        call    fat_find_lfn
        jc      .notfound
        test    byte [edi+11], 10h
        jz      .exists_file
; found directory
        add     esp, 36
        call    fat_unlock
        popad
        xor     eax, eax
        cmp     [ebp+FAT.createOption], 0
        jz      @f
        mov     al, ERROR_ACCESS_DENIED
@@:
        xor     ebx, ebx
        ret

.exists_file:
        cmp     [ebp+FAT.createOption], 1
        jz      @f
        add     esp, 36
        jmp     .noAccess

@@: ; delete FAT chain
        push    edi
        xor     eax, eax
        mov     dword [edi+28], eax     ; zero size
        xor     ecx, ecx
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]
        mov     word [edi+20], cx
        mov     word [edi+26], cx
        test    eax, eax
        jz      .done1
@@:
        cmp     eax, [ebp+FAT.fatRESERVED]
        jae     .done1
        xor     edx, edx
        call    set_FAT
        mov     eax, edx
        jc      .done1
        inc     ecx
        jmp     @b

.short_name_found:
        pop     ecx edi esi
        call    fat_next_short_name
        jnc     .test_short_name_loop
.disk_full:
        add     esp, 12+36
        jmp     .full

.notfound:  ; generate short name
        call    fat_name_is_legal
        jc      @f
        add     esp, 36
        jmp     .retNotFound

@@:
        sub     esp, 12
        mov     edi, esp
        call    fat_gen_short_name
.test_short_name_loop:
        push    esi edi ecx
        mov     esi, edi
        lea     eax, [esp+12+12+8]
        mov     edx, [eax+24]
        mov     [eax], edx
        and     dword [eax+4], 0
        call    dword [eax-4]
        jc      .found
.test_short_name_entry:
        cmp     byte [edi+11], 0xF
        jz      .test_short_name_cont
        mov     ecx, 11
        push    esi edi
        repz cmpsb
        pop     edi esi
        jz      .short_name_found
.test_short_name_cont:
        lea     eax, [esp+12+12+8]
        call    dword [eax-8]
        jnc     .test_short_name_entry
.found:
        pop     ecx edi esi
; now find space in directory
; we need to save LFN <=> LFN is not equal to short name <=> generated name contains '~'
        mov     al, '~'
        push    ecx edi
        mov     ecx, 8
        repnz scasb
        movi    eax, 1     ; 1 entry
        jnz     .notilde
; we need ceil(strlen(esi)/13) additional entries = floor((strlen(esi)+12+13)/13) total
        xor     ecx, ecx
        push    esi
@@:
        call    utf8to16
        inc     ecx
        test    ax, ax
        jnz     @b
        pop     esi
        mov     eax, ecx
        add     eax, 12+13-1
        mov     ecx, 13
        cdq
        div     ecx
.notilde:
        push    -1
        push    -1
        push    -1
; find <eax> successive entries in directory
        xor     ecx, ecx
        push    eax
        lea     eax, [esp+16+8+12+8]
        mov     edx, [eax+24]
        mov     [eax], edx
        and     dword [eax+4], 0
        call    dword [eax-4]
        pop     eax
        jnc     .scan_dir
.fsfrfe3:
        add     esp, 12+8+12+36
        movi    eax, ERROR_DEVICE
        jmp     .ret1

.scan_dir:
        cmp     byte [edi], 0
        jz      .free
        cmp     byte [edi], 0xE5
        jz      .free
        xor     ecx, ecx
.scan_cont:
        push    eax
        lea     eax, [esp+16+8+12+8]
        call    dword [eax-8]
        mov     edx, eax
        pop     eax
        jnc     .scan_dir
        cmp     edx, ERROR_DEVICE
        jz      .fsfrfe3
        push    eax
        lea     eax, [esp+16+8+12+8]
        call    dword [eax+20]          ; extend directory
        pop     eax
        jnc     .scan_dir
        add     esp, 12+8+12+36
        jmp     .full

.free:
        test    ecx, ecx
        jnz     @f
        mov     [esp], edi
        mov     ecx, [esp+12+8+12+8]
        mov     [esp+4], ecx
        mov     ecx, [esp+12+8+12+12]
        mov     [esp+8], ecx
        xor     ecx, ecx
@@:
        inc     ecx
        cmp     ecx, eax
        jb      .scan_cont
; found!
        push    esi ecx
; If creating a directory, allocate one data cluster or fail immediately if this is impossible.
; This prevents from creating an invalid directory entry on a full disk.
        cmp     [ebp+FAT.createOption], 0
        jnz     .notFolder
        call    get_free_FAT
        jnc     @f
        add     esp, 8+12+8
        jmp     .disk_full

@@:
        mov     [esp+8+12+8+12+36+20], eax  ; store the cluster somewhere
.notFolder: ; calculate name checksum
        mov     esi, [esp+8+12]
        mov     ecx, 11
        xor     eax, eax
@@:
        ror     al, 1
        add     al, [esi]
        inc     esi
        loop    @b
        pop     ecx esi edi
        pop     dword [esp+8+12+12]
        pop     dword [esp+8+12+12]
; edi points to first entry in free chunk
        dec     ecx
        jz      .nolfn
        push    esi eax
        lea     eax, [esp+8+8+12+8]
        call    dword [eax+8]   ; begin write
        mov     al, 40h
.writelfn:
        or      al, cl
        stosb
        mov     esi, [esp+4]
        push    ecx
        dec     ecx
        jz      @f
        imul    ecx, 13
.scroll:
        call    utf8to16
        loop    .scroll
@@:
        mov     cl, 5
        call    fat_read_symbols
        mov     ax, 0xF
        stosw
        mov     al, [esp+4]
        stosb
        mov     cl, 6
        call    fat_read_symbols
        xor     eax, eax
        stosw
        mov     cl, 2
        call    fat_read_symbols
        pop     ecx
        lea     eax, [esp+8+8+12+8]
        call    dword [eax+12]  ; next write
        xor     eax, eax
        loop    .writelfn
        pop     eax esi
.nolfn:
        pop     esi
        add     esp, 16
        mov     ecx, 11
        rep movsb
        cmp     [ebp+FAT.createOption], 2
        jz      .copy
        mov     word [edi], 20h         ; attributes
        sub     edi, 11
        mov     byte [edi+13], 0        ; tenths of a second at file creation time
        call    get_time_for_file
        mov     [edi+14], ax            ; creation time
        mov     [edi+22], ax            ; last write time
        call    get_date_for_file
        mov     [edi+16], ax            ; creation date
        mov     [edi+24], ax            ; last write date
        mov     [edi+18], ax            ; last access date
        mov     word [edi+20], cx       ; high word of cluster
        mov     word [edi+26], cx       ; low word of cluster - to be filled
        mov     dword [edi+28], ecx     ; file size - to be filled
        cmp     [ebp+FAT.createOption], 0
        jnz     .doit
; create directory
        mov     byte [edi+11], 10h      ; attributes: folder
        mov     esi, edi
        mov     eax, [esp+36+20] ; extract saved cluster
        mov     [esp+36+20], edi ; this is needed for calculating arg of add_disk_free_space!
        push    ecx
        mov     ecx, [ebp+FAT.SECTORS_PER_CLUSTER]
        shl     ecx, 9
        push    ecx
        push    edi
        jmp     .doit2

.copy:
        lea     esi, [esp+72+11]
        mov     cl, 21
        rep movsb
        sub     edi, 32
        jmp     .doit

.done1:
        pop     edi
        call    get_time_for_file
        mov     [edi+22], ax
        call    get_date_for_file
        mov     [edi+24], ax
        mov     [edi+18], ax
        or      byte [edi+11], 20h      ; set 'archive' attribute
.doit:
        mov     esi, [esp+36+20]
        lea     eax, [esp+8]
        call    dword [eax+16]  ; flush directory
        push    ecx
        mov     ecx, [esp+4+36+24]
        xor     eax, eax
        test    ecx, ecx
        jz      .done
        push    ecx edi
        call    get_free_FAT
        jc      .diskfull
.doit2:
        push    eax
        mov     [edi+26], ax
        shr     eax, 16
        mov     [edi+20], ax
        lea     eax, [esp+16+8]
        call    dword [eax+16]  ; flush directory
        pop     eax
        mov     edx, [ebp+FAT.fatEND]
        call    set_FAT
.write_cluster:
        push    eax
        dec     eax
        dec     eax
        imul    eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+FAT.DATA_START]
        push    [ebp+FAT.SECTORS_PER_CLUSTER]
.write_sector:
        cmp     [ebp+FAT.createOption], 0
        jz      .writedir
        mov     ecx, 512
        cmp     dword [esp+12], ecx
        jb      .writeshort
; we can write directly from given buffer
        mov     ebx, esi
        add     esi, ecx
        jmp     .writecommon

.writedir:
        push    512
        lea     edi, [ebp+FAT.buffer]
        mov     ebx, edi
        mov     ecx, [ebp+FAT.SECTORS_PER_CLUSTER]
        shl     ecx, 9
        cmp     ecx, [esp+16]
        jnz     .writedircont
        dec     dword [esp+20]
        push    esi
        mov     ecx, 32/4
        rep movsd
        pop     esi
        mov     dword [edi-32], '.   '
        mov     dword [edi-32+4], '    '
        mov     dword [edi-32+8], '    '
        mov     byte [edi-32+11], 10h
        push    esi
        mov     ecx, 32/4
        rep movsd
        pop     esi
        mov     dword [edi-32], '..  '
        mov     dword [edi-32+4], '    '
        mov     dword [edi-32+8], '    '
        mov     byte [edi-32+11], 10h
        mov     ecx, [esp+20+36]
        cmp     ecx, [ebp+FAT.ROOT_CLUSTER]
        jnz     @f
        xor     ecx, ecx
@@:
        mov     word [edi-32+26], cx
        shr     ecx, 16
        mov     [edi-32+20], cx
        jmp     .writedircont

.writeshort:
        mov     ecx, [esp+12]
        push    ecx
        lea     edi, [ebp+FAT.buffer]
        mov     ebx, edi
        rep movsb
.writedircont:
        lea     ecx, [ebp+FAT.buffer+0x200]
        sub     ecx, edi
        push    eax
        xor     eax, eax
        rep stosb
        pop     eax
        pop     ecx
.writecommon:
        push    eax
        call    fs_write32_app
        test    eax, eax
        pop     eax
        jnz     .writeerr
        inc     eax
        sub     dword [esp+12], ecx
        jz      .writedone
        dec     dword [esp]
        jnz     .write_sector
        pop     eax
; allocate new cluster
        pop     eax
        mov     ecx, eax
        call    get_free_FAT
        jc      .diskfull
        mov     edx, [ebp+FAT.fatEND]
        call    set_FAT
        xchg    eax, ecx
        mov     edx, ecx
        call    set_FAT
        xchg    eax, ecx
        jmp     .write_cluster

.diskfull:
        mov     eax, ERROR_DISK_FULL
        jmp     .ret

.writeerr:
        pop     eax eax
        sub     esi, ecx
        mov     eax, ERROR_DEVICE
        jmp     .ret

.writedone:
        pop     eax eax
        xor     eax, eax
.ret:
        pop     edi ecx
        inc     ecx
.done:
        sub     esi, [esp+4+36+20]
        mov     [esp+4+36+28], eax
        mov     [esp+4+36+16], esi
        jecxz   @f
        lea     eax, [esp+12]
        call    dword [eax+8]
        mov     [edi+28], esi
        call    dword [eax+16]
@@:
        lea     eax, [esi+511]
        shr     eax, 9
        mov     ecx, [ebp+FAT.SECTORS_PER_CLUSTER]
        lea     eax, [eax+ecx-1]
        xor     edx, edx
        div     ecx
        pop     ecx
        sub     ecx, eax
        call    add_disk_free_space
        add     esp, 36
        cmp     [ebp+FAT.createOption], 2
        jz      @f
        call    update_disk
        call    fat_unlock
@@:
        popad
        ret

@@:
        or      eax, -1
        rep stosw
        ret

fat_read_symbols:
        test    esi, esi
        jz      @b
        call    utf8to16
        stosw
        test    ax, ax
        jnz     @f
        xor     esi, esi
@@:
        loop    fat_read_symbols
        ret

;----------------------------------------------------------------
fat_Write:
        call    fat_lock
        call    hd_find_lfn
        jc      .error
        cmp     dword [ebx+8], 0
        jnz     .eof    ; FAT does not support files larger than 4GB
        mov     ecx, [ebx+12]
        mov     edx, [ebx+16]
        mov     ebx, [ebx+4]
; now edi points to direntry, ebx=start byte to write,
; ecx=number of bytes to write, edx=data pointer
; extend file if needed
        add     ecx, ebx
        jc      .eof    ; FAT does not support files larger than 4GB
        push    edx
        push    eax     ; save directory sector
        push    0       ; return value=0
        call    get_time_for_file
        mov     [edi+22], ax            ; last write time
        call    get_date_for_file
        mov     [edi+24], ax            ; last write date
        mov     [edi+18], ax            ; last access date
        push    dword [edi+28]          ; save current file size
        cmp     ecx, [edi+28]
        jbe     .length_ok
        cmp     ecx, ebx
        jz      .length_ok
        call    hd_extend_file
        jnc     .length_ok
        mov     [esp+4], eax
; hd_extend_file can return three error codes: FAT table error, device error or disk full.
; First two cases are fatal errors, in third case we may write some data
        cmp     al, ERROR_DISK_FULL
        jnz     @f
; correct number of bytes to write
        mov     ecx, [edi+28]
        cmp     ecx, ebx
        ja      .length_ok
        push    0
.ret:
        pop     eax eax eax ecx ecx
        sub     edx, ecx
        push    eax edx
        call    update_disk
        pop     ebx
@@:
        call    fat_unlock
        pop     eax
        ret

.error:
        push    eax
        jmp     @b

.eof:
        push    ERROR_END_OF_FILE
        xor     ebx, ebx
        jmp     @b

.device_err2:
        pop     ecx
.device_err:
        mov     byte [esp+8], ERROR_DEVICE
        jmp     .ret

.fat_err:
        mov     byte [esp+8], ERROR_FS_FAIL
        jmp     .ret

.length_ok:
        mov     esi, [edi+28]
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]
        mov     edi, eax        ; edi=current cluster
        push    0               ; current sector in cluster
; save directory
        mov     eax, [esp+12]
        push    ebx
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_sys
        pop     ebx
        test    eax, eax
        jnz     .device_err
; now ebx=start pos, ecx=end pos, both lie inside file
        sub     ecx, ebx
        jz      .ret
.write_loop:
; skip unmodified sectors
        cmp     dword [esp+4], 0x200
        jb      .modify
        sub     ebx, 0x200
        jae     .skip
        add     ebx, 0x200
.modify:
; get length of data in current sector
        push    ecx
        sub     ebx, 0x200
        jb      .hasdata
        neg     ebx
        xor     ecx, ecx
        jmp     @f
.hasdata:
        neg     ebx
        cmp     ecx, ebx
        jbe     @f
        mov     ecx, ebx
@@:
; get current sector number
        mov     eax, edi
        dec     eax
        dec     eax
        imul    eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+FAT.DATA_START]
        add     eax, [esp+4]
; load sector if needed
        cmp     dword [esp+8], 0        ; we don't need to read uninitialized data
        jz      .noread
        cmp     ecx, 0x200      ; we don't need to read sector if it is fully rewritten
        jz      .noread
        cmp     ecx, esi        ; (same for the last sector)
        jz      .noread
        push    eax ebx
        lea     ebx, [ebp+FAT.buffer]
        call    fs_read32_app
        test    eax, eax
        pop     ebx eax
        jnz     .device_err2
.noread:
; zero uninitialized data if file was extended (because hd_extend_file does not this)
        push    eax ecx edi
        xor     eax, eax
        mov     ecx, 0x200
        sub     ecx, [esp+8+12]
        jbe     @f
        lea     edi, [ebp+FAT.buffer]
        add     edi, [esp+8+12]
        rep stosb
@@:
; zero uninitialized data in the last sector
        mov     ecx, 0x200
        sub     ecx, esi
        jbe     @f
        lea     edi, [ebp+FAT.buffer+esi]
        rep stosb
@@:
        pop     edi ecx
; copy new data
        mov     eax, edx
        neg     ebx
        jecxz   @f
        lea     ebx, [ebp+FAT.buffer+0x200+ebx]
        call    memmove
        xor     ebx, ebx
@@:
        pop     eax
; save sector
        push    ebx
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_app
        pop     ebx
        test    eax, eax
        jnz     .device_err2
        add     edx, ecx
        sub     [esp], ecx
        pop     ecx
        jz      .ret
.skip:
; next sector
        pop     eax
        inc     eax
        push    eax
        cmp     eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        jb      @f
        and     dword [esp], 0
        mov     eax, edi
        call    get_FAT
        mov     edi, eax
        jc      .device_err
        cmp     edi, 2
        jb      .fat_err
        cmp     edi, [ebp+FAT.fatRESERVED]
        jae     .fat_err
@@:
        sub     esi, 0x200
        jae     @f
        xor     esi, esi
@@:
        sub     dword [esp+4], 0x200
        jae     @f
        and     dword [esp+4], 0
@@:
        jmp     .write_loop

hd_extend_file.zero_size:
        xor     eax, eax
        jmp     hd_extend_file.start_extend

; extends file on hd to given size (new data area is undefined)
; in: edi->direntry, ecx=new size
; out: CF=0 => OK, eax=0
;      CF=1 => error, eax=code (ERROR_FS_FAIL or ERROR_DISK_FULL or ERROR_DEVICE)
hd_extend_file:
        push    esi
        mov     esi, [ebp+FAT.SECTORS_PER_CLUSTER]
        imul    esi, [ebp+FAT.BYTES_PER_SECTOR]
        push    ecx
; find the last cluster of file
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]
        mov     ecx, [edi+28]
        jecxz   .zero_size
.last_loop:
        sub     ecx, esi
        jbe     .last_found
        call    get_FAT
        jnc     @f
.device_err:
        pop     ecx
.device_err2:
        pop     esi
        push    ERROR_DEVICE
.ret_err:
        pop     eax
        stc
        ret
@@:
        cmp     eax, 2
        jb      .fat_err
        cmp     eax, [ebp+FAT.fatRESERVED]
        jb      .last_loop
.fat_err:
        pop     ecx esi
        push    ERROR_FS_FAIL
        jmp     .ret_err
.last_found:
        push    eax
        call    get_FAT
        jnc     @f
        pop     eax
        jmp     .device_err
@@:
        cmp     eax, [ebp+FAT.fatRESERVED]
        pop     eax
        jb      .fat_err
; set length to full number of clusters
        sub     [edi+28], ecx
.start_extend:
        pop     ecx
; now do extend
        push    edx
        mov     edx, 2          ; start scan from cluster 2
.extend_loop:
        cmp     [edi+28], ecx
        jae     .extend_done
; add new cluster
        push    eax
        call    get_free_FAT
        jc      .disk_full
        mov     edx, [ebp+FAT.fatEND]
        call    set_FAT
        mov     edx, eax
        pop     eax
        test    eax, eax
        jz      .first_cluster
        push    edx
        call    set_FAT
        pop     edx
        jmp     @f
.first_cluster:
        ror     edx, 16
        mov     [edi+20], dx
        ror     edx, 16
        mov     [edi+26], dx
@@:
        push    ecx
        mov     ecx, -1
        call    add_disk_free_space
        pop     ecx
        mov     eax, edx
        add     [edi+28], esi
        jmp     .extend_loop
.extend_done:
        mov     [edi+28], ecx
        pop     edx esi
        xor     eax, eax        ; CF=0
        ret
.device_err3:
        pop     edx
        jmp     .device_err2
.disk_full:
        pop     eax edx esi
        movi    eax, ERROR_DISK_FULL
        stc
        ret

;----------------------------------------------------------------
fat_SetFileEnd:
        call    fat_lock
        call    hd_find_lfn
        jc      .reteax
; must not be directory
        test    byte [edi+11], 10h
        jnz     .access_denied
; file size must not exceed 4 Gb
        cmp     dword [ebx+8], 0
        jnz     .endOfFile
        push    eax     ; save directory sector
; set file modification date/time to current
        call    get_time_for_file
        mov     [edi+22], ax    ; last write
        call    get_date_for_file
        mov     [edi+24], ax    ; last write
        mov     [edi+18], ax    ; last access
        mov     eax, [ebx+4]
        cmp     eax, [edi+28]
        jb      .truncate
        ja      .expand
        pop     eax
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_sys
        test    eax, eax
        jnz     .errorDevice
        push    0
        jmp     .ret

.access_denied:
        push    ERROR_ACCESS_DENIED
        jmp     .ret

.endOfFile:
        push    ERROR_END_OF_FILE
        jmp     .ret

.errorDevice:
        push    ERROR_DEVICE
        jmp     .ret

.expand:
        push    ebx ebp ecx
        push    dword [edi+28]  ; save old size
        mov     ecx, eax
        call    hd_extend_file
        push    eax             ; return code
        jnc     .expand_ok
        cmp     al, ERROR_DISK_FULL
        jnz     .pop_ret
.expand_ok: ; save directory
        mov     eax, [edi+28]
        xchg    eax, [esp+20]
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_sys
        test    eax, eax
        jnz     .pop_ret11
        mov     eax, [esp+20]
        sub     eax, [esp+4]
        cmp     eax, 1000001h
        jnc     .pop_ret
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]
        mov     edi, eax
        test    edi, edi
        jz      .pop_ret
; now zero new data
        push    0
; edi=current cluster, [esp]=sector in cluster
; [esp+24]=new size, [esp+8]=old size, [esp+4]=return code
.zero_loop:
        cmp     edi, 2
        jb      .error_fat
        cmp     edi, [ebp+FAT.fatRESERVED]
        jae     .error_fat
        sub     dword [esp+8], 0x200
        jae     .next_cluster
        lea     eax, [edi-2]
        imul    eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+FAT.DATA_START]
        add     eax, [esp]
        cmp     dword [esp+8], -0x200
        jz      .noread
        push    eax
        lea     ebx, [ebp+FAT.buffer]
        call    fs_read32_app
        test    eax, eax
        pop     eax
        jnz     .err_next
.noread:
        mov     ecx, [esp+8]
        neg     ecx
        push    edi
        lea     edi, [ebp+FAT.buffer+0x200]
        add     edi, [esp+12]
        push    eax
        xor     eax, eax
        mov     [esp+16], eax
        rep stosb
        pop     eax
        pop     edi
        call    fs_write32_app
        test    eax, eax
        jz      .next_cluster
.err_next:
        mov     byte [esp+4], ERROR_DEVICE
.next_cluster:
        pop     eax
        sub     dword [esp+20], 0x200
        jbe     .pop_ret
        inc     eax
        push    eax
        cmp     eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        jb      .zero_loop
        and     dword [esp], 0
        mov     eax, edi
        call    get_FAT
        mov     edi, eax
        jnc     .zero_loop
        pop     eax
.pop_ret11:
        mov     byte [esp], ERROR_DEVICE
.pop_ret:
        call    update_disk
        pop     eax ecx ecx ebp ebx ecx
.reteax:
        push    eax
.ret:
        call    fat_unlock
        pop     eax
        ret

.error_fat:
        pop     eax
        mov     byte [esp], ERROR_FS_FAIL
        jmp     .pop_ret

.error_fat2:
        pop     eax ecx eax
        call    update_disk
        push    ERROR_FS_FAIL
        jmp     .ret

.truncate:
        mov     [edi+28], eax
        push    ecx
        mov     ecx, [edi+20-2]
        mov     cx, [edi+26]
        push    eax
        test    eax, eax
        jz      .zero_size
@@: ; find new last cluster
        cmp     ecx, 2
        jb      .error_fat2
        cmp     ecx, [ebp+FAT.fatRESERVED]
        jae     .error_fat2
        mov     eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        shl     eax, 9
        sub     [esp], eax
        jbe     @f
        mov     eax, ecx
        call    get_FAT
        mov     ecx, eax
        jnc     @b
.device_err3:
        pop     eax ecx eax
        call    update_disk
        push    ERROR_DEVICE
        jmp     .ret

@@:
; we will zero data at the end of last sector - remember it
        push    ecx
; terminate FAT chain
        push    edx
        mov     eax, ecx
        mov     edx, [ebp+FAT.fatEND]
        call    set_FAT
        mov     eax, edx
        pop     edx
        jnc     @f
.device_err4:
        pop     ecx
        jmp     .device_err3

.zero_size:
        and     word [edi+20], 0
        and     word [edi+26], 0
        push    0
        mov     eax, ecx
@@:
; delete FAT chain
        call    clear_cluster_chain
        jc      .device_err4
; save directory
        mov     eax, [esp+12]
        push    ebx
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_sys
        pop     ebx
        test    eax, eax
        jnz     .device_err4
; zero last sector, ignore errors
        pop     ecx
        pop     eax
        dec     ecx
        imul    ecx, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     ecx, [ebp+FAT.DATA_START]
        push    eax
        sar     eax, 9
        add     ecx, eax
        pop     eax
        and     eax, 0x1FF
        jz      .truncate_done
        push    ebx eax
        mov     eax, ecx
        lea     ebx, [ebp+FAT.buffer]
        call    fs_read32_app
        pop     eax
        lea     edi, [ebp+FAT.buffer+eax]
        push    ecx
        mov     ecx, 0x200
        sub     ecx, eax
        xor     eax, eax
        rep stosb
        pop     eax
        call    fs_write32_app
        pop     ebx
.truncate_done:
        pop     ecx eax
        call    update_disk
        call    fat_unlock
        xor     eax, eax
        ret

;----------------------------------------------------------------
fat_GetFileInfo:
        cmp     byte [esi], 0
        jz      .volume
        call    fat_lock
        call    hd_find_lfn
        jc      @f
        push    ebp
        xor     ebp, ebp
        mov     esi, [ebx+16]
        mov     dword [esi+4], ebp
        call    fat_entry_to_bdfe2
        pop     ebp
        xor     eax, eax
@@:
        push    eax
        call    fat_unlock
        pop     eax
@@:
        ret

.volume:
        mov     eax, dword[ebp+FAT.Length]
        mov     edx, dword[ebp+FAT.Length+4]
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
        lea     esi, [ebp+FAT.volumeLabel]
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

;----------------------------------------------------------------
fat_SetFileInfo:
        call    fat_lock
        call    hd_find_lfn
        jc      @f
        push    eax
        mov     edx, [ebx+16]
        call    bdfe_to_fat_entry
        pop     eax
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_sys
        call    update_disk
        xor     eax, eax
@@:
        push    eax
        call    fat_unlock
        pop     eax
        ret

;----------------------------------------------------------------
fat_Delete:
        call    fat_lock
        and     [ebp+FAT.longname_sec1], 0
        and     [ebp+FAT.longname_sec2], 0
        call    hd_find_lfn
        jc      .notFound
        cmp     dword [edi], '.   '
        jz      .access_denied2
        cmp     dword [edi], '..  '
        jz      .access_denied2
        test    byte [edi+11], 10h
        jz      .dodel
; we can delete only empty folders!
        pushad
        mov     esi, [edi+20-2]
        mov     si, [edi+26]
        xor     ecx, ecx
        lea     eax, [esi-2]
        imul    eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+FAT.DATA_START]
        lea     ebx, [ebp+FAT.buffer]
        call    fs_read32_sys
        test    eax, eax
        jnz     .err1
        lea     eax, [ebx+0x200]
        add     ebx, 2*0x20
.checkempty:
        cmp     byte [ebx], 0
        jz      .empty
        cmp     byte [ebx], 0xE5
        jnz     .notempty
        add     ebx, 0x20
        cmp     ebx, eax
        jb      .checkempty
        inc     ecx
        cmp     ecx, [ebp+FAT.SECTORS_PER_CLUSTER]
        jb      @f
        mov     eax, esi
        call    get_FAT
        jc      .err1
        cmp     eax, 2
        jb      .error_fat
        cmp     eax, [ebp+FAT.fatRESERVED]
        jae     .empty
        mov     esi, eax
        xor     ecx, ecx
@@:
        lea     eax, [esi-2]
        imul    eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+FAT.DATA_START]
        add     eax, ecx
        lea     ebx, [ebp+FAT.buffer]
        call    fs_read32_sys
        test    eax, eax
        lea     eax, [ebx+0x200]
        jz      .checkempty
.err1:
        popad
.err2:
        push    ERROR_DEVICE
.ret:
        call    fat_unlock
        pop     eax
        ret

.notFound:
        push    ERROR_FILE_NOT_FOUND
        jmp     .ret

.error_fat:
        popad
        push    ERROR_FS_FAIL
        jmp     .ret

.notempty:
        popad
.access_denied2:
        push    ERROR_ACCESS_DENIED
        jmp     .ret

.empty:
        popad
        push    eax ebx
        lea     ebx, [ebp+FAT.buffer]
        call    fs_read32_sys
        test    eax, eax
        pop     ebx eax
        jnz     .err2
.dodel:
        push    eax
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]
        xchg    eax, [esp]
; delete folder entry
        mov     byte [edi], 0xE5
; delete LFN (if present)
.lfndel:
        lea     edx, [ebp+FAT.buffer]
        cmp     edi, edx
        ja      @f
        cmp     [ebp+FAT.longname_sec2], 0
        jz      .lfndone
        push    [ebp+FAT.longname_sec2]
        push    [ebp+FAT.longname_sec1]
        pop     [ebp+FAT.longname_sec2]
        and     [ebp+FAT.longname_sec1], 0
        push    ebx
        mov     ebx, edx
        call    fs_write32_sys
        mov     eax, [esp+4]
        call    fs_read32_sys
        pop     ebx
        pop     eax
        lea     edi, [ebp+FAT.buffer+0x200]
@@:
        sub     edi, 0x20
        cmp     byte [edi], 0xE5
        jz      .lfndone
        cmp     byte [edi+11], 0xF
        jnz     .lfndone
        mov     byte [edi], 0xE5
        jmp     .lfndel
.lfndone:
        push    ebx
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_sys
        pop     ebx
; delete FAT chain
        pop     eax
        call    clear_cluster_chain
        call    update_disk
        call    fat_unlock
        xor     eax, eax
        ret

;----------------------------------------------------------------
fat_Rename:
; in: edi -> new path string in UTF-8
        push    esi edi
        call    fat_lock
        call    hd_find_lfn
        pop     ebx
        jc      .error
        sub     esp, 32
        mov     esi, edi
        mov     edi, esp
        mov     ecx, 8
        rep movsd
        mov     [ebp+FAT.createOption], 2
        mov     esi, ebx
        call    fat_CreateFile.rename
        add     esp, 32
        pop     esi
        test    eax, eax
        jnz     .ret
        push    eax
        mov     [ebp+FAT.longname_sec1], eax
        mov     [ebp+FAT.longname_sec2], eax
        call    hd_find_lfn
        jc      .error
        mov     byte [edi], 0xE5
        jmp     fat_Delete.lfndel

.error:
        push    eax
        call    fat_unlock
        pop     eax ebx
.ret:
        ret
