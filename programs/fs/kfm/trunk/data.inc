;---------------------------------------------------------------------
files_information_window_x: dd 100 shl 16 + 250
files_information_window_y: dd 100 shl 16 + 300
;---------------------------------------------------------------------
error_window_x: dd 100 shl 16 + 250
error_window_y: dd 100 shl 16 + 80
;---------------------------------------------------------------------
mouse_key_delay dd 100
;---------------------------------------------------------------------
files_counter dd 0
directories_counter dd 0
files_size:
     .low dd 0
     .high dd 0
;---------------------------------------------------------------------
open_param dd 0
;---------------------------------------------------------------------
PID  dd 0
active_process dd 0
;---------------------------------------------------------------------
window_high	dd 0
window_width	     dd 0
window_status dd 0
skin_high	     dd 0
fbutton_width	     dd 0
drawing_panel_folder db 0 ; 1-left, 2 right
left_panel_x:
    .size_x		 dw 0
    .start_x	     dw 0
left_panel_y:
    .size_y		 dw 0
    .start_y	     dw 0
right_panel_x:
    .size_x		 dw 0
    .start_x	     dw 0
right_panel_y:
    .size_y		 dw 0
    .start_y	     dw 0
;---------------------------------------------------------------------
mouse_position:
    .y		 dw 0
    .x	     dw 0
mouse_keys dd 0
mouse_keys_delta db 0
mouse_keys_delta_1 db 0
scroll_bar_event db 0
scroll_pointer db 0
scroll_pointer_1 db 0
mouse_keys_tick dd 0
mouse_scroll_data:
    .vertical   dw 0
    .horizontal dw 0
left_scroll_compens dd 0
right_scroll_compens dd 0
;---------------------------------------------------------------------
device_present:
    .rd     db 0
    .fd     db 0
    .hd0	db 0
    .hd1	db 0
    .hd2	db 0
    .hd3	db 0
    .cd0	db 0
    .cd1	db 0
    .cd2	db 0
    .cd3	db 0
;device_present_text_counter dd 0
device_present_buffer:
    .fd       db 0
    .hd_cd	  db 0
    .hd0_part db 0
    .hd1_part db 0
    .hd2_part db 0
    .hd3_part db 0
    rb 4
;---------------------------------------------------------------------
copy_many_files:
     db 0
max_progress_value dd 0
current_progress_value dd 0
current_progress_value_1 dd 0
temp_progress_value:
     dd 0
draw_all_progressbar db 0
amount_output_numbers dd 0
;---------------------------------------------------------------------
max_panel_line dd 0
temp_panel_end dd 0
;---------------------------------------------------------------------
left_start_draw_line dd 0
left_folder_block dd 0
left_start_draw_cursor_line dw 0
;left_panel_clear_all db 0
;---------------------------------------------------------------------
right_start_draw_line dd 0
right_folder_block dd 0
right_start_draw_cursor_line dw 0
;right_panel_clear_all db 0
;---------------------------------------------------------------------
sort_counter dd 0
sorting_low_limit dd 0
sorting_high_limit dd 0
dir_or_file  db  0
dir_temp_counter dd 0
dir_temp_counter_1 dd 0
draw_scroll_character db 0
nesting_direction db 0
;---------------------------------------------------------------------
left_marked_counter dd 0
right_marked_counter dd 0
marked_file db 0
marked_action db 0
mark_action_counter db 0
work_confirmation db 0
work_confirmation_yes db 0
delete_marked_yes db 0
inf_marked_yes db 0
confirmation_type dd 0
move_file_flag db 0
recursive_pointer dd 0
;---------------------------------------------------------------------
search_pre_folder db 0
not_clear_lines db 0
select_panel_counter db 0
extended_key db 0
start_draw_menu_cursor_line dd 0
press_enter_menu db  0
close_menu db 0
extension_start dd 0
extension_size dd 0
file_name_length dd 0
temp_counter_dword dd 0
error_pointer dd 0
file_system_error dd 0
exit_to_dir db 0
exit_dir db 0
end_tag dd 0
icons_end_tag dd 0
use_patch db 0
;---------------------------------------------------------------------
appl_memory dd 0
appl_memory_old dd 0
buttons_img_start dd 0
ini_file_start dd 0
left_folder_data dd 0
right_folder_data dd 0
;---------------------------------------------------------------------
shift_flag db 0
ctrl_flag  db 0
alt_flag   db 0
;---------------------------------------------------------------------
left_sort_flag dd 0
right_sort_flag dd 0
;---------------------------------------------------------------------
retrieved_devices_table_counter dd 0
temp_counter db 0
temp_counter_1 dd 0
;---------------------------------------------------------------------
; for EDITBOX
mouse_flag: dd 0x0
;---------------------------------------------------------------------
create_dir_name:
    db 0
    rb 256
;---------------------------------------------------------------------
retrieved_devices_table:
    rb 10*100
;---------------------------------------------------------------------
procinfo:
    rb 1024
;---------------------------------------------------------------------
type_name:
	rb 512
;---------------------------------------------------------------------
type_size_string:
	rb 32
;---------------------------------------------------------------------
type_files_string:
	rb 16
;---------------------------------------------------------------------
type_directories_string:
	rb 16
;---------------------------------------------------------------------
delete_dir_data:
    rb 32
;---------------------------------------------------------------------
;stack
	align 4
	rb 512
stacktop:
;---------------------------------------------------------------------
	align 4
	rb 4096
thread_stack:
;---------------------------------------------------------------------
file_features_temp_area:
    rb 40
;---------------------------------------------------------------------
initial_data:
    rb 4096
;---------------------------------------------------------------------
path:
    rb 4096
;---------------------------------------------------------------------
file_name:
    rb 4096
;---------------------------------------------------------------------
start_parameter:
    rb 4096
;---------------------------------------------------------------------
read_folder_name:
    rb 4096
;---------------------------------------------------------------------
read_folder_1_name:
    rb 4096
;---------------------------------------------------------------------
; for EDITBOX
;structure_of_potock:
;    rb 1024
;---------------------------------------------------------------------