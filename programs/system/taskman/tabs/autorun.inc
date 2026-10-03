; Autorun tab: the programs /sys/LAUNCHER starts from /sys/settings/autorun.dat.
; A line is: program parameters delay; "text" may hold spaces, # starts
; a comment. A program after # is turned off: the button or Space turns it
; on or off in the file.

struct AR
        prog            dd ?
        params          dd ?
        delay           dd ?
        num             dd ?            ; of the entry
        off             db ?            ; 1: after #, turned off
ends

; Read the file, its lines into the list.
autorun_load:
        mcall   SF_FILE, autorun_info   ; its size; read a byte more to see
        mov     ecx, [autorun_bdfe.FileSizeLow] ; its end
        test    eax, eax
        jz      @f
        xor     ecx, ecx

@@:
        inc     ecx
        mov     [autorun_file.Count], ecx
        inc     ecx                     ; a # more, then 0
        inc     ecx
        mcall   SF_SYS_MISC, SSF_MEM_REALLOC, , [autorun_file.Buffer]
        mov     [autorun_file.Buffer], eax
        mov     [autorun_save.Buffer], eax
        mov     ebp, [list.view]
        test    eax, eax
        jz      list_done

        mcall   SF_FILE, autorun_file
        cmp     eax, 6                  ; the end of the file: all of it
        je      @f
        xor     ebx, ebx

@@:
        mov     esi, [autorun_file.Buffer]
        mov     byte [esi + ebx], 0
        add     ebx, esi

        mov     edi, esi                ; the rows: at most the bytes that
        xor     edx, edx                ; may end a line, 0..13, and one

.count:
        inc     edx
@@:
        cmp     edi, ebx
        jae     @f
        cmp     byte [edi], 14
        inc     edi
        jb      .count
        jmp     @b

@@:
        imul    ecx, edx, 4 + sizeof.AR ; the view, then the rows
        push    ebx edx
        mcall   SF_SYS_MISC, SSF_MEM_REALLOC, , [list.view]
        pop     edx ebx
        mov     [list.view], eax
        mov     ebp, eax
        lea     edi, [eax + edx*4]
        test    eax, eax
        jz      list_done

.line:
        cmp     esi, ebx
        jae     list_done

        push    esi
@@:
        lodsb                           ; find the end of the line
        cmp     al, 13
        je      @f
        cmp     al, 10
        je      @f
        test    al, al
        jnz     @b

@@:
        mov     byte [esi - 1], 0
        xchg    esi, [esp]

        call    get_token               ; the row goes to edi, its pointer to ebp
        cmp     byte [eax], 0           ; empty line
        je      .next
        cmp     word [eax], "#/"        ; a program turned off
        je      @f
        cmp     byte [eax], "#"         ; comment
        je      .next

@@:
        mov     dl, [eax]
        mov     [ebp], edi
        add     ebp, 4
        stosd                           ; AR.prog, params, delay

        call    get_token
        stosd
        call    get_token
        stosd

        mov     eax, ebp
        sub     eax, [list.view]
        shr     eax, 2
        stosd
        cmp     dl, "#"                 ; AR.off
        sete    al
        stosb

.next:
        pop     esi
        jmp     .line

; Put # before the selected program or take it away and save the file.
autorun_toggle:
        cmp     [list.count], 0         ; no rows, maybe no buffer
        je      .done
        mcall   SF_FILE, autorun_file   ; the file again, as it is
        cmp     eax, 6                  ; all of it
        jne     .done

        mov     edi, [list.key]         ; AR.prog: the line in the buffer
        mov     ecx, [autorun_file.Buffer]
        add     ecx, ebx
        sub     ecx, edi                ; not past the end: the file was cut
        jbe     .done

        cmp     byte [edi], "#"
        je      .on

        lea     esi, [edi + ecx - 1]    ; one byte on from the end
        lea     edi, [esi + 1]
        std
        rep movsb
        cld

        mov     byte [edi], "#"
        inc     ebx
        jmp     .save

.on:
        lea     esi, [edi + 1]
        dec     ecx
        rep movsb
        dec     ebx

.save:
        mov     [autorun_save.Count], ebx
        mcall   SF_FILE, autorun_save
        call    autorun_load
        jmp     list_rows

.done:
        ret

; esi -> text; out: eax -> the next word or "text", cut with zero; esi after it
get_token:
        mov     eax, esi
        mov     dh, [esi]
        test    dh, dh
        jz      .done
        inc     esi
        cmp     dh, " "                 ; skip blanks
        jbe     get_token

        mov     cx, " " shl 8           ; a word ends with a blank
        cmp     dh, '"'
        jne     .next
        mov     cx, '"'                 ; "text" with the quote
        mov     eax, esi

.next:
        mov     dh, [esi]
        test    dh, dh
        jz      .done
        inc     esi
        sub     dh, cl                  ; cl..cl + ch ends it
        cmp     dh, ch
        ja      .next

        mov     byte [esi - 1], 0
.done:
        ret

autorun_bar:
        BAR     BAR_BUTTON, sz_ar_toggle, sc.work_button
        db      BAR_END

autorun_list    LIST    autorun_cols, autorun_cols_end, autorun_cols, nothing, nothing, autorun_keys, AR.off, 0, 0, 0, 0, 0, 0   ; the view on the heap
autorun_keys:
        HOTKEY  KEY_SPACE, autorun_toggle
        db      0
autorun_cols:
        COL     sz_col_num, AR.num, KIND_NUM, ID_W
        COL     sz_col_program, AR.prog, KIND_STR, 0
        COL     sz_col_params, AR.params, KIND_STR, COL_W
        COL     sz_col_delay, AR.delay, KIND_STR, COL_W
autorun_cols_end:

if lang eq ru_RU
sz_tab_autorun  db      "Автозагрузка", 0
sz_col_program  db      "Программа", 0
sz_col_params   db      "Параметры", 0
sz_col_delay    db      "Задержка", 0
sz_ar_toggle    db      "Включить/выключить", 0
else if lang eq es_ES
sz_tab_autorun  db      "Inicio", 0
sz_col_program  db      "Programa", 0
sz_col_params   db      "Parámetros", 0
sz_col_delay    db      "Retardo", 0
sz_ar_toggle    db      "Activar/desactivar", 0
else
sz_tab_autorun  db      "Autorun", 0
sz_col_program  db      "Program", 0
sz_col_params   db      "Parameters", 0
sz_col_delay    db      "Delay", 0
sz_ar_toggle    db      "Turn on/off", 0
end if

autorun_info    FileInfoBlock SSF_GET_INFO, 0, 0, 0, autorun_bdfe, 0, sz_autorun
autorun_file    FileInfoBlock SSF_READ_FILE, 0, 0, 0, 0, 0, sz_autorun  ; the buffer on
autorun_save    FileInfoBlock SSF_CREATE_FILE, 0, 0, 0, 0, 0, sz_autorun        ; the heap
sz_autorun      db      "/sys/settings/autorun.dat", 0

virtual at tab_data                     ; only the open tab uses it
autorun_bdfe    FileInfoA
AUTORUN_DATA = $ - tab_data
end virtual
