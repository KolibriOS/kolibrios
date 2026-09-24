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
; KFM v0.49 22/09/2026
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

;---------------------------------------------------------------------
include   'files.inc'
;---------------------------------------------------------------------
STRLEN = 1024

@use_library
;---------------------------------------------------------------------
align 4
START:
    load_libraries l_libs_start,end_l_libs
    cmp   eax,-1
    jz    exit_apl
    purge copy_path

    stdcall	[sort_init], 1

    mcall   SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
    mcall   SF_THREAD_INFO, procinfo,-1
    mov     ecx,[ebx+30]    ; PID
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
get_window_param:
    mcall SF_THREAD_INFO, procinfo, -1
    mov   eax,[ebx+46]
    mov   [window_high],eax
    mov   eax,[ebx+42]
    mov   [window_width],eax
    mov   eax,[ebx+70]
    mov   [window_status],eax
    mcall SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
    mov   [skin_high],eax
    ret
;---------------------------------------------------------------------
align 4
draw_window:
    mcall SF_REDRAW, SSF_BEGIN_DRAW
        xor     esi,esi
    mcall SF_CREATE_WINDOW, <20,728>, <20,460>, 0x63cccccc   ; 0x805080D0, 0x005080D0
    call  get_window_param

    mcall SF_SET_CAPTION, 1, header_text

        test    [window_status],100b    ; window is rolled up
        jnz     .exit

        test    [window_status],10b     ; window is minimized to panel
        jnz     .exit

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
    mov   ebx,ini_file_name
    mov   esi,path
    mov   edi,file_name
    call  copy_path
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
    mov   ebp,icons_associations
    call  search_star_and_end_tags
    mov   eax,[end_tag]
    mov   [icons_end_tag],eax
	jmp   @f
.err_msg:
	notify_window_run error_open_ini_file
@@:
    ret
;---------------------------------------------------------------------
align 4
add_memory_for_folders:
    mov   ecx,304*32+32
    mov   [lfd_size],ecx
    mov   [rfd_size],ecx
    mcall SF_SYS_MISC, SSF_MEM_ALLOC
    mov   [left_folder_data],eax
    mov   [read_folder.return],eax
    mcall SF_SYS_MISC, SSF_MEM_ALLOC
    mov   [right_folder_data],eax
    mov   [read_folder_1.return],eax
    ret
;---------------------------------------------------------------------
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
    mov  [confirmation_type],exit_type
    call confirmation_action
    cmp  [work_confirmation_yes],1
    jne  red
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
include   'scroll.inc'
;---------------------------------------------------------------------
include   'file_inf.inc'
;---------------------------------------------------------------------
include   'text.inc'
;---------------------------------------------------------------------
plugins_directory db 0

system_dir_Boxlib db '/sys/lib/box_lib.obj',0
system_dir_Sort 	db '/sys/lib/sort.obj',0
system_dir_Kmenu 	db '/sys/lib/kmenu.obj',0

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
scrollbar_v_draw dd scroll_bar_vertical.draw
scrollbar_v_mouse  dd scroll_bar_vertical.mouse
scrollbar_h_draw dd scroll_bar_horizontal.draw
scrollbar_h_mouse  dd scroll_bar_horizontal.mouse
end if

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
	dd 0,0
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

align 4
mouse_scroll_data:
    .vertical   rw 1
    .horizontal rw 1
scroll_bar_event rb 1
scroll_pointer rb 1
align 4
sb_left  scrollbar 15, 348, 200, 24+FILE_BR_TOP_LINE, 16, 5, 1, 0, 0xeeeeee, 0xbbddff, 0, 1
sb_right scrollbar 15, 708, 200, 24+FILE_BR_TOP_LINE, 16, 5, 1, 0, 0xeeeeee, 0xbbddff, 0, 1

align 16
I_END:
;---------------------------------------------------------------------
include   'data.inc'
;---------------------------------------------------------------------
mem: