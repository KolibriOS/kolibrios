;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


read_ramdisk:
; READ RAMDISK IMAGE FROM HD

        cmp     [boot_dev+OS_BASE+0x10000], 1
        jne     no_sys_on_hd

        xor     ebp, ebp
.hd_loop:
        lea     eax, [ebp+'0']
        mov     [read_image_fsinfo.name_digit], al
        movzx   eax, byte [DRIVE_DATA+2+ebp]
        test    eax, eax
        jz      .next_hd
        push    eax
        mov     esi, 1
.partition_loop:
        mov     eax, esi
        push    -'0'
@@:
        xor     edx, edx
        div     [_10]
        push    edx
        test    eax, eax
        jnz     @b
        mov     edi, read_image_fsinfo.partition
@@:
        pop     eax
        add     al, '0'
        stosb
        jnz     @b
        mov     byte [edi-1], '/'
        push    esi edi
        mov     esi, bootpath1
        mov     ecx, bootpath1.len
        rep movsb
        call    read_image
        test    eax, eax
        jz      .yes
        cmp     eax, 6
        jz      .yes
        pop     edi
        push    edi
        mov     esi, bootpath2
        mov     ecx, bootpath2.len
        rep movsb
        call    read_image
        test    eax, eax
        jz      .yes
        cmp     eax, 6
        jz      .yes
        pop     edi esi
        inc     esi
        cmp     esi, [esp]
        jbe     .partition_loop
        pop     eax
.next_hd:
        inc     ebp
        cmp     ebp, 4
        jb      .hd_loop
        jmp     no_sys_on_hd
.yes:
        pop     edi esi eax
        jmp     yes_sys_on_hd

iglobal
align 4
read_image_fsinfo:
        dd      0               ; function: read
        dq      0               ; offset: zero
        dd      1474560         ; size
        dd      RAMDISK         ; buffer
        db      '/hd'
.name_digit db  '0'
        db      '/'
.partition:
        rb      64      ; should be enough for '255/KOLIBRI/KOLIBRI.IMG'

bootpath1       db      'KOLIBRI.IMG',0
.len = $ - bootpath1
bootpath2       db      'KOLIBRI/KOLIBRI.IMG',0
.len = $ - bootpath2
endg

read_image:
        mov     ebx, read_image_fsinfo
        pushad
        call    file_system_lfn_protected
        popad
        ret

no_sys_on_hd:
        ; test_to_format_ram_disk (need if not using ram disk)
        cmp     [boot_dev+OS_BASE+0x10000], 3
        jne     not_format_ram_disk
        ; format_ram_disk
        mov     edi, RAMDISK
        mov     ecx, 0x1080
        xor     eax, eax
@@:             
        stosd
        loop    @b

        mov     ecx, 0x58F7F
        mov     eax, 0xF6F6F6F6
@@:             
        stosd
        loop    @b
        
        mov     [RAMDISK+0x200], dword 0xFFFFF0         ; fat table
        mov     [RAMDISK+0x4200], dword 0xFFFFF0
        
not_format_ram_disk:
yes_sys_on_hd:
