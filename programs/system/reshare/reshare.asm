; SPDX-License-Identifier: GPL-2.0-only
;
; Reshare - Shared Resources Daemon
;
; Copyright (C) 2024-2026 KolibriOS Team
; Copyright (C) 2024-2026 KolibriOS-NG Team

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

        mcall   SF_SYS_MISC, SSF_HEAP_INIT

        mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 1

        ; check if this is second instance of app. if so - run gui, else - run daemon
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, meta_name, 0, SHM_READ
        test    eax, eax
        jz      MODE_DAEMON
        mcall   SF_SYS_MISC, SSF_MEM_CLOSE, meta_name
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

        add     eax, WIN.H
        mov     esi, eax
        mcall   SF_GET_SCREEN_SIZE
        movzx   ecx, ax
        shr     eax, 16
        sub     eax, WIN.W
        shr     eax, 1
        shl     eax, 16
        add     eax, WIN.W
        mov     ebx, eax
        sub     ecx, esi
        shr     ecx, 1
        shl     ecx, 16
        add     ecx, esi
        mov     edx, [sc.work]
        add     edx, WIN_STYLE

        mcall   SF_CREATE_WINDOW, , , , , title

        ; visual dividers
        mcall   SF_DRAW_RECT, <PAD, GRID.W>, <BTN.Y + BTN.H + PAD, 1>, [sc.work_graph]
        mcall               ,              , <GRID.Y + GRID.H, 1>,

        ; info row
        mov     ecx, FONT_TYPE shl 24
        add     ecx, [sc.work_text]
        mcall   SF_DRAW_TEXT, <PAD, GRID.Y + GRID.H + PAD - 1>, , lb_info

        ; tabs labels
        mov     esi, tab_label_strs
        xor     edi, edi
        .tab_label_loop:
                mov     ebx, edi
                imul    ebx, TAB_STEP
                push    ebx
                add     ebx, PAD * 2
                shl     ebx, 16
                add     ebx, BTN.Y
                mov     edx, [esi]
                mcall   SF_DRAW_TEXT
                add     ebx, 1 shl 16   ; bold: x+1, same string
                mcall   SF_DRAW_TEXT
                pop     ebx
                add     ebx, PAD * 11
                shl     ebx, 16
                add     ebx, BTN.Y
                mov     edx, [esi + 4]
                mcall   SF_DRAW_TEXT
                add     esi, 8
                inc     edi
                cmp     edi, TAB_COUNT
                jb      .tab_label_loop

        ; tabs buttons
        mov     ecx, BTN.Y shl 16 + BTN.H
        mov     esi, ACTIVE_ICONS32
        xor     edi, edi
        .btn_loop:
                mov     ebx, edi
                imul    ebx, TAB_STEP
                add     ebx, PAD
                shl     ebx, 16
                add     ebx, BTN.W
                lea     edx, [esi + BTN_BASE + BTN_HIDE]
                mcall   SF_DEFINE_BUTTON
                add     esi, esi        ; 1→2→4→8 (next ACTIVE bit)
                inc     edi
                cmp     edi, TAB_COUNT
                jb      .btn_loop

        ; tabs content
        call    draw_tabs

        mcall   SF_REDRAW, SSF_END_DRAW

        jmp     .event_loop


.event_key:

        mcall   SF_GET_KEY
        cmp     ah, KEY_TAB
        jne     .event_loop

        shl     [active_tab], 1 ; * 2
        cmp     [active_tab], ACTIVE_CHECKBOX
        jbe     .tab_draw
        mov     [active_tab], 1

        .tab_draw:
        call    draw_tabs

        jmp     .event_loop


.event_button:

        mcall   SF_GET_BUTTON

        cmp     ah, 1
        je      .event_exit

        movzx   eax, ah
        sub     eax, BTN_BASE
        mov     [active_tab], eax
        call    draw_tabs

        jmp     .event_loop


.event_exit:
        mcall   SF_TERMINATE_PROCESS

; ====================================================================

draw_tabs:

        ; draw tab underlines; active tab gets highlight color
        xor     edi, edi
        mov     esi, 1  ; current tab bit (ACTIVE_ICONS32=1, then 2, 4, 8)

        .loop:
                mov     edx, [sc.work_dark]
                test    [active_tab], esi
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
                add     esi, esi        ; shift to next tab bit (1→2→4→8)
                cmp     edi, TAB_COUNT
                jb      .loop

        ; draw current tab content
        mcall   SF_DRAW_RECT, <GRID.X, GRID.W>, <GRID.Y, GRID.H>, [sc.work]

        mov     eax, [active_tab]
        bsf     eax, eax        ; 0=i32, 1=i18, 2=i18w, 3=cbox
        cmp     eax, 3
        je      .cbox
        lea     esi, [tab_icon_args + eax * 8]
        stdcall draw_tab_icons, [esi], [esi + 4]
        ret

        .cbox:
        stdcall draw_tab_cbox
        ret


proc draw_tab_icons stdcall, _shm_name, _meta_off
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, meta_name, 0, SHM_READ
        test    eax, eax
        jz      .done

        add     eax, [_meta_off]
        mov     ecx, [eax]      ; icon_c
        mov     edx, [eax + 4]  ; icon_w (always next field: _meta_off + 4)

        test    ecx, ecx
        jz      .close_meta
        test    edx, edx
        jz      .close_meta

        push    ecx
        push    edx
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, [_shm_name], 0, SHM_READ
        pop     edx
        pop     ecx

        test    eax, eax
        jz      .close_meta

        stdcall draw_tab_icons_grid, eax, ecx, edx

        mcall   SF_SYS_MISC, SSF_MEM_CLOSE, [_shm_name]

        .close_meta:
        mcall   SF_SYS_MISC, SSF_MEM_CLOSE, meta_name

        .done:
        ret
endp


proc draw_tab_icons_grid stdcall uses ebx esi edi, _icon_img, _icon_c, _icon_w
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

        ; y_step = CELL.H
        mov     edi, CELL.H

        xor     ecx, ecx

        .for_icons:
                cmp     ecx, [_icon_c]
                jae     .end_for_icons
                push    ecx

                ; SF_PUT_IMAGE_EXT      _icon_img+icon_size*index, <_icon_w, _icon_w>, <x+x_off, y+GRID_Y>, 32, 0, 0
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

                ; draw number with no leading zeros, centered in CELL.W
                mov     edx, [x]
                add     edx, GAP
                shl     edx, 16
                add     edx, [y]
                add     edx, [_icon_w]
                add     edx, GRID.Y + 2
                mov     esi, DRAWNUM_NOZERO
                add     esi, [sc.work_graph]

                pop     ecx
                mcall   SF_DRAW_NUMBER, DRAWNUM_DEC3, , , ,
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
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, lb_tab_cbox, 0, SHM_READ
        test    eax, eax
        jz      .done
        mov     ebx, eax
        mcall   SF_PUT_IMAGE, , \
                <CBOX_WIDTH, CBOX_HEIGHT>, \
                <(WIN.W - CBOX_WIDTH)/2, (WIN.H - CBOX_HEIGHT)/2>
        mcall   SF_SYS_MISC, SSF_MEM_CLOSE, lb_tab_cbox
        .done:
        ret
endp

; ====================================================================

MODE_DAEMON:

        stdcall dll.Load, LIBS

        ; load shared resources from files
        DEBUGF  DBG_INFO, "I: @reshare: loading resources...\n"
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

        mov     dword [edi + META_CBOX_W], CBOX_WIDTH
        mov     dword [edi + META_CBOX_H], CBOX_HEIGHT
        mov     dword [edi + META_CBOX_SIZE], CBOX_IMG_SIZE

        .meta_done:
        DEBUGF  DBG_INFO, "I: @reshare: starting in daemon mode\n"

        mcall   SF_SYS_MISC, SSF_MEM_OPEN, lb_tab_cbox, CBOX_IMG_SIZE, SHM_CREATE + SHM_WRITE
        test    eax, eax
        jz      .skip_cbox
                mov     edi, eax
                mov     esi, cbox_indexed
                mov     ecx, CBOX_PIXELS
                cld
                .cbox_expand:
                        movzx   ebx, byte [esi]
                        inc     esi
                        mov     eax, [cbox_palette + ebx * 4]
                        stosb                   ; B
                        shr     eax, 8
                        stosb                   ; G
                        shr     eax, 8
                        stosb                   ; R
                        dec     ecx
                        jnz     .cbox_expand
        .skip_cbox:

        cmp     [icons32_image], 0
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
                mov     [shared_i18], eax

                mcall   SF_SYS_MISC, SSF_MEM_OPEN, lb_tab_i18w, [size18], SHM_CREATE + SHM_WRITE
                test    eax, eax
                jz      .skip_i18
                mov     [shared_i18w], eax

                invoke  img.destroy, [icons18_image]
                mov     dword [icons18_image], 0
        .skip_i18:

        mcall   SF_SET_EVENTS_MASK, EVM_BACKGROUND

        .event_loop:
                push    [sc.work]
                mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors
                pop     eax
                cmp     eax, [sc.work]
                je      .wait_event
                cmp     [shared_i18], 0
                jz      .wait_event
                cmp     [shared_i18w], 0
                jz      .wait_event
                mov     esi, [shared_i18]
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


proc load_icons stdcall uses ebx ecx, _path, _img_ptr, _size_ptr
        invoke  img.from_file, [_path]
        test    eax, eax
        jz      .fail

        mov     ebx, [_img_ptr]
        mov     [ebx], eax
        mov     ebx, [eax + Image.Width]
        mov     ecx, [eax + Image.Height]
        imul    ecx, ebx
        shl     ecx, 2
        mov     ebx, [_size_ptr]
        mov     [ebx], ecx
        ret

        .fail:
        DEBUGF  DBG_ERR, "E: @reshare: error loading icons from %s\n", [_path]
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
        shr     ecx, 2
        cld
        rep     movsd
        .done:
        ret
endp


proc replace_2cols stdcall uses edi, _imgsrc, _imgsize, _col_old1, _col_new1, _col_old2, _col_new2
        mov     edx, [_imgsize]
        add     edx, [_imgsrc]
        mov     edi, [_imgsrc]
        .loop:
                cmp     edi, edx
                jae     .done
                mov     eax, [edi]
                cmp     eax, [_col_old1]
                jne     .check2
                mov     eax, [_col_new1]
                mov     [edi], eax
                jmp     .next
                .check2:
                cmp     eax, [_col_old2]
                jne     .next
                mov     eax, [_col_new2]
                mov     [edi], eax
                .next:
                add     edi, 4
                jmp     .loop
        .done:
        ret
endp

; ====================================================================

BORD            = 5
PAD             = 8
GAP             = 12

GRID_COLS       = 16
GRID_ROWS       = 9

GRID_Y          = BTN.Y + BTN.H + PAD * 2 + 1
TAB_STEP        = BTN.W + PAD * 2

WIN             RECT      0,       0, GRID.W + PAD * 2 + BORD * 2, GRID.H + GRID_Y + 1 + PAD + 10 + GAP
BTN             RECT      0, PAD + 4, CELL.W * 4 - PAD * 2       , 22
CELL            RECT      0,       0,         32 + PAD * 2       , 32 + 4 + 10 + PAD

GRID            RECT    PAD, GRID_Y, CELL.W * GRID_COLS - PAD * 2, CELL.H * GRID_ROWS

FONT_TYPE       = 0x90
WIN_STYLE       = 0x34000000   ; skinned window, draws itself

BTN_HIDE        = 0x60000000
BTN_BASE        = 10           ; first user-defined button ID

DRAWNUM_DEC3    = 0x00030000   ; SF_DRAW_NUMBER: decimal, 3 digits
DRAWNUM_NOZERO  = 0x10000000   ; SF_DRAW_NUMBER: no leading zeros

KEY_TAB         = 15           ; Tab key scan code

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
lb_tab_cbox_res db "13x13x24bpp", 0

; ====================================================================

DBG_ALL         = 0  ; all messages
DBG_INFO        = 1  ; info and errors
DBG_ERR         = 2  ; only errors

__DEBUG__       = 1
__DEBUG_LEVEL__ = DBG_ERR

SHM_READ        = 0x00
SHM_WRITE       = 0x01
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

; ====================================================================

meta_name       db "RESHARE_META", 0

icons32_path    db "/SYS/ICONS32.PNG", 0
icons18_path    db "/SYS/ICONS18.PNG", 0

; pointers to Image structures
icons32_image   dd 0
icons18_image   dd 0

; sizes of icons image data in bytes
size32          dd 0
size18          dd 0

; currenly selected i18w section
shared_i18      dd 0
shared_i18w     dd 0

active_tab      dd ACTIVE_ICONS32

tab_icon_args:
        dd lb_tab_i32,  META_I32_C      ; META_I32_W  = META_I32_C  + 4
        dd lb_tab_i18,  META_I18_C      ; META_I18_W  = META_I18_C  + 4
        dd lb_tab_i18w, META_I18W_C     ; META_I18W_W = META_I18W_C + 4

tab_label_strs:
        dd lb_tab_i32,  lb_tab_i32_res
        dd lb_tab_i18,  lb_tab_i18_res
        dd lb_tab_i18w, lb_tab_i18_res
        dd lb_tab_cbox, lb_tab_cbox_res

sc      system_colors

include_debug_strings

; ====================================================================

I_END:
        rb      4096
        align   16
STACKTOP:

MEM:
