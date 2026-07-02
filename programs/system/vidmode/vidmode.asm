; === VidMode - GUI for switching video modes via driver services ===
; Loads the service (sysfn 68.16), fetches the mode list (68.17 io_code=1),
; draws a window with the list; clicking a row selects it, Apply sets the mode
; (68.17 io_code=2, input = index).

use32
        org     0
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      mem
        dd      mem
        dd      param    ; get app param. if param[0]=='b' then show msg 'High resolution modes are available'
        dd      0

include "../../macros.inc"
include "../../KOSfuncs.inc"
include "../../gui_patterns.inc"
include "../../proc32.inc"
include "../../dll.inc"

macro cmpe a, b, c {     ; cmp a,b / je c  (as in shell.inc)
    cmp     a, b
    je      c
}

VBE_GET_MODES  = 1
VBE_SET_MODE   = 2
VBE_GET_INFO   = 3
MODES_BUF_SIZE = 4 + 48*24
REC_SIZE       = 24
MAX_DRV        = 5         ; max number of drivers in the list

LINE_H      = 22
LINE_W      = 150
SB_W        = 16               ; width of the vertical scrollbar
DEV_LINE_W  = 150 + SB_W + 1
RIGHT_COL_X = LINE_W + 50
MODE_VIS    = 10               ; visible mode rows (beyond that - scrollbar)
SB_X        = 13 + LINE_W  ; scrollbar right next to the list

start:
        ; === Probe ALL drivers: only those that returned modes make the list ===
        ; Loading a driver (68.16) does not touch hardware - just detect+RegService;
        ; the mode changes only on Apply. So we load all of them and offer a choice.
        mcall   68, 11                          ; init heap for dll.Load
        mov     dword [have_sb], 0
        stdcall dll.Load, imports               ; load box_lib.obj (scrollbar)
        test    eax, eax
        jnz     @f
        mov     dword [have_sb], 1
@@:
        mcall   40, 0x27                         ; events: +mouse (for scrollbar)
        mov     dword [cur_appl_drv], -1        ; history of applied modes (Backspace)
        mov     dword [prev_appl_drv], -1
        mov     dword [avail_count], 0
        mov     dword [probe_idx], 0
.probe_loop:
        mov     ebx, [probe_idx]
        cmp     ebx, NUM_DRIVERS
        jae     .probe_done
        mov     ecx, [name_table + ebx*4]
        mov     [probe_name], ecx
        mcall   68, 16, [probe_name]
        test    eax, eax
        jz      .probe_next
        mov     [drv_handle], eax
        mov     [tmp_handle], eax
        call    do_get_modes            ; does it have any modes?
        cmp     dword [modes_buf], 0
        je      .probe_next
        mov     ecx, [avail_count]
        cmp     ecx, MAX_DRV
        jae     .probe_next
        mov     eax, [tmp_handle]
        mov     [avail_handles + ecx*4], eax
        mov     ebx, [probe_idx]
        mov     eax, [name_table + ebx*4]
        mov     [avail_names + ecx*4], eax
        inc     dword [avail_count]
.probe_next:
        inc     dword [probe_idx]
        jmp     .probe_loop
.probe_done:
        cmp     dword [avail_count], 0
        je      .nodrv
        mov     dword [sel_drv], 0
        call    select_driver           ; select the first one by default
        jmp     .ready
.nodrv:
        mov     dword [modes_buf], 0
        mov     dword [info_buf], 0
        mov     dword [cur_drv_name], name_none
        call    format_ver
.ready:

red:
        call    load_icons
        call    draw_window
still:
        mcall   10
        cmp     eax, 1
        je      red
        cmp     eax, 2
        je      key
        cmp     eax, 3
        je      button
        cmp     eax, 6
        je      mouse
        jmp     still

key:
        mcall   2
        test    al, al                  ; al=0 -> key code in ah
        jnz     still
        cmp     ah, 9                   ; Tab       -> next driver     (whole window)
        je      .next_drv
        cmp     ah, 178                 ; Up        -> selection up     (list only)
        je      .sel_up
        cmp     ah, 177                 ; Down      -> selection down   (list only)
        je      .sel_down
        cmp     ah, 180                 ; Home      -> first mode       (list only)
        je      .sel_home
        cmp     ah, 181                 ; End       -> last mode        (list only)
        je      .sel_end
        cmp     ah, 8                   ; Backspace -> revert to previous mode (one-shot)
        je      .bksp
        jmp     still

.sel_up:
        cmp     dword [modes_buf], 0
        je      still                   ; no modes
        mov     eax, [selected]
        test    eax, eax
        jz      still                   ; already at the first mode
        dec     eax
        mov     [selected], eax
        call    keep_selected_visible   ; scroll if needed + redraw ONLY the list
        jmp     still

.sel_down:
        cmp     dword [modes_buf], 0
        je      still                   ; no modes
        mov     eax, [selected]
        inc     eax
        cmp     eax, dword [modes_buf]
        jae     still                   ; already at the last mode
        mov     [selected], eax
        call    keep_selected_visible   ; scroll if needed + redraw ONLY the list
        jmp     still

.sel_home:
        cmp     dword [modes_buf], 0
        je      still                   ; no modes
        mov     dword [selected], 0     ; jump to the first mode
        call    keep_selected_visible
        jmp     still

.sel_end:
        cmp     dword [modes_buf], 0
        je      still                   ; no modes
        mov     eax, dword [modes_buf]
        dec     eax                     ; jump to the last mode (count-1)
        mov     [selected], eax
        call    keep_selected_visible
        jmp     still

.next_drv:
        cmp     dword [avail_count], 0
        je      still                   ; no drivers to cycle
        mov     eax, [sel_drv]
        inc     eax
        cmp     eax, [avail_count]
        jb      @f
        xor     eax, eax                ; wrap around to the first driver
@@:
        mov     [sel_drv], eax
        call    select_driver
        mov     dword [selected], 0     ; reset mode selection when driver changes
        jmp     red                     ; driver changed -> redraw the whole window

.bksp:
        cmp     dword [bksp_armed], 0
        je      still                   ; not armed (no Apply since last revert)
        mov     eax, [prev_appl_drv]
        cmp     eax, [avail_count]
        jae     still
        mov     [sel_drv], eax
        call    select_driver
        mov     eax, [prev_appl_mode]
        mov     [selected], eax
        ; the previous becomes current; disarm Backspace (one-shot)
        mov     eax, [prev_appl_drv]
        mov     [cur_appl_drv], eax
        mov     eax, [prev_appl_mode]
        mov     [cur_appl_mode], eax
        mov     dword [bksp_armed], 0
        call    set_mode_now
        jmp     red

button:
        mcall   17
        shr     eax, 8
        cmp     eax, 1
        je      exit
        cmp     eax, 30
        je      apply
        cmp     eax, 200
        jae     .drvsel                 ; 200+idx - driver selection
        cmp     eax, 100
        jae     .modesel                ; 100+idx - mode selection
        jmp     still
.modesel:
        sub     eax, 100                ; visible row
        add     eax, [scroll_modes.position]    ; -> absolute mode index
        cmp     eax, dword [modes_buf]
        jae     still
        mov     [selected], eax
        call    draw_mode_rows          ; clicked row is already visible -> redraw ONLY the list
        jmp     still
.drvsel:
        sub     eax, 200
        cmp     eax, [avail_count]
        jae     still
        mov     [sel_drv], eax
        call    select_driver
        mov     dword [selected], 0     ; reset mode selection when driver changes
        jmp     red

apply:
        ; history: prev <- currently applied; cur <- new selection
        mov     eax, [cur_appl_drv]
        mov     [prev_appl_drv], eax
        mov     eax, [cur_appl_mode]
        mov     [prev_appl_mode], eax
        mov     eax, [sel_drv]
        mov     [cur_appl_drv], eax
        mov     eax, [selected]
        mov     [cur_appl_mode], eax
        ; arm Backspace only if there is somewhere to revert to
        mov     dword [bksp_armed], 0
        cmp     dword [prev_appl_drv], -1
        je      @f
        mov     dword [bksp_armed], 1
@@:
        call    set_mode_now
        jmp     still

exit:
        mcall   -1

; Wheel/dragging of the mode list scrollbar
mouse:
        mcall   SF_MOUSE_GET, SSF_SCROLL_DATA
        and     eax, 0xFFFF
        cmpe    eax, 65535, .scroll_up
        cmpe    eax, 0, @f

.scroll_down:                           ; wheel down -> position +1
        cmp     dword [modes_buf], MODE_VIS
        jbe     still                   ; list does not scroll
        mov     eax, [scroll_modes.position]
        mov     ecx, dword [modes_buf]
        sub     ecx, MODE_VIS           ; max scroll position
        cmp     eax, ecx
        jae     still                   ; already at the very bottom
        inc     dword [scroll_modes.position]
        jmp     .wheel_redraw

.scroll_up:                             ; wheel up -> position -1
        cmp     dword [scroll_modes.position], 0
        jbe     still                   ; already at the top
        dec     dword [scroll_modes.position]

.wheel_redraw:                          ; wheel moves the slider itself (via v_draw)
        call    draw_mode_rows          ; rows for the new offset
        cmp     dword [have_sb], 0
        je      still                   ; without box_lib - just no slider
        mov     dword [scroll_modes.all_redraw], 0
        push    dword scroll_modes
        call    [scrollbar_v_draw]      ; move the slider to the new position
        jmp     still

@@:
        cmp     dword [have_sb], 0
        je      still
        cmp     dword [modes_buf], 0
        je      still
        mov     eax, [scroll_modes.max_area]
        cmp     eax, [scroll_modes.cur_area]
        jbe     still                   ; list fits entirely - no scroll needed
        push    dword scroll_modes
        call    [scrollbar_v_mouse]     ; draws the slider ITSELF on click/drag
        cmp     dword [scroll_modes.redraw], 0
        je      still
        mov     dword [scroll_modes.redraw], 0
        ; IMPORTANT: do NOT call scrollbar_v_draw here - it would redraw the slider from the
        ; quantized position and "snap" it off the cursor. Redraw only the list (like opendial).
        call    draw_mode_rows
        jmp     still

; Apply mode [drv_handle]/[selected] (SET_MODE), without changing history
set_mode_now:
        mov     eax, [drv_handle]
        test    eax, eax
        jz      .ret
        mov     [ioctl_handle], eax
        mov     dword [ioctl_iocode], VBE_SET_MODE
        mov     eax, [selected]
        mov     [set_index], eax
        mov     dword [ioctl_input], set_index
        mov     dword [ioctl_inpsize], 4
        mov     dword [ioctl_output], 0
        mov     dword [ioctl_outsize], 0
        mcall   68, 17, ioctl
.ret:
        ret

; =====================================================================
; IOCTL helpers (use [drv_handle])
do_get_modes:
        mov     eax, [drv_handle]
        mov     [ioctl_handle], eax
        mov     dword [ioctl_iocode], VBE_GET_MODES
        mov     dword [ioctl_input], 0
        mov     dword [ioctl_inpsize], 0
        mov     dword [ioctl_output], modes_buf
        mov     dword [ioctl_outsize], MODES_BUF_SIZE
        mcall   68, 17, ioctl
        ret

do_get_info:
        mov     eax, [drv_handle]
        mov     [ioctl_handle], eax
        mov     dword [ioctl_iocode], VBE_GET_INFO
        mov     dword [ioctl_input], 0
        mov     dword [ioctl_inpsize], 0
        mov     dword [ioctl_output], info_buf
        mov     dword [ioctl_outsize], 8
        mcall   68, 17, ioctl
        ret

; Select driver [sel_drv] from the available list: handle/name/modes/info/version
select_driver:
        mov     eax, [sel_drv]
        mov     ecx, [avail_handles + eax*4]
        mov     [drv_handle], ecx
        mov     ecx, [avail_names + eax*4]
        mov     [cur_drv_name], ecx
        mov     dword [is_kms], 0       ; show mode refresh rate only for kms
        cmp     ecx, name_kms
        jne     .not_kms
        mov     dword [is_kms], 1
.not_kms:
        call    do_get_info
        call    do_get_modes
        call    format_ver
        mov     dword [scroll_modes.position], 0   ; reset scroll when driver changes
        ret


load_icons:
        mcall   SF_SYS_MISC, SSF_HEAP_INIT
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, icons18_name, , 0
        mov     [icons18_img], eax
        ret
; =====================================================================
draw_window:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW

        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors

        ; --- height of the driver block (min. 1 row for the message) ---
        mov     eax, [avail_count]
        test    eax, eax
        jnz     @f
        mov     eax, 1
@@:
        imul    eax, LINE_H
        mov     [drv_block_h], eax

        ; --- number of mode rows (for window height) ---
        mov     ecx, dword [modes_buf]
        test    ecx, ecx
        jnz     @f
        mov     ecx, 1
@@:
        cmp     ecx, MODE_VIS
        jbe     @f
        mov     ecx, MODE_VIS
@@:
        imul    ecx, LINE_H

        mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT,
        mov     ebx, eax ;skin_h

        ; total = 26 (driver header) + driver block + 12 + 18 (mode header) + modes + 50
        cmp     dword [modes_buf], 0
        jne     .h_modes
        ; no drivers/modes - window sized to the driver block height
        mov     eax, [drv_block_h]
        add     eax, 26 + 26           ; driver header + bottom margin
        jmp     .h_done
.h_modes:
        ; height = mode_y0 + max(18+viewport, right column ~118) + bottom margin
        add     ecx, 18                ; list extends from mode_y0+18 down by viewport
        cmp     ecx, 118
        jae     @f
        mov     ecx, 118               ; few rows - right column (Apply+notes) is lower
@@:
        mov     eax, [drv_block_h]
        add     eax, 26 + 20           ; mode_y0 (must match .mode_section)
        add     eax, ecx
        add     eax, 26                ; bottom margin
.h_done:
        add     eax, ebx               ; + skin height
        mov     ecx, 100
        shl     ecx, 16
        add     ecx, eax

        mov     edx, [sc.work]
        add     edx, 0x33000000
        mcall   SF_CREATE_WINDOW, 100*65536+RIGHT_COL_X+160, ecx, , , title

        mov     ebx, [icons18_img]
        add     ebx, 25*18*18*4
        mcall   SF_PUT_IMAGE_EXT, , <18,18>, <16, 6>, 32

        ; ===== Section 1: driver selection =====
        mov     ecx, [sc.work_text]
        add     ecx, 0x90000000
        mcall   SF_DRAW_TEXT, (15+18+6)*65536+7, ecx, set_driver
        cmp     dword [avail_count], 0
        jne     .drvloop_init
        mcall   SF_DRAW_TEXT, 15*65536+35, 0x90FF0000, msg_nodrv
        jmp     .mode_section
.drvloop_init:
        mov     dword [cur_idx], 0
.drvloop:
        mov     eax, [cur_idx]
        cmp     eax, [avail_count]
        jae     .mode_section
        imul    eax, LINE_H                 ; y = 26 + idx*18
        add     eax, 26
        mov     [row_y], eax
        ; highlight the selected driver
        mov     ecx, [row_y]
        shl     ecx, 16
        or      ecx, LINE_H
        mov     edx, 0xFFFFFF
        mov     eax, [cur_idx]
        cmp     eax, [sel_drv]
        jne     @f
        mov     edx, 0x66DD66

@@:
        mcall   SF_DRAW_RECT, 13*65536+DEV_LINE_W, ecx
        ; driver name
        mov     eax, [cur_idx]
        mov     edx, [avail_names + eax*4]
        mov     ebx, [row_y]
        add     ebx, (LINE_H-16)/2
        or      ebx, 18*65536
        mcall   SF_DRAW_TEXT, ebx, 0x90000000, edx
        ; invisible button id = 200+idx
        mov     ecx, [row_y]
        shl     ecx, 16
        or      ecx, LINE_H
        mov     edx, [cur_idx]
        add     edx, 200
        or      edx, 0x40000000
        mcall   SF_DEFINE_BUTTON, 13*65536+DEV_LINE_W, ecx, edx
        inc     dword [cur_idx]
        jmp     .drvloop

        ; ===== Section 2: mode selection =====
.mode_section:
        mov     eax, [drv_block_h]
        add     eax, 26 + 20
        mov     [mode_y0], eax
        ; --- header + icon + driver frame: only when drivers exist (Task 2/3) ---
        cmp     dword [avail_count], 0
        je      .skip_mode_hdr
        ; --- frame around the driver list (Task 3) ---
        mov     dword [fr_x], 12
        mov     dword [fr_y], 25
        mov     dword [fr_w], DEV_LINE_W+1
        mov     eax, [drv_block_h]
        inc     eax
        mov     [fr_h], eax
        call    draw_frame
        mov     ebx, [mode_y0]
        or      ebx, (15+18+6)*65536
        dec     ebx
        mov     ecx, [sc.work_text]
        add     ecx, 0x90000000
        mcall   SF_DRAW_TEXT, ebx, ecx, sel_mode
        mov     ebx, [mode_y0]
        or      ebx, RIGHT_COL_X*65536
        dec     ebx
        mcall   SF_DRAW_TEXT, ebx, ecx, verbuf
        mov     edx, 16
        shl     edx, 16
        add     edx, [mode_y0]
        sub     edx, 3
        mov     ebx, [icons18_img]
        add     ebx, 61*18*18*4
        mcall   SF_PUT_IMAGE_EXT, , <18,18>, , 32
.skip_mode_hdr:
        mov     ecx, dword [modes_buf]
        test    ecx, ecx
        jz      .done
        ; visible = min(count, MODE_VIS)
        mov     eax, ecx
        cmp     eax, MODE_VIS
        jbe     .vis_ok
        mov     eax, MODE_VIS
.vis_ok:
        mov     [mode_vis_count], eax
        mov     eax, [mode_y0]
        add     eax, 18
        mov     [mode_list_y0], eax
        ; --- scrollbar runtime geometry ---
        mov     eax, [mode_list_y0]
        dec     ax
        mov     word [scroll_modes.start_y], ax
        mov     eax, [mode_vis_count]
        imul    eax, LINE_H
        add     eax, 2
        mov     word [scroll_modes.size_y], ax
        mov     eax, dword [modes_buf]
        mov     [scroll_modes.max_area], eax
        mov     eax, [mode_vis_count]
        mov     [scroll_modes.cur_area], eax
        ; clamp position to [0, count-visible]
        mov     eax, dword [modes_buf]
        sub     eax, [mode_vis_count]
        jns     .clamp_ok
        xor     eax, eax
.clamp_ok:
        cmp     [scroll_modes.position], eax
        jbe     .pos_ok
        mov     [scroll_modes.position], eax
.pos_ok:
        call    draw_mode_rows
        ; --- frame around the mode list (Task 3) ---
        mov     dword [fr_x], 12
        mov     eax, [mode_list_y0]
        dec     eax
        mov     [fr_y], eax
        mov     dword [fr_w], LINE_W+1
        mov     eax, [mode_vis_count]
        imul    eax, LINE_H
        inc     eax
        mov     [fr_h], eax
        call    draw_frame
        ; --- single mode: a blue bar (selection color) instead of a useless scrollbar ---
        cmp     dword [modes_buf], 1
        jne     .draw_sb
        movzx   ecx, word [scroll_modes.start_y]
        add     ecx, 2
        shl     ecx, 16
        movzx   eax, word [scroll_modes.size_y]
        or      ecx, eax
        mov     edx, [sc.work]
        mcall   13, SB_X*65536+SB_W
        jmp     .done_list
.draw_sb:
        ; --- vertical scrollbar ---
        cmp     dword [have_sb], 0
        je      .done_list
        mov     dword [scroll_modes.all_redraw], 1
        push    dword scroll_modes
        call    [scrollbar_v_draw]
        mov     dword [scroll_modes.all_redraw], 0
.done_list:
        ; Apply button below the mode list
        mov     eax, [mode_y0]
        add     eax, 18 + 8
        mov     [row_y], eax
        mov     ecx, [row_y]
        shl     ecx, 16
        or      ecx, 26
        mcall   SF_DEFINE_BUTTON, RIGHT_COL_X*65536+110, ecx, 30, 0x44AA44
        mov     ebx, [row_y]
        add     ebx, 6
        or      ebx, (RIGHT_COL_X+35)*65536
        mcall   SF_DRAW_TEXT, ebx, 0x90FFFFFF, btn_apply

        mov     ecx, [sc.work_text]
        add     ecx, 0x90000000

        add     ebx, -33*65536+35
        mcall   SF_DRAW_TEXT, ebx, ecx, bs_notice_1
        add     ebx, 20
        mcall   SF_DRAW_TEXT, ebx, ecx, bs_notice_2
        add     ebx, 20
        mcall   SF_DRAW_TEXT, ebx, ecx, bs_notice_3
.done:
        mcall   SF_REDRAW, SSF_END_DRAW
        ret

; =====================================================================
; Redraw the mode viewport rows (clear + rows + buttons).
; offset = [scroll_modes.position], visible count = [mode_vis_count].
draw_mode_rows:
        mov     eax, [mode_vis_count]
        imul    eax, LINE_H
        mov     ecx, [mode_list_y0]
        shl     ecx, 16
        or      ecx, eax
        mov     edx, [sc.work]
        mcall   13, 13*65536+LINE_W
        mov     dword [cur_idx], 0
.dmr_row:
        mov     eax, [cur_idx]
        cmp     eax, [mode_vis_count]
        jae     .dmr_done
        imul    eax, LINE_H
        add     eax, [mode_list_y0]
        mov     [row_y], eax
        mov     eax, [scroll_modes.position]
        add     eax, [cur_idx]
        cmp     eax, dword [modes_buf]
        jae     .dmr_next
        mov     [mode_abs], eax
        mov     edx, 0xFFFFFF
        cmp     eax, [selected]
        jne     @f
        mov     edx, 0x77C2FF
@@:
        mov     ecx, [row_y]
        shl     ecx, 16
        or      ecx, LINE_H
        mcall   13, 13*65536+LINE_W
        mov     eax, [mode_abs]
        imul    eax, REC_SIZE
        lea     esi, [modes_buf + 4 + eax]
        call    format_mode
        mov     ebx, [row_y]
        add     ebx, (LINE_H-16)/2
        or      ebx, 20*65536
        mcall   SF_DRAW_TEXT, ebx, 0x10000000, linebuf, [line_len]
        mov     ecx, [row_y]
        shl     ecx, 16
        or      ecx, LINE_H
        mov     edx, [cur_idx]
        add     edx, 100
        or      edx, 0x40000000
        mcall   SF_DEFINE_BUTTON, 13*65536+LINE_W, ecx, edx
.dmr_next:
        inc     dword [cur_idx]
        jmp     .dmr_row
.dmr_done:
        ret

; =====================================================================
; Keyboard moved [selected]: scroll the viewport so the selected row stays
; visible, then redraw ONLY the mode list (and the scrollbar slider if the
; offset actually changed). No SF_REDRAW BEGIN/END -> the rest of the window
; is untouched, exactly like wheel scrolling.
keep_selected_visible:
        mov     eax, [selected]
        cmp     eax, [scroll_modes.position]
        jae     .check_bottom
        mov     [scroll_modes.position], eax    ; selected above the viewport - scroll up
        jmp     .redraw
.check_bottom:
        mov     ecx, [scroll_modes.position]
        add     ecx, [mode_vis_count]           ; first row below the viewport
        cmp     eax, ecx
        jb      .redraw                         ; still visible - offset unchanged
        inc     eax
        sub     eax, [mode_vis_count]           ; position = selected - visible + 1
        mov     [scroll_modes.position], eax
.redraw:
        call    draw_mode_rows
        cmp     dword [have_sb], 0
        je      .ret                            ; no box_lib - list only
        cmp     dword [modes_buf], 1
        jbe     .ret                            ; single mode - no scrollbar
        mov     eax, [scroll_modes.max_area]
        cmp     eax, [scroll_modes.cur_area]
        jbe     .ret                            ; whole list fits - no scrollbar
        mov     dword [scroll_modes.all_redraw], 0
        push    dword scroll_modes
        call    [scrollbar_v_draw]              ; move slider to the new position
.ret:
        ret

; =====================================================================
; 1px frame around the perimeter: [fr_x],[fr_y],[fr_w],[fr_h]; color sc.work_graph
draw_frame:
        mov     edx, [sc.work_graph]    ; top
        mov     ebx, [fr_x]
        shl     ebx, 16
        or      ebx, [fr_w]
        mov     ecx, [fr_y]
        shl     ecx, 16
        or      ecx, 1
        mcall   13
        mov     edx, [sc.work_graph]    ; bottom
        mov     ebx, [fr_x]
        shl     ebx, 16
        or      ebx, [fr_w]
        inc     ebx
        mov     ecx, [fr_y]
        add     ecx, [fr_h]
        shl     ecx, 16
        or      ecx, 1
        mcall   13
        mov     edx, [sc.work_graph]    ; left
        mov     ebx, [fr_x]
        shl     ebx, 16
        or      ebx, 1
        mov     ecx, [fr_y]
        shl     ecx, 16
        or      ecx, [fr_h]
        mcall   13
        mov     edx, [sc.work_graph]    ; right
        mov     ebx, [fr_x]
        add     ebx, [fr_w]
        shl     ebx, 16
        or      ebx, 1
        mov     ecx, [fr_y]
        shl     ecx, 16
        or      ecx, [fr_h]
        mcall   13
        ret

; =====================================================================
; esi -> mode record (w,h,bpp,...). Builds linebuf, length in [line_len]
format_mode:
        mov     edi, linebuf
        mov     eax, [esi+0]            ; width
        call    PutDec
        mov     al, 'x'
        stosb
        mov     eax, [esi+4]            ; height
        call    PutDec
        mov     al, 'x'
        stosb
        mov     eax, [esi+8]            ; bpp
        call    PutDec
        ; --- refresh rate: only for the kms driver (freq in word +14 of the record) ---
        cmp     dword [is_kms], 0       ; not kms -> no Hz (others have junk at +14)
        je      .fm_done
        movzx   ebx, word [esi+14]      ; refresh, Hz
        test    ebx, ebx
        jz      .fm_done
        mov     al, ' '
        stosb
        mov     eax, ebx
        call    PutDec
        mov     al, 'H'
        stosb
        mov     al, 'z'
        stosb
.fm_done:
        mov     eax, edi
        sub     eax, linebuf
        mov     [line_len], eax
        ret

; eax -> decimal into [edi] (edi advances)
PutDec:
        push    ebx ecx edx
        mov     ebx, 10
        xor     ecx, ecx
        test    eax, eax
        jnz     .d
        mov     al, '0'
        stosb
        jmp     .e
.d:
        xor     edx, edx
        div     ebx
        push    edx
        inc     ecx
        test    eax, eax
        jnz     .d
.p:
        pop     edx
        mov     al, dl
        add     al, '0'
        stosb
        dec     ecx
        jnz     .p
.e:
        pop     edx ecx ebx
        ret

; info_buf[0..1] = VbeVersion (word) -> builds "VESA X.Y" in verbuf
format_ver:
        movzx   eax, byte [info_buf+1]  ; major; 0 -> native driver (not VBE)
        test    al, al
        jnz     .vesa
        mov     dword [verbuf], 'nati'
        mov     word [verbuf+4], 've'
        mov     byte [verbuf+6], 0
        ret
.vesa:
        mov     edi, verbuf
        mov     dword [edi], 'VESA'
        add     edi, 4
        mov     byte [edi], ' '
        inc     edi
        movzx   eax, byte [info_buf+1]  ; major (high byte of version)
        add     al, '0'
        mov     [edi], al
        inc     edi
        mov     byte [edi], '.'
        inc     edi
        movzx   eax, byte [info_buf+0]  ; minor (low byte of version)
        add     al, '0'
        mov     [edi], al
        mov     byte [edi+1], 0
        ret

; =====================================================================
name_vesa20     db 'vesa20', 0
name_clgd       db 'clgd54xx', 0
name_s3         db 's3vid', 0
name_vintel     db 'vidintel', 0
name_kms        db 'kms', 0
name_none       db '(no driver)', 0

title           db 'Video Mode Switcher', 0
set_driver      db 'Select driver:',0
sel_mode        db 'Select a mode:',0
btn_apply       db 'Apply',0
bs_notice_1     db 'Press Backspace',0
bs_notice_2     db 'to return to the',0
bs_notice_3     db 'previous mode.',0
msg_nodrv       db 'No drivers with modes!',0

icons18_name    db 'ICONS18W', 0

param           rb 256

align 4
; All drivers that we probe (detection happens inside each driver).
name_table      dd name_vesa20, name_clgd, name_s3, name_vintel, name_kms
NUM_DRIVERS     = ($ - name_table) / 4

sc        system_colors

align 4
drv_handle      dd 0
cur_drv_name    dd 0
selected        dd 0
set_index       dd 0
ioctl:
ioctl_handle    dd 0
ioctl_iocode    dd 0
ioctl_input     dd 0
ioctl_inpsize   dd 0
ioctl_output    dd 0
ioctl_outsize   dd 0

align 4
imports:
        library boxlib, 'box_lib.obj'
        import  boxlib, scrollbar_v_draw,  'scrollbar_v_draw', \
                        scrollbar_v_mouse, 'scrollbar_v_mouse'

align 4
scroll_modes:
.size_x     dw SB_W
.start_x    dw SB_X
.size_y     dw 100
.start_y    dw 50
.btn_high   dd 15
.type       dd 0
.max_area   dd 0
.cur_area   dd 0
.position   dd 0
.bckg_col   dd 0xEEEEEE
.frnt_col   dd 0xBBDDFF
.line_col   dd 0x888888
.redraw     dd 0
.delta      dw 0
.delta2     dw 0
.r_size_x   dw 0
.r_start_x  dw 0
.r_size_y   dw 0
.r_start_y  dw 0
.m_pos      dd 0
.m_pos_2    dd 0
.m_keys     dd 0
.run_size   dd 0
.position2  dd 0
.work_size  dd 0
.all_redraw dd 0
.ar_offset  dd 1

i_end:
; ---- uninitialized data ----
cur_idx         rd 1
cur_rec         rd 1
row_y           rd 1
line_len        rd 1
drv_block_h     rd 1
mode_y0         rd 1
have_sb         rd 1
fr_x            rd 1
fr_y            rd 1
fr_w            rd 1
fr_h            rd 1
mode_abs        rd 1
mode_vis_count  rd 1
mode_list_y0    rd 1
is_kms          rd 1
avail_handles   rd MAX_DRV
avail_names     rd MAX_DRV
avail_count     rd 1
sel_drv         rd 1
probe_idx       rd 1
probe_name      rd 1
tmp_handle      rd 1
cur_appl_drv    rd 1        ; currently applied (driver,mode)
cur_appl_mode   rd 1
prev_appl_drv   rd 1        ; previously applied - for Backspace
prev_appl_mode  rd 1
bksp_armed      rd 1        ; 1 = Backspace armed (one-shot, until next Apply)
info_buf        rb 8       ; [dd vbe_version][dd pci_lfb] from the driver
verbuf          rb 16      ; "VESA X.Y"
linebuf         rb 32
icons18_img     rd 1
modes_buf       rb MODES_BUF_SIZE
                rb 2048                 ; stack
align 16
mem:
