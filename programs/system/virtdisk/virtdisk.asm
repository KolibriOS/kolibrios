;-----------------------------------------------------------------------------;
;      Copyright (C) 2023, Mikhail Frolov aka Doczom . All rights reserved.   ;
;           Distributed under terms of the GNU General Public License         ;
;                                                                             ;
;                   Demo program for the VIRT_DISK driver.                    ;
;                                                                             ;
;                       GNU GENERAL PUBLIC LICENSE                            ;
;                          Version 2, June 1991                               ;
;                                                                             ;
;-----------------------------------------------------------------------------;
format  binary as ""
  use32
  org    0
  db     'MENUET01'
  dd     1, START, I_END, MEM, STACKTOP, PATH, 0

include 'parser.inc'
include 'shell.inc'
START:
        call    _sc_init

        mov     al, 68
        mov     bl, 16
        mov     ecx, drv_name
        int     0x40
        mov     [ioctl_add_disk.hand], eax
        mov     [ioctl_del_disk.hand], eax
        mov     [ioctl_info_disk.hand], eax
        mov     [ioctl_list_disk.hand], eax
        mov     [ioctl_count_disk.hand], eax
        test    eax, eax
        jz      .end

        cmp     byte[PATH], 0
        jz      .end

        call    parse_cmd

        cmp     dword[param_cmd],0
        jz      .end

        mov     al, 68
        mov     bl, 17
        mov     ecx, ioctl_add_disk
        int     0x40
        test    eax, eax
        jnz     @f
        push    str_command_successfully
        call    _sc_puts
        jmp     .end
@@:
        push    str_error
        call    _sc_puts
.end:
        call    _sc_exit
        mov     eax,-1
        int     0x40
ERROR_EXIT:
        push    str_runtime_err
        call    _sc_puts

        call    _sc_exit
        mov     eax,-1
        int     0x40
write_disk_info:
        pusha
        push    str_disk_info.path
        call    _sc_puts

        push    info_buffer.path
        call    _sc_puts

        push    str_newline
        call    _sc_puts
        popa
        ret


I_END:
drv_name: db 'VIRT_DISK',0

; messages
str_runtime_err:
        db 'Runtime error', 13, 10, 0
str_command_successfully:
        db 'Command successfully', 13, 10, 0
str_header_disk_list:
        db ' disk  | sector | flags | file', 13, 10
        db '-------|--------|-------|---------------------',13, 10, 0
str_input_disk_number:
        db '       | ' ; ,0
str_input_disk_sector:
        db '       | ';,0
str_input_disk_flags:
        db '      | ',0
str_error:
        db 'Error',0
str_disk_info:
.num:   db 'Disk number: ',0
;.type:  db 'Type: ', 0
;.sector_size:
;        db 'Sector size: ', 0
.path:  db 'File: ', 0
str_newline:
        db ' ', 13, 10, 0

ioctl_count_disk:
.hand:  dd      0, 3 ;iocode
        dd      .get_count_disk, 8
        dd      ioctl_list_disk.count, 4
.get_count_disk:
        dd      0, 0

ioctl_list_disk:
.hand:  dd      0, 3 ;iocode
        dd      .inp, 8
.buffer:
        dd      0
.size_buffer:
        dd      0
.inp:
        dd      0
.count: dd      0

ioctl_info_disk:
.hand:  dd      0, 4 ;iocode
        dd      disk_num, 4
        dd      info_buffer, sizeof.info_buffer
ioctl_del_disk:
.hand:  dd      0, 2 ;iocode
        dd      disk_num, 4
        dd      0, 0
ioctl_add_disk:
.hand:  dd      0, 1 ;iocode
        dd      add_disk, add_disk.end - add_disk
        dd      disk_num, 4

disk_num: rd 0
add_disk:
.flags: dd 11b ;rw
.type:  dd 0   ; TypeImage 0 - RAW
.size:  dd 512
.file:  rb 4096
.end:

PATH:   rb 4096

info_buffer:
.sector_count: rd 2
.disk_hand:    rd 1
.disk_num:     rd 1
.flags:        rd 1
.type:         rd 1
.sector_size:  rd 1
.path:         rb 4096
sizeof.info_buffer =  $ - info_buffer

        rb      4096
STACKTOP:
MEM:
; EXAMPLE COMMANDS:
; virtdisk -f/sd0/4/kolibri.img -s512
; virtdisk -f/sd0/4/kolibri.img
; virtdisk -f/sd0/4/kolibri.iso -s2048
; default sector size = 512 for all disk
;                       2048 for ISO disk

;struct  IMAGE_ADD_STRUCT
;        Flags           rd      1 ; 1-ro 2-wo 3-rw
;        TypeImage       rd      1 ; 0-raw 1-vhd 2-vdi 3-imd
;        SectorSize      rd      1
;        DiskPath        rb      maxPathLength
;ends