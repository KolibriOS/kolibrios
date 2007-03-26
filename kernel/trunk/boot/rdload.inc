$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; READ RAMDISK IMAGE FROM HD

        cmp   [boot_dev],1
        jne   no_sys_on_hd

        test  [DRIVE_DATA+1],byte 0x40
        jz    position_2
        mov   [hdbase],0x1f0
        mov   [hdid],0x0
        mov   [hdpos],1
        mov   [fat32part],0
  position_1_1:
        inc   [fat32part]
        call  search_and_read_image
        cmp   [image_retrieved],1
        je    yes_sys_on_hd
        movzx eax,byte [DRIVE_DATA+2]
        cmp   [fat32part],eax
        jle     position_1_1
  position_2:
        test  [DRIVE_DATA+1],byte 0x10
        jz    position_3
        mov   [hdbase],0x1f0
        mov   [hdid],0x10
        mov   [hdpos],2
        mov   [fat32part],0
  position_2_1:
        inc   [fat32part]
        call  search_and_read_image
        cmp   [image_retrieved],1
        je    yes_sys_on_hd
        movzx eax,byte [DRIVE_DATA+3]
        cmp   eax,[fat32part]
        jle     position_2_1
  position_3:
        test  [DRIVE_DATA+1],byte 0x4
        jz    position_4
        mov   [hdbase],0x170
        mov   [hdid],0x0
        mov   [hdpos],3
        mov   [fat32part],0
  position_3_1:
        inc   [fat32part]
        call  search_and_read_image
        cmp   [image_retrieved],1
        je    yes_sys_on_hd
        movzx eax,byte [DRIVE_DATA+4]
        cmp   eax,[fat32part]
        jle     position_3_1
  position_4:
        test  [DRIVE_DATA+1],byte 0x1
        jz    no_sys_on_hd
        mov   [hdbase],0x170
        mov   [hdid],0x10
        mov   [hdpos],4
        mov   [fat32part],0
  position_4_1:
        inc   [fat32part]
        call  search_and_read_image
        cmp   [image_retrieved],1
        je    yes_sys_on_hd
        movzx eax,byte [DRIVE_DATA+5]
        cmp   eax,[fat32part]
        jle     position_4_1
        jmp   yes_sys_on_hd

  search_and_read_image:
        call  set_FAT32_variables
        mov   edx, bootpath
        call  read_image
        test  eax, eax
        jz   image_present
        mov   edx, bootpath2
        call  read_image
        test  eax, eax
        jz   image_present
        ret
    image_present:
        mov   [image_retrieved],1
        ret

read_image:
        mov   eax, hdsysimage
        mov   ebx, 1474560/512
        mov   ecx, RAMDISK
        mov   esi, 0
        mov   edi, 12
        call  file_read
        ret

image_retrieved  db 0
counter_of_partitions db 0
no_sys_on_hd:
yes_sys_on_hd:
