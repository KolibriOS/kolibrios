;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved.    ;;
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

$Revision$


cache_max equ 1919      ; max. is 1919*512+0x610000=0x6ffe00

ERROR_SUCCESS        = 0
ERROR_DISK_BASE      = 1
ERROR_UNSUPPORTED_FS = 2
ERROR_UNKNOWN_FS     = 3
ERROR_PARTITION      = 4
ERROR_FILE_NOT_FOUND = 5
ERROR_END_OF_FILE    = 6
ERROR_MEMORY_POINTER = 7
ERROR_DISK_FULL      = 8
ERROR_FAT_TABLE      = 9
ERROR_ACCESS_DENIED  = 10

PUSHAD_EAX equ [esp+28]
PUSHAD_ECX equ [esp+24]
PUSHAD_EDX equ [esp+20]
PUSHAD_EBX equ [esp+16]
PUSHAD_EBP equ [esp+8]
PUSHAD_ESI equ [esp+4]
PUSHAD_EDI equ [esp+0]

uglobal
align 4
partition_count      dd 0       ; partitions found by set_FAT32_variables
longname_sec1        dd 0       ; used by analyze_directory to save 2 previous
longname_sec2        dd 0       ; directory sectors for delete long filename

hd_error             dd 0       ; set by wait_for_sector_buffer
hd_setup             dd 0
hd_wait_timeout      dd 0

cluster_tmp          dd 0       ; used by analyze_directory
                                ; and analyze_directory_to_write

file_size            dd 0       ; used by file_read

cache_search_start   dd 0       ; used by find_empty_slot
endg

iglobal
fat_in_cache         dd -1
endg

uglobal
align 4
fat_cache:           times 512 db 0
 Sector512:                      ; label for dev_hdcd.inc
  buffer:              times 512 db 0
  fsinfo_buffer:       times 512 db 0
endg

uglobal
  fat16_root           db 0       ; flag for fat16 rootdir
  fat_change           db 0       ; 1=fat has changed
endg

reserve_hd1:

    pushfd
    cli

    cmp   [hd1_status],0                          ;FIXME  use mutex
    je    reserve_ok1

    popfd

    call  change_task
    jmp   reserve_hd1

  reserve_ok1:

    push  eax
    mov   eax,[CURRENT_TASK]
    shl   eax,5
    mov   eax,[eax+CURRENT_TASK+TASKDATA.pid]
    mov   [hd1_status],eax
    pop   eax
    popfd
    ret
;********************************************

uglobal
hd_in_cache db ?
endg

reserve_hd_channel:
; BIOS disk accesses are protected with common mutex hd1_status
; This must be modified when hd1_status will not be valid!
        cmp     [hdpos], 0x80
        jae     .ret
    cmp   [hdbase], 0x1F0
    jne   .IDE_Channel_2
.IDE_Channel_1:
    pushfd
    cli
    cmp   [IDE_Channel_1],0
    je    .reserve_ok_1
    popfd
    call  change_task
    jmp   .IDE_Channel_1
.IDE_Channel_2:
    pushfd
    cli
    cmp   [IDE_Channel_2],0
    je    .reserve_ok_2
    popfd
    call  change_task
    jmp   .IDE_Channel_2
.reserve_ok_1:
        mov     [IDE_Channel_1], 1
        popfd
        push    eax
        mov     al, 1
        jmp     @f
.reserve_ok_2:
        mov     [IDE_Channel_2], 1
        popfd
        push    eax
        mov     al, 3
@@:
        cmp     [hdid], 1
        sbb     al, -1
        cmp     al, [hd_in_cache]
        jz      @f
        mov     [hd_in_cache], al
        call    clear_hd_cache
@@:
	pop     eax
.ret:
        ret

free_hd_channel:
; see comment at reserve_hd_channel
        cmp     [hdpos], 0x80
        jae     .ret
    cmp   [hdbase], 0x1F0
    jne   .IDE_Channel_2
.IDE_Channel_1:
    mov [IDE_Channel_1],0
.ret:
    ret
.IDE_Channel_2:
    mov [IDE_Channel_2],0
    ret
;********************************************
problem_partition db 0  ; used for partitions search

include  'part_set.inc'

set_FAT:
;--------------------------------
; input  : EAX = cluster
;          EDX = value to save
; output : EDX = old value
;--------------------------------
    push  eax ebx esi

    cmp   eax,2
    jb    sfc_error
    cmp   eax,[LAST_CLUSTER]
    ja    sfc_error
    cmp   [fs_type],16
    je    sfc_1
    add   eax,eax
  sfc_1:
    add   eax,eax
    mov   esi,511
    and   esi,eax               ; esi = position in fat sector
    shr   eax,9                 ; eax = fat sector
    add   eax,[FAT_START]
    mov   ebx,fat_cache

    cmp   eax,[fat_in_cache]    ; is fat sector already in memory?
    je    sfc_in_cache          ; yes

    cmp   [fat_change],0        ; is fat changed?
    je    sfc_no_change         ; no
    call  write_fat_sector      ; yes. write it into disk
    cmp   [hd_error],0
    jne   sfc_error

  sfc_no_change:
    mov   [fat_in_cache],eax    ; save fat sector
    call  hd_read
    cmp  [hd_error],0
    jne  sfc_error


  sfc_in_cache:
    cmp   [fs_type],16
    jne   sfc_test32

  sfc_set16:
    xchg  [ebx+esi],dx          ; save new value and get old value
    jmp   sfc_write

  sfc_test32:
    mov   eax,[fatMASK]

  sfc_set32:
    and   edx,eax
    xor   eax,-1                ; mask for high bits
    and   eax,[ebx+esi]         ; get high 4 bits
    or    eax,edx
    mov   edx,[ebx+esi]         ; get old value
    mov   [ebx+esi],eax         ; save new value

  sfc_write:
    mov   [fat_change],1        ; fat has changed

  sfc_nonzero:
    and   edx,[fatMASK]

  sfc_error:
    pop   esi ebx eax
    ret


get_FAT:
;--------------------------------
; input  : EAX = cluster
; output : EAX = next cluster
;--------------------------------
    push  ebx esi

    cmp   [fs_type],16
    je    gfc_1
    add   eax,eax
  gfc_1:
    add   eax,eax
    mov   esi,511
    and   esi,eax               ; esi = position in fat sector
    shr   eax,9                 ; eax = fat sector
    add   eax,[FAT_START]
    mov   ebx,fat_cache

    cmp   eax,[fat_in_cache]    ; is fat sector already in memory?
    je    gfc_in_cache

    cmp   [fat_change],0        ; is fat changed?
    je    gfc_no_change         ; no
    call  write_fat_sector      ; yes. write it into disk
    cmp  [hd_error],0
    jne  hd_error_01

  gfc_no_change:
    mov   [fat_in_cache],eax
    call  hd_read
    cmp  [hd_error],0
    jne  hd_error_01

  gfc_in_cache:
    mov   eax,[ebx+esi]
    and   eax,[fatMASK]
 hd_error_01:
    pop   esi ebx
    ret


get_free_FAT:
;-----------------------------------------------------------
; output : if CARRY=0 EAX = # first cluster found free
;          if CARRY=1 disk full
; Note   : for more speed need to use fat_cache directly
;-----------------------------------------------------------
    push  ecx
    mov   ecx,[LAST_CLUSTER]    ; counter for full disk
    sub   ecx,2
    mov   eax,[fatStartScan]
    cmp   eax,2
    jb    gff_reset

  gff_test:
    cmp   eax,[LAST_CLUSTER]    ; if above last cluster start at cluster 2
    jbe   gff_in_range
  gff_reset:
    mov   eax,2

  gff_in_range:
    push  eax
    call  get_FAT               ; get cluster state
    cmp   [hd_error],0
    jne   gff_not_found_1

    test  eax,eax               ; is it free?
    pop   eax
    je    gff_found             ; yes
    inc   eax                   ; next cluster
    dec   ecx                   ; is all checked?
    jns   gff_test              ; no

  gff_not_found_1:
    add   esp,4
  gff_not_found:
    pop   ecx                   ; yes. disk is full
    stc
    ret

  gff_found:
    lea   ecx,[eax+1]
    mov   [fatStartScan],ecx
    pop   ecx
    clc
    ret


write_fat_sector:
;-----------------------------------------------------------
; write changed fat to disk
;-----------------------------------------------------------
    push  eax ebx ecx

    mov   [fat_change],0
    mov   eax,[fat_in_cache]
    cmp   eax,-1
    jz    write_fat_not_used
    mov   ebx,fat_cache
    mov   ecx,[NUMBER_OF_FATS]

  write_next_fat:
    call  hd_write
    cmp   [hd_error],0
    jne   write_fat_not_used

    add   eax,[SECTORS_PER_FAT]
    dec   ecx
    jnz   write_next_fat

  write_fat_not_used:
    pop   ecx ebx eax
    ret


analyze_directory:
;-----------------------------------------------------------
; input  : EAX = first cluster of the directory
;          EBX = pointer to filename
; output : IF CARRY=0 EAX = sector where th file is found
;                     EBX = pointer in buffer
;                     [buffer .. buffer+511]
;                     ECX,EDX,ESI,EDI not changed
;          IF CARRY=1 filename not found
; Note   : if cluster=0 it's changed to read rootdir
;          save 2 previous directory sectors in longname_sec
;-----------------------------------------------------------
    push  ecx edx esi edi ebx   ; ebx = [esp+0]
    mov   [longname_sec1],0
    mov   [longname_sec2],0

  adr_new_cluster:
    mov   [cluster_tmp],eax
    mov   [fat16_root],0
    cmp   eax,[LAST_CLUSTER]
    ja    adr_not_found         ; too big cluster number, something is wrong
    cmp   eax,2
    jnb   adr_data_cluster

    mov   eax,[ROOT_CLUSTER]    ; if cluster < 2 then read rootdir
    cmp   [fs_type],16
    jne   adr_data_cluster
    mov   eax,[ROOT_START]
    mov   edx,[ROOT_SECTORS]
    mov   [fat16_root],1        ; flag for fat16 rootdir
    jmp   adr_new_sector

  adr_data_cluster:
    sub   eax,2
    mov   edx,[SECTORS_PER_CLUSTER]
    imul  eax,edx
    add   eax,[DATA_START]

  adr_new_sector:
    mov   ebx,buffer
    call  hd_read
    cmp  [hd_error],0
    jne  adr_not_found

    mov   ecx,512/32            ; count of dir entrys per sector = 16

  adr_analyze:
    mov   edi,[ebx+11]          ; file attribute
    and   edi,0xf
    cmp   edi,0xf
    je    adr_long_filename
    test  edi,0x8               ; skip over volume label
    jne   adr_long_filename     ; Note: label can be same name as file/dir

    mov   esi,[esp+0]           ; filename need to be uppercase
    mov   edi,ebx
    push  ecx
    mov   ecx,11
    cld
    rep   cmpsb                 ; compare 8+3 filename
    pop   ecx
    je    adr_found

  adr_long_filename:
    add   ebx,32                ; position of next dir entry
    dec   ecx
    jnz   adr_analyze

    mov   ecx,[longname_sec1]   ; save 2 previous directory sectors
    mov   [longname_sec1],eax   ; for delete long filename
    mov   [longname_sec2],ecx
    inc   eax                   ; next sector
    dec   edx
    jne   adr_new_sector
    cmp   [fat16_root],1        ; end of fat16 rootdir
    je    adr_not_found

  adr_next_cluster:
    mov   eax,[cluster_tmp]
    call  get_FAT               ; get next cluster
    cmp  [hd_error],0
    jne  adr_not_found

    cmp   eax,2                 ; incorrect fat chain?
    jb    adr_not_found         ; yes
    cmp   eax,[fatRESERVED]     ; is it end of directory?
    jb    adr_new_cluster       ; no. analyse it

  adr_not_found:
    pop   edi edi esi edx ecx   ; first edi will remove ebx
    stc                         ; file not found
    ret

  adr_found:
    pop   edi edi esi edx ecx   ; first edi will remove ebx
    clc                         ; file found
    ret


get_data_cluster:
;-----------------------------------------------------------
; input  : EAX = cluster
;          EBX = pointer to buffer
;          EDX = # blocks to read in buffer
;          ESI = # blocks to skip over
; output : if CARRY=0 ok EBX/EDX/ESI updated
;          if CARRY=1 cluster out of range
; Note   : if cluster=0 it's changed to read rootdir
;-----------------------------------------------------------
    push  eax ecx

    mov   [fat16_root],0
    cmp   eax,[LAST_CLUSTER]
    ja    gdc_error             ; too big cluster number, something is wrong
    cmp   eax,2
    jnb   gdc_cluster

    mov   eax,[ROOT_CLUSTER]    ; if cluster < 2 then read rootdir
    cmp   [fs_type],16
    jne   gdc_cluster
    mov   eax,[ROOT_START]
    mov   ecx,[ROOT_SECTORS]    ; Note: not cluster size
    mov   [fat16_root],1        ; flag for fat16 rootdir
    jmp   gdc_read

  gdc_cluster:
    sub   eax,2
    mov   ecx,[SECTORS_PER_CLUSTER]
    imul  eax,ecx
    add   eax,[DATA_START]

  gdc_read:
    test  esi,esi               ; first wanted block
    je    gdcl1                 ; yes, skip count is 0
    dec   esi
    jmp   gdcl2

  gdcl1:
    call  hd_read
    cmp  [hd_error],0
    jne  gdc_error

    add   ebx,512               ; update pointer
    dec   edx

  gdcl2:
    test  edx,edx               ; is all read?
    je    out_of_read

    inc   eax                   ; next sector
    dec   ecx
    jnz   gdc_read

  out_of_read:
    pop   ecx eax
    clc
    ret

  gdc_error:
    pop   ecx eax
    stc
    ret


get_cluster_of_a_path:
;---------------------------------------------------------
; input  : EBX = pointer to a path string
;          (example: the path "/files/data/document" become
;                             "files......data.......document...0"
;          '.' = space char
;          '0' = char(0) (ASCII=0) !!! )
; output : if (CARRY=1) -> ERROR in the PATH
;          if (CARRY=0) -> EAX=cluster
;---------------------------------------------------------
    push  ebx edx

    mov   eax,[ROOT_CLUSTER]
    mov   edx,ebx

search_end_of_path:
    cmp   byte [edx],0
    je    found_end_of_path

    inc   edx ; '/'
    mov   ebx,edx
    call  analyze_directory
    jc    directory_not_found

    mov   eax,[ebx+20-2]        ; read the HIGH 16bit cluster field
    mov   ax,[ebx+26]           ; read the LOW 16bit cluster field
    and   eax,[fatMASK]
    add   edx,11                ; 8+3 (name+extension)
    jmp   search_end_of_path

found_end_of_path:
    pop   edx ebx
    clc                         ; no errors
    ret

directory_not_found:
    pop   edx ebx
    stc                         ; errors occour
    ret


bcd2bin:
;----------------------------------
; input  : AL=BCD number (eg. 0x11)
; output : AH=0
;          AL=decimal number (eg. 11)
;----------------------------------
    xor   ah,ah
    shl   ax,4
    shr   al,4
    aad
    ret


get_date_for_file:
;-----------------------------------------------------
; Get date from CMOS and pack day,month,year in AX
; DATE   bits  0..4   : day of month 0..31
;              5..8   : month of year 1..12
;              9..15  : count of years from 1980
;-----------------------------------------------------
    mov   al,0x7        ;day
    out   0x70,al
    in    al,0x71
    call  bcd2bin
    ror   eax,5

    mov   al,0x8        ;month
    out   0x70,al
    in    al,0x71
    call  bcd2bin
    ror   eax,4

    mov   al,0x9        ;year
    out   0x70,al
    in    al,0x71
    call  bcd2bin
    add   ax,20         ;because CMOS return only the two last
                        ;digit (eg. 2000 -> 00 , 2001 -> 01) and we
    rol   eax,9         ;need the difference with 1980 (eg. 2001-1980)
    ret


get_time_for_file:
;-----------------------------------------------------
; Get time from CMOS and pack hour,minute,second in AX
; TIME   bits  0..4   : second (the low bit is lost)
;              5..10  : minute 0..59
;              11..15 : hour 0..23
;-----------------------------------------------------
    mov   al,0x0        ;second
    out   0x70,al
    in    al,0x71
    call  bcd2bin
    ror   eax,6

    mov   al,0x2        ;minute
    out   0x70,al
    in    al,0x71
    call  bcd2bin
    ror   eax,6

    mov   al,0x4        ;hour
    out   0x70,al
    in    al,0x71
    call  bcd2bin
    rol   eax,11
    ret


set_current_time_for_entry:
;-----------------------------------------------------
; Set current time/date for file entry
; input  : ebx = file entry pointer
;-----------------------------------------------------
    push  eax
    call  get_time_for_file     ; update files date/time
    mov   [ebx+22],ax
    call  get_date_for_file
    mov   [ebx+24],ax
    pop   eax
    ret



add_disk_free_space:
;-----------------------------------------------------
; input  : ecx = cluster count
; Note   : negative = remove clusters from free space
;          positive = add clusters to free space
;-----------------------------------------------------
    test  ecx,ecx               ; no change
    je    add_dfs_no
    cmp   [fs_type],32         ; free disk space only used by fat32
    jne   add_dfs_no

    push  eax ebx
    mov   eax,[ADR_FSINFO]
    mov   ebx,fsinfo_buffer
    call  hd_read
    cmp  [hd_error],0
    jne  add_not_fs

    cmp   dword [ebx+0x1fc],0xaa550000 ; check sector id
    jne   add_not_fs

    add   [ebx+0x1e8],ecx
    push  [fatStartScan]
    pop   dword [ebx+0x1ec]
    call  hd_write
;    cmp   [hd_error],0
;    jne   add_not_fs

  add_not_fs:
    pop   ebx eax

  add_dfs_no:
    ret


file_read:
;--------------------------------------------------------------------------
;   INPUT :  user-register register-in-this  meaning         symbol-in-this
;
;            EAX           EDI               system call to write   /
;            EBX           EAX   (PAR0)      pointer to file-name   PAR0
;            EDX           ECX   (PAR1)      pointer to buffer      PAR1
;            ECX           EBX   (PAR2)   vt file blocks to read    PAR2
;            ESI           EDX   (PAR3)      pointer to path        PAR3
;            EDI           ESI            vt first 512 block to read
;                          EDI               if 0 - read root
;
; output : eax = 0 - ok
;                3 - unknown FS
;                5 - file not found
;                6 - end of file
;                9 - fat table corrupted
;               10 - access denied
;          ebx = size of file/directory
;--------------------------------------------------------------------------
        cmp     [fs_type], 16
        jz      fat_ok_for_reading
        cmp     [fs_type], 32
        jz      fat_ok_for_reading
    xor   ebx,ebx
    mov   eax,ERROR_UNKNOWN_FS
    mov   [hd1_status], ebx
    ret

  fat_ok_for_reading:
;    call  reserve_hd1

    pushad

    mov   ebx,edx
    call  get_cluster_of_a_path
    jc    file_to_read_not_found

    test  edi,edi               ; read rootdir
    jne   no_read_root

    xor   eax,eax
    call  get_dir_size          ; return rootdir size
    cmp   [hd_error],0
    jne   file_access_denied

    mov   [file_size],eax
    mov   eax,[ROOT_CLUSTER]
    jmp   file_read_start

  no_read_root:
    mov   ebx,PUSHAD_EAX        ; file name
    call  analyze_directory
    jc    file_to_read_not_found

    mov   eax,[ebx+28]          ; file size
    test  byte [ebx+11],0x10    ; is it directory?
    jz    read_set_size         ; no

    mov   eax,[ebx+20-2]        ; FAT entry
    mov   ax,[ebx+26]
    and   eax,[fatMASK]
    call  get_dir_size
    cmp   [hd_error],0
    jne   file_access_denied

  read_set_size:
    mov   [file_size],eax

    mov   eax,[ebx+20-2]        ; FAT entry
    mov   ax,[ebx+26]
    and   eax,[fatMASK]

  file_read_start:
    mov   ebx,PUSHAD_ECX        ; pointer to buffer
    mov   edx,PUSHAD_EBX        ; file blocks to read
    mov   esi,PUSHAD_ESI        ; first 512 block to read

  file_read_new_cluster:
    call  get_data_cluster
    jc    file_read_eof         ; end of file or cluster out of range

    test  edx,edx               ; is all read?
    je    file_read_OK          ; yes

    call  get_FAT               ; get next cluster
    cmp   [hd_error],0
    jne   file_access_denied

    cmp   eax,[fatRESERVED]     ; end of file
    jnb   file_read_eof
    cmp   eax,2                 ; incorrect fat chain
    jnb   file_read_new_cluster

    popad
    mov   [hd1_status],0
    mov   ebx,[file_size]
    mov   eax,ERROR_FAT_TABLE
    ret

  file_read_eof:
    cmp   [hd_error],0
    jne   file_access_denied
    popad
    mov   [hd1_status],0
    mov   ebx,[file_size]
    mov   eax,ERROR_END_OF_FILE
    ret

  file_read_OK:
    popad
    mov   [hd1_status],0
    mov   ebx,[file_size]
    xor   eax,eax
    ret

  file_to_read_not_found:
    cmp   [hd_error],0
    jne   file_access_denied
    popad
    mov   [hd1_status],0
    xor   ebx,ebx
    mov   eax,ERROR_FILE_NOT_FOUND
    ret

  file_access_denied:
    popad
    mov   [hd1_status],0
    xor   ebx,ebx
    mov   eax,ERROR_ACCESS_DENIED
    ret

get_dir_size:
;-----------------------------------------------------
; input  : eax = first cluster (0=rootdir)
; output : eax = directory size in bytes
;-----------------------------------------------------
    push  edx
    xor   edx,edx               ; count of directory clusters
    test  eax,eax
    jnz   dir_size_next

    mov   eax,[ROOT_SECTORS]
    shl   eax,9                 ; fat16 rootdir size in bytes
    cmp   [fs_type],16
    je    dir_size_ret
    mov   eax,[ROOT_CLUSTER]

  dir_size_next:
    cmp   eax,2                 ; incorrect fat chain
    jb    dir_size_end
    cmp   eax,[fatRESERVED]     ; end of directory
    ja    dir_size_end
    call  get_FAT               ; get next cluster
    cmp   [hd_error],0
    jne   dir_size_ret

    inc   edx
    jmp   dir_size_next

  dir_size_end:
    imul  eax,[SECTORS_PER_CLUSTER],512 ; cluster size in bytes
    imul  eax,edx

  dir_size_ret:
    pop   edx
    ret


clear_cluster_chain:
;-----------------------------------------------------
; input  : eax = first cluster
;-----------------------------------------------------
    push  eax ecx edx
    xor   ecx,ecx               ; cluster count

  clean_new_chain:
    cmp   eax,[LAST_CLUSTER]    ; end of file
    ja    delete_OK
    cmp   eax,2                 ; unfinished fat chain or zero length file
    jb    delete_OK
    cmp   eax,[ROOT_CLUSTER]    ; don't remove root cluster
    jz    delete_OK

    xor   edx,edx
    call  set_FAT               ; clear fat entry
    cmp  [hd_error],0
    jne  access_denied_01

    inc   ecx                   ; update cluster count
    mov   eax,edx               ; old cluster
    jmp   clean_new_chain

  delete_OK:
    call  add_disk_free_space   ; add clusters to free disk space
  access_denied_01:
    pop   edx ecx eax
    ret


get_hd_info:
;-----------------------------------------------------------
; output : eax = 0 - ok
;                3 - unknown FS
;               10 - access denied
;          edx = cluster size in bytes
;          ebx = total clusters on disk
;          ecx = free clusters on disk
;-----------------------------------------------------------
        cmp     [fs_type], 16
        jz      info_fat_ok
        cmp     [fs_type], 32
        jz      info_fat_ok
    xor   edx,edx
    xor   ebx,ebx
    xor   ecx,ecx
    mov   eax,ERROR_UNKNOWN_FS
    ret

  info_fat_ok:
;    call  reserve_hd1

    xor   ecx,ecx               ; count of free clusters
    mov   eax,2
    mov   ebx,[LAST_CLUSTER]

  info_cluster:
    push  eax
    call  get_FAT               ; get cluster info
    cmp   [hd_error],0
    jne   info_access_denied

    test  eax,eax               ; is it free?
    jnz   info_used             ; no
    inc   ecx

  info_used:
    pop   eax
    inc   eax
    cmp   eax,ebx               ; is above last cluster?
    jbe   info_cluster          ; no. test next cluster

    dec   ebx                   ; cluster count
    imul  edx,[SECTORS_PER_CLUSTER],512 ; cluster size in bytes
    mov   [hd1_status],0
    xor   eax,eax
    ret

  info_access_denied:
    add   esp,4
    xor   edx,edx
    xor   ebx,ebx
    xor   ecx,ecx
    mov   eax,ERROR_ACCESS_DENIED
    ret

update_disk:
;-----------------------------------------------------------
; write changed fat and cache to disk
;-----------------------------------------------------------
    cmp   [fat_change],0        ; is fat changed?
    je    upd_no_change

    call  write_fat_sector
    cmp   [hd_error],0
    jne   update_disk_acces_denied

  upd_no_change:

    call  write_cache
  update_disk_acces_denied:
    ret


; \begin{diamond}
hd_find_lfn:
; in: esi+ebp -> name
; out: CF=1 - file not found
;      else CF=0 and edi->direntry, eax=sector
; destroys eax
        push    esi edi
        push    0
        push    0
        push    fat16_root_first
        push    fat16_root_next
        mov     eax, [ROOT_CLUSTER]
        cmp     [fs_type], 32
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
        ret
.found:
        test    ebp, ebp
        jz      @f
        mov     esi, ebp
        xor     ebp, ebp
        jmp     .continue
@@:
        lea     eax, [esp+8]
        cmp     dword [eax], 0
        jz      .root
        call    fat_get_sector
        jmp     .cmn
.root:
        mov     eax, [eax+4]
        add     eax, [ROOT_START]
.cmn:
        add     esp, 20         ; CF=0
        pop     esi
        ret

;----------------------------------------------------------------
;
;  fs_HdRead - LFN variant for reading hard disk
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
        or      ebx, -1
        mov     eax, ERROR_UNKNOWN_FS
        ret
@@:
    push    edi
    cmp    byte [esi], 0
    jnz    @f
.noaccess:
    pop    edi
.noaccess_2:
    or    ebx, -1
    mov    eax, ERROR_ACCESS_DENIED
    ret

.noaccess_3:
    add esp,4
.noaccess_1:
    add esp,4
.noaccess_4:
    add esp,4*5
    jmp  .noaccess_2

@@:
    call    hd_find_lfn
    jnc    .found
    pop    edi
    cmp   [hd_error],0
    jne   .noaccess_2
    or    ebx, -1
    mov    eax, ERROR_FILE_NOT_FOUND
    ret

.found:
    test    byte [edi+11], 0x10    ; do not allow read directories
    jnz    .noaccess
    test    ebx, ebx
    jz    .l1
    cmp    dword [ebx+4], 0
    jz    @f
        xor     ebx, ebx
.reteof:
    mov    eax, 6
    pop    edi
    ret
@@:
    mov    ebx, [ebx]
.l1:
        push    ecx edx
        push    0
        mov     eax, [edi+28]
        sub     eax, ebx
        jb      .eof
        cmp     eax, ecx
        jae     @f
        mov     ecx, eax
        mov     byte [esp], 6
@@:
    mov    eax, [edi+20-2]
    mov    ax, [edi+26]
; now eax=cluster, ebx=position, ecx=count, edx=buffer for data
.new_cluster:
    jecxz    .new_sector
    test    eax, eax
    jz    .eof
    cmp    eax, [fatRESERVED]
    jae    .eof
    mov    [cluster_tmp], eax
    dec    eax
    dec    eax
    mov    edi, [SECTORS_PER_CLUSTER]
    imul    eax, edi
    add    eax, [DATA_START]
.new_sector:
    test    ecx, ecx
    jz    .done
    sub    ebx, 512
    jae    .skip
    add    ebx, 512
    jnz    .force_buf
    cmp    ecx, 512
    jb    .force_buf
; we may read directly to given buffer
    push    ebx
    mov    ebx, edx
    call    hd_read
    cmp  [hd_error],0
    jne  .noaccess_1
    pop    ebx
    add    edx, 512
    sub    ecx, 512
    jmp    .skip
.force_buf:
; we must read sector to temporary buffer and then copy it to destination
    push    eax ebx
    mov    ebx, buffer
    call    hd_read
    cmp  [hd_error],0
    jne  .noaccess_3

    mov    eax, ebx
    pop    ebx
    add    eax, ebx
    push    ecx
    add    ecx, ebx
    cmp    ecx, 512
    jbe    @f
    mov    ecx, 512
@@:
    sub    ecx, ebx
    mov    ebx, edx
    call    memmove
    add    edx, ecx
    sub    [esp], ecx
    pop    ecx
    pop    eax
    xor    ebx, ebx
.skip:
    inc    eax
    dec    edi
    jnz    .new_sector
    mov    eax, [cluster_tmp]
    call    get_FAT
    cmp   [hd_error],0
    jne   .noaccess_4

    jmp    .new_cluster
.done:
        mov     ebx, edx
        pop     eax edx ecx edi
        sub     ebx, edx
        ret
.eof:
        mov     ebx, edx
        pop     eax edx ecx
        sub     ebx, edx
        jmp     .reteof

;----------------------------------------------------------------
;
;  fs_HdReadFolder - LFN variant for reading hard disk folder
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
        cmp     [fs_type], 1
        jz      ntfs_HdReadFolder
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        push    ERROR_UNSUPPORTED_FS
        pop     eax
        or      ebx, -1
        ret
@@:
        mov     eax, [ROOT_CLUSTER]
        push    edi
        cmp     byte [esi], 0
        jz      .doit
        call    hd_find_lfn
        jnc     .found
        pop     edi
        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
.found:
        test    byte [edi+11], 0x10     ; do not allow read files
        jnz     .found_dir
        pop     edi
        or      ebx, -1
        mov     eax, ERROR_ACCESS_DENIED
        ret
.found_dir:
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]    ; eax=cluster
.doit:
        push    esi ecx
        push    ebp
        sub     esp, 262*2      ; reserve space for LFN
        mov     ebp, esp
        push    dword [ebx+4]   ; for fat_get_name: read ANSI/UNICODE name
        mov     ebx, [ebx]
; init header
        push    eax ecx
        mov     edi, edx
        mov     ecx, 32/4
        xor     eax, eax
        rep     stosd
        pop     ecx eax
        mov     byte [edx], 1   ; version
        mov     esi, edi        ; esi points to BDFE
.new_cluster:
        mov     [cluster_tmp], eax
        test    eax, eax
        jnz     @f
        cmp     [fs_type], 32
        jz      .notfound
        mov     eax, [ROOT_START]
        push    [ROOT_SECTORS]
        push    ebx
        jmp     .new_sector
@@:
        dec     eax
        dec     eax
        imul    eax, [SECTORS_PER_CLUSTER]
        push    [SECTORS_PER_CLUSTER]
        add     eax, [DATA_START]
        push    ebx
.new_sector:
        mov     ebx, buffer
        mov     edi, ebx
        call    hd_read
        cmp     [hd_error], 0
        jnz     .notfound2
        add     ebx, 512
        push    eax
.l1:
        call    fat_get_name
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
        mov     eax, [cluster_tmp]
        test    eax, eax
        jz      .done
        call    get_FAT
        cmp     [hd_error], 0
        jnz     .notfound2
        cmp     eax, 2
        jb      .done
        cmp     eax, [fatRESERVED]
        jae     .done
        push    eax
        mov     eax, [SECTORS_PER_CLUSTER]
        mov     [esp+8], eax
        pop     eax
        mov     [cluster_tmp], eax
        dec     eax
        dec     eax
        imul    eax, [SECTORS_PER_CLUSTER]
        add     eax, [DATA_START]
@@:
        mov     ebx, buffer
        mov     edi, ebx
        call    hd_read
        cmp     [hd_error], 0
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
        call    fat_entry_to_bdfe
.l2:
        add     edi, 0x20
        cmp     edi, ebx
        jb      .l1
        pop     eax
        inc     eax
        dec     dword [esp+4]
        jnz     .new_sector
        mov     eax, [cluster_tmp]
        test    eax, eax
        jz      .done
        call    get_FAT
        cmp     [hd_error], 0
        jnz     .notfound2
        cmp     eax, 2
        jb      .done
        cmp     eax, [fatRESERVED]
        jae     .done
        push    eax
        mov     eax, [SECTORS_PER_CLUSTER]
        mov     [esp+8], eax
        pop     eax
        pop     ebx
        add     esp, 4
        jmp     .new_cluster
.notfound2:
        add     esp, 8
.notfound:
        add     esp, 262*2+4
        pop     ebp ecx esi edi
        mov     eax, ERROR_FILE_NOT_FOUND
        or      ebx, -1
        ret
.done:
        add     esp, 262*2+4+8
        pop     ebp
        mov     ebx, [edx+4]
        xor     eax, eax
        dec     ecx
        js      @f
        mov     al, ERROR_END_OF_FILE
@@:
        pop     ecx esi edi
        ret

fat16_root_next:
        cmp     edi, buffer+0x200-0x20
        jae     fat16_root_next_sector
        add     edi, 0x20
        ret     ; CF=0
fat16_root_next_sector:
; read next sector
        push    [longname_sec2]
        pop     [longname_sec1]
        push    ecx
        mov     ecx, [eax+4]
        push    ecx
        add     ecx, [ROOT_START]
        mov     [longname_sec2], ecx
        pop     ecx
        inc     ecx
        mov     [eax+4], ecx
        cmp     ecx, [ROOT_SECTORS]
        pop     ecx
        jae     fat16_root_first.readerr
fat16_root_first:
        mov     eax, [eax+4]
        add     eax, [ROOT_START]
        push    ebx
        mov     edi, buffer
        mov     ebx, edi
        call    hd_read
        pop     ebx
        cmp     [hd_error], 0
        jnz     .readerr
        ret     ; CF=0
.readerr:
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
        add     eax, [ROOT_START]
        mov     ebx, buffer
        call    hd_write
        popa
        ret
fat16_root_next_write:
        cmp     edi, buffer+0x200
        jae     @f
        ret
@@:
        call    fat16_root_end_write
        jmp     fat16_root_next_sector
fat16_root_extend_dir:
        stc
        ret

fat_notroot_next:
        cmp     edi, buffer+0x200-0x20
        jae     fat_notroot_next_sector
        add     edi, 0x20
        ret     ; CF=0
fat_notroot_next_sector:
        push    [longname_sec2]
        pop     [longname_sec1]
        push    eax
        call    fat_get_sector
        mov     [longname_sec2], eax
        pop     eax
        push    ecx
        mov     ecx, [eax+4]
        inc     ecx
        cmp     ecx, [SECTORS_PER_CLUSTER]
        jae     fat_notroot_next_cluster
        mov     [eax+4], ecx
        jmp     @f
fat_notroot_next_cluster:
        push    eax
        mov     eax, [eax]
        call    get_FAT
        mov     ecx, eax
        pop     eax
        cmp     [hd_error], 0
        jnz     fat_notroot_next_err
        cmp     ecx, [fatRESERVED]
        jae     fat_notroot_next_err
        mov     [eax], ecx
        and     dword [eax+4], 0
@@:
        pop     ecx
fat_notroot_first:
        call    fat_get_sector
        push    ebx
        mov     edi, buffer
        mov     ebx, edi
        call    hd_read
        pop     ebx
        cmp     [hd_error], 0
        jnz     @f
        ret     ; CF=0
fat_notroot_next_err:
        pop     ecx
@@:
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
        mov     ebx, buffer
        call    hd_write
        pop     ebx
        ret
fat_notroot_next_write:
        cmp     edi, buffer+0x200
        jae     @f
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
        mov     edx, [fatEND]
        call    set_FAT
        mov     edx, eax
        mov     eax, [esp+4]
        mov     eax, [eax]
        push    edx
        call    set_FAT
        pop     edx
        cmp     [hd_error], 0
        jz      @f
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
        mov     edi, buffer
        push    edi
        xor     eax, eax
        rep     stosd
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
        mov     ecx, [SECTORS_PER_CLUSTER]
        imul    eax, ecx
        add     eax, [DATA_START]
        mov     ebx, edi
@@:
        call    hd_write
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
        imul    ecx, [SECTORS_PER_CLUSTER]
        add     ecx, [DATA_START]
        add     ecx, [eax+4]
        mov     eax, ecx
        pop     ecx
        ret

;----------------------------------------------------------------
;
;  fs_HdRewrite - LFN variant for writing hard disk
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
fshrad:
        mov     eax, ERROR_ACCESS_DENIED
        xor     ebx, ebx
        ret
fshrfs:
        mov     eax, ERROR_UNKNOWN_FS
        xor     ebx, ebx
        ret

fs_HdCreateFolder:
        mov     al, 1
        jmp     fs_HdRewrite.common

fs_HdRewrite:
        xor     eax, eax
.common:
        cmp     [fs_type], 1
        jz      ntfs_HdRewrite
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jnz     fshrfs
@@:
        cmp     byte [esi], 0
        jz      fshrad
        pushad
        xor     edi, edi
        push    esi
        test    ebp, ebp
        jz      @f
        mov     esi, ebp
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
        test    ebp, ebp
        jnz     .hasebp
        mov     ebp, [ROOT_CLUSTER]
        cmp     [fs_type], 32
        jz      .pushnotroot
        push    fat16_root_extend_dir
        push    fat16_root_end_write
        push    fat16_root_next_write
        push    fat16_root_begin_write
        xor     ebp, ebp
        push    ebp
        push    ebp
        push    fat16_root_first
        push    fat16_root_next
        jmp     .common1
.hasebp:
        mov     eax, ERROR_ACCESS_DENIED
        cmp     byte [ebp], 0
        jz      .ret1
        push    ebp
        xor     ebp, ebp
        call    hd_find_lfn
        pop     esi
        jc      .notfound0
        jmp     .common0
.noroot:
        mov     eax, ERROR_ACCESS_DENIED
        cmp     byte [edi+1], 0
        jz      .ret1
; check existence
        mov     byte [edi], 0
        push    edi
        call    hd_find_lfn
        pop     esi
        mov     byte [esi], '/'
        jnc     @f
.notfound0:
        mov     eax, ERROR_FILE_NOT_FOUND
.ret1:
        mov     [esp+28], eax
        popad
        xor     ebx, ebx
        ret
@@:
        inc     esi
.common0:
        test    byte [edi+11], 0x10     ; must be directory
        mov     eax, ERROR_ACCESS_DENIED
        jz      .ret1
        mov     ebp, [edi+20-2]
        mov     bp, [edi+26]            ; ebp=cluster
        mov     eax, ERROR_FAT_TABLE
        cmp     ebp, 2
        jb      .ret1
.pushnotroot:
        push    fat_notroot_extend_dir
        push    fat_notroot_end_write
        push    fat_notroot_next_write
        push    fat_notroot_begin_write
        push    0
        push    ebp
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
        add     esp, 32
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
        cmp     byte [esp+32+28], 0
        jz      @f
        add     esp, 32
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
        cmp     eax, [fatRESERVED]
        jae     .done1
        push    edx
        xor     edx, edx
        call    set_FAT
        mov     eax, edx
        pop     edx
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
        add     esp, 32
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
        mov     [eax], ebp
        and     dword [eax+4], 0
        call    dword [eax-4]
        jc      .found
.test_short_name_entry:
        cmp     byte [edi+11], 0xF
        jz      .test_short_name_cont
        mov     ecx, 11
        push    esi edi
        repz    cmpsb
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
        add     esp, 12+32
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
        repnz   scasb
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
        mov     [eax], ebp
        and     dword [eax+4], 0
        call    dword [eax-4]
        pop     eax
        jnc     .scan_dir
.fsfrfe3:
        add     esp, 12+8+12+32
        popad
        mov     eax, 11
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
        pop     eax
        jnc     .scan_dir
        cmp     [hd_error], 0
        jnz     .fsfrfe3
        push    eax
        lea     eax, [esp+16+8+12+8]
        call    dword [eax+20]          ; extend directory
        pop     eax
        jnc     .scan_dir
        add     esp, 12+8+12+32
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
; calculate name checksum
        push    esi ecx
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
        rep     movsb
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
        cmp     byte [esp+32+28], cl
        jz      .doit
; create directory
        mov     byte [edi+11], 10h      ; attributes: folder
        mov     edx, edi
        lea     eax, [esp+8]
        call    dword [eax+16]  ; flush directory
        push    ecx
        mov     ecx, [SECTORS_PER_CLUSTER]
        shl     ecx, 9
        jmp     .doit2
.doit:
        lea     eax, [esp+8]
        call    dword [eax+16]  ; flush directory
        push    ecx
        mov     ecx, [esp+4+32+24]
.doit2:
        push    ecx
        push    edi
        mov     esi, edx
        test    ecx, ecx
        jz      .done
        call    get_free_FAT
        jc      .diskfull
        push    eax
        mov     [edi+26], ax
        shr     eax, 16
        mov     [edi+20], ax
        lea     eax, [esp+16+8]
        call    dword [eax+16]  ; flush directory
        pop     eax
        push    edx
        mov     edx, [fatEND]
        call    set_FAT
        pop     edx
.write_cluster:
        push    eax
        dec     eax
        dec     eax
        mov     ebp, [SECTORS_PER_CLUSTER]
        imul    eax, ebp
        add     eax, [DATA_START]
; write data
.write_sector:
        cmp     byte [esp+16+32+28], 0
        jnz     .writedir
        mov     ecx, 512
        cmp     dword [esp+8], ecx
        jb      .writeshort
; we can write directly from given buffer
        mov     ebx, esi
        add     esi, ecx
        jmp     .writecommon
.writeshort:
        mov     ecx, [esp+8]
        push    ecx
        mov     edi, buffer
        mov     ebx, edi
        rep     movsb
.writedircont:
        mov     ecx, buffer+0x200
        sub     ecx, edi
        push    eax
        xor     eax, eax
        rep     stosb
        pop     eax
        pop     ecx
.writecommon:
        call    hd_write
        cmp     [hd_error], 0
        jnz     .writeerr
        inc     eax
        sub     dword [esp+8], ecx
        jz      .writedone
        dec     ebp
        jnz     .write_sector
; allocate new cluster
        pop     eax
        mov     ecx, eax
        call    get_free_FAT
        jc      .diskfull
        push    edx
        mov     edx, [fatEND]
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
        pop     eax
        sub     esi, ecx
        mov     eax, 11
        jmp     .ret
.writedone:
        pop     eax
.done:
        xor     eax, eax
.ret:
        pop     edi ecx
        mov     ebx, esi
        sub     ebx, edx
        pop     ebp
        mov     [esp+32+28], eax
        lea     eax, [esp+8]
        call    dword [eax+8]
        mov     [edi+28], ebx
        call    dword [eax+16]
        mov     [esp+32+16], ebx
        lea     eax, [ebx+511]
        shr     eax, 9
        mov     ecx, [SECTORS_PER_CLUSTER]
        lea     eax, [eax+ecx-1]
        xor     edx, edx
        div     ecx
        mov     ecx, ebp
        sub     ecx, eax
        call    add_disk_free_space
        add     esp, 32
        call    update_disk
        popad
        ret
.writedir:
        push    512
        mov     edi, buffer
        mov     ebx, edi
        mov     ecx, [SECTORS_PER_CLUSTER]
        shl     ecx, 9
        cmp     ecx, [esp+12]
        jnz     .writedircont
        dec     dword [esp+16]
        push    esi
        mov     ecx, 32/4
        rep     movsd
        pop     esi
        mov     dword [edi-32], '.   '
        mov     dword [edi-32+4], '    '
        mov     dword [edi-32+8], '    '
        mov     byte [edi-32+11], 10h
        push    esi
        mov     ecx, 32/4
        rep     movsd
        pop     esi
        mov     dword [edi-32], '..  '
        mov     dword [edi-32+4], '    '
        mov     dword [edi-32+8], '    '
        mov     byte [edi-32+11], 10h
        mov     ecx, [esp+20+8]
        cmp     ecx, [ROOT_CLUSTER]
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
fs_HdWrite.access_denied:
        push    ERROR_ACCESS_DENIED
fs_HdWrite.ret0:
        pop     eax
        xor     ebx, ebx
        ret

fs_HdWrite.ret11:
        push    11
        jmp     fs_HdWrite.ret0

fs_HdWrite:
        cmp     [fs_type], 1
        jz      ntfs_HdWrite
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        push    ERROR_UNKNOWN_FS
        jmp     .ret0
@@:
        cmp     byte [esi], 0
        jz      .access_denied
        pushad
        call    hd_find_lfn
        pushfd
        cmp     [hd_error], 0
        jz      @f
        popfd
        popad
        push    11
        jmp     .ret0
@@:
        popfd
        jnc     .found
        popad
        push    ERROR_FILE_NOT_FOUND
        jmp     .ret0
.found:
; FAT does not support files larger than 4GB
        test    ebx, ebx
        jz      .l1
        cmp     dword [ebx+4], 0
        jz      @f
.eof:
        popad
        push    ERROR_END_OF_FILE
        jmp     .ret0
@@:
        mov     ebx, [ebx]
.l1:
; now edi points to direntry, ebx=start byte to write,
; ecx=number of bytes to write, edx=data pointer

; extend file if needed
        add     ecx, ebx
        jc      .eof    ; FAT does not support files larger than 4GB
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
        pop     eax
        pop     eax
        mov     [esp+4+28], eax
        pop     eax
        popad
        xor     ebx, ebx
        ret
.disk_full:
; correct number of bytes to write
        mov     ecx, [edi+28]
        cmp     ecx, ebx
        ja      .length_ok
.ret:
        call    update_disk
        cmp     [hd_error], 0
        jz      @f
        mov     byte [esp+4], 11
@@:
        pop     eax
        pop     eax
        mov     [esp+4+28], eax ; eax=return value
        pop     eax
        sub     edx, [esp+20]
        mov     [esp+16], edx   ; ebx=number of written bytes
        popad
        ret
.length_ok:
        mov     esi, [edi+28]
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]
        mov     edi, eax        ; edi=current cluster
        xor     ebp, ebp        ; ebp=current sector in cluster
; save directory
        mov     eax, [esp+8]
        push    ebx
        mov     ebx, buffer
        call    hd_write
        pop     ebx
        cmp     [hd_error], 0
        jz      @f
.device_err:
        mov     byte [esp+4], 11
        jmp     .ret
@@:

; now ebx=start pos, ecx=end pos, both lie inside file
        sub     ecx, ebx
        jz      .ret
.write_loop:
; skip unmodified sectors
        cmp     dword [esp], 0x200
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
        imul    eax, [SECTORS_PER_CLUSTER]
        add     eax, [DATA_START]
        add     eax, ebp
; load sector if needed
        cmp     dword [esp+4], 0        ; we don't need to read uninitialized data
        jz      .noread
        cmp     ecx, 0x200      ; we don't need to read sector if it is fully rewritten
        jz      .noread
        cmp     ecx, esi        ; (same for the last sector)
        jz      .noread
        push    ebx
        mov     ebx, buffer
        call    hd_read
        pop     ebx
        cmp     [hd_error], 0
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
        sub     ecx, [esp+4+12]
        jbe     @f
        mov     edi, buffer
        add     edi, [esp+4+12]
        rep     stosb
@@:
; zero uninitialized data in the last sector
        mov     ecx, 0x200
        sub     ecx, esi
        jbe     @f
        mov     edi, buffer
        add     edi, esi
        rep     stosb
@@:
        pop     edi ecx
; copy new data
        mov     eax, edx
        neg     ebx
        jecxz   @f
        add     ebx, buffer+0x200
        call    memmove
        xor     ebx, ebx
@@:
        pop     eax
; save sector
        push    ebx
        mov     ebx, buffer
        call    hd_write
        pop     ebx
        cmp     [hd_error], 0
        jnz     .device_err2
        add     edx, ecx
        sub     [esp], ecx
        pop     ecx
        jz      .ret
.skip:
; next sector
        inc     ebp
        cmp     ebp, [SECTORS_PER_CLUSTER]
        jb      @f
        xor     ebp, ebp
        mov     eax, edi
        call    get_FAT
        mov     edi, eax
        cmp     [hd_error], 0
        jnz     .device_err
@@:
        sub     esi, 0x200
        jae     @f
        xor     esi, esi
@@:
        sub     dword [esp], 0x200
        jae     @f
        and     dword [esp], 0
@@:     jmp     .write_loop

hd_extend_file.zero_size:
        xor     eax, eax
        jmp     hd_extend_file.start_extend

; extends file on hd to given size (new data area is undefined)
; in: edi->direntry, ecx=new size
; out: CF=0 => OK, eax=0
;      CF=1 => error, eax=code (ERROR_FAT_TABLE or ERROR_DISK_FULL or 11)
hd_extend_file:
        push    ebp
        mov     ebp, [SECTORS_PER_CLUSTER]
        imul    ebp, [BYTES_PER_SECTOR]
        push    ecx
; find the last cluster of file
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]
        mov     ecx, [edi+28]
        jecxz   .zero_size
.last_loop:
        sub     ecx, ebp
        jbe     .last_found
        call    get_FAT
        cmp     [hd_error], 0
        jz      @f
.device_err:
        pop     ecx
.device_err2:
        pop     ebp
        push    11
.ret_err:
        pop     eax
        stc
        ret
@@:
        cmp     eax, 2
        jb      .fat_err
        cmp     eax, [fatRESERVED]
        jb      .last_loop
.fat_err:
        pop     ecx ebp
        push    ERROR_FAT_TABLE
        jmp     .ret_err
.last_found:
        push    eax
        call    get_FAT
        cmp     [hd_error], 0
        jz      @f
        pop     eax
        jmp     .device_err
@@:
        cmp     eax, [fatRESERVED]
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
        mov     edx, [fatEND]
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
        cmp     [hd_error], 0
        jnz     .device_err3
        add     [edi+28], ebp
        jmp     .extend_loop
.extend_done:
        mov     [edi+28], ecx
        pop     edx ebp
        xor     eax, eax        ; CF=0
        ret
.device_err3:
        pop     edx
        jmp     .device_err2
.disk_full:
        pop     eax edx ebp
        push    ERROR_DISK_FULL
        pop     eax
        cmp     [hd_error], 0
        jz      @f
        mov     al, 11
@@:     stc
        ret

;----------------------------------------------------------------
;
;  fs_HdSetFileEnd - set end of file on hard disk
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
        cmp     [fs_type], 1
        jz      ntfs_HdSetFileEnd
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        push    ERROR_UNKNOWN_FS
.ret:
        pop     eax
        ret
@@:
        cmp     byte [esi], 0
        jnz     @f
.access_denied:
        push    ERROR_ACCESS_DENIED
        jmp     .ret
@@:
        push    edi
        call    hd_find_lfn
        pushfd
        cmp     [hd_error], 0
        jz      @f
        popfd
        push    11
        jmp     .ret
@@:
        popfd
        jnc     @f
        pop     edi
        push    ERROR_FILE_NOT_FOUND
        jmp     .ret
@@:
; must not be directory
        test    byte [edi+11], 10h
        jz      @f
        pop     edi
        jmp     .access_denied
@@:
; file size must not exceed 4 Gb
        cmp     dword [ebx+4], 0
        jz      @f
        pop     edi
        push    ERROR_END_OF_FILE
        jmp     .ret
@@:
        push    eax     ; save directory sector
; set file modification date/time to current
        call    fat_update_datetime
        mov     eax, [ebx]
        cmp     eax, [edi+28]
        jb      .truncate
        ja      .expand
        pop     eax
        mov     ebx, buffer
        call    hd_write
        pop     edi
        xor     eax, eax
        cmp     [hd_error], 0
        jz      @f
        mov     al, 11
@@:
        ret
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
        pop     eax ecx ebp ebx ecx edi edi
        ret
.expand_ok:
.disk_full:
; save directory
        mov     eax, [edi+28]
        xchg    eax, [esp+20]
        mov     ebx, buffer
        call    hd_write
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]
        mov     edi, eax
        cmp     [hd_error], 0
        jz      @f
.pop_ret11:
        mov     byte [esp], 11
        jmp     .pop_ret
@@:
; now zero new data
        xor     ebp, ebp
; edi=current cluster, ebp=sector in cluster
; [esp+20]=new size, [esp+4]=old size, [esp]=return code
.zero_loop:
        sub     dword [esp+4], 0x200
        jae     .next_cluster
        lea     eax, [edi-2]
        imul    eax, [SECTORS_PER_CLUSTER]
        add     eax, [DATA_START]
        add     eax, ebp
        cmp     dword [esp+4], -0x200
        jz      .noread
        mov     ebx, buffer
        call    hd_read
        cmp     [hd_error], 0
        jnz     .err_next
.noread:
        mov     ecx, [esp+4]
        neg     ecx
        push    edi
        mov     edi, buffer+0x200
        add     edi, [esp+8]
        push    eax
        xor     eax, eax
        mov     [esp+12], eax
        rep     stosb
        pop     eax
        pop     edi
        call    hd_write
        cmp     [hd_error], 0
        jz      .next_cluster
.err_next:
        mov     byte [esp], 11
.next_cluster:
        sub     dword [esp+20], 0x200
        jbe     .pop_ret
        inc     ebp
        cmp     ebp, [SECTORS_PER_CLUSTER]
        jb      .zero_loop
        xor     ebp, ebp
        mov     eax, edi
        call    get_FAT
        mov     edi, eax
        cmp     [hd_error], 0
        jnz     .pop_ret11
        jmp     .zero_loop
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
        mov     eax, [SECTORS_PER_CLUSTER]
        shl     eax, 9
        sub     [esp], eax
        jbe     @f
        mov     eax, ecx
        call    get_FAT
        mov     ecx, eax
        cmp     [hd_error], 0
        jz      @b
.device_err3:
        pop     eax ecx eax edi
        push    11
        pop     eax
        ret
@@:
; we will zero data at the end of last sector - remember it
        push    ecx
; terminate FAT chain
        push    edx
        mov     eax, ecx
        mov     edx, [fatEND]
        call    set_FAT
        mov     eax, edx
        pop     edx
        cmp     [hd_error], 0
        jz      @f
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
        cmp     [hd_error], 0
        jnz     .device_err4
; save directory
        mov     eax, [esp+12]
        push    ebx
        mov     ebx, buffer
        call    hd_write
        pop     ebx
        cmp     [hd_error], 0
        jnz     .device_err4
; zero last sector, ignore errors
        pop     ecx
        pop     eax
        dec     ecx
        imul    ecx, [SECTORS_PER_CLUSTER]
        add     ecx, [DATA_START]
        push    eax
        sar     eax, 9
        add     ecx, eax
        pop     eax
        and     eax, 0x1FF
        jz      .truncate_done
        push    ebx eax
        mov     eax, ecx
        mov     ebx, buffer
        call    hd_read
        pop     eax
        lea     edi, [buffer+eax]
        push    ecx
        mov     ecx, 0x200
        sub     ecx, eax
        xor     eax, eax
        rep     stosb
        pop     eax
        call    hd_write
        pop     ebx
.truncate_done:
        pop     ecx eax edi
        call    update_disk
        xor     eax, eax
        cmp     [hd_error], 0
        jz      @f
        mov     al, 11
@@:
        ret

fs_HdGetFileInfo:
        cmp     [fs_type], 1
        jz      ntfs_HdGetFileInfo
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        mov     eax, ERROR_UNKNOWN_FS
        ret
@@:
        cmp     byte [esi], 0
        jnz     @f
        mov     eax, 2
        ret
@@:
        push    edi
        call    hd_find_lfn
        pushfd
        cmp     [hd_error], 0
        jz      @f
        popfd
        pop     edi
        mov     eax, 11
        ret
@@:
        popfd
        jmp     fs_GetFileInfo_finish

fs_HdSetFileInfo:
        cmp     [fs_type], 1
        jz      ntfs_HdSetFileInfo
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        mov     eax, ERROR_UNKNOWN_FS
        ret
@@:
        cmp     byte [esi], 0
        jnz     @f
        mov     eax, 2
        ret
@@:
        push    edi
        call    hd_find_lfn
        pushfd
        cmp     [hd_error], 0
        jz      @f
        popfd
        pop     edi
        mov     eax, 11
        ret
@@:
        popfd
        jnc     @f
        pop     edi
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
@@:
        push    eax
        call    bdfe_to_fat_entry
        pop     eax
        mov     ebx, buffer
        call    hd_write
        call    update_disk
        pop     edi
        xor     eax, eax
        ret

;----------------------------------------------------------------
;
;  fs_HdDelete - delete file or empty folder from hard disk
;
;  esi  points to filename
;
;  ret  eax = 0 ok or other = errormsg
;
;--------------------------------------------------------------
fs_HdDelete:
        cmp     [fs_type], 1
        jz      ntfs_HdDelete
        cmp     [fs_type], 16
        jz      @f
        cmp     [fs_type], 32
        jz      @f
        push    ERROR_UNKNOWN_FS
.pop_ret:
        pop     eax
        ret
@@:
        cmp     byte [esi], 0
        jnz     @f
; cannot delete root!
.access_denied:
        push    ERROR_ACCESS_DENIED
        jmp     .pop_ret
@@:
        and     [longname_sec1], 0
        and     [longname_sec2], 0
        push    edi
        call    hd_find_lfn
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
        mov     ebp, [edi+20-2]
        mov     bp, [edi+26]
        xor     ecx, ecx
        lea     eax, [ebp-2]
        imul    eax, [SECTORS_PER_CLUSTER]
        add     eax, [DATA_START]
        mov     ebx, buffer
        call    hd_read
        cmp     [hd_error], 0
        jnz     .err1
        add     ebx, 2*0x20
.checkempty:
        cmp     byte [ebx], 0
        jz      .empty
        cmp     byte [ebx], 0xE5
        jnz     .notempty
        add     ebx, 0x20
        cmp     ebx, buffer+0x200
        jb      .checkempty
        inc     ecx
        cmp     ecx, [SECTORS_PER_CLUSTER]
        jb      @f
        mov     eax, ebp
        call    get_FAT
        cmp     [hd_error], 0
        jnz     .err1
        mov     ebp, eax
        xor     ecx, ecx
@@:
        lea     eax, [ebp-2]
        imul    eax, [SECTORS_PER_CLUSTER]
        add     eax, [DATA_START]
        add     eax, ecx
        mov     ebx, buffer
        call    hd_read
        cmp     [hd_error], 0
        jz      .checkempty
.err1:
        popad
.err2:
        pop     edi
        push    11
        pop     eax
        ret
.notempty:
        popad
.access_denied2:
        pop     edi
        push    ERROR_ACCESS_DENIED
        pop     eax
        ret
.empty:
        popad
        push    ebx
        mov     ebx, buffer
        call    hd_read
        pop     ebx
        cmp     [hd_error], 0
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
        cmp     edi, buffer
        ja      @f
        cmp     [longname_sec2], 0
        jz      .lfndone
        push    [longname_sec2]
        push    [longname_sec1]
        pop     [longname_sec2]
        and     [longname_sec1], 0
        push    ebx
        mov     ebx, buffer
        call    hd_write
        mov     eax, [esp+4]
        call    hd_read
        pop     ebx
        pop     eax
        mov     edi, buffer+0x200
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
        mov     ebx, buffer
        call    hd_write
        pop     ebx
; delete FAT chain
        pop     eax
        call    clear_cluster_chain
        call    update_disk
        pop     edi
        xor     eax, eax
        cmp     [hd_error], 0
        jz      @f
        mov     al, 11
@@:
        ret

; \end{diamond}
