;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  FAT32.INC                                                      ;;
;;                                                                 ;;
;;  FAT16/32 functions for KolibriOS                               ;;
;;                                                                 ;;
;;  Copyright 2002 Paolo Minazzi, paolo.minazzi@inwind.it          ;;
;;                                                                 ;;
;;  See file COPYING for details                                   ;;
;;  04.02.2007 LFN create folder - diamond                         ;;
;;  08.10.2006 LFN delete file/folder - diamond                    ;;
;;  20.08.2006 LFN set file size (truncate/extend) - diamond       ;;
;;  17.08.2006 LFN write/append to file - diamond                  ;;
;;  23.06.2006 LFN start application - diamond                     ;;
;;  15.06.2006 LFN get/set file/folder info - diamond              ;;
;;  27.05.2006 LFN create/rewrite file - diamond                   ;;
;;  04.05.2006 LFN read folder - diamond                           ;;
;;  29.04.2006 Elimination of hangup after the                     ;;
;;             expiration hd_wait_timeout -  Mario79               ;;
;;  23.04.2006 LFN read file - diamond                             ;;
;;  28.01.2006 find all Fat16/32 partition in all input point      ;;
;;             to MBR, see file part_set.inc - Mario79             ;;
;;  15.01.2005 get file size/attr/date, file_append - ATV          ;;
;;  04.12.2004 skip volume label, file delete bug fixed - ATV      ;;
;;  29.11.2004 get_free_FAT changed, append dir bug fixed - ATV    ;;
;;  23.11.2004 don't allow overwrite dir with file - ATV           ;;
;;  18.11.2004 get_disk_info and more error codes - ATV            ;;
;;  17.11.2004 set_FAT/get_FAT and disk cache rewritten - ATV      ;;
;;  10.11.2004 removedir clear whole directory structure - ATV     ;;
;;  08.11.2004 rename - ATV                                        ;;
;;  30.10.2004 file_read return also dirsize in bytes - ATV        ;;
;;  20.10.2004 Makedir/Removedir - ATV                             ;;
;;  14.10.2004 Partition chain/Fat16 - ATV (thanks drh3xx)         ;;
;;  06.9.2004  Fix free space by Mario79 added - MH                ;;
;;  24.5.2004  Write back buffer for File_write -VT                ;;
;;  20.5.2004  File_read function to work with syscall 58 - VT     ;;
;;  30.3.2004  Error parameters at function return - VT            ;;
;;  01.5.2002  Bugfix in device write - VT                         ;;
;;  20.5.2002  Hd status check - VT                                ;;
;;  29.6.2002  Improved fat32 verification - VT                    ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision $


cache_max equ 1919      ; max. is 1919*512+0x610000=0x6ffe00

PUSHAD_EAX equ [esp+28]
PUSHAD_ECX equ [esp+24]
PUSHAD_EDX equ [esp+20]
PUSHAD_EBX equ [esp+16]
PUSHAD_EBP equ [esp+8]
PUSHAD_ESI equ [esp+4]
PUSHAD_EDI equ [esp+0]

; Internal data for every FAT partition.
struct FAT
p       PARTITION       ; must be the first item
fs_type              db ?
fat16_root           db 0       ; flag for fat16 rootdir
fat_change           db 0       ; 1=fat has changed
                     db ?       ; alignment
Lock                 MUTEX ?    ; currently operations with one partition
                                 ; can not be executed in parallel since the
                                 ; legacy code is not ready; this mutex guards
                                 ; all operations
SECTORS_PER_FAT      dd 0x1f3a
NUMBER_OF_FATS       dd 0x2
SECTORS_PER_CLUSTER  dd 0x8
BYTES_PER_SECTOR     dd 0x200   ; Note: if BPS <> 512 need lots of changes
ROOT_CLUSTER         dd 2       ; first rootdir cluster
FAT_START            dd 0       ; start of fat table
ROOT_START           dd 0       ; start of rootdir (only fat16)
ROOT_SECTORS         dd 0       ; count of rootdir sectors (only fat16)
DATA_START           dd 0       ; start of data area (=first cluster 2)
LAST_CLUSTER         dd 0       ; last availabe cluster
ADR_FSINFO           dd 0       ; used only by fat32

fatRESERVED          dd 0x0FFFFFF6
fatBAD               dd 0x0FFFFFF7
fatEND               dd 0x0FFFFFF8
fatMASK              dd 0x0FFFFFFF

fatStartScan         dd 2

cluster_tmp          dd 0       ; used by analyze_directory
                                ; and analyze_directory_to_write

longname_sec1        dd 0       ; used by analyze_directory to save 2 previous
longname_sec2        dd 0       ; directory sectors for delete long filename

fat_in_cache         dd -1

fat_cache            rb 512
buffer               rb 512
fsinfo_buffer        rb 512
ends

uglobal
align 4
partition_count      dd 0       ; partitions found by set_FAT32_variables

hd_error             dd 0       ; set by wait_for_sector_buffer
hd_setup             dd 0
hd_wait_timeout      dd 0

cache_search_start   dd 0       ; used by find_empty_slot
endg

uglobal
align 4
 Sector512:                      ; label for dev_hdcd.inc
  buffer:
                times 512 db 0
endg

iglobal
align 4
fat_user_functions:
        dd      (fat_user_functions_end - fat_user_functions - 4) / 4
        dd      fat_Read
        dd      fat_ReadFolder
        dd      fat_Rewrite
        dd      fat_Write
        dd      fat_SetFileEnd
        dd      fat_GetFileInfo
        dd      fat_SetFileInfo
        dd      0
        dd      fat_Delete
        dd      fat_CreateFolder
fat_user_functions_end:
endg

; these labels are located before the main function to make
; most of jumps to these be short
fat_create_partition.free_return0:
        push    ebx
        mov     eax, ebp
        call    free
        pop     ebx
        pop     ebp
fat_create_partition.return0:
        xor     eax, eax
        ret
fat_create_partition:
; bootsector must have been successfully read
        cmp     dword [esp+4], 1
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
        push    ebx
        push    sizeof.FAT
        pop     eax
        call    malloc
        pop     ebx
        test    eax, eax
        jz      .return0
        mov     ecx, [ebp+8]
        mov     dword [eax+FAT.p.FirstSector], ecx
        mov     ecx, [ebp+12]
        mov     dword [eax+FAT.p.FirstSector+4], ecx
        mov     ecx, [ebp+16]
        mov     dword [eax+FAT.p.Length], ecx
        mov     ecx, [ebp+20]
        mov     dword [eax+FAT.p.Length+4], ecx
        mov     [eax+FAT.p.Disk], esi
        mov     [eax+FAT.p.FSUserFunctions], fat_user_functions
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
        imul    eax, [ebp+FAT.SECTORS_PER_FAT]
        add     eax, [ebp+FAT.FAT_START]
        mov     [ebp+FAT.ROOT_START], eax       ; rootdir = fat_start + fat_size * fat_count
        add     eax, [ebp+FAT.ROOT_SECTORS]     ; rootdir sectors should be 0 on fat32
        mov     [ebp+FAT.DATA_START], eax       ; data area = rootdir + rootdir_size

        movzx   eax, word [ebx+0x13]    ; total sector count <65536
        test    eax, eax
        jnz     @f
        mov     eax, [ebx+0x20]         ; total sector count
@@:
        mov     dword [ebp+FAT.p.Length], eax
        and     dword [ebp+FAT.p.Length+4], 0
        sub     eax, [ebp+FAT.DATA_START]       ; eax = count of data sectors
        xor     edx, edx
        div     [ebp+FAT.SECTORS_PER_CLUSTER]
        inc     eax
        mov     [ebp+FAT.LAST_CLUSTER], eax
        dec     eax                     ; cluster count
        mov     [ebp+FAT.fatStartScan], 2

        ; limits by Microsoft Hardware White Paper v1.03
        cmp     eax, 4085               ; 0xff5
        jb      .free_return0           ; fat12 not supported
        cmp     eax, 65525              ; 0xfff5
        jb      .fat16
.fat32:
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
        mov     [fs_type], al
        mov     [ebp+FAT.fs_type], al
        mov     eax, ebp
        pop     ebp
        ret
.fat16:
        and     [ebp+FAT.ROOT_CLUSTER], 0
        mov     [ebp+FAT.fatRESERVED], 0x0000FFF6
        mov     [ebp+FAT.fatBAD], 0x0000FFF7
        mov     [ebp+FAT.fatEND], 0x0000FFF8
        mov     [ebp+FAT.fatMASK], 0x0000FFFF
        mov     al, 16
        mov     [fs_type], al
        mov     [ebp+FAT.fs_type], al
        mov     eax, ebp
        pop     ebp
        ret

set_FAT:
;--------------------------------
; input  : EAX = cluster
;          EDX = value to save
;          EBP = pointer to FAT structure
; output : EDX = old value
;--------------------------------
; out: CF set <=> error
        push    eax ebx esi

        cmp     eax, 2
        jb      sfc_error
        cmp     eax, [ebp+FAT.LAST_CLUSTER]
        ja      sfc_error
        cmp     [ebp+FAT.fs_type], 16
        je      sfc_1
        add     eax, eax
  sfc_1:
        add     eax, eax
        mov     esi, 511
        and     esi, eax        ; esi = position in fat sector
        shr     eax, 9          ; eax = fat sector
        add     eax, [ebp+FAT.FAT_START]
        lea     ebx, [ebp+FAT.fat_cache]

        cmp     eax, [ebp+FAT.fat_in_cache]; is fat sector already in memory?
        je      sfc_in_cache    ; yes

        cmp     [ebp+FAT.fat_change], 0; is fat changed?
        je      sfc_no_change   ; no
        call    write_fat_sector; yes. write it into disk
        jc      sfc_error

  sfc_no_change:
        mov     [ebp+FAT.fat_in_cache], eax; save fat sector
        call    fs_read32_sys
        test    eax, eax
        jne     sfc_error


  sfc_in_cache:
        cmp     [ebp+FAT.fs_type], 16
        jne     sfc_test32

  sfc_set16:
        xchg    [ebx+esi], dx   ; save new value and get old value
        jmp     sfc_write

  sfc_test32:
        mov     eax, [ebp+FAT.fatMASK]

  sfc_set32:
        and     edx, eax
        xor     eax, -1         ; mask for high bits
        and     eax, [ebx+esi]  ; get high 4 bits
        or      eax, edx
        mov     edx, [ebx+esi]  ; get old value
        mov     [ebx+esi], eax  ; save new value

  sfc_write:
        mov     [ebp+FAT.fat_change], 1; fat has changed

  sfc_nonzero:
        and     edx, [ebp+FAT.fatMASK]

  sfc_return:
        pop     esi ebx eax
        ret
  sfc_error:
        stc
        jmp     sfc_return


get_FAT:
;--------------------------------
; input  : EAX = cluster
;          EBP = pointer to FAT structure
; output : EAX = next cluster
;--------------------------------
; out: CF set <=> error
        push    ebx esi

        cmp     [ebp+FAT.fs_type], 16
        je      gfc_1
        add     eax, eax
  gfc_1:
        add     eax, eax
        mov     esi, 511
        and     esi, eax        ; esi = position in fat sector
        shr     eax, 9          ; eax = fat sector
        add     eax, [ebp+FAT.FAT_START]
        lea     ebx, [ebp+FAT.fat_cache]

        cmp     eax, [ebp+FAT.fat_in_cache]; is fat sector already in memory?
        je      gfc_in_cache

        cmp     [ebp+FAT.fat_change], 0; is fat changed?
        je      gfc_no_change   ; no
        call    write_fat_sector; yes. write it into disk
        jc      hd_error_01

  gfc_no_change:
        mov     [ebp+FAT.fat_in_cache], eax
        call    fs_read32_sys
        test    eax, eax
        jne     hd_error_01

  gfc_in_cache:
        mov     eax, [ebx+esi]
        and     eax, [ebp+FAT.fatMASK]
  gfc_return:
        pop     esi ebx
        ret
 hd_error_01:
        stc
        jmp     gfc_return


get_free_FAT:
;-----------------------------------------------------------
; output : if CARRY=0 EAX = # first cluster found free
;          if CARRY=1 disk full
; Note   : for more speed need to use fat_cache directly
;-----------------------------------------------------------
        push    ecx
        mov     ecx, [ebp+FAT.LAST_CLUSTER]; counter for full disk
        sub     ecx, 2
        mov     eax, [ebp+FAT.fatStartScan]
        cmp     eax, 2
        jb      gff_reset

  gff_test:
        cmp     eax, [ebp+FAT.LAST_CLUSTER]; if above last cluster start at cluster 2
        jbe     gff_in_range
  gff_reset:
        mov     eax, 2

  gff_in_range:
        push    eax
        call    get_FAT         ; get cluster state
        jc      gff_not_found_1

        test    eax, eax        ; is it free?
        pop     eax
        je      gff_found       ; yes
        inc     eax             ; next cluster
        dec     ecx             ; is all checked?
        jns     gff_test        ; no

  gff_not_found:
        pop     ecx             ; yes. disk is full
        stc
        ret

  gff_not_found_1:
        pop     eax
        jmp     gff_not_found

  gff_found:
        lea     ecx, [eax+1]
        mov     [ebp+FAT.fatStartScan], ecx
        pop     ecx
        clc
        ret


write_fat_sector:
;-----------------------------------------------------------
; write changed fat to disk
;-----------------------------------------------------------
        push    eax ebx ecx

        mov     [ebp+FAT.fat_change], 0
        mov     eax, [ebp+FAT.fat_in_cache]
        cmp     eax, -1
        jz      write_fat_not_used
        lea     ebx, [ebp+FAT.fat_cache]
        mov     ecx, [ebp+FAT.NUMBER_OF_FATS]

  write_next_fat:
        push    eax
        call    fs_write32_sys
        test    eax, eax
        pop     eax
        jnz     write_fat_not_used

        add     eax, [ebp+FAT.SECTORS_PER_FAT]
        dec     ecx
        jnz     write_next_fat

  write_fat_not_used:
        pop     ecx ebx eax
        ret





bcd2bin:
;----------------------------------
; input  : AL=BCD number (eg. 0x11)
; output : AH=0
;          AL=decimal number (eg. 11)
;----------------------------------
        xor     ah, ah
        shl     ax, 4
        shr     al, 4
        aad
        ret


get_date_for_file:
;-----------------------------------------------------
; Get date from CMOS and pack day,month,year in AX
; DATE   bits  0..4   : day of month 0..31
;              5..8   : month of year 1..12
;              9..15  : count of years from 1980
;-----------------------------------------------------
        mov     al, 0x7 ;day
        out     0x70, al
        in      al, 0x71
        call    bcd2bin
        ror     eax, 5

        mov     al, 0x8 ;month
        out     0x70, al
        in      al, 0x71
        call    bcd2bin
        ror     eax, 4

        mov     al, 0x9 ;year
        out     0x70, al
        in      al, 0x71
        call    bcd2bin
        add     ax, 20  ;because CMOS return only the two last
                        ;digit (eg. 2000 -> 00 , 2001 -> 01) and we
        rol     eax, 9  ;need the difference with 1980 (eg. 2001-1980)
        ret


get_time_for_file:
;-----------------------------------------------------
; Get time from CMOS and pack hour,minute,second in AX
; TIME   bits  0..4   : second (the low bit is lost)
;              5..10  : minute 0..59
;              11..15 : hour 0..23
;-----------------------------------------------------
        mov     al, 0x0 ;second
        out     0x70, al
        in      al, 0x71
        call    bcd2bin
        ror     eax, 6

        mov     al, 0x2 ;minute
        out     0x70, al
        in      al, 0x71
        call    bcd2bin
        ror     eax, 6

        mov     al, 0x4 ;hour
        out     0x70, al
        in      al, 0x71
        call    bcd2bin
        rol     eax, 11
        ret


set_current_time_for_entry:
;-----------------------------------------------------
; Set current time/date for file entry
; input  : ebx = file entry pointer
;-----------------------------------------------------
        push    eax
        call    get_time_for_file; update files date/time
        mov     [ebx+22], ax
        call    get_date_for_file
        mov     [ebx+24], ax
        pop     eax
        ret



add_disk_free_space:
;-----------------------------------------------------
; input  : ecx = cluster count
; Note   : negative = remove clusters from free space
;          positive = add clusters to free space
;-----------------------------------------------------
        test    ecx, ecx        ; no change
        je      add_dfs_no
        cmp     [ebp+FAT.fs_type], 32  ; free disk space only used by fat32
        jne     add_dfs_no

        push    eax ebx
        mov     eax, [ebp+FAT.ADR_FSINFO]
        lea     ebx, [ebp+FAT.fsinfo_buffer]
        call    fs_read32_sys
        test    eax, eax
        jnz     add_not_fs

        cmp     dword [ebx+0x1fc], 0xaa550000; check sector id
        jne     add_not_fs

        add     [ebx+0x1e8], ecx
        push    [ebp+FAT.fatStartScan]
        pop     dword [ebx+0x1ec]
        mov     eax, [ebp+FAT.ADR_FSINFO]
        call    fs_write32_sys
;    jc    add_not_fs

  add_not_fs:
        pop     ebx eax

  add_dfs_no:
        ret



clear_cluster_chain:
;-----------------------------------------------------
; input  : eax = first cluster
;-----------------------------------------------------
        push    eax ecx edx
        xor     ecx, ecx        ; cluster count

  clean_new_chain:
        cmp     eax, [ebp+FAT.LAST_CLUSTER]; end of file
        ja      delete_OK
        cmp     eax, 2          ; unfinished fat chain or zero length file
        jb      delete_OK
        cmp     eax, [ebp+FAT.ROOT_CLUSTER]; don't remove root cluster
        jz      delete_OK

        xor     edx, edx
        call    set_FAT         ; clear fat entry
        jc      access_denied_01

        inc     ecx             ; update cluster count
        mov     eax, edx        ; old cluster
        jmp     clean_new_chain

  delete_OK:
        call    add_disk_free_space; add clusters to free disk space
        clc
  access_denied_01:
        pop     edx ecx eax
        ret


if 0
get_hd_info:
;-----------------------------------------------------------
; output : eax = 0 - ok
;                3 - unknown FS
;               10 - access denied
;          edx = cluster size in bytes
;          ebx = total clusters on disk
;          ecx = free clusters on disk
;-----------------------------------------------------------
        cmp     [ebp+FAT.fs_type], 16
        jz      info_fat_ok
        cmp     [ebp+FAT.fs_type], 32
        jz      info_fat_ok
        xor     edx, edx
        xor     ebx, ebx
        xor     ecx, ecx
        mov     eax, ERROR_UNKNOWN_FS
        ret

  info_fat_ok:
;    call  reserve_hd1

        xor     ecx, ecx        ; count of free clusters
        mov     eax, 2
        mov     ebx, [ebp+FAT.LAST_CLUSTER]

  info_cluster:
        push    eax
        call    get_FAT         ; get cluster info
        jc      info_access_denied

        test    eax, eax        ; is it free?
        jnz     info_used       ; no
        inc     ecx

  info_used:
        pop     eax
        inc     eax
        cmp     eax, ebx        ; is above last cluster?
        jbe     info_cluster    ; no. test next cluster

        dec     ebx             ; cluster count
        imul    edx, [ebp+FAT.SECTORS_PER_CLUSTER], 512; cluster size in bytes
        xor     eax, eax
        ret

  info_access_denied:
        add     esp, 4
        xor     edx, edx
        xor     ebx, ebx
        xor     ecx, ecx
        mov     eax, ERROR_ACCESS_DENIED
        ret
end if

update_disk:
;-----------------------------------------------------------
; write changed fat and cache to disk
;-----------------------------------------------------------
        cmp     [ebp+FAT.fat_change], 0 ; is fat changed?
        je      upd_no_change

        call    write_fat_sector
        jc      update_disk_acces_denied

  upd_no_change:

        push    esi
        mov     esi, [ebp+PARTITION.Disk]
        call    disk_sync
        pop     esi
  update_disk_acces_denied:
        ret

fat_lock:
        lea     ecx, [ebp+FAT.Lock]
        jmp     mutex_lock
fat_unlock:
        lea     ecx, [ebp+FAT.Lock]
        jmp     mutex_unlock

; \begin{diamond}
hd_find_lfn:
; in: ebp -> FAT structure
; in: esi+[esp+4] -> name
; out: CF=1 - file not found, eax=error code
;      else CF=0 and edi->direntry, eax=sector
; destroys eax
        push    esi edi
        push    0
        push    0
        push    fat16_root_first
        push    fat16_root_next
        mov     eax, [ebp+FAT.ROOT_CLUSTER]
        cmp     [ebp+FAT.fs_type], 32
        jz      .fat32
.loop:
        call    fat_find_lfn
        jc      .notfound
        cmp     byte [esi], 0
        jz      .found
.continue:
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
        ret     4
.found:
        lea     eax, [esp+4+24]
        cmp     dword [eax], 0
        jz      @f
        mov     esi, [eax]
        and     dword [eax], 0
        jmp     .continue
@@:
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
        ret     4

;----------------------------------------------------------------
;
;  fs_HdRead - LFN variant for reading hard disk
;
;  Obsolete, will be replaced with filesystem-specific functions.
;
;  esi  points to filename
;  ebx  pointer to 64-bit number = first wanted byte, 0+
;       may be ebx=0 - start from first byte
;  ecx  number of bytes to read, 0+
;  edx  mem location to return data
;
;  ret ebx = bytes read or 0xffffffff file not found
;      eax = 0 ok read or other = errormsg
;
;--------------------------------------------------------------
fs_HdRead:
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        cmp     [fs_type], 1
        jz      ntfs_HdRead
        cmp     [fs_type], 2
        jz      ext2_HdRead
        or      ebx, -1
        mov     eax, ERROR_UNKNOWN_FS
        ret
@@:
        sub     ebx, 4
        push    ebp
        mov     ebp, [fs_dependent_data_start.partition]
        call    fat_Read
        pop     ebp
        ret

;----------------------------------------------------------------
; fat_Read - FAT16/32 implementation of reading a file
; in:  ebp = pointer to FAT structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
fat_Read:
        call    fat_lock
        push    edi
        cmp     byte [esi], 0
        jnz     @f
.noaccess:
        pop     edi
.noaccess_2:
        call    fat_unlock
        or      ebx, -1
        mov     eax, ERROR_ACCESS_DENIED
        ret

@@:
        stdcall hd_find_lfn, [esp+4+4]
        jnc     .found
        pop     edi
        push    eax
        call    fat_unlock
        pop     eax
        or      ebx, -1
        ret

.found:
        test    byte [edi+11], 0x10; do not allow read directories
        jnz     .noaccess
        cmp     dword [ebx+8], 0
        jz      @f
        xor     ebx, ebx
.reteof:
        call    fat_unlock
        mov     eax, ERROR_END_OF_FILE
        pop     edi
        ret
@@:
        mov     ecx, [ebx+12]   ; size
        mov     edx, [ebx+16]   ; pointer
        mov     ebx, [ebx+4]    ; file offset
        push    edx
        push    0
        mov     eax, [edi+28]
        sub     eax, ebx
        jb      .eof
        cmp     eax, ecx
        jae     @f
        mov     ecx, eax
        mov     byte [esp], 6
@@:
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]
; now eax=cluster, ebx=position, ecx=count, edx=buffer for data
.new_cluster:
        jecxz   .new_sector
        cmp     eax, 2
        jb      .eof
        cmp     eax, [ebp+FAT.fatRESERVED]
        jae     .eof
        mov     [ebp+FAT.cluster_tmp], eax
        dec     eax
        dec     eax
        mov     edi, [ebp+FAT.SECTORS_PER_CLUSTER]
        imul    eax, edi
        add     eax, [ebp+FAT.DATA_START]
.new_sector:
        test    ecx, ecx
        jz      .done
        sub     ebx, 512
        jae     .skip
        add     ebx, 512
        jnz     .force_buf
        cmp     ecx, 512
        jb      .force_buf
; we may read directly to given buffer
        push    eax ebx
        mov     ebx, edx
        call    fs_read32_app
        test    eax, eax
        pop     ebx eax
        jne     .noaccess_1
        add     edx, 512
        sub     ecx, 512
        jmp     .skip
.force_buf:
; we must read sector to temporary buffer and then copy it to destination
        push    eax ebx
        lea     ebx, [ebp+FAT.buffer]
        call    fs_read32_app
        test    eax, eax
        mov     eax, ebx
        pop     ebx
        jne     .noaccess_3
        add     eax, ebx
        push    ecx
        add     ecx, ebx
        cmp     ecx, 512
        jbe     @f
        mov     ecx, 512
@@:
        sub     ecx, ebx
        mov     ebx, edx
        call    memmove
        add     edx, ecx
        sub     [esp], ecx
        pop     ecx
        pop     eax
        xor     ebx, ebx
.skip:
        inc     eax
        dec     edi
        jnz     .new_sector
        mov     eax, [ebp+FAT.cluster_tmp]
        call    get_FAT
        jc      .noaccess_1

        jmp     .new_cluster
.noaccess_3:
        pop     eax
.noaccess_1:
        pop     eax
        push    ERROR_DEVICE
.done:
        mov     ebx, edx
        call    fat_unlock
        pop     eax edx edi
        sub     ebx, edx
        ret
.eof:
        mov     ebx, edx
        pop     eax edx
        sub     ebx, edx
        jmp     .reteof

;----------------------------------------------------------------
;
;  fs_HdReadFolder - LFN variant for reading hard disk folder
;
;  Obsolete, will be replaced with filesystem-specific functions.
;
;  esi  points to filename
;  ebx  pointer to structure 32-bit number = first wanted block, 0+
;                          & flags (bitfields)
; flags: bit 0: 0=ANSI names, 1=UNICODE names
;  ecx  number of blocks to read, 0+
;  edx  mem location to return data
;
;  ret ebx = blocks read or 0xffffffff folder not found
;      eax = 0 ok read or other = errormsg
;
;--------------------------------------------------------------
fs_HdReadFolder:
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        cmp     [fs_type], 1
        jz      ntfs_HdReadFolder
        cmp     [fs_type], 2
        jz      ext2_HdReadFolder
        push    ERROR_UNSUPPORTED_FS
        pop     eax
        or      ebx, -1
        ret
@@:
        sub     ebx, 4
        push    ebp
        mov     ebp, [fs_dependent_data_start.partition]
        call    fat_ReadFolder
        pop     ebp
        ret

;----------------------------------------------------------------
; fat_ReadFolder - FAT16/32 implementation of reading a folder
; in:  ebp = pointer to FAT structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
fat_ReadFolder:
        call    fat_lock
        mov     eax, [ebp+FAT.ROOT_CLUSTER]
        push    edi
        cmp     byte [esi], 0
        jz      .doit
        stdcall hd_find_lfn, [esp+4+4]
        jnc     .found
        pop     edi
        push    eax
        call    fat_unlock
        pop     eax
        or      ebx, -1
        ret
.found:
        test    byte [edi+11], 0x10     ; do not allow read files
        jnz     .found_dir
        pop     edi
        call    fat_unlock
        or      ebx, -1
        mov     eax, ERROR_ACCESS_DENIED
        ret
.found_dir:
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]    ; eax=cluster
.doit:
        push    esi
        sub     esp, 262*2      ; reserve space for LFN
        push    dword [ebx+8]   ; for fat_get_name: read ANSI/UNICODE name
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
        push    ebp
        lea     ebp, [esp+20]
        call    fat_get_name
        pop     ebp
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
        pop     esi edi
        mov     ebx, [edx+4]
        call    fat_unlock
        mov     eax, ERROR_DEVICE
        ret
.done:
        add     esp, 262*2+4+8
        mov     ebx, [edx+4]
        xor     eax, eax
        dec     ecx
        js      @f
        mov     al, ERROR_END_OF_FILE
@@:
        push    eax
        call    fat_unlock
        pop     eax
        pop     esi edi
        ret

fat16_root_next:
        push    ecx
        lea     ecx, [ebp+FAT.buffer+0x200-0x20]
        cmp     edi, ecx
        jae     fat16_root_next_sector
        pop     ecx
        add     edi, 0x20
        ret     ; CF=0
fat16_root_next_sector:
; read next sector
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
        pop     ecx
        jb      fat16_root_first
        mov     eax, ERROR_FILE_NOT_FOUND
        stc
        ret
fat16_root_first:
        mov     eax, [eax+4]
        add     eax, [ebp+FAT.ROOT_START]
        push    ebx
        lea     edi, [ebp+FAT.buffer]
        mov     ebx, edi
        call    fs_read32_sys
        pop     ebx
        test    eax, eax
        jnz     .readerr
        ret     ; CF=0
.readerr:
        mov     eax, ERROR_DEVICE
        stc
        ret
.notfound:
        mov     eax, ERROR_FILE_NOT_FOUND
        stc
        ret
fat16_root_begin_write:
        push    edi eax
        call    fat16_root_first
        pop     eax edi
        ret
fat16_root_end_write:
        pusha
        mov     eax, [eax+4]
        add     eax, [ebp+FAT.ROOT_START]
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_sys
        popa
        ret
fat16_root_next_write:
        push    ecx
        lea     ecx, [ebp+FAT.buffer+0x200]
        cmp     edi, ecx
        jae     @f
        pop     ecx
        ret
@@:
        call    fat16_root_end_write
        jmp     fat16_root_next_sector
fat16_root_extend_dir:
        stc
        ret

fat_notroot_next:
        push    ecx
        lea     ecx, [ebp+FAT.buffer+0x200-0x20]
        cmp     edi, ecx
        jae     fat_notroot_next_sector
        pop     ecx
        add     edi, 0x20
        ret     ; CF=0
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
fat_notroot_next_err:
        pop     ecx
        mov     eax, ERROR_FILE_NOT_FOUND
        stc
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
fat_notroot_next_write:
        push    ecx
        lea     ecx, [ebp+FAT.buffer+0x200]
        cmp     edi, ecx
        jae     @f
        pop     ecx
        ret
@@:
        push    eax
        call    fat_notroot_end_write
        pop     eax
        jmp     fat_notroot_next_sector
fat_notroot_extend_dir:
        push    eax
        call    get_free_FAT
        jnc     .found
        pop     eax
        ret     ; CF=1
.found:
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
        jnc     @f
.writeerr:
        pop     edx
        pop     eax
        stc
        ret
@@:
        push    ecx
        or      ecx, -1
        call    add_disk_free_space
; zero new cluster
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
;
;  fs_HdRewrite - LFN variant for writing hard disk
;
;  Obsolete, will be replaced with filesystem-specific functions.
;
;  esi  points to filename
;  ebx  ignored (reserved)
;  ecx  number of bytes to write, 0+
;  edx  mem location to data
;
;  ret ebx = number of written bytes
;      eax = 0 ok read or other = errormsg
;
;--------------------------------------------------------------
fs_HdCreateFolder:
        mov     al, 1
        jmp     fs_HdRewrite.common

fs_HdRewrite:
        xor     eax, eax
.common:
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        cmp     [fs_type], 1
        jz      ntfs_HdRewrite
        cmp     [fs_type], 2
        jz      ext2_HdRewrite
        mov     eax, ERROR_UNKNOWN_FS
        xor     ebx, ebx
        ret
@@:
        sub     ebx, 4
        push    ebp
        mov     ebp, [fs_dependent_data_start.partition]
        test    eax, eax
        mov     eax, fat_CreateFolder
        jnz     @f
        mov     eax, fat_Rewrite
@@:
        call    eax
        pop     ebp
        ret

fshrad:
        call    fat_unlock
        mov     eax, ERROR_ACCESS_DENIED
        xor     ebx, ebx
        ret

;----------------------------------------------------------------
; fat_CreateFolder - FAT16/32 implementation of creating a folder
; in:  ebp = pointer to FAT structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
fat_CreateFolder:
        push    1
        jmp     fat_Rewrite.common

;----------------------------------------------------------------
; fat_HdRewrite - FAT16/32 implementation of creating a new file
; in:  ebp = pointer to FAT structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
fat_Rewrite:
        push    0
.common:
        call    fat_lock
        pop     eax
        cmp     byte [esi], 0
        jz      fshrad
        mov     ecx, [ebx+12]
        mov     edx, [ebx+16]
        pushad
        xor     edi, edi
        mov     edx, [esp+4+20h]
        push    esi
        test    edx, edx
        jz      @f
        mov     esi, edx
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
        test    edx, edx
        jnz     .hasebp
        mov     edx, [ebp+FAT.ROOT_CLUSTER]
        cmp     [ebp+FAT.fs_type], 32
        jz      .pushnotroot
        xor     edx, edx
        push    edx
        push    fat16_root_extend_dir
        push    fat16_root_end_write
        push    fat16_root_next_write
        push    fat16_root_begin_write
        push    edx
        push    edx
        push    fat16_root_first
        push    fat16_root_next
        jmp     .common1
.hasebp:
        mov     eax, ERROR_ACCESS_DENIED
        cmp     byte [edx], 0
        jz      .ret1
        stdcall hd_find_lfn, 0
        mov     esi, [esp+4+20h]
        jc      .ret1
        jmp     .common0
.noroot:
        mov     eax, ERROR_ACCESS_DENIED
        cmp     byte [edi+1], 0
        jz      .ret1
; check existence
        mov     byte [edi], 0
        push    edi
        stdcall hd_find_lfn, [esp+4+24h]
        pop     esi
        mov     byte [esi], '/'
        jnc     @f
.notfound0:
        mov     eax, ERROR_FILE_NOT_FOUND
.ret1:
        mov     [esp+28], eax
        call    fat_unlock
        popad
        xor     ebx, ebx
        ret
@@:
        inc     esi
.common0:
        test    byte [edi+11], 0x10     ; must be directory
        mov     eax, ERROR_ACCESS_DENIED
        jz      .ret1
        mov     edx, [edi+20-2]
        mov     dx, [edi+26]            ; ebp=cluster
        mov     eax, ERROR_FAT_TABLE
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
; found
        test    byte [edi+11], 10h
        jz      .exists_file
; found directory; if we are creating directory, return OK,
;                  if we are creating file, say "access denied"
        add     esp, 36
        call    fat_unlock
        popad
        test    al, al
        mov     eax, ERROR_ACCESS_DENIED
        jz      @f
        mov     al, 0
@@:
        xor     ebx, ebx
        ret
.exists_file:
; found file; if we are creating directory, return "access denied",
;             if we are creating file, delete existing file and continue
        cmp     byte [esp+36+28], 0
        jz      @f
        add     esp, 36
        call    fat_unlock
        popad
        mov     eax, ERROR_ACCESS_DENIED
        xor     ebx, ebx
        ret
@@:
; delete FAT chain
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
        push    edx
        xor     edx, edx
        call    set_FAT
        mov     eax, edx
        pop     edx
        jc      .done1
        inc     ecx
        jmp     @b
.done1:
        pop     edi
        call    get_time_for_file
        mov     [edi+22], ax
        call    get_date_for_file
        mov     [edi+24], ax
        mov     [edi+18], ax
        or      byte [edi+11], 20h      ; set 'archive' attribute
        jmp     .doit
.notfound:
; file is not found; generate short name
        call    fat_name_is_legal
        jc      @f
        add     esp, 36
        call    fat_unlock
        popad
        mov     eax, ERROR_FILE_NOT_FOUND
        xor     ebx, ebx
        ret
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
        jmp     .found
.short_name_found:
        pop     ecx edi esi
        call    fat_next_short_name
        jnc     .test_short_name_loop
.disk_full:
        add     esp, 12+36
        call    fat_unlock
        popa
        mov     eax, ERROR_DISK_FULL
        xor     ebx, ebx
        ret
.found:
        pop     ecx edi esi
; now find space in directory
; we need to save LFN <=> LFN is not equal to short name <=> generated name contains '~'
        mov     al, '~'
        push    ecx edi
        mov     ecx, 8
        repnz scasb
        push    1
        pop     eax     ; 1 entry
        jnz     .notilde
; we need ceil(strlen(esi)/13) additional entries = floor((strlen(esi)+12+13)/13) total
        xor     eax, eax
@@:
        cmp     byte [esi], 0
        jz      @f
        inc     esi
        inc     eax
        jmp     @b
@@:
        sub     esi, eax
        add     eax, 12+13
        mov     ecx, 13
        push    edx
        cdq
        div     ecx
        pop     edx
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
        call    fat_unlock
        popad
        mov     eax, ERROR_DEVICE
        xor     ebx, ebx
        ret
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
        call    fat_unlock
        popad
        mov     eax, ERROR_DISK_FULL
        xor     ebx, ebx
        ret
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
; If creating a directory, allocate one data cluster now and fail immediately
; if this is impossible. This prevents from creating an invalid directory entry
; on a full disk.
; yup, the argument is quite non-intuitive... but what should I do if
; the entire function uses such arguments? BTW, it refers to al from pushad,
; which in turn is filled with 0 in fat_Rewrite and 1 in fat_CreateFolder.
        cmp     byte [esp+8+12+8+12+36+28], 0
        jz      .no.preallocate.folder.data
        call    get_free_FAT
        jnc     @f
        add     esp, 8+12+8
        jmp     .disk_full
@@:
        mov     [esp+8+12+8+12+36+20], eax ; store the cluster somewhere
.no.preallocate.folder.data:
; calculate name checksum
        mov     esi, [esp+8+12]
        mov     ecx, 11
        xor     eax, eax
@@:
        ror     al, 1
        add     al, [esi]
        inc     esi
        loop    @b
        pop     ecx esi
        pop     edi
        pop     dword [esp+8+12+12]
        pop     dword [esp+8+12+12]
; edi points to first entry in free chunk
        dec     ecx
        jz      .nolfn
        push    esi
        push    eax
        lea     eax, [esp+8+8+12+8]
        call    dword [eax+8]         ; begin write
        mov     al, 40h
.writelfn:
        or      al, cl
        mov     esi, [esp+4]
        push    ecx
        dec     ecx
        imul    ecx, 13
        add     esi, ecx
        stosb
        mov     cl, 5
        call    fs_RamdiskRewrite.read_symbols
        mov     ax, 0xF
        stosw
        mov     al, [esp+4]
        stosb
        mov     cl, 6
        call    fs_RamdiskRewrite.read_symbols
        xor     eax, eax
        stosw
        mov     cl, 2
        call    fs_RamdiskRewrite.read_symbols
        pop     ecx
        lea     eax, [esp+8+8+12+8]
        call    dword [eax+12]         ; next write
        xor     eax, eax
        loop    .writelfn
        pop     eax
        pop     esi
;        lea     eax, [esp+8+12+8]
;        call    dword [eax+16]          ; end write
.nolfn:
        xchg    esi, [esp]
        mov     ecx, 11
        rep movsb
        mov     word [edi], 20h         ; attributes
        sub     edi, 11
        pop     esi ecx
        add     esp, 12
        mov     byte [edi+13], 0        ; tenths of a second at file creation time
        call    get_time_for_file
        mov     [edi+14], ax            ; creation time
        mov     [edi+22], ax            ; last write time
        call    get_date_for_file
        mov     [edi+16], ax            ; creation date
        mov     [edi+24], ax            ; last write date
        mov     [edi+18], ax            ; last access date
        xor     ecx, ecx
        mov     word [edi+20], cx       ; high word of cluster
        mov     word [edi+26], cx       ; low word of cluster - to be filled
        mov     dword [edi+28], ecx     ; file size - to be filled
        cmp     byte [esp+36+28], cl
        jz      .doit
; create directory
        mov     byte [edi+11], 10h      ; attributes: folder
        mov     esi, edi
        lea     eax, [esp+8]
        call    dword [eax+16]  ; flush directory
        mov     eax, [esp+36+20] ; extract saved cluster
        mov     [esp+36+20], edi ; this is needed for calculating arg of add_disk_free_space!
        push    ecx
        mov     ecx, [ebp+FAT.SECTORS_PER_CLUSTER]
        shl     ecx, 9
        push    ecx
        push    edi
        jmp     .doit2
.doit:
        mov     esi, [esp+36+20]
        lea     eax, [esp+8]
        call    dword [eax+16]  ; flush directory
        push    ecx
        mov     ecx, [esp+4+36+24]
        push    ecx
        push    edi
        test    ecx, ecx
        jz      .done
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
        push    edx
        mov     edx, [ebp+FAT.fatEND]
        call    set_FAT
        pop     edx
.write_cluster:
        push    eax
        dec     eax
        dec     eax
        imul    eax, [ebp+FAT.SECTORS_PER_CLUSTER]
        add     eax, [ebp+FAT.DATA_START]
        push    [ebp+FAT.SECTORS_PER_CLUSTER]
; write data
.write_sector:
        cmp     byte [esp+20+36+28], 0
        jnz     .writedir
        mov     ecx, 512
        cmp     dword [esp+12], ecx
        jb      .writeshort
; we can write directly from given buffer
        mov     ebx, esi
        add     esi, ecx
        jmp     .writecommon
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
        push    edx
        mov     edx, [ebp+FAT.fatEND]
        call    set_FAT
        xchg    eax, ecx
        mov     edx, ecx
        call    set_FAT
        pop     edx
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
.done:
        xor     eax, eax
.ret:
        pop     edi ecx
        sub     esi, [esp+4+36+20]
        mov     [esp+4+36+28], eax
        mov     [esp+4+36+16], esi
        lea     eax, [esp+12]
        call    dword [eax+8]
        mov     [edi+28], esi
        call    dword [eax+16]
        mov     [esp+36+16], ebx
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
        call    update_disk
        call    fat_unlock
        popad
        ret
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

;----------------------------------------------------------------
;
;  fs_HdWrite - LFN variant for writing to hard disk
;
;  Obsolete, will be replaced with filesystem-specific functions.
;
;  esi  points to filename
;  ebx  pointer to 64-bit number = first wanted byte, 0+
;       may be ebx=0 - start from first byte
;  ecx  number of bytes to write, 0+
;  edx  mem location to data
;
;  ret ebx = bytes written (maybe 0)
;      eax = 0 ok write or other = errormsg
;
;--------------------------------------------------------------
fat_Write.access_denied:
        push    ERROR_ACCESS_DENIED
fs_HdWrite.ret0:
        pop     eax
        xor     ebx, ebx
        ret

fs_HdWrite.ret11:
        push    ERROR_DEVICE
        jmp     fs_HdWrite.ret0

fs_HdWrite:
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        cmp     [fs_type], 1
        jz      ntfs_HdWrite
        cmp     [fs_type], 2
        jz      ext2_HdWrite
        push    ERROR_UNKNOWN_FS
        jmp     .ret0
@@:
        sub     ebx, 4
        push    ebp
        mov     ebp, [fs_dependent_data_start.partition]
        call    fat_Write
        pop     ebp
        ret

;----------------------------------------------------------------
; fat_Write - FAT16/32 implementation of writing to file
; in:  ebp = pointer to FAT structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
fat_Write:
        cmp     byte [esi], 0
        jz      .access_denied
        call    fat_lock
        push    edi
        stdcall hd_find_lfn, [esp+4+4]
        jnc     .found
        pop     edi
        push    eax
        call    fat_unlock
        jmp     fs_HdWrite.ret0
.found:
; FAT does not support files larger than 4GB
        cmp     dword [ebx+8], 0
        jz      @f
.eof:
        pop     edi
        push    ERROR_END_OF_FILE
        call    fat_unlock
        jmp     fs_HdWrite.ret0
@@:
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
        jz      .disk_full
        call    fat_unlock
        pop     eax
        pop     eax
        pop     ecx
        pop     edx
        pop     edi
        xor     ebx, ebx
        ret
.disk_full:
; correct number of bytes to write
        mov     ecx, [edi+28]
        cmp     ecx, ebx
        ja      .length_ok
        push    0
.ret:
        pop     eax
        sub     edx, [esp+12]
        mov     ebx, edx        ; ebx=number of written bytes
        call    update_disk
        test    eax, eax
        jz      @f
        mov     byte [esp+4], ERROR_DEVICE
@@:
        call    fat_unlock
        pop     eax
        pop     eax
        pop     ecx
        pop     edx
        pop     edi
        ret
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
        jz      @f
.device_err:
        mov     byte [esp+8], ERROR_DEVICE
        jmp     .ret
.fat_err:
        mov     byte [esp+8], ERROR_FAT_TABLE
        jmp     .ret
@@:

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
        jz      @f
.device_err2:
        pop     ecx
        jmp     .device_err
@@:
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
;      CF=1 => error, eax=code (ERROR_FAT_TABLE or ERROR_DISK_FULL or ERROR_DEVICE)
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
        push    ERROR_FAT_TABLE
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
        push    ERROR_DISK_FULL
        pop     eax
        stc
        ret

;----------------------------------------------------------------
;
;  fs_HdSetFileEnd - set end of file on hard disk
;
;  Obsolete, will be replaced with filesystem-specific functions.
;
;  esi  points to filename
;  ebx  points to 64-bit number = new file size
;  ecx  ignored (reserved)
;  edx  ignored (reserved)
;
;  ret eax = 0 ok or other = errormsg
;
;--------------------------------------------------------------
fs_HdSetFileEnd:
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        cmp     [fs_type], 1
        jz      ntfs_HdSetFileEnd
        cmp     [fs_type], 2
        jz      ext2_HdSetFileEnd
        push    ERROR_UNKNOWN_FS
        pop     eax
        ret
@@:
        sub     ebx, 4
        push    ebp
        mov     ebp, [fs_dependent_data_start.partition]
        call    fat_SetFileEnd
        pop     ebp
        ret

;----------------------------------------------------------------
; fat_SetFileEnd - FAT16/32 implementation of setting end-of-file
; in:  ebp = pointer to FAT structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
fat_SetFileEnd:
        call    fat_lock
        push    edi
        cmp     byte [esi], 0
        jnz     @f
.access_denied:
        push    ERROR_ACCESS_DENIED
.ret:
        call    fat_unlock
        pop     eax
        pop     edi
        ret
@@:
        stdcall hd_find_lfn, [esp+4+4]
        jnc     @f
.reteax:
        push    eax
        jmp     .ret
@@:
; must not be directory
        test    byte [edi+11], 10h
        jnz     .access_denied
; file size must not exceed 4 Gb
        cmp     dword [ebx+8], 0
        jz      @f
        push    ERROR_END_OF_FILE
        jmp     .ret
@@:
        push    eax     ; save directory sector
; set file modification date/time to current
        call    fat_update_datetime
        mov     eax, [ebx+4]
        cmp     eax, [edi+28]
        jb      .truncate
        ja      .expand
        pop     eax
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_sys
        pop     edi
        test    eax, eax
        jz      @f
        push    ERROR_DEVICE
        jmp     .ret
@@:
        push    0
        jmp     .ret
.expand:
        push    ebx ebp ecx
        push    dword [edi+28]  ; save old size
        mov     ecx, eax
        call    hd_extend_file
        push    eax             ; return code
        jnc     .expand_ok
        cmp     al, ERROR_DISK_FULL
        jz      .disk_full
.pop_ret:
        call    update_disk
        pop     eax ecx ecx ebp ebx ecx
        jmp     .reteax
.expand_ok:
.disk_full:
; save directory
        mov     eax, [edi+28]
        xchg    eax, [esp+20]
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_sys
        test    eax, eax
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]
        mov     edi, eax
        jz      @f
.pop_ret11:
        mov     byte [esp], ERROR_DEVICE
        jmp     .pop_ret
@@:
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
        jmp     .pop_ret11
.truncate:
        mov     [edi+28], eax
        push    ecx
        mov     ecx, [edi+20-2]
        mov     cx, [edi+26]
        push    eax
        test    eax, eax
        jz      .zero_size
; find new last cluster
@@:
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
        pop     eax ecx eax edi
        call    update_disk
        call    fat_unlock
        push    ERROR_DEVICE
        pop     eax
        ret
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
        pop     ecx eax edi
        call    update_disk
        call    fat_unlock
        xor     eax, eax
        ret
.error_fat:
        pop     eax
        mov     byte [esp], ERROR_FAT_TABLE
        jmp     .pop_ret
.error_fat2:
        pop     eax ecx eax edi
        call    update_disk
        call    fat_unlock
        push    ERROR_FAT_TABLE
        pop     eax
        ret

fs_HdGetFileInfo:
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        cmp     [fs_type], 1
        jz      ntfs_HdGetFileInfo
        cmp     [fs_type], 2
        jz      ext2_HdGetFileInfo
        mov     eax, ERROR_UNKNOWN_FS
        ret
@@:
        sub     ebx, 4
        push    ebp
        mov     ebp, [fs_dependent_data_start.partition]
        call    fat_GetFileInfo
        pop     ebp
        ret

;----------------------------------------------------------------
; fat_GetFileInfo - FAT16/32 implementation of getting file info
; in:  ebp = pointer to FAT structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
fat_GetFileInfo:
        cmp     byte [esi], 0
        jnz     @f
        mov     eax, 2
        ret
@@:
        push    edi
        call    fat_lock
        stdcall hd_find_lfn, [esp+4+4]
        jc      .error
        push    ebp
        xor     ebp, ebp
        mov     esi, [ebx+16]
        mov     dword [esi+4], ebp
        call    fat_entry_to_bdfe2
        pop     ebp
        call    fat_unlock
        xor     eax, eax
        pop     edi
        ret
.error:
        push    eax
        call    fat_unlock
        pop     eax
        pop     edi
        ret

fs_HdSetFileInfo:
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        cmp     [fs_type], 1
        jz      ntfs_HdSetFileInfo
        cmp     [fs_type], 2
        jz      ext2_HdSetFileInfo
        mov     eax, ERROR_UNKNOWN_FS
        ret
@@:
        sub     ebx, 4
        push    ebp
        mov     ebp, [fs_dependent_data_start.partition]
        call    fat_SetFileInfo
        pop     ebp
        ret

;----------------------------------------------------------------
; fat_SetFileInfo - FAT16/32 implementation of setting file info
; in:  ebp = pointer to FAT structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
fat_SetFileInfo:
        cmp     byte [esi], 0
        jnz     @f
        mov     eax, 2
        ret
@@:
        push    edi
        call    fat_lock
        stdcall hd_find_lfn, [esp+4+4]
        jc      .error
        push    eax
        mov     edx, [ebx+16]
        call    bdfe_to_fat_entry
        pop     eax
        lea     ebx, [ebp+FAT.buffer]
        call    fs_write32_sys
        call    update_disk
        call    fat_unlock
        pop     edi
        xor     eax, eax
        ret
.error:
        push    eax
        call    fat_unlock
        pop     eax
        pop     edi
        ret

;----------------------------------------------------------------
;
;  fs_HdDelete - delete file or empty folder from hard disk
;
;  Obsolete, will be replaced with filesystem-specific functions.
;
;  esi  points to filename
;
;  ret  eax = 0 ok or other = errormsg
;
;--------------------------------------------------------------
fs_HdDelete:
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        cmp     [fs_type], 1
        jz      ntfs_HdDelete
        cmp     [fs_type], 2
        jz      ext2_HdDelete
        push    ERROR_UNKNOWN_FS
        pop     eax
        ret
@@:
        sub     ebx, 4
        push    ebp
        mov     ebp, [fs_dependent_data_start.partition]
        call    fat_Delete
        pop     ebp
        ret

;----------------------------------------------------------------
; fat_Delete - FAT16/32 implementation of deleting a file/folder
; in:  ebp = pointer to FAT structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
fat_Delete:
        call    fat_lock
        cmp     byte [esi], 0
        jnz     @f
; cannot delete root!
.access_denied:
        push    ERROR_ACCESS_DENIED
.pop_ret:
        call    fat_unlock
        pop     eax
        xor     ebx, ebx
        ret
@@:
        and     [ebp+FAT.longname_sec1], 0
        and     [ebp+FAT.longname_sec2], 0
        push    edi
        stdcall hd_find_lfn, [esp+4+4]
        jnc     .found
        pop     edi
        push    ERROR_FILE_NOT_FOUND
        jmp     .pop_ret
.found:
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
        pop     edi
        call    fat_unlock
        push    ERROR_DEVICE
        pop     eax
        ret
.error_fat:
        popad
        pop     edi
        call    fat_unlock
        push    ERROR_FAT_TABLE
        pop     eax
        ret
.notempty:
        popad
.access_denied2:
        pop     edi
        call    fat_unlock
        push    ERROR_ACCESS_DENIED
        pop     eax
        ret
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
        pop     edi
        xor     eax, eax
        ret

; \end{diamond}
