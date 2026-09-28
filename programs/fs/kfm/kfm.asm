; SPDX-License-Identifier: NOASSERTION
;

;*****************************************************************************
; KFM - Kolibri File Manager
; Copyright (c) 2006 - 2026, Marat Zakiyanov aka Mario79, aka Mario
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;        * Redistributions of source code must retain the above copyright
;          notice, this list of conditions and the following disclaimer.
;        * Redistributions in binary form must reproduce the above copyright
;          notice, this list of conditions and the following disclaimer in the
;          documentation and/or other materials provided with the distribution.
;        * Neither the name of the <organization> nor the
;          names of its contributors may be used to endorse or promote products
;          derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Marat Zakiyanov ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*****************************************************************************
; KFM v0.49 07/10/2026
;---------------------------------------------------------------------
use32
org     0x0

	db 'MENUET01'
	dd 1, START, I_END, mem, stacktop, 0, path

;include   'lang.inc'
include '../../proc32.inc'
include '../../macros.inc'
;include '../../debug.inc'           ;for nightbuild
include '../../KOSfuncs.inc'

;define __DEBUG__ 1
;define __DEBUG_LEVEL__ 1
;include '../../debug-fdo.inc'
include '../../load_lib.mac'
include '../../develop/libraries/box_lib/box_lib.mac'

USE_STATIC_LIBS equ 0

F_BUT_PANEL_H    equ 23
FILE_BR_TOP_LINE equ F_BUT_PANEL_H+52
FILE_BR_SEL_DR_W equ 60
FILE_BR_COL_LEFT equ 60+15 ;ширина списка + отступ до кнопки name
file_br_col_w1   dd   ? ;name
FILE_BR_COL_W2   equ 45 ;type
FILE_BR_COL_W3   equ 45 ;size
FILE_BR_COL_W4   equ 70 ;date

ICON_TOP_B  = 1 ; top border
ICON_BOT_B  = 1
ICON_LEFT_B = 1 ; left border
ICON_RIGHT_B = 1
BUTTON_SIZE_W = (18+ICON_LEFT_B+ICON_RIGHT_B)
BUTTON_SIZE_H = (18+ICON_TOP_B+ICON_BOT_B)
BUTTON_W_SP = (BUTTON_SIZE_W+6)
ICON_SIZE   = BUTTON_SIZE_W*BUTTON_SIZE_H*3

;---------------------------------------------------------------------
include   'files.inc'
;---------------------------------------------------------------------
STRLEN = 1024

@use_library
;---------------------------------------------------------------------
align 4
START:
    load_libraries l_libs_start,end_l_libs
    purge copy_path

if USE_STATIC_LIBS eq 0
    cmp   [library01.status_lib],0 ;boxlib
    jnz   exit_apl.t_p
end if
    cmp   [library02.status_lib],0 ;sort
    jz   @f
    mov   [sort_init], f_null_1
    mov   [sort_dir], f_null_3
@@:
    cmp   [library03.status_lib],0 ;menu
    jz   @f
    mov   [kmainmenu_dispatch_cursorevent], f_null_1
    mov   [kmainmenu_draw], f_null_1
@@:

    stdcall	[sort_init], 1
    mcall   SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
    ;mov     eax,[sc.work_text]
    ;mov     [ps_dialog.font_color],eax
    ;mov     eax,[sc.work_light]
    ;mov     [ps_dialog.background_color],eax

    mcall   SF_SYS_MISC, SSF_MEM_OPEN, str_icon_18,, 0
    mov     [fb_left.icon_raw_area],eax
    mov     [fb_left.resolution_raw],32
    mov     [fb_right.icon_raw_area],eax
    mov     [fb_right.resolution_raw],32

if USE_STATIC_LIBS eq 0
    ; read icons
    mcall   SF_SYS_MISC, SSF_MEM_OPEN, str_icon_18w,, 0
    or      eax, eax
    jz      @f
    mov     [icons_max_size], edx
    mov     esi, eax
    stdcall copy_icon, buttons_file_data,esi,1
    stdcall copy_icon, eax,esi,66
    stdcall copy_icon, eax,esi,0
    stdcall copy_icon, eax,esi,20
    stdcall copy_icon, eax,esi,31 ;30?
    stdcall copy_icon, eax,esi,55
    stdcall copy_icon, eax,esi,56
    stdcall copy_icon, eax,esi,67
end if

    mcall   SF_THREAD_INFO, procinfo,-1
    mov     ecx,[procinfo.PID]
    mcall   SF_SYSTEM, SSF_GET_THREAD_SLOT
    mov     [active_process],eax    ; WINDOW SLOT

    mov   ax,[select_disk_char]
    mov   [read_folder_name],ax
    mov   [read_folder_1_name],ax
    call  load_initiation_file

    call  init_menu
    call  add_memory_for_folders
    call  device_detect_f70
    call  select_starting_directories
    mcall SF_KEYBOARD, SSF_SET_INPUT_MODE, 1
    mov   eax,2
    mov   [left_sort_flag],eax
    mov   [right_sort_flag],eax

    call  proc_read_left_folder
    test  eax,eax
    jz    @f

    cmp   eax,6
    jne   read_folder_error
@@:
    call  proc_read_right_folder
    test  eax,eax
    jz    @f

    cmp   eax,6
    je    @f
; if /hd read error for start then use /rd
    mov   esi,retrieved_devices_table+1
    call  copy_folder_name_1
    call  proc_read_right_folder
    test  eax,eax
    jz    @f

    cmp   eax,6
    jne   read_folder_1_error
@@:
        mcall SF_SET_EVENTS_MASK, EVM_MOUSE + EVM_BUTTON + EVM_KEY + EVM_REDRAW
        jmp   red_1
;---------------------------------------------------------------------
align 4
red:
    call  get_window_param
    test  [window_status],10b
    jnz   red_1   ;still
    test  [window_status],100b
    jnz   red_1
    cmp   [window_high],180
    ja    @f
    mov   esi,180
    mcall SF_CHANGE_WINDOW, -1,ebx,ebx
@@:
    cmp   [window_width],495
    ja    red_1
    mov   edx,495
    mcall SF_CHANGE_WINDOW, -1,ebx, ,ebx
red_1:
    call  draw_window
;---------------------------------------------------------------------
align 16
still:
    mcall SF_WAIT_EVENT

    call  check_active_process_for_clear_all_flags

    cmp   eax,EV_REDRAW
    je    red
    cmp   eax,EV_KEY
    je    key
    cmp   eax,EV_BUTTON
    je    button
    cmp   eax,EV_MOUSE
    je    mouse
    jmp   still
;---------------------------------------------------------------------
align 4
check_active_process_for_clear_all_flags:
        push    eax
        mcall   SF_SYSTEM, SSF_GET_ACTIVE_WINDOW
        cmp     [active_process],eax
        je      .exit

        xor     eax,eax
        cmp     [shift_flag],al
        jne     .clear_all_flags

        cmp     [ctrl_flag],al
        jne     .clear_all_flags

        cmp     [ctrl_flag],al
        je      .exit
;--------------------------------------
.clear_all_flags:
        mov     [shift_flag],al
        mov     [ctrl_flag],al
        mov     [alt_flag],al
        call    erase_fbutton
        call    draw_fbutton
;--------------------------------------
.exit:
        pop     eax
        ret
;---------------------------------------------------------------------
align 4
get_window_param:
    mcall SF_THREAD_INFO, procinfo, -1
    mov   eax,[procinfo.box.height]
    mov   [window_high],eax
    mov   eax,[procinfo.box.width]
    mov   [window_width],eax
    mov   al,[procinfo.wnd_state]
    mov   [window_status],al
    mcall SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
    mov   [skin_high],eax
    ret
;---------------------------------------------------------------------
align 4
draw_window:
    mcall SF_REDRAW, SSF_BEGIN_DRAW
    xor   esi,esi
    mcall SF_CREATE_WINDOW, <20,728>, <20,460>, 0x63cccccc   ; 0x805080D0, 0x005080D0
    call  get_window_param

    mcall SF_SET_CAPTION, 1, header_text

    test  [window_status],100b    ; window is rolled up
    jnz   .exit

    test  [window_status],10b     ; window is minimized to panel
    jnz   .exit

    cmp   [window_high],180
    jb    .exit
    cmp   [window_width],495
    jb    .exit

    call  draw_fbutton
    call  draw_left_panel
    call  draw_right_panel
    call  draw_device_button
    call  draw_left_select_disk_button
    call  draw_left_sort_button
    call  draw_right_select_disk_button
    call  draw_right_sort_button
    call  draw_menu_bar
    call  draw_buttons_panel
.exit:
    mcall SF_REDRAW, SSF_END_DRAW
    ret
;---------------------------------------------------------------------
align 4
load_initiation_file:
    mov   esi,ini_file_name
    mov   edi,file_name
    call  proc_copy_patch
    call  get_file_size
    test  eax,eax
    jnz   .err_msg
    mov   ecx,[file_features_temp_area+32]
    or    ecx,ecx
    jz    .err_msg
    push  edx
    mcall SF_SYS_MISC, SSF_MEM_REALLOC,, [ini_file_start]
    mov   [ini_file_start],eax
    mov   [ini_size],ecx
    mov   [read_file.return],eax
    mov   [read_file.size],ecx
    pop   edx
    call  load_file
    test  eax,eax
    jnz   .err_msg
    call  calc_ini
    jmp   @f
.err_msg:
    notify_window_run error_open_ini_file
@@:
    ret
;---------------------------------------------------------------------
align 4
calc_ini:
; Convert the new "ID=ext,ext,..." icons18 layout from the loaded ini file into
; the "ext=ID" CR/LF text that box_lib's FileBrowser expects, storing the result
; in converted_ini_buffer. Every scan is bounded by ini_src_end, so a truncated
; or malformed file can never read past the loaded buffer (it has no 0 terminator).
	pusha
	mov	esi, [ini_file_start]
	mov	eax, esi
	add	eax, [ini_size]
	mov	[ini_src_end], eax
	mov	edi, converted_ini_buffer
	mov	[fb_left.ini_file_start], edi
	mov	[fb_right.ini_file_start], edi

; find the "[icons18]" section header
.find_section:
	cmp	esi, [ini_src_end]
	jae	.finished
	lodsb
	cmp	al, '['
	jne	.find_section
	cmp	dword [esi], 'icon'
	jne	.find_section
.skip_header:
	cmp	esi, [ini_src_end]
	jae	.finished
	lodsb
	cmp	al, 10
	jne	.skip_header

; convert each "ID=ext,ext,..." line until the next section or EOF
.parse_line:
	cmp	esi, [ini_src_end]
	jae	.finished
	cmp	byte [esi], '['
	je	.finished
	mov	edx, esi		; edx = ID start
.find_eq:
	cmp	esi, [ini_src_end]
	jae	.finished
	lodsb
	cmp	al, '='
	jne	.find_eq
	lea	ebx, [esi-1]
	sub	ebx, edx		; ebx = ID length

.next_ext:
	mov	ebp, esi		; ebp = extension start
.scan_ext:
	cmp	esi, [ini_src_end]
	jae	.finished
	lodsb
	cmp	al, ','
	je	.ext_sep
	cmp	al, 13
	je	.ext_eol
	cmp	al, 10
	je	.ext_eol
	jmp	.scan_ext
.ext_sep:
	call	.emit_ext
	jmp	.next_ext
.ext_eol:
	call	.emit_ext
.skip_eol:				; absorb the whole CR/LF run (and blank lines)
	cmp	esi, [ini_src_end]
	jae	.finished
	lodsb
	cmp	al, 13
	je	.skip_eol
	cmp	al, 10
	je	.skip_eol
	dec	esi
	jmp	.parse_line

.finished:
	mov	[fb_left.ini_file_end], edi
	mov	[fb_right.ini_file_end], edi
	popa
	ret

; Emit one "ext=ID\r\n" record. esi-1 = delimiter just consumed, ebp = ext start,
; edx/ebx = ID start/length. Empty extensions are skipped, esi is preserved.
.emit_ext:
	push	esi
	lea	ecx, [esi-1]
	sub	ecx, ebp		; ecx = extension length
	jecxz	.emit_done		; drop empty extension
	mov	esi, ebp
	rep	movsb			; ext
	mov	al, '='
	stosb
	mov	esi, edx
	mov	ecx, ebx
	rep	movsb			; ID
	mov	ax, 0x0A0D
	stosw				; CR, LF
.emit_done:
	pop	esi
	ret
;---------------------------------------------------------------------
align 4
add_memory_for_folders:
    mov   ecx,304*32+32
    mov   [lfd_size],ecx
    mov   [rfd_size],ecx
    mcall SF_SYS_MISC, SSF_MEM_ALLOC
    mov   [fb_left.folder_data],eax
    mov   [read_folder.return],eax
    mcall SF_SYS_MISC, SSF_MEM_ALLOC
    mov   [fb_right.folder_data],eax
    mov   [read_folder_1.return],eax
    ret
;---------------------------------------------------------------------
;in:
;  ebx - file name1
;  edi - buffer
;  esi - file path + '/' + file name0
;out:
;  edi = file path + '/' + file name1
align 4
copy_path:
    xor   eax,eax
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    mov   esi,edi
@@:
    std
    lodsb
    cmp   al,'/'
    jnz   @b
    mov   edi,esi
    add   edi,2
    mov   esi,ebx
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    ret
;---------------------------------------------------------------------
;in:
;  ebx - file name
;  edi - buffer
;  esi - file path
;out:
;  edi = file path + '/' + file name
align 4
copy_path_1:
    xor   eax,eax
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    mov   esi,ebx
    mov   [edi-1],byte '/'
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    ret
;---------------------------------------------------------------------
exit_apl:
    mov  [confirmation_type],sz_Exit
    call confirmation_action
    cmp  [work_confirmation_yes],1
    jne  red
.t_p:
    mcall SF_TERMINATE_PROCESS
;---------------------------------------------------------------------
include   'key.inc'
;---------------------------------------------------------------------
include   'markfile.inc'
;---------------------------------------------------------------------
include   'button.inc'
;---------------------------------------------------------------------
include   'mouse.inc'
;---------------------------------------------------------------------
include   'openfile.inc'
;---------------------------------------------------------------------
include   'draw.inc'
;---------------------------------------------------------------------
include   'drw_dbut.inc'
;---------------------------------------------------------------------
include   'menu_bar.inc'
;---------------------------------------------------------------------
include   'menu_drv.inc'
;---------------------------------------------------------------------
include   'delete.inc'
;---------------------------------------------------------------------
include   'copy.inc'
;---------------------------------------------------------------------
include   'creatdir.inc'
;---------------------------------------------------------------------
include   'creatfile.inc'
;---------------------------------------------------------------------
include   'confirm.inc'
;---------------------------------------------------------------------
include   'err_wind.inc'
;---------------------------------------------------------------------
include   'detect.inc'
;---------------------------------------------------------------------
include   'tran_ini.inc'
;---------------------------------------------------------------------
include   'help.inc'
;---------------------------------------------------------------------
include   'convchar.inc'
;---------------------------------------------------------------------
include   'sort.inc'
;---------------------------------------------------------------------
include   'progrbar.inc'
;---------------------------------------------------------------------
include   'file_inf.inc'
;---------------------------------------------------------------------
include   'text.inc'
;---------------------------------------------------------------------
plugins_directory db 0

system_dir_Boxlib db '/sys/lib/box_lib.obj',0
system_dir_Sort   db '/sys/lib/sort.obj',0
system_dir_Kmenu  db '/sys/lib/kmenu.obj',0

align 4
l_libs_start:
if USE_STATIC_LIBS eq 0
library01	l_libs	system_dir_Boxlib+9,file_name,system_dir_Boxlib,\
import_box_lib,plugins_directory
end if

library02	l_libs	system_dir_Sort+9,file_name,system_dir_Sort,\
Sort_import,plugins_directory

library03	l_libs	system_dir_Kmenu+9,file_name,system_dir_Kmenu,\
import_libkmenu,plugins_directory
end_l_libs:

if USE_STATIC_LIBS eq 0
include '../../develop/libraries/box_lib/import.inc'
else
include '../../develop/libraries/box_lib/keys.inc'
include '../../develop/libraries/box_lib/editbox.asm'
include '../../develop/libraries/box_lib/scrollbar.asm'
include '../../develop/libraries/box_lib/filebrowser.asm'
include '../../develop/libraries/box_lib/pathshow.asm'
scrollbar_v_draw  dd scroll_bar_vertical.draw
scrollbar_v_mouse dd scroll_bar_vertical.mouse
FileBrowser_draw  dd fb_draw_panel
FileBrowser_mouse dd fb_mouse
FileBrowser_key   dd fb_key
end if

align 4
f_null_3:
    ret 8
align 4
f_null_1:
    ret 4

align 4
proc draw_edge uses eax ebx ecx edx edi esi, box_l:dword, box_t:dword, box_w:dword, box_h:dword,\
        col_0:dword, col_1:dword, col_2:dword

	mov esi,[col_1]
	and esi,111111101111111011111110b

	mov eax,SF_DRAW_RECT
	;bottom line
	mov edx,[col_2]
	mov ebx,[box_l]
	shl ebx,16
	add ebx,[box_w]
	inc ebx ;для заливки диагональных пикселей
	mov ecx,[box_t]
	add ecx,[box_h]
	shl ecx,16
	inc ecx

	mov edi,3 ;for cycle
	@@:
		;calculate colors
		and edx,111111101111111011111110b
		add edx,esi
		shr edx,1
		;line move up and ->...<-
		sub ecx,1 shl 16 ;move up
		add ebx,1 shl 16 ;->...
		sub ebx,2 ;...<-
		;draw line
		int 0x40
		dec edi
	jnz @b

	;right line
	mov edx,[col_2]
	mov ebx,[box_l]
	add ebx,[box_w]
	shl ebx,16
	inc ebx
	mov ecx,[box_t]
	shl ecx,16
	add ecx,[box_h]

	mov edi,3 ;for cycle
	@@:
		;calculate colors
		and edx,111111101111111011111110b
		add edx,esi
		shr edx,1
		;line move left and ...
		sub ebx,1 shl 16 ;move left
		add ecx,1 shl 16
		sub ecx,2
		;draw line
		int 0x40
		dec edi
	jnz @b

	;top line
	mov edx,[col_0]
	mov ebx,[box_l]
	shl ebx,16
	add ebx,[box_w]
	mov ecx,[box_t]
	shl ecx,16
	inc ecx

	mov edi,3 ;for cycle
        @@:
		;calculate colors
		and edx,111111101111111011111110b
		add edx,esi
		shr edx,1
		;line move down and ->...<-
		add ecx,1 shl 16 ;move down
		add ebx,1 shl 16 ;->...
		sub ebx,2 ;...<-
		;draw line
		int 0x40
		dec edi
	jnz @b

	;left line
	mov edx,[col_0]
	mov ebx,[box_l]
	shl ebx,16
	inc ebx
	mov ecx,[box_t]
	shl ecx,16
	add ecx,[box_h]

	mov edi,3 ;for cycle
	@@:
		;calculate colors
		and edx,111111101111111011111110b
		add edx,esi
		shr edx,1
		;line move left and ...
		add ebx,1 shl 16 ;move left
		add ecx,1 shl 16
		sub ecx,2
		;draw line
		int 0x40
		dec edi
	jnz @b

	ret
endp

align 4
Sort_import:
sort_init	dd aSort_init
sort_version	dd aSort_version
sort_dir	dd aSort_SortDir
sort_strcmpi	dd aSort_strcmpi
sz_null	dd 0,0
aSort_init	db 'START',0
aSort_version	db 'version',0
aSort_SortDir	db 'SortDir',0
aSort_strcmpi	db 'strcmpi',0

align 4
import_libkmenu:
	kmenu_init                     dd akmenu_init
	kmainmenu_draw                 dd akmainmenu_draw
	kmainmenu_dispatch_cursorevent dd akmainmenu_dispatch_cursorevent
	ksubmenu_new                   dd aksubmenu_new
	ksubmenu_delete                dd aksubmenu_delete
	ksubmenu_draw                  dd aksubmenu_draw
	ksubmenu_add                   dd aksubmenu_add
	kmenuitem_new                  dd akmenuitem_new
	kmenuitem_delete               dd akmenuitem_delete
	kmenuitem_draw                 dd akmenuitem_draw
dd 0,0
	akmenu_init                     db 'kmenu_init',0
	akmainmenu_draw                 db 'kmainmenu_draw',0
	akmainmenu_dispatch_cursorevent db 'kmainmenu_dispatch_cursorevent',0
	aksubmenu_new                   db 'ksubmenu_new',0
	aksubmenu_delete                db 'ksubmenu_delete',0
	aksubmenu_draw                  db 'ksubmenu_draw',0
	aksubmenu_add                   db 'ksubmenu_add',0
	akmenuitem_new                  db 'kmenuitem_new',0
	akmenuitem_delete               db 'kmenuitem_delete',0
	akmenuitem_draw                 db 'kmenuitem_draw',0

;in:
;  buf_d = pointer to destination buffer 24-bit
;  buf_s = pointer to source buffer 32-bit (with icons)
;  ind   = icon index
;out:
;  eax   = pointer to destination buffer + icon size
align 4
proc copy_icon uses ebx ecx esi edi, buf_d:dword, buf_s:dword, ind:dword
    mov     edi,[buf_d]
    mov     esi,[ind]
    imul    esi,18*18*4
    cmp     esi,[icons_max_size]
    jge     .quit
    ; fill the whole button area with sc.work:
    ; write the first pixel, then propagate the 3-byte pattern with overlapping movsb
    mov     eax,[sc.work_light]
    mov     [edi],ax        ; B, G
    shr     eax,16
    mov     [edi+2],al      ; R
    push    esi
    mov     esi,edi
    add     edi,3
    mov     ecx,ICON_SIZE-3
    rep     movsb
    pop     esi
    ; copy icon into the center
    add     esi,[buf_s]
    mov     edi,[buf_d]
    add     edi,(BUTTON_SIZE_W*ICON_TOP_B+ICON_LEFT_B)*3
    mov     ebx,18
.cycle0:
    mov     ecx,18
.cycle1:
    cmp     byte[esi+3],255
    je      @f
    add     edi,3 ;skip
    add     esi,4
    loop    .cycle1
    jmp     .cycle1e
@@:
    movsw ;copy
    movsb
    inc     esi ; skip transparent byte
    loop    .cycle1
.cycle1e:
    add     edi,(ICON_RIGHT_B+ICON_LEFT_B)*3
    dec     ebx
    jnz     .cycle0

; draw shadow 1
    mov     eax,[sc.work]
    mov     esi,[ind]
    imul    esi,18*18*4
    add     esi,(18+1)*4
    add     esi,[buf_s]
    mov     edi,[buf_d]
    add     edi,(BUTTON_SIZE_W*(ICON_TOP_B+1)+ICON_LEFT_B+1)*3
    mov     ebx,18-1
.cycle2:
    mov     ecx,18-1
.cycle3:
    cmp     byte[esi+3],255
    je      @f
    cmp     byte[esi+3-(18+1)*4],255
    jne     @f
    stosw
    stosb
    jmp     .1
@@:
    add     edi,3
.1:
    add     esi,4
    loop    .cycle3
    add     edi,(ICON_RIGHT_B+ICON_LEFT_B+1)*3
    add     esi,4
    dec     ebx
    jnz     .cycle2

    ; draw shadow 2
    ;mov     eax,[sc.work]
    mov     edi,[buf_d]
    add     edi,(BUTTON_SIZE_W*(ICON_TOP_B+1)+ICON_LEFT_B+1)*3	
    mov     esi,[ind]
    imul    esi,18*18*4
    add     esi,[buf_s]
    mov     ebx,18
.cycle4:
    mov     ecx,18
.cycle5:
    cmp     ebx,1
    jle     .2
    cmp     ecx,1
    jg      @f
.2:
    cmp     byte[esi+3],255
    jne     @f
    stosw
    stosb
    jmp     .3
@@:
    add     edi,3
.3:
    add     esi,4
    loop    .cycle5
    add     edi,(ICON_RIGHT_B+ICON_LEFT_B)*3
    dec     ebx
    jnz     .cycle4

.quit:
    mov     eax,[buf_d]
    add     eax,ICON_SIZE
    ret
endp

str_icon_18 db 'ICONS18',0

if USE_STATIC_LIBS eq 0
numimages = 8
str_icon_18w db 'ICONS18W',0
end if

align 4
fb_left:
.type				dd 0 ;+0
.x:
.size_x 			dw 400 ;+4
.start_x			dw 10 ;+6
.y:
.size_y 			dw 550 ;+8
.start_y			dw FILE_BR_TOP_LINE ;+10
.icon_size_y			dw 18 ; +12
.icon_size_x			dw 18 ; +14
.line_size_x			dw 0 ; +16
.line_size_y			dw 18+1 ; +18
.type_size_x			dw 0 ; +20
.size_size_x			dw 0 ; +22
.date_size_x			dw 0 ; +24
.attributes_size_x		dw 0 ; +26
.icon_assoc_area		dd 0 ; +28
.icon_raw_area			dd 0 ; +32
.resolution_raw 		dd 0 ; +36
.palette_raw			dd 0 ; +40
.directory_path_area		dd 0 ; +44
.file_name_area 		dd 0 ; +48
.select_flag			dd 0 ; +52
.background_color		dd 0xffffff ; +56
.select_color			dd 0xbbddff ; +60
.seclect_text_color		dd 0 ; +64
.text_color			dd 0 ; +68
.reduct_text_color		dd 0xff0000 ; +72
.marked_text_color		dd 0 ; +76
.max_panel_line 		dd 0 ; +80
.select_panel_counter		dd 1 ; +84
.folder_block			dd 0 ; +88
.start_draw_line		dd 0 ; +92
.start_draw_cursor_line 	dw 0 ; +96 ; pixels
.folder_data			dd 0 ; +98
.temp_counter			dd 0 ; +102
.file_name_length		dd 0 ; +106
.marked_file			dd 0 ; +110
.extension_size 		dd 0 ; +114
.extension_start		dd 0 ; +118
.type_table			dd type_table ; +122
.ini_file_start 		dd 0 ; +126
.ini_file_end			dd 0 ; +130
.draw_scroll_bar		dd 0 ; +134
.font_size_y			dw 9 ; +138
.font_size_x			dw 6 ; +140
.mouse_keys			dd 0 ; +142
.mouse_keys_old 		dd 0 ; +146
.mouse_pos			dd 0 ; +150
.mouse_keys_delta		dd 0 ; +154
.mouse_key_delay		dd 50 ; +158
.mouse_keys_tick		dd 0 ; +162
.start_draw_cursor_line_2	dw 0 ;+166
.all_redraw			dd 0 ;+168 0 - lines, 1 - all, 2 - folders+lines
.selected_BDVK_adress		dd 0 ;+172
.key_action			dw 0 ;+176
.key_action_num 		dw 0 ;+178
.name_temp_area 		dd name_temp_area ;+180
.max_name_temp_size		dd 0 ;+184
.display_name_max_length	dd 0 ;+188
.draw_panel_selection_flag	dd 0 ;+192
.mouse_pos_old			dd 0 ;+196
.marked_counter 		dd 0 ;+200
.keymap_pointer 		dd keymap_area ;+204

align 4
fb_right:
.type				dd 0 ;+0
.x:
.size_x 			dw 400 ;+4
.start_x			dw 10 ;+6
.y:
.size_y 			dw 550 ;+8
.start_y			dw FILE_BR_TOP_LINE ;+10
.icon_size_y			dw 18 ; +12
.icon_size_x			dw 18 ; +14
.line_size_x			dw 0 ; +16
.line_size_y			dw 18+1 ; +18
.type_size_x			dw 0 ; +20
.size_size_x			dw 0 ; +22
.date_size_x			dw 0 ; +24
.attributes_size_x		dw 0 ; +26
.icon_assoc_area		dd 0 ; +28
.icon_raw_area			dd 0 ; +32
.resolution_raw 		dd 0 ; +36
.palette_raw			dd 0 ; +40
.directory_path_area		dd 0 ; +44
.file_name_area 		dd 0 ; +48
.select_flag			dd 0 ; +52
.background_color		dd 0xffffff ; +56
.select_color			dd 0xbbddff ; +60
.seclect_text_color		dd 0 ; +64
.text_color			dd 0 ; +68
.reduct_text_color		dd 0xff0000 ; +72
.marked_text_color		dd 0 ; +76
.max_panel_line 		dd 0 ; +80
.select_panel_counter		dd 0 ; +84
.folder_block			dd 0 ; +88
.start_draw_line		dd 0 ; +92
.start_draw_cursor_line 	dw 0 ; +96 ; pixels
.folder_data			dd 0 ; +98
.temp_counter			dd 0 ; +102
.file_name_length		dd 0 ; +106
.marked_file			dd 0 ; +110
.extension_size 		dd 0 ; +114
.extension_start		dd 0 ; +118
.type_table			dd type_table ; +122
.ini_file_start 		dd 0 ; +126
.ini_file_end			dd 0 ; +130
.draw_scroll_bar		dd 0 ; +134
.font_size_y			dw 9 ; +138
.font_size_x			dw 6 ; +140
.mouse_keys			dd 0 ; +142
.mouse_keys_old 		dd 0 ; +146
.mouse_pos			dd 0 ; +150
.mouse_keys_delta		dd 0 ; +154
.mouse_key_delay		dd 50 ; +158
.mouse_keys_tick		dd 0 ; +162
.start_draw_cursor_line_2	dw 0 ;+166
.all_redraw			dd 0 ;+168
.selected_BDVK_adress		dd 0 ;+172
.key_action			dw 0 ;+176
.key_action_num 		dw 0 ;+178
.name_temp_area 		dd name_temp_area ;+180
.max_name_temp_size		dd 0 ;+184
.display_name_max_length	dd 0 ;+188
.draw_panel_selection_flag	dd 0 ;+192
.mouse_pos_old			dd 0 ;+196
.marked_counter 		dd 0 ;+200
.keymap_pointer 		dd keymap_area ;+204

align 4
mouse_scroll_data:
    .vertical   rw 1
    .horizontal rw 1
align 4
sb_left  scrollbar 15, 348, 200, FILE_BR_TOP_LINE, 16, 5, 1, 0, 0xeeeeee, 0xbbddff, 0, 1
sb_right scrollbar 15, 708, 200, FILE_BR_TOP_LINE, 16, 5, 1, 0, 0xeeeeee, 0xbbddff, 0, 1

ps_left:
.type			dd 0	;+0
.start_y		dw 10	;+4
.start_x		dw 10	;+6
.font_size_x		dw 8	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw 100	;+10
.font_number		dd 1	;+12	; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0	;+20
.background_color	dd 0xffffcc	;+24
.text_pointer		dd read_folder_name	;+28
.work_area_pointer	dd initial_data	;+32
.temp_text_length	dd 0	;+36

ps_right:
.type			dd 0	;+0
.start_y		dw 10	;+4
.start_x		dw 10	;+6
.font_size_x		dw 8	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw 100	;+10
.font_number		dd 1	;+12	; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0	;+20
.background_color	dd 0xffffcc	;+24
.text_pointer		dd read_folder_1_name	;+28
.work_area_pointer	dd initial_data	;+32
.temp_text_length	dd 0	;+36

ps_dialog:
.type			dd 0	;+0
.start_y		dw 10	;+4
.start_x		dw 10	;+6
.font_size_x		dw 6	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw 100	;+10
.font_number		dd 0	;+12	; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0xffffff	;+20
.background_color	dd 0	;+24
.text_pointer		dd file_name	;+28
.work_area_pointer	dd initial_data	;+32
.temp_text_length	dd 0	;+36

align 16
I_END:
;---------------------------------------------------------------------
include   'data.inc'
;---------------------------------------------------------------------
mem: