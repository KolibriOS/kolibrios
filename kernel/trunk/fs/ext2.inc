;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;   23.01.2010  turbanoff  - read from ext2fs                  ;;
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
EXT2_S_IFREG            = 0x8000
EXT2_S_IFDIR            = 0x4000
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
EXT2_S_IROTH        = 0x0002
EXT2_S_IROTH        = 0x0001

EXT2_FT_REG_FILE        = 1     ;это файл, запись в родительском каталоге
EXT2_FT_DIR             = 2     ;это папка

FS_FT_DIR               = 0x10  ;это папка
FS_FT_ASCII             = 0     ;имя в ascii
FS_FT_UNICODE           = 1     ;имя в unicode

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


ext2_test_superblock:
        mov     eax, [PARTITION_START]
        add     eax, 2                  ;superblock start at 1024b
        call    hd_read

        cmp     [fs_type], 0x83
        jne     .no
        cmp     dword [ebx+24], 3       ;s_block_size 0,1,2,3
        ja      .no
        cmp     word [ebx+56], 0xEF53   ;s_magic
        jne     .no
        cmp     word [ebx+58], 1        ;s_state (EXT_VALID_FS=1)
        jne     .no

    ; OK, this is correct EXT2 superblock
        clc
        ret
   .no:
    ; No, this superblock isn't EXT2
        stc
        ret

ext2_setup:
        mov     [fs_type], 2
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

        push    eax eax eax                             ;3 kernel_alloc

        mov     eax, edx
        mul     edx
        mov     [ext2_data.count_pointer_in_block_square], eax

        call    kernel_alloc
        mov     [ext2_data.global_desc_table],eax       ;reserve mem for gdt
        call    kernel_alloc
        mov     [ext2_data.ext2_save_block], eax        ; and for temp block
        call    kernel_alloc
        mov     [ext2_data.ext2_temp_block], eax        ; and for get_inode proc

        mov     ebp, [ebx+88]
        mov     ecx, [ebx+32]
        mov     edx, [ebx+40]
        mov     eax, [ebx+20]                           ; first_data_block

        mov     [ext2_data.inode_size], ebp
        mov     [ext2_data.blocks_per_group], ecx
        mov     [ext2_data.inodes_per_group], edx

        mov     ebx, [ext2_data.global_desc_table]
        inc     eax                                     ; first_data_block + 1 = gdt
        call    ext2_get_block                          ; read gtd

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

        popad
        call    free_hd_channel
        and     [hd1_status], 0
        ret
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
; in:  ecx = номер блока
;      ebp = адрес inode
; out: ecx = адрес очередного блока
ext2_get_inode_block:
        cmp     ecx, 12
        jbe     .get_direct_block

        sub     ecx, 12
        cmp     ecx, [ext2_data.count_pointer_in_block]
        jbe     .get_indirect_block

        sub     ecx, [ext2_data.count_pointer_in_block]
        cmp     ecx, [ext2_data.count_pointer_in_block_square]
        jbe     .get_double_indirect_block

    ;.get_triple_indirect_block:
        push    eax edx ebx

        mov    eax, [ebx + EXT2_INODE_STRUC.i_block + 14*4]
        mov    ebx, [ext2_data.ext2_temp_block]
        call    ext2_get_block

        xor     edx, edx
        mov     eax, ecx
        div     [ext2_data.count_pointer_in_block_square]

    ;eax - номер в полученном блоке   edx - номер дальше
        mov    eax, [ebx + eax*4]
        call    ext2_get_block

        mov    eax, edx
        jmp    @F

    .get_double_indirect_block:
        push    eax edx ebx

        mov     eax, [ebp + EXT2_INODE_STRUC.i_block + 13*4]
        mov     ebx, [ext2_data.ext2_temp_block]
        call    ext2_get_block

        mov     eax, ecx
      @@:
        xor     edx, edx
        div     [ext2_data.count_pointer_in_block]

        mov    eax, [ebx + eax*4]
        call    ext2_get_block
        mov    ecx, [ebx + edx*4]

        pop     ebx edx eax
        ret

    .get_indirect_block:
        push    eax ebx
        mov     eax, [ebp + EXT2_INODE_STRUC.i_block + 12*4]
        mov    ebx, [ext2_data.ext2_temp_block]
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
    mov     edi, ebx        ;сохраним адрес inode
    dec    eax
    xor    edx, edx
    div    [ext2_data.inodes_per_group]

        push    edx                             ;locale num

        mov     edx, 32
        mul     edx                             ; address block_group in global_desc_table

        add     eax, [ext2_data.global_desc_table]
        mov     eax, [eax+8]                    ; номер блока - в терминах ext2

        mov     ecx, [ext2_data.log_block_size]
        shl     eax, cl
        add     eax, [PARTITION_START]          ; а старт раздела - в терминах hdd (512)

        ;eax - указывает на таблицу inode-ов на hdd
        mov     ecx, eax                        ;сохраним его пока в ecx

        ; прибавим локальный адрес inode-а
        pop     eax                             ; index
        mul     [ext2_data.inode_size]        ; (index * inode_size)
        mov     ebp, 512
        div     ebp                             ;поделим на размер блока

        add     eax, ecx                        ;нашли адрес блока для чтения
        mov     ebx, [ext2_data.ext2_temp_block]
        call    hd_read

        mov     esi, edx                        ;добавим "остаток"
        add     esi, ebx                        ;к адресу
        mov     ecx, [ext2_data.inode_size]
        rep     movsb                           ;копируем inode
        popad
        ret

;----------------------------------------------------------------
ext2_upcase:
        cmp     al, 'a'
        jb      .ret
        cmp     al, 'z'
        ja      .ret
        and     al, 0xDF        ; upcase = clear 0010 0000
    .ret:
        ret
;----------------------------------------------------------------
; in:  esi -> children
;      ebx -> pointer to dir block
; out: esi -> name without parent or not_changed
;      ebx -> dir_rec of inode children      or trash
ext2_test_block_by_name:
        push    eax ecx edx edi

        mov        edx, ebx
        add        edx, [ext2_data.block_size]    ;запомним конец блока

        .start_rec:
            cmp     [ebx + EXT2_DIR_STRUC.inode], 0
            jz      .next_rec
;            test    [ebx + EXT2_DIR_STRUC.file_type], EXT2_FT_DIR
;            jz      .next_rec

            push    esi
            movzx   ecx, [ebx + EXT2_DIR_STRUC.name_len]
            lea     edi, [ebx + EXT2_DIR_STRUC.name]
            inc     ecx

          @@:
                dec     ecx
                jecxz   .test_find
                lodsb
                mov     ah, [edi]
                inc     edi

                call    ext2_upcase
                xchg    al, ah
                call    ext2_upcase
                cmp     al, ah
                je      @B
          @@:                           ;не подошло
            pop     esi
        .next_rec:
            movzx   eax, [ebx + EXT2_DIR_STRUC.rec_len]
            add     ebx, eax                                ;к след. записи
            cmp     ebx, edx                                ;проверим конец ли
            jne     .start_rec
            jmp     .ret

    .test_find:
        cmp     byte [esi], 0
        je      .find               ;нашли конец
        cmp     byte [esi], '/'
        jne      @B
        inc     esi
    .find:
        pop eax         ;удаляем из стека сохраненое значение
       ; mov ebx, [ebx + EXT2_DIR_STRUC.inode]
    .ret:
        pop edi edx ecx eax
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
uglobal
    EXT2_files_in_folder    dd ?   ;всего файлов в папке
    EXT2_read_in_folder     dd ?   ;сколько файлов "считали"
    EXT2_end_block          dd ?   ;конец очередного блока папки
    EXT2_counter_blocks     dd ?
endg

ext2_HdReadFolder:
        mov     ebp, [ext2_data.root_inode]
        mov     [EXT2_read_in_folder], ebx
        mov     [EXT2_files_in_folder], ecx     ;сохраним регистры

    .next_folder:
        cmp     byte [esi], 0
        jz      .get_folder

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
        je      .next_block_folder

        cmp     [ebx + EXT2_DIR_STRUC.file_type], EXT2_FT_DIR
        jne     .not_found                                      ;нашли, но это не папка
        mov     eax, [ebx + EXT2_DIR_STRUC.inode]
        mov     ebx, [ext2_data.ext2_save_inode]                ;все же папка.
        call    ext2_get_inode
        mov     ebp, ebx
        jmp     .next_folder

    .not_found:
        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
     ;--------------------------------------------- final step
        ; in ebp - pointer to final inode (folder).
    .get_folder:
        push    edx                             ;заголовок будем заполнять в конце (адрес - в стеке) edx
        push    [EXT2_read_in_folder]           ;сохраненный регистр тоже в стек

        mov     edi, [EXT2_files_in_folder]     ;edi = число блоков для чтения
        and     [EXT2_read_in_folder], 0
        and     [EXT2_files_in_folder], 0

        mov     esi, [ebp + EXT2_INODE_STRUC.i_blocks]
        mov     [EXT2_counter_blocks], esi

        add     edx, 32                         ;заголовок будем заполнять в конце (адрес - в стеке) edx = current mem for return
        xor     esi, esi                        ;esi = номер блока по порядку

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
            cmp     [eax + EXT2_DIR_STRUC.inode], 0     ; if (inode = 0) => not used
            je      @F
            inc     [EXT2_files_in_folder]
          @@:
            movzx   ebx, [eax+EXT2_DIR_STRUC.rec_len]
            add     eax, ebx                            ; к следующей записи
            cmp     eax, [EXT2_end_block]              ; проверяем "конец"
            je      .end_block_find_wanted
            loop    .find_wanted_cycle
    .find_wanted_end:

        mov     ecx, edi
    .wanted_start:                                      ; ищем first_wanted+count
        jecxz   .wanted_end
    .wanted_cycle:
            cmp     [eax + EXT2_DIR_STRUC.inode], 0     ; if (inode = 0) => not used
            jz      .empty_rec
            inc     [EXT2_files_in_folder]
            inc     [EXT2_read_in_folder]

            test    [eax + EXT2_DIR_STRUC.file_type], EXT2_FT_DIR   ;папка или нет
            jnz      @F

            push    eax                                             ;получим размер, если это файл
            mov     eax, [eax + EXT2_DIR_STRUC.inode]
            mov     ebx, [ext2_data.ext2_temp_inode]
            call    ext2_get_inode
            mov     eax, [ebx + EXT2_INODE_STRUC.i_size]            ;low size
            mov     ebx, [ebx + EXT2_INODE_STRUC.i_dir_acl]         ;high size
            mov     dword [edx+32], eax
            mov     dword [edx+36], ebx
            xor     dword [edx], FS_FT_DIR          ; для файлов xor - 2 раза
            pop     eax
          @@:
            xor     dword [edx], FS_FT_DIR
            mov     dword [edx+4], FS_FT_ASCII      ; symbol type in name

            push    ecx esi                         ;copy name
            movzx   ecx, [eax + EXT2_DIR_STRUC.name_len]
            mov     edi, edx
            add     edi, 40
            lea     esi, [eax + EXT2_DIR_STRUC.name]
            rep     movsb
            pop     esi ecx
            and     byte [edi], 0

            add     edx, 40 + 264                       ; go to next record
        .empty_rec:
            movzx   ebx, [eax + EXT2_DIR_STRUC.rec_len]
            add     eax, ebx
            cmp     eax, [EXT2_end_block]
            je      .end_block_wanted
            loop    .wanted_cycle

    .wanted_end:                        ;теперь дойдем до конца чтобы узнать сколько файлов в папке
        or      ecx, -1                 ;цикл уже есть, просто поставим ему огромный счетчик
        jmp     .find_wanted_cycle

    .end_block_find_wanted:                                   ;вылетили из цикла find_wanted
        mov     ebx, [ext2_data.count_block_in_block]
        sub     [EXT2_counter_blocks], ebx
        jz      .end_dir

        ;получаем новый блок
        inc     esi
        push    ecx
        mov     ecx, esi
        call    ext2_get_inode_block
        mov     eax, ecx
        mov     ebx, [ext2_data.ext2_save_block]
        call    ext2_get_block
        pop     ecx
        dec     ecx
        mov     eax, ebx
        add     ebx, [ext2_data.block_size]
        mov     [EXT2_end_block], ebx
        jmp     .find_wanted_start

    .end_block_wanted:                                      ;вылетели из цикла wanted
        mov     ebx, [ext2_data.count_block_in_block]
        sub     [EXT2_counter_blocks], ebx
        jz      .end_dir

        inc     esi
        push    ecx
        mov     ecx, esi
        call    ext2_get_inode_block
        mov     eax, ecx
        mov     ebx, [ext2_data.ext2_save_block]
        call    ext2_get_block
        pop     ecx
        dec     ecx
        mov     eax, ebx
        add     ebx, [ext2_data.block_size]
        mov     [EXT2_end_block], ebx
        jmp     .wanted_start

    .end_dir:
        pop     edx
        mov     ebx, [EXT2_read_in_folder]
        mov     ecx, [EXT2_files_in_folder]
        mov     dword [edx], 1    ;version
        xor     eax, eax
        mov     [edx+4], ebx
        mov     [edx+8], ecx
        lea     edi, [edx + 12]
        mov     ecx, 20 / 4
        rep     stosd
        ret
;====================== end ext2_HdReadFolder

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
        mov     ebp, [ext2_data.root_inode]

	push	ecx edx ebx
    .next_folder:
        push    esi
    @@:
            lodsb
            test    al, al
            jz      .find_end
            cmp     al, '/'
            jz      .find_folder
            jmp     @B

    .find_end:
        ;установим флаг что ищем файл или очередную папку
        mov     edi, 1
        jmp     .find_any
    .find_folder:
        xor     edi, edi
    .find_any:
        pop     esi

        cmp     byte [esi], 0
        jz      .not_found

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
        je      .next_block_folder

        cmp     [ebx + EXT2_DIR_STRUC.file_type], EXT2_FT_REG_FILE
        je      .test_file
        cmp     [ebx + EXT2_DIR_STRUC.file_type], EXT2_FT_DIR
        jne     .not_found
    .test_dir:
        cmp     edi, 0
        jne     .this_is_folder

        mov     eax, [ebx + EXT2_DIR_STRUC.inode]
        mov     ebx, [ext2_data.ext2_save_inode]                ;все же папка.
        call    ext2_get_inode
        mov     ebp, ebx
        jmp     .next_folder

    .test_file:
        cmp     edi, 0
        je      .not_found

        mov     eax, [ebx + EXT2_DIR_STRUC.inode]
        mov     ebx, [ext2_data.ext2_save_inode]
        call    ext2_get_inode
        jmp     .get_file

    .not_found:
    	pop     edx ecx ebx
        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
    .this_is_folder:
        pop     edx ecx ebx
        or      ebx, -1
        mov     eax, ERROR_ACCESS_DENIED
    .end_read:
        ret

    ;-----------------------------------------------------------------------------final step
    .get_file:
        xchg    bx, bx
        mov     ebp ,ebx

        ;pop     eax edi ecx         ; первый_блок память кол-во_байт
        mov     esi, [esp]
        mov     edi, [esp + 8]      ;edi = нужно считать байт

        ;///// сравним хватит ли нам файла или нет
        mov     ebx, [esi+4]
        mov     eax, [esi]          ; ebx : eax - стартовый номер байта

        mov     edx, [ebp + EXT2_INODE_STRUC.i_dir_acl]
        mov     ecx, [ebp + EXT2_INODE_STRUC.i_size]        ;edx : ecx - размер файла

        cmp     edx, ebx
        ja      .size_great
        jb      .size_less

        cmp     ecx, eax
        ja      .size_great

    .size_less:
        add     esp, 12
        mov     ebx, 0
        mov     eax, 6      ;EOF
        ret
    .size_great:
        ;прибавим к старту кол-во байт для чтения
        add     eax, edi
        jnc     @F
        inc     ebx
    @@:
        cmp     edx, ebx
        ja      .size_great_great
        jb      .size_great_less
        cmp     ecx, eax
        jae     .size_great_great
        ;jmp      .size_great_less
                                                ; а если равно, то не важно куда
    .size_great_less:
        or      [EXT2_files_in_folder], 1       ;читаем по границе размера
        sub     ecx, [esi]
        pop     eax edi edx                     ;ecx - не меняем
        jmp     @F

    .size_great_great:
        and     [EXT2_files_in_folder], 0       ;читаем нормально
        pop     eax edi ecx

    @@:
        push    ecx                             ;сохраним размер считанных байт в стеке
        test    eax, eax
        je      .zero_start

        ;пока делаем п..ц криво =)
        mov     edx, [eax+4]
        mov     eax, [eax]
        div     [ext2_data.block_size]

        mov     [EXT2_counter_blocks], eax       ;номер блока запоминаем

        push    ecx
        mov     ecx, eax
        call    ext2_get_inode_block
        mov     ebx , [ext2_data.ext2_save_block]
        mov     eax, ecx
        call    ext2_get_block
        pop     ecx
        add     ebx, edx

        neg     edx
        add     edx,[ext2_data.block_size]      ;block_size - стартоый блок = сколько байт 1-го блока
        cmp     ecx, edx
        jbe     .only_one_block

        mov     eax, ecx
        sub     eax, edx
        mov     ecx, edx

        mov     esi, ebx
        rep     movsb                           ;кусок 1-го блока

    .zero_start:
        mov     ebx, edi                        ;чтение блока прям в ebx
        ;теперь в eax кол-во оставшихся байт для чтения
        xor     edx, edx
        div     [ext2_data.block_size]
        mov     [EXT2_end_block], eax          ;кол-во целых блоков
    @@:
        cmp     [EXT2_end_block], 0
        jz      .finish_block
        inc     [EXT2_counter_blocks]
        mov     ecx, [EXT2_counter_blocks]
        call    ext2_get_inode_block

        mov     eax, ecx            ;а ebx уже забит нужным значением
        call    ext2_get_block
        add     ebx, [ext2_data.block_size]

        dec     [EXT2_end_block]
        jmp     @B

    .finish_block:          ;в edx - кол-во байт в последнем блоке
        cmp     edx, 0
        je      .end_read

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
        rep     movsb                           ;кусок 1-го блока (последнего)
        pop     ebx
        cmp     [EXT2_files_in_folder], 0
        jz      @F

        mov     eax, 6      ;EOF
        ret
    @@:
        xor     eax, eax
        ret
;========================

ext2_HdRewrite:
;    xchg    bx, bx
        xor     ebx, ebx
        mov     eax, ERROR_UNSUPPORTED_FS
        ret

ext2_HdWrite:
;    xchg    bx, bx
        xor     ebx, ebx
        mov     eax, ERROR_UNSUPPORTED_FS
        ret
ext2_HdSetFileEnd:
;    xchg    bx, bx
        xor     ebx, ebx
        mov     eax, ERROR_UNSUPPORTED_FS
        ret
ext2_HdGetFileInfo:
;    xchg    bx, bx
        xor     ebx, ebx
        mov     eax, ERROR_UNSUPPORTED_FS
        ret
ext2_HdSetFileInfo:
;    xchg    bx, bx
        xor     ebx, ebx
        mov     eax, ERROR_UNSUPPORTED_FS
        ret
ext2_HdDelete:
;    xchg    bx, bx
        xor     ebx, ebx
        mov     eax, ERROR_UNSUPPORTED_FS
        ret

