; SPDX-License-Identifier: NOASSERTION
;
; DocPack - KolibriOS documentation
; Copyright (C) 2006-2026 KolibriOS team
;
; Authors: Ivushkin Andrey aka Willow, Burer
;
; DOCPACK    window with the documents and the online documentation
; DOCPACK x  open the document or the link of the letter x, one of
;            l h k c d s n i o w f, the English hotkeys in every language

; ====================================================================

include "../../macros.inc"
include "../../KOSfuncs.inc"
include "../../encoding.inc"

format meos 01
entry START
stack 512

include "group_box.inc"

; ====================================================================

START:
        ; "DOCPACK x" opens the item of the letter x and exits; the bit 5
        ; makes a capital small, and no parameter a space: the window
        GetCommandLine eax
        mov     al, [eax]
        or      al, 'a' - 'A'

        mov     edx, ITEM.letter
        call    find
        jc      redraw

        call    [edi + ITEM.open]
        jmp     exit

; ====================================================================

still:
        mcall   SF_WAIT_EVENT

        cmp     al, EV_REDRAW
        je      redraw
        cmp     al, EV_KEY
        je      key
        cmp     al, EV_BUTTON
        je      button

        jmp     still

; ====================================================================

key:
        ; its scancode, the bits 16..23, so the hotkeys work in any layout
        ; and case
        mcall   SF_GET_KEY
        shr     eax, 16

        mov     edx, ITEM.key
        jmp     press

; ====================================================================

button:
        mcall   SF_GET_BUTTON

        cmp     ah, BTN_CLOSE
        je      exit

        ; the id of a button is the letter of its item
        mov     al, ah
        mov     edx, ITEM.letter

; open the item of the key or the button, if it is one
press:
        call    find
        jc      still

        call    [edi + ITEM.open]
        jmp     still

exit:
        mcall   SF_TERMINATE_PROCESS

; ====================================================================

; al = the byte at the offset edx of an ITEM: edi -> that ITEM, no carry;
; carry if there is none
find:
        mov     edi, items

.next:
        cmp     al, [edi + edx]
        je      .found

        add     edi, sizeof.ITEM
        cmp     edi, items_end
        jb      .next

        stc
        ret

.found:
        clc
        ret

; ====================================================================

; edi -> the ITEM of a document: CEdit shows it
open_doc:
        mcall   SF_FILE, cedit_run
        test    eax, eax
        js      notify

        ; then the text by IPC, once CEdit has set its IPC area; the
        ; sleep changes only eax and ebx, the PID, text and size stay
        mov     ecx, eax
        mov     edx, [edi + ITEM.arg]
        mov     esi, [edx]
        add     edx, 4
        mov     edi, SEND_TRIES

.send:
        mcall   SF_SLEEP, SEND_DELAY
        mcall   SF_IPC, SSF_SEND_MESSAGE
        test    eax, eax
        jz      .done

        dec     edi
        jnz     .send

        jmp     notify

.done:
        ret

; edi -> the ITEM of a link: WebView opens its URL
open_link:
        mov     eax, [edi + ITEM.arg]
        mov     [webview_run.url], eax
        mcall   SF_FILE, webview_run
        test    eax, eax
        js      notify

        ret

; the item did not open
notify:
        mcall   SF_FILE, notify_run

        ret

; ====================================================================

redraw:
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors
        mcall   SF_REDRAW, SSF_BEGIN_DRAW

        ; the window in the centre of the screen; the kernel takes its
        ; place only the first time
        mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
        lea     esi, [eax + WINDOW_H]
        mcall   SF_GET_SCREEN_SIZE

        movzx   ecx, ax
        sub     ecx, esi
        shr     ecx, 1
        shl     ecx, 16
        add     ecx, esi

        shr     eax, 16
        sub     eax, WINDOW_W
        shr     eax, 1
        shl     eax, 16
        lea     ebx, [eax + WINDOW_W]

        mov     edx, [sc.work]
        or      edx, WINDOW_STYLE
        mcall   SF_CREATE_WINDOW, , , , 0, title

        ; the frames of the groups with their titles
        mov     edi, boxes

.group:
        call    group_box

        add     edi, sizeof.GROUP_BOX
        cmp     edi, boxes_end
        jb      .group

        ; the buttons of the items with their captions
        mov     edi, items

.button:
        ; the button, the kernel makes it a pixel wider and higher
        mov     ecx, [edi + ITEM.x_y]
        mov     ebx, ecx
        and     ebx, 0xFFFF0000
        add     ebx, BTN.W - 1
        shl     ecx, 16
        add     ecx, BTN.H - 1
        movzx   edx, [edi + ITEM.letter]
        mcall   SF_DEFINE_BUTTON, , , , [sc.work_light]

        ; its caption: the number of the hotkey letter, then the text
        mov     ebx, [edi + ITEM.x_y]
        add     ebx, (GAP shl 16) + TEXT_DY
        mov     edx, [edi + ITEM.caption]
        inc     edx
        mov     ecx, [sc.work_text]
        or      ecx, FONT
        mcall   SF_DRAW_TEXT

        ; the line under the hotkey letter
        movzx   eax, byte [edx - 1]
        imul    eax, FONT_CHAR_W shl 16
        add     ebx, eax
        mov     ecx, ebx
        shl     ecx, 16
        add     ecx, (UNDERLINE_DY shl 16) + 1
        and     ebx, 0xFFFF0000
        add     ebx, FONT_CHAR_W
        mcall   SF_DRAW_RECT, , , [sc.work_text]

        add     edi, sizeof.ITEM
        cmp     edi, items_end
        jb      .button

        mcall   SF_REDRAW, SSF_END_DRAW
        jmp     still

; ====================================================================

; layout, a RECT is x, y, width, height; BTN is after the captions
GAP         = 12
ROW         = BTN.H + GAP
ROWS        = 5
; the capitals and the underline centred in a button, the underline
; under the text
TEXT_DY     = (BTN.H - FONT_CAPS_H - 2)/2 - FONT_CAPS_Y
UNDERLINE_DY = FONT_CAPS_Y + FONT_CAPS_H + 1

; two groups of the documents, GAP from the top to the capitals of the
; titles, the group of the links below them, full width; the buttons
; GAP inside the lines and GAP under the titles
GROUP_W     = GROUP_LINES + GAP + BTN.W + GAP + GROUP_LINES
GROUP_TOP   = GROUP_TITLE_BELOW + GAP           ; from the top line to the buttons
GENERAL     RECT    GAP, GAP + GROUP_TITLE_ABOVE, GROUP_W, GROUP_TOP + ROWS*ROW + GROUP_LINES
DEVELOPMENT RECT    GAP + GROUP_W + GAP, GENERAL.Y, GROUP_W, GENERAL.H
ONLINE      RECT    GAP, GENERAL.Y + GENERAL.H + GAP + GROUP_TITLE_ABOVE, GROUP_W*2 + GAP, GROUP_TOP + ROW + GROUP_LINES

; the buttons: x of the columns, y of the first rows
COL1        = GENERAL.X + GROUP_LINES + GAP
COL2        = DEVELOPMENT.X + GROUP_LINES + GAP
BTN_Y       = GENERAL.Y + GROUP_TOP
LINK_Y      = ONLINE.Y + GROUP_TOP

; window
CLIENT_W    = GAP + ONLINE.W + GAP
CLIENT_H    = ONLINE.Y + ONLINE.H + GAP
WINDOW_W    = CLIENT_W + 9                      ; borders 5 + 5, the kernel adds a pixel
WINDOW_H    = CLIENT_H + 4                      ; border 5, the same pixel; redraw adds the skin
WINDOW_STYLE = 0x34000000                       ; skinned, fixed size, caption, client coordinates

; text
FONT        = 0xB0000000                        ; zero terminated, 8x16 UTF-8

; the close button of the window
BTN_CLOSE   = 1

; CEdit waits 2 s for the text: SEND_TRIES sleeps of SEND_DELAY/100 s
SEND_TRIES  = 20
SEND_DELAY  = 10
DOC_MAX     = 524288                            ; the largest document, the size in cedit_param

; ====================================================================

; a button of the window, its key and what it opens
struct ITEM
        x_y             dd ?            ; x shl 16 + y
        letter          db ?            ; on the command line, the id of the button
        key             db ?            ; the scancode of the hotkey
        caption         dd ?
        open            dd ?            ; open_doc or open_link
        arg             dd ?            ; the document or the URL
ends

; an ITEM in a table
macro item_row x, y, letter, key, caption, open, arg
{
        local   start
start:
        dd      (x) shl 16 + (y)
        db      letter, key
        dd      caption, open, arg
        assert  $ - start = sizeof.ITEM
}

; a caption: the scancode of its hotkey, the number of the underlined
; letter from 0, the text; CAPTION_MAX is the length of the longest text
; in characters, its bytes but the ones that go on with a character,
; 10xxxxxx
CAPTION_MAX = 0

struc caption key, underlined, text
{
        local   n, b

        .key = key
        . db    underlined, text, 0

        n = 0
        repeat  $ - . - 2
                load    b byte from . + %
                if b shr 6 <> 10b
                        n = n + 1
                end if
        end repeat
        if n > CAPTION_MAX
                CAPTION_MAX = n
        end if
}

; a document: its size, then its text
struc document path
{
        local   size

        . dd    size
        file    path
        size = $ - . - 4

        assert  size <= DOC_MAX
}

; a document of data/<language>/docs
struc doc name
{
if lang eq ru_RU
        . document "../../../data/ru_RU/docs/" # name
else if lang eq es_ES
        . document "../../../data/es_ES/docs/" # name
else
        . document "../../../data/en_US/docs/" # name
end if
}

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
.url    dd      0
        dd      0, 0
        db      "/sys/network/webview", 0

notify_run:
        dd      SSF_START_APP
        dd      0
        dd      notify_text
        dd      0, 0
        db      "/sys/@notify", 0

; the groups, the titles over the left edges of the buttons
boxes:
        ;             x              y              width          height         title           title_x  colors
        group_box_row GENERAL.X,     GENERAL.Y,     GENERAL.W,     GENERAL.H,     sz_general,     GAP,     sc
        group_box_row DEVELOPMENT.X, DEVELOPMENT.Y, DEVELOPMENT.W, DEVELOPMENT.H, sz_development, GAP,     sc
        group_box_row ONLINE.X,      ONLINE.Y,      ONLINE.W,      ONLINE.H,      sz_online,      GAP,     sc
boxes_end:

; the items: where, the letter, the key and the caption, what opens it and
; what
items:
        ;        x     y              letter key              caption      open       arg
        item_row COL1, BTN_Y,         'l',   sz_license.key,  sz_license,  open_doc,  doc_license
        item_row COL1, BTN_Y + ROW,   'h',   sz_hotkeys.key,  sz_hotkeys,  open_doc,  doc_hotkeys
        item_row COL1, BTN_Y + 2*ROW, 'k',   sz_kfar.key,     sz_kfar,     open_doc,  doc_kfar
        item_row COL1, BTN_Y + 3*ROW, 'c',   sz_credits.key,  sz_credits,  open_doc,  doc_credits
        item_row COL2, BTN_Y,         'd',   sz_debugger.key, sz_debugger, open_doc,  doc_debugger
        item_row COL2, BTN_Y + ROW,   's',   sz_sysfuncs.key, sz_sysfuncs, open_doc,  doc_sysfuncs
        item_row COL2, BTN_Y + 2*ROW, 'n',   sz_network.key,  sz_network,  open_doc,  doc_network
        item_row COL2, BTN_Y + 3*ROW, 'i',   sz_ini.key,      sz_ini,      open_doc,  doc_ini
        item_row COL2, BTN_Y + 4*ROW, 'o',   sz_opendial.key, sz_opendial, open_doc,  doc_opendial
        item_row COL1, LINK_Y,        'w',   sz_wiki.key,     sz_wiki,     open_link, url_wiki
        item_row COL2, LINK_Y,        'f',   sz_fasm.key,     sz_fasm,     open_link, url_fasm
items_end:

if lang eq ru_RU
url_wiki        db "https://wiki.kolibrios.org/wiki/For_developers/ru", 0
else
url_wiki        db "https://wiki.kolibrios.org/wiki/For_developers/en", 0
end if
url_fasm        db "https://flatassembler.net/docs.php?article=manual", 0

; CEdit takes the size of its IPC buffer after "*", the text comes with
; its own size, so one size for all the documents: DOC_MAX in decimal
cedit_param     db "*524288", 0

; ====================================================================

; the texts: the title starts with 3, it is UTF-8; @notify takes
; 'title\ntext' -tE in cp866, E: the error icon; the captions take the
; scancodes of the hotkeys in the layout of the language, for ru_RU
; their keys in QWERTY are at the right
if lang eq ru_RU

title           db 3, "Документация KolibriOS", 0
sz_general      db "Общее", 0
sz_development  db "Разработка", 0
sz_online       db "Онлайн-документация", 0
notify_text     cp866 "'DocPack\nНе удалось открыть документацию' -tE", 0

sz_license      caption 0x25, 0, "Лицензия"             ; K
sz_hotkeys      caption 0x16, 0, "Горячие клавиши"      ; U
sz_kfar         caption 0x13, 0, "Клавиши KFar"         ; R
sz_credits      caption 0x33, 0, "Благодарности"        ; ,
sz_debugger     caption 0x24, 0, "Отладчик"             ; J
sz_sysfuncs     caption 0x2E, 0, "Системные функции"    ; C
sz_network      caption 0x31, 2, "Сетевые функции"      ; N
sz_ini          caption 0x1E, 0, "Файлы INI"            ; A
sz_opendial     caption 0x26, 0, "Диалог открытия"      ; L
sz_wiki         caption 0x20, 0, "Вики KolibriOS"       ; D
sz_fasm         caption 0x23, 0, "Руководство FASM"     ; H


else if lang eq es_ES

title           db 3, "Documentación de KolibriOS", 0
sz_general      db "General", 0
sz_development  db "Desarrollo", 0
sz_online       db "Documentación en línea", 0
notify_text     db "'DocPack\nNo se pudo abrir la documentacion' -tE", 0

sz_license      caption 0x26, 0, "Licencia"
sz_hotkeys      caption 0x1E, 0, "Atajos de teclado"
sz_kfar         caption 0x14, 0, "Teclas de KFar"
sz_credits      caption 0x2E, 0, "Créditos"
sz_debugger     caption 0x20, 0, "Depurador"
sz_sysfuncs     caption 0x21, 0, "Funciones del sistema"
sz_network      caption 0x13, 13, "Funciones de red"
sz_ini          caption 0x17, 9, "Archivos INI"
sz_opendial     caption 0x18, 4, "Diálogo de apertura"
sz_wiki         caption 0x11, 0, "Wiki de KolibriOS"
sz_fasm         caption 0x32, 0, "Manual de FASM"


else

title           db 3, "KolibriOS Documentation", 0
sz_general      db "General", 0
sz_development  db "Development", 0
sz_online       db "Online documentation", 0
notify_text     db "'DocPack\nCould not open the documentation' -tE", 0

sz_license      caption 0x26, 0, "License"
sz_hotkeys      caption 0x23, 0, "Hotkeys"
sz_kfar         caption 0x25, 0, "KFar keys"
sz_credits      caption 0x2E, 0, "Credits"
sz_debugger     caption 0x20, 0, "Debugger"
sz_sysfuncs     caption 0x1F, 0, "System functions"
sz_network      caption 0x31, 0, "Network functions"
sz_ini          caption 0x17, 0, "INI files"
sz_opendial     caption 0x18, 0, "Open dialog"
sz_wiki         caption 0x11, 10, "KolibriOS Wiki"
sz_fasm         caption 0x21, 0, "FASM manual"


end if

; a button fits the longest caption
BTN         RECT    0, 0, CAPTION_MAX*FONT_CHAR_W + GAP*2, 24

; ====================================================================

doc_license     doc "Copying.txt"
doc_hotkeys     doc "Hot_Keys.txt"
doc_kfar        doc "KFAR_Keys.txt"
doc_credits     doc "Credits.txt"
doc_debugger    doc "Mtdbg.txt"
; SysFuncr.txt: the Tupfile makes it from the kernel's sysfuncr.txt, cp866
if lang eq ru_RU
doc_sysfuncs    document "SysFuncr.txt"
else
doc_sysfuncs    document "../../../kernel/trunk/docs/sysfuncs.txt"
end if
doc_network     document "../../../kernel/trunk/docs/stack.txt"
doc_ini         doc "INI.txt"
doc_opendial    doc "OpenDial.txt"

; ====================================================================

sc              system_colors
