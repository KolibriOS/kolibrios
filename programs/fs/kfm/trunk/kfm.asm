;*****************************************************************************
; KFM - Kolibri File Manager
; Copyright (c) 2006 - 2014, Marat Zakiyanov aka Mario79, aka Mario
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
; KFM v0.48d 23/12/2021
;---------------------------------------------------------------------
use32
org     0x0

    db    'MENUET01'
    dd    0x01
    dd    START
    dd    I_END
    dd    mem
    dd    stacktop
    dd    0x0
    dd    path

;include   'lang.inc'
;include   'kglobals.inc'
;include   'macros.inc'
include '../../../macros.inc'
include '../../../config.inc'           ;for nightbuild
;include '../../../debug.inc'           ;for nightbuild

;define __DEBUG__ 1
;define __DEBUG_LEVEL__ 1
;include '../../../debug-fdo.inc'

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
        mcall   9,procinfo,-1
        mov     ecx,[ebx+30]    ; PID
        mcall   18,21
        mov     [active_process],eax    ; WINDOW SLOT
    mov   [appl_memory],mem
    mov   ax,[select_disk_char]
    mov   [read_folder_name],ax
    mov   [read_folder_1_name],ax
    call  load_initiation_file
    call  add_memory_for_folders
    call  device_detect_f70
    call  select_starting_directories
    mcall 66, 1, 1
    mov   eax,1
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
    ja    @f
    mov   esi,180
    mcall 67,-1,ebx,ebx
@@:
    cmp   [window_width],495
    ja    red_1
    mov   edx,495
    mcall 67,-1,ebx, ,ebx
red_1:
    call  draw_window
;---------------------------------------------------------------------
still:
    mcall 10

    call  check_active_process_for_clear_all_flags

    cmp   eax,1
    je    red
    cmp   eax,2
    je    key
    cmp   eax,3
    je    button
    cmp   eax,6
    je    mouse
    jmp   still
;---------------------------------------------------------------------
check_active_process_for_clear_all_flags:
        push    eax
        mcall   18,7
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
        xor     esi,esi
    mcall 0, <20,728>, <20,460>, 0x43cccccc   ; 0x805080D0, 0x005080D0
    call  get_window_param

    mcall 71, 1, header_text

        test    [window_status],100b    ; window is rolled up
        jnz     .exit

        test    [window_status],10b     ; window is minimized to panel
        jnz     .exit

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
    mcall 12, 2
    ret
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

    mov   eax,[left_folder_data]
    mov   [read_folder.return],eax 
    mov   eax,[right_folder_data]
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
exit_apl:
    mov  [confirmation_type],exit_type
    call confirmation_action
    cmp  [work_confirmation_yes],1
    jne  red
    mcall -1
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
;include_debug_strings
I_END:
;---------------------------------------------------------------------
include   'data.inc'
;---------------------------------------------------------------------
mem:
