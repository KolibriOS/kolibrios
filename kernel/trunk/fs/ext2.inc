;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;   02.02.2010  turbanoff  - support 70.5                      ;;
;;   23.01.2010  turbanoff  - support 70.0 70.1                 ;;
;;                                                              ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

EXT2_BAD_INO        = 1
EXT2_ROOT_INO        = 2
EXT2_ACL_IDX_INO    = 3
EXT2_ACL_DATA_INO    = 4
EXT2_BOOT_LOADER_INO= 5
EXT2_UNDEL_DIR_INO    = 6

;type inode
EXT2_S_IFREG        = 0x8000
EXT2_S_IFDIR        = 0x4000
;user inode right's
EXT2_S_IRUSR        = 0x0100
EXT2_S_IWUSR        = 0x0080
EXT2_S_IXUSR        = 0x0040
;group inode right's
EXT2_S_IRGRP        = 0x0020
EXT2_S_IWGRP        = 0x0010
EXT2_S_IXGRP        = 0x0008
;other inode right's
EXT2_S_IROTH        = 0x0004
EXT2_S_IWOTH        = 0x0002
EXT2_S_IXOTH        = 0x0001
EXT2_777_MODE       = EXT2_S_IROTH or EXT2_S_IWOTH or EXT2_S_IXOTH or \
                      EXT2_S_IRGRP or EXT2_S_IWGRP or EXT2_S_IXGRP or \
                      EXT2_S_IRUSR or EXT2_S_IWUSR or EXT2_S_IXUSR

EXT2_FT_REG_FILE        = 1     ;это файл, запись в родительском каталоге
EXT2_FT_DIR             = 2     ;это папка

FS_FT_HIDDEN            = 2
FS_FT_DIR               = 0x10  ;это папка
FS_FT_ASCII             = 0     ;имя в ascii
FS_FT_UNICODE           = 1     ;имя в unicode

EXT2_FEATURE_INCOMPAT_FILETYPE = 0x0002

uglobal
    EXT2_files_in_folder    dd ?   ;всего файлов в папке
    EXT2_read_in_folder     dd ?   ;сколько файлов "считали"
    EXT2_end_block          dd ?   ;конец очередного блока папки
    EXT2_counter_blocks     dd ?
    EXT2_filename           db 256 dup ?
    EXT2_parent_name        db 256 dup ?
    EXT2_name_len           dd ?
endg

struct EXT2_INODE_STRUC
    .i_mode         dw ?
    .i_uid          dw ?
    .i_size         dd ?
    .i_atime        dd ?
    .i_ctime        dd ?
    .i_mtime        dd ?
    .i_dtime        dd ?
    .i_gid          dw ?
    .i_links_count  dw ?
    .i_blocks       dd ?
    .i_flags        dd ?
    .i_osd1         dd ?
    .i_block        dd 15 dup ?
    .i_generation   dd ?
    .i_file_acl     dd ?
    .i_dir_acl      dd ?
    .i_faddr        dd ?
    .i_osd2         dd ? ; 1..12
ends

struct EXT2_DIR_STRUC
    .inode          dd ?
    .rec_len        dw ?
    .name_len       db ?
    .file_type      db ?
    .name           db ? ; 0..255
ends

struct EXT2_BLOCK_GROUP_DESC
    .block_bitmap       dd ?
    .inode_bitmap       dd ?
    .inode_table        dd ?
    .free_blocks_count  dw ?
    .free_inodes_count  dw ?
    .used_dirs_count    dw ?
ends

struct EXT2_SB_STRUC
    .inodes_count       dd ? ;+0
    .blocks_count       dd ? ;+4
    .r_block_count      dd ? ;+8
    .free_block_count   dd ? ;+12
    .free_inodes_count  dd ? ;+16
    .first_data_block   dd ? ;+20
    .log_block_size     dd ? ;+24
    .log_frag_size      dd ? ;+28
    .blocks_per_group   dd ? ;+32
    .frags_per_group    dd ? ;+36
    .inodes_per_group   dd ? ;+40
    .mtime              dd ? ;+44
    .wtime              dd ? ;+48
    .mnt_count          dw ? ;+52
    .max_mnt_count      dw ? ;+54
    .magic              dw ? ;+56
    .state              dw ? ;+58
    .errors             dw ? ;+60
    .minor_rev_level    dw ? ;+62
    .lastcheck          dd ? ;+64
    .check_intervals    dd ? ;+68
    .creator_os         dd ? ;+72
    .rev_level          dd ? ;+76
    .def_resuid         dw ? ;+80
    .def_resgid         dw ? ;+82
    .first_ino          dd ? ;+84
    .inode_size         dw ? ;+88
    .block_group_nr     dw ? ;+90
    .feature_compat     dd ? ;+92
    .feature_incompat   dd ? ;+96
    .feature_ro_compat  dd ? ;+100
    .uuid               db 16 dup ? ;+104
    .volume_name        db 16 dup ? ;+120
    .last_mounted       db 64 dup ? ;+136
    .algo_bitmap        dd ? ;+200
    .prealloc_blocks    db ? ;+204
  .preallock_dir_blocks db ? ;+205
                        dw ? ;+206 alignment
    .journal_uuid       db 16 dup ? ;+208
    .journal_inum       dd ? ;+224
    .journal_dev        dd ? ;+228
    .last_orphan        dd ? ;+232
    .hash_seed          dd 4 dup ?  ;+236
    .def_hash_version   db ? ;+252
                        db 3 dup ?  ;+253 reserved
 .default_mount_options dd ? ;+256
    .first_meta_bg      dd ? ;+260
ends

ext2_test_superblock:
        cmp     [fs_type], 0x83
        jne     .no

        mov     eax, [PARTITION_START]
        add     eax, 2                  ;superblock start at 1024b
        call    hd_read

        cmp     dword [ebx+24], 3       ;s_block_size 0,1,2,3
        ja      .no
        cmp     word [ebx+56], 0xEF53   ;s_magic
        jne     .no
        cmp     word [ebx+58], 1        ;s_state (EXT_VALID_FS=1)
        jne     .no
        mov     eax, [ebx+96]
        test    eax, EXT2_FEATURE_INCOMPAT_FILETYPE
        jz      .no
        test    eax, not EXT2_FEATURE_INCOMPAT_FILETYPE
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

        mov     ecx, [ebx+24]
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

        movzx   ebp, word [ebx+88]
        mov     ecx, [ebx+32]
        mov     edx, [ebx+40]

        mov     [ext2_data.inode_size], ebp
        mov     [ext2_data.blocks_per_group], ecx
        mov     [ext2_data.inodes_per_group], edx

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
;in: eax = i_block
;    ebx = pointer to return memory
ext2_get_block:
        push    eax ebx ecx
        mov     ecx, [ext2_data.log_block_size]
        shl     eax, cl
        add     eax, [PARTITION_START]
        mov     ecx, [ext2_data.count_block_in_block]
    @@:
        call    hd_read
        inc     eax
        add     ebx, 512
        loop    @B
        pop     ecx ebx eax
        ret
;===================================================================
; in:  ecx = номер блока в inode (0..)
;      ebp = адрес inode
; out: ecx = адрес очередного блока
ext2_get_inode_block:
        cmp     ecx, 12                                         ; 0..11 - direct block address
        jb      .get_direct_block

        sub     ecx, 12
        cmp     ecx, [ext2_data.count_pointer_in_block]         ; 12.. - indirect block
        jb      .get_indirect_block

        sub     ecx, [ext2_data.count_pointer_in_block]
        cmp     ecx, [ext2_data.count_pointer_in_block_square]
        jb      .get_double_indirect_block

        sub     ecx, [ext2_data.count_pointer_in_block_square]
    ;.get_triple_indirect_block:
        push    eax edx ebx

        mov     eax, [ebx + EXT2_INODE_STRUC.i_block + 14*4]
        mov     ebx, [ext2_data.ext2_temp_block]
        call    ext2_get_block

        xor     edx, edx
        mov     eax, ecx
        div     [ext2_data.count_pointer_in_block_square]

    ;eax - номер в полученном блоке   edx - номер дальше
        mov     eax, [ebx + eax*4]
        call    ext2_get_block

        mov     eax, edx
        jmp     @F

    .get_double_indirect_block:
        push    eax edx ebx

        mov     eax, [ebp + EXT2_INODE_STRUC.i_block + 13*4]
        mov     ebx, [ext2_data.ext2_temp_block]
        call    ext2_get_block

        mov     eax, ecx
      @@:
        xor     edx, edx
        div     [ext2_data.count_pointer_in_block]

        mov     eax, [ebx + eax*4]
        call    ext2_get_block
        mov     ecx, [ebx + edx*4]

        pop     ebx edx eax
        ret

    .get_indirect_block:
        push    eax ebx
        mov     eax, [ebp + EXT2_INODE_STRUC.i_block + 12*4]
        mov     ebx, [ext2_data.ext2_temp_block]
        call    ext2_get_block

        mov     ecx, [ebx + ecx*4]
        pop     ebx eax
        ret

    .get_direct_block:
        mov     ecx, dword [ebp + EXT2_INODE_STRUC.i_block + ecx*4]
        ret

;===================================================================
;get content inode by num
;in:   eax = inode_num
;      ebx = address of inode content
ext2_get_inode:

        pushad
        mov     edi, ebx   ;сохраним адрес inode
        dec     eax
        xor     edx, edx
        div     [ext2_data.inodes_per_group]

        push    edx                             ;locale num in group

        mov     edx, 32
        mul     edx                             ; address block_group in global_desc_table

        ; в eax - смещение группы с inode-ом относительно начала глобальной дескрипторной таблицы
        ; найдем блок в котором он находится

        div     [ext2_data.block_size]
        mov     ecx, [ext2_data.sb]
        add     eax, [ecx + EXT2_SB_STRUC.first_data_block]
        inc     eax
        mov     ebx, [ext2_data.ext2_temp_block]
        call    ext2_get_block

        add     ebx, edx                        ; локальный номер в блоке
        mov     eax, [ebx+8]                    ; номер блока - в терминах ext2

        mov     ecx, [ext2_data.log_block_size]
        shl     eax, cl
        add     eax, [PARTITION_START]          ; а старт раздела - в терминах hdd (512)

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

        mov     esi, edx                        ;добавим "остаток"
        add     esi, ebx                        ;к адресу
       ; mov     ecx, [ext2_data.inode_size]
        rep movsb                               ;копируем inode
        popad
        ret

;----------------------------------------------------------------
; in:  esi -> children
;      ebx -> pointer to dir block
; out: esi -> name without parent or not_changed
;      ebx -> dir_rec of inode children      or trash
ext2_test_block_by_name:
        push    eax ecx edx edi

        mov     edx, ebx
        add     edx, [ext2_data.block_size]       ;запомним конец блока

        .start_rec:
        cmp     [ebx + EXT2_DIR_STRUC.inode], 0
        jz      .next_rec

        push    esi
        movzx   ecx, [ebx + EXT2_DIR_STRUC.name_len]
        mov     edi, EXT2_filename
        lea     esi, [ebx + EXT2_DIR_STRUC.name]

        call    utf8toansi_str
        mov     ecx, edi
        sub     ecx, EXT2_filename          ;кол-во байт в получившейся строке

        mov     edi, EXT2_filename
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
        jmp     .ret

    .test_find:
        cmp     byte [esi], 0
        je      .find               ;нашли конец
        cmp     byte [esi], '/'
        jne     @B
        inc     esi
    .find:
        pop     eax     ;удаляем из стека сохраненое значение
    .ret:
        pop     edi edx ecx eax
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
        jz      .doit

        push    ecx ebx
        call    ext2_find_lfn
        jnc     .doit2
        pop     ebx
    .not_found:
        pop     ecx
        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret

    .doit:
        mov     ebp, [ext2_data.root_inode]
        push    ecx
        jmp     @F
    .doit2:
        pop     ebx
        test    [ebp + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR
        jz      .not_found
    @@:
        xor     eax, eax
        mov     edi, edx
        mov     ecx, 32/4
        rep stosd                               ; fill header zero
        pop     edi                             ; edi = число блоков для чтения
        push    edx ebx

     ;--------------------------------------------- final step
        and     [EXT2_read_in_folder], 0
        and     [EXT2_files_in_folder], 0

        mov     eax, [ebp + EXT2_INODE_STRUC.i_blocks]
        mov     [EXT2_counter_blocks], eax

        add     edx, 32                         ; (header pointer in stack) edx = current mem for return
        xor     esi, esi                        ; esi = номер блока по порядку

    .new_block_folder:              ;reserved label
        mov     ecx, esi                        ; получим номер блока
        call    ext2_get_inode_block

        mov     eax, ecx
        mov     ebx, [ext2_data.ext2_save_block]
        call    ext2_get_block                  ; и считываем блок с hdd

        mov     eax, ebx                        ; eax = current dir record
        add     ebx, [ext2_data.block_size]
        mov     [EXT2_end_block], ebx           ; запомним конец очередного блока

        pop     ecx
        mov     ecx, [ecx]                      ; ecx = first wanted (flags ommited)

    .find_wanted_start:
        jecxz   .find_wanted_end
    .find_wanted_cycle:
        cmp     [eax + EXT2_DIR_STRUC.inode], 0         ; if (inode = 0) => not used
        jz      @F
        inc     [EXT2_files_in_folder]
        dec     ecx
          @@:
        movzx   ebx, [eax+EXT2_DIR_STRUC.rec_len]

        cmp     ebx, 12                                 ; минимальная длина записи
        jb      .end_error
        test    ebx, 0x3                                ; длина записи должна делиться на 4
        jnz     .end_error

        add     eax, ebx                                ; к следующей записи
        cmp     eax, [EXT2_end_block]                  ; проверяем "конец"
        jb      .find_wanted_start

        push    .find_wanted_start
   .end_block:                                                  ;вылетили из цикла
        mov     ebx, [ext2_data.count_block_in_block]
        sub     [EXT2_counter_blocks], ebx
        jbe     .end_dir

        inc     esi                                                                             ;получаем новый блок
        push    ecx
        mov     ecx, esi
        call    ext2_get_inode_block
        mov     eax, ecx
        mov     ebx, [ext2_data.ext2_save_block]
        call    ext2_get_block
        pop     ecx
        mov     eax, ebx
        add     ebx, [ext2_data.block_size]
        mov     [EXT2_end_block], ebx
        ret                                                                                             ; опять в цикл

    .wanted_end:
        loop    .find_wanted_cycle                      ; ecx = -1

    .find_wanted_end:
        mov     ecx, edi
    .wanted_start:                                      ; ищем first_wanted+count
        jecxz   .wanted_end
        cmp     [eax + EXT2_DIR_STRUC.inode], 0         ; if (inode = 0) => not used
        jz      .empty_rec
        inc     [EXT2_files_in_folder]
        inc     [EXT2_read_in_folder]

        mov     edi, edx
        push    eax ecx
        xor     eax, eax
        mov     ecx, 40 / 4
        rep stosd
        pop     ecx eax

        push    eax esi edx                                            ;получим inode
        mov     eax, [eax + EXT2_DIR_STRUC.inode]
        mov     ebx, [ext2_data.ext2_temp_inode]
        call    ext2_get_inode

        lea     edi, [edx + 8]

        mov     eax, [ebx + EXT2_INODE_STRUC.i_ctime]               ; переведем время в ntfs формат
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
        test    [ebx + EXT2_INODE_STRUC.i_mode], EXT2_S_IFDIR       ; для папки размер
        jnz     @F                                                  ; не возвращаем

        mov     eax, [ebx + EXT2_INODE_STRUC.i_size]                ;low size
        stosd
        mov     eax, [ebx + EXT2_INODE_STRUC.i_dir_acl]             ;high size
        stosd
        xor     dword [edx], FS_FT_DIR
        @@:
        xor     dword [edx], FS_FT_DIR
        pop     esi eax

        or      dword [edx+4], FS_FT_ASCII          ; symbol type in name

            ;теперь скопируем имя, сконвертировав из UTF-8 в CP866
        push    eax ecx esi
        movzx   ecx, [eax + EXT2_DIR_STRUC.name_len]
        lea     edi, [edx + 40]
        lea     esi, [eax + EXT2_DIR_STRUC.name]
        call    utf8toansi_str
        pop     esi ecx eax
        and     byte [edi], 0

        cmp     byte [edx + 40], '.'
        jne     @F
        or      dword [edx], FS_FT_HIDDEN
        @@:

        add     edx, 40 + 264                           ; go to next record
        dec     ecx                                     ; если запись пустая ecx не надо уменьшать
        .empty_rec:
        movzx   ebx, [eax + EXT2_DIR_STRUC.rec_len]
        cmp     ebx, 12                                 ; минимальная длина записи
        jb      .end_error
        test    ebx, 0x3                                ; длина записи должна делиться на 4
        jnz     .end_error

        add     eax, ebx
        cmp     eax, [EXT2_end_block]
        jb      .wanted_start

        push    .wanted_start                           ; дошли до конца очередного блока
        jmp     .end_block

    .end_dir:
        pop     eax             ; мусор (адрес возврата в цикл)
    .end_error:
        pop     edx
        mov     ebx, [EXT2_read_in_folder]
        mov     ecx, [EXT2_files_in_folder]
        mov     dword [edx], 1    ;version
        xor     eax, eax
        mov     [edx+4], ebx
        mov     [edx+8], ecx
        lea     edi, [edx + 12]
        mov     ecx, 20 / 4
        rep stosd
        ret
;====================== end ext2_HdReadFolder
utf8toansi_str:
; convert UTF-8 string to ASCII-string (codepage 866)
; in: ecx=length source, esi->source, edi->buffer
; destroys: eax,esi,edi
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
;
;--------------------------------------------------------------
ext2_HdRead:
        cmp     byte [esi], 0
        jnz     @F

    .this_is_nofile:
        or      ebx, -1
        mov     eax, ERROR_ACCESS_DENIED
        ret

    @@:
        push    ecx ebx
        call    ext2_find_lfn
        pop     ebx ecx
        jnc     .doit
    ;.not_found:
        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret

    .doit:
        test    [ebp + EXT2_INODE_STRUC.i_mode], EXT2_S_IFREG
        jz      .this_is_nofile

    ;-----------------------------------------------------------------------------final step
        mov     edi, edx            ; edi = pointer to return mem
        mov     esi, ebx            ; esi = pointer to first_wanted

        ;///// сравним хватит ли нам файла или нет
        mov     ebx, [esi+4]
        mov     eax, [esi]          ; ebx : eax - стартовый номер байта

        cmp     [ebp + EXT2_INODE_STRUC.i_dir_acl], ebx
        ja      .size_great
        jb      .size_less

        cmp     [ebp + EXT2_INODE_STRUC.i_size], eax
        ja      .size_great

    .size_less:
        xor     ebx, ebx
        mov     eax, ERROR_END_OF_FILE
        ret
    .size_great:
        add     eax, ecx                  ;add to first_wanted кол-во байт для чтения
        adc     ebx, 0

        cmp     [ebp + EXT2_INODE_STRUC.i_dir_acl], ebx
        ja      .size_great_great
        jb      .size_great_less
        cmp     [ebp + EXT2_INODE_STRUC.i_size], eax
        jae     .size_great_great               ; а если равно, то не важно куда

    .size_great_less:
        or      [EXT2_files_in_folder], 1       ;читаем по границе размера
        mov     ecx, [ebp + EXT2_INODE_STRUC.i_size]
        sub     ecx, [esi]                      ;(размер - старт)
        jmp     @F

    .size_great_great:
        and     [EXT2_files_in_folder], 0       ;читаем столько сколько запросили

    @@:
        push    ecx                             ;save for return
        test    esi, esi
        jz      .zero_start

        ;пока делаем п..ц криво =)
        mov     edx, [esi+4]
        mov     eax, [esi]
        div     [ext2_data.block_size]

        mov     [EXT2_counter_blocks], eax       ;номер блока запоминаем

        push    ecx
        mov     ecx, eax
        call    ext2_get_inode_block
        mov     ebx, [ext2_data.ext2_save_block]
        mov     eax, ecx
        call    ext2_get_block
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
        jmp     @F

    .zero_start:
        mov     eax, ecx
        ;теперь в eax кол-во оставшихся байт для чтения
    @@:
        mov     ebx, edi                        ;чтение блока прям в ->ebx
        xor     edx, edx
        div     [ext2_data.block_size]          ;кол-во байт в последнем блоке (остаток) в edx
        mov     edi, eax                        ;кол-во целых блоков в edi
    @@:
        test    edi, edi
        jz      .finish_block
        inc     [EXT2_counter_blocks]
        mov     ecx, [EXT2_counter_blocks]
        call    ext2_get_inode_block

        mov     eax, ecx                        ;а ebx уже забит нужным значением
        call    ext2_get_block
        add     ebx, [ext2_data.block_size]

        dec     edi
        jmp     @B

    .finish_block:          ;в edx - кол-во байт в последнем блоке
        test    edx, edx
        jz      .end_read

        mov     ecx, [EXT2_counter_blocks]
        inc     ecx
        call    ext2_get_inode_block

        mov     edi, ebx
        mov     eax, ecx
        mov     ebx, [ext2_data.ext2_save_block]
        call    ext2_get_block

        mov     ecx, edx

    .only_one_block:
        mov     esi, ebx
        rep movsb                               ;кусок last блока
    .end_read:
        pop     ebx
        cmp     [EXT2_files_in_folder], 0
        jz      @F

        mov     eax, ERROR_END_OF_FILE
        ret
    @@:
        xor     eax, eax
        ret
;========================
;in : esi -> name           not save: eax ebx ecx
;out: ebp -> inode cf=0
;     ebp -> trash cf=1
ext2_find_lfn:
        mov     ebp, [ext2_data.root_inode]
    .next_folder:
        or      [EXT2_counter_blocks], -1               ;счетчик блоков папки    cur block of inode
        mov     eax, [ebp + EXT2_INODE_STRUC.i_blocks]  ;убывающий счетчик блоков
        add     eax, [ext2_data.count_block_in_block]
        mov     [EXT2_end_block], eax
    .next_block_folder:
        mov     eax, [ext2_data.count_block_in_block]
        sub     [EXT2_end_block], eax
        jz      .not_found
        inc     [EXT2_counter_blocks]
        mov     ecx, [EXT2_counter_blocks]
        call    ext2_get_inode_block

        mov     eax, ecx
        mov     ebx, [ext2_data.ext2_save_block]        ;ebx = cur dir record
        call    ext2_get_block

        mov     eax, esi
        call    ext2_test_block_by_name
        cmp     eax, esi                                ;нашли имя?
        jz      .next_block_folder

        cmp     byte [esi], 0
        jz      .get_inode_ret

        cmp     [ebx + EXT2_DIR_STRUC.file_type], EXT2_FT_DIR
        jne     .not_found                                      ;нашли, но это не папка
        mov     eax, [ebx + EXT2_DIR_STRUC.inode]
        mov     ebx, [ext2_data.ext2_save_inode]                ;все же папка.
        call    ext2_get_inode
        mov     ebp, ebx
        jmp     .next_folder

    .not_found:
        stc
        ret
    .get_inode_ret:
        mov     [EXT2_end_block], ebx                           ; сохраняем указатеть на dir_rec
        mov     eax, [ebx + EXT2_DIR_STRUC.inode]
        mov     ebx, [ext2_data.ext2_save_inode]
        call    ext2_get_inode
        mov     ebp, ebx
        clc
        ret


;========================

ext2_HdGetFileInfo:
        cmp     byte [esi], 0
        jz      .doit

        call    ext2_find_lfn
        jnc     .doit2
    ;.not_found:
        mov     eax, ERROR_FILE_NOT_FOUND
        ret

    .doit:
        mov     ebp, [ext2_data.root_inode]
        mov     ebx, .doit                        ;неважно что лишь бы этому адресу не '.'
        jmp     @F
    .doit2:
        mov     ebx, [EXT2_end_block]
        add     ebx, EXT2_DIR_STRUC.name
    @@:
        xor     eax, eax
        mov     edi, edx
        mov     ecx, 40/4
        rep stosd                   ; fill zero

        cmp     byte [ebx], '.'
        jnz     @F
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
        mov     eax, [ebx + EXT2_INODE_STRUC.i_ctime]
        xor     edx, edx
        add     eax, 3054539008
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
;----------------------------------------------------------------
;
;  ext2_HdCreateFolder - create new folder
;
;  esi  points to filename
;
;  ret  eax = 0 ok read or other = errormsg
;
;--------------------------------------------------------------
        cmp     byte [esi], 0
        jz      .not_found
        cmp     byte [esi], '/'
        jz      .not_found

        mov     ebx, esi            ; save source pointer
        xor     edi, edi            ; slah pointer
    @@:
        lodsb
        cmp     al, 0
        jz      .zero
        cmp     al, '/'
        jz      .slash
        jmp     @B

    .slash:
        lodsb
        cmp     al, 0
        jz      .zero               ; уберем слеш из имени
        cmp     al, '/'
        jz      .not_found
        mov     edi, esi            ; edi -> next symbol after '/'
        dec     edi
        jmp     @B

    .zero:
        dec     esi
        test    edi, edi
        jz      .doit

        ;слеш был
        mov     eax, esi
        sub     eax, edi
        mov     [EXT2_name_len], eax

        mov     ecx, edi
        sub     ecx, ebx
        dec     ecx                     ;выкинули '/' из имени ролителя
        mov     esi, ebx
        mov     edi, EXT2_parent_name
        rep movsb
        ; esi - pointer to last slash

        mov     edx, esi
        mov     esi, EXT2_parent_name
        call    ext2_find_lfn
        jnc     .doit2
    .not_found:
        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret

    .doit:
        mov     ebp, [ext2_data.root_inode]
        mov     edx, ebx                        ; имя создаваемой папки
        sub     esi, ebx
        mov     [EXT2_name_len], esi
    .doit2:
        ;ebp -> parent_inode    ebx->name_new_folder   [EXT2_name_len]=length of name
; стратегия выбора группы для нового inode: (так делает линукс)
; 1) Ищем группу в которой меньше всего папок и в есть свободное место
; 2) Если такая группа не нашлась, то берем группу в которой больше свободного места




        call    ext2_balloc
        jmp     ext2_HdDelete

        push    ebx
        push    ebp

        mov     ecx, [ext2_data.sb]
        cmp     [ecx + EXT2_SB_STRUC.free_inodes_count], 0      ; есть ли место для inode
        jz      .no_space
        mov     eax, [ecx + EXT2_SB_STRUC.free_block_count]
        sub     eax, [ecx + EXT2_SB_STRUC.r_block_count]
        cmp     eax, 2                                          ; и как минимум на 2 блока
        jb      .no_space

        mov     ecx, [ext2_data.groups_count]
        mov     esi, [ext2_data.global_desc_table]
        mov     edi, -1                         ;указатель на лучшую группу
        mov     edx, 0
    .find_group_dir:
        jecxz   .end_find_group_dir
        movzx   eax, [esi + EXT2_BLOCK_GROUP_DESC.free_inodes_count]
        cmp     eax, edx
        jbe     @F
        cmp     [esi + EXT2_BLOCK_GROUP_DESC.free_blocks_count], 0
        jz      @F
        mov     edi, esi
        movzx   edx, [esi + EXT2_BLOCK_GROUP_DESC.free_inodes_count]
    @@:
        dec     ecx
        add     esi, 32                         ;размер структуры
        jmp     .find_group_dir
    .end_find_group_dir:
        cmp     edx, 0
        jz      .no_space

        ;нашли группу, получим битовую карту inode-ов (найдем locale number)
        mov     eax, [edi + EXT2_BLOCK_GROUP_DESC.inode_bitmap]
        mov     ebx, [ext2_data.ext2_save_block]
        call    ext2_get_block

        ;теперь цикл по всем битам
        mov     esi, ebx
        mov     ecx, [ext2_data.inodes_per_group]
        shr     ecx, 5                              ;делим на 32
        mov     ebp, ecx                            ; всего сохраним в ebp
        or      eax, -1                             ; ищем первый свободный inode (!= -1)
        repne scasd
        jnz     .test_last_dword                    ;нашли или нет
        mov     eax, [esi-4]

        sub     ebp, ecx
        dec     ebp
        shl     ebp, 5                              ; глобальный номер локального номера

        mov     ecx, 32
    @@:
        test    eax, 1
        jz      @F
        shr     eax, 1
        loop    @B
    @@:
        mov     eax, 32
        sub     eax, ecx

        add     ebp, eax                            ; locale num of inode

        mov     eax, [esi-4]
    ;устанавливаем в eax крайний справа нулевой бит в 1
        mov     ecx, eax
        inc     ecx
        or      eax, ecx        ; x | (x+1)
        mov     [esi-4], eax
        mov     ebx, [ext2_data.ext2_save_block]
        mov     eax, [edi + EXT2_BLOCK_GROUP_DESC.inode_bitmap]
        call    ext2_set_block
    ;считаем таблицу inode
        sub     edi, [ext2_data.global_desc_table]
        shr     edi, 5

        mov     eax, edi
        mul     [ext2_data.inodes_per_group]
        add     eax, ebp
        inc     eax                                     ; теперь в eax (ebp) номер inode-а
        mov     ebp, eax
        ;call    ext2_get_inode_address

        mov     ebx, [ext2_data.ext2_save_block]
        call    hd_read
        add     edx, ebx                                ; в edx адрес нужного inode

        ;забьем 0 для начала
        mov     edi, edx
        mov     ecx, [ext2_data.inode_size]
        shr     ecx, 2
        xor     eax, eax
        rep stosd

        mov     edi, edx
        mov     eax, EXT2_S_IFDIR or EXT2_777_MODE
        stosd                                           ; i_mode
        xor     eax, eax
        stosd                                           ; i_uid
        mov     eax, [ext2_data.block_size]
        stosd                                           ; i_size
        xor     eax, eax
        stosd                                           ; i_atime
        stosd                                           ; i_ctime
        stosd                                           ; i_mtime
        stosd                                           ; i_dtime
        stosd                                           ; i_gid
        inc     eax
        stosd                                           ; i_links_count
        mov     eax, [ext2_data.count_block_in_block]
        stosd                                           ; i_blocks




.test_last_dword:

        xor     ebx, ebx
        mov     eax, ERROR_UNSUPPORTED_FS
        ret



    .no_space:
        or      ebx, -1
        mov     eax, ERROR_DISK_FULL
        ret

;выделяет новый блок, если это можно
;иначе возвращает eax=0
ext2_balloc:
        mov     ecx, [ext2_data.sb]
        mov     eax, [ecx + EXT2_SB_STRUC.free_block_count]
        sub     eax, [ecx + EXT2_SB_STRUC.r_block_count]
        jbe     .no_space

        mov     ecx, [ext2_data.groups_count]
        mov     edi, [ext2_data.global_desc_table]
        ;mov     esi, -1                         ;указатель на лучшую группу
        mov     edx, 0
    .find_group:
        jecxz   .end_find_group
        movzx   eax, [edi + EXT2_BLOCK_GROUP_DESC.free_blocks_count]
        cmp     eax, edx
        jbe     @F
        mov     esi, edi
        mov     edx, eax
    @@:
        dec     ecx
        add     edi, 32                         ;размер структуры
        jmp     .find_group
    .end_find_group:
        cmp     edx, 0
        jz      .no_space

        ;нашли группу, получим битовую карту block-ов
        mov     eax, [esi + EXT2_BLOCK_GROUP_DESC.block_bitmap]
        mov     ebx, [ext2_data.ext2_save_block]
        call    ext2_get_block

        ;теперь цикл по всем битам
        mov     edi, ebx
        mov     ecx, [ext2_data.blocks_per_group]
        shr     ecx, 5                              ;делим на 32
        mov     ebp, ecx                            ;всего сохраним в ebp
        or      eax, -1                             ;ищем первый свободный inode (!= -1)
        repe scasd
        jz      .test_last_dword                    ;нашли или нет

        mov     eax, [edi-4]
        sub     ebp, ecx
        dec     ebp
        shl     ebp, 5                              ; ebp = 32*(номер div 32). Теперь найдем (номер mod 32)

        mov     ecx, 32
    @@:
        test    eax, 1
        jz      @F
        shr     eax, 1
        loop    @B
    @@:
        mov     eax, 32
        sub     eax, ecx

        add     ebp, eax                            ; ebp = номер блока в группе

        mov     eax, [edi-4]
        mov     ecx, eax
        inc     ecx
        or      eax, ecx        ; x | (x+1) - устанавливает в 1 крайний справа нулевой бит (block used)
        mov     [edi-4], eax

        mov     ebx, [ext2_data.ext2_save_block]
        mov     eax, [esi + EXT2_BLOCK_GROUP_DESC.inode_bitmap]
       ; call    ext2_set_block                      ; и пишем на hdd новую битовую маску

        ;============== тут получаем номер блока
        mov     eax, [ext2_data.blocks_per_group]
        sub     esi, [ext2_data.global_desc_table]
        shr     esi, 5                                  ;esi - номер группы
        mul     esi
        add     ebp, eax                                ;(номер_группы) * (blocks_per_group) + локальный номер в группе
        mov     eax, [ext2_data.sb]
        add     ebp, [eax + EXT2_SB_STRUC.first_data_block]

        ;теперь поправим глобальную дескрипторную таблицу и суперблок
        mov     ebx, [ext2_data.sb]
        dec     [ebx + EXT2_SB_STRUC.free_block_count]
        mov     eax, 2
        add     eax, [PARTITION_START]
        call    hd_write
        mov     eax, [ebx + EXT2_SB_STRUC.first_data_block]
        inc     eax
        dec     [esi + EXT2_BLOCK_GROUP_DESC.free_blocks_count];edi все еще указывает на группу в которой мы выделил блок
        call    ext2_set_block

        mov     eax, ebx
        ret

    .test_last_dword:
        lodsd
        mov     ecx, [ext2_data.blocks_per_group]
        and     ecx, not (32-1)                     ;обнуляем все кроме последних 5 бит
        mov     edx, ecx
        mov     ebx, 1
    @@:
        jecxz   .no_space
        mov     edx, ebx
        or      edx, eax                            ; тестируем очередной бит
        shl     ebx, 1
        jmp     @B
    @@:
        sub     edx, ecx
        dec     edx                                 ;номер в последнем блоке


    .no_space:
        xor     eax, eax
        ret

;in: eax = i_block
;    ebx = pointer to memory
ext2_set_block:
        push    eax ebx ecx
        mov     ecx, [ext2_data.log_block_size]
        shl     eax, cl
        add     eax, [PARTITION_START]
        mov     ecx, [ext2_data.count_block_in_block]
    @@:
        call    hd_write
        inc     eax
        add     ebx, 512
        loop    @B
        pop     ecx ebx eax
        ret

