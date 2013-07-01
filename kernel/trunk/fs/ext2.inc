;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;   02.02.2010  turbanoff  - support 70.5                      ;;
;;   23.01.2010  turbanoff  - support 70.0 70.1                 ;;
;;   21.06.2013  yogev_ezra - Translate Russian comments        ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

EXT2_BAD_INO         = 1
EXT2_ROOT_INO        = 2
EXT2_ACL_IDX_INO     = 3
EXT2_ACL_DATA_INO    = 4
EXT2_BOOT_LOADER_INO = 5
EXT2_UNDEL_DIR_INO   = 6

;RUS: флаги, указываемые в inode файла   ;ENG: flags specified in file inode
EXT2_S_IFREG         = 0x8000
EXT2_S_IFDIR         = 0x4000
EXT2_S_IFMT          = 0xF000 ;RUS: маска для типа файла  ;ENG: mask for file type

;RUS: флаги, указываемые в linked list родительской папки
;ENG: flags specified in linked list of parent folder
EXT2_FT_REG_FILE     = 1     ;RUS: это файл, запись в родительском каталоге
                             ;ENG: this is a file, record in parent catalog
EXT2_FT_DIR          = 2     ;RUS: это папка   ;ENG: this is a folder

;RUS: флаги используемые KolibriOS   ;ENG: flags used by KolibriOS
FS_FT_HIDDEN         = 2
FS_FT_DIR            = 0x10  ;RUS: это папка      ;ENG: this is a folder
FS_FT_ASCII          = 0     ;RUS: имя в ascii    ;ENG: name in ASCII
FS_FT_UNICODE        = 1     ;RUS: имя в unicode  ;ENG: name in UNICODE

EXT2_FEATURE_INCOMPAT_FILETYPE = 0x0002 ;RUS: тип файла должен указываться в директории
                                        ;ENG: file type must be specified in the folder
EXT4_FEATURE_INCOMPAT_EXTENTS  = 0x0040 ;RUS: экстенты   ;ENG: extents
EXT4_FEATURE_INCOMPAT_FLEX_BG  = 0x0200 ;RUS: гибкие группы блоков   ;ENG: flexible block groups

;RUS: реализованные ext[234] features   ;ENG: implemented ext[234] features
EXT4_FEATURE_INCOMPAT_SUPP = EXT2_FEATURE_INCOMPAT_FILETYPE \
        or      EXT4_FEATURE_INCOMPAT_EXTENTS \
                or EXT4_FEATURE_INCOMPAT_FLEX_BG

;RUS: флаги, указываемые для inode в i_flags   ;ENG: flags specified for inode in i_flags
EXT2_EXTENTS_FL      = 0x00080000

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
        i_osd2          dd ? ; 1..12
ends

struct  EXT2_DIR_STRUC
        inode           dd ?
        rec_len         dw ?
        name_len        db ?
        file_type       db ?
        name            db ? ; 0..255
ends

struct  EXT2_BLOCK_GROUP_DESC
        block_bitmap            dd ? ;+0
        inode_bitmap            dd ? ;+4
        inode_table             dd ? ;+8
        free_blocks_count       dw ? ;+12
        free_inodes_count       dw ? ;+14
        used_dirs_count         dw ? ;+16
        pad                     dw ? ;+18
        reserved                rb 12;+20
ends

struct  EXT2_SB_STRUC
        inodes_count            dd ? ;+0
        blocks_count            dd ? ;+4
        r_block_count           dd ? ;+8
        free_block_count        dd ? ;+12
        free_inodes_count       dd ? ;+16
        first_data_block        dd ? ;+20
        log_block_size          dd ? ;+24
        log_frag_size           dd ? ;+28
        blocks_per_group        dd ? ;+32
        frags_per_group         dd ? ;+36
        inodes_per_group        dd ? ;+40
        mtime                   dd ? ;+44
        wtime                   dd ? ;+48
        mnt_count               dw ? ;+52
        max_mnt_count           dw ? ;+54
        magic                   dw ? ;+56
        state                   dw ? ;+58
        errors                  dw ? ;+60
        minor_rev_level         dw ? ;+62
        lastcheck               dd ? ;+64
        check_intervals         dd ? ;+68
        creator_os              dd ? ;+72
        rev_level               dd ? ;+76
        def_resuid              dw ? ;+80
        def_resgid              dw ? ;+82
        first_ino               dd ? ;+84
        inode_size              dw ? ;+88
        block_group_nr          dw ? ;+90
        feature_compat          dd ? ;+92
        feature_incompat        dd ? ;+96
        feature_ro_compat       dd ? ;+100
        uuid                    rb 16 ;+104
        volume_name             rb 16 ;+120
        last_mounted            rb 64 ;+136
        algo_bitmap             dd ? ;+200
        prealloc_blocks         db ? ;+204
        preallock_dir_blocks    db ? ;+205
        reserved_gdt_blocks     dw ? ;+206
        journal_uuid            rb 16 ;+208
        journal_inum            dd ? ;+224
        journal_dev             dd ? ;+228
        last_orphan             dd ? ;+232
        hash_seed               rd 4 ;+236
        def_hash_version        db ? ;+252
                                rb 3 ;+253 reserved
        default_mount_options   dd ? ;+256
        first_meta_bg           dd ? ;+260
        mkfs_time               dd ? ;+264
        jnl_blocks              rd 17 ;+268
        blocks_count_hi         dd ? ;+336
        r_blocks_count_hi       dd ? ;+340
        free_blocks_count_hi    dd ? ;+344
        min_extra_isize         dw ? ;+348
        want_extra_isize        dw ? ;+350
        flags                   dd ? ;+352
        raid_stride             dw ? ;+356
        mmp_interval            dw ? ;+358
        mmp_block               dq ? ;+360
        raid_stripe_width       dd ? ;+368
        log_groups_per_flex     db ? ;+372
ends

struct EXT4_EXTENT_HEADER       ;RUS: заголовок блока экстентов/индексов
        eh_magic        dw ?    ;RUS: в текущей реализации ext4 должно быть 0xF30A
        eh_entries      dw ?    ;RUS: количество экстентов/индексов в блоке
        eh_max          dw ?    ;RUS: max количество (используется при записи)
        eh_depth        dw ?    ;RUS: глубина дерева (0, если это блок экстентов)
        eh_generation   dd ?    ;???
ends

struct EXT4_EXTENT              ;RUS: экстент            ;ENG: extent
        ee_block        dd ?    ;RUS: номер ext4 блока   ;ENG: number of ext4 block
        ee_len          dw ?    ;RUS: длина экстента     ;ENG: extent length
        ee_start_hi     dw ?    ;RUS: старшие 16 бит 48-битного адреса (пока не используются в KOS)
                                ;ENG: upper 16 bits of the 48-bit address (not used in KolibriOS yet)
        ee_start_lo     dd ?    ;RUS: младшие 32 бита 48-битного адреса
                                ;ENG: lower 32 bits of the 48-bit address
ends

struct EXT4_EXTENT_IDX          ;RUS: индекс - указатель на блок с экстентами/индексами
                                ;ENG: index - pointer to block of extents/indexes
        ei_block        dd ?    ;RUS: номер ext4 блока   ;ENG: number of ext4 block
        ei_leaf_lo      dd ?    ;RUS: младшие 32 бит 48-битного адреса
                                ;ENG: lower 32 bits of the 48-bit address
        ei_leaf_hi      dw ?    ;RUS: старшие 16 бит 48-битного адреса (пока не используются в KOS)
                                ;ENG: upper 16 bits of the 48-bit address (not used in KolibriOS yet)
        ei_unused       dw ?    ;RUS: зарезервировано   ;ENG: reserved
ends

struct EXTFS PARTITION
    Lock                           MUTEX
    log_block_size                 dd ?
    block_size                     dd ?
    count_block_in_block           dd ?
    blocks_per_group               dd ?
    global_desc_table              dd ?
    root_inode                     dd ?   ; pointer to root inode in memory
    inode_size                     dd ?
    count_pointer_in_block         dd ?   ;  block_size / 4
    count_pointer_in_block_square  dd ?   ; (block_size / 4)**2
    ext2_save_block                dd ?   ;RUS: блок на глобальную 1 процедуру   ;ENG: block for 1 global procedure
    ext2_temp_block                dd ?   ;RUS: блок для мелких процедур         ;ENG: block for small procedures
    ext2_save_inode                dd ?   ;RUS: inode на глобальную процедуру    ;ENG: inode for global procedure
    ext2_temp_inode                dd ?   ;RUS: inode для мелких процедур        ;ENG: inode for small procedures
    groups_count                   dd ?
    superblock                     rd 512/4
ends

iglobal
align 4
ext2_user_functions:
        dd      ext2_free
        dd      (ext2_user_functions_end - ext2_user_functions - 4) / 4
        dd      ext2_Read
        dd      ext2_ReadFolder
        dd      ext2_Rewrite
        dd      ext2_Write
        dd      ext2_SetFileEnd
        dd      ext2_GetFileInfo
        dd      ext2_SetFileInfo
        dd      0
        dd      ext2_Delete
        dd      ext2_CreateFolder
ext2_user_functions_end:
endg

proc ext2_create_partition
        push    ebx

        mov     eax, 2                  ;superblock start at 1024b
        add     ebx, 512        ; get pointer to fs-specific buffer
        call    fs_read32_sys

        cmp     [ebx + EXT2_SB_STRUC.log_block_size], 3  ;0,1,2,3
        ja      .no
        cmp     [ebx + EXT2_SB_STRUC.magic], 0xEF53
        jne     .no
        cmp     [ebx + EXT2_SB_STRUC.state], 1        ;EXT_VALID_FS=1
        jne     .no
        cmp     [ebx + EXT2_SB_STRUC.inodes_per_group], 0
        je      .no
        
        mov     eax, [ebx + EXT2_SB_STRUC.feature_incompat]
        test    eax, EXT2_FEATURE_INCOMPAT_FILETYPE
        jz      .no
        test    eax, not EXT4_FEATURE_INCOMPAT_SUPP
        jz      ext2_setup

   .no:
    ; No, this superblock isn't EXT2
        pop     ebx
        xor     eax, eax
        ret

    ; OK, this is correct EXT2 superblock
ext2_setup:
        movi    eax, sizeof.EXTFS
        call    malloc
        test    eax, eax
        jz      ext2_create_partition.no

        mov     ecx, dword [ebp+PARTITION.FirstSector]
        mov     dword [eax+EXTFS.FirstSector], ecx
        mov     ecx, dword [ebp+PARTITION.FirstSector+4]
        mov     dword [eax+EXTFS.FirstSector+4], ecx
        mov     ecx, dword [ebp+PARTITION.Length]
        mov     dword [eax+EXTFS.Length], ecx
        mov     ecx, dword [ebp+PARTITION.Length+4]
        mov     dword [eax+EXTFS.Length+4], ecx
        mov     ecx, [ebp+PARTITION.Disk]
        mov     [eax+EXTFS.Disk], ecx
        mov     [eax+EXTFS.FSUserFunctions], ext2_user_functions
        push    ebp esi edi
        mov     ebp, eax
        lea     ecx, [eax+EXTFS.Lock]
        call    mutex_init

        mov     esi, ebx
        lea     edi, [ebp+EXTFS.superblock]
        mov     ecx, 512/4
        rep movsd                                       ; copy sb to reserved mem

        mov     eax, [ebx + EXT2_SB_STRUC.blocks_count]
        sub     eax, [ebx + EXT2_SB_STRUC.first_data_block]
        dec     eax
        xor     edx, edx
        div     [ebx + EXT2_SB_STRUC.blocks_per_group]
        inc     eax
        mov     [ebp+EXTFS.groups_count], eax

        mov     ecx, [ebx + EXT2_SB_STRUC.log_block_size]
        inc     ecx
        mov     [ebp+EXTFS.log_block_size], ecx    ; 1, 2, 3, 4   equ 1kb, 2kb, 4kb, 8kb

        mov     eax, 1
        shl     eax, cl
        mov     [ebp+EXTFS.count_block_in_block], eax

        shl     eax, 7
        mov     [ebp+EXTFS.count_pointer_in_block], eax
        mov     edx, eax   ;RUS: потом еще квадрат найдем   ;ENG: we'll find a square later

        shl     eax, 2
        mov     [ebp+EXTFS.block_size], eax

        push    eax eax                                 ; 2 kernel_alloc

        mov     eax, edx
        mul     edx
        mov     [ebp+EXTFS.count_pointer_in_block_square], eax

        call    kernel_alloc
        mov     [ebp+EXTFS.ext2_save_block], eax        ; and for temp block
        call    kernel_alloc
        mov     [ebp+EXTFS.ext2_temp_block], eax        ; and for get_inode proc

        movzx   eax, word [ebx + EXT2_SB_STRUC.inode_size]
        mov     ecx, [ebx + EXT2_SB_STRUC.blocks_per_group]

        mov     [ebp+EXTFS.inode_size], eax
        mov     [ebp+EXTFS.blocks_per_group], ecx

        push    eax eax eax                             ;3 kernel_alloc
        call    kernel_alloc
        mov     [ebp+EXTFS.ext2_save_inode], eax
        call    kernel_alloc
        mov     [ebp+EXTFS.ext2_temp_inode], eax
        call    kernel_alloc
        mov     [ebp+EXTFS.root_inode], eax

        mov     ebx, eax
        mov     eax, EXT2_ROOT_INO
        call    ext2_get_inode                          ; read root inode

        mov     eax, ebp        ; return pointer to EXTFS
        pop     edi esi ebp ebx
        ret
endp

proc ext2_free
        push    ebp
        xchg    ebp, eax
        stdcall kernel_free, [ebp+EXTFS.ext2_save_block]
        stdcall kernel_free, [ebp+EXTFS.ext2_temp_block]
        stdcall kernel_free, [ebp+EXTFS.ext2_save_inode]
        stdcall kernel_free, [ebp+EXTFS.ext2_temp_inode]
        stdcall kernel_free, [ebp+EXTFS.root_inode]
        xchg    ebp, eax
        call    free
        pop     ebp
        ret
endp

proc ext2_lock
        lea     ecx, [ebp+EXTFS.Lock]
        jmp     mutex_lock
endp

proc ext2_unlock
        lea     ecx, [ebp+EXTFS.Lock]
        jmp     mutex_unlock
endp

;==================================================================
;read ext2 block form FS to memory
;in:  eax = i_block (address of block in ext2 terms)
;     ebx = pointer to return memory
;     ebp = pointer to EXTFS
;out: eax - error code (0 = no_error)
ext2_get_block:
        push    ebx ecx
        mov     ecx, [ebp+EXTFS.log_block_size]
        shl     eax, cl
        mov     ecx, eax
        push    [ebp+EXTFS.count_block_in_block]
    @@:
        mov     eax, ecx
        call    fs_read32_sys
        test    eax, eax
        jnz     .fail
        inc     ecx
        add     ebx, 512
        dec     dword [esp]
        jnz     @B
        pop     ecx
        xor     eax, eax
    @@:
        pop     ecx ebx
        ret
    .fail:
        mov     eax, ERROR_DEVICE
        jmp     @B
        

;===================================================================
;RUS: получает номер блока из extent inode                    ;ENG: receives block number from extent inode
;RUS: in:  ecx = номер блока по порядку                       ;ENG: in:  ecx = consecutive block number
;RUS:      esi = адрес extent header`а                        ;ENG:      esi = address of extent header
;RUS:      ebp = указатель на структуру EXTFS                 ;ENG:      ebp = pointer to EXTFS
;RUS: out: ecx - адрес очередного блока в случае успеха       ;ENG: out: ecx - address of next block, if successful
;RUS:      eax - номер ошибки (если равно 0, то ошибки нет)   ;ENG:      eax - error number (0 - no error)
ext4_block_recursive_search:
        cmp     word [esi + EXT4_EXTENT_HEADER.eh_magic], 0xF30A ;EXT4_EXT_MAGIC
        jne     .fail
        
        movzx   ebx, [esi + EXT4_EXTENT_HEADER.eh_entries]
        add     esi, sizeof.EXT4_EXTENT_HEADER
        cmp     word [esi - sizeof.EXT4_EXTENT_HEADER + EXT4_EXTENT_HEADER.eh_depth], 0
        je      .leaf_block     ;листовой ли это блок?
        
        ;не листовой блок, а индексный ; eax - ext4_extent_idx
        test    ebx, ebx
        jz      .fail               ;пустой индексный блок -> ошибка

        ;цикл по индексам экстентов
    @@:
        cmp     ebx, 1              ;у индексов не хранится длина, 
        je      .end_search_index   ;поэтому, если остался последний - то это нужный
        
        cmp     ecx, [esi + EXT4_EXTENT_IDX.ei_block]
        jb      .fail
        
        cmp     ecx, [esi + sizeof.EXT4_EXTENT_IDX + EXT4_EXTENT_IDX.ei_block] ;блок слeдующего индекса
        jb      .end_search_index ;следующий дальше - значит текущий, то что нам нужен
        
        add     esi, sizeof.EXT4_EXTENT_IDX
        dec     ebx
        jmp     @B
        
    .end_search_index:
        ;ebp указывает на нужный extent_idx, считываем следующий блок
        mov     ebx, [ebp+EXTFS.ext2_temp_block]
        mov     eax, [esi + EXT4_EXTENT_IDX.ei_leaf_lo]
        call    ext2_get_block
        test    eax, eax
        jnz     .fail
        mov     esi, ebx
        jmp     ext4_block_recursive_search ;рекурсивно прыгаем в начало
        
    .leaf_block:    ;листовой блок esi - ext4_extent
        ;цикл по экстентам
    @@: 
        test    ebx, ebx
        jz      .fail       ;ни один узел не подошел - ошибка

        mov     edx, [esi + EXT4_EXTENT.ee_block]
        cmp     ecx, edx
        jb      .fail       ;если меньше, значит он был в предыдущих блоках -> ошибка

        movzx   edi, [esi + EXT4_EXTENT.ee_len]
        add     edx, edi
        cmp     ecx, edx
        jb      .end_search_extent     ;нашли нужный блок
        
        add     esi, sizeof.EXT4_EXTENT
        dec     ebx
        jmp     @B
        
    .end_search_extent:
        mov     edx, [esi + EXT4_EXTENT.ee_start_lo]
        sub     ecx, [esi + EXT4_EXTENT.ee_block] ;разница в ext4 блоках
        add     ecx, edx
        xor     eax, eax
        ret

    .fail:
        mov     eax, ERROR_FS_FAIL
        ret

;===================================================================
;получает адрес ext2 блока из inode с определнным номером
;RUS: in:  ecx = номер блока в inode (0..)   ;ENG: in:  ecx = number of block in inode (0..)
;RUS:      esi = адрес inode                 ;ENG:      esi = inode address
;RUS:      ebp = указатель на структуру EXTFS;ENG:      ebp = pointer to EXTFS
;RUS: out: ecx = адрес очередного блока      ;ENG: out: ecx = next block address
;RUS:      eax - error code                  ;ENG:      eax - error code
ext2_get_inode_block:
        test    [esi + EXT2_INODE_STRUC.i_flags], EXT2_EXTENTS_FL
        jz      @F

        pushad
        add     esi, EXT2_INODE_STRUC.i_block             ;esi - extent_header
        call    ext4_block_recursive_search
        mov     PUSHAD_ECX, ecx
        mov     PUSHAD_EAX, eax
        popad
        ret

    @@:
        cmp     ecx, 12                                         ; 0..11 - direct block address
        jb      .get_direct_block

        sub     ecx, 12
        cmp     ecx, [ebp+EXTFS.count_pointer_in_block]         ; 12.. - indirect blocks
        jb      .get_indirect_block

        sub     ecx, [ebp+EXTFS.count_pointer_in_block]
        cmp     ecx, [ebp+EXTFS.count_pointer_in_block_square]
        jb      .get_double_indirect_block

        sub     ecx, [ebp+EXTFS.count_pointer_in_block_square]
    ;triple indirect block
        push    edx ebx

        mov     eax, [esi + EXT2_INODE_STRUC.i_block + 14*4]
        mov     ebx, [ebp+EXTFS.ext2_temp_block]
        call    ext2_get_block
        test    eax, eax
        jnz     .fail

        xor     edx, edx
        mov     eax, ecx
        div     [ebp+EXTFS.count_pointer_in_block_square]

    ;RUS: eax - номер в полученном блоке   edx - номер дальше
    ;ENG: eax - current block number, edx - next block number
        mov     eax, [ebx + eax*4]
        call    ext2_get_block
        test    eax, eax
        jnz     .fail

        mov     eax, edx
        jmp     @F

    .get_double_indirect_block:
        push    edx ebx

        mov     eax, [esi + EXT2_INODE_STRUC.i_block + 13*4]
        mov     ebx, [ebp+EXTFS.ext2_temp_block]
        call    ext2_get_block
        test    eax, eax
        jnz     .fail

        mov     eax, ecx
    @@:
        xor     edx, edx
        div     [ebp+EXTFS.count_pointer_in_block]

        mov     eax, [ebx + eax*4]
        call    ext2_get_block
        test    eax, eax
        jnz     .fail

        mov     ecx, [ebx + edx*4]
    .fail:
        pop     ebx edx
        ret

    .get_indirect_block:
        push    ebx
        mov     eax, [esi + EXT2_INODE_STRUC.i_block + 12*4]
        mov     ebx, [ebp+EXTFS.ext2_temp_block]
        call    ext2_get_block
        test    eax, eax
        jnz     @F          ;RUS: если не было ошибки   ;ENG: if there was no error

        mov     ecx, [ebx + ecx*4]   ;RUS: заносим результат   ;ENG: ???
    @@:
        pop     ebx
        ret

    .get_direct_block:
        mov     ecx, [esi + EXT2_INODE_STRUC.i_block + ecx*4]
        xor     eax, eax
        ret

;===================================================================
;get content inode by num
;in:  eax = inode_num
;     ebx = address of inode content
;     ebp = pointer to EXTFS
;out: eax - error code
ext2_get_inode:
        pushad
        mov     edi, ebx   ;сохраним адрес inode
        dec     eax
        xor     edx, edx
        
        div     [ebp + EXT2_SB_STRUC.inodes_per_group + EXTFS.superblock]

        push    edx                             ;locale num in group

        mov     edx, 32
        mul     edx                             ; address block_group in global_desc_table

        ;RUS: в eax - смещение группы с inode-ом относительно начала глобальной дескрипторной таблицы
        ;RUS: найдем блок в котором он находится
        ;ENG: in eax - inode group offset relative to global descriptor table start
        ;ENG: let's find the block this inode is in
        div     [ebp+EXTFS.block_size]
        add     eax, [ebp + EXT2_SB_STRUC.first_data_block + EXTFS.superblock]
        inc     eax
        mov     ebx, [ebp+EXTFS.ext2_temp_block]
        call    ext2_get_block
        test    eax, eax
        jnz     .fail

        add     ebx, edx   ;RUS: локальный номер в блоке   ;ENG: local number inside block
        mov     eax, [ebx + EXT2_BLOCK_GROUP_DESC.inode_table]   ;RUS: номер блока - в терминах ext2
                                                                 ;ENG: block number - in ext2 terms
        mov     ecx, [ebp+EXTFS.log_block_size]
        shl     eax, cl
        ;RUS: eax - указывает на таблицу inode-ов на hdd   ;ENG: eax - points to inode table on HDD
        mov     esi, eax   ;RUS: сохраним его пока в esi   ;ENG: let's save it in esi for now

        ;RUS: прибавим локальный адрес inode-а   ;ENG: add local address of inode
        pop     eax                             ; index
        mov     ecx, [ebp+EXTFS.inode_size]
        mul     ecx                             ; (index * inode_size)
                ;RUS: поделим на размер блока        ;ENG: divide by block size
        mov     ecx, eax
        and     ecx, 512 - 1
        shrd    eax, edx, 9

        add     eax, esi   ;RUS: нашли адрес блока для чтения   ;ENG: found block address to read
        mov     ebx, [ebp+EXTFS.ext2_temp_block]
        call    fs_read32_sys
        test    eax, eax
        jnz     .fail

        mov     esi, ecx         ;RUS: добавим "остаток"   ;ENG: add the "remainder"
        mov     ecx, [ebp+EXTFS.inode_size]
        add     esi, ebx         ;RUS: к адресу            ;ENG: to the address
        rep movsb                ;RUS: копируем inode      ;ENG: copy inode
        xor     eax, eax
    .fail:
        mov     PUSHAD_EAX, eax
        popad
        ret

;----------------------------------------------------------------
; ext2_ReadFolder - EXT2FS implementation of reading a folder
; in:  ebp = pointer to EXTFS structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
ext2_ReadFolder:
        call    ext2_lock
        cmp     byte [esi], 0
        jz      .root_folder

        push    ebx
        stdcall ext2_find_lfn, [esp+4+4]    ;вернет в esi адрес inode
        pop     ebx
        test    eax, eax
        jnz     .error_ret
        test    [esi + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR
        jz      .error_not_found
        jmp     @F

    .root_folder:
        mov     esi, [ebp+EXTFS.root_inode]
        test    [esi + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR
        jz      .error_root
        ;придется копировать inode
        mov     edi, [ebp+EXTFS.ext2_save_inode]
        mov     ecx, [ebp+EXTFS.inode_size]
        shr     ecx, 2
        push    edi
        rep movsd
        pop     esi
    @@:
        cmp     [esi + EXT2_INODE_STRUC.i_size], 0 ;папка пуста
        je      .error_empty_dir
        
        mov     edx, [ebx + 16]
        push    edx      ;адрес результата              [edi + 28]
        push    0        ;конец очередного блока папки   [edi + 24]
        push    dword [ebx +12];сколько файлов нужно прочитать [edi + 20]
        push    dword [ebx + 4];первый "нужный" файл           [edi + 16]
        push    dword [ebx + 8];флаги                        [edi + 12]
        push    0       ;[EXT2_read_in_folder]     [edi + 8]
        push    0       ;[EXT2_files_in_folder]    [edi + 4]
        push    0       ;номер блока по порядку    [edi]

        mov     edi, edx
        mov     ecx, 32/4
        rep stosd                               ; fill header zero
        
        mov     edi, esp                        ; edi - указатель на локальные переменные
        add     edx, 32                         ; edx = current mem for return

        xor     ecx, ecx                        ; получим номер первого блока
        call    ext2_get_inode_block
        test    eax, eax
        jnz     .error_get_block

        mov     eax, ecx
        mov     ebx, [ebp+EXTFS.ext2_save_block]
        call    ext2_get_block                  ; и считываем блок с hdd
        test    eax, eax
        jnz     .error_get_block

        mov     eax, ebx                         ; ebx = current dir record
        add     eax, [ebp+EXTFS.block_size]
        mov     [edi + 24], eax               ; запомним конец очередного блока

        mov     ecx, [edi + 16]                        ; ecx = first wanted (flags ommited)

    .find_wanted_start:
        jecxz   .find_wanted_end
    .find_wanted_cycle:
        cmp     [ebx + EXT2_DIR_STRUC.inode], 0     ; if (inode = 0) => not used
        jz      @F
        inc     dword [edi + 4]                        ; EXT2_files_in_folder
        dec     ecx
    @@:
        movzx   eax, [ebx + EXT2_DIR_STRUC.rec_len]

        cmp     eax, 12                                 ; минимальная длина записи
        jb      .error_bad_len
        test    eax, 0x3                                ; длина записи должна делиться на 4
        jnz     .error_bad_len

        sub     [esi + EXT2_INODE_STRUC.i_size], eax    ;вычитаем напрямую из структуры inode
        add     ebx, eax                                ; к следующей записи
        cmp     ebx, [edi + 24]                        ; сравниваем с концом блока
        jb      .find_wanted_start

        push    .find_wanted_start
   .end_block:                                              ;вылетели из цикла
        cmp     [esi + EXT2_INODE_STRUC.i_size], 0
        jle     .end_dir

        inc     dword [edi]                                 ;получаем новый блок
        push    ecx
        mov     ecx, [edi]
        call    ext2_get_inode_block
        test    eax, eax
        jnz     .error_get_block

        mov     eax, ecx
        mov     ebx, [ebp+EXTFS.ext2_save_block]
        call    ext2_get_block
        test    eax, eax
        jnz     .error_get_block

        pop     ecx
        mov     eax, ebx
        add     eax, [ebp+EXTFS.block_size]
        mov     [edi + 24], eax                            ;запомним конец блока
        ret                                                 ; опять в цикл

    .wanted_end:
        loop    .find_wanted_cycle                          ; ecx 0 => -1 нужно посчитать сколько файлов

    ;дошли до первого "нужного" файла
    .find_wanted_end:
        mov     ecx, [edi + 20]
    .wanted_start:                                          ; ищем first_wanted+count
        jecxz   .wanted_end
        cmp     [ebx + EXT2_DIR_STRUC.inode], 0         ; if (inode = 0) => not used
        jz      .empty_rec
        inc     dword [edi + 8]
        inc     dword [edi + 4]

        push    edi ecx
        mov     edi, edx                            ;обнуляем место под очереное имя файла/папки
        xor     eax, eax
        mov     ecx, 40 / 4
        rep stosd
        pop     ecx edi

        push    ebx edi edx
        mov     eax, [ebx + EXT2_DIR_STRUC.inode]             ;получим дочерний inode
        mov     ebx, [ebp+EXTFS.ext2_temp_inode]
        call    ext2_get_inode
        test    eax, eax
        jnz     .error_read_subinode

        lea     edi, [edx + 8]

        mov     eax, [ebx + EXT2_INODE_STRUC.i_ctime]           ; переведем время в ntfs формат
        xor     edx, edx
        add     eax, 3054539008                                     ;(369 * 365 + 89) * 24 * 3600
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        mov     eax, [ebx + EXT2_INODE_STRUC.i_atime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        mov     eax, [ebx + EXT2_INODE_STRUC.i_mtime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        pop     edx                                                 ; пока достаем только буфер
        test    [ebx + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR   ; для папки размер
        jnz     @F                                                  ; не возвращаем

        mov     eax, [ebx + EXT2_INODE_STRUC.i_size]            ;low size
        stosd
        mov     eax, [ebx + EXT2_INODE_STRUC.i_dir_acl]         ;high size
        stosd
        xor     dword [edx], FS_FT_DIR                              ;помечаем, что это файл(2 раза xor)
    @@:
        xor     dword [edx], FS_FT_DIR                              ;помечаем, что это файл

        ;теперь скопируем имя, сконвертировав из UTF-8 в CP866
        push    ecx esi ;edi уже сохранен в стеке
        mov     esi, [esp+12]
        movzx   ecx, [esi + EXT2_DIR_STRUC.name_len]
        lea     edi, [edx + 40]
        lea     esi, [esi + EXT2_DIR_STRUC.name]
        call    utf8_to_cp866
        and     byte [edi], 0
        pop     esi ecx edi ebx

        cmp     byte [edx + 40], '.'                    ; в linux файл, начинающийся с точки - скрытый
        jne     @F
        or      dword [edx], FS_FT_HIDDEN
    @@:

        add     edx, 40 + 264                           ; go to next record
        dec     ecx                                     ; если запись пустая ecx не надо уменьшать
    .empty_rec:
        movzx   eax, [ebx + EXT2_DIR_STRUC.rec_len]
        cmp     eax, 12                                 ; минимальная длина записи
        jb      .error_bad_len
        test    eax, 0x3                                ; длина записи должна делиться на 4
        jnz     .error_bad_len

        sub     [esi + EXT2_INODE_STRUC.i_size], eax    ;вычитаем напрямую из структуры inode
        add     ebx, eax
        cmp     ebx, [edi + 24]                         ;дошли ли до конца блока?
        jb      .wanted_start

        push    .wanted_start                           ; дошли 
        jmp     .end_block

    .end_dir:   ;конец папки, когда еще не дошли до нужного файла
        call    ext2_unlock
        mov     edx, [edi + 28] ;адрес структуры результата
        mov     ebx, [edi + 8] ;EXT2_read_in_folder
        mov     ecx, [edi + 4] ;EXT2_files_in_folder
        mov     dword [edx], 1    ;version
        mov     [edx + 4], ebx
        mov     [edx + 8], ecx
        
        lea     esp, [edi + 32]
        
        xor     eax, eax        ;RUS: зарезервировано: нули в текущей реализации
                                ;ENG: reserved: zeros in current implementation
        lea     edi, [edx + 12]
        mov     ecx, 20 / 4
        rep stosd
        ret

    .error_bad_len:
        mov     eax, ERROR_FS_FAIL
    .error_read_subinode:
    .error_get_block:
        lea     esp, [edi + 32]
    .error_ret:
        or      ebx, -1
        push    eax
        call    ext2_unlock
        pop     eax
        ret
        
    .error_empty_dir:   ;RUS: inode папки без блоков   ;ENG: inode of folder without blocks
    .error_root:        ;RUS: root - не папка          ;ENG: root is not a folder
        mov     eax, ERROR_FS_FAIL
        jmp     .error_ret

    .error_not_found:   ;RUS: файл не найден           ;ENG: file not found
        mov     eax, ERROR_FILE_NOT_FOUND
        jmp     .error_ret
            
;============================================
;convert UTF-8 string to ASCII-string (codepage 866)
;in:  ecx = length source
;     esi = source
;     edi = buffer
; destroys: eax,esi,edi
utf8_to_cp866:
        jecxz   .ret
    .start:
        lodsw
        cmp     al, 0x80
        jb      .ascii

        xchg    al, ah              ; big-endian
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
        mov     al, 0xf0    ; Ё capital
        jmp     .doit
    .yo2:
        mov     al, 0xf1    ; ё small
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
    .ret:
        ret

;----------------------------------------------------------------
; ext2_Read - EXT2FS implementation of reading a file
; in:  ebp = pointer to FAT structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
ext2_Read:
        call    ext2_lock
        cmp     byte [esi], 0
        jnz     @F

    .this_is_nofile:
        call    ext2_unlock
        or      ebx, -1
        mov     eax, ERROR_ACCESS_DENIED
        ret

    @@:
        push    ebx
        stdcall ext2_find_lfn, [esp+4+4]
        pop     ebx
        test    eax, eax
        jz      @F

        call    ext2_unlock
        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret

    @@:
        mov     ax, [esi + EXT2_INODE_STRUC.i_mode]
        and     ax, EXT2_S_IFMT     ;оставляем только тип inode в ax
        cmp     ax, EXT2_S_IFREG
        jne     .this_is_nofile

        mov     edi, [ebx+16]       ; edi = pointer to return mem
        mov     ecx, [ebx+12]

        mov     eax, [ebx+4]
        mov     edx, [ebx+8] ;RUS: edx : eax - стартовый номер байта   ;ENG: edx : eax - start byte number

        ;RUS: ///// сравним хватит ли нам файла или нет    ;ENG: ///// check if file is big enough for us
        cmp     [esi + EXT2_INODE_STRUC.i_dir_acl], edx
        ja      .size_great
        jb      .size_less

        cmp     [esi + EXT2_INODE_STRUC.i_size], eax
        ja      .size_great

    .size_less:
        call    ext2_unlock
        xor     ebx, ebx
        mov     eax, ERROR_END_OF_FILE
        ret
        
    .size_great:
        add     eax, ecx    ;RUS: add to first_wanted кол-во байт для чтения
                            ;ENG: add to first_wanted number of bytes to read
        adc     edx, 0

        cmp     [esi + EXT2_INODE_STRUC.i_dir_acl], edx
        ja      .size_great_great
        jb      .size_great_less
        cmp     [esi + EXT2_INODE_STRUC.i_size], eax
        jae     .size_great_great   ; and if it's equal, no matter where we jump

    .size_great_less:
        push    1            ;RUS: читаем по границе размера   ;ENG: reading till the end of file
        mov     ecx, [esi + EXT2_INODE_STRUC.i_size]
        sub     ecx, [ebx+4] ;RUS: (размер - старт) = сколько читать   ;ENG: to read = (size - start)
        jmp     @F

    .size_great_great:
        push    0            ;RUS: читаем столько, сколько запросили   ;ENG: reading as much as requested

    @@:
        ;здесь мы точно знаем сколько байт читать - ecx
        ;edi - return memory
        ;esi -> first wanted

        push    ecx                             ;количество считанных байт

        ;получим кусок из первого блока
        mov     edx, [ebx+8]
        mov     eax, [ebx+4]
        div     [ebp+EXTFS.block_size]

        push    eax       ;счетчик блоков ложим в стек
        
        push    ecx
        mov     ecx, eax
        call    ext2_get_inode_block
        test    eax, eax
        jnz     .error_at_first_block
        mov     ebx, [ebp+EXTFS.ext2_save_block]
        mov     eax, ecx
        call    ext2_get_block
        test    eax, eax
        jnz     .error_at_first_block
        pop     ecx
        add     ebx, edx

        neg     edx
        add     edx, [ebp+EXTFS.block_size]     ;RUS: block_size - стартовый байт = сколько байт 1-го блока
                                                ;ENG: block_size - start byte = number of bytes in 1st block
        cmp     ecx, edx
        jbe     .only_one_block

        mov     eax, ecx
        sub     eax, edx
        mov     ecx, edx

        push    esi
        mov     esi, ebx
        rep movsb                               ;RUS: кусок 1-го блока   ;ENG: part of 1st block
        pop     esi

        ;теперь в eax кол-во оставшихся байт для чтения
    .calc_blocks_count:
        mov     ebx, edi                        ;чтение блока прям в ->ebx
        xor     edx, edx
        div     [ebp+EXTFS.block_size]       ;кол-во байт в последнем блоке (остаток) в edx
        mov     edi, eax                        ;кол-во целых блоков в edi
    @@:
        test    edi, edi
        jz      .finish_block
        inc     dword [esp]
        mov     ecx, [esp]
        call    ext2_get_inode_block
        test    eax, eax
        jnz     .error_at_read_cycle

        mov     eax, ecx                        ;а ebx уже забит нужным значением
        call    ext2_get_block
        test    eax, eax
        jnz     .error_at_read_cycle
        add     ebx, [ebp+EXTFS.block_size]

        dec     edi
        jmp     @B

    .finish_block:          ;в edx - кол-во байт в последнем блоке
        test    edx, edx
        jz      .end_read

        pop     ecx         ;счетчик блоков -> ecx
        inc     ecx
        call    ext2_get_inode_block
        test    eax, eax
        jnz     .error_at_finish_block

        mov     edi, ebx
        mov     eax, ecx
        mov     ebx, [ebp+EXTFS.ext2_save_block]
        call    ext2_get_block
        test    eax, eax
        jnz     .error_at_finish_block

        mov     ecx, edx
        mov     esi, ebx
        rep movsb                               ;кусок last блока
        jmp     @F

    .end_read:
        pop     ecx                             ;счетчик блоков, который хранился в стеке
    @@:
        pop     ebx                             ;количество считанных байт
        call    ext2_unlock
        pop     eax                             ; 1 или 0 - достигли ли конца файла
        test    eax, eax
        jz      @F

        mov     eax, ERROR_END_OF_FILE
        ret
    @@:
        xor     eax, eax
        ret
        
    .only_one_block:
        mov     esi, ebx
        rep movsb                               ;кусок last блока
        jmp     .end_read
        
   .error_at_first_block:
        pop     edx
   .error_at_read_cycle:
        pop     ebx
   .error_at_finish_block:
        pop     ecx edx
        or      ebx, -1
        push    eax
        call    ext2_unlock
        pop     eax
        ret

;----------------------------------------------------------------
; in:  esi = file path
;      ebx = pointer to dir block
;      ebp = pointer to EXTFS structure
; out: esi - name without parent or not_changed
;      ebx - dir_rec of inode children
ext2_test_block_by_name:
        sub     esp, 256        ;EXT2_filename
        mov     edx, ebx
        add     edx, [ebp+EXTFS.block_size]   ;RUS: запомним конец блока   ;ENG: save block end

    .start_rec:
        cmp     [ebx + EXT2_DIR_STRUC.inode], 0
        jz      .next_rec

        mov     edi, esp
        push    esi
        movzx   ecx, [ebx + EXT2_DIR_STRUC.name_len]
        lea     esi, [ebx + EXT2_DIR_STRUC.name]
        call    utf8_to_cp866

        mov     ecx, edi
        lea     edi, [esp + 4]
        sub     ecx, edi   ;RUS: кол-во байт в получившейся строке   ;ENG: number of bytes in resulting string

        mov     esi, [esp]
    @@:
        jecxz   .test_find
        dec     ecx

        lodsb
        call    char_toupper

        mov     ah, [edi]
        inc     edi
        xchg    al, ah
        call    char_toupper
        cmp     al, ah
        je      @B
    @@:                           ;RUS: не подошло   ;ENG: didn't fit
        pop     esi
    .next_rec:
        movzx   eax, [ebx + EXT2_DIR_STRUC.rec_len]
        add     ebx, eax          ;RUS: к след. записи      ;ENG: go to next record
        cmp     ebx, edx          ;RUS: проверим конец ли   ;ENG: check if this is the end
        jb      .start_rec
        add     esp, 256
        ret

    .test_find:
        cmp     byte [esi], 0
        je      .ret              ;RUS: нашли конец         ;ENG: the end reached
        cmp     byte [esi], '/'
        jne     @B
        inc     esi
    .ret:
        add     esp, 256 + 4
        ret

;========================
;Ищет inode по строке пути
;in:  esi+[esp+4] = name
;     ebp = pointer to EXTFS
;out: eax - error code
;     esi = inode
;      dl - первый байт из имени файла/папки
ext2_find_lfn:
        mov     edx, [ebp+EXTFS.root_inode]
        cmp     [edx + EXT2_INODE_STRUC.i_blocks], 0
        je      .error_empty_root
        
    .next_path_part:
        push    [edx + EXT2_INODE_STRUC.i_blocks]
        xor     ecx, ecx
    .folder_block_cycle:
        push    ecx
        xchg    esi, edx
        call    ext2_get_inode_block
        xchg    esi, edx
        test    eax, eax
        jnz     .error_get_inode_block
        
        mov     eax, ecx
        mov     ebx, [ebp+EXTFS.ext2_save_block]        ;ebx = cur dir record
        call    ext2_get_block
        test    eax, eax
        jnz     .error_get_block
        
        push    esi
        push    edx
        call    ext2_test_block_by_name
        pop     edx
        pop     edi ecx

        cmp     edi, esi             ;RUS: нашли имя?   ;ENG: did we find a name?
        je      .next_folder_block   ;RUS: не нашли -> к след. блоку   ;ENG: we didn't -> moving to next block
        
        cmp     byte [esi], 0   ;RUS: дошли до "конца" пути -> возваращаемся
                                ;ENG: reached the "end" of path -> returning
        jnz     @f
        cmp     dword [esp+8], 0
        jz      .get_inode_ret
        mov     esi, [esp+8]
        mov     dword [esp+8], 0
    @@:

        cmp     [ebx + EXT2_DIR_STRUC.file_type], EXT2_FT_DIR   ;RUS: нашли, но это не папка
        jne     .not_found                                      ;ENG: found, but it's not a folder

        mov     eax, [ebx + EXT2_DIR_STRUC.inode]
        mov     ebx, [ebp+EXTFS.ext2_save_inode]   ;RUS: все же папка.   ;ENG: it's a folder afterall
        call    ext2_get_inode
        test    eax, eax
        jnz     .error_get_inode
        pop     ecx   ;RUS: в стеке лежит кол-во блоков   ;ENG: stack top contains number of blocks
        mov     edx, ebx
        jmp     .next_path_part
        
    .next_folder_block:
        ;к следующему блоку в текущей папке
        pop     eax                ;RUS: счетчик блоков   ;ENG: blocks counter
        sub     eax, [ebp+EXTFS.count_block_in_block]
        jle     .not_found
        
        push    eax
        inc     ecx
        jmp     .folder_block_cycle

    .not_found:
        mov     eax, ERROR_FILE_NOT_FOUND
        ret     4

    .get_inode_ret:
        pop     ecx   ;RUS: в стеке лежит кол-во блоков   ;ENG: stack top contains number of blocks
        mov     dl, [ebx + EXT2_DIR_STRUC.name]   ;RUS: в dl - первый символ ()   ;ENG: ???
        mov     eax, [ebx + EXT2_DIR_STRUC.inode]
        mov     ebx, [ebp+EXTFS.ext2_save_inode]
        call    ext2_get_inode
        mov     esi, ebx
        xor     eax, eax
        ret     4

    .error_get_inode_block:
    .error_get_block:
        pop     ecx
    .error_get_inode:
        pop     ebx
    .error_empty_root:
        mov     eax, ERROR_FS_FAIL
        ret     4

;----------------------------------------------------------------
; ext2_GetFileInfo - EXT2 implementation of getting file info
; in:  ebp = pointer to EXTFS structure
; in:  esi+[esp+4] = name
; in:  ebx = pointer to parameters from sysfunc 70
; out: eax, ebx = return values for sysfunc 70
;----------------------------------------------------------------
ext2_GetFileInfo:
        call    ext2_lock
        mov     edx, [ebx+16]
        cmp     byte [esi], 0
        jz      .is_root

        push    edx
        stdcall ext2_find_lfn, [esp+4+4]
        mov     ebx, edx
        pop     edx
        test    eax, eax
        jz      @F
        push    eax
        call    ext2_unlock
        pop     eax
        ret

    .is_root:
        xor     ebx, ebx      ;RUS: root не может быть скрытым   ;ENG: root cannot be hidden
        mov     esi, [ebp+EXTFS.root_inode]
    @@:
        xor     eax, eax
        mov     edi, edx
        mov     ecx, 40/4
        rep stosd                   ; fill zero

        cmp     bl, '.'
        jne     @F
        or      dword [edx], FS_FT_HIDDEN
    @@:

        test    [esi + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR
        jnz     @F
        mov     eax, [esi + EXT2_INODE_STRUC.i_size]            ;low size
        mov     ebx, [esi + EXT2_INODE_STRUC.i_dir_acl]         ;high size
        mov     dword [edx+32], eax
        mov     dword [edx+36], ebx
        xor     dword [edx], FS_FT_DIR
    @@:
        xor     dword [edx], FS_FT_DIR

        lea     edi, [edx + 8]
        mov     eax, [esi + EXT2_INODE_STRUC.i_ctime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        mov     eax, [esi + EXT2_INODE_STRUC.i_atime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        mov     eax, [esi + EXT2_INODE_STRUC.i_mtime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        call    ext2_unlock
        xor     eax, eax
        ret

ext2_Rewrite:
ext2_Write:
ext2_SetFileEnd:
ext2_SetFileInfo:
ext2_Delete:
ext2_CreateFolder:
        xor     ebx, ebx
        mov     eax, ERROR_UNSUPPORTED_FS
        ret
