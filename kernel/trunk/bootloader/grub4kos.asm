;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2014. All rights reserved.      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Kolibri OS support loader for GRUB
;
; Copyright (C) Alex Nogueira Teixeira
; Copyright (C) Diamond
; Copyright (C) Dmitry Kartashov aka shurf
; Copyright (C) Serge
;
; Distributed under GPL, see file COPYING for details
;
; Version 1.0

lf              equ     0x0A
cr              equ     0x0D

use32


org 0x100000

mboot:
  dd  0x1BADB002
  dd  0x00010003
  dd  -(0x1BADB002 + 0x00010003)
  dd  mboot
  dd  0x100000
  dd  __edata
  dd  __end
  dd  __start

align 16
__start:

virtual at ebp+3
.BS_OEMName      rb 8
.BPB_BytsPerSec  rw 1           ; bytes per sector
.BPB_SecPerClus  rb 1           ; sectors per cluster
.BPB_RsvdSecCnt  rw 1           ; number of reserver sectors
.BPB_NumFATs     rb 1           ; count of FAT data structures
.BPB_RootEntCnt  rw 1           ; count of 32-byte dir. entries (224*32 = 14 sectors)
.BPB_TotSec16    rw 1           ; count of sectors on the volume (2880 for 1.44 mbytes disk)
.BPB_Media       rb 1           ; f0 - used for removable media
.BPB_FATSz16     rw 1           ; count of sectors by one copy of FAT
.BPB_SecPerTrk   rw 1           ; sectors per track
.BPB_NumHeads    rw 1           ; number of heads
.BPB_HiddSec     rd 1           ; count of hidden sectors
.BPB_TotSec32    rd 1           ; count of sectors on the volume (if > 65535)
end virtual

        cld
        mov     esi, mboot
        mov     edi, 0x80000
        mov     ecx, 600/4                      ;magic value
        rep movsd
        jmp     .check_mbi

org $-0x80000
align 4
.check_mbi:
        cmp     eax, 0x2BADB002
        mov     ecx, sz_invboot
        jne     .fault

        bt      dword [ebx], 3
        mov     ecx, sz_nomods
        jnc     .fault

        mov     edx, [ebx+20]                       ;mods_count
        mov     esi, [ebx+24]                       ;mods_addr
        cmp     edx, 1
        jne     .fault

.scan_mod:
        mov     ebp, [esi]                          ;image start
        mov     ecx, [esi+4]                        ;image end
        sub     ecx, ebp                            ;image size
        cmp     ecx, 512*18*80*2                    ;1.44 floppy
        jne     .fault

        mov     [_image_start], ebp
        mov     [_image_size], ecx

; calculate some disk parameters
; - beginning sector of RootDir

        movzx   eax, word [.BPB_FATSz16]
        movzx   ecx, byte [.BPB_NumFATs]
        mul     ecx
        add     ax, [.BPB_RsvdSecCnt]
        mov     [FirstRootDirSecNum], eax
        mov     esi, eax

; - count of sectors in RootDir
        movzx   ebx, word [.BPB_BytsPerSec]
        mov     cl, 5                           ; divide ax by 32
        shr     ebx, cl                         ; bx = directory entries per sector
        movzx   eax, word [.BPB_RootEntCnt]
        xor     edx, edx
        div     ebx
        mov     [RootDirSecs], eax

        ; - data start
        add     esi, eax                        ; add beginning sector of RootDir and count sectors in RootDir
        mov     [data_start], esi

; reading root directory
; al=count root dir sectrors !!!! TODO: al, max 255 sectors !!!!

        mov     eax, [FirstRootDirSecNum]
        mul     word [.BPB_BytsPerSec]
        lea     esi, [ebp+eax]

        mov     eax, [RootDirSecs]
        mul     word [.BPB_BytsPerSec]
        add     eax, esi                        ; EAX = end of root dir. in buffer pos_read_tmp

; find kernel file in root directory

.loop_find_dir_entry:
        push    esi
        mov     ecx, 11
        mov     edi, kernel_name
        rep cmpsb                               ; compare es:si and es:di, cx bytes long
        pop     esi
        je      .found_kernel_file
        add     esi, 32                         ; next dir. entry
        cmp     esi, eax                        ; end of directory
        jb      .loop_find_dir_entry

        mov     ecx, sz_kernel
        jmp     .fault

        ; === KERNEL FOUND. LOADING... ===

.found_kernel_file:

        movzx   ecx, word [esi+01ah]            ; first cluster of kernel file

        ; reading first FAT table
        movzx   eax, word [.BPB_RsvdSecCnt]     ; begin first FAT abs sector number
        mul     word [.BPB_BytsPerSec]
        lea     ebx, [ebp+eax]                  ; FAT address

;ebx = FAT
;ecx = cluster
;esi = src
;edi = dst
;ebp = image

; copy kernel file

        movzx   eax, word [.BPB_BytsPerSec]
        movsx   edx, byte [.BPB_SecPerClus]
        mul     edx
        shr     eax, 2
        mov     [cluster_size], eax

        mov     edi, 0x10000                    ;kernel base address

.copy_kernel:

        ; convert cluster number to sector number
        mov     eax, ecx                        ; data cluster to read
        sub     eax, 2
        movzx   edx, byte [.BPB_SecPerClus]
        mul     edx
        add     eax, [data_start]
        movzx   edx, word [.BPB_BytsPerSec]
        mul     edx

        lea     esi, [ebp+eax]
        mov     edx, ecx
        mov     ecx, [cluster_size]
        rep movsd
        mov     ecx, edx

        shr     edx, 1
        pushf
        add     edx, ecx                        ; di = bp * 1.5
        mov     ax, word [ebx+edx]             ; read next entry from FAT-chain
        popf
        jc      .move_4_right
        and     ax, 0fffh
        jmp     .verify_end_sector
.move_4_right:
        shr     ax, 4
.verify_end_sector:
        cmp     ax, 0ff8h                       ; last cluster
        jae     .execute_kernel
        movzx   ecx, ax
        jmp     .copy_kernel

.execute_kernel:

        mov     edi, 0x100000
        mov     esi, [_image_start]
        mov     ecx, [_image_size]
        shr     ecx, 2
        rep movsd
        xor     eax, eax
        mov     ecx, 1024
        rep stosd

        xor     ebx, ebx
        xor     ecx, ecx
        xor     edx, edx
        xor     esi, esi
        xor     edi, edi
        xor     ebp, ebp
        xor     esp, esp

        lgdt    [.tmp_gdt]
        jmp     far 0x08:.mode_16 and 0xFFFF

.fault:
;           push ecx
;           call _lcls
;           call __bprintf
._hlt:
        hlt
        jmp     ._hlt

align 8
.tmp_gdt: dw 15
          dd .tmp_gdt
          dw 0

.code16:  dw 0xFFFF
          dw 0
          db 8
          db 10011010b
          dw 0

use16
.mode_16:
        mov     eax, cr0
        and     eax, not 0x80000001
        mov     cr0, eax
        jmp     far 0x8000:.real_mode and 0xFFFF

.real_mode:
        xor     eax, eax
        mov     ds, ax
        mov     es, ax
        mov     ss, ax
        mov     gs, ax
        mov     fs, ax
        jmp     far 0x1000:0000

sz_invboot   db 'Invalid multiboot loader magic value',cr,lf
             db 'Halted',0

sz_nomods    db 'No modules loaded',cr,lf
             db 'Halted',0

sz_kernel    db cr,lf
kernel_name  db 'KERNEL  MNT ?',cr,lf,0

org $+0x80000
__edata:

align 4
_image_start        rd 1
_image_size         rd 1

FirstRootDirSecNum  rd 1
RootDirSecs         rd 1
data_start          rd 1
cluster_size        rd 1
__end:
