; SPDX-License-Identifier: NOASSERTION
;
; DocPack - KolibriOS documentation
; Copyright (C) 2006-2026 KolibriOS team
;
; Authors: Ivushkin Andrey aka Willow, Burer
;
; DOCPACK    window with the documents and the online documentation
; DOCPACK x  open the document x (a..i) in CEdit or the link (j, k) in WebView

; ====================================================================

include "../../macros.inc"
include "../../KOSfuncs.inc"
include "../../encoding.inc"

format meos 01
entry START

; ====================================================================

CHAR_W   = 8
CAPS_Y   = 2                            ; the capitals are the rows 2..11 of the font
CAPS_H   = 10
GAP      = 12
BTN_H    = 24
TEXT_DY  = (BTN_H - CAPS_H)/2 - CAPS_Y  ; the capitals centred in a button
TITLE_DY = CAPS_Y + CAPS_H/2            ; the frame crosses the capitals of the title
TITLE_ABOVE = TITLE_DY - CAPS_Y
TITLE_BELOW = CAPS_Y + CAPS_H - TITLE_DY
HOTKEY_W = 4*CHAR_W                     ; "A | "
BTN_W    = HOTKEY_W + LABEL_CHARS*CHAR_W + GAP*2
COLUMNS  = 2
ROWS     = 5
GROUP_Y  = GAP + TITLE_ABOVE            ; GAP to the capitals of the title
GROUP_W  = 1 + GAP + BTN_W + GAP + 1
GROUP_H  = TITLE_BELOW + GAP + ROWS*(BTN_H + GAP) + 1
BTN_X    = GAP + 1 + GAP                ; of the first group
BTN_Y    = GROUP_Y + TITLE_BELOW + GAP  ; GAP below the capitals of the title
LINKS_Y  = GROUP_Y + GROUP_H + GAP + TITLE_ABOVE    ; the group of the links, full width
LINKS_H  = TITLE_BELOW + GAP + BTN_H + GAP + 1
LINK_Y   = LINKS_Y + TITLE_BELOW + GAP
LINKS    = 2
DOC_ID   = 10
CLIENT_W = GAP + COLUMNS*(GROUP_W + GAP)
CLIENT_H = LINKS_Y + LINKS_H + GAP
WINDOW_W = CLIENT_W + 9                 ; the borders of a skinned window
WINDOW_H = CLIENT_H + 4                 ; and the skin height

WINDOW_STYLE = 0x34000000               ; skinned, fixed size, caption, client coordinates
FONT     = 0xB0000000                   ; zero terminated, 8x16 UTF-8
FILL     = 0x40000000                   ; draw the background of the text

SEND_TRIES = 20
SEND_DELAY = 10                         ; CEdit waits 2 s for the text

FILE_COUNT = 0

macro embed_file path
{
        local   label, label2
        dd      label2 - label
label:
        file    path
label2:
        FILE_COUNT = FILE_COUNT + 1
}

macro embed_doc name
{
if lang eq ru_RU
        embed_file "../../../data/ru_RU/docs/" # name
else if lang eq es_ES
        embed_file "../../../data/es_ES/docs/" # name
else
        embed_file "../../../data/en_US/docs/" # name
end if
}

; ====================================================================

START:
        GetCommandLine eax
        movzx   ecx, byte [eax]
        or      ecx, 'a' - 'A'
        sub     ecx, 'a'
        cmp     ecx, FILECOUNT + LINKS
        jae     window
        inc     [from_cmdline]

open:
        cmp     ecx, FILECOUNT
        jb      .document
        mov     eax, [urls + ecx*4 - FILECOUNT*4]
        mov     [webview_run.url], eax
        mcall   SF_FILE, webview_run
        test    eax, eax
        jns     .done
        mov     eax, notify_webview
.notify:
        mov     [notify_run.text], eax
        mcall   SF_FILE, notify_run
        jmp     .done
.document:
        mov     edx, embedded
.find:
        mov     esi, [edx]
        add     edx, 4
        dec     ecx
        js      .found
        add     edx, esi
        jmp     .find
.found:
        push    edx
; convert number in esi to decimal representation
        mov     ecx, 10
        push    -'0'
        mov     eax, esi
@@:
        xor     edx, edx
        div     ecx
        push    edx
        test    eax, eax
        jnz     @b
        mov     edi, cedit_size
@@:
        pop     eax
        add     al, '0'
        stosb
        jnz     @b
        mcall   SF_FILE, cedit_run
        pop     edx
        mov     ecx, eax
        mov     eax, notify_cedit
        test    ecx, ecx
        js      .notify
        mov     edi, SEND_TRIES
.send:
        mcall   SF_SLEEP, SEND_DELAY
        mcall   SF_IPC, SSF_SEND_MESSAGE
        test    eax, eax
        jz      .done
        cmp     eax, 2                  ; 1: no IPC area yet, 2: it is locked
        ja      .done
        dec     edi
        jnz     .send
.done:
        cmp     [from_cmdline], 0
        je      still
close:
        mcall   SF_TERMINATE_PROCESS

; ====================================================================

window:
        mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 1
redraw:
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
        lea     esi, [eax + WINDOW_H]
        mcall   SF_GET_SCREEN_SIZE
        shr     eax, 1
        and     eax, 0x7FFF7FFF         ; the centre of the screen
        movzx   ecx, ax
        mov     edx, esi
        shr     edx, 1
        sub     ecx, edx
        shl     ecx, 16
        add     ecx, esi
        shr     eax, 16
        sub     eax, WINDOW_W/2
        shl     eax, 16
        lea     ebx, [eax + WINDOW_W]
        mov     edx, [sc.work]
        or      edx, WINDOW_STYLE
        mcall   SF_CREATE_WINDOW, , , , 0, title

        xor     ebp, ebp
.group:
        imul    ebx, ebp, GROUP_W + GAP
        add     ebx, GAP
        shl     ebx, 16
        add     ebx, GROUP_W
        mov     ecx, (GROUP_Y shl 16) + GROUP_H
        call    group_box
        inc     ebp
        cmp     ebp, COLUMNS
        jb      .group
        mov     ebx, (GAP shl 16) + CLIENT_W - GAP*2
        mov     ecx, (LINKS_Y shl 16) + LINKS_H
        call    group_box

; a button: the column in the high nibble of place, the row in the low one,
; the row after the groups is the one of the links
        xor     ebp, ebp
.button:
        movzx   eax, [place + ebp]
        mov     esi, eax
        shr     esi, 4
        imul    esi, GROUP_W + GAP
        add     esi, BTN_X
        and     eax, 0x0F
        imul    edi, eax, BTN_H + GAP
        add     edi, BTN_Y
        cmp     eax, ROWS
        jb      @f
        add     edi, LINK_Y - BTN_Y - ROWS*(BTN_H + GAP)
@@:
        mov     ecx, ebp
        mov     edx, labels
        call    nth_string
        call    button
        inc     ebp
        cmp     ebp, FILECOUNT + LINKS
        jb      .button

        mcall   SF_REDRAW, SSF_END_DRAW

; ====================================================================

still:
        mcall   SF_WAIT_EVENT
        dec     eax
        jz      redraw
        dec     eax
        jnz     .button
        mcall   SF_GET_KEY
        cmp     ah, 1                   ; Esc
        je      close
        mov     al, ah
        mov     edi, scancodes
        mov     ecx, FILECOUNT + LINKS
        repne   scasb
        jne     still
        sub     edi, scancodes + 1
        mov     ecx, edi
        jmp     open
.button:
        mcall   SF_GET_BUTTON
        cmp     ah, 1
        je      close
        movzx   ecx, ah
        sub     ecx, DOC_ID
        cmp     ecx, FILECOUNT + LINKS
        jb      open
        jmp     still

; ====================================================================

; ebp = number of the button, its hotkey is 'A' + ebp
; esi = x, edi = y, edx = label
button:
        push    edx esi
        mov     ebx, esi
        shl     ebx, 16
        add     ebx, BTN_W - 1
        mov     ecx, edi
        shl     ecx, 16
        add     ecx, BTN_H - 1
        lea     edx, [ebp + DOC_ID]
        mcall   SF_DEFINE_BUTTON, , , , [sc.work_light]
        pop     esi
        lea     eax, [ebp + 'A']
        mov     [hotkey], al
        lea     ebx, [esi + GAP]
        shl     ebx, 16
        lea     ebx, [ebx + edi + TEXT_DY]
        mov     ecx, [sc.work_text]
        or      ecx, FONT
        mcall   SF_DRAW_TEXT, , , hotkey
        pop     edx
        add     ebx, HOTKEY_W shl 16
        mcall   SF_DRAW_TEXT
        ret

; group box: a frame, the title over it on the work colour
; ebp = number of the title, ebx = x shl 16 + width, ecx = y shl 16 + height
group_box:
        mcall   SF_DRAW_RECT, , , [sc.work_graph]
        add     ebx, (1 shl 16) - 2
        add     ecx, (1 shl 16) - 2
        mcall   SF_DRAW_RECT, , , [sc.work]
        shr     ebx, 16
        add     ebx, GAP - CHAR_W - 1           ; a space, then the letter from its column 1
        shl     ebx, 16
        shr     ecx, 16
        lea     ebx, [ebx + ecx - 1 - TITLE_DY]
        mov     ecx, ebp
        mov     edx, groups
        call    nth_string
        mov     ecx, [sc.work_text]
        or      ecx, FONT + FILL
        mcall   SF_DRAW_TEXT, , , , , [sc.work]
        ret

; edx = string number ecx of the zero separated list at edx
nth_string:
        jecxz   .done
@@:
        inc     edx
        cmp     byte [edx - 1], 0
        jnz     @b
        loop    @b
.done:
        ret

; ====================================================================

cedit_run:
        dd      SSF_START_APP
        dd      0
        dd      cedit_param
        dd      0, 0
        db      "/sys/develop/cedit", 0

webview_run:
        dd      SSF_START_APP
        dd      0
.url:   dd      0
        dd      0, 0
        db      "/sys/network/webview", 0

notify_run:
        dd      SSF_START_APP
        dd      0
.text:  dd      0
        dd      0, 0
        db      "/sys/@notify", 0

urls            dd url_wiki, url_fasm
                assert ($ - urls)/4 = LINKS
url_fasm        db "https://flatassembler.net/docs.php?article=manual", 0

; the keys A..K in any layout and case
scancodes       db 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25
                assert $ - scancodes = FILECOUNT + LINKS
place           db 0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13, 0x14, 0x05, 0x15
                assert $ - place = FILECOUNT + LINKS
hotkey          db "A | ", 0

cedit_param     db "*"
cedit_size      db "0000000000", 0

; labels in the order of the embedded files, then the ones of the links
if lang eq ru_RU
LABEL_CHARS = 17
title           db 3, "Документация KolibriOS", 0
groups          db " Общее ", 0, " Разработка ", 0, " Онлайн-документация ", 0
url_wiki        db "https://wiki.kolibrios.org/wiki/For_developers/ru", 0
notify_cedit    cp866 "'DocPack\nНе удалось запустить /sys/develop/cedit' -tE", 0
notify_webview  cp866 "'DocPack\nНе удалось запустить /sys/network/webview' -tE", 0
labels          db "Лицензия", 0, "Горячие клавиши", 0, "Клавиши KFar", 0, "Благодарности", 0
                db "Отладчик", 0, "Системные функции", 0, "Сетевые функции", 0
                db "Файлы INI", 0, "Диалог открытия", 0
                db "Вики KolibriOS", 0, "Руководство FASM", 0
else if lang eq es_ES
LABEL_CHARS = 21
title           db 3, "Documentación de KolibriOS", 0
groups          db " General ", 0, " Desarrollo ", 0, " Documentación en línea ", 0
url_wiki        db "https://wiki.kolibrios.org/wiki/For_developers/en", 0
notify_cedit    db "'DocPack\nNo se pudo iniciar /sys/develop/cedit' -tE", 0
notify_webview  db "'DocPack\nNo se pudo iniciar /sys/network/webview' -tE", 0
labels          db "Licencia", 0, "Atajos de teclado", 0, "Teclas de KFar", 0, "Créditos", 0
                db "Depurador", 0, "Funciones del sistema", 0, "Funciones de red", 0
                db "Archivos INI", 0, "Diálogo de apertura", 0
                db "Wiki de KolibriOS", 0, "Manual de FASM", 0
else
LABEL_CHARS = 17
title           db 3, "KolibriOS Documentation", 0
groups          db " General ", 0, " Development ", 0, " Online documentation ", 0
url_wiki        db "https://wiki.kolibrios.org/wiki/For_developers/en", 0
notify_cedit    db "'DocPack\nCould not start /sys/develop/cedit' -tE", 0
notify_webview  db "'DocPack\nCould not start /sys/network/webview' -tE", 0
labels          db "License", 0, "Hotkeys", 0, "KFar keys", 0, "Credits", 0
                db "Debugger", 0, "System functions", 0, "Network functions", 0
                db "INI files", 0, "Open dialog", 0
                db "KolibriOS Wiki", 0, "FASM manual", 0
end if

; ====================================================================

embedded:
        embed_doc  "Copying.txt"                                ; a
        embed_doc  "Hot_Keys.txt"                               ; b
        embed_doc  "KFAR_Keys.txt"                              ; c
        embed_doc  "Credits.txt"                                ; d
        embed_doc  "Mtdbg.txt"                                  ; e
if lang eq ru_RU
        embed_file "SysFuncr.txt"                               ; f
else
        embed_file "../../../kernel/trunk/docs/sysfuncs.txt"    ; f
end if
        embed_file "../../../kernel/trunk/docs/stack.txt"       ; g
        embed_doc  "INI.txt"                                    ; h
        embed_doc  "OpenDial.txt"                               ; i

FILECOUNT = FILE_COUNT

; ====================================================================

from_cmdline    db ?
sc              system_colors
