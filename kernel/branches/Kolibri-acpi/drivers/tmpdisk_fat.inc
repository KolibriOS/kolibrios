; FAT-specific code for tmpdisk.asm.
; Formats a disk to FAT16 or FAT32, depending on size.
; Note: formatting is adjusted for memory-based disks. Although the resulting
; image is a valid FAT32 volume, it has no "spare" sectors, e.g. second copy
; of FAT or place for second sector of MS FAT32 bootloader.

; Some constants
FAT16_ROOTDIR_SECTORS = 16 ; can be changed, but why not?
; FAT16:
;     1 bootsector,
;     min 0xFF5 sectors for data,
;     min (0xFF5*2/512) = 16 sectors per FAT, we use only one copy,
;     FAT16_ROOTDIR_SECTORS for root directory
MIN_FAT16_SIZE = 1 + 16 + FAT16_ROOTDIR_SECTORS + 0xFF5
; FAT32:
;     1 bootsector,
;     1 sector for fsinfo,
;     min 0xFFF5 sectors for data,
;     min (0xFFF5*4/512) = 512 sectors per FAT, we use only one copy
MIN_FAT32_SIZE = 1 + 1 + 512 + 0xFFF5
MAX_SIZE = 1 shl (30 - 9) ; 1G in 512-byte sectors

; Initializes FATxx structures on the disk.
; Called with edi = pointer to disk data, esi = size of disk.
proc format_disk
; Determine FAT type and jump to the corresponding handler.
        cmp     esi, MIN_FAT32_SIZE
        jae     format_disk_fat32
; Fall through to format_disk_fat16.
endp

; Structure of FAT16 bootsector. Field names are from MS spec.
struc FAT16BOOT
{
.BS_jmpBoot     rb      3
.BS_OEMName     rb      8
.BPB_BytsPerSec dw      ?
.BPB_SecsPerClus db     ?
.BPB_RsvdSecCnt dw      ?
.BPB_NumFATs    db      ?
.BPB_RootEntCnt dw      ?
.BPB_TotSec16   dw      ?
.BPB_Media      db      ?
.BPB_FATSz16    dw      ?
.BPB_SecPerTrk  dw      ?
.BPB_NumHeads   dw      ?
.BPB_HiddSec    dd      ?
.BPB_TotSec32   dd      ?
.BS_DrvNum      db      ?
.BS_Reserved1   db      ?
.BS_BootSig     db      ?
.BS_VolID       dd      ?
.BS_VolLab      rb      11
.BS_FilSysType  rb      8
}
virtual at 0
FAT16BOOT FAT16BOOT
end virtual

; Initializes FAT16 structures on the disk.
; Called with edi = pointer to disk data, esi = size of disk.
format_disk_fat16:
; 1. Calculate number of clusters.
; 1a. There are fixed-sized areas and there are data+FAT;
; every cluster uses 512 bytes in data area and 2 bytes in FAT area.
        lea     eax, [esi-1-FAT16_ROOTDIR_SECTORS]
; two following lines are equivalent to edx = floor(eax*512/514)
        mov     ecx, 0xFF00FF01
        mul     ecx     ; edx = number of clusters
; 1b. Force the number be less than 0xfff5.
        mov     eax, 0xFFF4
        cmp     edx, eax
        jb      @f
        mov     edx, eax
@@:
; 2. Zero all system areas on the disk.
        lea     ecx, [256*(1+FAT16_ROOTDIR_SECTORS)+edx+255]
        and     ecx, not 255
        shr     ecx, 1
        xor     eax, eax
        push    edi
        rep stosd
        pop     edi
; 3. Generate the bootsector.
; 3a. Copy static stub.
        push    esi edi
        mov     esi, fat16bootsector_stub
        mov     ecx, fat16bootsector_stub_size
        rep movsb
        pop     edi esi
        mov     word [edi+510], 0xAA55
; 3b. Set fields which depend on size.
        cmp     esi, 0x10000
        jae     .size_is_32bit
        mov     [edi+FAT16BOOT.BPB_TotSec16], si
        jmp     .size_written
.size_is_32bit:
        mov     [edi+FAT16BOOT.BPB_TotSec32], esi
.size_written:
        lea     eax, [edx+255]
        shr     eax, 8
        mov     [edi+FAT16BOOT.BPB_FATSz16], ax
; 3c. Generate volume ID.
        call    generate_volume_id
        mov     [edi+FAT16BOOT.BS_VolID], eax
; 4. Initialize FAT.
        mov     dword [edi+512], 0xFFFFFFF8
; 5. Return.
        ret

; Structure of FAT32 bootsector. Field names are from MS spec.
struc FAT32BOOT
{
.BS_jmpBoot     rb      3
.BS_OEMName     rb      8
.BPB_BytsPerSec dw      ?
.BPB_SecsPerClus db     ?
.BPB_RsvdSecCnt dw      ?
.BPB_NumFATs    db      ?
.BPB_RootEntCnt dw      ?
.BPB_TotSec16   dw      ?
.BPB_Media      db      ?
.BPB_FATSz16    dw      ?
.BPB_SecPerTrk  dw      ?
.BPB_NumHeads   dw      ?
.BPB_HiddSec    dd      ?
.BPB_TotSec32   dd      ?
.BPB_FATSz32    dd      ?
.BPB_ExtFlags   dw      ?
.BPB_FSVer      dw      ?
.BPB_RootClus   dd      ?
.BPB_FSInfo     dw      ?
.BPB_BkBootSec  dw      ?
.BPB_Reserved   rb      12
.BS_DrvNum      db      ?
.BS_Reserved1   db      ?
.BS_BootSig     db      ?
.BS_VolID       dd      ?
.BS_VolLab      rb      11
.BS_FilSysType  rb      8
}
virtual at 0
FAT32BOOT FAT32BOOT
end virtual

; Initializes FAT32 structures on the disk.
; Called with edi = pointer to disk data, esi = size of disk.
format_disk_fat32:
; 1. Calculate number of clusters.
; 1a. There is fixed-sized area and there are data+FAT;
; every cluster uses 512 bytes in data area and 4 bytes in FAT area.
        lea     eax, [esi-1-1]
; two following lines are equivalent to edx=floor(eax*512/516) if eax<10000000h
        mov     ecx, 0xFE03F810
        mul     ecx     ; edx = number of clusters
; 2. Zero all system areas on the disk and first cluster of data,
; used for root directory.
        lea     ecx, [128*(1+1+1)+edx+127]
        and     ecx, not 127
        xor     eax, eax
        push    edi
        rep stosd
        pop     edi
; 3. Generate the bootsector.
; 3a. Copy static stub.
        push    esi edi
        mov     esi, fat32bootsector_stub
        mov     ecx, fat32bootsector_stub_size
        rep movsb
        pop     edi esi
        mov     word [edi+510], 0xAA55
; 3b. Set fields which depend on size.
        mov     [edi+FAT32BOOT.BPB_TotSec32], esi
        lea     eax, [edx+127]
        shr     eax, 7
        mov     [edi+FAT32BOOT.BPB_FATSz32], eax
; 3c. Generate volume ID.
        call    generate_volume_id
        mov     [edi+FAT32BOOT.BS_VolID], eax
; 4. Initialize fsinfo sector.
        mov     dword [edi+512], 'RRaA'
        mov     dword [edi+512+484], 'rrAa'
        dec     edx ; one cluster is occupied by root dir
        mov     dword [edi+512+488], edx ; free count
        mov     byte [edi+512+492], 3 ; first free cluster
        mov     word [edi+512+510], 0xAA55
; 5. Initialize FAT.
        mov     dword [edi+512*2], 0x0FFFFFF8
        mov     dword [edi+512*2+4], 0x0FFFFFFF
        mov     dword [edi+512*2+8], 0x0FFFFFFF
; 6. Return.
        ret

; Generate volume serial number, which should try to be unique for each volume.
; Use CMOS date+time, copy-pasted from fat32.inc.
generate_volume_id:
        call    get_time_for_file
        mov     cx, ax
        call    get_date_for_file
        shl     eax, 16
        mov     ax, cx
        ret

; Three following procedures are copy-pasted from fat32.inc.
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

; some data
fat16bootsector_stub:
        db 0EBh, 3Ch, 90h       ; BS_jmpBoot
        db 'KOLIBRI '           ; BS_OEMName
        dw 512                  ; BPB_BytsPerSec
        db 1                    ; BPB_SecsPerClus
        dw 1                    ; BPB_RsvdSecCnt
        db 1                    ; BPB_NumFATs
        dw FAT16_ROOTDIR_SECTORS*16 ; BPB_RootEntCnt
        dw 0                    ; BPB_TotSec16, filled in format_disk_fat16
        db 0F8h                 ; BPB_Media
        dw 0                    ; BPB_FATSz16, filled in format_disk_fat16
        dw 32                   ; BPB_SecPerTrk
        dw 128                  ; BPB_NumHeads
        dd 0                    ; BPB_HiddSec
        dd 0                    ; BPB_TotSec32, filled in format_disk_fat16
        db 80h                  ; BS_DrvNum
        db 0                    ; BS_Reserved1
        db 29h                  ; BS_BootSig
        dd 0                    ; BS_VolID, filled in format_disk_fat16
        db 'NO NAME    '        ; BS_VolLab
        db 'FAT16   '           ; BS_FilSysType
; just in case add some meaningful bytes if someone tries to boot
        db 0CDh, 19h, 0EBh, 0FEh ; int 19h, jmp $
fat16bootsector_stub_size = $ - fat16bootsector_stub
fat32bootsector_stub:
        db 0EBh, 58h, 90h       ; BS_jmpBoot
        db 'KOLIBRI '           ; BS_OEMName
        dw 512                  ; BPB_BytsPerSec
        db 1                    ; BPB_SecsPerClus
        dw 2                    ; BPB_RsvdSecCnt
        db 1                    ; BPB_NumFATs
        dw 0                    ; BPB_RootEntCnt
        dw 0                    ; BPB_TotSec16
        db 0F8h                 ; BPB_Media
        dw 0                    ; BPB_FATSz16
        dw 32                   ; BPB_SecPerTrk
        dw 128                  ; BPB_NumHeads
        dd 0                    ; BPB_HiddSec
        dd 0                    ; BPB_TotSec32, filled in format_disk_fat32
        dd 0                    ; BPB_FATSz32, filled in format_disk_fat32
        dw 0                    ; BPB_ExtFlags
        dw 0                    ; BPB_FSVer
        dd 2                    ; BPB_RootClus
        dw 1                    ; BPB_FSInfo
        dw 0                    ; BPB_BkBootSec
        rb 12                   ; BPB_Reserved
        db 80h                  ; BS_DrvNum
        db 0                    ; BS_Reserved1
        db 29h                  ; BS_BootSig
        dd 0                    ; BS_VolID, filled in format_disk_fat32
        db 'NO NAME    '        ; BS_VolLab
        db 'FAT32   '           ; BS_FilSysType
; same bytes as in fat16bootsector_stub
        db 0CDh, 19h, 0EBh, 0FEh ; int 19h, jmp $
fat32bootsector_stub_size = $ - fat32bootsector_stub
