; Drivers tab: the drivers loaded by the kernel (function 68.31).

struct DRV
        num             dd ?            ; in the list, the key of the row
        name            rb 16           ; from here on the item of the kernel
        next            dd ?
        handle          dd ?            ; the kernel keeps the previous item here
        base            dd ?
        start           dd ?            ; the entry of the driver
        service         dd ?
ends

MAX_DRIVERS     = 64

; The drivers the kernel has now.
drivers_scan:
        mcall   SF_SYS_MISC, SSF_GET_DRIVER_DATA, 1
        mov     esi, ecx                ; the last driver
        mov     edx, ebx                ; the first one

        mov     ebp, drivers_view
        mov     edi, drivers + DRV.name

.next:
        cmp     ebp, drivers_view + MAX_DRIVERS*4
        jae     list_done

        mcall   SF_SYS_MISC, SSF_GET_DRIVER_DATA, 2
        test    eax, eax
        jnz     list_done

        lea     eax, [edi - DRV.name]
        mov     [ebp], eax
        add     ebp, 4

        lea     eax, [ebp - drivers_view]
        shr     eax, 2
        mov     dword [edi + DRV.num - DRV.name], eax

        mov     eax, dword [edi + DRV.next - DRV.name]
        mov     dword [edi + DRV.handle - DRV.name], edx

        add     edi, sizeof.DRV
        cmp     edx, esi
        mov     edx, eax
        jne     .next
        jmp     list_done

drivers_load:                           ; RUN with the name of the driver to type
        mov     esi, sz_run_path
        mov     edx, sz_loaddrv
        jmp     run_app

drivers_bar:
        BAR     BAR_BUTTON, sz_load, sc.work_button
        db      BAR_END

drivers_list    LIST    drivers_cols, drivers_cols_end, drivers_cols, nothing, nothing, no_keys, 0, 0, 0, 0, 0, 0, tab_data
drivers_cols:
        COL     sz_col_num, DRV.num, KIND_NUM, ID_W
        COL     sz_col_name, DRV.name, KIND_TEXT, 0
        COL     sz_col_handle, DRV.handle, KIND_HEX, COL_W
        COL     sz_col_base, DRV.base, KIND_HEX, COL_W
        COL     sz_col_service, DRV.service, KIND_HEX, COL_W
drivers_cols_end:

if lang eq ru_RU
sz_tab_drivers  db      "Драйверы", 0
sz_col_handle   db      "Дескриптор", 0
sz_col_base     db      "База", 0
sz_col_service  db      "Сервис", 0
sz_load         db      "Загрузить...", 0
else if lang eq es_ES
sz_tab_drivers  db      "Controladores", 0
sz_col_handle   db      "Manejador", 0
sz_col_base     db      "Base", 0
sz_col_service  db      "Servicio", 0
sz_load         db      "Cargar...", 0
else
sz_tab_drivers  db      "Drivers", 0
sz_col_handle   db      "Handle", 0
sz_col_base     db      "Base", 0
sz_col_service  db      "Service", 0
sz_load         db      "Load...", 0
end if
sz_loaddrv      db      "/sys/LOADDRV ", 0

virtual at tab_data                     ; only the open tab uses it
drivers_view    rd      MAX_DRIVERS
drivers         rb      MAX_DRIVERS * sizeof.DRV
DRIVERS_DATA = $ - tab_data
end virtual
