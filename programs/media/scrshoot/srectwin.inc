align 4
set_rect_window:
        mov     ebx,set_rect_window_procinfo
        call    get_slot_n

        mov     [set_rect_window_slot],ecx

        set_events_mask (evm_redraw+evm_key+evm_button+evm_mouse)
        call    init_rect
        edit_boxes_set_sys_color rect_input,rect_input_end,sc
.red:
        labels_set_sys_color rect_input_labels,rect_input_labels_end,sc
        check_boxes_set_sys_color2 riw_check_boxes,riw_check_boxes_end,sc
        call    .draw_window
align 4
.still:
        wait_event .red,.key,.button,.mouse

.key:
        get_key

        stdcall    [edit_box_key], rect_input.left
        stdcall    [edit_box_key], rect_input.top
        stdcall    [edit_box_key], rect_input.width
        stdcall    [edit_box_key], rect_input.height

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

        stdcall    [edit_box_mouse], rect_input.left
        stdcall    [edit_box_mouse], rect_input.top
        stdcall    [edit_box_mouse], rect_input.width
        stdcall    [edit_box_mouse], rect_input.height

;;;;;;;;;;;;;;;;;;;;;;
        stdcall    [check_box_mouse], use_rect_active_window

        jmp     .still

align 4
.draw_window:
        start_draw_window

        mov     edx,[sc.work]
        add     edx,0x33000000
        mov     edi,riw_grab_text
        xor     esi,esi
        mcall   SF_CREATE_WINDOW, 100*65536+250, 100*65536+130

        draw_labels rect_input_labels,rect_input_labels_end

        stdcall    [edit_box_draw], rect_input.left
        stdcall    [edit_box_draw], rect_input.top
        stdcall    [edit_box_draw], rect_input.width
        stdcall    [edit_box_draw], rect_input.height
;;;;;;;;;;;;;;;;;;;;;;
        stdcall    [check_box_draw], use_rect_active_window

        call    read_rect
        call    draw_rect_on_screen

        stop_draw_window
ret

align 4
init_rect:
        bt      dword [use_rect_active_window.flags],1
        jc      init_rect_from_active_window
pushad
        mov     edi,rect_input_buffer.left
        movsx   eax,word[rect.left]
		mov     ecx,rect_input.left
		call    init_editbox
        mov     edi,rect_input_buffer.top
        movsx   eax,word[rect.top]
		mov     ecx,rect_input.top
		call    init_editbox
        mov     edi,rect_input_buffer.width
        movsx   eax,word[rect.width]
		mov     ecx,rect_input.width
		call    init_editbox
        mov     edi,rect_input_buffer.height
        movsx   eax,word[rect.height]
		mov     ecx,rect_input.height
		call    init_editbox
popad
ret

align 4
init_rect_from_active_window:

ret

align 4
init_editbox:
		push    edi
		push    ecx
		xor     ebx,ebx
		inc     ebx
		cmp     eax,10
		jl @f
		inc     ebx
		@@:
		cmp     eax,100
		jl @f
		inc     ebx
		@@:
		cmp     eax,1000
		jl @f
		inc     ebx
		@@:
		call    int_to_str
		call    [edit_box_set_text] ;ecx,edi
ret
;--------------------------------------------------------------------
;--- Считывание области ---------------------------------------------
;--------------------------------------------------------------------
align 4
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

align 4
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

align 4
draw_rect_on_screen:

        xor     edx,edx
        mcall   SF_DRAW_RECT, 150*65536+80, 5*65536+60

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

        mcall   SF_DRAW_RECT,,,0xffffff
ret

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;DATA данные

system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_ProcLib	db '/sys/lib/proc_lib.obj',0
system_dir_LibImg   db '/sys/lib/libimg.obj',0

;---------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init		dd aOpenDialog_Init
OpenDialog_Start	dd aOpenDialog_Start
;OpenDialog__Version	dd aOpenDialog_Version
        dd      0,0
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
edit_box_set_text	dd aEdit_box_set_text
;version_ed		dd aVersion_ed

init_checkbox		dd aInit_checkbox
check_box_draw		dd aCheck_box_draw
check_box_mouse 	dd aCheck_box_mouse
;version_ch	dd	aVersion_ch

option_box_draw	dd aOption_box_draw
option_box_mouse	dd aOption_box_mouse
;version_op		dd aVersion_op

PathShow_prepare	dd sz_PathShow_prepare
PathShow_draw		dd sz_PathShow_draw
;Version_path_show	dd szVersion_path_show
			dd 0,0

;a_init			db 'lib_init',0
;a_version		db 'version',0

aEdit_box_draw		db 'edit_box_draw',0
aEdit_box_key		db 'edit_box_key',0
aEdit_box_mouse		db 'edit_box_mouse',0
aEdit_box_set_text	db 'edit_box_set_text',0
;aVersion_ed		db 'version_ed',0


aInit_checkbox	 db 'init_checkbox2',0
aCheck_box_draw  db 'check_box_draw2',0
aCheck_box_mouse db 'check_box_mouse2',0
;aVersion_ch	 db 'version_ch2',0

aOption_box_draw	db 'option_box_draw',0
aOption_box_mouse	db 'option_box_mouse',0
;aVersion_op		db 'version_op',0

sz_PathShow_prepare	db 'PathShow_prepare',0
sz_PathShow_draw	db 'PathShow_draw',0
;szVersion_path_show	db 'version_PathShow',0
;---------------------------------------------------------------------
align 4
import_libimg:
	dd alib_init1
	img_is_img  dd aimg_is_img
	img_info    dd aimg_info
	img_from_file dd aimg_from_file
	img_to_file dd aimg_to_file
	img_from_rgb dd aimg_from_rgb
	img_to_rgb  dd aimg_to_rgb
	img_to_rgb2 dd aimg_to_rgb2
	img_decode  dd aimg_decode
	img_encode  dd aimg_encode
	img_create  dd aimg_create
	img_destroy dd aimg_destroy
	img_destroy_layer dd aimg_destroy_layer
	img_count   dd aimg_count
	img_lock_bits dd aimg_lock_bits
	img_unlock_bits dd aimg_unlock_bits
	img_flip    dd aimg_flip
	img_flip_layer dd aimg_flip_layer
	img_rotate  dd aimg_rotate
	img_rotate_layer dd aimg_rotate_layer
	img_draw    dd aimg_draw
dd 0,0
	alib_init1   db 'lib_init',0
	aimg_is_img  db 'img_is_img',0
	aimg_info    db 'img_info',0
	aimg_from_file db 'img_from_file',0
	aimg_to_file db 'img_to_file',0
	aimg_from_rgb db 'img_from_rgb',0
	aimg_to_rgb  db 'img_to_rgb',0
	aimg_to_rgb2 db 'img_to_rgb2',0
	aimg_decode  db 'img_decode',0
	aimg_encode  db 'img_encode',0
	aimg_create  db 'img_create',0
	aimg_destroy db 'img_destroy',0
	aimg_destroy_layer db 'img_destroy_layer',0
	aimg_count   db 'img_count',0
	aimg_lock_bits db 'img_lock_bits',0
	aimg_unlock_bits db 'img_unlock_bits',0
	aimg_flip    db 'img_flip',0
	aimg_flip_layer db 'img_flip_layer',0
	aimg_rotate  db 'img_rotate',0
	aimg_rotate_layer db 'img_rotate_layer',0
	aimg_draw    db 'img_draw',0
;---------------------------------------------------------------------
;width,left,top,color,shift_color,focus_border_color,\
;       blur_border_color,text_color,max,text,mouse_variable,flags,size,pos

rect_input:
.left   edit_box 35,95,5, cl_white,0,0,0,0,5,rect_input_buffer.left,  mouse_dd1,ed_figure_only ;+ed_focus
.top    edit_box 35,95,25,cl_white,0,0,0,0,5,rect_input_buffer.top,   mouse_dd1,ed_figure_only
.width  edit_box 35,95,45,cl_white,0,0,0,0,5,rect_input_buffer.width, mouse_dd1,ed_figure_only
.height edit_box 35,95,65,cl_white,0,0,0,0,5,rect_input_buffer.height,mouse_dd1,ed_figure_only
rect_input_end:
mouse_dd1	rd 1
rect_input_labels:
.left   label 10,10,0,rect_input_labels_text.left
.top    label 10,30,0,rect_input_labels_text.top
.width  label 10,50,0,rect_input_labels_text.width
.height label 10,70,0,rect_input_labels_text.height
rect_input_labels_end:

rect_input_labels_text:
if lang eq ru
.left db 'Отступ слева:',0
.top  db 'Отступ сверху:',0
.width  db 'Ширина:',0
.height db 'Высота:',0
else
.left db 'Left:',0
.top  db 'Top:',0
.width  db 'Width:',0
.height db 'Height:',0
end if

riw_check_boxes:
use_rect_active_window check_box2 (10 shl 16)+10,(85 shl 16) +10,5,cl_white,0,0,\
riw_check_boxes_text, ch_flag_bottom
riw_check_boxes_end:

if lang eq ru
riw_check_boxes_text db 'Использовать область активного окна',0
riw_grab_text db 'Координаты и размеры области:',0
else
riw_check_boxes_text db 'Use area of the active window',0
riw_grab_text db 'Coordinates and size of the field:',0
end if