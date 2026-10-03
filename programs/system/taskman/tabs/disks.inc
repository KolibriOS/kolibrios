; Disks tab: the partitions in a sortable list, the used space as a bar
; across what the other columns leave. The kernel does not report free
; space, so for memory disks (/rd, /tmp*) the space taken by files is summed
; up. The disks are found in a thread, the tab is drawn at once.

struct DISK
        num             dd ?            ; in the order found, the key of the row
        icon            dd ?            ; in ICONS18, by the kind of the disk
        fill            dd ?            ; used, 1/1024 of the size, -1: unknown
        used            dd ?            ; KB, -1: unknown
        total           dd ?            ; KB
        name            rb 16           ; the path
        label           rb 28
ends

MAX_DISKS       = 16
WALK_MAX        = 1024                  ; path length
WALK_DEPTH      = 16

; Start the thread that finds the disks.
disks_scan:
        xor     eax, eax
        mov     [list.count], eax
        mov     [list.first], eax
        mov     [disk_n], eax
        mov     [walk_stop], eax

        inc     [walking]
        mcall   SF_THREAD_CONTROL, SSF_CREATE_THREAD, walker, walk_stack_top

        inc     eax                     ; -1: no thread, nothing to wait for
        jnz     @f
        and     [walking], 0
@@:
        ret

; The walker found more: the view of the disks found so far, sorted.
disks_update:
        xor     eax, eax
        xchg    eax, [walk_new]
        test    eax, eax
        jz      .done

        mov     esi, disks
        mov     ebp, disks_view
        mov     ecx, [disk_n]
@@:
        mov     [ebp], esi
        add     ebp, 4
        add     esi, sizeof.DISK
        loop    @b

        call    list_done
        jmp     list_rows

.done:
        ret

disks_open:                             ; the selected disk in Eolite
        cmp     [list.count], 0
        je      .done
        imul    edx, [list.key], sizeof.DISK      ; DISK.num
        add     edx, disks + DISK.name
        mov     esi, sz_eolite_path
        jmp     run_app

.done:
        ret

;-------------------------------------------------------------------------------
; The thread that finds the disks and sums up the files of memory disks.

disks_find:
        xor     ebp, ebp                ; device index in "/"

.device:
        mov     word [walk_path], "/"
        call    read_entry
        jnz     .done

        mov     edi, walk_path + 1
        call    fmt_str
        mov     ecx, edi                ; end of "/device"
        xor     edx, edx                ; partition index

.partition:
        mov     byte [ecx], 0
        xchg    ebp, edx
        call    read_entry
        xchg    ebp, edx
        jnz     .device

        cmp     [disk_n], MAX_DISKS
        jae     .done

        mov     edi, ecx
        mov     byte [edi], "/"
        inc     edi
        call    fmt_str
        call    disk_add
        jmp     .partition

.done:
        ret

; Read entry ebp of the folder [walk_path], ebp to the next one; out:
; ZF = 1 if read, esi -> its name
read_entry:
        cmp     [walk_stop], 0          ; asked to stop: the thread ends
        jne     walker.stop
        mov     [fs_read.Position], ebp
        inc     ebp
        mcall   SF_FILE, fs_read
        mov     esi, fs_entry.FileName
        test    eax, eax
        ret

; [walk_path] = root of the partition, edi -> its end
disk_add:
        pusha
        mcall   SF_FILE, fs_info
        test    eax, eax
        jnz     .done

        mov     eax, [disk_n]
        imul    ebx, eax, sizeof.DISK
        add     ebx, disks
        mov     [ebx + DISK.num], eax

        mov     ax, word [walk_path + 1]        ; the kind by the device
        mov     esi, disk_icons - 3
@@:
        add     esi, 3
        cmp     word [esi], 0
        je      @f
        cmp     [esi], ax
        jne     @b

@@:
        movzx   eax, byte [esi + 2]
        mov     [ebx + DISK.icon], eax

        mov     esi, walk_path
        lea     edi, [ebx + DISK.name]
        mov     ecx, DISK.label - DISK.name
        call    copy_cut
        mov     esi, fs_buf + FileInfoA.FileName  ; of the partition
        lea     edi, [ebx + DISK.label]
        mov     ecx, sizeof.DISK - DISK.label
        call    copy_cut

        mov     eax, [fs_buf + FileInfoA.FileSizeLow]
        mov     edx, [fs_buf + FileInfoA.FileSizeHigh]
        shrd    eax, edx, 10
        shr     edx, 10                 ; 4 TB and more: unknown
        jz      @f
        or      eax, -1
@@:
        mov     [ebx + DISK.total], eax
        or      dword [ebx + DISK.used], -1
        or      dword [ebx + DISK.fill], -1

        inc     [disk_n]                ; listed when it is all there
        mov     [walk_new], ebx
        test    eax, eax
        jz      .done

        mov     eax, dword [walk_path + 1]
        cmp     ax, "rd"
        je      @f
        and     eax, 0xFFFFFF
        cmp     eax, "tmp"
        jne     .done

@@:
        xor     ecx, ecx
        xor     edx, edx
        mov     edi, [esp + PUSHA_EDI]  ; the end of the path
        call    disk_walk

        cmp     ecx, [ebx + DISK.total] ; not more used than there is
        jbe     @f
        mov     ecx, [ebx + DISK.total]

@@:
        mov     [ebx + DISK.used], ecx
        xchg    eax, ecx
        mov     edx, 1024
        mul     edx
        div     dword [ebx + DISK.total]
        mov     [ebx + DISK.fill], eax
        mov     [walk_new], ebx

.done:
        popa
        ret

; esi -> text, edi -> a field of ecx bytes: the text, cut to fit it, not in
; the middle of a character
copy_cut:
        rep movsb

@@:
        dec     edi
        mov     al, [edi]
        and     al, 0xC0
        cmp     al, 0x80
        je      @b
        mov     [edi], cl
        ret

; The thread.
walker:
        call    disks_find

.stop:
        and     [walking], 0
        mcall   SF_TERMINATE_PROCESS

; Ask the walker to stop and wait till it is gone.
stop_walker:
        mov     [walk_stop], 1

@@:
        and     [walk_new], 0
        cmp     [walking], 0
        je      @f
        mcall   SF_SLEEP, 1
        jmp     @b

@@:
        ret

; Add the size of files under [walk_path] to ecx, KB; edi -> end of the path,
; edx = depth
disk_walk:
        pusha
        xor     ebp, ebp                ; index in the folder
        inc     edx

.next:
        call    read_entry
        jnz     .done

        test    byte [fs_entry.Attributes], FA_LABEL
        jnz     .next
        test    byte [fs_entry.Attributes], FA_FOLDER
        jnz     .folder

        mov     eax, [fs_entry.FileSizeLow]
        add     eax, 1023
        shr     eax, 10
        add     ecx, eax
        jmp     .next

.folder:
        cmp     byte [esi], "."         ; skip "." and ".."
        jne     @f
        cmp     byte [esi + 1], 0
        je      .next
        cmp     word [esi + 1], "."
        je      .next

@@:
        cmp     edx, WALK_DEPTH
        jae     .next

        lea     eax, [edi + 264]
        cmp     eax, walk_path + WALK_MAX
        jae     .next

        push    edi
        mov     byte [edi], "/"
        inc     edi
        call    fmt_str
        call    disk_walk
        pop     edi
        mov     byte [edi], 0
        jmp     .next

.done:
        mov     [esp + PUSHA_ECX], ecx
        popa
        ret

disks_bar:
        BAR     BAR_BUTTON, sz_open, sc.work_button
        db      BAR_END

disks_list      LIST    disks_cols, disks_cols_end, disks_cols + sizeof.COL, nothing, disks_open, disks_keys, 0, 0, 0, 0, 0, 0, tab_data
disks_keys:
        HOTKEY  KEY_ENTER, disks_open
        db      0
disks_cols:
        COL     sz_col_num, DISK.icon, KIND_ICON, ID_W
        COL     sz_col_name, DISK.name, KIND_TEXT, COL_W
        COL     sz_col_label, DISK.label, KIND_TEXT, COL_W
        COL     sz_pct, DISK.fill, KIND_BAR, 0
        COL     sz_used, DISK.used, KIND_SIZE, COL_W
        COL     sz_col_size, DISK.total, KIND_SIZE, COL_W
disks_cols_end:

if lang eq ru_RU
sz_col_size     db      "Размер", 0
sz_col_label    db      "Метка", 0
sz_open         db      "Открыть", 0
else if lang eq es_ES
sz_col_size     db      "Tamaño", 0
sz_col_label    db      "Etiqueta", 0
sz_open         db      "Abrir", 0
else
sz_col_size     db      "Size", 0
sz_col_label    db      "Label", 0
sz_open         db      "Open", 0
end if

sz_eolite_path  db      "/sys/File Managers/EOLITE", 0

; the first two letters of the device, the icon; the last one for the others
disk_icons      db      "rd", 58, "tm", 51, "fd", 5, "cd", 19, "us", 57, 0, 0, 50

fs_read         FileInfoBlock SSF_READ_FOLDER, 0, 1, 1, fs_buf, 0, walk_path   ; cp866
                                        ; names, one entry from Position on
fs_info         FileInfoBlock SSF_GET_INFO, 0, 3, 0, fs_buf, 0, walk_path      ; UTF-8
                                        ; volume label

uglobal
disk_n          dd      ?               ; the disks the walker found
walking         dd      ?
walk_stop       dd      ?
walk_new        dd      ?
                rb      1024
walk_stack_top:
endg

virtual at tab_data                     ; only the open tab uses it
disks_view      rd      MAX_DISKS
disks           rb      MAX_DISKS * sizeof.DISK
fs_buf          FileInfoHeader          ; then an entry of a folder; a
fs_entry        FileInfoA               ; partition from fs_buf on
walk_path       rb      WALK_MAX
DISKS_DATA = $ - tab_data
end virtual
