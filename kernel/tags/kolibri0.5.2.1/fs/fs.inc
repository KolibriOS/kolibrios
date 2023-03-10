;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;; System service for filesystem call                                   ;;
;; (C) 2004 Ville Turjanmaa, License: GPL                               ;;
;;                                                                      ;;
;; 15.01.2005 get file size/attr/date, file_append (only for hd) - ATV  ;;
;; 23.11.2004 test if hd/partition is set - ATV                         ;;
;; 18.11.2004 get_disk_info and more error codes - ATV                  ;;
;; 08.11.2004 expand_pathz and rename (only for hd) - ATV               ;;
;; 20.10.2004 Makedir/Removedir (only for hd) - ATV                     ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

iglobal
dir0:        db  'HARDDISK   '
             db  'RAMDISK    '
             db  'FLOPPYDISK '
             db  0

dir1:        db  'FIRST      '
             db  'SECOND     '
             db  'THIRD      '
             db  'FOURTH     '
             db  0

not_select_IDE db 0

hd_address_table:  dd  0x1f0,0x00,0x1f0,0x10
                   dd  0x170,0x00,0x170,0x10
endg

file_system: 
; IN:
;
; eax = 0  ; read file          /RamDisk/First  6   /HardDisk/First 30
; eax = 1  ; write file         /RamDisk/First 33   /HardDisk/First 56
; eax = 2  ; delete file        /RamDisk/First 32   /HardDisk/First 57
; eax = 3  ; append to a file   /RamDisk/First ??   /HardDisk/First ??
; eax = 4  ; makedir
; eax = 5  ; rename file/directory
; eax = 8  ; lba read
; eax = 12 ; get_filesize
; eax = 13 ; get_fileattr
; eax = 14 ; get_filedate
; eax = 15 ; get_disk_info
; eax = 16 ; start application
;
; OUT:
;
; eax = 0  : read ok
; eax = 1  : no fd base and/or partition defined
; eax = 2  : yet unsupported FS
; eax = 3  : unknown FS
; eax = 4  : partition not defined at hd
; eax = 5  : file not found
; eax = 6  : end of file
; eax = 7  : memory pointer not in application area
; eax = 8  : disk full
; eax = 9  : fat table corrupted
; eax = 10 : access denied
;
; ebx = size

    ; Extract parameters

    mov   edi,[0x3010]
    add   eax,[edi+0x10]        ; abs start of info block

    cmp   dword [eax+0],12      ; Get file size
    je    fs_read
    cmp   dword [eax+0],13      ; Get file attribute
    je    fs_read
    cmp   dword [eax+0],14      ; Get file date/time
    je    fs_read
    cmp   dword [eax+0],15      ; GET_DISK_INFO
    je    fs_info
    cmp   dword [eax+0],16      ; RUN - dont care about read&write blocks
    je    fs_read
    cmp   dword [eax+0],5       ; RENAME - dont care about read&write blocks
    je    fs_read
    cmp   dword [eax+0],4       ; MAKEDIR - dont care about read&write blocks
    je    fs_read
    cmp   dword [eax+0],2       ; DELETE - dont care about read&write blocks
    je    fs_read

    cmp   dword [0x3000],1      ; no memory checks for kernel requests
    jz    no_checks_for_kernel
;iglobal
;  buffer_failed db 'Buffer check failed',13,10,0
;endg
    mov   edx,eax
    cmp   dword [eax+0],1
    jz    .check_for_write_op
    cmp   dword [eax+0],3
    jnz   .usual_check
.check_for_write_op:    
    mov   ebx,[eax+12]
    add   ebx,std_application_base_address
    mov   ecx,[eax+8]
    call  check_region
    test  eax,eax
    jnz   area_in_app_mem
    
.error_output:
    mov   esi,buffer_failed
    call  sys_msg_board_str
    mov   eax,7
    mov   dword [esp+36],7
    ret
iglobal
  buffer_failed db 'Buffer check failed',13,10,0
endg 
.usual_check:
    cmp   dword [eax+0],0
    mov   ecx,512
    jnz   .small_size
    mov   ecx,[eax+8]
    shl   ecx,9
.small_size:
    mov   ebx,[eax+12]
    add   ebx,std_application_base_address
    call  check_region
    test  eax,eax
    jz    .error_output
    jmp   area_in_app_mem           
;    mov   ebx,[0x3000]          ; pointer in application memory ?
;    shl   ebx,8
;    mov   ebx,[ebx+0x80000+0x8c]

;    mov   ebp,ebx               ; save for checking at stack save
;    sub   ebp,[eax+12]
;    shr   ebp,9

;    sub   ebx,512               ; need atleast one block

;    cmp   ebx,[eax+12]
;    ja    area_in_app_mem
;    mov   eax,7
;    mov   dword [esp+36],7
;    ret
  area_in_app_mem:
    mov   eax,edx
  no_checks_for_kernel:


    cmp   dword [eax+0],3       ; APPEND - allow write 0 bytes (truncate)
    je    fs_read
    cmp   dword [eax+8],0       ; read or write 0 blocks/bytes ?
    jne   fs_read
    mov   dword [esp+36],0
    ret
  fs_read:

    mov   ebx,[eax+20]          ; program wants root directory ?
    test  bl,bl
    je    fs_getroot
    test  bh,bh
    jne   fs_noroot
  fs_getroot:
    mov   edx,[edi+0x10]
    mov   esi,dir0
    mov   edi,[eax+12]
    add   edi,edx
    mov   ecx,11
    cld
    rep   movsb
    mov   eax,0x10
    stosb
    add   edi,32-11-1
    mov   ecx,11
    rep   movsb
    stosb
    mov   dword [esp+36],0      ; ok read
    mov   dword [esp+24],32*2   ; size of root
    ret

  fs_info:                      ;start of code - Mihasik
    mov   edi,eax
    push  edi
    cmp   [eax+21],byte 'h'
    je    fs_info_h
    cmp   [eax+21],byte 'H'
    je    fs_info_h
    cmp   [eax+21],byte 'r'
    je    fs_info_r
    cmp   [eax+21],byte 'R'
    je    fs_info_r
    mov   eax,3                 ;if unknown disk
    xor   ebx,ebx
    xor   ecx,ecx
    xor   edx,edx
    jmp   fs_info1
  fs_info_r:
    call  ramdisk_free_space    ;if ramdisk
    mov   ecx,edi               ;free space in ecx
    shr   ecx,9                 ;free clusters
    mov   ebx,2847              ;total clusters
    mov   edx,512               ;cluster size
    xor   eax,eax               ;always 0
    jmp   fs_info1
  fs_info_h:                    ;if harddisk
    call  get_hd_info
  fs_info1:
    pop   edi
    mov   [esp+36],eax
    mov   [esp+24],ebx           ; total clusters on disk
    mov   [esp+32],ecx           ; free clusters on disk
    mov   [edi],edx              ; cluster size in bytes
    ret                          ;end of code - Mihasik

  fs_noroot:

    mov   ebx,[eax+0]
    push  ebx                   ; read/write/delete/.../makedir/rename/lba/run
    mov   ebx,[eax+4]
    push  ebx                   ; 512 block number to read
    mov   ebx,[eax+8]

;    cmp   dword [eax+0],0       ; if read, check that the data stays at
;    jne   ret_size_fine         ; application memory
;    cmp   ebx,ebp
;    jbe   ret_size_fine
;    mov   ebx,ebp
;  ret_size_fine:

    push  ebx                   ; bytes to write/append or 512 blocks to read
    mov   ebx,[eax+12]
    add   ebx,[edi+0x10]
    push  ebx                   ; abs start of return/save area

    lea   esi,[eax+20]          ; abs start of dir + filename
    mov   edi,[edi+0x10]        ; abs start of work area
    add   edi,[eax+16]

    call  expand_pathz

    push  edi                   ; dir start
    push  ebx                   ; name of file start

    mov   ebx,[dir0+11]         ; /RAMDISK
    mov   eax,[edi+1]
    cmp   eax,'RD  '
    je    fs_yesramdisk
    cmp   eax,ebx
    jne   fs_noramdisk

  fs_yesramdisk:

    cmp   byte [edi+1+11],0
    je    fs_give_dir1

    mov   ebx,[dir1]            ; /FIRST
    mov   eax,[edi+1+12]
    cmp   eax,'1   '
    je    fs_yesramdisk_first
    cmp   eax,ebx
    jne   fs_noramdisk

  fs_yesramdisk_first:

    cmp   dword [esp+20],8      ; LBA read ramdisk
    jne   fs_no_LBA_read_ramdisk

    mov   eax,[esp+16]          ; LBA block to read
    mov   ecx,[esp+8]           ; abs pointer to return area

    call  LBA_read_ramdisk
    jmp   file_system_return


  fs_no_LBA_read_ramdisk:

    cmp   dword [esp+20],0      ; READ
    jne   fs_noramdisk_read

    mov   eax,[esp+4]           ; fname
    add   eax,2*12+1
    mov   ebx,[esp+16]          ; block start
    inc   ebx
    mov   ecx,[esp+12]          ; block count
    mov   edx,[esp+8]           ; return
    mov   esi,[esp+0]
    sub   esi,eax
    add   esi,12+1              ; file name length
    call  fileread

    jmp   file_system_return


  fs_noramdisk_read:

    cmp   dword [esp+20],1      ; WRITE
    jne   fs_noramdisk_write

    mov   eax,[esp+4]           ; fname
    add   eax,2*12+1
    mov   ebx,[esp+8]           ; buffer
    mov   ecx,[esp+12]          ; count to write
    mov   edx,0                 ; create new
    call  filesave

    ; eax=0 ok - eax=1 not enough free space

    jmp   file_system_return

  fs_noramdisk_write:

    cmp   dword [esp+20],16     ; START APPLICATION
    jne   fs_noramdisk_start_application

    mov   eax,[esp+4]           ; fname
    add   eax,2*12+1

    xor   ebx,ebx               ; parameters to pass
    cmp   dword [esp+12],0
    je    no_fl_start_param
    mov   ebx,[0x3010]
    mov   ebx,[ebx+0x10]
    add   ebx,[esp+12]
  no_fl_start_param:

    call  start_application_fl

    jmp   file_system_return

  fs_noramdisk_start_application:     ;there's new code - Mihasik
    cmp   dword [esp+20],2      ;DELETE
    jne   fs_noramdisk_delete
    mov   eax,[esp+4]           ; fname
    add   eax,2*12+1
    call  filedelete
    jmp   file_system_return

  fs_noramdisk_delete:
    cmp   dword [esp+20],12     ;GET TIME,DATE,SIZE AND ATTRS
    jb    fs_noramdisk_getinfo
    cmp   dword [esp+20],14
    ja    fs_noramdisk_getinfo
    mov   eax,[esp+4]           ; fname
    add   eax,2*12+1
    mov   ebx,[esp+20]
    mov   ecx,[esp+0]
    sub   ecx,eax
    add   ecx,12+1              ; file name length
    call  rd_getfileinfo
    jmp   file_system_return
  fs_noramdisk_getinfo:             ;End of code - Mihasik

  fs_noramdisk:
  
  ;********************************************************************
    mov   ebx,[dir0+22]         ; /FLOPPYDISK
    mov   eax,[edi+1]
    cmp   eax,'FD  '
    je    fs_yesflpdisk
    cmp   eax,ebx
    jne   fs_noflpdisk

  fs_yesflpdisk:
    call   reserve_flp

    cmp   byte [edi+1+11],0
    je    fs_give_dir1

    mov   ebx,[dir1]            ; /FIRST
    mov   eax,[edi+1+12]
    cmp   eax,'1   '            
    je    fs_yesflpdisk_first
    cmp   eax,ebx
    je    fs_yesflpdisk_first
    mov   ebx,[dir1+11]         ; /SECOND
    cmp   eax,'2   '
    je    fs_yesflpdisk_second
    cmp   eax,ebx
    jne   fs_noflpdisk
    jmp   fs_yesflpdisk_second

  fs_yesflpdisk_first:
    mov   [flp_number],1
    jmp   fs_yesflpdisk_start
  fs_yesflpdisk_second:
    mov   [flp_number],2
  fs_yesflpdisk_start:
    cmp   dword [esp+20],0      ; READ
    jne   fs_noflpdisk_read

    mov   eax,[esp+4]           ; fname
    add   eax,2*12+1
    mov   ebx,[esp+16]          ; block start
    inc   ebx
    mov   ecx,[esp+12]          ; block count
    mov   edx,[esp+8]           ; return
    mov   esi,[esp+0]
    sub   esi,eax
    add   esi,12+1              ; file name length
    call  floppy_fileread

    jmp   file_system_return


  fs_noflpdisk_read:

    cmp   dword [esp+20],1      ; WRITE
    jne   fs_noflpdisk_write

    mov   eax,[esp+4]           ; fname
    add   eax,2*12+1
    mov   ebx,[esp+8]           ; buffer
    mov   ecx,[esp+12]          ; count to write
    mov   edx,0                 ; create new
    call  floppy_filesave

    ; eax=0 ok - eax=1 not enough free space

    jmp   file_system_return

  fs_noflpdisk_write:

    cmp   dword [esp+20],2      ; DELETE
    jne   fs_noflpdisk_delete

    mov   eax,[esp+4]           ; fname
    add   eax,2*12+1
    call  floppy_filedelete
    mov   [flp_status],0
    jmp   file_system_return

  fs_noflpdisk_delete:
    cmp   dword [esp+20],16     ; START APPLICATION
    jne   fs_noflpdisk_start_application

    mov   eax,[esp+4]           ; fname
    add   eax,2*12+1

    xor   ebx,ebx               ; parameters to pass
    cmp   dword [esp+12],0
    je    no_flp_start_param
    mov   ebx,[0x3010]
    mov   ebx,[ebx+0x10]
    add   ebx,[esp+12]

  no_flp_start_param:

    call  start_application_floppy

    jmp   file_system_return

  fs_noflpdisk_start_application:

  fs_noflpdisk:
  ;*****************************************************************
  
    mov   eax,[edi+1]
    cmp   eax,'HD0 '
    je    fs_yesharddisk_IDE0
    cmp   eax,'HD1 '
    je    fs_yesharddisk_IDE1
    cmp   eax,'HD2 '
    je    fs_yesharddisk_IDE2
    cmp   eax,'HD3 '
    je    fs_yesharddisk_IDE3
    jmp   old_path_harddisk
fs_yesharddisk_IDE0:
     call  reserve_hd1
     mov  [hdbase],0x1f0
     mov  [hdid],0x0
     mov  [hdpos],1
     jmp  fs_yesharddisk_partition
fs_yesharddisk_IDE1:
     call  reserve_hd1
     mov  [hdbase],0x1f0
     mov  [hdid],0x10
     mov  [hdpos],2
     jmp  fs_yesharddisk_partition
fs_yesharddisk_IDE2:
     call  reserve_hd1
     mov  [hdbase],0x170
     mov  [hdid],0x0
     mov  [hdpos],3
     jmp  fs_yesharddisk_partition
fs_yesharddisk_IDE3:
     call  reserve_hd1
     mov  [hdbase],0x170
     mov  [hdid],0x10
     mov  [hdpos],4
fs_yesharddisk_partition:
;    call  choice_necessity_partition 
;    jmp   fs_yesharddisk_all     
    jmp   fs_for_new_semantic

choice_necessity_partition:
    mov   eax,[edi+1+12]
    call  StringToNumber
        mov   [fat32part],eax
choice_necessity_partition_1:
    mov   [0xfe10],dword 0    ; entries in hd cache
    mov   ecx,[hdpos]
    xor   eax,eax
    mov   edx,0x40002
 search_partition_array:
    mov   bl,[edx]
    movzx ebx,bl
    add   eax,ebx
    inc   edx
    loop  search_partition_array
    sub   eax,ebx
    add   eax,[fat32part]
    dec   eax
    xor   edx,edx
    imul  eax,100
    add   eax,0x4000a
    mov   [transfer_adress],eax
    call  partition_data_transfer_1
    ret

 old_path_harddisk:
    mov   ebx,[dir0]            ; /HARDDISK
    mov   eax,[edi+1]
    cmp   eax,'HD  '
    je    fs_yesharddisk
    cmp   eax,ebx
    jne   fs_noharddisk

  fs_yesharddisk:
    call  reserve_hd1

    cmp   dword [esp+20],8      ; LBA read
    jne   fs_no_LBA_read
    mov   eax,[esp+16]          ; LBA block to read
    lea   ebx,[edi+1+12]        ; pointer to FIRST/SECOND/THIRD/FOURTH
    mov   ecx,[esp+8]           ; abs pointer to return area
    call  LBA_read
    jmp   file_system_return

  fs_no_LBA_read:

    cmp   byte [edi+1+11],0     ; directory read
    je    fs_give_dir1
 fs_for_new_semantic:
    call  choice_necessity_partition 

  fs_yesharddisk_all:
    mov   eax,1
    cmp   [hdpos],0             ; is hd base set?
    jz    file_system_return    ; no
    cmp   [fat32part],0         ; is partition set?
    jz    file_system_return    ; no

    cmp   dword [esp+20],0      ; READ
    jne   fs_noharddisk_read

    mov   eax,[esp+0]           ; /fname
    lea   edi,[eax+12]
    mov   byte [eax],0          ; path to asciiz
    inc   eax                   ; filename start

    mov   ebx,[esp+12]          ; count to read
    mov   ecx,[esp+8]           ; buffer
    mov   edx,[esp+4]
    add   edx,12*2              ; dir start
    sub   edi,edx               ; path length
    mov   esi,[esp+16]          ; blocks to read

    call  file_read

    mov   edi,[esp+0]
    mov   byte [edi],'/'

    jmp   file_system_return

  fs_noharddisk_read:


    cmp   dword [esp+20],1      ; WRITE
    jne   fs_noharddisk_write

    mov   eax,[esp+0]           ; /fname
    mov   byte [eax],0          ; path to asciiz
    inc   eax                   ; filename start

    mov   ebx,[esp+12]          ; count to write
    mov   ecx,[esp+8]           ; buffer
    mov   edx,[esp+4]
    add   edx,12*2              ; path start

    call  file_write

    mov   edi,[esp+0]
    mov   byte [edi],'/'

    ; eax=0 ok - eax=1 not enough free space

    jmp   file_system_return


  fs_noharddisk_write:

    cmp   dword [esp+20],2      ; DELETE
    jne   fs_noharddisk_delete

    mov   eax,[esp+0]           ; /dirname or /filename
    mov   byte [eax],0          ; path to asciiz
    inc   eax                   ; filename start
    mov   edx,[esp+4]
    add   edx,12*2              ; path start

    call  removedir

    mov   edi,[esp+0]
    mov   byte [edi],'/'

    jmp   file_system_return

  fs_noharddisk_delete:

    cmp   dword [esp+20],3      ; APPEND
    jne   fs_noharddisk_append

    mov   eax,[esp+0]           ; /dirname or /filename
    mov   byte [eax],0          ; path to asciiz
    inc   eax                   ; filename start
    mov   edx,[esp+4]
    add   edx,12*2              ; path start
    mov   ecx,[esp+8]           ; buffer
    mov   ebx,[esp+12]          ; count to write
    mov   esi,[esp+16]          ; bytes to skip over

    call  file_append

    mov   edi,[esp+0]
    mov   byte [edi],'/'

    jmp   file_system_return

  fs_noharddisk_append:

    cmp   dword [esp+20],4      ; MAKEDIR
    jne   fs_noharddisk_makedir

    mov   eax,[esp+0]           ; /dirname
    mov   byte [eax],0          ; path to asciiz
    inc   eax                   ; filename start
    mov   edx,[esp+4]
    add   edx,12*2              ; path start

    call  makedir

    mov   edi,[esp+0]
    mov   byte [edi],'/'

    jmp   file_system_return

  fs_noharddisk_makedir:

    cmp   dword [esp+20],5      ; RENAME
    jne   fs_noharddisk_rename

    mov   edi,[esp+0]           ; start of source file name
    add   edi,12+1              ; continue after name
    call  expand_pathz          ; convert destination name

    mov   edx,[dir0]            ; /HARDDISK
    mov   eax,[edi+1]
    cmp   eax,'HD  '
    je    fs_rename_test1
    cmp   eax,edx
    jne   fs_rename_error

  fs_rename_test1:
    mov   edx,[dir1]            ; /FIRST
    mov   eax,[edi+1+12]
    cmp   eax,'1   '
    je    fs_rename_start
    cmp   eax,edx
    jne   fs_rename_error

  fs_rename_start:
    mov   byte [ebx],0          ; path to asciiz
    inc   ebx                   ; filename start
    add   edi,12*2              ; path start
    cmp   byte [ebx],0
    je    fs_rename_error
    cmp   byte [ebx],32
    je    fs_rename_error

    mov   eax,[esp+0]           ; /filename
    mov   byte [eax],0          ; path to asciiz
    inc   eax                   ; filename start
    mov   edx,[esp+4]
    add   edx,12*2              ; path start

    call  rename

    mov   edi,[esp+0]
    mov   byte [edi],'/'

    jmp   file_system_return

  fs_rename_error:
    mov   eax,4                 ; partition not defined at hd
    jmp   file_system_return

  fs_noharddisk_rename:

    cmp   dword [esp+20],12     ; get FILESIZE
    jne   fs_noharddisk_get_filesize

    mov   eax,[esp+0]           ; /fname
    lea   edi,[eax+12]
    mov   byte [eax],0          ; path to asciiz
    inc   eax                   ; filename start
    mov   edx,[esp+4]
    add   edx,12*2              ; path start
    sub   edi,edx               ; path length

    call  get_filesize

    mov   edi,[esp+0]
    mov   byte [edi],'/'

    jmp   file_system_return

  fs_noharddisk_get_filesize:

    cmp   dword [esp+20],13     ; get FILEATTR
    jne   fs_noharddisk_get_fileattr

    mov   eax,[esp+0]           ; /dirname
    mov   byte [eax],0          ; path to asciiz
    inc   eax                   ; filename start
    mov   edx,[esp+4]
    add   edx,12*2              ; path start

    call  get_fileattr

    mov   edi,[esp+0]
    mov   byte [edi],'/'

    jmp   file_system_return

  fs_noharddisk_get_fileattr:

    cmp   dword [esp+20],14     ; get FILEDATE
    jne   fs_noharddisk_get_filedate

    mov   eax,[esp+0]           ; /dirname
    mov   byte [eax],0          ; path to asciiz
    inc   eax                   ; filename start
    mov   edx,[esp+4]
    add   edx,12*2              ; path start

    call  get_filedate

    mov   edi,[esp+0]
    mov   byte [edi],'/'

    jmp   file_system_return

  fs_noharddisk_get_filedate:

    cmp   dword [esp+20],16     ; START APPLICATION
    jne   fs_noharddisk_start_application

    mov   eax,[esp+4]           ; fname
    add   eax,12*2

    mov   ebx,[esp+0]           ; length
    sub   ebx,eax
    add   ebx,12

    mov   ecx,[esp+4]           ; work area
    add   ecx,512

    xor   ebp,ebp               ; parameters to pass
    cmp   dword [esp+12],0
    je    no_hd_start_param
    mov   ebp,[0x3010]
    mov   ebp,[ebp+0x10]
    add   ebp,[esp+12]
  no_hd_start_param:

    call  start_application_hd

    jmp   file_system_return

  fs_noharddisk_start_application:

  fs_noharddisk:

  file_system_return:

    add   esp,24

    mov   [esp+36],eax
    mov   [esp+24],ebx
    ret


  fs_give_dir1:

    mov   eax,0x10
    mov   ebx,1
    mov   edi,[esp+8]
    mov   esi,dir1
  fs_d1_new:
    mov   ecx,11
    cld
    rep   movsb
    stosb
    add   edi,32-11-1
    dec   ebx
    jne   fs_d1_new

    add   esp,24

    mov   dword [esp+36],0      ; ok read
    mov   dword [esp+24],32*1   ; dir/data size
    ret



LBA_read_ramdisk:

    cmp   [lba_read_enabled],1
    je    lbarrl1

    xor   ebx,ebx
    mov   eax,2
    ret

  lbarrl1:

    cmp   eax,18*2*80
    jb    lbarrl2
    xor   ebx,ebx
    mov   eax,3
    ret

  lbarrl2:

    pushad

    call  restorefatchain

    mov   edi,ecx
    mov   esi,eax

    shl   esi,9
    add   esi,0x100000
    mov   ecx,512/4
    cld
    rep   movsd

    popad

    xor   ebx,ebx
    xor   eax,eax
    ret

LBA_read:

; IN:
;
; eax = LBA block to read
; ebx = pointer to FIRST/SECOND/THIRD/FOURTH
; ecx = abs pointer to return area

    cmp   [lba_read_enabled],1
    je    lbarl1
    mov   eax,2
    ret

  lbarl1:

;    call  reserve_hd1

    push  eax
    push  ecx

    mov   edi,hd_address_table
    mov   esi,dir1
    mov   eax,[ebx]
    mov   edx,'1   '
    mov   ecx,4
  blar0:
    cmp   eax,[esi]
    je    blar2
    cmp   eax,edx
    je    blar2
    inc   edx
    add   edi,8
    add   esi,11
    dec   ecx
    jnz   blar0

    mov   eax,1
    mov   ebx,1
    jmp   LBA_read_ret

  blar2:
    mov   eax,[edi+0]
    mov   ebx,[edi+4]

    call  wait_for_hd_idle

    ; eax = hd port
    ; ebx = set for primary (0x00) or slave (0x10)

    cli

    mov   edx,eax
    inc   edx
    xor   eax,eax
    out   dx,al
    inc   edx
    inc   eax
    out   dx,al
    inc   edx
    mov   eax,[esp+4]
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
    add   al,bl
    add   al,128+64+32
    out   dx,al

    inc   edx
    mov   al,20h
    out   dx,al

    sti

    call  wait_for_sector_buffer

    cli

    mov   edi,[esp+0]
    mov   ecx,256
    sub   edx,7
    cld
    rep   insw

    sti

    xor   eax,eax
    xor   ebx,ebx

  LBA_read_ret:

    mov   [hd1_status],0
    add   esp,2*4

    ret


expand_pathz:
; IN:
;   esi = asciiz path & file
;   edi = buffer for path & file name
; OUT:
;   edi = directory & file : / 11 + / 11 + / 11 - zero terminated
;   ebx = /file name - zero terminated
;   esi = pointer after source

    push  eax
    push  ecx
    push  edi ;[esp+0]

  pathz_start:
    mov   byte [edi],'/'
    inc   edi
    mov   al,32
    mov   ecx,11
    cld
    rep   stosb                 ; clear filename area
    sub   edi,11
    mov   ebx,edi               ; start of dir/file name

  pathz_new_char:
    mov   al,[esi]
    inc   esi
    cmp   al,0
    je    pathz_end

    cmp   al,'/'
    jne   pathz_not_path
    cmp   edi,ebx               ; skip first '/'
    jz    pathz_new_char
    lea   edi,[ebx+11]          ; start of next directory
    jmp   pathz_start

  pathz_not_path:
    cmp   al,'.'
    jne   pathz_not_ext
    lea   edi,[ebx+8]           ; start of extension
    jmp   pathz_new_char

  pathz_not_ext:
    cmp   al,'a'
    jb    pathz_not_low
    cmp   al,'z'
    ja    pathz_not_low
    sub   al,0x20               ; char to uppercase

  pathz_not_low:
    mov   [edi],al
    inc   edi
    mov   eax,[esp+0]           ; start_of_dest_path
    add   eax,512               ; keep maximum path under 512 bytes
    cmp   edi,eax
    jb    pathz_new_char

  pathz_end:
    cmp   ebx,edi               ; if path end with '/'
    jnz   pathz_put_zero        ; go back 1 level
    sub   ebx,12

  pathz_put_zero:
    mov   byte [ebx+11],0
    dec   ebx                   ; include '/' char into file name
    pop   edi
    pop   ecx
    pop   eax
    ret

;*******************************************
;* string to number
;* input eax - 4 byte string
;* output eax - number
;*******************************************
StringToNumber:
;    ??????? ?????????? ????? ? ???????? ???
;    ????:
;        EDI - ????? ?????? ? ??????. ????? ????? ??????? ????? 0Dh
;    ?????:
;        CF - ????????? ??????:
;            0 - ?????? ???;
;            1 - ??????
;        ???? CF=0, ?? AX - ?????.

    push    bx
    push    cx
    push    dx
    push    edi
    mov   [partition_string],eax
    mov    edi,partition_string
    xor    cx,cx
i1: 
    mov    al,[edi]
    cmp    al,32  ;13
    je    i_exit
;    cmp    al,'0'
;    jb    err
;    cmp    al,'9'
;    ja    err
    sub    al,48
    shl    cx,1
    jc    err
    mov    bx,cx
    shl    cx,1
    jc    err
    shl    cx,1
    jc    err
    add    cx,bx
    jc    err
    cbw
    add    cx,ax
    jc    err
i3:
    inc    edi
    jmp    i1
i_exit:
    mov    ax,cx
    clc
i4:
    movzx  eax,ax
    pop    edi
    pop    dx
    pop    cx
    pop    bx
    ret

err:
    stc
    jmp    i4

partition_string: dd 0
                  db 32
