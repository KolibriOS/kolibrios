; SPDX-License-Identifier: GPL-2.0-only
;
; Reshare - Shared Resources Daemon
; Copyright (C) 2024-2025 KolibriOS/-NG Team
;
; Contributor Leency - Original version in C--
; Contributor rgmaid - Main code
; Contributor Burer  - Refactoring and localization

; ====================================================================

use32
org 0

; ====================================================================

db      "MENUET01"
dd      1
dd      START
dd      I_END
dd      MEM
dd      STACKTOP
dd      0
dd      0

; ====================================================================

include "../../macros.inc"
include "../../KOSfuncs.inc"
include "../../encoding.inc"
include "../../proc32.inc"
include "../../dll.inc"
include "../../string.inc"
include "../../debug-fdo.inc"

include "../../develop/libraries/libs-dev/libimg/libimg.inc"

include "checkbox.inc"

; ====================================================================

LIBS:
        library img,    "libimg.obj"
        import  img,    img.destroy,   "img_destroy", \
                        img.from_file, "img_from_file"

; ====================================================================

START:
        mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 1

        mcall   SF_SYS_MISC, SSF_HEAP_INIT

        ; check if this is second instance of app. if so - run gui, else - run daemon
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, meta_name, 0, SHM_OPEN + SHM_READ
        test    eax, eax
        jz      MODE_DAEMON
        jmp     MODE_GUI

.exit:
        mcall   SF_TERMINATE_PROCESS

; ====================================================================

MODE_GUI:

.event_loop:

        mcall   SF_WAIT_EVENT

        cmp     eax, EV_REDRAW
        je      .event_redraw

        cmp     eax, EV_KEY
        je      .event_key

        cmp     eax, EV_BUTTON
        je      .event_button

        jmp     .event_loop


.event_redraw:

        mcall   SF_REDRAW, SSF_BEGIN_DRAW

        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors
        mcall                    , SSF_GET_SKIN_HEIGHT

        mov     ecx, WIN.Y shl 16 + WIN.H
        add     ecx, eax
        mov     edx, [sc.work]
        add     edx, 0x34000000

        mcall   SF_CREATE_WINDOW, <WIN.X, WIN.W>, , , , title

        ; visual dividers
        mcall   SF_DRAW_RECT, <PAD, GRID.W>, <BTN.Y + BTN.H + GAP, 1>, [sc.work_graph]
        mcall               ,              , <GRID.Y + GRID.H, 1>,

        ; info row
        mov     ecx, FONT_TYPE shl 24
        add     ecx, [sc.work_text]
        mcall   SF_DRAW_TEXT, <PAD,    GRID.Y + GRID.H + 1 + GAP>, , lb_info

        ; tabs labels
        mcall               , <TAB_STEP * 0 + PAD * 2,     BTN.Y>, , lb_tab_i32
        mcall               , <TAB_STEP * 0 + PAD * 2 + 1, BTN.Y>, ,
        mcall               , <TAB_STEP * 0 + PAD *11,     BTN.Y>, , lb_tab_i32_res
        mcall               , <TAB_STEP * 1 + PAD * 2,     BTN.Y>, , lb_tab_i18
        mcall               , <TAB_STEP * 1 + PAD * 2 + 1, BTN.Y>, ,
        mcall               , <TAB_STEP * 1 + PAD *11,     BTN.Y>, , lb_tab_i18_res
        mcall               , <TAB_STEP * 2 + PAD * 2,     BTN.Y>, , lb_tab_i18w
        mcall               , <TAB_STEP * 2 + PAD * 2 + 1, BTN.Y>, ,
        mcall               , <TAB_STEP * 2 + PAD *11,     BTN.Y>, , lb_tab_i18w_res
        mcall               , <TAB_STEP * 3 + PAD * 2,     BTN.Y>, , lb_tab_cbox
        mcall               , <TAB_STEP * 3 + PAD * 2 + 1, BTN.Y>, ,
        mcall               , <TAB_STEP * 3 + PAD *11,     BTN.Y>, , lb_tab_cbox_res

        ; tabs buttons
        mcall   SF_DEFINE_BUTTON, <TAB_STEP * 0 + PAD, BTN.W>, <BTN.Y, BTN.H>, 10 + BTN_HIDE + ACTIVE_ICONS32
        mcall                   , <TAB_STEP * 1 + PAD, BTN.W>,               , 10 + BTN_HIDE + ACTIVE_ICONS18
        mcall                   , <TAB_STEP * 2 + PAD, BTN.W>,               , 10 + BTN_HIDE + ACTIVE_ICONS18W
        mcall                   , <TAB_STEP * 3 + PAD, BTN.W>,               , 10 + BTN_HIDE + ACTIVE_CHECKBOX

        ; tabs content
        stdcall draw_tabs

        mcall   SF_REDRAW, SSF_END_DRAW

        jmp     .event_loop


.event_key:

        mcall   SF_GET_KEY
        cmp     ah, 15 ; TAB
        jne     .event_loop

        shl     [active_tab], 1 ; * 2
        cmp     [active_tab], ACTIVE_CHECKBOX
        jbe     .tab_draw
        mov     [active_tab], 1

        .tab_draw:
        stdcall draw_tabs

        jmp     .event_loop


.event_button:

        mcall   SF_GET_BUTTON

        cmp     ah, 1
        je      .event_exit

        movzx   eax, ah
        sub     eax, 10
        mov     [active_tab], eax
        stdcall draw_tabs

        jmp     .event_loop


.event_exit:
        mcall   SF_TERMINATE_PROCESS

; ====================================================================

proc draw_tabs stdcall
        ; draw tabs headers underlines
        mov     esi, [active_tab]
        xor     edi, edi

        .loop:
                mov     edx, [sc.work_dark]
                mov     eax, [tab_masks + edi * 4]
                test    esi, eax
                jz      .color_ok
                mov     edx, [sc.work_button]

        .color_ok:
                mov     eax, edi
                imul    eax, TAB_STEP
                add     eax, PAD
                mpack   ebx, eax, BTN.W
                mpack   ecx, PAD + BTN.H + 2, 2
                mcall   SF_DRAW_RECT, ebx, ecx, edx

                inc     edi
                cmp     edi, TAB_COUNT
                jb      .loop

        ; draw current tab content
        mcall   SF_DRAW_RECT, <GRID.X, GRID.W>, <GRID.Y, GRID.H>, [sc.work]

        mov     eax, [active_tab]

        .tab_i32:
        cmp     eax, ACTIVE_ICONS32
        jne     .tab_i18
        stdcall draw_tab_icons, lb_tab_i32,  META_I32_C,  META_I32_W
        ret

        .tab_i18:
        cmp     eax, ACTIVE_ICONS18
        jne     .tab_i18w
        stdcall draw_tab_icons, lb_tab_i18,  META_I18_C,  META_I18_W
        ret

        .tab_i18w:
        cmp     eax, ACTIVE_ICONS18W
        jne     .tab_cbox
        stdcall draw_tab_icons, lb_tab_i18w, META_I18W_C, META_I18W_W
        ret

        .tab_cbox:
        stdcall draw_tab_cbox
        ret
endp


proc draw_tab_icons stdcall uses ebx ecx, _shm_name, _meta_c_off, _meta_w_off
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, meta_name, 0, SHM_OPEN + SHM_READ
        test    eax, eax
        jz      .done

        mov     ebx, [_meta_c_off]
        mov     ecx, [eax + ebx]    ; icon_c
        mov     ebx, [_meta_w_off]
        mov     edx, [eax + ebx]    ; icon_w

        test    ecx, ecx
        jz      .done
        test    edx, edx
        jz      .done

        push    ecx
        push    edx
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, [_shm_name], 0, SHM_OPEN + SHM_READ
        pop     edx
        pop     ecx

        test    eax, eax
        jz      .done

        mov     ebx, eax

        stdcall draw_tab_icons_grid, ebx, ecx, edx

        .done:
        ret
endp


proc draw_tab_icons_grid stdcall uses eax ebx ecx edx esi edi, _icon_img, _icon_c, _icon_w
locals
        x          dd 0
        x_off      dd 0
        y          dd 0
        icon_size  dd 0
endl
        ; icon_bytes = icon_w * icon_w * 4
        mov     eax, [_icon_w]
        imul    eax, eax
        shl     eax, 2
        mov     [icon_size], eax

        ; x_off = (CELL.W - icon_w) / 2
        mov     eax, CELL.W
        sub     eax, [_icon_w]
        shr     eax, 1
        mov     [x_off], eax

        ; y_step = icon_w + 28
        mov     eax, [_icon_w]
        add     eax, 28
        mov     edi, eax

        xor     ecx, ecx

        .for_icons:
                cmp     ecx, [_icon_c]
                jae     .end_for_icons
                push    ecx

                ; SF_PUT_IMAGE_EXT, _icon_img+icon_size*index, <_icon_w, _icon_w>, <x+x_off, y+RES_Y>, 32, 0, 0
                mov     ebx, [icon_size]
                imul    ebx, ecx
                add     ebx, [_icon_img]

                mov     ecx, [_icon_w]
                shl     ecx, 16
                add     ecx, [_icon_w]

                mov     edx, [x]
                add     edx, [x_off]
                shl     edx, 16
                add     edx, [y]
                add     edx, GRID.Y

                push    edi
                push    ebp
                mcall   SF_PUT_IMAGE_EXT, , , , 32, 0, 0
                pop     ebp
                pop     edi

                ; draw number, centered in CELL.W
                mov     edx, [x]
                add     edx, GAP
                shl     edx, 16
                add     edx, [y]
                add     edx, [_icon_w]
                add     edx, GRID.Y + 4
                mov     esi, 0x10000000
                add     esi, [sc.work_graph]

                pop     ecx
                mcall   SF_DRAW_NUMBER, 0x00030000, , , ,
                inc     ecx

                add     [x], CELL.W
                cmp     [x], WIN.W - CELL.W
                jle     .for_icons
                mov     [x], 0
                add     [y], edi

                jmp     .for_icons
        .end_for_icons:

        ret
endp


proc draw_tab_cbox stdcall
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, lb_tab_cbox, 0, SHM_OPEN + SHM_READ
        test    eax, eax
        jz      .done
        mov     ebx, eax
        mcall   SF_PUT_IMAGE, , \
                <CHBOX_WIDTH, CHBOX_HEIGHT>, \
                <(WIN.W - CHBOX_WIDTH)/2, (WIN.H - RES_Y - CHBOX_HEIGHT)/2 + RES_Y>

        .done:
        ret
endp

; ====================================================================

MODE_DAEMON:

        stdcall dll.Load, LIBS

        ; load shared resources from files
        DEBUGF  DBG_INFO, "@reshare: loading resources...\n"

        stdcall load_icons, icons32_path, icons32_image, size32
        stdcall load_icons, icons18_path, icons18_image, size18

        ; publish meta
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, meta_name, META_SIZE, SHM_CREATE + SHM_WRITE
        test    eax, eax
        jz      .meta_done
        mov     edi, eax

        xor     ebx, ebx
        xor     ecx, ecx
        mov     eax, [icons32_image]
        test    eax, eax
        jz      .i32_set

        mov     ebx, [eax + Image.Width]
        mov     eax, [eax + Image.Height]
        test    ebx, ebx
        jz      .i32_set
        xor     edx, edx
        div     ebx
        mov     ecx, eax

        .i32_set:
        mov     [edi + META_I32_W], ebx
        mov     [edi + META_I32_C], ecx
        xor     ebx, ebx
        xor     ecx, ecx
        mov     eax, [icons18_image]
        test    eax, eax
        jz      .i18_set
        mov     ebx, [eax + Image.Width]
        mov     eax, [eax + Image.Height]
        test    ebx, ebx
        jz      .i18_set
        xor     edx, edx
        div     ebx
        mov     ecx, eax

        .i18_set:
        mov     [edi + META_I18_W], ebx
        mov     [edi + META_I18_C], ecx
        mov     [edi + META_I18W_W], ebx
        mov     [edi + META_I18W_C], ecx

        mov     dword [edi + META_CBOX_W], CHBOX_WIDTH
        mov     dword [edi + META_CBOX_H], CHBOX_HEIGHT
        mov     dword [edi + META_CBOX_SIZE], CHBOX_IMG_SIZE

        .meta_done:
        DEBUGF  DBG_INFO, "@reshare: starting in daemon mode\n"

        mcall   SF_SYS_MISC, SSF_MEM_OPEN, lb_tab_cbox, CHBOX_IMG_SIZE, SHM_CREATE + SHM_WRITE
        test    eax, eax
        jz      .skip_cbox
                mov     esi, cbox_image
                mov     edi, eax
                mov     ecx, CHBOX_IMG_SIZE
                cld
                rep     movsb
        .skip_cbox:

        cmp    [icons32_image], 0
        jz      .skip_i32
                stdcall copy_image_to_shm, lb_tab_i32, size32, icons32_image
                test    eax, eax
                jz      .skip_i32
                invoke  img.destroy, [icons32_image]
        .skip_i32:

        cmp     [icons18_image], 0
        jz      .skip_i18
                stdcall copy_image_to_shm, lb_tab_i18, size18, icons18_image
                test    eax, eax
                jz      .skip_i18

                mcall   SF_SYS_MISC, SSF_MEM_OPEN, lb_tab_i18w, [size18], SHM_CREATE + SHM_WRITE
                test    eax, eax
                jz      .skip_i18
                mov     [shared_i18w], eax
        .skip_i18:

        mcall   SF_SET_EVENTS_MASK, EVM_BACKGROUND

        .event_loop:
                push    [sc.work]
                mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors
                pop     eax
                cmp     eax, [sc.work]
                je      .wait_event
                cmp     [icons18_image], 0
                jz      .wait_event
                cmp     [shared_i18w], 0
                jz      .wait_event
                mov     ebx, [icons18_image]
                mov     esi, [ebx + Image.Data]
                mov     edi, [shared_i18w]
                mov     ecx, [size18]
                shr     ecx, 2 ; / 4 to get size in dwords
                cld
                rep     movsd
                stdcall replace_2cols, [shared_i18w], [size18], 0xFFFFFFFF, [sc.work], 0xFFCACBD6, [sc.work_dark]

        .wait_event:
                mcall   SF_WAIT_EVENT
                cmp     eax, EV_BACKGROUND
                je      .event_loop
                jmp     .wait_event

; ====================================================================

proc load_icons stdcall uses ebx ecx, _path, _img_ptr, _size_ptr
        invoke  img.from_file, [_path]
        test    eax, eax
        jz      .fail

        mov     ebx, [_img_ptr]
        mov     [ebx], eax
        mov     ebx, [eax + Image.Width]
        mov     ecx, [eax + Image.Height]
        imul    ecx, ebx
        shl     ecx, 2 ; * 4 to get size in bytes
        mov     ebx, [_size_ptr]
        mov     [ebx], ecx

        ret

        .fail:
        DEBUGF  DBG_ERR, "@reshare: error loading icons from %s\", [_path]

        ret
endp


proc copy_image_to_shm stdcall uses ebx ecx edx esi edi, _shm_name, _size_ptr, _image_ptr
        mov     edx, [_size_ptr]
        mov     edx, [edx]
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, [_shm_name], edx, SHM_CREATE + SHM_WRITE
        test    eax, eax
        jz      .done
        mov     ebx, [_image_ptr]
        mov     ebx, [ebx]
        mov     esi, [ebx + Image.Data]
        mov     edi, eax
        mov     ecx, [_size_ptr]
        mov     ecx, [ecx]
        shr     ecx, 2 ; / 4 to get size in dwords
        cld
        rep     movsd

        .done:
        ret
endp


proc replace_2cols stdcall uses edi, imgsrc, imgsize, color_old_1, color_new_1, color_old_2, color_new_2
        mov     edx, [imgsize]
        add     edx, [imgsrc]
        mov     edi, [imgsrc]

        .loop:
                cmp     edi, edx
                jae     .done
                mov     eax, [edi]
                cmp     eax, [color_old_1]
                jne     .check_second
                mov     eax, [color_new_1]
                mov     [edi], eax
                jmp     .next

                .check_second:
                cmp     eax, [color_old_2]
                jne     .next
                mov     eax, [color_new_2]
                mov     [edi], eax

                .next:
                add     edi, 4
                jmp     .loop
        .done:

        ret
endp

; ====================================================================

ICON_32_SIZE    = 32
ICON_18_SIZE    = 18

BORD            = 5
PAD             = 8
GAP             = 12

GRID_COLS       = 16
GRID_ROWS       = 9

RES_Y           = BTN.Y + BTN.H + PAD * 2 + 1
TAB_STEP        = BTN.W + PAD * 2

WIN             RECT     80,      50, GRID.W + PAD * 2 + BORD * 2, 638
BTN             RECT      0, PAD + 4, CELL.W * 4 - PAD * 2       ,  22
CELL            RECT      0,       0,         32 + PAD * 2       ,  60

GRID            RECT    PAD, RES_Y + PAD, CELL.W * GRID_COLS - PAD * 2, CELL.H * GRID_ROWS

FONT_TYPE       = 0x90
BTN_HIDE        = 0x60000000

; ====================================================================

if lang eq ru_RU

        title   cp866 "@RESHARE - Служба общих ресурсов", 0
        lb_info cp866 "Имя каждой вкладки соответствует имени области памяти, доступной через сисфункцию 68.22.", 0

else if lang eq es_ES

        title   db "@RESHARE - Servicio de recursos compartidos", 0
        lb_info db "Cada nombre de pestana corresponde al nombre de memoria accesible mediante sysfunc 68.22.", 0

else

        title   cp850 "@RESHARE - Shared resources service", 0
        lb_info cp850 "Each tab name corresponds to shared memory page name that can be accessed via sysfunc 68.22.", 0

endf

lb_tab_i32      db "ICONS32", 0
lb_tab_i18      db "ICONS18", 0
lb_tab_i18w     db "ICONS18W", 0
lb_tab_cbox     db "CHECKBOX", 0

lb_tab_i32_res  db "32x32x32bpp", 0
lb_tab_i18_res  db "18x18x32bpp", 0
lb_tab_i18w_res db "18x18x32bpp", 0
lb_tab_cbox_res db "13x13x24bpp", 0

dg_icons32_fail db "@reshare: error, icons32 not found in %s\n", 0
dg_icons18_fail db "@reshare: error, icons18 not found in %s\n", 0

; ====================================================================

DBG_ALL         = 0  ; all messages
DBG_INFO        = 1  ; info and errors
DBG_ERR         = 2  ; only errors

__DEBUG__       = 1
__DEBUG_LEVEL__ = DBG_ERR

SHM_OPEN        = 0x00
SHM_READ        = 0x00
SHM_WRITE       = 0x01
SHM_OPEN_ALWAYS = 0x04
SHM_CREATE      = 0x08

ACTIVE_ICONS32  = 1
ACTIVE_ICONS18  = 2
ACTIVE_ICONS18W = 4
ACTIVE_CHECKBOX = 8

TAB_COUNT       = 4

META_I32_C      = 0
META_I32_W      = 4
META_I18_C      = 8
META_I18_W      = 12
META_I18W_C     = 16
META_I18W_W     = 20
META_CBOX_W     = 24
META_CBOX_H     = 28
META_CBOX_SIZE  = 32
META_SIZE       = 36

tab_masks       dd ACTIVE_ICONS32, ACTIVE_ICONS18, ACTIVE_ICONS18W, ACTIVE_CHECKBOX

; ====================================================================

meta_name       db "RESHARE_META", 0

icons32_path    db "/SYS/ICONS32.PNG", 0
icons18_path    db "/SYS/ICONS16.PNG", 0

; pointers to Image structures
icons32_image   dd 0
icons18_image   dd 0

; sizes of icons image data in bytes
size32          dd 0
size18          dd 0

; currenly selected i18w section
shared_i18w     dd 0

active_tab      dd ACTIVE_ICONS32

sc              system_colors

thread_info     process_information
thread_name     rb 16

include_debug_strings

; ====================================================================

I_END:
        rb      4096
        align   16
STACKTOP:

MEM:
