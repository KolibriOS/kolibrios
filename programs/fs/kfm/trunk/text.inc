header_text       db 'Kolibri File Manager v0.48e',0
;---------------------------------------------------------------------
fbutton_name:
    .f1   db '1 Help',0
    .f2   db '2',0
    .f3   db '3 View',0
    .f4   db '4 Edit',0
    .f5   db '5 Copy',0
    .f6   db '6 Move',0
    .f7   db '7 Dir',0
    .f8   db '8 Del',0
    .f9   db '9 File',0
    .f10  db '10',0
    .f11  db '11',0
    .f12  db '12 Exit',0
;---------------------------------------------------------------------
fbutton_name_shift:
    .f1   db '1',0
    .f2   db '2',0
    .f3   db '3',0
    .f4   db '4',0
    .f5   db '5',0
    .f6   db '6 ren',0
    .f7   db '7',0
    .f8   db '8',0
    .f9   db '9',0
    .f10  db '10',0
    .f11  db '11',0
    .f12  db '12',0
;---------------------------------------------------------------------
fbutton_name_ctrl:
    .f1   db '1',0
    .f2   db '2',0
    .f3   db '3 Name',0
    .f4   db '4 Type',0
    .f5   db '5 Size',0
    .f6   db '6 Date',0
    .f7   db '7',0
    .f8   db '8',0
    .f9   db '9',0
    .f10  db '10',0
    .f11  db '11',0
    .f12  db '12',0
;---------------------------------------------------------------------
fbutton_name_alt:
    .f1   db '1 Left',0
    .f2   db '2 Right',0
    .f3   db '3',0
    .f4   db '4',0
    .f5   db '5',0
    .f6   db '6',0
    .f7   db '7',0
    .f8   db '8',0
    .f9   db '9',0
    .f10  db '10',0
    .f11  db '11',0
    .f12  db '12',0
;---------------------------------------------------------------------
select_disk_char:
     db '/',0
;---------------------------------------------------------------------
truncated_filename_char db '..'
;---------------------------------------------------------------------
sort_button_name:
    .name  db 'Name'
    .type  db 'Type'
    .size  db 'Size'
    .date  db 'Date'
;---------------------------------------------------------------------
type_table:
     db '<DIR> '
;---------------------------------------------------------------------
size_table:
     db '1023b '
;---------------------------------------------------------------------
date_table:
     db '  .  .     22:00 '
;---------------------------------------------------------------------
year_table:
     db '     '
;---------------------------------------------------------------------
ini_file_name:
     db 'kfm.ini',0
;---------------------------------------------------------------------
error_type:
     db 'File system error',0
;---------------------------------------------------------------------
delete_type:
     db 'Delete file(s)',0
;---------------------------------------------------------------------
copy_type:
     db '  Copy files',0
;---------------------------------------------------------------------
copy_type_1:
     db 'Copy file',0
;---------------------------------------------------------------------
move_type:
     db '  Move files',0
;---------------------------------------------------------------------
move_type_1:
     db 'Move file',0
;---------------------------------------------------------------------
rename_type:
     db '  Rename files',0
;---------------------------------------------------------------------
rename_type_1:
     db 'Rename file',0
;---------------------------------------------------------------------
create_dir:
     db 'Create directory',0
;---------------------------------------------------------------------
create_file:
     db 'Create file',0
;---------------------------------------------------------------------
exit_type:
     db '     Exit?',0
;---------------------------------------------------------------------
type_yes:
     db 'Yes',0
;---------------------------------------------------------------------
type_no:
     db 'No',0
;---------------------------------------------------------------------
type_location:
     db 'Location:',0
;---------------------------------------------------------------------
type_file_name:
     db 'File name:',0
;---------------------------------------------------------------------
type_dir_name:
     db 'Dir. name:',0
;---------------------------------------------------------------------
type_files:
     db 'Files:',0
;---------------------------------------------------------------------
type_dir:
     db 'Directories:',0
;---------------------------------------------------------------------
type_size:
     db 'Size:',0
;---------------------------------------------------------------------
type_create:
     db 'Create date:',0
;---------------------------------------------------------------------
type_change:
     db 'Change date:',0
;---------------------------------------------------------------------
type_access:
     db 'Access date:',0
;---------------------------------------------------------------------
type_limiters_data_time:
     db '/  /       :  :',0
;---------------------------------------------------------------------
mouse_left_panel:
     db 'Left ',0
mouse_right_panel:
     db 'Right',0
mouse_other_panel:
     db 'Other',0
;---------------------------------------------------------------------
type_of:
     db 'of',0
;---------------------------------------------------------------------
help_file_name:
     db 'kfm_keys.txt',0
;---------------------------------------------------------------------
;initiation file tags
start_line:
    db 13,10
files_associations:
    db '<>files_associations',13,10
files_associations_1:
    db '<>files_association1',13,10
icons_associations:
    db '<>icons_associations',13,10
end_section_tag:
    db '<>end',13,10
;---------------------------------------------------------------------
;open file parameters
background_tile:
    db '\T  '
background_stretch:
    db '\S  '
;---------------------------------------------------------------------
INF_WINW = 400
INF_WINH = 200
INF_DATAX = 110
INF_DATAY = 10
INF_LINEH = 18
files_information_window_x: dd 100 shl 16 + INF_WINW
files_information_window_y: dd 100 shl 16 + INF_WINH
infowin_title db 'Properties',0
;---------------------------------------------------------------------
ERR_WINW = 300
ERR_WINH = 80
error_window_x: dd 100 shl 16 + ERR_WINW
error_window_y: dd 100 shl 16 + ERR_WINH
;---------------------------------------------------------------------
mouse_key_delay dd 100
;---------------------------------------------------------------------
copy_editboxes:
create_editboxes:
edit1 edit_box 150,10,10,0xffffff,0xbbddff,0,0,0,255,create_dir_name,ed_focus+ed_always_focus,0
copy_editboxes_end:
create_editboxes_end:
;---------------------------------------------------------------------
;copy_editboxes:
;edit2 edit_box 150,10,10,0xffffff,0,0,0,0,255,create_dir_name,ed_focus,0
;copy_editboxes_end:
;---------------------------------------------------------------------
align 4
sysopen:
        dd  7,0,start_parameter,0,0
        db  '/sys/@open',0
;---------------------------------------------------------------------
buttons_file_data file 'images/buttons.raw'
icons_file_data	file 'images/icons.raw'
;---------------------------------------------------------------------