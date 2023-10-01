; data for parsing string
param_cmd: dd 0 ;set when for "-a" command

; virtdisk -d <DISK_NUMBER>
; virtdisk -i <DISK_NUMBER>
; virtdisk -a <PATH> -s <SECTOR_SIZE> -t <IMAGE_TYPE> -f <FLAGS>
; virtdisk -l
parse_cmd:
        mov     edi, PATH
        ; find string length
        xor     al, al
        mov     ecx, 4096
        repne   scasb

        mov     ecx, edi
        sub     ecx, PATH
        mov     edi, PATH
.still:
        mov     al, ' '
        repz    scasb

        test    ecx, ecx
        jz      .end_parser

        dec     edi
        or      word[edi], 0x2020 ; переводим в нижний регистр
        ; -a -d -i -l -s -t -f

        cmp     word[edi], '-a'
        jnz     @f
        ;add virt disk
        mov     dword[param_cmd],-1

        add     edi, 3
        sub     ecx, 2
        js      ERROR_EXIT ; error not found path

        mov     edx, add_disk.file
        call    .copy_str
        or      dword[edx -4], 0x20202020
        mov     dword[add_disk.size], 512
        cmp     dword[edx -4], '.iso'
        jnz     .still
        mov     dword[add_disk.size], 2048

        jmp     .still
@@:
        cmp     word[edi], '-d'
        jnz     @f

        add     edi, 3
        sub     ecx, 2
        js      ERROR_EXIT ; error not found path

        call    .get_number
        mov     [disk_num],eax

        pusha
        mov     al, 68
        mov     bl, 17
        mov     ecx, ioctl_del_disk
        int     0x40

        push    str_command_successfully
        call    _sc_puts
        popa

        jmp     .still
@@:
        cmp     word[edi], '-i'
        jnz     .no_disk_info
        ; write info
        add     edi, 3
        sub     ecx, 2
        js      ERROR_EXIT ; error not found path
        ; get disk number
        call    .get_number
        mov     [disk_num],eax

        pusha
        mov     al, 68
        mov     bl, 17
        mov     ecx, ioctl_info_disk
        int     0x40

        call    write_disk_info
        popa
        jmp     .still

.no_disk_info:
        cmp     word[edi], '-l'
        jnz     .no_disk_list
        ; write list disks
        add     edi, 2
        sub     ecx, 1
        pusha

        mov     al, 68
        mov     bl, 17
        mov     ecx, ioctl_count_disk
        int     0x40
        test    eax, eax
        jnz     ERROR_EXIT

        push    str_header_disk_list
        call    _sc_puts

        mov     ecx, ioctl_list_disk.count
        mov     eax, 68
        mov     bl, 12
        imul    ecx, sizeof.info_buffer
        add     ecx, 4
        mov     [ioctl_list_disk.size_buffer], ecx
        int     0x40
        test    eax, eax
        jz      ERROR_EXIT

        mov     [ioctl_list_disk.buffer], eax
        mov     esi, eax
        mov     edi, eax
        add     esi, 4

        mov     al, 68
        mov     bl, 17
        mov     ecx, ioctl_list_disk
        int     0x40
        test    eax, eax
        jnz     ERROR_EXIT

        cmp     dword[edi], 0
        jz      .end_list
.next_item_list:

        ; num2str
        push    dword 10
        mov     ecx, esp
        mov     eax, [esi + info_buffer.disk_num - info_buffer]
@@:
        xor     edx, edx
        div     dword[esp]
        dec     ecx
        add     dl, '0'
        mov     byte[ecx], dl
        test    eax, eax
        jnz     @b

        mov     edx, str_input_disk_number + 1
        mov     dword[edx], '    '
@@:
        mov     al, byte[ecx]
        mov     byte[edx], al
        inc     edx
        inc     ecx
        cmp     ecx, esp
        jnz      @b
        ;-------
        mov     ecx, esp
        mov     eax, [esi + info_buffer.sector_size - info_buffer]
@@:
        xor     edx, edx
        div     dword[esp]
        dec     ecx
        add     dl, '0'
        mov     byte[ecx], dl
        test    eax, eax
        jnz     @b

        mov     edx, str_input_disk_sector
        mov     dword[edx], '    '
@@:
        mov     al, byte[ecx]
        mov     byte[edx], al
        inc     edx
        inc     ecx
        cmp     ecx, esp
        jnz      @b
        ;-------
        add      esp, 4
        ; flags
        mov     dword[str_input_disk_flags], '    '
        cmp     dword[esi + info_buffer.flags - info_buffer], 1b
        jnz     @f
        mov     word[str_input_disk_flags], 'ro'
@@:
        cmp     dword[esi + info_buffer.flags - info_buffer], 11b
        jnz     @f
        mov     word[str_input_disk_flags], 'rw'
@@:
        ;-------
        pusha
        add     esi, info_buffer.path - info_buffer
        push    esi
        push    str_input_disk_number
        call    _sc_puts
        call    _sc_puts
        push    str_newline
        call    _sc_puts
        popa

        add     esi, sizeof.info_buffer
        dec     dword[edi]
        jnz     .next_item_list
.end_list:

        mov     eax, 68
        mov     ebx, 13
        mov     ecx, edi
        int     0x40

        popa
        jmp     .still
.no_disk_list:
        cmp     dword[param_cmd],0
        jz      .no_cmd

        cmp     word[edi], '-s'
        jnz     .no_sector_size
        ; set sector size for -a command
        add     edi, 3
        sub     ecx, 2
        js      ERROR_EXIT ; error
        ; get number
        call    .get_number
        mov     [add_disk.size], eax
        jmp     .still
.no_sector_size:
        cmp     word[edi], '-t'
        jnz     .no_disk_type
        ; set image type for -a command
        add     edi, 3+3
        sub     ecx, 2+3
        js      ERROR_EXIT ; error

        or      dword[edi - 4], 0x20202020
        cmp     dword[edi - 4], ' raw'
        jnz     .still
        ; TODO!!!
        mov     dword[add_disk.type], 0
        jmp     .still
.no_disk_type:
        cmp     word[edi], '-f'
        jnz     .no_cmd
        ; set flags for -a command
        add     edi, 3+2
        sub     ecx, 2+2
        js      ERROR_EXIT ; error

        or      word[edi - 2], 0x2020
        cmp     word[edi - 2], 'ro'
        jnz     @f
        mov     dword[add_disk.flags], 1b

@@:     cmp     word[edi - 2], 'rw'
        jnz     .still
        mov     dword[add_disk.flags], 11b
        jmp     .still
.no_cmd:
        inc     edi
        jmp     .still
.end_parser:
        ret

.get_str:
        push    edi
        inc     dword[esp]
        mov     al, '"'
        cmp     byte[edi], al
        jz      @f
        dec     dword[esp]
        mov     al, ' '
        dec     edi
@@:
        inc     edi
        repne   scasb
        and     byte[edi - 1], 0
        pop     eax
        ret

; edx - buffer
.copy_str:
        mov     al, ' '
        cmp     byte[edi], '"'
        jnz     @f
        mov     al, '"'
        inc     edi
        dec     ecx
@@:
        mov     ah, byte[edi]
        test    ah, ah
        jz      @f
        cmp     ah, al
        jz      @f
        mov     byte[edx], ah
        inc     edx
        inc     edi
        dec     ecx
        jmp     @b
@@:
        mov     byte[edx], 0
        ret

.get_number:
        xor     eax, eax
@@:
        movzx   edx, byte[edi]
        test    edx, edx
        jz      @f
        cmp     dl, ' '
        jz      @f
        sub     dl, '0'
        js      ERROR_EXIT

        cmp     dl, 9
        ja      ERROR_EXIT

        imul    eax, 10
        add     eax, edx
        dec     ecx
        inc     edi
        jmp     @b
@@:
        ret