set_rect_window:
        mov     ebx,set_rect_window_procinfo
        call    get_slot_n

        mov     [set_rect_window_slot],ecx

        set_events_mask (evm_redraw+evm_key+evm_button+evm_mouse)
.red:
        edit_boxes_set_sys_color rect_input,rect_input_end,sc
        labels_set_sys_color rect_input_labels,rect_input_labels_end,sc
        check_boxes_set_sys_color riw_check_boxes,riw_check_boxes_end,sc
        call    .draw_window
.still:
        wait_event .red,.key,.button,.mouse

.key:
        get_key

        push    dword rect_input.left
        call    [edit_box_key]

        push    dword rect_input.top
        call    [edit_box_key]

        push    dword rect_input.width
        call    [edit_box_key]

        push    dword rect_input.height
        call    [edit_box_key]
;        key_edit_boxes rect_input,rect_input_end

        call    read_rect
        call    draw_rect_on_screen

        jmp     .still

.button:
        get_pressed_button

        cmp     ah,1
        jne     @f
        btr     dword [flags],3
        jmp     close
@@:

        jmp     .still

.mouse:
        get_active_window
        cmp     eax,[set_rect_window_slot]
        jne     .still

        push    dword rect_input.left
        call    [edit_box_mouse]

        push    dword rect_input.top
        call    [edit_box_mouse]

        push    dword rect_input.width
        call    [edit_box_mouse]

        push    dword rect_input.height
        call    [edit_box_mouse]

;;;;;;;;;;;;;;;;;;;;;;
        push    dword use_rect_active_window
        call    [check_box_mouse]

;        mouse_edit_boxes rect_input,rect_input_end
;        mouse_check_boxes riw_check_boxes,riw_check_boxes_end

        jmp     .still

.draw_window:
        start_draw_window

        xor     eax,eax
        mov     ebx,100*65536+250
        mov     ecx,100*65536+130
        mov     edx,[sc.work]
        add     edx,0x33000000
        mov     edi,riw_grab_text
        xor     esi,esi
        mcall

        draw_labels rect_input_labels,rect_input_labels_end

        push    dword rect_input.left
        call    [edit_box_draw]

        push    dword rect_input.top
        call    [edit_box_draw]

        push    dword rect_input.width
        call    [edit_box_draw]

        push    dword rect_input.height
        call    [edit_box_draw]
;;;;;;;;;;;;;;;;;;;;;;
        push    dword use_rect_active_window
        call    [check_box_draw]
;        draw_edit_boxes rect_input,rect_input_end
;        draw_check_boxes riw_check_boxes,riw_check_boxes_end

        call    read_rect
        call    draw_rect_on_screen

        stop_draw_window
ret

;--------------------------------------------------------------------
;--- Считывание облати ----------------------------------------------
;--------------------------------------------------------------------
read_rect:
        bt      dword [use_rect_active_window.flags],1
        jc      read_rect_from_active_window

        mov     edi,rect_input_buffer.left
        call    zstr_to_int
        cmp     ax,[scr.width]
        jb      @f
        mov     ax,[scr.width]
@@:
        mov     [rect.left],ax

        mov     edi,rect_input_buffer.top
        call    zstr_to_int
        cmp     ax,[scr.height]
        jb      @f
        mov     ax,[scr.height]
@@:
        mov     [rect.top],ax

        mov     edi,rect_input_buffer.width
        call    zstr_to_int
        mov     bx,[scr.width]
        sub     bx,[rect.left]
        cmp     ax,bx
        jb      @f
        mov     ax,bx
@@:
        mov     [rect.width],ax

        mov     edi,rect_input_buffer.height
        call    zstr_to_int
        mov     bx,[scr.height]
        sub     bx,[rect.top]
        cmp     ax,bx
        jb      @f
        mov     ax,bx
@@:
        mov     [rect.height],ax
ret

read_rect_from_active_window:
        call    get_active_window_info

        mov     eax,[active_app.left]
        mov     [rect.left],ax
        mov     eax,[active_app.top]
        mov     [rect.top],ax
        mov     eax,[active_app.width]
        inc     eax
        mov     [rect.width],ax
        mov     eax,[active_app.height]
        inc     eax
        mov     [rect.height],ax
ret

draw_rect_on_screen:

        mov     eax,13
        mov     ebx,150*65536+80
        mov     ecx,5*65536+60
        xor     edx,edx
        mcall

        ;movzx   eax,word [rect.left]
        ;mul     word [scr.width]
        xor     edx,edx
        movzx   eax,word [scr.width]
        mov     ebx,80
        div     ebx
        mov     ebx,eax

        xor     edx,edx
        movzx   eax,word [rect.height]
        div     ebx
        push    ax

        xor     edx,edx
        movzx   eax,word [rect.width]
        div     ebx
        push    ax

        xor     edx,edx
        movzx   eax,word [rect.top]
        div     ebx
        push    ax

        xor     edx,edx
        movzx   eax,word [rect.left]
        div     ebx
        push    ax

        pop     bx
        add     bx,150
        shl    ebx,16

        pop     cx
        add     cx,5
        shl     ecx,16

        pop     bx
        pop     cx

        mov     edx,0xffffff
        mov     eax,13
        mcall
ret

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;DATA данные
;Всегда соблюдать последовательность в имени.

system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_ProcLib	db '/sys/lib/proc_lib.obj',0

head_f_i:
head_f_l	db 'System error',0

err_message_found_lib1	db 'box_lib.obj - Not found!',0
err_message_found_lib2	db 'proc_lib.obj - Not found!',0

err_message_import1	db 'box_lib.obj - Wrong import!',0
err_message_import2	db 'proc_lib.obj - Wrong import!',0

;---------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init		dd aOpenDialog_Init
OpenDialog_Start	dd aOpenDialog_Start
;OpenDialog__Version	dd aOpenDialog_Version
        dd      0
        dd      0
aOpenDialog_Init	db 'OpenDialog_init',0
aOpenDialog_Start	db 'OpenDialog_start',0
;aOpenDialog_Version	db 'Version_OpenDialog',0
;---------------------------------------------------------------------
align 4
Box_lib_import:	
;init_lib		dd a_init
;version_lib		dd a_version


edit_box_draw		dd aEdit_box_draw
edit_box_key		dd aEdit_box_key
edit_box_mouse		dd aEdit_box_mouse
;version_ed		dd aVersion_ed


init_checkbox		dd aInit_checkbox
check_box_draw		dd aCheck_box_draw
check_box_mouse 	dd aCheck_box_mouse
;version_ch	dd	aVersion_ch

;check_box_draw		dd aCheck_box_draw
;check_box_mouse		dd aCheck_box_mouse
;version_ch		dd aVersion_ch

option_box_draw	dd aOption_box_draw
option_box_mouse	dd aOption_box_mouse
;version_op		dd aVersion_op

;scrollbar_ver_draw	dd aScrollbar_ver_draw
;scrollbar_ver_mouse	dd aScrollbar_ver_mouse
;scrollbar_hor_draw	dd aScrollbar_hor_draw
;scrollbar_hor_mouse	dd aScrollbar_hor_mouse
;version_scrollbar	dd aVersion_scrollbar

;dinamic_button_draw	dd aDbutton_draw
;dinamic_button_mouse	dd aDbutton_mouse
;version_dbutton	dd aVersion_dbutton

;menu_bar_draw		dd aMenu_bar_draw
;menu_bar_mouse		dd aMenu_bar_mouse
;menu_bar_activate	dd aMenu_bar_activate
;version_menu_bar	dd aVersion_menu_bar

;FileBrowser_draw	dd aFileBrowser_draw
;FileBrowser_mouse	dd aFileBrowser_mouse
;FileBrowser_key	dd aFileBrowser_key
;Version_FileBrowser	dd aVersion_FileBrowser

PathShow_prepare	dd sz_PathShow_prepare
PathShow_draw		dd sz_PathShow_draw
;Version_path_show	dd szVersion_path_show
			dd 0
			dd 0

;a_init			db 'lib_init',0
;a_version		db 'version',0

aEdit_box_draw		db 'edit_box',0
aEdit_box_key		db 'edit_box_key',0
aEdit_box_mouse		db 'edit_box_mouse',0
;aVersion_ed		db 'version_ed',0


aInit_checkbox	 db 'init_checkbox2',0
aCheck_box_draw  db 'check_box_draw2',0
aCheck_box_mouse db 'check_box_mouse2',0
;aVersion_ch	 db 'version_ch2',0

;aCheck_box_draw	db 'check_box_draw',0
;aCheck_box_mouse	db 'check_box_mouse',0
;aVersion_ch		db 'version_ch',0

aOption_box_draw	db 'option_box_draw',0
aOption_box_mouse	db 'option_box_mouse',0
;aVersion_op		db 'version_op',0

;aScrollbar_ver_draw	db 'scrollbar_v_draw',0
;aScrollbar_ver_mouse	db 'scrollbar_v_mouse',0
;aScrollbar_hor_draw	db 'scrollbar_h_draw',0
;aScrollbar_hor_mouse	db 'scrollbar_h_mouse',0
;aVersion_scrollbar	db 'version_scrollbar',0

;aDbutton_draw		db 'dbutton_draw',0
;aDbutton_mouse		db 'dbutton_mouse',0
;aVersion_dbutton	db 'version_dbutton',0

;aMenu_bar_draw		db 'menu_bar_draw',0
;aMenu_bar_mouse		db 'menu_bar_mouse',0
;aMenu_bar_activate	db 'menu_bar_activate',0
;aVersion_menu_bar	db 'version_menu_bar',0

;aFileBrowser_draw	db 'FileBrowser_draw',0
;aFileBrowser_mouse	db 'FileBrowser_mouse',0
;aFileBrowser_key	db 'FileBrowser_key',0
;aVersion_FileBrowser	db 'version_FileBrowser',0

sz_PathShow_prepare	db 'PathShow_prepare',0
sz_PathShow_draw	db 'PathShow_draw',0
;szVersion_path_show	db 'version_PathShow',0
;---------------------------------------------------------------------

;width,left,top,color,shift_color,focus_border_color,\
;       blur_border_color,text_color,max,text,mouse_variable,flags,size,pos

rect_input:
.left   edit_box 35,95,5, cl_white,0,0,0,5,rect_input_buffer.left,mouse_dd1,ed_figure_only+ed_focus
.top    edit_box 35,95,25,cl_white,0,0,0,5,rect_input_buffer.top,mouse_dd1,ed_figure_only
.width  edit_box 35,95,45,cl_white,0,0,0,5,rect_input_buffer.width,mouse_dd1,ed_figure_only
.height edit_box 35,95,65,cl_white,0,0,0,5,rect_input_buffer.height,mouse_dd1,ed_figure_only
rect_input_end:
mouse_dd1	rd 1
rect_input_labels:
.left label 10,10,0,rect_input_labels_text.left
.top label 10,30,0,rect_input_labels_text.top
.width label 10,50,0,rect_input_labels_text.width
.height label 10,70,0,rect_input_labels_text.height
rect_input_labels_end:

rect_input_labels_text:
.left db 'Отступ слева:',0
.top  db 'Отступ сверху:',0
.width  db 'Ширина:',0
.height db 'Высота:',0

riw_check_boxes:
use_rect_active_window check_box 10,85,5,11,cl_white,0,0,\
riw_check_boxes_text.1,\
riw_check_boxes_text.2-riw_check_boxes_text.1
riw_check_boxes_end:

riw_check_boxes_text:
.1 db 'Использовать область активного окна'
.2 db ''

riw_grab_text db 'Координаты и размеры области:',0