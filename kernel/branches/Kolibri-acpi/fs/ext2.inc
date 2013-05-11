;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;   02.02.2010  turbanoff  - support 70.5                      ;;
;;   23.01.2010  turbanoff  - support 70.0 70.1                 ;;
;;                                                              ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

EXT2_BAD_INO         = 1
EXT2_ROOT_INO        = 2
EXT2_ACL_IDX_INO     = 3
EXT2_ACL_DATA_INO    = 4
EXT2_BOOT_LOADER_INO = 5
EXT2_UNDEL_DIR_INO   = 6

;флаги, указываемый в inode файла
EXT2_S_IFREG         = 0x8000
EXT2_S_IFDIR         = 0x4000
EXT2_S_IFMT          = 0xF000 ;маска для типа файла

;флаги, указываемые в linked list родительской папки
EXT2_FT_REG_FILE     = 1     ;это файл, запись в родительском каталоге
EXT2_FT_DIR          = 2     ;это папка

;флаги используемые KolibriOS
FS_FT_HIDDEN         = 2
FS_FT_DIR            = 0x10  ;это папка
FS_FT_ASCII          = 0     ;имя в ascii
FS_FT_UNICODE        = 1     ;имя в unicode

EXT2_FEATURE_INCOMPAT_FILETYPE = 0x0002 ;тип файла должен указываться в директории
EXT4_FEATURE_INCOMPAT_EXTENTS  = 0x0040 ;экстенты
EXT4_FEATURE_INCOMPAT_FLEX_BG  = 0x0200 ;гибкие группы блоков
;реализованные ext[234] features
EXT4_FEATURE_INCOMPAT_SUPP = EXT2_FEATURE_INCOMPAT_FILETYPE \
                          or EXT4_FEATURE_INCOMPAT_EXTENTS \
                          or EXT4_FEATURE_INCOMPAT_FLEX_BG


;флаги, указываемые для inode в i_flags
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

struct EXT4_EXTENT_HEADER       ;заголовок блока экстентов/индексов
        eh_magic        dw ?    ;в текущей реализации ext4 должно быть 0xF30A
        eh_entries      dw ?    ;количество экстентов/индексов в блоке
        eh_max          dw ?    ;max количество (используется при записи)
        eh_depth        dw ?    ;глубина дерева (0, если это блок экстентов)
        eh_generation   dd ?    ;???
ends

struct EXT4_EXTENT              ;экстент
        ee_block        dd ?    ;номер ext4 блока
        ee_len          dw ?    ;длина экстента
        ee_start_hi     dw ?    ;старшие 16 бит 48-битного адреса (пока не используются в KOS)
        ee_start_lo     dd ?    ;младшие 32 бита 48-битного адреса
ends

struct EXT4_EXTENT_IDX          ;индес - указатель на блок с экстентами/индексами
        ei_block        dd ?    ;номер ext4 блока
        ei_leaf_lo      dd ?    ;младшие 32 бит 48-битного адреса
        ei_leaf_hi      dw ?    ;старшие 16 бит 48-битного адреса (пока не используются в KOS)
        ei_unused       dw ?    ;зарезервировано
ends

ext2_test_superblock:
        cmp     [fs_type], 0x83
        jne     .no

        mov     eax, [PARTITION_START]
        add     eax, 2                  ;superblock start at 1024b
        call    hd_read

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
        jnz     .no

    ; OK, this is correct EXT2 superblock
        clc
        ret
   .no:
    ; No, this superblock isn't EXT2
        stc
        ret

ext2_setup:
        mov     [fs_type], 2

        push    512
        call    kernel_alloc                            ; mem for superblock
        mov     esi, ebx
        mov     edi, eax
        mov     ecx, 512/4
        rep movsd                                       ; copy sb to reserved mem
        mov     ebx, eax
        mov     [ext2_data.sb], eax

        mov     eax, [ebx + EXT2_SB_STRUC.blocks_count]
        sub     eax, [ebx + EXT2_SB_STRUC.first_data_block]
        dec     eax
        xor     edx, edx
        div     [ebx + EXT2_SB_STRUC.blocks_per_group]
        inc     eax
        mov     [ext2_data.groups_count], eax

        mov     ecx, [ebx + EXT2_SB_STRUC.log_block_size]
        inc     ecx
        mov     [ext2_data.log_block_size], ecx    ; 1, 2, 3, 4   equ 1kb, 2kb, 4kb, 8kb

        mov     eax, 1
        shl     eax, cl
        mov     [ext2_data.count_block_in_block], eax

        shl     eax, 7
        mov     [ext2_data.count_pointer_in_block], eax
        mov     edx, eax                                ; потом еще квадрат найдем

        shl     eax, 2
        mov     [ext2_data.block_size], eax

        push    eax eax                                 ; 2 kernel_alloc

        mov     eax, edx
        mul     edx
        mov     [ext2_data.count_pointer_in_block_square], eax

        call    kernel_alloc
        mov     [ext2_data.ext2_save_block], eax        ; and for temp block
        call    kernel_alloc
        mov     [ext2_data.ext2_temp_block], eax        ; and for get_inode proc

        movzx   ebp, word [ebx + EXT2_SB_STRUC.inode_size]
        mov     ecx, [ebx + EXT2_SB_STRUC.blocks_per_group]

        mov     [ext2_data.inode_size], ebp
        mov     [ext2_data.blocks_per_group], ecx

        push    ebp ebp ebp                             ;3 kernel_alloc
        call    kernel_alloc
        mov     [ext2_data.ext2_save_inode], eax
        call    kernel_alloc
        mov     [ext2_data.ext2_temp_inode], eax
        call    kernel_alloc
        mov     [ext2_data.root_inode], eax

        mov     ebx, eax
        mov     eax, EXT2_ROOT_INO
        call    ext2_get_inode                          ; read root inode

        jmp     return_from_part_set

;==================================================================
;read ext2 block form FS to memory
;in:  eax = i_block (address of block in ext2 terms)
;     ebx = pointer to return memory
;out: eax - error code (0 = no_error)
ext2_get_block:
        push    ebx ecx
        mov     ecx, [ext2_data.log_block_size]
        shl     eax, cl
        add     eax, [PARTITION_START]
        mov     ecx, [ext2_data.count_block_in_block]
    @@:
        call    hd_read
        cmp     [hd_error], 0
        jnz     .fail
        inc     eax
        add     ebx, 512
        loop    @B
        xor     eax, eax
    @@:
        pop     ecx ebx
        ret
    .fail:
        mov     eax, ERROR_DEVICE
        jmp     @B
        

;===================================================================
;получает номер блока из extent inode
;in:  ecx = номер блока по порядку
;     ebp = адрес extent header`а
;out: ecx - адрес очередного блока в случае успеха
;     eax - номер ошибки (если равно 0, то ошибки нет)
ext4_block_recursive_search:
        cmp     word [ebp + EXT4_EXTENT_HEADER.eh_magic], 0xF30A ;EXT4_EXT_MAGIC
        jne     .fail
        
        movzx   ebx, [ebp + EXT4_EXTENT_HEADER.eh_entries]
        add     ebp, sizeof.EXT4_EXTENT_HEADER
        cmp     word [ebp - sizeof.EXT4_EXTENT_HEADER + EXT4_EXTENT_HEADER.eh_depth], 0
        je      .leaf_block     ;листовой ли это блок?
        
        ;не листовой блок, а индексный ; eax - ext4_extent_idx
        test    ebx, ebx
        jz      .fail               ;пустой индексный блок -> ошибка

        ;цикл по индексам экстентов
    @@:
        cmp     ebx, 1              ;у индексов не хранится длина, 
        je      .end_search_index   ;поэтому, если остался последний - то это нужный
        
        cmp     ecx, [ebp + EXT4_EXTENT_IDX.ei_block]
        jb      .fail
        
        cmp     ecx, [ebp + sizeof.EXT4_EXTENT_IDX + EXT4_EXTENT_IDX.ei_block] ;блок слeдующего индекса
        jb      .end_search_index ;следующий дальше - значит текущий, то что нам нужен
        
        add     ebp, sizeof.EXT4_EXTENT_IDX
        dec     ebx
        jmp     @B
        
    .end_search_index:
        ;ebp указывает на нужный extent_idx, считываем следующий блок
        mov     ebx, [ext2_data.ext2_temp_block]
        mov     eax, [ebp + EXT4_EXTENT_IDX.ei_leaf_lo]
        call    ext2_get_block
        test    eax, eax
        jnz     .fail
        mov     ebp, ebx
        jmp     ext4_block_recursive_search ;рекурсивно прыгаем в начало
        
    .leaf_block:    ;листовой блок ebp - ext4_extent
        ;цикл по экстентам
    @@: 
        test    ebx, ebx
        jz      .fail       ;ни один узел не подошел - ошибка

        mov     edx, [ebp + EXT4_EXTENT.ee_block]
        cmp     ecx, edx
        jb      .fail       ;если меньше, значит он был в предыдущих блоках -> ошибка

        movzx   edi, [ebp + EXT4_EXTENT.ee_len]
        add     edx, edi
        cmp     ecx, edx
        jb      .end_search_extent     ;нашли нужный блок
        
        add     ebp, sizeof.EXT4_EXTENT
        dec     ebx
        jmp     @B
        
    .end_search_extent:
        mov     edx, [ebp + EXT4_EXTENT.ee_start_lo]
        sub     ecx, [ebp + EXT4_EXTENT.ee_block] ;разница в ext4 блоках
        add     ecx, edx
        xor     eax, eax
        ret

    .fail:
        mov     eax, ERROR_FS_FAIL
        ret

;===================================================================
;получает адрес ext2 блока из inode с определнным номером
;in:  ecx = номер блока в inode (0..)
;     ebp = адрес inode
;out: ecx = адрес очередного блока
;     eax - error code
ext2_get_inode_block:
        test    [ebp + EXT2_INODE_STRUC.i_flags], EXT2_EXTENTS_FL
        jz      @F

        pushad
        add     ebp, EXT2_INODE_STRUC.i_block             ;ebp - extent_header
        call    ext4_block_recursive_search
        mov     PUSHAD_ECX, ecx
        mov     PUSHAD_EAX, eax
        popad
        ret

    @@:
        cmp     ecx, 12                                         ; 0..11 - direct block address
        jb      .get_direct_block

        sub     ecx, 12
        cmp     ecx, [ext2_data.count_pointer_in_block]         ; 12.. - indirect blocks
        jb      .get_indirect_block

        sub     ecx, [ext2_data.count_pointer_in_block]
        cmp     ecx, [ext2_data.count_pointer_in_block_square]
        jb      .get_double_indirect_block

        sub     ecx, [ext2_data.count_pointer_in_block_square]
    ;triple indirect block
        push    edx ebx

        mov     eax, [ebx + EXT2_INODE_STRUC.i_block + 14*4]
        mov     ebx, [ext2_data.ext2_temp_block]
        call    ext2_get_block
        test    eax, eax
        jnz     .fail

        xor     edx, edx
        mov     eax, ecx
        div     [ext2_data.count_pointer_in_block_square]

    ;eax - номер в полученном блоке   edx - номер дальше
        mov     eax, [ebx + eax*4]
        call    ext2_get_block
        test    eax, eax
        jnz     .fail

        mov     eax, edx
        jmp     @F

    .get_double_indirect_block:
        push    edx ebx

        mov     eax, [ebp + EXT2_INODE_STRUC.i_block + 13*4]
        mov     ebx, [ext2_data.ext2_temp_block]
        call    ext2_get_block
        test    eax, eax
        jnz     .fail

        mov     eax, ecx
    @@:
        xor     edx, edx
        div     [ext2_data.count_pointer_in_block]

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
        mov     eax, [ebp + EXT2_INODE_STRUC.i_block + 12*4]
        mov     ebx, [ext2_data.ext2_temp_block]
        call    ext2_get_block
        test    eax, eax
        jnz     @F                      ;если не было ошибки

        mov     ecx, [ebx + ecx*4]    ;заносим результат
    @@:
        pop     ebx
        ret

    .get_direct_block:
        mov     ecx, [ebp + EXT2_INODE_STRUC.i_block + ecx*4]
        xor     eax, eax
        ret

;===================================================================
;get content inode by num
;in:  eax = inode_num
;     ebx = address of inode content
;out: eax - error code
ext2_get_inode:
        pushad
        mov     edi, ebx   ;сохраним адрес inode
        dec     eax
        xor     edx, edx
        
        mov     ecx, [ext2_data.sb]
        div     [ecx + EXT2_SB_STRUC.inodes_per_group]

        push    edx                             ;locale num in group

        mov     edx, 32
        mul     edx                             ; address block_group in global_desc_table

        ; в eax - смещение группы с inode-ом относительно начала глобальной дескрипторной таблицы
        ; найдем блок в котором он находится
        div     [ext2_data.block_size]
        add     eax, [ecx + EXT2_SB_STRUC.first_data_block]
        inc     eax
        mov     ebx, [ext2_data.ext2_temp_block]
        call    ext2_get_block
        test    eax, eax
        jnz     .fail     

        add     ebx, edx                        ; локальный номер в блоке
        mov     eax, [ebx + EXT2_BLOCK_GROUP_DESC.inode_table]; номер блока - в терминах ext2

        mov     ecx, [ext2_data.log_block_size]
        shl     eax, cl
        add     eax, [PARTITION_START]        ; а старт раздела - в терминах hdd (512)

        ;eax - указывает на таблицу inode-ов на hdd
        mov     esi, eax                        ;сохраним его пока в esi

        ; прибавим локальный адрес inode-а
        pop     eax                             ; index
        mov     ecx, [ext2_data.inode_size]
        mul     ecx                             ; (index * inode_size)
        mov     ebp, 512
        div     ebp                             ;поделим на размер блока

        add     eax, esi                        ;нашли адрес блока для чтения
        mov     ebx, [ext2_data.ext2_temp_block]
        call    hd_read
        cmp     [hd_error], 0
        jnz     .fail

        mov     esi, edx                        ;добавим "остаток"
        add     esi, ebx                        ;к адресу
        rep movsb                               ;копируем inode
        xor     eax, eax
    .fail:
        mov     PUSHAD_EAX, eax
        popad
        ret

;----------------------------------------------------------------
;
;  ext2_HdReadFolder - read disk folder
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
ext2_HdReadFolder:
        cmp     byte [esi], 0
        jz      .root_folder

        push    ebx ecx edx
        call    ext2_find_lfn               ;вернет в ebp адрес inode
        pop     edx ecx ebx
        test    eax, eax
        jnz     .error_ret
        test    [ebp + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR
        jz      .error_not_found
        jmp     @F

    .root_folder:
        mov     ebp, [ext2_data.root_inode]
        test    [ebp + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR
        jz      .error_root
        ;придется копировать inode
        push    ecx
        mov     esi, ebp
        mov     edi, [ext2_data.ext2_save_inode]
        mov     ecx, [ext2_data.inode_size]
        shr     ecx, 2
        mov     ebp, edi
        rep movsd
        pop     ecx
    @@:
        cmp     [ebp + EXT2_INODE_STRUC.i_size], 0 ;папка пуста
        je      .error_empty_dir
        
        push    edx      ;адрес результата              [edi + 28]
        push    0        ;конец очередного блока папки   [edi + 24]
        push    ecx      ;сколько файлов нужно прочитать [edi + 20]
        push    dword [ebx]   ;первый "нужный" файл           [edi + 16]
        push    dword [ebx + 4];флаги                        [edi + 12]
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
        mov     ebx, [ext2_data.ext2_save_block]
        call    ext2_get_block                  ; и считываем блок с hdd
        test    eax, eax
        jnz     .error_get_block

        mov     esi, ebx                         ; esi = current dir record
        add     ebx, [ext2_data.block_size]
        mov     [edi + 24], ebx               ; запомним конец очередного блока

        mov     ecx, [edi + 16]                        ; ecx = first wanted (flags ommited)

    .find_wanted_start:
        jecxz   .find_wanted_end
    .find_wanted_cycle:
        cmp     [esi + EXT2_DIR_STRUC.inode], 0     ; if (inode = 0) => not used
        jz      @F
        inc     dword [edi + 4]                        ; EXT2_files_in_folder
        dec     ecx
    @@:
        movzx   ebx, [esi + EXT2_DIR_STRUC.rec_len]

        cmp     ebx, 12                                 ; минимальная длина записи
        jb      .error_bad_len
        test    ebx, 0x3                                ; длина записи должна делиться на 4
        jnz     .error_bad_len

        sub     [ebp + EXT2_INODE_STRUC.i_size], ebx    ;вычитаем напрямую из структуры inode
        add     esi, ebx                                ; к следующей записи
        cmp     esi, [edi + 24]                        ; сравниваем с концом блока
        jb      .find_wanted_start

        push    .find_wanted_start
   .end_block:                                              ;вылетели из цикла
        cmp     [ebp + EXT2_INODE_STRUC.i_size], 0
        jle     .end_dir

        inc     dword [edi]                                 ;получаем новый блок
        push    ecx
        mov     ecx, [edi]
        call    ext2_get_inode_block
        test    eax, eax
        jnz     .error_get_block

        mov     eax, ecx
        mov     ebx, [ext2_data.ext2_save_block]
        call    ext2_get_block
        test    eax, eax
        jnz     .error_get_block

        pop     ecx
        mov     esi, ebx
        add     ebx, [ext2_data.block_size]
        mov     [edi + 24], ebx                            ;запомним конец блока
        ret                                                 ; опять в цикл

    .wanted_end:
        loop    .find_wanted_cycle                          ; ecx 0 => -1 нужно посчитать сколько файлов

    ;дошли до первого "нужного" файла
    .find_wanted_end:
        mov     ecx, [edi + 20]
    .wanted_start:                                          ; ищем first_wanted+count
        jecxz   .wanted_end
        cmp     [esi + EXT2_DIR_STRUC.inode], 0         ; if (inode = 0) => not used
        jz      .empty_rec
        inc     dword [edi + 8]
        inc     dword [edi + 4]

        push    edi ecx
        mov     edi, edx                            ;обнуляем место под очереное имя файла/папки
        xor     eax, eax
        mov     ecx, 40 / 4
        rep stosd
        pop     ecx edi

        push    esi edi edx
        mov     eax, [esi + EXT2_DIR_STRUC.inode]             ;получим дочерний inode
        mov     ebx, [ext2_data.ext2_temp_inode]
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
        push    ecx  ;edi и esi уже сохранены в стеке
        movzx   ecx, [esi + EXT2_DIR_STRUC.name_len]
        lea     edi, [edx + 40]
        lea     esi, [esi + EXT2_DIR_STRUC.name]
        call    utf8_to_cp866
        and     byte [edi], 0
        pop     ecx edi esi

        cmp     byte [edx + 40], '.'                    ; в linux файл, начинающийся с точки - скрытый
        jne     @F
        or      dword [edx], FS_FT_HIDDEN
    @@:

        add     edx, 40 + 264                           ; go to next record
        dec     ecx                                     ; если запись пустая ecx не надо уменьшать
    .empty_rec:
        movzx   ebx, [esi + EXT2_DIR_STRUC.rec_len]
        cmp     ebx, 12                                 ; минимальная длина записи
        jb      .error_bad_len
        test    ebx, 0x3                                ; длина записи должна делиться на 4
        jnz     .error_bad_len

        sub     [ebp + EXT2_INODE_STRUC.i_size], ebx    ;вычитаем напрямую из структуры inode
        add     esi, ebx
        cmp     esi, [edi + 24]                         ;дошли ли до конца блока?
        jb      .wanted_start

        push    .wanted_start                           ; дошли 
        jmp     .end_block

    .end_dir:   ;конец папки, когда еще не дошли до нужного файла
        mov     edx, [edi + 28] ;адрес структуры результата
        mov     ebx, [edi + 8] ;EXT2_read_in_folder
        mov     ecx, [edi + 4] ;EXT2_files_in_folder
        mov     dword [edx], 1    ;version
        mov     [edx + 4], ebx
        mov     [edx + 8], ecx
        
        lea     esp, [edi + 32]
        
        xor     eax, eax            ;зарезервировано: нули в текущей реализации
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
        ret
        
    .error_empty_dir:               ;inode папки без блоков
    .error_root:                    ;root - не папка
        mov     eax, ERROR_FS_FAIL
        jmp     .error_ret

    .error_not_found:               ;файл не найден
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
;
;  ext2_HdRead - read hard disk
;
;  esi  points to filename
;  ebx  pointer to 64-bit number = first wanted byte, 0+
;       may be ebx=0 - start from first byte
;  ecx  number of bytes to read, 0+
;  edx  mem location to return data
;
;  ret ebx = bytes read or 0xffffffff file not found
;      eax = 0 ok read or other = errormsg

;--------------------------------------------------------------
ext2_HdRead:
        cmp     byte [esi], 0
        jnz     @F

    .this_is_nofile:
        or      ebx, -1
        mov     eax, ERROR_ACCESS_DENIED
        ret

    @@:
        push    ecx ebx edx
        call    ext2_find_lfn
        pop     edx ebx ecx
        test    eax, eax
        jz      @F

        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret

    @@:
        mov     ax, [ebp + EXT2_INODE_STRUC.i_mode]
        and     ax, EXT2_S_IFMT     ;оставляем только тип inode в ax
        cmp     ax, EXT2_S_IFREG
        jne     .this_is_nofile

        mov     edi, edx            ; edi = pointer to return mem

        test    ebx, ebx
        jz      @F
        mov     esi, ebx            ; esi = pointer to first_wanted
        mov     ebx, [esi+4]
        mov     eax, [esi]          ; ebx : eax - стартовый номер байта

        ;///// сравним хватит ли нам файла или нет
        cmp     [ebp + EXT2_INODE_STRUC.i_dir_acl], ebx
        ja      .size_great
        jb      .size_less

        cmp     [ebp + EXT2_INODE_STRUC.i_size], eax
        ja      .size_great

    .size_less:
        xor     ebx, ebx
        mov     eax, ERROR_END_OF_FILE
        ret
        
    @@:
        xor     ebx, ebx
        xor     eax, eax
    .size_great:
        add     eax, ecx                  ;add to first_wanted кол-во байт для чтения
        adc     ebx, 0

        cmp     [ebp + EXT2_INODE_STRUC.i_dir_acl], ebx
        ja      .size_great_great
        jb      .size_great_less
        cmp     [ebp + EXT2_INODE_STRUC.i_size], eax
        jae     .size_great_great

    .size_great_less:
        push    1                                ;читаем по границе размера
        mov     ecx, [ebp + EXT2_INODE_STRUC.i_size]
        sub     ecx, [esi]                      ;(размер - старт) = сколько читать
        jmp     @F

    .size_great_great:
        push    0                                ;читаем столько сколько запросили

    @@:
        ;здесь мы точно знаем сколько байт читать - ecx
        ;edi - return memory
        ;esi -> first wanted

        push    ecx                             ;количество считанных байт
        test    esi, esi
        jz      .zero_start

        ;получим кусок из первого блока
        mov     edx, [esi+4]
        mov     eax, [esi]
        div     [ext2_data.block_size]

        push    eax       ;счетчик блоков ложим в стек
        
        push    ecx
        mov     ecx, eax
        call    ext2_get_inode_block
        test    eax, eax
        jnz     .error_at_first_block
        mov     ebx, [ext2_data.ext2_save_block]
        mov     eax, ecx
        call    ext2_get_block
        test    eax, eax
        jnz     .error_at_first_block
        pop     ecx
        add     ebx, edx

        neg     edx
        add     edx, [ext2_data.block_size]     ;block_size - стартовый байт = сколько байт 1-го блока
        cmp     ecx, edx
        jbe     .only_one_block

        mov     eax, ecx
        sub     eax, edx
        mov     ecx, edx

        mov     esi, ebx
        rep movsb                               ;кусок 1-го блока
        jmp     .calc_blocks_count

    .zero_start:
        mov     eax, ecx
        push    0                               ;счетчик блоков ложим в стек
        ;теперь в eax кол-во оставшихся байт для чтения
    .calc_blocks_count:
        mov     ebx, edi                        ;чтение блока прям в ->ebx
        xor     edx, edx
        div     [ext2_data.block_size]       ;кол-во байт в последнем блоке (остаток) в edx
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
        add     ebx, [ext2_data.block_size]

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
        mov     ebx, [ext2_data.ext2_save_block]
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
        ret

;----------------------------------------------------------------
; in:  esi = file path
;      ebx = pointer to dir block
; out: esi - name without parent or not_changed
;      ebx - dir_rec of inode children
ext2_test_block_by_name:
        sub     esp, 256        ;EXT2_filename
        mov     edx, ebx
        add     edx, [ext2_data.block_size]       ;запомним конец блока

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
        sub     ecx, edi          ;кол-во байт в получившейся строке

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
    @@:                           ;не подошло
        pop     esi
    .next_rec:
        movzx   eax, [ebx + EXT2_DIR_STRUC.rec_len]
        add     ebx, eax                                    ;к след. записи
        cmp     ebx, edx                                    ;проверим конец ли
        jb      .start_rec
        add     esp, 256
        ret

    .test_find:
        cmp     byte [esi], 0
        je      .ret                ;нашли конец
        cmp     byte [esi], '/'
        jne     @B
        inc     esi
    .ret:
        add     esp, 256 + 4
        ret

;========================
;Ищет inode по строке пути
;in:  esi = name
;out: eax - error code
;     ebp = inode
;      dl - первый байт из имени файла/папки
ext2_find_lfn:
        mov     ebp, [ext2_data.root_inode]
        cmp     [ebp + EXT2_INODE_STRUC.i_blocks], 0
        je      .error_empty_root
        
    .next_path_part:
        push    [ebp + EXT2_INODE_STRUC.i_blocks]
        xor     ecx, ecx
    .folder_block_cycle:
        push    ecx
        call    ext2_get_inode_block
        test    eax, eax
        jnz     .error_get_inode_block
        
        mov     eax, ecx
        mov     ebx, [ext2_data.ext2_save_block]        ;ebx = cur dir record
        call    ext2_get_block
        test    eax, eax
        jnz     .error_get_block
        
        push    esi
        call    ext2_test_block_by_name
        pop     edi ecx

        cmp     edi, esi                                ;нашли имя?
        je      .next_folder_block                      ;не нашли -> к след. блоку
        
        cmp     byte [esi], 0                           ;дошли до "конца" пути -> возваращаемся
        jz      .get_inode_ret

        cmp     [ebx + EXT2_DIR_STRUC.file_type], EXT2_FT_DIR   ;нашли, но это не папка
        jne     .not_found

        mov     eax, [ebx + EXT2_DIR_STRUC.inode]
        mov     ebx, [ext2_data.ext2_save_inode]                ;все же папка.
        call    ext2_get_inode
        test    eax, eax
        jnz     .error_get_inode
        pop     ecx                                     ;в стеке лежит кол-во блоков
        mov     ebp, ebx
        jmp     .next_path_part
        
    .next_folder_block:
        ;к следующему блоку в текущей папке
        pop     eax                                     ;счетчик блоков
        sub     eax, [ext2_data.count_block_in_block]
        jle     .not_found
        
        push    eax
        inc     ecx
        jmp     .folder_block_cycle

    .not_found:
        mov     eax, ERROR_FILE_NOT_FOUND
        ret

    .get_inode_ret:
        pop     ecx                                     ;в стеке лежит кол-во блоков
        mov     dl, [ebx + EXT2_DIR_STRUC.name]     ;в dl - первый символ ()
        mov     eax, [ebx + EXT2_DIR_STRUC.inode]
        mov     ebx, [ext2_data.ext2_save_inode]
        call    ext2_get_inode
        mov     ebp, ebx
        xor     eax, eax
        ret

    .error_get_inode_block:
    .error_get_block:
        pop     ecx
    .error_get_inode:
        pop     ebx
    .error_empty_root:
        mov     eax, ERROR_FS_FAIL
        ret

;----------------------------------------------------------------
;ext2_HdGetFileInfo - read file info from block device
;
;in: esi  points to filename
;    edx  mem location to return data
;--------------------------------------------------------------
ext2_HdGetFileInfo:
        xchg    bx, bx
        cmp     byte [esi], 0
        jz      .is_root

        push    edx
        call    ext2_find_lfn
        mov     ebx, edx
        pop     edx
        test    eax, eax
        jz      @F
        ret

    .is_root:
        xor     ebx, ebx                        ;root не может быть скрытым
        mov     ebp, [ext2_data.root_inode]
    @@:
        xor     eax, eax
        mov     edi, edx
        mov     ecx, 40/4
        rep stosd                   ; fill zero

        cmp     bl, '.'
        jne     @F
        or      dword [edx], FS_FT_HIDDEN
    @@:

        test    [ebp + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR
        jnz     @F
        mov     eax, [ebp + EXT2_INODE_STRUC.i_size]            ;low size
        mov     ebx, [ebp + EXT2_INODE_STRUC.i_dir_acl]         ;high size
        mov     dword [edx+32], eax
        mov     dword [edx+36], ebx
        xor     dword [edx], FS_FT_DIR
    @@:
        xor     dword [edx], FS_FT_DIR

        lea     edi, [edx + 8]
        mov     eax, [ebp + EXT2_INODE_STRUC.i_ctime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        mov     eax, [ebp + EXT2_INODE_STRUC.i_atime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        mov     eax, [ebp + EXT2_INODE_STRUC.i_mtime]
        xor     edx, edx
        add     eax, 3054539008
        adc     edx, 2
        call    ntfs_datetime_to_bdfe.sec

        xor     eax, eax
        ret

ext2_HdRewrite:
ext2_HdWrite:
ext2_HdSetFileEnd:
ext2_HdSetFileInfo:
ext2_HdDelete:
ext2_HdCreateFolder:
        xor     ebx, ebx
        mov     eax, ERROR_UNSUPPORTED_FS
        ret
