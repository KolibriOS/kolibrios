; Processes tab: the threads in a sortable list.

; The threads to show, then the whole list or the rows only.
procs_draw:
        call    procs_filter
        jmp     list_draw

procs_update:
        call    procs_filter
        jmp     list_rows

; The threads to show, without the system ones unless asked.
procs_filter:
        mov     esi, procs
        mov     ebp, procs_view

.next:
        cmp     esi, [procs_end]
        jae     list_done

        cmp     [show_system], 0
        jne     .add

        cmp     esi, procs              ; IDLE
        je      .skip
        cmp     byte [esi + PROC.name], "@"
        je      .skip
        cmp     dword [esi + PROC.name], "OS"
        je      .skip

.add:
        mov     [ebp], esi
        add     ebp, 4

.skip:
        add     esi, sizeof.PROC
        jmp     .next

procs_button:                           ; edx = button id
        jmp     [.btns + (edx - BTN_OWN)*4]
.btns   dd      .system, procs_info, procs_kill, procs_run     ; as in procs_bar

.system:
        xor     [show_system], 1
        jmp     draw_window

; The buttons: Run..., End, Details.
procs_run:
        mov     esi, sz_run_path
        jmp     run_plain

procs_kill:                             ; the key of a row is its PID
        mcall   SF_SYSTEM, SSF_TERMINATE_THREAD_ID, [list.key]
        ret

procs_info:
        mcall   SF_SYSTEM, SSF_GET_THREAD_SLOT, [list.key]
        test    eax, eax
        jz      .done

        mov     edi, text_buf
        call    fmt_uint

        mov     esi, sz_tinfo_path
        mov     edx, text_buf
        jmp     run_app

.done:
        ret

procs_bar:
        BAR     BAR_CHECK, sz_system, show_system
        BAR     BAR_BUTTON, sz_info, sc.work_button
        BAR     BAR_BUTTON, sz_kill, red
        BAR     BAR_GAP, 0, 0                   ; away from End
        BAR     BAR_BUTTON, sz_run, green
        db      BAR_END

procs_list      LIST    procs_cols, procs_cols_end, procs_cols, nothing, procs_info, procs_keys, 0, 0, 0, 0, 0, 0, tab_data
procs_keys:
        HOTKEY  KEY_ENTER, procs_info
        HOTKEY  KEY_DEL, procs_kill
        db      0
procs_cols:
        COL     sz_col_id, PROC.pid, KIND_NUM, ID_W
        COL     sz_col_name, PROC.name, KIND_TEXT, 0
        COL     sz_col_cpu, PROC.cpu, KIND_PCT, COL_W
        COL     sz_col_mem, PROC.mem, KIND_SIZE, COL_W
procs_cols_end:

uglobal
show_system     db      ?
endg

virtual at tab_data                     ; only the open tab uses it
procs_view      rd      MAX_PROCS
PROCS_DATA = $ - tab_data
end virtual
