; SPDX-License-Identifier: NOASSERTION
;
; RDsave - save the RAM-disk back to a real medium
;
; Modes:
;   /sys/rdsave          window
;   /sys/rdsave -h       silent: save to the path from the ini,
;                        report via /sys/@notify ("h" also accepted
;                        for compatibility with 1.x callers)
;   /sys/rdsave <path>   silent: save to <path>, report via /sys/@notify
;
; The default target is the boot medium (SF 18.6 with ecx = 0) when
; the system was booted from a floppy or a hard disk; a file picked in
; the dialog overrides it. In the path form, "/fd/1" or "/fd/2"
; address the floppy device (SF 16), anything else is a file (SF 18.6).
;
; Authors: Mario79, Heavyiron, Lrz, Burer

appname equ 'RDsave '
version equ '2.0'

use32
org 0

db 'MENUET01'
dd 1
dd START
dd IM_END
dd I_END
dd stacktop
dd PARAMS
dd cur_dir_path

include '../../macros.inc'
include '../../proc32.inc'
include '../../dll.inc'
include '../../KOSfuncs.inc'
include '../../encoding.inc'
include '../../string.inc'

; do_save result codes; 1..11 match the file system error codes
SAVE_OK          = 0
SAVE_ERR_DEV     = 11   ; floppy device write failed
SAVE_ERR_NOPATH  = 12   ; no target path selected
SAVE_ERR_ITSELF  = 13   ; target is on the RAM-disk
SAVE_ERR_UNKNOWN = 14

BTN_SAVE   = 2
BTN_SELECT = 3

TEXT_CP866 = 0x90000000 ; fn 4: zero-terminated, 8x16 font, CP866

WIN_W    = 420
WIN_CH   = 116  ; client area height
LABEL_X  = 10
VALUE_X  = 160
BTN_Y    = 52
STATUS_Y = 90

;---------------------------------------------------------------------

START:
        mcall   SF_SYS_MISC, SSF_HEAP_INIT
        stdcall dll.Load, importLib
        test    eax, eax
        jnz     exit

        mcall   SF_SYSTEM_GET, SSF_RD_BOOT_SOURCE
        mov     [boot_src], eax

        invoke  ini_get_int, ini_file, ini_sec, key_autoclose, 1
        mov     [autoclose], eax
        invoke  ini_get_str, ini_file, ini_sec, key_path, fname_buf, 4096, path_default

        mov     esi, PARAMS
        mov     al, [esi]
        test    al, al
        jz      gui_mode
        cmp     al, '/'
        je      path_mode
        cmp     word [esi], 'h'         ; bare "h": 1.x compatibility
        je      silent_save
        cmp     word [esi], 'H'
        je      silent_save
        cmp     al, '-'
        jne     gui_mode
        cmp     byte [esi+2], 0
        jne     gui_mode
        mov     al, [esi+1]
        or      al, 0x20
        cmp     al, 'h'
        je      silent_save
        jmp     gui_mode

;---------------------------------------------------------------------

; ZF set when fname_buf addresses the floppy device ("/fd/...")
check_floppy_prefix:
        mov     eax, dword [fname_buf]
        or      eax, 0x00202000         ; the path is matched case-insensitively
        cmp     eax, '/fd/'
        ret

; save the RAM-disk to the boot medium or to the path in fname_buf
; out: eax = SAVE_* code
do_save:
        cmp     [use_boot], 0
        je      .by_path
        mcall   SF_SYSTEM, SSF_RD_TO_HDD, 0
        jmp     .clamp

        .by_path:
        cmp     byte [fname_buf], 0
        jne     @f
        mov     eax, SAVE_ERR_NOPATH
        ret

        @@:
        mov     eax, dword [fname_buf]
        or      eax, 0x00202000
        and     eax, 0x00FFFFFF
        cmp     eax, '/rd'
        jne     .not_rd
        mov     al, [fname_buf+3]
        test    al, al
        jz      .itself
        cmp     al, '/'
        jne     .not_rd

        .itself:
        mov     eax, SAVE_ERR_ITSELF
        ret

        .not_rd:
        call    check_floppy_prefix
        jne     .file
        mov     al, [fname_buf+4]
        cmp     al, '1'
        je      .flp1
        cmp     al, '2'
        jne     .file
        mov     ebx, 2
        jmp     .flp_write

        .flp1:
        mov     ebx, 1

        .flp_write:
        mcall   SF_RD_TO_FLOPPY
        test    eax, eax
        jz      .done
        mov     eax, SAVE_ERR_DEV

        .done:
        ret

        .file:
        mcall   SF_SYSTEM, SSF_RD_TO_HDD, fname_buf

        .clamp:
        cmp     eax, 11
        jbe     .done
        mov     eax, SAVE_ERR_UNKNOWN
        ret

;---------------------------------------------------------------------

path_mode:
        stdcall string.copy, PARAMS, fname_buf

silent_save:
        call    do_save
        stdcall string.copy, ntf_head, ntf_msg  ; string.* preserve eax
        test    eax, eax
        jnz     .err

        stdcall string.concatenate, msg_ok, ntf_msg
        stdcall string.concatenate, fname_buf, ntf_msg
        mov     edx, ntf_end_ok
        jmp     .send

        .err:
        mov     esi, [err_msgs+eax*4]
        stdcall string.concatenate, err_prefix, ntf_msg
        stdcall string.concatenate, esi, ntf_msg
        mov     edx, ntf_end_err

        .send:
        stdcall string.concatenate, edx, ntf_msg
        mcall   SF_FILE, notify_finfo

exit:
        mcall   SF_TERMINATE_PROCESS

;---------------------------------------------------------------------

gui_mode:
        cmp     [boot_src], RD_LOAD_FROM_FLOPPY
        je      .boot_target
        cmp     [boot_src], RD_LOAD_FROM_HD
        jne     @f

        .boot_target:
        mov     [use_boot], 1

        @@:
        stdcall string.copy, def_fname, filename_area
        push    dword OpenDialog_data
        call    [OpenDialog_Init]

red:
        call    draw_window

still:
        mcall   SF_WAIT_EVENT
        dec     eax
        jz      red
        dec     eax
        jz      .key

        ; button event
        mcall   SF_GET_BUTTON
        shr     eax, 8
        cmp     eax, 1
        je      exit
        cmp     eax, BTN_SAVE
        je      gui_save
        cmp     eax, BTN_SELECT
        je      gui_select
        jmp     still

        .key:
        mcall   SF_GET_KEY
        cmp     ah, 27                  ; Esc
        je      exit
        cmp     ah, 13                  ; Enter
        je      gui_save
        jmp     still

gui_select:
        push    dword OpenDialog_data
        call    [OpenDialog_Start]
        cmp     [OpenDialog_data.status], 1
        jne     red
        stdcall string.copy, dialog_path, fname_buf
        mov     [use_boot], 0
        jmp     red

gui_save:
        cmp     [use_boot], 0
        jne     @f
        cmp     byte [fname_buf], 0
        je      gui_select              ; nowhere to save yet - ask for a path

        @@:
        mov     esi, msg_saving
        mov     ecx, [sc.work_text]
        or      ecx, TEXT_CP866
        call    show_status
        call    do_save
        test    eax, eax
        jz      .ok

        mov     esi, [err_msgs+eax*4]
        mov     ecx, TEXT_CP866 or 0x00AA0000
        call    show_status
        jmp     still

        .ok:
        cmp     [use_boot], 0           ; only file targets are remembered
        jne     @f
        call    check_floppy_prefix
        je      @f
        stdcall string.length, fname_buf
        invoke  ini_set_str, ini_file, ini_sec, key_path, fname_buf, eax

        @@:
        mov     esi, msg_ok
        mov     ecx, TEXT_CP866 or 0x0000AA00
        call    show_status
        cmp     [autoclose], 1
        jne     still
        mcall   SF_SLEEP, 100
        jmp     exit

;---------------------------------------------------------------------

; in: esi = text, ecx = color with the charset flags
show_status:
        push    ecx esi
        mcall   SF_DRAW_RECT, <8, WIN_W-16>, <STATUS_Y-2, 18>, [sc.work]
        pop     edx ecx
        mcall   SF_DRAW_TEXT, <LABEL_X, STATUS_Y>
        ret

; choose the target description for the window -> edx (clobbers eax)
pick_target_text:
        mov     edx, tgt_boot
        cmp     [use_boot], 0
        jne     .done

        mov     edx, tgt_none
        cmp     byte [fname_buf], 0
        je      .done

        mov     edx, tgt_floppy
        call    check_floppy_prefix
        je      .done

        mov     edx, fname_buf          ; fn 4 clips at the window edge

        .done:
        ret

;---------------------------------------------------------------------

draw_window:
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors
        mcall   SF_REDRAW, SSF_BEGIN_DRAW

        mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
        mov     ecx, 200*65536 + WIN_CH
        add     ecx, eax
        mov     edx, [sc.work]
        or      edx, 0x34000000
        mcall   SF_CREATE_WINDOW, <200, WIN_W>, , , , title

        mov     ecx, [sc.work_text]
        or      ecx, TEXT_CP866
        mcall   SF_DRAW_TEXT, <LABEL_X, 10>, , lab_boot
        mov     eax, [boot_src]
        cmp     eax, 5
        jbe     @f
        xor     eax, eax

        @@:
        mov     edx, [boot_names+eax*4]
        mcall   SF_DRAW_TEXT, <VALUE_X, 10>
        mcall   SF_DRAW_TEXT, <LABEL_X, 30>, , lab_target
        call    pick_target_text
        mcall   SF_DRAW_TEXT, <VALUE_X, 30>

        mcall   SF_DEFINE_BUTTON, <10, 110>, <BTN_Y, 24>, BTN_SAVE, [sc.work_button]
        mcall   SF_DEFINE_BUTTON, <130, 140>, <BTN_Y, 24>, BTN_SELECT, [sc.work_button]
        stdcall draw_btn_label, btn_save_t, 10, 110
        stdcall draw_btn_label, btn_select_t, 130, 140

        mcall   SF_REDRAW, SSF_END_DRAW
        ret

proc draw_btn_label txt:dword, x:dword, w:dword
        stdcall string.length, [txt]    ; CP866: one byte per character
        shl     eax, 3                  ; text width, 8 px per character
        mov     ebx, [w]
        sub     ebx, eax
        sar     ebx, 1
        add     ebx, [x]
        shl     ebx, 16
        add     ebx, BTN_Y+5
        mov     ecx, [sc.work_button_text]
        or      ecx, TEXT_CP866
        mcall   SF_DRAW_TEXT, , , [txt]
        ret
endp

;---------------------------------------------------------------------
;---  Data  ----------------------------------------------------------
;---------------------------------------------------------------------

if lang eq ru_RU
        lab_boot     cp866 'Источник загрузки:',0
        lab_target   cp866 'Сохранить в:',0
        src_unknown  cp866 'неизвестно',0
        src_floppy   cp866 'дискета A',0
        src_hd       cp866 'жёсткий диск',0
        src_memory   cp866 'образ в памяти',0
        src_format   cp866 'пустой рамдиск',0
        src_none     cp866 'нет рамдиска',0
        tgt_boot     cp866 'загрузочный носитель',0
        tgt_floppy   cp866 'дискета A',0
        tgt_none     cp866 'путь не выбран',0
        btn_save_t   cp866 'Сохранить',0
        btn_select_t cp866 'Выбрать файл...',0
        msg_saving   cp866 'Сохранение образа...',0
        msg_ok       cp866 'Образ сохранён успешно ',0
        err_prefix   cp866 'Ошибка: ',0
        e1           cp866 'не определена база и/или раздел жёсткого диска',0
        e2           cp866 'функция не поддерживается данной файловой системой',0
        e3           cp866 'неизвестная файловая система',0
        e4           cp866 'зарезервировано (код 4)',0
        e5           cp866 'неверный путь',0
        e6           cp866 'файл закончился',0
        e7           cp866 'указатель вне памяти приложения',0
        e8           cp866 'диск переполнен',0
        e9           cp866 'файловая структура разрушена',0
        e10          cp866 'доступ запрещён',0
        e11          cp866 'ошибка устройства',0
        eNoPath      cp866 'путь не выбран',0
        eItself      cp866 'нельзя сохранять образ на самого себя',0
        eUnknown     cp866 'неизвестная ошибка',0
else if lang eq es_ES
        lab_boot     cp850 'Origen de arranque:',0
        lab_target   cp850 'Guardar en:',0
        src_unknown  cp850 'desconocido',0
        src_floppy   cp850 'disquete A',0
        src_hd       cp850 'disco duro',0
        src_memory   cp850 'imagen en memoria',0
        src_format   cp850 'ramdisk vacío',0
        src_none     cp850 'sin ramdisk',0
        tgt_boot     cp850 'medio de arranque',0
        tgt_floppy   cp850 'disquete A',0
        tgt_none     cp850 'ruta no seleccionada',0
        btn_save_t   cp850 'Guardar',0
        btn_select_t cp850 'Elegir archivo...',0
        msg_saving   cp850 'Guardando la imagen...',0
        msg_ok       cp850 'Imagen guardada con éxito ',0
        err_prefix   cp850 'Error: ',0
        e1           cp850 'base y/o partición del disco duro no definida',0
        e2           cp850 'el sistema de archivos no soporta esta función',0
        e3           cp850 'sistema de archivos desconocido',0
        e4           cp850 'reservado (código 4)',0
        e5           cp850 'ruta incorrecta',0
        e6           cp850 'fin de archivo',0
        e7           cp850 'el puntero está fuera de la memoria de la aplicación',0
        e8           cp850 'disco lleno',0
        e9           cp850 'estructura de archivos dañada',0
        e10          cp850 'acceso denegado',0
        e11          cp850 'error de dispositivo',0
        eNoPath      cp850 'ruta no seleccionada',0
        eItself      cp850 'no se puede guardar la imagen sobre sí misma',0
        eUnknown     cp850 'error desconocido',0
else ; Default to en_US
        lab_boot     db 'Boot source:',0
        lab_target   db 'Save to:',0
        src_unknown  db 'unknown',0
        src_floppy   db 'floppy A',0
        src_hd       db 'hard disk',0
        src_memory   db 'image in memory',0
        src_format   db 'empty ramdisk',0
        src_none     db 'no ramdisk',0
        tgt_boot     db 'boot medium',0
        tgt_floppy   db 'floppy A',0
        tgt_none     db 'path not selected',0
        btn_save_t   db 'Save',0
        btn_select_t db 'Select file...',0
        msg_saving   db 'Saving the image...',0
        msg_ok       db 'Image saved successfully ',0
        err_prefix   db 'Error: ',0
        e1           db 'hard disk base and/or partition not defined',0
        e2           db 'the file system does not support this function',0
        e3           db 'unknown file system',0
        e4           db 'reserved (code 4)',0
        e5           db 'incorrect path',0
        e6           db 'end of file',0
        e7           db 'pointer is outside of application memory',0
        e8           db 'disk is full',0
        e9           db 'file structure is destroyed',0
        e10          db 'access denied',0
        e11          db 'device error',0
        eNoPath      db 'path not selected',0
        eItself      db "you can't save the image onto itself",0
        eUnknown     db 'unknown error',0
end if

title db appname, version, 0

;---------------------------------------------------------------------

align 4
err_msgs:
        dd msg_ok, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11
        dd eNoPath, eItself, eUnknown

boot_names:
        dd src_unknown, src_floppy, src_hd, src_memory, src_format, src_none

ini_file      db '/sys/settings/app.ini',0
ini_sec       db 'RDSave',0
key_path      db 'path',0
key_autoclose db 'autoclose',0
path_default  db 0
def_fname     db 'kolibri.img',0

ntf_head      db '"RDsave\n',0
ntf_end_ok    db '" -tO',0
ntf_end_err   db '" -tE',0

notify_finfo:
        dd 7, 0, ntf_msg, 0, 0
        db '/sys/@notify',0

;---------------------------------------------------------------------

align 4
importLib:
library libini, 'libini.obj', proclib, 'proc_lib.obj'

import  libini, \
        ini_get_str, 'ini_get_str', \
        ini_get_int, 'ini_get_int', \
        ini_set_str, 'ini_set_str'

import  proclib, \
        OpenDialog_Init,  'OpenDialog_init', \
        OpenDialog_Start, 'OpenDialog_start'

;---------------------------------------------------------------------

OpenDialog_data:
        .type             dd 1                  ; Save
        .procinfo         dd procinfo
        .com_area_name    dd communication_area_name
        .com_area         dd 0
        .opendir_path     dd temp_dir_path
        .dir_default_path dd communication_area_default_path
        .start_path       dd open_dialog_path
        .draw_window      dd draw_window
        .status           dd 0
        .openfile_path    dd dialog_path        ; kept empty of the ini path so
                                                ; a stale folder never opens
        .filename_area    dd filename_area
        .filter_area      dd Filter
        .x:
        .x_size           dw 420
        .x_start          dw 200
        .y:
        .y_size           dw 320
        .y_start          dw 120

communication_area_name db 'FFFFFFFF_open_dialog',0

open_dialog_path:
if __nightbuild eq yes
        db '/sys/MANAGERS/opendial',0
else
        db '/sys/File Managers/opendial',0
end if

communication_area_default_path db '/',0

Filter:
        dd Filter.end - Filter
        db 'IMG',0
        db 'IMA',0
        .end:
        db 0

;---------------------------------------------------------------------

IM_END:

align 4
boot_src  rd 1
autoclose rd 1
use_boot  rd 1

sc system_colors

PARAMS        rb 256
fname_buf     rb 4096
dialog_path   rb 4096
ntf_msg       rb 1152
cur_dir_path  rb 4096
temp_dir_path rb 4096
filename_area rb 256
procinfo      rb 1024

;---------------------------------------------------------------------

align 32
        rb 4096
stacktop:
I_END:
