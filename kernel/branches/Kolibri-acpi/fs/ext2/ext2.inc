;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Contains ext2 structures, and macros.                        ;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2013-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 5363 $


; Future jobs for driver, in order of preference:
;     * clean up existing extents support.
;     * add b-tree directories support.
;     * add long file support.
;     * add journal support.
;     * add minor features that come with ext3/4.

; Recommended move to some kernel-wide bitmap handling code (with a bit of abstraction, of course).

;---------------------------------------------------------------------
; Clears a bit.
; Input:        eax = index into bitmap.
;               [EXTFS.ext2_save_block] = address of bitmap.
;               ebp = address of EXTFS.
; Output:       Bit cleared.
;               eax = non-zero, if already cleared.
;---------------------------------------------------------------------
bitmap_clear_bit:
        push    ebx ecx edx

        xor     edx, edx
        mov     ecx, 8
        div     ecx

        add     eax, [ebp + EXTFS.ext2_save_block]
        
        ; Get the mask.
        mov     ebx, 1
        mov     ecx, edx
        shl     ebx, cl

        test    [eax], ebx
        jz      .cleared

        not     ebx
        and     [eax], ebx

        xor     eax, eax
    .return:
        pop     edx ecx ebx
        ret

    ; Already cleared.
    .cleared:
        xor     eax, eax
        not     eax
        jmp     .return

;---------------------------------------------------------------------
; Finds free bit in the bitmap.
; Input:        ecx = number of bits in the bitmap.
;               [EXTFS.ext2_save_block] = address of bitmap.
;               ebp = address of EXTFS.
; Output:       eax = index of free bit in the bitmap; marked set.
;                     0xFFFFFFFF if no free bit found.
;---------------------------------------------------------------------
ext2_find_free_bit:
bitmap_find_free_bit:
        push    esi ebx ecx edx
        mov     esi, [ebp + EXTFS.ext2_save_block]

        ; Get total DWORDS in eax; total bits in last dword, if any, in edx.
        xor     edx, edx
        mov     eax, ecx
        mov     ecx, 32
        div     ecx

        mov     ecx, eax
        xor     eax, eax
        push    edx

        test    ecx, ecx
        jz      .last_bits

    ; Check in the DWORDS.
    .dwords:
        mov     ebx, [esi]
        not     ebx

        bsf     edx, ebx

        ; If 0, then the original value would be 0xFFFFFFFF, hence no free bits.
        jz      @F

        ; We found the value. Let's return with it.
        add     esp, 4

        add     eax, edx
        jmp     .return
        
    @@:
        add     esi, 4
        add     eax, 32
        loop    .dwords

    .last_bits:
        ; Check in the last few bits.
        pop     ecx
        test    ecx, ecx
        jz      @F
        
        mov     ebx, [esi]
        not     ebx
        bsf     ebx, edx

        ; If 0, no free bits.
        jz      @F

        ; If free bit is greater than the last known bit, then error.
        cmp     edx, ecx
        jg      @F

        add     eax, edx
        jmp     .return

    @@:
        ; Didn't find any free bits.
        xor     eax, eax
        not     eax
        jmp     @F

    .return:
        mov     ecx, edx
        mov     edx, 1
        shl     edx, cl
        or      [esi], edx

    @@:
        pop     edx ecx ebx esi
        ret

; Recommended move to some kernel-wide string handling code.
;---------------------------------------------------------------------
; Find the length of a string.
; Input:        esi = source.
; Output:       length in ecx
;---------------------------------------------------------------------
strlen:
        push    eax esi
        xor     ecx, ecx

    @@:
        lodsb
        test    al, al
        jz      .ret

        inc     ecx
        jmp     @B

    .ret:
        pop     esi eax
        ret

;---------------------------------------------------------------------
; Convert UTF-8 string to ASCII-string (codepage 866)
; Input:        esi = source.
;               edi = buffer.
;               ecx = length of source.
; Output:       destroys eax, esi, edi
;---------------------------------------------------------------------
utf8_to_cp866:
        ; Check for zero-length string.
        jecxz   .return

    .start:
        lodsw
        cmp     al, 0x80
        jb      .ascii

        xchg    al, ah                                  ; Big-endian.
        cmp     ax, 0xd080
        jz      .yo1

        cmp     ax, 0xd191
        jz      .yo2

        cmp     ax, 0xd090
        jb      .unk

        cmp     ax, 0xd180
        jb      .rus1

        cmp     ax, 0xd190
        jb      .rus2

    .unk:
        mov     al, '_'
        jmp     .doit
 
    .yo1:
        mov     al, 0xf0                                ; Ё capital.
        jmp     .doit

    .yo2:
        mov     al, 0xf1                                ; ё small.
        jmp     .doit

    .rus1:
        sub     ax, 0xd090 - 0x80
        jmp     .doit

    .rus2:
        sub     ax, 0xd18f - 0xEF
    
    .doit:
        stosb
        sub     ecx, 2
        ja      .start
        ret

    .ascii:
        stosb
        dec     esi
        dec     ecx
        jnz     .start
 
    .return:
        ret

; Recommended move to some kernel-wide time handling code.

; Total cumulative seconds till each month.
cumulative_seconds_in_month:
        .january:       dd 0 * (60 * 60 * 24)
        .february:      dd 31 * (60 * 60 * 24)
        .march:         dd 59 * (60 * 60 * 24)
        .april:         dd 90 * (60 * 60 * 24)
        .may:           dd 120 * (60 * 60 * 24)
        .june:          dd 151 * (60 * 60 * 24)
        .july:          dd 181 * (60 * 60 * 24)
        .august:        dd 212 * (60 * 60 * 24)
        .september:     dd 243 * (60 * 60 * 24)
        .october:       dd 273 * (60 * 60 * 24)
        .november:      dd 304 * (60 * 60 * 24)
        .december:      dd 334 * (60 * 60 * 24)

current_bdfe_time:
        dd 0
current_bdfe_date:
        dd 0

;---------------------------------------------------------------------
; Stores current unix time.
; Input:        edi = buffer to output Unix time.
;---------------------------------------------------------------------
current_unix_time:
        push    eax esi
        mov     esi, current_bdfe_time
        
        ; Just a small observation:
        ; The CMOS is a pretty bad source to get time from. One shouldn't rely on it,
        ; since it messes up the time by tiny bits. Of course, this is all technical,
        ; but one can look it up on the osdev wiki. What is better is to get the time
        ; from CMOS during boot, then update system time using a more accurate timer.
        ; I'll probably add that after the Summer of Code, so TODO! TODO! TODO!.

        ; Get time from CMOS.
        ; Seconds.
        mov     al, 0x00
        out     0x70, al
        in      al, 0x71
        call    bcd2bin
        mov     [esi + 0], al

        ; Minute.
        mov     al, 0x02
        out     0x70, al
        in      al, 0x71
        call    bcd2bin
        mov     [esi + 1], al

        ; Hour.
        mov     al, 0x04
        out     0x70, al
        in      al, 0x71
        call    bcd2bin
        mov     [esi + 2], al

        ; Get date.

        ; Day.
        mov     al, 0x7
        out     0x70, al
        in      al, 0x71
        call    bcd2bin
        mov     [esi + 4], al

        ; Month.
        mov     al, 0x8
        out     0x70, al
        in      al, 0x71
        call    bcd2bin
        mov     [esi + 5], al

        ; Year.
        mov     al, 0x9
        out     0x70, al
        in      al, 0x71
        call    bcd2bin
        add     ax, 2000        ; CMOS only returns last two digits.
                                ; Note that everywhere in KolibriOS this is used.
                                ; This is hacky, since the RTC can be incorrectly set
                                ; to something before 2000.
        mov     [esi + 6], ax

        call    bdfe_to_unix_time
        pop     esi eax
        ret

;---------------------------------------------------------------------
; Convert time+date from BDFE to Unix time.
; Input:        esi = pointer to BDFE time+date.
;               edi = buffer to output Unix time.
;---------------------------------------------------------------------
bdfe_to_unix_time:
        push    eax ebx ecx edx
        mov     dword[edi], 0x00000000
     
        ; The minimum representable time is 1901-12-13.
        cmp     word[esi + 6], 1901
        jb      .ret
        jg      .max

        cmp     byte[esi + 5], 12
        jb      .ret

        cmp     byte[esi + 4], 13
        jbe     .ret
        jg      .convert

    ; Check if it is more than the maximum representable time.
    .max:
        ; The maximum representable time is 2038-01-19.
        cmp     word[esi + 6], 2038
        jg      .ret
        jb      .convert

        cmp     byte[esi + 5], 1
        jg      .ret

        cmp     byte[esi + 4], 19
        jge     .ret

    ; Convert the time.
    .convert:
        ; Get if current year is leap year in ECX.
        xor     ecx, ecx
        mov     ebx, 4
        xor     edx, edx

        cmp     word[esi + 6], 1970
        jb      .negative

        movzx   eax, word[esi + 6]              ; Year.
        cmp     byte[esi + 5], 3                ; If the month is less than March, than that year doesn't matter.
        jge     @F

        test    eax, 3
        ; Not a leap year.
        jnz     @F

        inc     ecx
    @@:
        ; Number of leap years between two years = ((end date - 1)/4) - (1970/4)
        dec     eax
        div     ebx
        sub     eax, 1970/4

        ; EAX is the number of leap years.
        add     eax, ecx
        mov     ecx, (60 * 60 * 24)             ; Seconds in a day.
        mul     ecx

        ; Account for leap years, i.e., one day extra for each.
        add     [edi], eax

        ; Get total days in EAX.
        movzx   eax, byte[esi + 4]
        dec     eax
        mul     ecx

        ; Account for days.
        add     [edi], eax

        ; Account for month.
        movzx   eax, byte[esi + 5]
        dec     eax
        mov     eax, [cumulative_seconds_in_month + (eax * 4)]
        add     [edi], eax

        ; Account for year.
        movzx   eax, word[esi + 6]
        sub     eax, 1970
        mov     ecx, (60 * 60 * 24) * 365       ; Seconds in a year.
        mul     ecx
        add     [edi], eax

        ; Seconds.
        movzx   eax, byte[esi + 0]
        add     [edi], eax        

        ; Minutes.
        movzx   eax, byte[esi + 1]
        mov     ecx, 60
        mul     ecx
        add     [edi], eax

        ; Hours.
        movzx   eax, byte[esi + 2]
        mov     ecx, (60 * 60)
        mul     ecx
        add     [edi], eax  

    ; The time wanted is before the epoch; handle it here.
    .negative:
        ; TODO.

    .ret:
        pop     edx ecx ebx eax
        ret

; Recommended move to some kernel-wide alloc handling code.
macro KERNEL_ALLOC store, label
{
        call    kernel_alloc
        mov     store, eax
        test    eax, eax    
        jz      label   
}

macro KERNEL_FREE data, label
{
        cmp     data, 0
        jz      label
        push    data
        call    kernel_free
}

struct EXTFS PARTITION
        lock MUTEX
        partition_flags                dd ?
        log_block_size                 dd ?
        block_size                     dd ?
        count_block_in_block           dd ?
        blocks_per_group               dd ?
        global_desc_table              dd ?
        root_inode                     dd ?         ; Pointer to root inode in memory.
        inode_size                     dd ?
        count_pointer_in_block         dd ?         ; (block_size / 4)
        count_pointer_in_block_square  dd ?         ; (block_size / 4)**2
        ext2_save_block                dd ?         ; Block for 1 global procedure.
        ext2_temp_block                dd ?         ; Block for small procedures.
        ext2_save_inode                dd ?         ; inode for global procedures.
        ext2_temp_inode                dd ?         ; inode for small procedures.
        groups_count                   dd ?
        superblock                     rd 1024/4
ends

; EXT2 revisions.
EXT2_GOOD_OLD_REV    = 0

; For fs_type.
FS_TYPE_UNDEFINED    = 0
FS_TYPE_EXT          = 2

; Some set inodes.
EXT2_BAD_INO         = 1
EXT2_ROOT_INO        = 2
EXT2_ACL_IDX_INO     = 3
EXT2_ACL_DATA_INO    = 4
EXT2_BOOT_LOADER_INO = 5
EXT2_UNDEL_DIR_INO   = 6

; EXT2_SUPER_MAGIC.
EXT2_SUPER_MAGIC     = 0xEF53
EXT2_VALID_FS        = 1

; Flags defining i_mode values.
EXT2_S_IFMT          = 0xF000           ; Mask for file type.

EXT2_S_IFREG         = 0x8000           ; Regular file.
EXT2_S_IFDIR         = 0x4000           ; Directory.

EXT2_S_IRUSR         = 0x0100           ; User read
EXT2_S_IWUSR         = 0x0080           ; User write
EXT2_S_IXUSR         = 0x0040           ; User execute
EXT2_S_IRGRP         = 0x0020           ; Group read
EXT2_S_IWGRP         = 0x0010           ; Group write
EXT2_S_IXGRP         = 0x0008           ; Group execute
EXT2_S_IROTH         = 0x0004           ; Others read
EXT2_S_IWOTH         = 0x0002           ; Others write
EXT2_S_IXOTH         = 0x0001           ; Others execute

PERMISSIONS          = EXT2_S_IRUSR or EXT2_S_IWUSR \
                       or EXT2_S_IRGRP or EXT2_S_IWGRP \
                       or EXT2_S_IROTH or EXT2_S_IWOTH

; File type defining values in directory entry.
EXT2_FT_REG_FILE     = 1                ; Regular file.
EXT2_FT_DIR          = 2                ; Directory.

; Flags used by KolibriOS.
FS_FT_HIDDEN         = 2
FS_FT_DIR            = 0x10             ; Directory.

; ext2 partition flags.
EXT2_RO              = 0x01

FS_FT_ASCII          = 0                ; Name in ASCII.
FS_FT_UNICODE        = 1                ; Name in Unicode.

EXT2_FEATURE_INCOMPAT_FILETYPE = 0x0002 ; Have file type in directory entry.
EXT4_FEATURE_INCOMPAT_EXTENTS  = 0x0040 ; Extents.
EXT4_FEATURE_INCOMPAT_FLEX_BG  = 0x0200 ; Flexible block groups.

EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER = 0x0001 ; Sparse Superblock
EXT2_FEATURE_RO_COMPAT_LARGE_FILE   = 0x0002 ; Large file support (64-bit file size)

; Implemented ext[2,3,4] features.
EXT4_FEATURE_INCOMPAT_SUPP = EXT2_FEATURE_INCOMPAT_FILETYPE \
                             or EXT4_FEATURE_INCOMPAT_EXTENTS \
                             or EXT4_FEATURE_INCOMPAT_FLEX_BG

; Implemented features which otherwise require "read-only" mount.
EXT2_FEATURE_RO_COMPAT_SUPP = EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER \
                              or EXT2_FEATURE_RO_COMPAT_LARGE_FILE

; ext4 features not support for write.
EXT4_FEATURE_INCOMPAT_W_NOT_SUPP = EXT4_FEATURE_INCOMPAT_EXTENTS \
                                   or EXT4_FEATURE_INCOMPAT_FLEX_BG

; Flags specified in i_flags.
EXT2_EXTENTS_FL      = 0x00080000       ; Extents.

struct  EXT2_INODE_STRUC
        i_mode          dw ?
        i_uid           dw ?
        i_size          dd ?
        i_atime         dd ?
        i_ctime         dd ?
        i_mtime         dd ?
        i_dtime         dd ?
        i_gid           dw ?
        i_links_count   dw ?
        i_blocks        dd ?
        i_flags         dd ?
        i_osd1          dd ?
        i_block         rd 15
        i_generation    dd ?
        i_file_acl      dd ?
        i_dir_acl       dd ?
        i_faddr         dd ?
        i_osd2          dd ?        ; 12 bytes.
ends

struct  EXT2_DIR_STRUC
        inode           dd ?
        rec_len         dw ?
        name_len        db ?
        file_type       db ?
        name            db ?         ; 255 (max) bytes.
ends

struct  EXT2_BLOCK_GROUP_DESC
        block_bitmap            dd ?         ; +0
        inode_bitmap            dd ?         ; +4
        inode_table             dd ?         ; +8
        free_blocks_count       dw ?         ; +12
        free_inodes_count       dw ?         ; +14
        used_dirs_count         dw ?         ; +16
        pad                     dw ?         ; +18
        reserved                rb 12        ; +20
ends

struct  EXT2_SB_STRUC
        inodes_count            dd ?         ; +0
        blocks_count            dd ?         ; +4
        r_block_count           dd ?         ; +8
        free_block_count        dd ?         ; +12
        free_inodes_count       dd ?         ; +16
        first_data_block        dd ?         ; +20
        log_block_size          dd ?         ; +24
        log_frag_size           dd ?         ; +28
        blocks_per_group        dd ?         ; +32
        frags_per_group         dd ?         ; +36
        inodes_per_group        dd ?         ; +40
        mtime                   dd ?         ; +44
        wtime                   dd ?         ; +48
        mnt_count               dw ?         ; +52
        max_mnt_count           dw ?         ; +54
        magic                   dw ?         ; +56
        state                   dw ?         ; +58
        errors                  dw ?         ; +60
        minor_rev_level         dw ?         ; +62
        lastcheck               dd ?         ; +64
        check_intervals         dd ?         ; +68
        creator_os              dd ?         ; +72
        rev_level               dd ?         ; +76
        def_resuid              dw ?         ; +80
        def_resgid              dw ?         ; +82
        first_ino               dd ?         ; +84
        inode_size              dw ?         ; +88
        block_group_nr          dw ?         ; +90
        feature_compat          dd ?         ; +92
        feature_incompat        dd ?         ; +96
        feature_ro_compat       dd ?         ; +100
        uuid                    rb 16        ; +104
        volume_name             rb 16        ; +120
        last_mounted            rb 64        ; +136
        algo_bitmap             dd ?         ; +200
        prealloc_blocks         db ?         ; +204
        preallock_dir_blocks    db ?         ; +205
        reserved_gdt_blocks     dw ?         ; +206
        journal_uuid            rb 16        ; +208
        journal_inum            dd ?         ; +224
        journal_dev             dd ?         ; +228
        last_orphan             dd ?         ; +232
        hash_seed               rd 4         ; +236
        def_hash_version        db ?         ; +252
        reserved                rb 3         ; +253 (reserved)
        default_mount_options   dd ?         ; +256
        first_meta_bg           dd ?         ; +260
        mkfs_time               dd ?         ; +264
        jnl_blocks              rd 17        ; +268
        blocks_count_hi         dd ?         ; +336
        r_blocks_count_hi       dd ?         ; +340
        free_blocks_count_hi    dd ?         ; +344
        min_extra_isize         dw ?         ; +348
        want_extra_isize        dw ?         ; +350
        flags                   dd ?         ; +352
        raid_stride             dw ?         ; +356
        mmp_interval            dw ?         ; +358
        mmp_block               dq ?         ; +360
        raid_stripe_width       dd ?         ; +368
        log_groups_per_flex     db ?         ; +372
ends

; Header block extents.
struct EXT4_EXTENT_HEADER
        eh_magic        dw ?    ; Magic value of 0xF30A, for ext4.
        eh_entries      dw ?    ; Number of blocks covered by the extent.
        eh_max          dw ?    ; Capacity of entries.
        eh_depth        dw ?    ; Tree depth (if 0, extents in the array are not extent indexes)
        eh_generation   dd ?    ; ???
ends

; Extent.
struct EXT4_EXTENT
        ee_block        dd ?    ; First logical block extent covers.
        ee_len          dw ?    ; Number of blocks covered by extent.
        ee_start_hi     dw ?    ; Upper 16 bits of 48-bit address (unused in KOS)
        ee_start_lo     dd ?    ; Lower 32 bits of 48-bit address.
ends

; Index on-disk structure; pointer to block of extents/indexes.
struct EXT4_EXTENT_IDX
        ei_block        dd ?    ; Covers logical blocks from here.
        ei_leaf_lo      dd ?    ; Lower 32-bits of pointer to the physical block of the next level. 
        ei_leaf_hi      dw ?    ; Higher 16-bits (unused in KOS).
        ei_unused       dw ?    ; Reserved.
ends