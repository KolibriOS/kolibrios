;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;  FAT32.INC                                                      ;;
;;                                                                 ;;
;;  FAT16/32 functions for MenuetOS                                ;;
;;                                                                 ;;
;;  Copyright 2002 Paolo Minazzi, paolo.minazzi@inwind.it          ;;
;;                                                                 ;;
;;  See file COPYING for details                                   ;;
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

cluster              dd 0       ; used by file_write,makedir,append
partition_count      dd 0       ; partitions found by set_FAT32_variables
longname_sec1        dd 0       ; used by analyze_directory to save 2 previous
longname_sec2        dd 0       ; directory sectors for delete long filename

hd_error             dd 0       ; set by wait_for_sector_buffer
hd_setup             dd 0
hd_wait_timeout      dd 0

cluster_tmp          dd 0       ; used by analyze_directory
                                ; and analyze_directory_to_write

file_size            dd 0       ; used by file_read

sector_tmp           dd 0       ; used by rename,append,file_write
entry_pos            dd 0       ; used by rename,append,file_write

old_filesize         dd 0       ; used by append
new_filepos          dd 0       ; used by append
bytes2write          dd 0       ; used by append

cache_search_start   dd 0       ; used by find_empty_slot

fat_in_cache         dd -1
fat_cache:           times 512 db 0

uglobal
 Sector512:                      ; label for dev_hdcd.inc
  buffer:              times 512 db 0
  deltree_buffer:      times 512 db 0
endg

iglobal
  NewDirEntry1         db ".          ",0x10
                     times 20 db 0
  NewDirEntry2         db "..         ",0x10
                     times 20 db 0
endg

uglobal
  dir_entry:           times 32 db 0

  startpath:           times 255 db 0

  fat16_root           db 0       ; flag for fat16 rootdir
  f_del                db 0       ; 1=overwrite fat entry
  fat_change           db 0       ; 1=fat has changed

endg

reserve_hd1:

    cli
    cmp   [hd1_status],0
    je    reserve_ok1

    sti
    call  change_task
    jmp   reserve_hd1

  reserve_ok1:

    push  eax
    mov   eax,[0x3000]
    shl   eax,5
    mov   eax,[eax+0x3000+4]
    mov   [hd1_status],eax
    pop   eax
    sti
    ret
;********************************************
reserve_hd_channel:
    cmp   [hdbase], 0x1F0
    jne   .IDE_Channel_2
.IDE_Channel_1:
    cli
    cmp   [IDE_Channel_1],0
    je    .reserve_ok_1
    sti
    call  change_task
    jmp   .IDE_Channel_1
.IDE_Channel_2:
    cli
    cmp   [IDE_Channel_2],0
    je    .reserve_ok_2
    sti
    call  change_task
    jmp   .IDE_Channel_1
.reserve_ok_1:
    mov [IDE_Channel_1],1
    ret
.reserve_ok_2:
    mov [IDE_Channel_2],1
    ret
    
free_hd_channel:
    cmp   [hdbase], 0x1F0
    jne   .IDE_Channel_2
.IDE_Channel_1:
    mov [IDE_Channel_1],0
    ret
.IDE_Channel_2:
    mov [IDE_Channel_2],0
    ret
;********************************************
clear_hd_cache:

    push  eax ecx edi
    mov   edi,0x600000
    mov   ecx,16384
    xor   eax,eax
    cld
    rep   stosd                 ; clear hd cache with 0
    mov   [cache_search_start],eax
    mov   [fat_in_cache],-1
    mov   [fat_change],0
    pop   edi ecx eax
    ret

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
    cmp   [fat_type],16
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
    cmp   [fat_type],16
    jne   sfc_test32

    cmp   [f_del],1             ; overwrite previous value?
    je    sfc_set16             ; yes
    cmp   word [ebx+esi],0      ; is cluster free?
    je    sfc_set16             ; yes
    mov   dword [8*0x100000],0xffffff
    mov   edx,[ebx+esi]         ; get old value
    jmp   sfc_nonzero

  sfc_set16:
    xchg  [ebx+esi],dx          ; save new value and get old value
    jmp   sfc_write

  sfc_test32:
    mov   eax,[fatMASK]
    cmp   [f_del],1             ; overwrite previous value?
    je    sfc_set32             ; yes
    test  eax,[ebx+esi]         ; is cluster free?
    je    sfc_set32             ; yes
    mov   dword [8*0x100000],0xffffff
    mov   edx,[ebx+esi]         ; get old value
    jmp   sfc_nonzero

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

    cmp   [fat_type],16
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
; input  : EAX = # cluster for start the searching
; output : if CARRY=0 EAX = # first cluster found free
;          if CARRY=1 disk full
; Note   : for more speed need to use fat_cache directly
;-----------------------------------------------------------
    push  ecx
    mov   ecx,[LAST_CLUSTER]    ; counter for full disk
    sub   ecx,2

  gff_test:
    cmp   eax,[LAST_CLUSTER]    ; if above last cluster start at cluster 2
    jbe   gff_in_range
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
    cmp   [fat_type],16
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


analyze_directory_to_write:
;-----------------------------------------------------------
; input  : EAX = first cluster of the directory
; output : IF CARRY=0 EAX = sector where the empty pos is found
;                     EBX = pointer in buffer
;                     [buffer .. buffer+511]
;                     ECX,EDX,ESI,EDI not changed
;          IF CARRY=1 disk full or fat corrupted
; Note   : if cluster=0 it's changed to read rootdir
;-----------------------------------------------------------
    push  ecx edx edi

  adw_new_cluster:
    mov   [cluster_tmp],eax
    mov   [fat16_root],0
    cmp   eax,[LAST_CLUSTER]
    ja    adw_not_found         ; too big cluster number, something is wrong
    cmp   eax,2
    jnb   adw_data_cluster

    mov   eax,[ROOT_CLUSTER]    ; if cluster < 2 then read rootdir
    cmp   [fat_type],16
    jne   adw_data_cluster
    mov   eax,[ROOT_START]
    mov   edx,[ROOT_SECTORS]
    mov   [fat16_root],1        ; flag for fat16 rootdir
    jmp   adw_new_sector

  adw_data_cluster:
    sub   eax,2
    mov   edx,[SECTORS_PER_CLUSTER]
    imul  eax,edx
    add   eax,[DATA_START]

  adw_new_sector:
    mov   ebx,buffer
    call  hd_read
    cmp  [hd_error],0
    jne  adw_not_found

    mov   ecx,512/32            ; count of dir entrys per sector = 16

  adw_analyze:
    cmp   byte [ebx],0x00       ; is free entry?
    je    adw_found             ; yes
    cmp   byte [ebx],0xe5       ; is deleted entry?
    je    adw_found             ; yes
    add   ebx,32                ; position of next dir entry
    dec   ecx
    jnz   adw_analyze

    inc   eax                   ; next sector
    dec   edx
    jne   adw_new_sector
    cmp   [fat16_root],1        ; end of fat16 rootdir
    je    adw_not_found

    mov   eax,[cluster_tmp]
    call  get_FAT               ; get next cluster
    cmp   [hd_error],0
    jne   adw_not_found

    cmp   eax,2                 ; incorrect fat chain?
    jb    adw_not_found         ; yes
    cmp   eax,[fatRESERVED]     ; is it end of directory?
    jb    adw_new_cluster       ; no. analyse it

    mov   eax,2                 ; this block of code add a new cluster
    call  get_free_FAT          ; for the directory because the directory
    jc    adw_not_found         ; is full

    mov   edx,[fatEND]          ; new end for directory
    call  set_FAT
    cmp  [hd_error],0
    jne  adw_not_found

    push  eax                   ; save new cluster
    mov   edx,eax
    mov   eax,[cluster_tmp]     ; change last cluster to point new cluster
    mov   [f_del],1
    call  set_FAT
    cmp  [hd_error],0
    jne  adw_not_found_1

    mov   [f_del],0

    mov   ecx,-1                ; remove 1 cluster from free disk space
    call  add_disk_free_space
    cmp  [hd_error],0
    jne    adw_not_found_1

    mov   ecx,512/4
    xor   eax,eax
    mov   edi,buffer
    cld
    rep   stosd                 ; clear new directory cluster
    pop   eax

    sub   eax,2
    mov   ecx,[SECTORS_PER_CLUSTER]
    imul  eax,ecx
    add   eax,[DATA_START]
    mov   ebx,buffer
    push  eax                   ; save sector number

  adw_set_empty_directory:
    call  hd_write
    cmp   [hd_error],0
    jne   adw_not_found_1

    inc   eax                   ; next sector
    dec   ecx
    jnz   adw_set_empty_directory

    pop   eax

  adw_found:
    pop   edi edx ecx
    clc                         ; free space found
    ret
  adw_not_found_1:
    add  esp,4
  adw_not_found:
    pop   edi edx ecx
    stc                         ; free space not found
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
    cmp   [fat_type],16
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


set_data_cluster:
;-----------------------------------------------------------
; input  : EAX = cluster
;          EBX = pointer to buffer
; output : if CARRY=0 ok
;          if CARRY=1 cluster out of range
;-----------------------------------------------------------
    push  eax ebx edx

    cmp   eax,[LAST_CLUSTER]
    ja    sdc_error             ; too big cluster number, something is wrong
    sub   eax,2
    jb    sdc_error             ; don't allow rootdir write

    mov   edx,[SECTORS_PER_CLUSTER]
    imul  eax,edx
    add   eax,[DATA_START]

  sdc_write:
    call  hd_write
    cmp   [hd_error],0
    jne   sdc_error

    add   ebx,512               ; update pointer
    inc   eax
    dec   edx
    jnz   sdc_write
    pop   edx ebx eax
    clc
    ret

  sdc_error:
    pop   edx ebx eax
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


makedir:
;-----------------------------------------------------
; input  : eax = directory name
;          edx = path
; output : eax = 0 - ok
;                3 - unknown FS
;                5 - file not found
;                8 - disk full
;               10 - access denied
; Note   : can only make one directory at time
;-----------------------------------------------------
    cmp   [fat_type],0
    jnz   make_dir_fat_ok
    mov   eax,ERROR_UNKNOWN_FS
    ret

  make_dir_fat_ok:
;    call  reserve_hd1

    pushad

    mov   ebx,edx
    call  get_cluster_of_a_path
    jnc   make_dir_found_path
    cmp   [hd_error],0
    jne   make_dir_error_1

  make_dir_path_not_found:
    popad
    call  update_disk           ; write all of cache and fat to hd
    cmp   [hd_error],0
    jne   make_dir_error_2

    mov   [hd1_status],0
    mov   eax,ERROR_FILE_NOT_FOUND
    ret

  make_dir_disk_full:
    cmp  [hd_error],0
    jne   make_dir_error_1
    popad
    call  update_disk           ; write all of cache and fat to hd
    cmp   [hd_error],0
    jne   make_dir_error_2

    mov   [hd1_status],0
    mov   eax,ERROR_DISK_FULL
    ret

  make_dir_already_exist:
    cmp  [hd_error],0
    jne   make_dir_error_1    
    mov   eax,[cluster]         ; directory cluster
    xor   edx,edx               ; free
    mov   [f_del],1
    call  set_FAT
    cmp  [hd_error],0
    jne   make_dir_error_1

    mov   [f_del],0

    popad
    call  update_disk           ; write all of cache and fat to hd
  make_dir_error_2:
    mov   [hd1_status],0
    mov   eax,ERROR_ACCESS_DENIED
    ret

  make_dir_error_1:
    popad
    jmp   make_dir_error_2

  make_dir_error_3:
    add  esp,4
    jmp   make_dir_error_1

  make_dir_found_path:
    cmp   eax,[ROOT_CLUSTER]
    jnz   make_dir_not_root
    xor   eax,eax

  make_dir_not_root:
    mov   ecx,eax               ; directorys start cluster
    mov   word [NewDirEntry2+26],cx ; 16 bits low of cluster
    shr   ecx,16
    mov   word [NewDirEntry2+20],cx ; 16 bits high of cluster (=0 fat16)

    push  eax                   ; save parent directory cluster
    mov   eax,2
    call  get_free_FAT
    mov   [cluster],eax         ; first free cluster
    pop   eax
    jc    make_dir_disk_full

    push  eax
    mov   eax,[cluster]         ; directory cluster
    mov   edx,[fatEND]          ; end for directory
    call  set_FAT
    cmp  [hd_error],0
    jne   make_dir_error_3
    pop   eax

    mov   ebx,PUSHAD_EAX        ; dir name
    push  eax
    call  analyze_directory     ; check if directory already exist
    cmp   [hd_error],0
    jne   make_dir_error_1

    pop   eax
    jnc   make_dir_already_exist ; need to free allocated cluster!

    call  analyze_directory_to_write
    jc    make_dir_already_exist ; need to free allocated cluster!

    mov   esi,PUSHAD_EAX        ; dir name
    mov   edi,ebx               ; pointer in buffer
    mov   ecx,11
    cld
    rep   movsb

    mov   dword [ebx+28],0      ; dir size is always 0
    mov   ecx,[cluster]
    mov   [ebx+26],cx           ; 16 bits low of cluster
    mov   word [NewDirEntry1+26],cx
    shr   ecx,16
    mov   [ebx+20],cx           ; 16 bits high of cluster (=0 fat16)
    mov   word [NewDirEntry1+20],cx
    mov   byte [ebx+11],0x10    ; attribute = directory

    call  set_current_time_for_entry
    mov   ecx,[ebx+22]
    mov   dword [NewDirEntry1+22],ecx
    mov   dword [NewDirEntry2+22],ecx

    mov   ebx,buffer            ; save the directory name,length,cluster
    call  hd_write
    cmp   [hd_error],0
    jne   make_dir_error_1

    mov   ecx,512/4
    xor   eax,eax
    mov   edi,buffer
    cld
    rep   stosd                 ; clear new directory cluster

    mov   eax,[cluster]         ; new directory cluster
    sub   eax,2
    mov   edx,[SECTORS_PER_CLUSTER]
    imul  eax,edx
    add   eax,[DATA_START]
    mov   ebx,buffer
    add   eax,edx               ; start from last sector

  dir_set_empty_directory:
    dec   eax                   ; next sector
    cmp   edx,1                 ; is first directory sector?
    jnz   not_first_sector      ; no. write empty sector
    mov   esi,NewDirEntry1
    mov   edi,buffer
    mov   ecx,64/4
    cld
    rep   movsd                 ; copy 2 first directory entrys "." and ".."

  not_first_sector:
    call  hd_write
    cmp   [hd_error],0
    jne   make_dir_error_1
  
    dec   edx
    jnz   dir_set_empty_directory

    mov   ecx,-1                ; remove 1 cluster from free disk space
    call  add_disk_free_space
    cmp  [hd_error],0
    jne   make_dir_error_1

    popad
    call  update_disk           ; write all of cache and fat to hd
    cmp   [hd_error],0
    jne   make_dir_error_2
    mov   [hd1_status],0
    xor   eax,eax
    ret


removedir:
;-----------------------------------------------------
; input  : eax = file/directory name
;          edx = path
; output : eax = 0 - ok
;                3 - unknown FS
;                5 - file not found
;               10 - access denied
;-----------------------------------------------------
    cmp   [fat_type],0
    jnz   remove_dir_fat_ok
    mov   eax,ERROR_UNKNOWN_FS
    ret

  remove_dir_fat_ok:
;    call  reserve_hd1

    push  edi
    mov   edi,1                 ; allow directory remove
    call  file_delete
    cmp   [hd_error],0
    jne   @f

    pop   edi

    call  update_disk           ; write all of cache and fat to hd
  @@:
    mov   [hd1_status],0
    ret
   

add_disk_free_space:
;-----------------------------------------------------
; input  : ecx = cluster count
; Note   : negative = remove clusters from free space
;          positive = add clusters to free space
;-----------------------------------------------------
    test  ecx,ecx               ; no change
    je    add_dfs_no
    cmp   [fat_type],32         ; free disk space only used by fat32
    jne   add_dfs_no

    push  eax ebx
    mov   eax,[ADR_FSINFO]
    mov   ebx,buffer
    call  hd_read
    cmp  [hd_error],0
    jne  add_not_fs

    cmp   dword [ebx+0x1fc],0xaa550000 ; check sector id
    jne   add_not_fs

    add   [ebx+0x1e8],ecx
    call  hd_write
    cmp   [hd_error],0
    jne   add_not_fs

  add_not_fs:
    pop   ebx eax

  add_dfs_no:
    ret


file_append:
;-----------------------------------------------------
; input  : eax = file name
;          edx = path
;          ecx = pointer to buffer
;          ebx = bytes to write (0 = truncate file)
;          esi = start position (-1 = end of file)
; output : eax = 0 - ok
;                3 - unknown FS
;                5 - file not found
;                6 - end of file
;                8 - disk full
;                9 - fat table corrupted
;               10 - access denied
;          ebx = bytes written
;-----------------------------------------------------
    cmp   [fat_type],0
    jnz   append_fat_ok
    mov   eax,ERROR_UNKNOWN_FS
    ret

  append_fat_ok:
;    call  reserve_hd1

    pushad

    mov   ebx,edx
    call  get_cluster_of_a_path
    jc    append_not_found

    mov   ebx,PUSHAD_EAX        ; file name
    call  analyze_directory
    jc    append_not_found

    mov   [sector_tmp],eax
    mov   [entry_pos],ebx

    test  byte [ebx+11],0x10    ; is it directory?
    jnz   append_access         ; yes

    mov   ecx,[ebx+28]          ; file size
    mov   edi,PUSHAD_ESI        ; write position
    cmp   edi,-1                ; -1 = eof
    jnz   append_inside_file
    mov   edi,ecx               ; file size

  append_inside_file:
    cmp   edi,ecx               ; start above old file size?
    ja    append_eof            ; yes

    mov   [old_filesize],ecx
    mov   [new_filepos],edi

    mov   ecx,PUSHAD_EBX        ; bytes to write
    test  ecx,ecx               ; truncate?
    jz    append_truncate       ; yes

    mov   [bytes2write],ecx     ; bytes to write
    mov   esi,PUSHAD_ECX        ; pointer to buffer
    mov   eax,[ebx+20-2]        ; FAT entry
    mov   ax,[ebx+26]
    and   eax,[fatMASK]
    jnz   append_find_pos       ; first cluster <> 0

    mov   eax,2
    call  get_free_FAT
    jc    append_disk_full
    mov   ecx,eax               ; set files first cluster
    mov   [ebx+26],cx           ; 16 bits low of cluster
    shr   ecx,16
    mov   [ebx+20],cx           ; 16 bits high of cluster (=0 fat16)
    mov   edx,[fatEND]          ; new end for cluster chain
    call  set_FAT
    cmp  [hd_error],0
    jne   append_access

    push  eax                   ; save first cluster
    mov   eax,[sector_tmp]
    mov   ebx,buffer
    call  hd_write              ; write new file entry back to disk
    cmp   [hd_error],0
    jne   append_access_1

    pop   eax

  append_remove_free:
    mov   ecx,-1                ; remove 1 cluster from free disk space
    call  add_disk_free_space   ; Note: uses buffer
    cmp  [hd_error],0
    jne   append_access

  append_found_cluster:
    mov   [cluster],eax
    sub   eax,2
    mov   ecx,[SECTORS_PER_CLUSTER]
    imul  eax,ecx
    add   eax,[DATA_START]
    xor   edi,edi

  append_new_sector:
    cmp   [hd_error],0
    jne   append_access    
    push  ecx
    mov   ecx,[bytes2write]     ; bytes left in buffer
    mov   ebx,512
    sub   ebx,edi               ; bytes left in sector
    cmp   ecx,ebx
    jb    append_bytes_ok
    mov   ecx,ebx

  append_bytes_ok:
    cmp   ecx,512               ; overwrite full sector?
    jz    append_full_sector    ; yes
    mov   ebx,buffer            ; overwrite part of sector
    call  hd_read               ; read old sector
    cmp  [hd_error],0
    jne  append_access_1

  append_full_sector:
    sub   [bytes2write],ecx
    add   [new_filepos],ecx
    add   edi,buffer
    cld
    rep   movsb
    pop   ecx

    mov   ebx,buffer
    call  hd_write
    cmp   [hd_error],0
    jne   append_access

    cmp   [bytes2write],0       ; is all done?
    jz    append_done
    xor   edi,edi
    inc   eax
    dec   ecx
    jnz   append_new_sector

    mov   eax,[cluster]
    call  get_FAT
    cmp   [hd_error],0
    jne   append_access

    cmp   eax,2
    jb    append_fat
    cmp   eax,[LAST_CLUSTER]
    jbe   append_found_cluster

  append_alloc_cluster:
    mov   eax,2                 ; ToDo: use temp array to keep track
    call  get_free_FAT          ;       of last free cluster
    jc    append_disk_full
    push  eax                   ; save new cluster
    mov   edx,[fatEND]          ; new end for cluster chain
    call  set_FAT
    cmp  [hd_error],0
    jne  append_access_1
      
    mov   edx,eax
    mov   eax,[cluster]
    mov   [f_del],1
    call  set_FAT               ; update previous cluster
    cmp  [hd_error],0
    jne  append_access_1

    mov   [f_del],0
    pop   eax
    jmp   append_remove_free

  append_find_pos:
    call  find_filepos
    mov   [cluster],ebx
    jnc   append_new_sector
    test  edi,edi
    jz    append_alloc_cluster

  append_fat:
    mov   eax,ERROR_FAT_TABLE
    jmp   append_ret_code
                        
  append_disk_full:
    cmp  [hd_error],0
    jne  append_access
    mov   eax,ERROR_DISK_FULL
    jmp   append_ret_code

  append_done:
    xor   eax,eax

  append_ret_code:
    mov   PUSHAD_EAX,eax        ; return code

    mov   eax,[sector_tmp]      ; update directory entry
    mov   ebx,buffer
    call  hd_read
    cmp  [hd_error],0
    jne  append_access
     
    mov   ebx,[entry_pos]
    mov   ecx,[new_filepos]
    cmp   ecx,[old_filesize]    ; is file pos above old size?
    jbe   append_size_ok        ; no
    mov   [ebx+28],ecx          ; new file size

  append_size_ok:
    call  set_current_time_for_entry
    mov   ebx,buffer
    call  hd_write              ; write new file entry back to disk
    cmp   [hd_error],0
    jne   append_access

    sub   ecx,PUSHAD_ESI        ; start position
    mov   PUSHAD_EBX,ecx        ; bytes written
    popad
    call  update_disk           ; write all of cache and fat to hd
    cmp   [hd_error],0
    jne   append_access_2

    mov   [hd1_status],0
    ret

  append_eof:
    popad
    mov   [hd1_status],0
    xor   ebx,ebx
    mov   eax,ERROR_END_OF_FILE
    ret

  append_not_found:
    cmp  [hd_error],0
    jne   append_access
    popad
    mov   [hd1_status],0
    xor   ebx,ebx
    mov   eax,ERROR_FILE_NOT_FOUND
    ret        
  append_access_1:
    add   esp,4
  append_access:
    popad
  append_access_2:
    mov   [hd1_status],0
    xor   ebx,ebx
    mov   eax,ERROR_ACCESS_DENIED
    ret

  append_truncate:
    mov   edx,[ebx+20-2]        ; FAT entry
    mov   dx,[ebx+26]
    and   edx,[fatMASK]
    mov   [ebx+28],edi          ; set new file size
    test  edi,edi               ; 0 length file?
    jnz   truncate_save_size    ; no
    mov   [ebx+20],di           ; FAT entry = 0
    mov   [ebx+26],di

  truncate_save_size:
    call  set_current_time_for_entry
    mov   ebx,buffer
    call  hd_write
    cmp   [hd_error],0
    jne   append_access

    mov   eax,edx               ; first cluster
    test  edi,edi               ; 0 length file?
    jz    truncate_clear_chain

    imul  esi,[SECTORS_PER_CLUSTER],512 ; esi = cluster size in bytes

  truncate_new_cluster:
    cmp   eax,2                 ; incorrect fat chain?
    jb    truncate_eof          ; yes
    cmp   eax,[fatRESERVED]     ; is it end of file?
    jnb   truncate_eof          ; yes
    sub   edi,esi
    jbe   truncate_pos_found
    call  get_FAT               ; get next cluster
    cmp   [hd_error],0
    jne   append_access

    jmp   truncate_new_cluster

  truncate_pos_found:
    mov   edx,[fatEND]          ; new end for cluster chain
    mov   [f_del],1
    call  set_FAT
    cmp  [hd_error],0
    jne  append_access

    mov   [f_del],0
    mov   eax,edx               ; clear rest of chain

  truncate_clear_chain:
    call  clear_cluster_chain
    cmp  [hd_error],0
    jne  append_access

  truncate_eof:
    popad
    call  update_disk           ; write all of cache and fat to hd
    cmp   [hd_error],0
    jne   append_access_2

    mov   [hd1_status],0
    xor   ebx,ebx
    xor   eax,eax
    ret


find_filepos:
;-----------------------------------------------------
; input  : eax = first cluster
;          edi = bytes to skip over (start position)
; output : if CARRY=0 file position found
;          if CARRY=1 end of file found
;          eax = current file sector
;          ebx = last cluster
;          ecx = sector count in last cluster
;          edi = bytes to skip over (sector position)
;-----------------------------------------------------
    push  esi
    mov   ecx,[SECTORS_PER_CLUSTER]
    imul  esi,ecx,512           ; esi = cluster size in bytes
    mov   ebx,eax

  filepos_new_cluster:
    cmp   eax,2                 ; incorrect fat chain?
    jb    filepos_eof           ; yes
    cmp   eax,[fatRESERVED]     ; is it end of file?
    jnb   filepos_eof           ; yes

    mov   ebx,eax
    cmp   edi,esi               ; skip over full cluster?
    jb    filepos_cluster_ok    ; no

    sub   edi,esi
    call  get_FAT               ; get next cluster
    cmp   [hd_error],0
    jne   filepos_eof

    jmp   filepos_new_cluster

  filepos_cluster_ok:
    sub   eax,2
    imul  eax,ecx
    add   eax,[DATA_START]

  filepos_new_sector:
    cmp   edi,512               ; skip over full sector?
    jb    filepos_sector_ok     ; no
    sub   edi,512
    inc   eax
    dec   ecx
    jnz   filepos_new_sector

  filepos_eof:
    pop   esi
    stc
    ret

  filepos_sector_ok:
    pop   esi
    clc
    ret


file_write:
;--------------------------------------------------------------------------
;   INPUT : user-reg  register-in-this  meaning      symbol-in-this-routine
;
;            EAX        EDI            system call to write    /
;            EBX        EAX   (PAR0)   pointer to file-name    PAR0
;            EDX        ECX   (PAR1)   pointer to buffer       PAR1
;            ECX        EBX   (PAR2)   file size               PAR2
;            ESI        EDX   (PAR3)   pointer to path         PAR3
;
; output : eax = 0 - ok
;                3 - unknown FS
;                5 - file not found
;                8 - disk full
;               10 - access denied
;--------------------------------------------------------------------------
    cmp   [fat_type],0
    jnz   fat_ok_for_writing
    mov   eax,ERROR_UNKNOWN_FS
    ret

  fat_ok_for_writing:
;    call  reserve_hd1

    pushad

    xor   edi,edi               ; don't allow directory remove
    call  file_delete           ; try to delete the file first
    cmp   [hd_error],0
    jne   exit_write_access_1

    test  eax,eax
    jz    old_deleted           ; deleted ok
    cmp   eax,ERROR_FILE_NOT_FOUND
    jnz   exit_write_access     ; it exist but can't delete

  old_deleted:
    mov   ebx,PUSHAD_EDX
    call  get_cluster_of_a_path
    jnc   found_directory_for_writing
    cmp   [hd_error],0
    jne   exit_write_access

  exit_writing_with_error:
    popad
    call  update_disk           ; write all of cache and fat to hd
    cmp   [hd_error],0
    jne   exit_write_access_2

    mov   [hd1_status],0
    mov   eax,ERROR_FILE_NOT_FOUND
    ret

  exit_writing_disk_full_clear:
    cmp  [hd_error],0
    jne  exit_write_access_1
    mov   eax,[sector_tmp]
    mov   ebx,buffer
    call  hd_read               ; read directory sector
    cmp  [hd_error],0
    jne  exit_write_access_1

    mov   edx,[entry_pos]
    mov   byte [edx],0xe5       ; mark as deleted
    call  hd_write
    cmp   [hd_error],0
    jne   exit_write_access_1

    mov   eax,[edx+20-2]        ; FAT entry
    mov   ax,[edx+26]
    and   eax,[fatMASK]
    call  clear_cluster_chain

  exit_writing_disk_full:
    cmp  [hd_error],0
    jne  exit_write_access_1
    popad
    call  update_disk           ; write all of cache and fat to hd
    cmp   [hd_error],0
    jne   exit_write_access_2
    mov   [hd1_status],0
    mov   eax,ERROR_DISK_FULL
    ret

  exit_write_access:
    popad
    call  update_disk           ; write all of cache and fat to hd
    mov   [hd1_status],0
    mov   eax,ERROR_ACCESS_DENIED
    ret

  exit_write_access_1:
    popad
  exit_write_access_2:
    mov   [hd1_status],0
    mov   eax,ERROR_ACCESS_DENIED
    ret

found_directory_for_writing:
    call  analyze_directory_to_write
    jc    exit_writing_disk_full

    mov   [sector_tmp],eax
    mov   [entry_pos],ebx
    push  eax                   ; save directory sector
    mov   eax,2
    call  get_free_FAT
    mov   [cluster],eax         ; first free cluster
    pop   eax
    jc    exit_writing_disk_full

    mov   esi,PUSHAD_EAX        ; file name
    mov   edi,ebx               ; pointer in buffer
    mov   ecx,11
    cld
    rep   movsb

    mov   esi,PUSHAD_EBX        ; file size (bytes left)
    mov   [ebx+28],esi          ; file size
    mov   ecx,[cluster]
    mov   [ebx+26],cx           ; 16 bits low of cluster
    shr   ecx,16
    mov   [ebx+20],cx           ; 16 bits high of cluster (=0 fat16)
    mov   byte [ebx+11],0x20    ; attribute = archive

    call  set_current_time_for_entry

    mov   ebx,buffer            ; save the directory name,length,cluster
    call  hd_write
    cmp   [hd_error],0
    jne   exit_write_access_1

    imul  edi,[SECTORS_PER_CLUSTER],512 ; edi = cluster size in bytes
    xor   ecx,ecx               ; cluster count
    mov   ebx,PUSHAD_ECX        ; ebx = buffer

hd_new_block_write:

    mov   eax,[cluster]         ; eax = block
    call  set_data_cluster
    cmp   [hd_error],0
    jne   exit_write_access_1

    sub   esi,edi               ; sub wrote bytes
    jbe   file_saved_OK         ; end if all done
    add   ebx,edi               ; update buffer position

    inc   eax
    call  get_free_FAT          ; next free in FAT
    jc    exit_writing_disk_full_clear

    mov   edx,eax
    xchg  eax,[cluster]         ; get old cluster and save new cluster
    call  set_FAT               ; add it in cluster chain
    cmp  [hd_error],0
    jne   exit_write_access_1

    dec   ecx                   ; update cluster count
    jmp   hd_new_block_write

file_saved_OK:

    mov   edx,[fatEND]          ; new end for cluster chain
    call  set_FAT
    cmp  [hd_error],0
    jne   exit_write_access_1

    dec   ecx                   ; update cluster count

    call  add_disk_free_space   ; remove clusters from free disk space
    cmp  [hd_error],0
    jne   exit_write_access_1

    popad
    call  update_disk           ; write all of cache and fat to hd
    cmp   [hd_error],0
    jne   exit_write_access_2
    mov   [hd1_status],0
    xor   eax,eax
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
    cmp   [fat_type],0
    jnz   fat_ok_for_reading
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
    cmp   [fat_type],16
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


file_delete:
;-----------------------------------------------------
; input  : eax = file/directory name
;          edx = path
;          edi = 1 - allow directory remove else don't remove directory
; output : eax = 0 - ok
;                3 - unknown FS
;                5 - file not found
;               10 - access denied
;-----------------------------------------------------
    cmp   [fat_type],0
    jnz   file_del_fat_ok
    mov   eax,ERROR_UNKNOWN_FS
    ret

  file_del_fat_ok:
    pushad

    mov   ebx,edx
    call  get_cluster_of_a_path
    jc    file_to_delete_not_found

    mov   ebx,PUSHAD_EAX        ; file/directory name
    call  analyze_directory
    jc    file_to_delete_not_found

    test  byte [ebx+11],0x10    ; is it directory?
    jz    delete_notdir         ; no. it's file
    cmp   edi,1                 ; allow directory remove
    jnz   delete_no_access      ; no

    push  eax                   ; save directory sector
    mov   eax,[ebx+20-2]        ; first cluster of file
    mov   ax,[ebx+26]           ; 0 length files start cluster = 0
    and   eax,[fatMASK]
    xor   ebp,ebp               ; counter for directory deepnes
    call  clear_directory
    pop   eax
    jc    delete_no_access

    push  ebx                   ; save directory pointer in buffer
    mov   ebx,buffer
    call  hd_read               ; read directory sector
    cmp  [hd_error],0
    jne  delete_no_access_1
    pop   ebx

  delete_notdir:
    call  delete_entry_name
    cmp   [hd_error],0
    jne   delete_no_access

    mov   eax,ecx               ; first cluster of file
    call  clear_cluster_chain
    cmp   [hd_error],0
    jne   delete_no_access

    popad
    xor   eax,eax
    ret

  delete_no_access_1:
     add esp,4
  delete_no_access:
    popad
    mov   eax,ERROR_ACCESS_DENIED
    ret

  file_to_delete_not_found:
    cmp  [hd_error],0
    jne  delete_no_access
    popad
    mov   eax,ERROR_FILE_NOT_FOUND
    ret


clear_cluster_chain:
;-----------------------------------------------------
; input  : eax = first cluster
;-----------------------------------------------------
    push  eax ecx edx
    xor   ecx,ecx               ; cluster count
    mov   [f_del],1             ; delete on

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
    mov   [f_del],0
    pop   edx ecx eax
    ret


clear_directory:
;-----------------------------------------------------
; input  : eax = directory cluster
;          ebp = directory deepnes
; Note   : use recursive call
;-----------------------------------------------------
    pushad
    inc   ebp
    cmp   ebp,64                ; if over 63 directory deep
    jnb   clear_error           ; something must be wrong

  clear_new_cluster:
    cmp   eax,[LAST_CLUSTER]
    ja    clear_end
    cmp   eax,[ROOT_CLUSTER]    ; don't remove root cluster
    jz    clear_end
    mov   esi,eax               ; esi = current directory cluster
    sub   eax,2
    jb    clear_end
    mov   ecx,[SECTORS_PER_CLUSTER]
    imul  eax,ecx
    add   eax,[DATA_START]

  clear_new_sector:
    mov   edi,eax               ; edi = current directory sector
    mov   ebx,deltree_buffer
    call  hd_read
    cmp  [hd_error],0
    jne  clear_error    

    mov   edx,512/32            ; count of dir entrys per sector = 16

  clear_analyze:
    mov   al,[ebx+11]           ; file attribute
    and   al,0xf
    cmp   al,0xf
    je    clear_long_filename

    cmp   byte [ebx],'.'        ; parent or current directory
    je    clear_next_entry
    cmp   byte [ebx],0xe5       ; deleted
    je    clear_next_entry
    cmp   byte [ebx],0          ; empty
    je    clear_write_last
    ;je    clear_next_entry

    mov   eax,[ebx+20-2]        ; first cluster of entry
    mov   ax,[ebx+26]
    and   eax,[fatMASK]

    test  byte [ebx+11],0x10    ; is it directory?
    jz    clear_file            ; no

    push  eax ebx
    mov   eax,edi
    mov   ebx,deltree_buffer    ; save buffer over recursive call
    call  hd_write              ; write directory sector to disk
    cmp   [hd_error],0
    jne   clear_error    

    pop   ebx eax

    call  clear_directory       ; recursive call !!!
    jc    clear_error           ; exit if error found

    push  eax ebx
    mov   eax,edi
    mov   ebx,deltree_buffer
    call  hd_read               ; read directory sector again
    cmp  [hd_error],0
    jne   clear_error_1

    pop   ebx eax

  clear_file:
    call  clear_cluster_chain
    cmp  [hd_error],0
    jne   clear_error

  clear_long_filename:
    mov   byte [ebx],0xe5

  clear_next_entry:
    add   ebx,32                ; position of next dir entry
    dec   edx
    jnz   clear_analyze

    mov   eax,edi
    mov   ebx,deltree_buffer
    call  hd_write              ; write directory sector to disk
    cmp   [hd_error],0
    jne   clear_error

    inc   eax                   ; next sector
    dec   ecx
    jnz   clear_new_sector

    mov   eax,esi
    call  get_FAT               ; get next cluster
    cmp   [hd_error],0
    jne   clear_error

    jmp   clear_new_cluster     ; clear it

  clear_write_last:
    mov   eax,edi
    mov   ebx,deltree_buffer
    call  hd_write              ; write directory sector to disk
    cmp   [hd_error],0
    jne   clear_error

  clear_end:
    popad
    clc
    ret
clear_error_1:
    add  esp,8
  clear_error:
    popad
    stc
    ret


delete_entry_name:
;-----------------------------------------------------
; input  : eax = directory sector
;          ebx = directory pointer in buffer
;          longname_sec = 2 previous directory sectors
; output : ecx = first cluster
; change : eax,ebx,edx
;-----------------------------------------------------
    mov   byte [ebx],0xe5
    mov   ecx,[ebx+20-2]        ; first cluster of file
    mov   cx,[ebx+26]           ; 0 length files start cluster = 0
    and   ecx,[fatMASK]

  delete_empty:
    sub   ebx,32
    cmp   ebx,buffer
    jnb   delete_test_long

    mov   ebx,buffer
    call  hd_write              ; write directory sector back
    cmp   [hd_error],0
    jne   delete_name_end

    xor   eax,eax
    xchg  eax,[longname_sec2]
    xchg  eax,[longname_sec1]
    test  eax,eax               ; is there previous directory sector?
    jz    delete_name_end       ; no

    mov   ebx,buffer
    call  hd_read               ; read previous sector
    cmp  [hd_error],0
    jne  delete_name_end    

    mov   ebx,buffer+0x1e0      ; start from last entry

  delete_test_long:
    mov   dh,[ebx+11]           ; file attribute
    and   dh,0xf
    cmp   dh,0xf
    jne   delete_write_buffer

    cmp   byte [ebx],0x40       ; end of long dir entry?
    mov   byte [ebx],0xe5
    jb    delete_empty

  delete_write_buffer:
    mov   ebx,buffer
    call  hd_write              ; write directory sector back

  delete_name_end:
    ret


rename:
;-----------------------------------------------------------
; input  : eax = source directory name
;          edx = source path
;          ebx = dest directory name
;          edi = dest path
; output : eax = 0 - ok
;                3 - unknown FS
;                5 - file not found
;                8 - disk full
;               10 - access denied
;-----------------------------------------------------------
    cmp   [fat_type],0
    jnz   fat_ok_for_rename
    mov   eax,ERROR_UNKNOWN_FS
    ret

  fat_ok_for_rename:
;    call  reserve_hd1

    pushad

    mov   ebx,edx               ; source path
    call  get_cluster_of_a_path
    jc    rename_entry_not_found

    mov   ebx,PUSHAD_EAX        ; source directory name
    call  analyze_directory
    jc    rename_entry_not_found

    mov   [sector_tmp],eax      ; save source sector
    mov   [entry_pos],ebx
    mov   esi,ebx
    mov   edi,dir_entry
    mov   ecx,32/4
    cld
    rep   movsd                 ; save entry

    mov   ebx,PUSHAD_EDI        ; dest path
    call  get_cluster_of_a_path
    jc    rename_entry_not_found

    mov   edx,eax               ; save dest directory cluster
    mov   ebx,PUSHAD_EBX        ; dest directory name
    push  [longname_sec1]
    push  [longname_sec2]
    call  analyze_directory     ; check if entry already exist
    cmp  [hd_error],0
    jne  rename_entry_already_exist_1

    pop   [longname_sec2]
    pop   [longname_sec1]
    jnc   rename_entry_already_exist

    mov   eax,edx
    call  analyze_directory_to_write
    jc    rename_disk_full

    mov   esi,dir_entry
    mov   edi,ebx
    mov   ecx,32/4
    cld
    rep   movsd                 ; copy entry
    mov   esi,PUSHAD_EBX        ; dest directory name
    mov   edi,ebx
    mov   ecx,11
    rep   movsb                 ; copy name

    mov   ebx,buffer            ; save the directory name,length,cluster
    call  hd_write

    test  byte [dir_entry+11],0x10 ; is it directory?
    jz    rename_not_dir           ; no
    mov   eax,[dir_entry+20-2]     ; FAT entry
    mov   ax,[dir_entry+26]
    and   eax,[fatMASK]
    call  change_2dot_cluster
    cmp   [hd_error],0
    jne   rename_entry_already_exist

  rename_not_dir:
    cmp   [hd_error],0
    jne   rename_entry_already_exist
    mov   eax,[sector_tmp]
    mov   ebx,buffer
    call  hd_read               ; read source directory sector
    cmp  [hd_error],0
    jne  rename_entry_already_exist

    mov   ebx,[entry_pos]
    call  delete_entry_name
    cmp   [hd_error],0
    jne   rename_entry_already_exist

    popad
    call  update_disk           ; write all of cache and fat to hd
    cmp   [hd_error],0
    jne   rename_entry_already_exist_2
    mov   [hd1_status],0
    xor   eax,eax
    ret

  rename_entry_not_found:
    cmp  [hd_error],0
    jne  rename_entry_already_exist
    popad
    mov   [hd1_status],0
    mov   eax,ERROR_FILE_NOT_FOUND
    ret

  rename_entry_already_exist_1:
    add   esp,8
  rename_entry_already_exist:
    popad
  rename_entry_already_exist_2:
    mov   [hd1_status],0
    mov   eax,ERROR_ACCESS_DENIED
    ret

  rename_disk_full:
    cmp  [hd_error],0
    jne  rename_entry_already_exist
    popad
    mov   [hd1_status],0
    mov   eax,ERROR_DISK_FULL
    ret


change_2dot_cluster:
;-----------------------------------------------------------
; input  : eax = directory cluster
;          edx = value to save
; change : eax,ebx,edx
;-----------------------------------------------------------
    cmp   eax,[LAST_CLUSTER]
    ja    not_2dot              ; too big cluster number, something is wrong
    sub   eax,2
    jb    not_2dot

    imul  eax,[SECTORS_PER_CLUSTER]
    add   eax,[DATA_START]
    mov   ebx,buffer
    call  hd_read
    cmp  [hd_error],0
    jne  not_2dot

    cmp   dword [ebx+32],'..  '
    jnz   not_2dot

    cmp   edx,[ROOT_CLUSTER]    ; is rootdir cluster?
    jne   not_2dot_root
    xor   edx,edx               ; yes. set it zero

  not_2dot_root:
    mov   [ebx+32+26],dx        ; 16 bits low of cluster
    shr   edx,16
    mov   [ebx+32+20],dx        ; 16 bits high of cluster (=0 fat16)
    call  hd_write

  not_2dot:
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
    cmp   [fat_type],0
    jnz   info_fat_ok
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


;**************************************************************************
;
;   0x600008  - first entry in cache list
;
;            +0   - lba sector
;            +4   - state of cache sector
;                   0 = empty
;                   1 = used for read  ( same as in hd )
;                   2 = used for write ( differs from hd )
;
;      +65536 - cache entries
;
;**************************************************************************


hd_read:
;-----------------------------------------------------------
; input  : eax = block to read
;          ebx = destination
;-----------------------------------------------------------
    push  ecx esi edi           ; scan cache

    mov   ecx,cache_max         ; entries in cache
    mov   esi,0x600000+8
    mov   edi,1

  hdreadcache:

    cmp   dword [esi+4],0       ; empty
    je    nohdcache

    cmp   [esi],eax             ; correct sector
    je    yeshdcache

  nohdcache:

    add   esi,8
    inc   edi
    dec   ecx
    jnz   hdreadcache

    call  find_empty_slot       ; ret in edi
    cmp   [hd_error],0
    jne   return_01

    push  eax edx

    call  wait_for_hd_idle
    cmp   [hd_error],0
    jne   hd_read_error    

    cli
    xor   eax,eax
    mov   edx,[hdbase]
    inc   edx
    out   dx,al   ; ATAFeatures регистр "особенностей"
    inc   edx
    inc   eax
    out   dx,al   ; ATASectorCount счетчик секторов
    inc   edx
    mov   eax,[esp+4]
    out   dx,al   ; ATASectorNumber регистр номера сектора
    shr   eax,8
    inc   edx
    out   dx,al   ; ATACylinder номер цилиндра (младший байт)
    shr   eax,8
    inc   edx
    out   dx,al   ; номер цилиндра (старший байт)
    shr   eax,8
    inc   edx
    and   al,1+2+4+8
    add   al,byte [hdid]
    add   al,128+64+32
    out   dx,al   ; номер головки/номер диска
    inc   edx
    mov   al,20h
    out   dx,al   ; ATACommand регистр команд
    sti

    call  wait_for_sector_buffer

    cmp   [hd_error],0
    jne   hd_read_error

    cli
    push  edi
    shl   edi,9
    add   edi,0x600000+65536
    mov   ecx,256
    mov   edx,[hdbase]
    cld
    rep   insw
    pop   edi
    sti

    pop   edx eax
 blok_read_2:
    lea   esi,[edi*8+0x600000]
    mov   [esi],eax             ; sector number
    mov   dword [esi+4],1       ; hd read - mark as same as in hd

  yeshdcache:

    mov   esi,edi
    shl   esi,9
    add   esi,0x600000+65536
    mov   edi,ebx
    mov   ecx,512/4
    cld
    rep   movsd                 ; move data
 return_01:
    pop   edi esi ecx
    ret
    

hd_write:
;-----------------------------------------------------------
; input  : eax = block
;          ebx = pointer to memory
;-----------------------------------------------------------
    push  ecx esi edi

    ; check if the cache already has the sector and overwrite it

    mov   ecx,cache_max
    mov   esi,0x600000+8
    mov   edi,1

  hdwritecache:

    cmp   dword [esi+4],0       ; if cache slot is empty
    je    not_in_cache_write

    cmp   [esi],eax             ; if the slot has the sector
    je    yes_in_cache_write

  not_in_cache_write:

    add   esi,8
    inc   edi
    dec   ecx
    jnz   hdwritecache

    ; sector not found in cache
    ; write the block to a new location

    call  find_empty_slot       ; ret in edi
    cmp   [hd_error],0
    jne   hd_write_access_denied

    lea   esi,[edi*8+0x600000]
    mov   [esi],eax             ; sector number

  yes_in_cache_write:

    mov   dword [esi+4],2       ; write - differs from hd

    shl   edi,9
    add   edi,0x600000+65536
    mov   esi,ebx
    mov   ecx,512/4
    cld
    rep   movsd                 ; move data
 hd_write_access_denied:
    pop   edi esi ecx
    ret


write_cache:
;-----------------------------------------------------------
; write all changed sectors to disk
;-----------------------------------------------------------
    push  eax ecx edx esi edi

    ; write difference ( 2 ) from cache to hd

    mov   ecx,cache_max
    mov   esi,0x600000+8
    mov   edi,1

  write_cache_more:

    cmp   dword [esi+4],2       ; if cache slot is not different
    jne   does_not_need_writing

    mov   dword [esi+4],1       ; same as in hd
    mov   eax,[esi]             ; eax = sector to write

    cmp   eax,[PARTITION_START]
    jb    danger
    cmp   eax,[PARTITION_END]
    ja    danger

    call  wait_for_hd_idle
    cmp   [hd_error],0
    jne   hd_write_error

    cli
    xor   eax,eax
    mov   edx,[hdbase]
    inc   edx
    out   dx,al
    inc   edx
    inc   eax
    out   dx,al
    inc   edx
    mov   eax,[esi]             ; eax = sector to write
    out   dx,al
    shr   eax,8
    inc   edx
    out   dx,al
    shr   eax,8
    inc   edx
    out   dx,al
    shr   eax,8
    inc   edx
    and   al,1+2+4+8
    add   al,byte [hdid]
    add   al,128+64+32
    out   dx,al
    inc   edx
    mov   al,30h
    out   dx,al
    sti

    call  wait_for_sector_buffer

    cmp   [hd_error],0
    jne   hd_write_error

    push  ecx esi

    cli
    mov   esi,edi
    shl   esi,9
    add   esi,0x600000+65536    ; esi = from memory position
    mov   ecx,256
    mov   edx,[hdbase]
    cld
    rep   outsw
    sti

    pop   esi ecx

  danger:
  does_not_need_writing:

    add   esi,8
    inc   edi
    dec   ecx
    jnz   write_cache_more
 return_02:
    pop   edi esi edx ecx eax
    ret


find_empty_slot:
;-----------------------------------------------------------
; find empty or read slot, flush cache if next 10% is used by write
; output : edi = cache slot
;-----------------------------------------------------------
    push  ecx esi

  search_again:

    mov   ecx,cache_max*10/100
    mov   edi,[cache_search_start]

  search_for_empty:

    inc   edi
    cmp   edi,cache_max
    jbe   inside_cache
    mov   edi,1

  inside_cache:

    cmp   dword [edi*8+0x600000+4],2    ; get cache slot info
    jb    found_slot                    ; it's empty or read
    dec   ecx
    jnz   search_for_empty

    call  write_cache                   ; no empty slots found, write all
    cmp   [hd_error],0
    jne   found_slot_access_denied

    jmp   search_again                  ; and start again

  found_slot:

    mov   [cache_search_start],edi
  found_slot_access_denied:
    pop   esi ecx
    ret


save_hd_wait_timeout:

    push  eax
    mov   eax,[timer_ticks];[0xfdf0]
    add   eax,300               ; 3 sec timeout
    mov   [hd_wait_timeout],eax
    pop   eax
    ret


check_hd_wait_timeout:

    push  eax
    mov   eax,[hd_wait_timeout]
    cmp   [timer_ticks], eax ;[0xfdf0],eax
    jg    hd_timeout_error
    pop   eax
    mov   [hd_error],0
    ret

iglobal
  hd_timeout_str   db 'K : FS - HD timeout',13,10,0
  hd_read_str      db 'K : FS - HD read error',13,10,0
  hd_write_str     db 'K : FS - HD write error',13,10,0
  hd_lba_str       db 'K : FS - HD LBA error',13,10,0
endg

hd_timeout_error:

    call  clear_hd_cache
    call  clear_application_table_status
    mov   esi,hd_timeout_str
    call  sys_msg_board_str
;    jmp   $
    mov   [hd_error],1
    pop   eax
    ret

hd_read_error:

    call  clear_hd_cache
    call  clear_application_table_status
    mov   esi,hd_read_str
    call  sys_msg_board_str
    pop   edx eax
    jmp    return_01
;    jmp   $

hd_write_error:

    call  clear_hd_cache
    call  clear_application_table_status
    mov   esi,hd_write_str
    call  sys_msg_board_str
    jmp    return_02
;    jmp   $

hd_lba_error:
    call  clear_hd_cache
    call  clear_application_table_status
    mov   esi,hd_lba_str
    call  sys_msg_board_str
    jmp   LBA_read_ret


wait_for_hd_idle:

    push  eax edx

    call  save_hd_wait_timeout

    mov   edx,[hdbase]
    add   edx,0x7

  wfhil1:

    call  check_hd_wait_timeout
    cmp   [hd_error],0
    jne   @f

    in    al,dx
    test  al,128
    jnz   wfhil1
  
 @@:

    pop   edx eax
    ret



wait_for_sector_buffer:

    push  eax edx

    mov   edx,[hdbase]
    add   edx,0x7

    call  save_hd_wait_timeout

  hdwait_sbuf:                  ; wait for sector buffer to be ready

    call  check_hd_wait_timeout
    cmp   [hd_error],0
    jne   @f

    in    al,dx
    test  al,8
    jz    hdwait_sbuf

    mov   [hd_error],0

    cmp   [hd_setup],1          ; do not mark error for setup request
    je    buf_wait_ok

    test  al,1                  ; previous command ended up with an error
    jz    buf_wait_ok
 @@:
    mov   [hd_error],1

  buf_wait_ok:

    pop   edx eax
    ret



read_hd_file:
;-----------------------------------------------------------------
;
; Converting old reading function for hd-application start.
;
; IN:
;
; eax - pointer to file (0 = read only first sector of drive: eg 'label')
; ebx - file lenght
; ecx - start 512 byte block number
; edx - number of blocks to read
; esi - pointer to return/work area (atleast 20 000 bytes)
;
; For new read function
;
; EAX   (PAR0)      pointer to file-name
; ECX   (PAR1)      pointer to buffer
; EBX   (PAR2)   vt file blocks to read
; EDX   (PAR3)      pointer to path
; ESI            vt first 512 block to read
; EDI               if 0 - return root
;--------------------------------------------------------------------------

    push  ecx esi edi
    mov   esi,eax
    mov   edi,startpath
    mov   ecx,250
    cld
    rep   movsb
    pop   edi esi ecx

    mov   eax,startpath
    mov   [eax+ebx-12],byte 0

    push  eax ebx ecx edx esi

    pop   ecx ; pointer to buffer
    add   ecx,1024
    pop   ebx ; number of blocks to read
    pop   esi ; first block to read
    dec   esi
    pop   eax ; file length
    pop   edx ; pointer to path

    mov   edi,12
    lea   eax,[eax+edx-12+1]
    call  file_read

    ret

; \begin{diamond}
hd_find_lfn:
; in: esi->name
; out: CF=1 - file not found
;      else CF=0 and edi->direntry, eax=sector
; destroys eax
        push    esi edi
        push    0
        push    0
        push    fat16_root_first
        push    fat16_root_next
        mov     eax, [ROOT_CLUSTER]
        cmp     [fat_type], 32
        jz      .fat32
.loop:
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
    cmp    [fat_type], 0
    jnz    @f
    or    ebx, -1
    mov    eax, ERROR_UNKNOWN_FS
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
        cmp     [fat_type], 32
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
        push    ecx
        mov     ecx, [eax+4]
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
        mov     eax, [eax]
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
        mov     [f_del], 1
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

fs_HdRewrite:
        cmp     [fat_type], 0
        jz      fshrfs
        cmp     byte [esi], 0
        jz      fshrad
        pushad
        xor     ebp, ebp
        push    esi
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, '/'
        jnz     @b
        lea     ebp, [esi-1]
        jmp     @b
@@:
        pop     esi
        test    ebp, ebp
        jnz     .noroot
        mov     ebp, [ROOT_CLUSTER]
        cmp     [fat_type], 32
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
.noroot:
; check existence
        mov     byte [ebp], 0
        call    hd_find_lfn
        mov     byte [ebp], '/'
        lea     esi, [ebp+1]
        jnc     @f
        mov     eax, ERROR_FILE_NOT_FOUND
.ret1:
        mov     [esp+28], eax
        popad
        xor     ebx, ebx
        ret
@@:
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
; found; must not be directory
        test    byte [edi+11], 10h
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
        mov     [f_del], 1
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
.doit:
        lea     eax, [esp+8]
        call    dword [eax+16]  ; flush directory
        push    ecx
        mov     ecx, [esp+4+32+24]
        push    ecx
        push    edi
        mov     esi, edx
        test    ecx, ecx
        jz      .done
        mov     eax, 2
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
        mov     [f_del], 1
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

fs_HdGetFileInfo:
        cmp     [fat_type], 0
        jnz     @f
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
        cmp     [fat_type], 0
        jnz     @f
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
;  fs_HdExecute - LFN variant for executing from harddisk
;
;  esi  points to hd filename (e.g. 'dir1/name')
;  ebp  points to full filename (e.g. '/hd0/1/dir1/name')
;  dword [ebx] = flags
;  dword [ebx+4] = cmdline
;
;  ret ebx,edx destroyed
;      eax > 0 - PID, < 0 - error
;
;--------------------------------------------------------------
fs_HdExecute:
        mov     edx, [ebx]
        mov     ebx, [ebx+4]
        test    ebx, ebx
        jz      @f
        add     ebx, std_application_base_address
@@:

;----------------------------------------------------------------
;
; fs_HdExecute.flags - second entry
;
;  esi  points to floppy filename (kernel address)
;  ebp  points to full filename
;  edx  flags
;  ebx  cmdline (kernel address)
;
;  ret  eax > 0 - PID, < 0 - error
;
;--------------------------------------------------------------

.flags:
        cmp     [fat_type], 0
        jnz     @f
        mov     eax, ERROR_UNKNOWN_FS
        ret
@@:
        cmp     byte [esi], 0
        jnz     @f
; cannot execute root!
        mov     eax, -ERROR_ACCESS_DENIED
        ret
@@:
        push    edi
        call    hd_find_lfn
        jnc     .found
        pop     edi
        mov     eax, -ERROR_FILE_NOT_FOUND
        cmp     [hd_error], 0
        jz      @f
        mov     al, -11
@@:
        ret
.found:
        mov     eax, [edi+20-2]
        mov     ax, [edi+26]
        push    0
        push    eax
        push    dword [edi+28]          ; size
        push    .DoRead
        call    fs_execute
        add     esp, 16
        pop     edi
        ret

.DoRead:
; read next block
; in: eax->parameters, edi->buffer
; out: eax = error code
        pushad
        cmp     dword [eax], 0  ; file size
        jz      .eof
        add     eax, 4
        call    fat_get_sector
        mov     ebx, edi
        call    hd_read
        cmp     [hd_error], 0
        jnz     .err
        mov     eax, [esp+28]
        mov     ecx, [eax]
        sub     ecx, 512
        jae     @f
        lea     edi, [edi+ecx+512]
        neg     ecx
        push    eax
        xor     eax, eax
        rep     stosb
        pop     eax
@@:
        mov     [eax], ecx
        mov     edx, [eax+8]
        inc     edx
        cmp     edx, [SECTORS_PER_CLUSTER]
        jb      @f
        push    eax
        mov     eax, [eax+4]
        call    get_FAT
        cmp     [hd_error], 0
        jnz     .err
        mov     ecx, eax
        pop     eax
        mov     [eax+4], ecx
        xor     edx, edx
@@:
        mov     [eax+8], edx
        popad
        xor     eax, eax
        ret
.eof:
        popad
        mov     eax, 6
        ret
.err:
        popad
        mov     eax, 11
        ret

; \end{diamond}
