format binary as ""

use32
org 0x0

db 'MENUET01'
dd 0x01, START, I_END, E_END, stacktop, __params, sys_path

;-----------------------------------------------------------------------------

__DEBUG__        = 0
__DEBUG_LEVEL__  = 1

LG_TRACE equ 1

include '../../../config.inc'
include '../../../proc32.inc'
include '../../../macros.inc'
include '../../../KOSfuncs.inc'
include '../../../dll.inc'
include '../../../debug-fdo.inc'
include '../../../develop/libraries/libs-dev/libimg/libimg.inc'


KEY_MOVE_PIXELS   = 50
SCROLL_WIDTH_SIZE = 15
AR_OFFSET         = 10

MIN_WINDOW_WIDTH  = 50+25*numimages
MIN_WINDOW_HEIGHT = 100
TOOLBAR_HEIGHT    = 31
CANVAS_PADDING    = 5
;-----------------------------------------------------------------------------

START:
        mcall   SF_SYS_MISC, SSF_HEAP_INIT
        mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 1  ; set kbd mode to scancodes
        mcall   SF_SET_EVENTS_MASK, EVM_REDRAW or EVM_KEY or EVM_BUTTON or \
                EVM_MOUSE or EVM_MOUSE_FILTER

        stdcall dll.Load, @IMPORT
        or      eax, eax
        jnz     exit

        invoke  sort.START, 1

        mov     ecx, 1  ; for 15.4: 1 = tile
        cmp     word[__params], '\T'
        jz      set_bgr
        inc     ecx     ; for 15.4: 2 = stretch
        cmp     word[__params], '\S'
        jz      set_bgr

        cmp     byte[__params], 0
        jz      @f
        mov     esi, __params
        mov     edi, path
        mov     ecx, 4096/4
        rep movsd
        mov     byte[edi-1], 0
@@:
; OpenDialog initialisation
        push    dword OpenDialog_data
        call    [OpenDialog_Init]

; initialize keyboard handling
        invoke  ini_get_shortcut, inifilename, aKivSection, aNext, -1, next_mod
        mov     [next_key], eax
        invoke  ini_get_shortcut, inifilename, aKivSection, aPrev, -1, prev_mod
        mov     [prev_key], eax
        invoke  ini_get_shortcut, inifilename, aKivSection, aSlide, -1, slide_mod
        mov     [slide_key], eax
        invoke  ini_get_shortcut, inifilename, aKivSection, aTglbar, -1, tglbar_mod
        mov     [tglbar_key], eax

        invoke  ini_get_int, inifilename, aKivSection, aWinX, 100
        mov     [window.left], eax
        invoke  ini_get_int, inifilename, aKivSection, aWinY, 150
        mov     [window.top], eax
        invoke  ini_get_int, inifilename, aKivSection, aWinW, 0
        mov     [window.width], eax
        invoke  ini_get_int, inifilename, aKivSection, aWinH, 0
        mov     [window.height], eax

        cmp     byte[__params], 0
        jnz     params_given

        mov     [OpenDialog_data.draw_window], draw_window_fake

; OpenDialog Open
        push    dword OpenDialog_data
        call    [OpenDialog_Start]

        cmp     [OpenDialog_data.status], 1
        jnz     exit

        mov     [OpenDialog_data.draw_window], draw_window

        mov     esi, path
        mov     edi, __params
        mov     ecx, 4096/4
        rep movsd
        mov     byte[edi-1], 0
        jmp     params_given

set_bgr:
        mcall   SF_BACKGROUND_SET, SSF_MODE_BG
        stdcall load_image, __params+4
        jc      exit
        call    set_as_bgr
        jmp     exit

params_given:
        mov     esi, __params
        push    esi
        call    find_last_name_component
        call    load_directory

        pop     eax
        stdcall load_image, eax
        jc      exit

;-----------------------------------------------------------------------------

redraw_all:
        call    draw_window

still:
        mov     eax, [orig_image]
        test    [eax+Image.Flags], Image.IsAnimated
        movi    eax, SF_WAIT_EVENT
        jz      .wait_event
        mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
        mov     edx, [cur_frame]
        mov     ebx, [cur_frame_time]
        add     ebx, [edx+Image.Delay]
        sub     ebx, eax
        cmp     ebx, [edx+Image.Delay]
        jna     @f
        call    red_update_frame
        jmp     still
@@:
        test    ebx, ebx
        jnz     @f
        call    red_update_frame
        jmp     still
@@:
        movi    eax, SF_WAIT_EVENT_TIMEOUT
.wait_event:
        mcall
        dec     eax
        jns     @f
        call    red_update_frame
        jmp     still
@@:
        jz      redraw_all
        dec     eax
        jz      key
        dec     eax
        jz      button

mouse:
        mov     eax, [need_scrollbar_v]
        add     eax, [need_scrollbar_h]
        test    eax, eax
        jz      .done
        mov     [pict_moved], 0

        invoke  scrollbar_vert_mouse, scroll_bar_data_vertical
        invoke  scrollbar_hort_mouse, scroll_bar_data_horizontal
        xor     ecx, ecx
        mov     eax, [scroll_bar_data_vertical.position]
        cmp     [pict.top], eax
        mov     [pict.top], eax
        setnz   cl
        mov     eax, [scroll_bar_data_horizontal.position]
        cmp     [pict.left], eax
        mov     [pict.left], eax
        setnz   ch
        test    ecx, ecx
        jz      @f
        call    draw_view
        call    draw_onimage_decorations
@@:

        ; check for scroll
        mcall   SF_MOUSE_GET, SSF_SCROLL_DATA
        test    eax, eax
        jz      .no_scroll
        movsx   ecx, ax
        shl     ecx, 4
        sar     eax, 16
        shl     eax, 4
        stdcall move_pictport, eax, ecx
        mov     [pict_moved], eax
        jmp     .mouse_done
.no_scroll:

        ; get cursor coordinates in window
        mcall   SF_MOUSE_GET, SSF_WINDOW_POSITION
        movsx   ebx, ax
        cmp     ebx, 0
        jge     @f
        add     eax, 0x10000
@@:
        mov     ecx, [mouse_pos]
        cmp     eax, ecx
        jz      .no_mouse_move
        mov     [mouse_pos], eax

        cmp     [pict_drag], 1
        jnz     .no_mouse_move
        sar     eax, 16
        movsx   edx, cx
        sar     ecx, 16
        sub     eax, ecx
        sub     ebx, edx
        neg     eax
        neg     ebx
        stdcall move_pictport, eax, ebx
        mov     [pict_moved], eax
        jmp     .no_mouse_move
.no_mouse_move:

        ; check buttons
        mcall   SF_MOUSE_GET, SSF_BUTTON
        mov     ecx, eax
        xor     ecx, [mouse_buttons]
        mov     [mouse_buttons], eax
        test    ecx, 0x01
        jz      .left_button_handled
        test    eax, 0x01
        jnz     .left_button_down
.left_button_up:
        mov     [pict_drag], 0
        jmp     .left_button_handled
.left_button_down:
        mov     ecx, [mouse_pos]
        movzx   edx, cx
        sar     ecx, 16
        mov     ebx, [canvas_abs_top]
        add     ebx, [view.top]
        cmp     ebx, edx
        jg      .left_click_pict_done
        add     ebx, [view.height]
        cmp     ebx, edx
        jl      .left_click_pict_done
        mov     ebx, [canvas_abs_left]
        add     ebx, [view.left]
        cmp     ebx, ecx
        jg      .left_click_pict_done
        add     ebx, [view.width]
        cmp     ebx, ecx
        jl      .left_click_pict_done
        mov     [pict_drag], 1
        jmp     .left_button_handled
.left_click_pict_done:

.left_button_handled:
.mouse_done:
        mov     eax, [pict_moved]
        test    eax, eax
        jz      .done
        stdcall update_scrollbars, eax
        call    draw_view
        call    draw_onimage_decorations
.done:
        jmp     still

key:
        xor     esi, esi
keyloop:
        mcall   SF_GET_KEY
        test    al, al
        jnz     keyloopdone
        shr     eax, 8
        mov     ecx, eax
        mcall   SF_KEYBOARD, SSF_GET_CONTROL_KEYS
        mov     edx, next_mod
        call    check_shortcut
        jz      .next
        add     edx, prev_mod-next_mod
        call    check_shortcut
        jz      .prev
        add     edx, slide_mod-prev_mod
        call    check_shortcut
        jz      .slide
        add     edx, tglbar_mod-slide_mod
        call    check_shortcut
        jz      .tglbar

        mov     edx, scale_none_mod
        call    check_shortcut
        jz      .set_scale_none
        add     edx, scale_fit_min_mod-scale_none_mod
        call    check_shortcut
        jz      .set_scale_fit_min
        add     edx, move_pictport_left_1_mod-scale_fit_min_mod
        call    check_shortcut
        jz      .move_pictport_left
        add     edx, move_pictport_left_2_mod-move_pictport_left_1_mod
        call    check_shortcut
        jz      .move_pictport_left
        add     edx, move_pictport_right_1_mod-move_pictport_left_2_mod
        call    check_shortcut
        jz      .move_pictport_right
        add     edx, move_pictport_right_2_mod-move_pictport_right_1_mod
        call    check_shortcut
        jz      .move_pictport_right
        add     edx, move_pictport_up_1_mod-move_pictport_right_2_mod
        call    check_shortcut
        jz      .move_pictport_up
        add     edx, move_pictport_up_2_mod-move_pictport_up_1_mod
        call    check_shortcut
        jz      .move_pictport_up
        add     edx, move_pictport_down_1_mod-move_pictport_up_2_mod
        call    check_shortcut
        jz      .move_pictport_down
        add     edx, move_pictport_down_2_mod-move_pictport_down_1_mod
        call    check_shortcut
        jz      .move_pictport_down

        cmp     cl, 1 ; Esc
        jz      .esc
        jmp     keyloop
.esc:
        test    [bSlideShow], 1
        jz      keyloop
        jmp     .slide
.tglbar:
        bt      [window_style], 25
        jnc     @f
        mov     [bToggleToolbar], 1
        xor     [bShowToolbar], 1
@@:
        jmp     keyloop
.slide:
        call    slide_show
        jmp     keyloop
.set_scale_none:
        mov     eax, LIBIMG_SCALE_NONE
        call    set_scale_mode
        jz      @f
        call    recalc_canvas
@@:
        jmp     keyloop
.set_scale_fit_min:
        mov     eax, LIBIMG_SCALE_FIT_MIN
        call    set_scale_mode
        jz      @f
        call    recalc_work
@@:
        jmp     keyloop
.move_pictport_left:
        stdcall move_pictport, -KEY_MOVE_PIXELS, 0
        stdcall update_scrollbars, eax
        call    draw_view
        call    draw_onimage_decorations
        jmp     keyloop
.move_pictport_right:
        stdcall move_pictport, KEY_MOVE_PIXELS, 0
        stdcall update_scrollbars, eax
        call    draw_view
        call    draw_onimage_decorations
        jmp     keyloop
.move_pictport_up:
        stdcall move_pictport, 0, -KEY_MOVE_PIXELS
        stdcall update_scrollbars, eax
        call    draw_view
        call    draw_onimage_decorations
        jmp     keyloop
.move_pictport_down:
        stdcall move_pictport, 0, KEY_MOVE_PIXELS
        stdcall update_scrollbars, eax
        call    draw_view
        call    draw_onimage_decorations
        jmp     keyloop
.prev:
        dec     esi
        jmp     keyloop
.next:
        inc     esi
        jmp     keyloop
keyloopdone:
        test    esi, esi
        jnz     next_or_prev_handler
        test    [bToggleSlideShow], 1
        jnz     redraw_all
        test    [bToggleToolbar], 1
        stdcall recalc_client
        jnz     redraw_all
        test    [bScaleModeChanged], 1
        mov     [bScaleModeChanged], 0
        jnz     redraw_all
        test    [bNewImage], 1
        mov     [bNewImage], 0
        jnz     redraw_all
        jmp     still
next_or_prev_handler:
        call    next_or_prev_image
        jmp     redraw_all

red_update_frame:
        mov     eax, [cur_frame]
        mov     eax, [eax+Image.Next]
        test    eax, eax
        jnz     @f
        mov     eax, [cur_image]
@@:
        mov     [cur_frame], eax
        mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
        mov     [cur_frame_time], eax
        call    draw_view
        ret

button:
        mcall   SF_GET_BUTTON
        shr     eax, 8

        ; flip horizontally
        cmp     eax, 'flh'
        jnz     .not_flh

        mov     eax, [cur_image]
        cmp     eax, [orig_image]
        jz      @f
        invoke  img.flip, [cur_image], FLIP_HORIZONTAL
@@:
        invoke  img.flip, [orig_image], FLIP_HORIZONTAL
        jmp     redraw_all

        ; flip vertically
.not_flh:
        cmp     eax, 'flv'
        jnz     .not_flv

        mov     eax, [cur_image]
        cmp     eax, [orig_image]
        jz      @f
        invoke  img.flip, [cur_image], FLIP_VERTICAL
@@:
        invoke  img.flip, [orig_image], FLIP_VERTICAL
        jmp     redraw_all

        ; flip both horizontally and vertically
.not_flv:
        cmp     eax, 'flb'
        jnz     .not_flb

        mov     eax, [cur_image]
        cmp     eax, [orig_image]
        jz      @f
        invoke  img.flip, [cur_image], FLIP_BOTH
@@:
        invoke  img.flip, [orig_image], FLIP_BOTH
        jmp     redraw_all

        ; rotate left
.not_flb:
        cmp     eax, 'rtl'
        jnz     .not_rtl

        push    ROTATE_90_CCW
.rotate_common:
        mov     eax, [cur_image]
        cmp     eax, [orig_image]
        jz      @f
        push    dword[esp]
        invoke  img.rotate, [cur_image]
@@:
        invoke  img.rotate, [orig_image]
        mov     [bNewImage], 1
        jmp     redraw_all

        ; rotate right
.not_rtl:
        cmp     eax, 'rtr'
        jnz     .not_rtr

        push    ROTATE_90_CW
        jmp     .rotate_common

        ; open new file
.not_rtr:
        cmp     eax, 'opn'
        jnz     @f

; OpenDialog Open
        push    dword OpenDialog_data
        call    [OpenDialog_Start]

        cmp     [OpenDialog_data.status], 1
        jnz     still

        mov     esi, path
        mov     edi, __params
        push    edi
        mov     ecx, 4096/4
        rep movsd
        mov     byte[edi-1], 0

        pop     esi
        push    esi
        call    find_last_name_component

        pop     eax
        push    [cur_image]
        stdcall load_image, eax
        jc      .restore_old
        call    free_directory
        jmp     redraw_all

.restore_old:
        pop     eax
        jmp     still

        ; set background
@@:
        cmp     eax, 'bgr'
        jnz     @f

        mcall   SF_BACKGROUND_SET, SSF_MODE_BG, 2 ; stretch by default
        call    set_as_bgr
        jmp     still

@@:
        cmp     eax, 'sld'
        jnz     @f

        call    slide_show
        jmp     redraw_all

@@:
        cmp     eax, 'scl'
        jnz     .not_scl
        mov     eax, LIBIMG_SCALE_NONE
        cmp     [scale_mode], LIBIMG_SCALE_NONE
        jnz     @f
        mov     eax, LIBIMG_SCALE_FIT_MIN
@@:
        call    set_scale_mode
        jz      @f
        call    recalc_work
@@:
        jmp     redraw_all
.not_scl:
        or      esi, -1
        cmp     eax, 'bck'
        jz      next_or_prev_handler
        neg     esi
        cmp     eax, 'fwd'
        jz      next_or_prev_handler

        cmp     eax, 1
        jnz     still

exit:
        invoke  ini_set_int, inifilename, aKivSection, aWinX, [window.left]
        invoke  ini_set_int, inifilename, aKivSection, aWinY, [window.top]
        invoke  ini_set_int, inifilename, aKivSection, aWinW, [window.width]
        invoke  ini_set_int, inifilename, aKivSection, aWinH, [window.height]
        mcall   -1


proc load_image _filename
        push    ebx esi edi
        invoke  img.from_file, [_filename]
        test    eax, eax
        jz      .error
        mov     ebx, eax

        test    [eax+Image.Flags], Image.IsAnimated
        jnz     @f
        cmp     [eax+Image.Next], 0
        jz      @f
        stdcall merge_icons_to_single_img, eax
        test    eax, eax
        jz      .error_destroy
@@:
        stdcall init_frame, eax
        clc
        jmp     .exit

.error_destroy:
        invoke  img.destroy, ebx
        xor     eax, eax
.error:
        stc
.exit:
        pop     edi esi ebx
        ret
endp


; in:  eax -- pointer to image
; out: fill pict structure
proc calculate_picture_size
        mov     edx, [eax+Image.Width]
        test    [eax+Image.Flags], Image.IsAnimated
        jnz     .not_in_row
        push    eax
@@:
        cmp     [eax+Image.Next], 0
        jz      @f
        mov     eax, [eax+Image.Next]
        add     edx, [eax+Image.Width]
        inc     edx
        jmp     @b
@@:
        pop     eax
.not_in_row:
        mov     [pict.width], edx
        add     edx, 19
        cmp     edx, 50+25*numimages
        jae     @f
        mov     edx, 50+25*numimages
@@:
        mov     esi, [eax+Image.Height]
        test    [eax+Image.Flags], Image.IsAnimated
        jnz     .max_equals_first
        push    eax
@@:
        cmp     [eax+Image.Next], 0
        jz      @f
        mov     eax, [eax+Image.Next]
        cmp     esi, [eax+Image.Height]
        jae     @b
        mov     esi, [eax+Image.Height]
        jmp     @b
@@:
        pop     eax
.max_equals_first:
        mov     [pict.height], esi
        ret
endp


; in:  [orig_image]
proc set_as_bgr
        mov     esi, [orig_image]
        mov     ecx, [esi+Image.Width]
        mov     edx, [esi+Image.Height]
        mcall   SF_BACKGROUND_SET, SSF_SIZE_BG
        mcall   SF_BACKGROUND_SET, SSF_MAP_BG
        test    eax, eax
        jz      @f

        push    eax
        invoke  img.to_rgb2, esi, eax
        pop     ecx
        mcall   SF_BACKGROUND_SET, SSF_UNMAP_BG

@@:
        mcall   SF_BACKGROUND_SET, SSF_REDRAW_BG
        ; save to file eskin.ini
        xor     al, al
        mov     ecx, 1024
        mov     edi, sys_path+2
        repne scasb
        sub     edi, sys_path+3
        invoke  ini_set_str, inifileeskin, amain, aprogram, sys_path+2, edi
        ; add param '\S__'
        cmp     word[__params], '\T'
        jz      @f
        cmp     word[__params], '\S'
        je      @f
        mov     esi, __params+4096-8
        mov     edi, __params+4096-4
        mov     ecx, 4096/4-1
        std
        rep movsd
        cld
        mov     dword[__params], '\S__'
@@:
        xor     al, al
        mov     ecx, 4096
        mov     edi, __params
        repne scasb
        sub     edi, __params+1
        invoke  ini_set_str, inifileeskin, amain, aparam, __params, edi
        ret
endp

proc slide_show
        push    ebx esi edi
        mov     [bToggleSlideShow], 1
        btc     [window_style], 25
        xor     [bSlideShow], 1
        jnz     .to_fullscreen
        ; back from fullscreen
        movzx   eax, [bShowToolbarSave]
        mov     [bShowToolbar], al
        mov     [canvas_padding], CANVAS_PADDING
        mov     [bg_color], 0x00ffffff
        mcall   SF_CHANGE_WINDOW, [window_save.left], [window_save.top], [window_save.width], [window_save.height]
        jmp     .done
.to_fullscreen:
        stdcall copy_box, window, window_save
        movzx   eax, [bShowToolbar]
        mov     [bShowToolbarSave], al
        mov     [bShowToolbar], 0
        mov     [canvas_padding], 0
        mov     [bg_color], 0x00000000
        mcall   SF_GET_SCREEN_SIZE
        mov     edx, eax
        shr     edx, 16
        movzx   eax, ax
        mov     esi, eax
        mcall   SF_CHANGE_WINDOW, 0, 0, ,
        mov     eax, LIBIMG_SCALE_FIT_MIN
        call    set_scale_mode

.done:
        pop     edi esi ebx
        ret
endp


; seek to ESI image files
; esi>0 means next file, esi<0-prev file
proc next_or_prev_image
locals
        files_cnt       dd ?
        file_idx        dd ?
endl
        push    ebx esi edi
        push    esi
        call    load_directory
        pop     esi
        mov     eax, [directory_ptr]
        mov     eax, [eax+4]
        mov     [files_cnt], eax
        cmp     [directory_ptr], 0
        jz      .ret
        cmp     [files_cnt], 0 ; number of files
        jz      .ret
        mov     eax, [cur_file_idx]
        cmp     eax, -1
        jnz     @f
        test    esi, esi
        jns     @f
        mov     eax, [files_cnt]
@@:
        add     eax, esi
@@:
        test    eax, eax
        jns     @f
        add     eax, [files_cnt]
        jmp     @b
@@:
        cmp     eax, [files_cnt]
        jb      @f
        sub     eax, [files_cnt]
        jmp     @b
@@:
        mov     [file_idx], eax
.scanloop:
        push    eax esi
        imul    esi, eax, 304
        add     esi, [directory_ptr]
        add     esi, 32+40
        mov     edi, curdir
@@:
        inc     edi
        cmp     byte[edi-1], 0
        jnz     @b
        mov     byte[edi-1], '/'
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     esi, curdir
        push    esi
        mov     edi, __params
        mov     ecx, 4096/4
        rep movsd
        mov     byte[edi-1], 0
        pop     esi
        stdcall load_image, curdir
        pushfd
@@:
        lodsb
        test    al, al
        jnz     @b
@@:
        dec     esi
        cmp     byte[esi], '/'
        jnz     @b
        mov     byte[esi], 0
        popfd
        pop     esi eax
        jnc     .loadedok
        test    esi, esi
        js      .try_prev
.try_next:
        inc     eax
        cmp     eax, [files_cnt]
        jb      @f
        xor     eax, eax
@@:
.try_common:
        cmp     eax, [file_idx]
        jz      .notfound
        jmp     .scanloop
.try_prev:
        dec     eax
        jns     @f
        mov     eax, [files_cnt]
        dec     eax
@@:
        jmp     .try_common
.loadedok:
        mov     [cur_file_idx], eax
.ret:
        pop     edi esi ebx
        ret
.notfound:
        pop     edi esi ebx
        ret
endp


load_directory:
        cmp     [directory_ptr], 0
        jnz     .ret
        mov     esi, __params
        mov     edi, curdir
        mov     ecx, [last_name_component]
        sub     ecx, esi
        dec     ecx
        js      @f
        rep movsb
@@:
        mov     byte[edi], 0
        mcall   68, 12, 0x1000
        test    eax, eax
        jz      .ret
        mov     ebx, readdir_fileinfo
        mov     dword[ebx+12], (0x1000-32) / 304      ; blocks to read
        mov     dword[ebx+16], eax      ; where to store
        mcall   70
        cmp     eax, 6  ; read ok, but there are more files
        jz      .dirok
        test    eax, eax
        jnz     free_directory
        mov     edx, [directory_ptr]
        mov     ecx, [edx+8]            ; total number of files
        mov     [readblocks], ecx
        imul    ecx, 304        ; try to read entire dir, FIXME
        add     ecx, 32         ; plus header
        mcall   68, 20          ; realloc
        test    eax, eax
        jz      free_directory
        mov     [directory_ptr], eax
        mcall   70, readdir_fileinfo
.dirok:
        cmp     ebx, 0
        jle     free_directory
        mov     eax, [directory_ptr]
        mov     edi, [eax+8]  ; total number of files
        mov     [files_num], edi
        add     eax, 32         ; skip header
        mov     edi, eax
        push    0
.dirskip:
        push    eax
        test    byte[eax], 0x18 ; volume label or folder
        jnz     .nocopy
        lea     esi, [eax+40]   ; name
        mov     ecx, esi
@@:
        lodsb
        test    al, al
        jnz     @b
@@:
        dec     esi
        cmp     esi, ecx
        jb      .noext
        cmp     byte[esi], '.'
        jnz     @b
        inc     esi
        mov     ecx, [esi]
        cmp     byte[esi+3], 0
        jnz     .not_3
        or      ecx, 0x202020
        cmp     ecx, 'jpg'
        jz      .copy
        cmp     ecx, 'bmp'
        jz      .copy
        cmp     ecx, 'gif'
        jz      .copy
        cmp     ecx, 'png'
        jz      .copy
        cmp     ecx, 'jpe'
        jz      .copy
        cmp     ecx, 'ico'
        jz      .copy
        cmp     ecx, 'cur'
        jz      .copy
        cmp     ecx, 'tga'
        jz      .copy
        cmp     ecx, 'pcx'
        jz      .copy
        cmp     ecx, 'xcf'
        jz      .copy
        cmp     ecx, 'pbm'
        jz      .copy
        cmp     ecx, 'pgm'
        jz      .copy
        cmp     ecx, 'pnm'
        jz      .copy
        cmp     ecx, 'ppm'
        jz      .copy
        cmp     ecx, 'tif'
        jz      .copy
        cmp     ecx, 'xbm'
        jz      .copy
.not_3:
        cmp     byte[esi+4], 0
        jnz     .nocopy
        or      ecx, 0x20202020
        cmp     ecx, 'tiff'
        jz      @f
        cmp     ecx, 'wbmp'
        jz      @f
        cmp     ecx, 'webp'
        jz      @f
        cmp     ecx, 'jpeg'
        jnz     .nocopy
@@:
        cmp     byte[esi+4], 0
        jnz     .nocopy
.copy:
        mov     esi, [esp]
        mov     ecx, 304 / 4
        rep movsd
        inc     dword[esp+4]
.nocopy:
.noext:
        pop     eax
        add     eax, 304
        dec     ebx
        jnz     .dirskip
        mov     eax, [directory_ptr]
        pop     ebx
        mov     [eax+4], ebx
        test    ebx, ebx
        jz      free_directory
        push    0   ; sort mode
        push    ebx
        add     eax, 32
        push    eax
        call    [SortDir]
        xor     eax, eax
        mov     edi, [directory_ptr]
        add     edi, 32+40    ; name
.scan:
        mov     esi, [last_name_component]
        push    edi
        invoke  strcmpi
        pop     edi
        jz      .found
        inc     eax
        add     edi, 304
        dec     ebx
        jnz     .scan
        or      eax, -1
.found:
        mov     [cur_file_idx], eax
.ret:
        ret

free_directory:
        mcall   68, 13, [directory_ptr]
        and     [directory_ptr], 0
        ret


; in: esi->full name (e.g. /path/to/file.png)
; out: [last_name_component]->last component (e.g. file.png)
proc find_last_name_component
        mov     ecx, esi
@@:
        lodsb
        test    al, al
        jnz     @b
@@:
        dec     esi
        cmp     esi, ecx
        jb      @f
        cmp     byte[esi], '/'
        jnz     @b
@@:
        inc     esi
        mov     [last_name_component], esi
        ret
endp


proc init_frame uses ebx edx, _img
        mov     eax, [orig_image]
        cmp     eax, [_img]
        jz      .exit
        test    eax, eax
        jz      .freed
        cmp     eax, [cur_image]
        jz      @f
        invoke  img.destroy, [orig_image]
@@:
        invoke  img.destroy, [cur_image]
.freed:

        mov     [bNewImage], 1
        mov     eax, [_img]
        mov     [orig_image], eax
        mov     [cur_image], eax
        mov     [cur_frame], eax
        test    byte[eax+Image.Flags], Image.IsAnimated
        jz      @f
        push    ebx
        mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
        pop     ebx
        mov     [cur_frame_time], eax
@@:
        mov     [pict.top], 0
        mov     [pict.left], 0
.exit:
        ret
endp


proc draw_window
        test    [bFirstWinDraw], 1
        jnz     .min_size_ok

        mcall   SF_THREAD_INFO, procinfo, -1
        xor     eax, eax
        mov     edx, -1
        mov     esi, -1
        cmp     [procinfo.wnd_state], 0x04
        je      .min_size_ok
        cmp     [procinfo.box.width], MIN_WINDOW_WIDTH
        ja      @f
        mov     edx, MIN_WINDOW_WIDTH
        inc     eax
@@:
        cmp     [procinfo.box.height], MIN_WINDOW_HEIGHT
        ja      @f
        mov     esi, MIN_WINDOW_HEIGHT
        inc     eax
@@:
        test    eax, eax
        jz      @f
        mcall   SF_CHANGE_WINDOW, -1, -1, ,
@@:

.min_size_ok:
        test    [bNewImage], 1
        jz      @f
        call    generate_window_header
@@:
        cmp     [window.width], 0
        jne     @f
        mcall   SF_GET_SCREEN_SIZE
        mov     ebx, eax
        shr     ebx, 16         ; ebx = width
        movzx   esi, ax         ; esi = height

        mov     eax, ebx
        xor     edx, edx
        mov     ebx, 3
        div     ebx
        imul    eax, 2
        mov     [window.width], eax

        xor     edx, edx
        mov     ebx, 4
        div     ebx
        mov     [window.left], eax

        mov     eax, esi
        xor     edx, edx
        mov     ebx, 3
        div     ebx
        imul    eax, 2
        mov     [window.height], eax

        xor     edx, edx
        mov     ebx, 4
        div     ebx
        mov     [window.top], eax
@@:

        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mov     ecx, [window.top]
        shl     ecx, 16
        mov     cx, word[window.height]
        mov     ebx, [window.left]
        shl     ebx, 16
        mov     bx, word[window.width]
        mcall   0, , , [window_style], 0, window_header

        mcall   SF_THREAD_INFO, procinfo, -1
        test    [procinfo.wnd_state], 0x04
        jnz     .nodraw

        stdcall copy_box, window, window_prev
        stdcall copy_box, procinfo.box, window
        test    [bFirstWinDraw], 1
        jnz     .recalc
        test    [bToggleSlideShow], 1
        jnz     .recalc
        mov     eax, [window.width]
        cmp     eax, [window_prev.width]
        jnz     .recalc
        mov     eax, [window.height]
        cmp     eax, [window_prev.height]
        jnz     .recalc
        test    [bNewImage], 1
        jnz     .recalc
        test    [bToggleToolbar], 1
        jnz     .recalc
        jmp     .recalc_done

.recalc:
        stdcall recalc_window
.recalc_done:

        stdcall draw_client
.nodraw:
        mcall   SF_REDRAW, SSF_END_DRAW
        mov     [bFirstWinDraw], 0
        mov     [bNewImage], 0
        mov     [bToggleToolbar], 0
        mov     [bToggleSlideShow], 0

        ret
endp


proc draw_view uses ebx esi edi
        cmp     [scale_mode], LIBIMG_SCALE_FIT_MIN
        jnz     .scale_none
        mov     ecx, [cur_frame]
        mov     eax, [ecx+Image.Width]
        cmp     eax, [view.width]
        jnz     .scale
        mov     eax, [ecx+Image.Height]
        cmp     eax, [view.height]
        jnz     .scale
        jmp     .draw
.scale:
        mov     eax, [orig_image]
        cmp     eax, [cur_image]
        jz      @f
        invoke  img.destroy, [cur_image]
        mov     eax, [orig_image]
@@:
        invoke  img.scale, eax, 0, 0, [eax+Image.Width], [eax+Image.Height], 0, LIBIMG_SCALE_STRETCH, LIBIMG_INTER_DEFAULT, [view.width], [view.height]
        test    eax, eax
;FIXME
        mov     [cur_image], eax
        mov     [cur_frame], eax        ; FIXME index
        jmp     .draw

.scale_none:

.draw:
        push    [pict.top]
        push    [pict.left]
        push    [view.height]
        push    [view.width]
        push    [view_abs_top]  ; ypos
        push    [view_abs_left] ; xpos
        invoke  img.draw, [cur_frame]
.done:
        ret
endp

proc draw_scale_button
        pushad
        mcall   65, buttons+scalebtn*20, <20,20>, [scale_button_xy], 8, palette
        mov     ebx, [scale_button_xy]
        add     ebx, 0x00050006
        ; print letter(s) corresponding to the current scaling mode
        mov     edi, 2
        mov     [scale_button_letter], 'x1'
        cmp     [scale_mode], LIBIMG_SCALE_NONE
        jz      @f
        add     ebx, 0x00020001
        mov     edi, 1
        mov     [scale_button_letter], 'W'
;        cmp     [scale_mode], LIBIMG_SCALE_FIT_MIN
@@:
        mcall   4, , 0x800000ff, scale_button_letter
        popad
        ret
endp

proc draw_toolbar uses ebx esi edi
        cmp     [toolbar.height], 0
        jz      .quit
        mov     ebx, [toolbar_abs_left]
        shl     ebx, 16
        add     ebx, [toolbar.width]
        inc     ebx
        mov     ecx, [toolbar_abs_top]
        shl     ecx, 16
        add     ecx, [toolbar.height]
        mcall   13, , , [bg_color]
        mov     ebx, [toolbar_abs_left]
        shl     ebx, 16
        add     ebx, [toolbar_abs_left]
        add     ebx, [toolbar.width]
        mov     ecx, [toolbar_abs_top]
        shl     ecx, 16
        add     ecx, [toolbar_abs_top]
        add     ecx, (30 SHL 16)+30
        mcall   38, , , 0x007F7F7F
        mov     ebx, [toolbar_abs_left]
        shl     ebx, 16
        add     ebx, [toolbar_abs_left]
        add     ebx, ((5+25*1) SHL 16)+(5+25*1)
        mov     ecx, [toolbar_abs_top]
        shl     ecx, 16
        add     ecx, [toolbar_abs_top]
        add     ecx, [toolbar.height]
        mcall
        add     ebx, ((5+25*2) SHL 16)+(5+25*2)
        mcall
        add     ebx, ((5+25*2) SHL 16)+(5+25*2)
        mcall
        add     ebx, ((5+25*1) SHL 16)+(5+25*1)
        mcall
        mov     ebx, [toolbar_abs_left]
        add     ebx, [toolbar.width]
        sub     ebx, 25*4+10
        shl     ebx, 16
        add     ebx, [toolbar_abs_left]
        add     ebx, [toolbar.width]
        sub     ebx, 25*4+10
        mcall

        mov     ebx, [toolbar_abs_left]
        shl     ebx, 16
        add     ebx, ((4+25*0) SHL 16)+21
        mov     ecx, [toolbar_abs_top]
        shl     ecx, 16
        add     ecx, (4 SHL 16)+21
        mcall   SF_DEFINE_BUTTON, , , 'opn'+40000000h
        add     ebx, (5+25*1) SHL 16
        mcall    , , , 'bck'+40000000h
        add     ebx, (0+25*1) SHL 16
        mcall    , , , 'fwd'+40000000h
        add     ebx, (5+25*1) SHL 16
        mcall    , , , 'bgr'+40000000h
        add     ebx, (0+25*1) SHL 16
        mcall    , , , 'sld'+40000000h
        add     ebx, (5+25*1) SHL 16
        mcall    , , , 'scl'+40000000h
        mov     ebx, [toolbar_abs_left]
        add     ebx, [toolbar.width]
        sub     ebx, 25*4+10
        add     ebx, 5
        shl     ebx, 16
        mov     bl, 21
        mcall   , , , 'flh'+40000000h
        add     ebx, 25 SHL 16
        mcall   , , , 'flv'+40000000h
        add     ebx, 30 SHL 16
        mcall   , , , 'rtr'+40000000h
        add     ebx, 25 SHL 16
        mcall   , , , 'rtl'+40000000h
        add     ebx, 25 SHL 16
        mcall   , , , 'flb'+40000000h

        mov     ebp, (numimages-1)*20

        mov     edx, [toolbar_abs_left]
        shl     edx, 16
        add     edx, [toolbar_abs_top]
        add     edx, ((5+25*0) SHL 16)+5
        mcall   65, buttons+openbtn   *20, <20, 20>, , 8, palette
        add     edx, ((5+25*1) SHL 16)+0
        mcall     , buttons+backbtn   *20
        add     edx, ((0+25*1) SHL 16)+0
        mcall     , buttons+forwardbtn*20
        add     edx, ((5+25*1) SHL 16)+0
        mcall     , buttons+bgrbtn    *20
        add     edx, ((0+25*1) SHL 16)+0
        mcall     , buttons+slidebtn  *20
        add     edx, ((5+25*1) SHL 16)+0
;        mcall     , buttons+scalebtn  *20
        mov     [scale_button_xy], edx
        call    draw_scale_button
        mov     edx, [client_abs_left]
        add     edx, [client.width]
        sub     edx, 25*4+4
        shl     edx, 16
        add     edx, [client_abs_top]
        add     edx, 5
        mcall   , buttons+fliphorzbtn*20
        add     edx, 25*65536
        mcall   , buttons+flipvertbtn*20
        add     edx, 30*65536
        mcall   , buttons+rotccwbtn*20
        add     edx, 25*65536
        mcall   , buttons+rotcwbtn*20

.quit:
        ret
endp


proc draw_canvas
        push    ebx esi edi

        mov     ebx, [canvas_abs_left]
        shl     ebx, 16
        add     ebx, [canvas.width]
        mov     ecx, [canvas_abs_top]
        shl     ecx, 16
        add     ecx, [view.top]
;mov edx, 0xff0000
        mcall   13, , , [bg_color]
        mcall   13
        mov     ecx, [view_abs_top]
        add     ecx, [view.height]
        shl     ecx, 16
        add     ecx, [canvas.height]
        sub     ecx, [view.top]
        sub     ecx, [view.height]
;mov edx, 0x00ff00
        mcall   13, , , [bg_color]
        mcall   13
        mov     ebx, [canvas_abs_left]
        shl     ebx, 16
        add     ebx, [view.left]
        mov     ecx, [canvas_abs_top]
        shl     ecx, 16
        add     ecx, [canvas.height]
;mov edx, 0x0000ff
;bg_color
        mcall
        mov     ebx, [view_abs_left]
        add     ebx, [view.width]
        shl     ebx, 16
        mov     eax, [canvas.width]
        sub     eax, [view.left]
        sub     ebx, [view.width]
        add     ebx, eax
;mov edx, 0xffff00
;bg_color
        mcall   13
@@:

        call    draw_view

        pop     edi esi ebx
        ret
endp


proc draw_client
        push    ebx esi edi

        test    [bShowToolbar], 1
        jz      .toolbar_done
        call    draw_toolbar
.toolbar_done:
        call    draw_work

        pop     edi esi ebx
        ret
endp


proc draw_work
        push    ebx esi edi

        mov     ebx, [work_abs_left]
        shl     ebx, 16
        add     ebx, [work.width]
        inc     ebx
        mov     ecx, [work_abs_top]
        shl     ecx, 16
        add     ecx, [canvas.top]
;        mcall   13, , , 0xff0000
        mcall   13, , , [bg_color]
        mov     eax, [canvas.height]
        ror     ecx, 16
        add     ecx, eax
        add     ecx, [canvas_padding]
        ror     ecx, 16
;        mcall   13, , , 0x00ff00
        mcall   13, , , [bg_color]
;        mcall   13

        mov     ebx, [work_abs_left]
        shl     ebx, 16
        add     ebx, [canvas.left]
        mov     ecx, [work_abs_top]
        add     ecx, [canvas_padding]
        shl     ecx, 16
        add     ecx, [canvas.height]
;        mcall   13, , , 0x0000ff
        mcall   13, , , [bg_color]
;        mcall
        mov     eax, [canvas.width]
        ror     ebx, 16
        add     ebx, eax
        add     ebx, [canvas_padding]
        ror     ebx, 16
;        mcall   13, , , 0xffff00
        mcall   13, , , [bg_color]
;        mcall   13

        call    draw_canvas
        call    draw_onimage_decorations

        mov     eax, 13
        cmp     [need_scrollbar_v], 1
        jnz     @f
        cmp     [need_scrollbar_h], 1
        jnz     @f
        mov     ebx, [work_abs_left]
        add     ebx, [work.width]
        sub     ebx, SCROLL_WIDTH_SIZE
        shl     ebx, 16
        add     ebx, SCROLL_WIDTH_SIZE
        inc     ebx
        mov     ecx, [work_abs_top]
        add     ecx, [work.height]
        sub     ecx, SCROLL_WIDTH_SIZE
        shl     ecx, 16
        add     ecx, SCROLL_WIDTH_SIZE
        inc     ecx
        mov     edx, [bg_color]
;        mov     edx, 0x00ffff
        mcall
@@:

        cmp     [need_scrollbar_v], 0
        jz      .v_scrollbar_done
        mov     eax, [client.left]
        add     eax, [client.width]
        sub     eax, SCROLL_WIDTH_SIZE
        mov     [scroll_bar_data_vertical.start_x], ax
        mov     eax, [toolbar.height]
        add     eax, [client.top]
        mov     [scroll_bar_data_vertical.start_y], ax
        mov     eax, [canvas.height]
        add     eax, [canvas_padding]
        add     eax, [canvas_padding]
        mov     [scroll_bar_data_vertical.size_y], ax
        mov     [scroll_bar_data_vertical.all_redraw], 1
        invoke  scrollbar_vert_draw, scroll_bar_data_vertical
.v_scrollbar_done:

        cmp     [need_scrollbar_h], 0
        jz      .h_scrollbar_done
        mov     eax, [client.left]
        mov     [scroll_bar_data_horizontal.start_x], ax
        mov     eax, [client.top]
        add     eax, [client.height]
        sub     eax, SCROLL_WIDTH_SIZE
        mov     [scroll_bar_data_horizontal.start_y], ax
        mov     eax, [canvas.width]
        add     eax, [canvas_padding]
        add     eax, [canvas_padding]
        mov     [scroll_bar_data_horizontal.size_x], ax
        mov     [scroll_bar_data_horizontal.all_redraw], 1
        invoke  scrollbar_hort_draw, scroll_bar_data_horizontal
.h_scrollbar_done:

        pop     edi esi ebx
        ret
endp


proc draw_onimage_decorations
        bt      [window_style], 25
        jc      @f
        ; draw fullscreen decorations on image
        call    draw_filename
        call    draw_fullscreen_controls
@@:
        ret
endp


proc draw_filename
        push    esi
        mcall   4, <100, 65>, 0x40ffffff, window_header, [window_header_len], 0x008800
        pop     esi
        ret
endp


proc draw_fullscreen_controls
        push    esi
        mov     ebx, [canvas.width]
        shr     ebx, 1
        add     ebx, [canvas.left]
        sub     ebx, 22
        shl     ebx, 16
        add     ebx, 20
        mov     ecx, [canvas.height]
        shr     ecx, 3
        neg     ecx
        add     ecx, [canvas.height]
        add     ecx, [canvas.top]
        shl     ecx, 16
        add     ecx, 20
        mcall   8, , , 'bck'+40000000h
        add     ebx, 25 SHL 16
        mcall   8, , , 'fwd'+40000000h
        mov     edx, [canvas.width]
        shr     edx, 1
        add     edx, [canvas.left]
        sub     edx, 22
        shl     edx, 16
        add     edx, [canvas.height]
        shr     dx, 3
        neg     dx
        add     dx, word[canvas.height]
        add     edx, [canvas.top]
        mcall   65, buttons+backbtn*20, <20, 20>, , 8, palette
        add     edx, 25 SHL 16
        mcall   65, buttons+forwardbtn*20,      , , 8,
        pop     esi
        ret
endp


proc check_shortcut
; in:   cl = scancode (from sysfn 2),
;   eax = state of modifiers (from sysfn 66.3),
;   edx -> shortcut descriptor
; out:  ZF set <=> fail
        cmp     cl, [edx+4]
        jnz     .not
        push    eax
        mov     esi, [edx]
        and     esi, 0xf
        and     al, 3
        call    dword[check_modifier_table+esi*4]
        test    al, al
        pop     eax
        jnz     .not
        push    eax
        mov     esi, [edx]
        shr     esi, 4
        and     esi, 0xf
        shr     al, 2
        and     al, 3
        call    dword[check_modifier_table+esi*4]
        test    al, al
        pop     eax
        jnz     .not
        push    eax
        mov     esi, [edx]
        shr     esi, 8
        and     esi, 0xf
        shr     al, 4
        and     al, 3
        call    dword[check_modifier_table+esi*4]
        test    al, al
        pop     eax
;       jnz     .not
.not:
        ret
endp


check_modifier_0:
        setnz   al
        ret
check_modifier_1:
        setp    al
        ret
check_modifier_2:
        cmp     al, 3
        setnz   al
        ret
check_modifier_3:
        cmp     al, 1
        setnz   al
        ret
check_modifier_4:
        cmp     al, 2
        setnz   al
        ret

; >edi = destination string
; >eax = number
proc bin2dec
        push    ebx ecx edx esi

        mov     ebx, 10
        xor     ecx, ecx
@@:
        xor     edx, edx
        div     ebx
        push    edx
        inc     ecx
        test    eax, eax
        jnz     @b

@@:
        pop     eax
        add     eax, '0'
        stosb
        inc     [window_header_len]
        dec     ecx
        jnz     @b

        pop     esi edx ecx ebx
        ret
endp


; fills window_header with window title
; window title is generated as '[k/n] <filename> (WxH) - Kolibri Image Viewer'
; n = total files in dir
; k = current file index
; W = current image width
; H = current image height
proc generate_window_header
        push    eax ebx esi edi
        mov     esi, [last_name_component]
        mov     edi, window_header
        mov     [window_header_len], 4    ; [,/,],

        mov     byte[edi], '['
        inc     edi
        mov     eax, [cur_file_idx]
        inc     eax
        call    bin2dec
        mov     byte[edi], '/'
        inc     edi
        mov     eax, [directory_ptr]
        mov     eax, [eax+4]
        call    bin2dec
        mov     word[edi], '] '
        add     edi, 2

        ; add filename
.next_symbol:
        lodsb
        test    al, al
        jz      @f
        stosb
        inc     [window_header_len]
        cmp     edi, window_header+256
        jb      .next_symbol
.overflow:
        mov     dword[edi-4], '...'
.ret:
        pop     edi esi ebx eax
        ret
@@:
        ; add size
        mov     word[edi], ' ('
        add     edi, 2

        mov     ebx, [orig_image]
        mov     eax, [ebx+Image.Width]
        call    bin2dec

        mov     byte[edi], 'x'
        inc     edi

        mov     eax, [ebx+Image.Height]
        call    bin2dec

        mov     byte[edi], ')'
        inc     edi

        mov     esi, s_header
@@:
        lodsb
        stosb
        test    al, al
        jz      .ret
        cmp     edi, window_header+256
        jb      @b
        jmp     .overflow
endp


proc scale_none_calc
        push    ebx

        mov     [scale_mode], LIBIMG_SCALE_NONE

        mov     eax, [cur_image]
        cmp     eax, [orig_image]
        jz      @f
        invoke  img.destroy, eax
@@:
        mov     eax, [orig_image]
        mov     [cur_image], eax
        mov     [cur_frame], eax
        mov     ebx, eax

        mov     [need_scrollbar_v], 0
        mov     [need_scrollbar_h], 0

        mov     eax, [ebx+Image.Width]
        cmp     eax, [canvas.width]
        jbe     @f
        sub     [canvas.height], SCROLL_WIDTH_SIZE+1
        mov     [need_scrollbar_h], 1
@@:
        mov     eax, [ebx+Image.Height]
        cmp     eax, [canvas.height]
        jbe     @f
        sub     [canvas.width], SCROLL_WIDTH_SIZE+1
        mov     [need_scrollbar_v], 1
@@:
        cmp     [need_scrollbar_h], 1
        jz      @f
        mov     eax, [ebx+Image.Width]
        cmp     eax, [canvas.width]
        jbe     @f
        sub     [canvas.height], SCROLL_WIDTH_SIZE+1
        mov     [need_scrollbar_h], 1
@@:


        mov     eax, [ebx+Image.Width]
        cmp     eax, [canvas.width]
        jbe     @f
        mov     eax, [canvas.width]
@@:
        mov     [view.width], eax
        mov     [pict.width], eax

        mov     eax, [ebx+Image.Height]
        cmp     eax, [canvas.height]
        jbe     @f
        mov     eax, [canvas.height]
@@:
        mov     [view.height], eax
        mov     [pict.height], eax

        mov     eax, [canvas.width]
        sub     eax, [view.width]
        sar     eax, 1
        mov     [view.left], eax
        mov     eax, [canvas.height]
        sub     eax, [view.height]
        sar     eax, 1
        mov     [view.top], eax

        mov     eax, [ebx+Image.Width]
        sub     eax, [pict.width]
        sar     eax, 1
        mov     [pict.left], eax
        mov     eax, [ebx+Image.Height]
        sub     eax, [pict.height]
        sar     eax, 1
        mov     [pict.top], eax


        mov     eax, [ebx+Image.Height]
        mov     [scroll_bar_data_vertical.max_area], eax
        mov     eax, [pict.height]
        mov     [scroll_bar_data_vertical.cur_area], eax
        mov     eax, [pict.top]
        mov     [scroll_bar_data_vertical.position], eax

        mov     eax, [ebx+Image.Width]
        mov     [scroll_bar_data_horizontal.max_area], eax
        mov     eax, [pict.width]
        mov     [scroll_bar_data_horizontal.cur_area], eax
        mov     eax, [pict.left]
        mov     [scroll_bar_data_horizontal.position], eax

        pop     ebx
        ret
endp


proc scale_fit_min_calc
        push    ebx

        mov     [need_scrollbar_v], 0
        mov     [need_scrollbar_h], 0
        mov     [scroll_bar_data_vertical.position], 0
        mov     [scroll_bar_data_horizontal.position], 0

        mov     eax, [orig_image]
        cmp     [eax+Image.Type], Image.bpp24
        jz      @f
        cmp     [eax+Image.Type], Image.bpp32
        jz      @f
        cmp     [eax+Image.Type], Image.bpp8g
        jz      @f
        invoke  img.convert, eax, 0, Image.bpp24, 0, 0
        test    eax, eax
;       jz      .error
        push    eax
        invoke  img.destroy, [orig_image]
        pop     eax
        mov     [orig_image], eax
        mov     [cur_image], eax
        mov     [cur_frame], eax
@@:

        mov     eax, [orig_image]
        mov     ecx, [eax+Image.Height]
        mov     eax, [eax+Image.Width]
        cmp     eax, [canvas.width]
        ja      .get_size
        cmp     ecx, [canvas.height]
        ja      .get_size
        jmp     .got_size
.get_size:
        invoke  img.get_scaled_size, eax, ecx, LIBIMG_SCALE_FIT_MIN, [canvas.width], [canvas.height]
.got_size:

        mov     [pict.top], 0
        mov     [pict.left], 0

        cmp     eax, [canvas.width]
        jbe     @f
        mov     eax, [canvas.width]
@@:
        mov     [view.width], eax
        mov     [pict.width], eax
        neg     eax
        add     eax, [canvas.width]
        shr     eax, 1
        mov     [view.left], eax

        mov     eax, ecx
        cmp     eax, [canvas.height]
        jbe     @f
        mov     eax, [canvas.height]
@@:
        mov     [view.height], eax
        mov     [pict.height], eax
        neg     eax
        add     eax, [canvas.height]
        shr     eax, 1
        mov     [view.top], eax


        pop     ebx
        ret
endp


; eax: new scaling mode
; z/Z - not/changed
proc set_scale_mode
        cmp     eax, [scale_mode]
        mov     [scale_mode], eax
        setnz   [bScaleModeChanged]
        ret
endp

proc move_pictport _dx, _dy
locals
        new_left dd ?
        new_top  dd ?
endl
        push    ebx ecx

        mov     ebx, [cur_image]
.x:
        mov     eax, [pict.left]
        add     eax, [_dx]
        cmp     eax, 0
        jge     @f
        mov     [new_left], 0
        jmp     .xdone
@@:
        mov     ecx, eax
        add     eax, [pict.width]
        cmp     eax, [ebx+Image.Width]
        ja      @f
        mov     [new_left], ecx
        jmp     .xdone
@@:
        mov     eax, [ebx+Image.Width]
        sub     eax, [pict.width]
        mov     [new_left], eax
        jmp     .xdone
.xdone:

.y:
        mov     eax, [pict.top]
        add     eax, [_dy]
        cmp     eax, 0
        jge     @f
        mov     [new_top], 0
        jmp     .ydone
@@:
        mov     ecx, eax
        add     eax, [pict.height]
        cmp     eax, [ebx+Image.Height]
        ja      @f
        mov     [new_top], ecx
        jmp     .ydone
@@:
        mov     eax, [ebx+Image.Height]
        sub     eax, [pict.height]
        mov     [new_top], eax
        jmp     .ydone
.ydone:

        xor     eax, eax
        mov     ecx, [new_left]
        mov     edx, [new_top]

        cmp     ecx, [pict.left]
        setnz   al
        shl     eax, 8

        cmp     edx, [pict.top]
        setnz   al

        mov     [pict.left], ecx
        mov     [pict.top], edx

        pop     ecx ebx
        ret
endp


proc update_scrollbars _xxhv
        mov     eax, [_xxhv]

        test    ah, ah
        jz      .no_h_scroll
        push    eax
        mov     [scroll_bar_data_horizontal.all_redraw], 0
        mov     eax, [pict.left]
        mov     [scroll_bar_data_horizontal.position], eax
        invoke  scrollbar_hort_draw, scroll_bar_data_horizontal
        pop     eax
.no_h_scroll:
        test    al, al
        jz      .no_v_scroll
        push    eax
        mov     [scroll_bar_data_vertical.all_redraw], 0
        mov     eax, [pict.top]
        mov     [scroll_bar_data_vertical.position], eax
        invoke  scrollbar_vert_draw, scroll_bar_data_vertical
        pop     eax
.no_v_scroll:

        ret
endp


proc merge_icons_to_single_img _img
        push    ebx esi edi

        mov     edx, [_img]
        mov     eax, [edx+Image.Width]
        mov     ecx, [edx+Image.Height]
.next:
        cmp     [edx+Image.Next], 0
        jz      .got_sizes
        inc     eax
        mov     edx, [edx+Image.Next]
        add     eax, [edx+Image.Width]
        cmp     ecx, [edx+Image.Height]
        jae     @f
        mov     ecx, [edx+Image.Height]
@@:
        jmp     .next

.got_sizes:
        invoke  img.create, eax, ecx, Image.bpp32
        test    eax, eax
        jz      .error
        mov     ebx, eax

        mov     eax, [bg_color]
        mov     edi, [ebx+Image.Data]
        mov     ecx, [ebx+Image.Width]
        imul    ecx, [ebx+Image.Height]
        rep stosd

        mov     eax, [_img]
        cmp     [eax+Image.Type], Image.bpp32
        jz      @f
        invoke  img.convert, eax, 0, Image.bpp32, 0, 0
        test    eax, eax
        jz      .error
        push    eax
        invoke  img.destroy, [_img]
        pop     eax
@@:
        mov     esi, eax
        xor     edi, edi
.next_img:
        stdcall put_img_on_img, ebx, esi, edi, 0
        add     edi, [esi+Image.Width]
        inc     edi
        cmp     [esi+Image.Next], 0
        jz      @f
        mov     esi, [esi+Image.Next]
        jmp     .next_img
@@:
        invoke  img.destroy, esi
        mov     eax, ebx
        jmp     .quit

.error:
        xor     eax, eax
.quit:
        pop     edi esi ebx
        ret
endp


proc put_img_on_img _bottom, _top, _x, _y
locals
        img_height dd ?
endl
        push    ebx esi edi

        mov     ebx, [_bottom]
        mov     edx, [_top]
        mov     eax, [edx+Image.Height]
        mov     [img_height], eax
        mov     esi, [edx+Image.Data]
        mov     edi, [ebx+Image.Data]
        mov     eax, [_y]
        imul    eax, [ebx+Image.Width]
        add     eax, [_x]
        shl     eax, 2
        add     edi, eax
.next_line:
        mov     ecx, [edx+Image.Width]
        rep movsd
        mov     eax, [ebx+Image.Width]
        sub     eax, [edx+Image.Width]
        shl     eax, 2
        add     edi, eax
        dec     [img_height]
        jnz     .next_line

        pop     edi esi ebx
        ret
endp


proc copy_box _src, _dst
        pushad

        mov     esi, [_src]
        mov     edi, [_dst]
        mov     ecx, 4
        rep movsd

        popad
        ret
endp


proc cmp_box _a, _b
        pushad

        mov     esi, [_a]
        mov     edi, [_b]
        mov     ecx, 4
        rep cmpsd

        popad
        ret
endp


proc recalc_client
        stdcall copy_box, toolbar, toolbar_prev
        mov     [toolbar.left], 0
        mov     [toolbar.top], 0
        mov     eax, [client.width]
        mov     [toolbar.width], eax
        mov     [toolbar.height], 0
        cmp     [bShowToolbar], 1
        jnz     @f
        mov     [toolbar.height], TOOLBAR_HEIGHT
@@:

        mov     eax, [toolbar.top]
        add     eax, [client_abs_top]
        mov     [toolbar_abs_top], eax
        mov     eax, [toolbar.left]
        add     eax, [client_abs_left]
        mov     [toolbar_abs_left], eax

        test    [bFirstWinDraw], 1
        jnz     .recalc_toolbar
        stdcall cmp_box, toolbar, toolbar_prev
        jnz     .recalc_toolbar
        test    [bNewImage], 1
        jnz     .recalc_toolbar
        jmp     .recalc_toolbar_done
.recalc_toolbar:
        stdcall recalc_toolbar
.recalc_toolbar_done:

        stdcall copy_box, work, work_prev
        xor     ecx, ecx
        test    [bShowToolbar], 1
        jz      @f
        mov     ecx, [toolbar.height]
@@:
        mov     eax, ecx
        mov     [work.top], eax
        mov     eax, [client.height]
        sub     eax, ecx
        mov     [work.height], eax
        mov     [work.left], 0
        mov     eax, [client.width]
        mov     [work.width], eax

        mov     eax, [work.top]
        add     eax, [client_abs_top]
        mov     [work_abs_top], eax
        mov     eax, [work.left]
        add     eax, [client_abs_left]
        mov     [work_abs_left], eax

        test    [bFirstWinDraw], 1
        jnz     .recalc_work
        test    [bNewImage], 1
        jnz     .recalc_work
        stdcall cmp_box, work, work_prev
        jnz     .recalc_work
        jmp     .recalc_work_done
.recalc_work:
        stdcall recalc_work
.recalc_work_done:

        ret
endp


proc recalc_toolbar

        ret
endp


proc recalc_window
        stdcall copy_box, client, client_prev
        test    [bSlideShow], 1
        jz      .no_slide_show
.slide_show:
        mov     [client.left], 0
        mov     [client.top], 0
        mov     eax, [procinfo.box.width]
        mov     [client.width], eax
        mov     eax, [procinfo.box.height]
        mov     [client.height], eax
        jmp     .calc_abs
.no_slide_show:
        mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
        mov     [client.top], eax
        neg     eax
        add     eax, [procinfo.box.height]
        sub     eax, 5
        mov     [client.height], eax
        mov     [client.left], 5
        mov     eax, [procinfo.box.width]
        sub     eax, 10
        mov     [client.width], eax

.calc_abs:
        mov     eax, [client.top]
        mov     [client_abs_top], eax
        mov     eax, [client.left]
        mov     [client_abs_left], eax

        stdcall cmp_box, client, client_prev
        jnz     .recalc_client
        test    [bNewImage], 1
        jnz     .recalc_client
        test    [bToggleToolbar], 1
        jnz     .recalc_client
        jmp     .recalc_client_done
.recalc_client:
        stdcall recalc_client
.recalc_client_done:

        ret
endp


proc recalc_work
        stdcall copy_box, canvas, canvas_prev
        mov     eax, [work.left]
        add     eax, [canvas_padding]
        mov     [canvas.left], eax
        mov     eax, [work.width]
        sub     eax, [canvas_padding]
        sub     eax, [canvas_padding]
        inc     eax
        mov     [canvas.width], eax
        mov     eax, [canvas_padding]
        mov     [canvas.top], eax
        mov     eax, [work.height]
        sub     eax, [canvas_padding]
        sub     eax, [canvas_padding]
        inc     eax
        mov     [canvas.height], eax

        mov     eax, [canvas.top]
        add     eax, [work_abs_top]
        mov     [canvas_abs_top], eax
        mov     eax, [canvas.left]
        add     eax, [work_abs_left]
        mov     [canvas_abs_left], eax

        test    [bFirstWinDraw], 1
        jnz     .recalc_canvas
        test    [bNewImage], 1
        jnz     .recalc_canvas
        stdcall cmp_box, canvas, canvas_prev
        jnz     .recalc_canvas
        cmp     [bScaleModeChanged], 0
        jnz     .recalc_canvas
        jmp     .recalc_canvas_done
.recalc_canvas:
        stdcall recalc_canvas
.recalc_canvas_done:
        ret
endp


proc recalc_canvas
        stdcall copy_box, view, view_prev
        mov     eax, [scale_mode]
        call    [scale_mode_calc+eax*4]

        mov     eax, [view.top]
        add     eax, [canvas_abs_top]
        mov     [view_abs_top], eax
        mov     eax, [view.left]
        add     eax, [canvas_abs_left]
        mov     [view_abs_left], eax

        ret
endp

;-----------------------------------------------------------------------------
DATA

s_header        db ' - Kolibri Image Viewer',0
window_style    dd 0x53FFFFFF
window          BOX 100, 150, 0, 0  ; left top width height
window_prev     BOX 0, 0, 0, 0
window_save     BOX 0, 0, 0, 0  ; restore after slide show
client          BOX 0, 0, 0, 0
client_prev     BOX 0, 0, 0, 0
client_abs_top  dd ?
client_abs_left dd ?
toolbar         BOX 0, 0, 0, 0
toolbar_prev    BOX 0, 0, 0, 0
toolbar_abs_top dd ?
toolbar_abs_left dd ?
work            BOX 0, 0, 0, 0
work_prev       BOX 0, 0, 0, 0
work_abs_top    dd ?
work_abs_left   dd ?
canvas          BOX 0, 0, 0, 0
canvas_prev     BOX 0, 0, 0, 0
canvas_abs_top  dd ?
canvas_abs_left dd ?
view            BOX -1, -1, 0, 0
view_prev       BOX -1, -1, 0, 0
view_abs_top    dd ?
view_abs_left   dd ?
pict            BOX 0, 0, 0, 0
canvas_padding  dd 5
bg_color        dd 0x00ffffff
scale_mode      dd LIBIMG_SCALE_FIT_MIN
pict_drag       dd 0
scroll_v_drag   dd 0
scroll_h_drag   dd 0

mouse_buttons    dd 0
mouse_pos        dd 0
need_scrollbar_v dd 0
need_scrollbar_h dd 0

pict_moved      dd 0

;-----------------------------------------------------------------------------
align   4
scroll_bar_data_vertical:
.x:
.size_x         dw SCROLL_WIDTH_SIZE
.start_x        dw 1
.y:
.size_y         dw 100
.start_y        dw 0
.btn_high       dd SCROLL_WIDTH_SIZE
.type           dd 0    ;+12
.max_area       dd 100  ;+16
.cur_area       dd 10   ;+20
.position       dd 0    ;+24
.bckg_col       dd 0xAAAAAA     ;+28
.frnt_col       dd 0xCCCCCC     ;+32
.line_col       dd 0    ;+36
.redraw         dd 0    ;+40
.delta          dw 0    ;+44
.delta2         dw 0    ;+46
.run_x:
.r_size_x       dw 0    ;+48
.r_start_x      dw 0    ;+50
.run_y:
.r_size_y       dw 0    ;+52
.r_start_y      dw 0    ;+54
.m_pos          dd 0    ;+56
.m_pos_2        dd 0    ;+60
.m_keys         dd 0    ;+64
.run_size       dd 0    ;+68
.position2      dd 0    ;+72
.work_size      dd 0    ;+76
.all_redraw     dd 0    ;+80
.ar_offset      dd KEY_MOVE_PIXELS   ;+84
;-----------------------------------------------------------------------------
align   4
scroll_bar_data_horizontal:
.x:
.size_x         dw 0    ;+0
.start_x        dw 0    ;+2
.y:
.size_y         dw SCROLL_WIDTH_SIZE    ;+4
.start_y        dw 0    ;+6
.btn_high       dd SCROLL_WIDTH_SIZE    ;+8
.type           dd 0    ;+12
.max_area       dd 50   ;+16
.cur_area       dd 50   ;+20
.position       dd 0    ;+24
.bckg_col       dd 0xAAAAAA     ;+28
.frnt_col       dd 0xCCCCCC     ;+32
.line_col       dd 0    ;+36
.redraw         dd 0    ;+40
.delta          dw 0    ;+44
.delta2         dw 0    ;+46
.run_x:
.r_size_x       dw 0    ;+48
.r_start_x      dw 0    ;+50
.run_y:
.r_size_y       dw 0    ;+52
.r_start_y      dw 0    ;+54
.m_pos          dd 0    ;+56
.m_pos_2        dd 0    ;+60
.m_keys         dd 0    ;+64
.run_size       dd 0    ;+68
.position2      dd 0    ;+72
.work_size      dd 0    ;+76
.all_redraw     dd 0    ;+80
.ar_offset      dd KEY_MOVE_PIXELS      ;+84
;-----------------------------------------------------------------------------
align 4
@IMPORT:

library                           \
        libgfx  , 'libgfx.obj'  , \
        libimg  , 'libimg.obj'  , \
        libini  , 'libini.obj'  , \
        sort    , 'sort.obj'    , \
        proc_lib, 'proc_lib.obj', \
        box_lib , 'box_lib.obj'


import libgfx                         , \
        libgfx.init  , 'lib_init'     , \
        gfx.open     , 'gfx_open'     , \
        gfx.close    , 'gfx_close'    , \
        gfx.pen.color, 'gfx_pen_color', \
        gfx.line     , 'gfx_line'

import libimg                                     , \
        libimg.init        , 'lib_init'           , \
        img.from_file      , 'img_from_file'      , \
        img.to_rgb2        , 'img_to_rgb2'        , \
        img.create         , 'img_create'         , \
        img.flip           , 'img_flip'           , \
        img.rotate         , 'img_rotate'         , \
        img.destroy        , 'img_destroy'        , \
        img.scale          , 'img_scale'          , \
        img.get_scaled_size, 'img_get_scaled_size', \
        img.convert        , 'img_convert'        , \
        img.draw           , 'img_draw'

import libini                               , \
        ini_get_shortcut, 'ini_get_shortcut', \
        ini_get_int,      'ini_get_int',\
        ini_set_int,      'ini_set_int',\
        ini_set_str,      'ini_set_str'

import sort                  ,\
        sort.START, 'START'  ,\
        SortDir   , 'SortDir',\
        strcmpi   , 'strcmpi'

import proc_lib                             ,\
        OpenDialog_Init , 'OpenDialog_init' ,\
        OpenDialog_Start, 'OpenDialog_start'

import box_lib                                   ,\
        scrollbar_vert_draw , 'scrollbar_v_draw' ,\
        scrollbar_vert_mouse, 'scrollbar_v_mouse',\
        scrollbar_hort_draw , 'scrollbar_h_draw' ,\
        scrollbar_hort_mouse, 'scrollbar_h_mouse'

bFirstWinDraw     db 1
bSlideShow        db 0
bToggleSlideShow  db 0
bShowToolbar      db 1
bShowToolbarSave  db 0  ; to restore state when return from slide show
bToggleToolbar    db 0
bScaleModeChanged db 0
bNewImage         db 0
;-----------------------------------------------------------------------------

virtual at 0
file 'kivicons.bmp':0xA,4
load offbits dword from 0
end virtual
numimages = 11
openbtn = 0
backbtn = 1
forwardbtn = 2
bgrbtn = 3
fliphorzbtn = 4
flipvertbtn = 5
rotcwbtn = 6
rotccwbtn = 7
rot180btn = 8
slidebtn = 9
scalebtn = 10

palette:
    file 'kivicons.bmp':0x36,offbits-0x36
buttons:
    file 'kivicons.bmp':offbits
repeat 10
y = %-1
z = 20-%
repeat numimages*5
load a dword from $ - numimages*20*20 + numimages*20*y + (%-1)*4
load b dword from $ - numimages*20*20 + numimages*20*z + (%-1)*4
store  dword a at $ - numimages*20*20 + numimages*20*z + (%-1)*4
store  dword b at $ - numimages*20*20 + numimages*20*y + (%-1)*4
end repeat
end repeat

inifilename db  '/sys/settings/app.ini',0
aKivSection  db  'Kiv',0
aNext       db  'Next',0
aPrev       db  'Prev',0
aSlide      db  'SlideShow',0
aTglbar     db  'ToggleBar',0
aWinX       db  'WinX',0
aWinY       db  'WinY',0
aWinW       db  'WinW',0
aWinH       db  'WinH',0

inifileeskin db '/sys/settings/system.ini',0
amain       db 'style',0
aprogram    db 'bg_program',0
aparam      db 'bg_param',0

align 4
check_modifier_table:
    dd  check_modifier_0
    dd  check_modifier_1
    dd  check_modifier_2
    dd  check_modifier_3
    dd  check_modifier_4

;---------------------------------------------------------------------
align 4
OpenDialog_data:
.type                   dd 0
.procinfo               dd procinfo                             ; +4
.com_area_name          dd communication_area_name              ; +8
.com_area               dd 0                                    ; +12
.opendir_path           dd temp_dir_path                        ; +16
.dir_default_path       dd communication_area_default_path      ; +20
.start_path             dd open_dialog_path                     ; +24
.draw_window            dd draw_window                          ; +28
.status                 dd 0                                    ; +32
.openfile_path          dd path                                 ; openfile_path ; +36
.filename_area          dd 0                                    ; +40
.filter_area            dd Filter
.x:
.x_size                 dw 420                                  ; +48 ; Window X size
.x_start                dw 10                                   ; +50 ; Window X position
.y:
.y_size                 dw 320                                  ; +52 ; Window y size
.y_start                dw 10                                   ; +54 ; Window Y position

communication_area_name:
    db 'FFFFFFFF_open_dialog',0

open_dialog_path:
if __nightbuild eq yes
    db '/sys/MANAGERS/opendial',0
else
    db '/sys/File Managers/opendial',0
end if
communication_area_default_path:
    db '/sys',0

Filter:
dd Filter.end-Filter
.1:
db 'BMP',0
db 'GIF',0
db 'JPG',0
db 'JPEG',0
db 'JPE',0
db 'PNG',0
db 'ICO',0
db 'CUR',0
db 'TGA',0
db 'PCX',0
db 'XCF',0
db 'PBM',0
db 'PGM',0
db 'PNM',0
db 'PPM',0
db 'TIF',0
db 'TIFF',0
db 'WBMP',0
db 'WEBP',0
db 'XBM',0
.end:
db 0

draw_window_fake:
        ret
;------------------------------------------------------------------------------
scale_mode_calc dd scale_none_calc, 0, 0, 0, scale_fit_min_calc

scale_none_mod    dd 0
scale_none_key    dd 13 ; '='
scale_fit_min_mod dd 0
scale_fit_min_key dd 17 ; 'w'

move_pictport_left_1_mod dd 0
move_pictport_left_1_key dd 35 ; 'h'
move_pictport_left_2_mod dd 0
move_pictport_left_2_key dd 75 ; arrow left

move_pictport_right_1_mod dd 0
move_pictport_right_1_key dd 38 ; 'l'
move_pictport_right_2_mod dd 0
move_pictport_right_2_key dd 77 ; arrow right

move_pictport_up_1_mod dd 0
move_pictport_up_1_key dd 37 ; 'k'
move_pictport_up_2_mod dd 0
move_pictport_up_2_key dd 72 ; arrow up

move_pictport_down_1_mod dd 0
move_pictport_down_1_key dd 36 ; 'j'
move_pictport_down_2_mod dd 0
move_pictport_down_2_key dd 80 ; arrow down

;shift_left_down_mod

;include_debug_strings

readdir_fileinfo:
    dd  1
    dd  0
    dd  0
readblocks dd   0
directory_ptr   dd 0
curdir          rb 1024
;------------------------------------------------------------------------------

I_END:
align 4
img_data_len    rd 1
fh              rd 1
orig_image      rd 1
cur_image       rd 1
files_num       rd 1
cur_file_idx    rd 1
cur_frame_time  rd 1
cur_frame       rd 1

next_mod          rd 1
next_key          rd 1
prev_mod          rd 1
prev_key          rd 1
slide_mod         rd 1
slide_key         rd 1
tglbar_mod        rd 1
tglbar_key        rd 1


last_name_component rd 1
toolbar.height_old  rd 1

procinfo        process_information
scale_button_xy dd ?
scale_button_letter dd ?        ; i.e. 'x1',0
align 16
path            rb 4096
window_header   rb 256
window_header_len rd 1
__params        rb 4096
;---------------------------------------------------------------------
sys_path rb 1024
temp_dir_path:
        rb 4096
;---------------------------------------------------------------------
    rb 4096
stacktop:
;---------------------------------------------------------------------
E_END:
