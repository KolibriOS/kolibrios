;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;                                                              ;;
;; System service for filesystem call                                   ;;
;; (C) 2004 Ville Turjanmaa, License: GPL                               ;;
;; 29.04.2006 Elimination of hangup after the                           ;;
;;            expiration hd_wait_timeout (for LBA) -  Mario79           ;;
;; 15.01.2005 get file size/attr/date,                          ;;
;;            file_append (only for hd) - ATV                   ;;
;; 23.11.2004 test if hd/partition is set - ATV                         ;;
;; 18.11.2004 get_disk_info and more error codes - ATV                  ;;
;; 08.11.2004 expand_pathz and rename (only for hd) - ATV               ;;
;; 20.10.2004 Makedir/Removedir (only for hd) - ATV                     ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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
; eax = 0  ; read file          /RamDisk/First  6
; eax = 1  ; write file         /RamDisk/First 33   /HardDisk/First 56
; eax = 8  ; lba read
; eax = 15 ; get_disk_info
;
; OUT:
;
; eax = 0  : read ok
; eax = 1  : no hd base and/or partition defined
; eax = 2  : function is unsupported for this FS
; eax = 3  : unknown FS
; eax = 4  : partition not defined at hd
; eax = 5  : file not found
; eax = 6  : end of file
; eax = 7  : memory pointer not in application area
; eax = 8  : disk full
; eax = 9  : fat table corrupted
; eax = 10 : access denied
; eax = 11 : disk error
;
; ebx = size

; \begin{diamond}[18.03.2006]
; for subfunction 16 (start application) error codes must be negative
;    because positive values are valid PIDs
; so possible return values are:
; eax > 0 : process created, eax=PID

; -0x10 <= eax < 0 : -eax is filesystem error code:
; eax = -1  = 0xFFFFFFFF : no hd base and/or partition defined
; eax = -3  = 0xFFFFFFFD : unknown FS
; eax = -5  = 0xFFFFFFFB : file not found
; eax = -6  = 0xFFFFFFFA : unexpected end of file (probably not executable file)
; eax = -9  = 0xFFFFFFF7 : fat table corrupted
; eax = -10 = 0xFFFFFFF6 : access denied

; -0x20 <= eax < -0x10: eax is process creation error code:
; eax = -0x20 = 0xFFFFFFE0 : too many processes
; eax = -0x1F = 0xFFFFFFE1 : not Menuet/Kolibri executable
; eax = -0x1E = 0xFFFFFFE2 : no memory

; ebx is not changed

; \end{diamond}[18.03.2006]

    ; Extract parameters
 ;   add    eax, std_application_base_address    ; abs start of info block

    cmp   dword [eax+0],15      ; GET_DISK_INFO
    je    fs_info

    cmp   dword [CURRENT_TASK],1      ; no memory checks for kernel requests
    jz    no_checks_for_kernel
    mov   edx,eax
    cmp   dword [eax+0],1
    jnz   .usual_check
    mov   ebx,[eax+12]
 ;   add   ebx,std_application_base_address
    mov   ecx,[eax+8]
    call  check_region
    test  eax,eax
    jnz   area_in_app_mem

.error_output:
    mov   esi,buffer_failed
    call  sys_msg_board_str
;    mov   eax,7
    mov   dword [esp+36],7
    ret
iglobal
  buffer_failed db 'K : Buffer check failed',13,10,0
endg
.usual_check:
    cmp   dword [eax+0],0
    mov   ecx,512
    jnz   .small_size
    mov   ecx,[eax+8]
    shl   ecx,9
.small_size:
    mov   ebx,[eax+12]
 ;   add   ebx,std_application_base_address
    call  check_region
    test  eax,eax
    jz    .error_output
  area_in_app_mem:
    mov   eax,edx
  no_checks_for_kernel:

  fs_read:

    mov   ebx,[eax+20]          ; program wants root directory ?
    test  bl,bl
    je    fs_getroot
    test  bh,bh
    jne   fs_noroot
  fs_getroot:
; \begin{diamond}[18.03.2006]
; root - only read is allowed
; other operations return "access denied", eax=10
; (execute operation returns eax=-10)
    cmp    dword [eax], 0
    jz    .read_root
    mov    dword [esp+36], 10
    ret
.read_root:
; \end{diamond}[18.03.2006]
    mov   esi,dir0
    mov   edi,[eax+12]
 ;   add   edi,std_application_base_address
    mov   ecx,11
    push  ecx
;    cld    ; already is
    rep   movsb
    mov   al,0x10
    stosb
    add   edi,32-11-1
    pop   ecx
    rep   movsb
    stosb
    and   dword [esp+36],0      ; ok read
    mov   dword [esp+24],32*2   ; size of root
    ret

  fs_info:                      ;start of code - Mihasik
    push  eax
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

    push  dword [eax+0]         ; read/write/delete/.../makedir/rename/lba/run
    push  dword [eax+4]         ; 512 block number to read
    push  dword [eax+8]         ; bytes to write/append or 512 blocks to read
    mov   ebx,[eax+12]
 ;   add   ebx,std_application_base_address
    push  ebx                   ; abs start of return/save area

    lea   esi,[eax+20]          ; abs start of dir + filename
    mov   edi,[eax+16]
 ;   add   edi,std_application_base_address    ; abs start of work area

    call  expand_pathz

    push  edi                   ; dir start
    push  ebx                   ; name of file start

    mov   eax,[edi+1]
    cmp   eax,'RD  '
    je    fs_yesramdisk
    cmp   eax,'RAMD'
    jne   fs_noramdisk

  fs_yesramdisk:

    cmp   byte [edi+1+11],0
    je    fs_give_dir1

    mov   eax,[edi+1+12]
    cmp   eax,'1   '
    je    fs_yesramdisk_first
    cmp   eax,'FIRS'
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
  fs_noramdisk:

  ;********************************************************************
    mov   eax,[edi+1]
    cmp   eax,'FD  '
    je    fs_yesflpdisk
    cmp   eax,'FLOP'
    jne   fs_noflpdisk

  fs_yesflpdisk:
    call   reserve_flp

    cmp   byte [edi+1+11],0
    je    fs_give_dir1

    mov   eax,[edi+1+12]
    cmp   eax,'1   '
    je    fs_yesflpdisk_first
    cmp   eax,'FIRS'
    je    fs_yesflpdisk_first
    cmp   eax,'2   '
    je    fs_yesflpdisk_second
    cmp   eax,'SECO'
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
        call    reserve_hd_channel
;    call  choice_necessity_partition
;    jmp   fs_yesharddisk_all
    jmp   fs_for_new_semantic

choice_necessity_partition:
    mov   eax,[edi+1+12]
    call  StringToNumber
        mov   [fat32part],eax
choice_necessity_partition_1:
    mov   ecx,[hdpos]
    xor   eax,eax
    mov   [hd_entries], eax    ; entries in hd cache
    mov   edx,DRIVE_DATA+2
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
    add   eax,DRIVE_DATA+0xa
    mov   [transfer_adress],eax
    call  partition_data_transfer_1
    ret

 old_path_harddisk:
    mov   eax,[edi+1]
    cmp   eax,'HD  '
    je    fs_yesharddisk
    cmp   eax,'HARD'
    jne   fs_noharddisk

  fs_yesharddisk:
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
    call  reserve_hd1
 fs_for_new_semantic:
    call  choice_necessity_partition

  fs_yesharddisk_all:
    mov   eax,1
    mov    ebx, [esp+24+24]
    cmp   [hdpos],0             ; is hd base set?
    jz    hd_err_return
    cmp   [fat32part],0         ; is partition set?
    jnz   @f
hd_err_return:
    call  free_hd_channel
    and   [hd1_status], 0
    jmp   file_system_return
@@:

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

    call  free_hd_channel
    and   [hd1_status], 0
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

    call  free_hd_channel
    and   [hd1_status], 0
    jmp   file_system_return


  fs_noharddisk_write:


    call  free_hd_channel
    and   [hd1_status], 0

  fs_noharddisk:
; \begin{diamond}[18.03.2006]
    mov    eax, 5        ; file not found
; а может быть, возвращать другой код ошибки?
    mov    ebx, [esp+24+24]    ; do not change ebx in application
; \end{diamond}[18.03.2006]

  file_system_return:

    add   esp,24

    mov   [esp+36],eax
    mov   [esp+24],ebx
    ret


  fs_give_dir1:

; \begin{diamond}[18.03.2006]
; /RD,/FD,/HD - only read is allowed
; other operations return "access denied", eax=10
; (execute operation returns eax=-10)
    cmp    dword [esp+20], 0
    jz    .read
    add    esp, 20
    pop    ecx
    mov    dword [esp+36], 10
    ret
.read:
; \end{diamond}[18.03.2006]
    mov   al,0x10
    mov   ebx,1
    mov   edi,[esp+8]
    mov   esi,dir1
  fs_d1_new:
    mov   ecx,11
;    cld
    rep   movsb
    stosb
    add   edi,32-11-1
    dec   ebx
    jne   fs_d1_new

    add   esp,24

    and   dword [esp+36],0      ; ok read
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
    add   esi,RAMDISK
    mov   ecx,512/4
;    cld
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

    call  reserve_hd1

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

    mov  [hdbase],eax
    mov  [hdid],ebx

    call  wait_for_hd_idle
    cmp   [hd_error],0
    jne   hd_lba_error

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
    cmp   [hd_error],0
    jne   hd_lba_error

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
    mov [hd_error],0
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
;    ПЕРЕВОД СТРОКОВОГО ЧИСЛА В ЧИСЛОВОЙ ВИД
;    Вход:
;        EDI - адрес строки с числом. Конец числа отмечен кодом 0Dh
;    Выход:
;        CF - индикатор ошибок:
;            0 - ошибок нет;
;            1 - ошибка
;        Если CF=0, то AX - число.

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
