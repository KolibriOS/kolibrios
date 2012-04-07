;*****************************************************************************
; KFM - Kolibri File Manager
; Copyright (c) 2006 - 2010, Marat Zakiyanov aka Mario79, aka Mario
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
; KFM v0.47d 07.04.2012
;---------------------------------------------------------------------
use32
org	0x0

    db	  'MENUET01'
    dd	  0x01
    dd	  START
    dd	  I_END
    dd	  mem
    dd	  stacktop
    dd	  0x0
    dd	  path

;include   'lang.inc'
;include   'kglobals.inc'
;include   'macros.inc'
include '../../../macros.inc'
include '../../../config.inc'		;for nightbuild

include   'editbox.inc'
;use_edit_box
use_edit_box procinfo
;include   'ASCGL.INC'
;---------------------------------------------------------------------
include   'files.inc'
;---------------------------------------------------------------------
STRLEN = 1024
;---------------------------------------------------------------------
START:
;    mcall 9, procinfo, -1
;    mov   eax,[ebx+30]
;    mov   [PID],eax
;    xor   ecx,ecx
;@@:
;    inc   ecx
;    mcall 9, procinfo
;    mov   eax,[PID]
;    cmp   eax,[ebx+30]
;    jne   @r
;    mov  [active_process],ecx
	mcall	9,procinfo,-1
	mov	ecx,[ebx+30]	; PID
	mcall	18,21
	mov	[active_process],eax	; WINDOW SLOT
    mov   [appl_memory],mem
    mov   ax,[select_disk_char]
    mov   [read_folder_name],ax
    mov   [read_folder_1_name],ax
    call  load_icon_and_convert_to_img
    call  load_buttons_and_convert_to_img
    call  load_initiation_file
    call  add_memory_for_folders
    call  device_detect
    call  device_detect_f70
    mcall 66, 1, 1
;    call  draw_window
;    xor   eax,eax
    mov   eax,1
    mov   [left_sort_flag],eax
    mov   [right_sort_flag],eax

    call  proc_read_left_folder
    test  eax,eax
    jz	  @f
    cmp   eax,6
    jne   read_folder_error
@@:
    call  proc_read_right_folder
    test  eax,eax
    jz	  @f
    cmp   eax,6
    jne   read_folder_1_error
@@:
	mcall 40, 0x27
	jmp   red_1
;---------------------------------------------------------------------
red:
    call  get_window_param
    test  [window_status],10b
    jnz   red_1   ;still
    test  [window_status],100b
    jnz   red_1
    cmp   [window_high],180
    ja	  @f
    mov   esi,180
    mcall 67,-1,ebx,ebx
@@:
    cmp   [window_width],495
    ja	  red_1
    mov   edx,495
    mcall 67,-1,ebx, ,ebx
red_1:
    call  draw_window
;---------------------------------------------------------------------
still:
    mcall 10
    cmp   eax,1
    je	  red
    cmp   eax,2
    je	  key
    cmp   eax,3
    je	  button
    cmp   eax,6
    je	  mouse
    jmp   still
;---------------------------------------------------------------------
get_window_param:
    mcall 9, procinfo, -1
    mov   eax,[ebx+46]
    mov   [window_high],eax
    mov   eax,[ebx+42]
    mov   [window_width],eax
    mov   eax,[ebx+70]
    mov   [window_status],eax
    mcall 48,4
    mov   [skin_high],eax
    ret
;---------------------------------------------------------------------
draw_window:
    mcall 12, 1
;    mcall 0, <20,620>, <20,460>, 0x03cccccc   ; 0x805080D0, 0x005080D0
	xor	esi,esi
    mcall 0, <20,620>, <20,460>, 0x43cccccc   ; 0x805080D0, 0x005080D0
    call  get_window_param
;    mov   ecx,[temp_esi]
	test	[window_status],100b	; window is rolled up
	jnz	.exit

	test	[window_status],10b	; window is minimized to panel
	jnz	.exit

    mcall 71, 1 , header_text
    ; create_dir_name
    ; start_parameter
    ; file_name
    ; [temp_edi]
    ; header
    ; delete_file_data.name
    ; start_file_data.name
    ; start_parameter
     ; start_file_data.name
      ; read_icon_file.name
		; read_file_features.name ;path ;header

    cmp   [window_high],180
    jb	  .exit
    cmp   [window_width],495
    jb	  .exit

;    pusha
;    mcall 4,<15,25>,0,read_folder.name,100
;    popa
    call  draw_fbutton

;    mov   [left_panel_clear_all],1

    call  draw_left_panel

;    mov   [right_panel_clear_all],1

    call  draw_right_panel
    call  draw_device_button
    call  draw_left_select_disk_button
    call  draw_left_sort_button
    call  draw_right_select_disk_button
    call  draw_right_sort_button
    call  draw_menu_bar
    call  draw_buttons_panel
    call  draw_ATAPI_tray_control
    
;    mcall 47,0x80000,[left_scroll_compens],<300, 5>,0xffffff
;    call  mouse.draw_data
;    mcall 18, 7
;    mov   [temp_eax],eax
;    mcall 47,0x80000,[active_process],<300, 5>,0xffffff
;    mcall 47,0x80000,[left_marked_counter],<300, 5>,0xffffff
;    mcall 47,0x80000,[right_marked_counter],<400, 5>,0xffffff
;    mcall 47,0x80000,[sorting_low_limit],<100, 5>,0xffffff
;    mcall 47,0x80000,[sort_counter],<200, 5>,0xffffff
;    mcall 47,0x80000,[sorting_high_limit],<300, 5>,0xffffff
;    mcall 47,0x80000,[dir_temp_counter],<400, 5>,0xffffff

;    mcall 47,0x80000,[timer_tick],<500, 5>,0xffffff
;    mcall 47,0x80000,[temp_eax],<400, 5>,0xffffff
;    mcall 47,0x80000,[temp_ebx],<400, 5>,0xffffff
;    mcall 47,0x80000,[temp_ecx],<500, 5>,0xffffff
;    mcall 47,0x80000,[temp_ebx],<500, 5>,0xffffff
;    mcall 47,0x80000,[ini_file_start],<100, 5>,0xffffff
;    mcall 47,0x80000,[left_folder_data],<200, 5>,0xffffff
;    mcall 47,0x80000,[right_folder_data],<300, 5>,0xffffff
;    mcall 47,0x80000,[appl_memory],<500, 5>,0xffffff
;    mcall 47,0x80000,[temp_znak],<500, 5>,0xffffff

;    mcall 47,0x80000,[sort_counter],<200, 5>,0xffffff
;    mcall 47,0x80000,[temp_edi],<250, 5>,0xffffff
;    mcall 47,0x80000,[temp_esi],<300, 5>,0xffffff
;    mcall 47,0x80000,[temp_ecx],<350, 5>,0xffffff
;    mcall 47,0x80000,[temp_znak],<400, 5>,0xffffff

;    movzx ecx,[left_start_draw_cursor_line]
;    mcall 47,0x40000, ,<300, 5>,0xffffff
;    mcall 47,0x40000,[left_start_draw_line],<400, 5>,0xffffff

;    mcall 47,0x40000,[window_width],<100, 5>,0xffffff
;    mcall 47,0x40000,[window_high],<130, 5>,0xffffff

;    mcall 47,0x80100,[left_panel_x],<200, 5>,0xffffff
;    mcall 47,0x80100,[left_panel_y],<250, 5>,0xffffff
;    mov   edx,[temp_counter_dword_1]
;    mcall 4,<150,3>,0x80000000
;    mov   edx,[temp_counter_dword]
;    mcall 4,<5,3>,0x80000000
.exit:
    mcall 12, 2
    ret
;temp_eax dd 0
;temp_ebx dd 0
;temp_ecx dd 0
;temp_edx dd 0
;temp_esi dd 0
;temp_edi dd 0
;temp_ebp dd 0
;temp_esp dd 0
;temp_znak dd 0
;temp_counter_dword_1 dd 0
;extension_size_1 dd 0
;timer_tick dd 0
;---------------------------------------------------------------------
prepare_load_data:
    mov   esi,path
    mov   edi,file_name
    call  copy_path
    call  get_file_size
    test  eax,eax
    ret
;---------------------------------------------------------------------
prepare_load_data_1:
    mov   [read_file.return],eax
    mov   ebp,eax
prepare_load_data_4:
    call  load_file
    test  eax,eax
    ret
;---------------------------------------------------------------------
prepare_load_data_2:
    call  add_application_memory
prepare_load_data_3:
    call  add_application_memory
    mov   eax,[file_features_temp_area+32]
    mov   [read_file.size],eax
    ret
;---------------------------------------------------------------------
load_icon_and_convert_to_img:
    mov   ebx,icons_file_name
    call  prepare_load_data
    jnz   icon_error
    call  prepare_load_data_2
    add   eax,mem
    call  prepare_load_data_1
    jnz   icon_error
    call  convert_bmp_to_img
    call  sub_application_memory
    ret
;---------------------------------------------------------------------
load_buttons_and_convert_to_img:
    mov   ebx,buttons_file_name
    call  prepare_load_data
    jnz   buttons_error
    mov   eax,[appl_memory]
    mov   [buttons_img_start],eax
    call  prepare_load_data_2
    add   eax,[buttons_img_start]
    call  prepare_load_data_1
    jnz   buttons_error
    call  convert_bmp_to_img
    call  sub_application_memory
    ret
;---------------------------------------------------------------------
load_initiation_file:
    mov   ebx,ini_file_name
    call  prepare_load_data
    jnz   initiation_error
    call  prepare_load_data_3
    mov   eax,[appl_memory]
    mov   [left_folder_data],eax
    sub   eax,[read_file.size]
    mov   [read_file.return],eax
    mov   [ini_file_start],eax
    call  load_file
    test  eax,eax
    jnz   initiation_error
    mov   ebp,icons_associations
    call  search_star_and_end_tags
;    cmp   ebp,-1
;    je    .end
    mov   eax,[end_tag]
    mov   [icons_end_tag],eax
    ret
;---------------------------------------------------------------------
add_memory_for_folders:
    mov   ecx,[appl_memory]
    add   ecx,304*32+32
    mov   [right_folder_data],ecx
    add   ecx,304*32+32
    mov   [appl_memory],ecx
    mcall 64,1
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
;    dec   esi
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
add_application_memory:
    mov   ecx,[file_features_temp_area+32]
.1:
    add   ecx,[appl_memory]
    mov   [appl_memory],ecx
    mcall 64,1
    ret
;---------------------------------------------------------------------
sub_application_memory:
    mov   ecx,[appl_memory]
    sub   ecx,[file_features_temp_area+32]
.1:
    mov   [appl_memory],ecx
    mcall 64,1
    ret
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
include   'confirm.inc'
;---------------------------------------------------------------------
include   'err_wind.inc'
;---------------------------------------------------------------------
include   'detect.inc'
;---------------------------------------------------------------------
include   'conv_bmp.inc'
;---------------------------------------------------------------------
include   'tran_ini.inc'
;---------------------------------------------------------------------
include   'help.inc'
;---------------------------------------------------------------------
include   'convchar.inc'
;---------------------------------------------------------------------
include   'sort.inc'
;---------------------------------------------------------------------
include   'exit.inc'
;---------------------------------------------------------------------
include   'progrbar.inc'
;---------------------------------------------------------------------
include   'scroll.inc'
;---------------------------------------------------------------------
include   'file_inf.inc'
;---------------------------------------------------------------------
include   'text.inc'
;---------------------------------------------------------------------
I_END:
;---------------------------------------------------------------------
include   'data.inc'
;---------------------------------------------------------------------
mem:
