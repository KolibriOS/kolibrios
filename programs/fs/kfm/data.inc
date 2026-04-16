;---------------------------------------------------------------------
align 4
files_counter	rd 1
directories_counter	rd 1

files_size:
	.low	rd 1
	.high	rd 1
;Формат времени:
;  * +0: byte: секунды
;  * +1: byte: минуты
;  * +2: byte: часы
;  * +3: byte: зарезервировано (0)
;  * например, 23.59.59 записывается как (в hex) 3B 3B 17 00
time:
.create:
	rd 1
.access:
	rd 1
.change:
	rd 1
;Формат даты:
;  * +0: byte: день
;  * +1: byte: месяц
;  * +2: word: год
;  * например, 25.11.1979 записывается как (в hex) 19 0B BB 07
date:
.create:
	rd 1
.access:
	rd 1
.change:
	rd 1
;---------------------------------------------------------------------
open_param rd 1
;---------------------------------------------------------------------
align 4
PID  rd 1
active_process rd 1
;---------------------------------------------------------------------
align 4
window_high rd 1
window_width	     rd 1
window_status rd 1
skin_high	     rd 1
fbutton_width	     rd 1

left_panel_x:
    .size_x		 rw 1
    .start_x	     rw 1

left_panel_y:
    .size_y		 rw 1
    .start_y	     rw 1

right_panel_x:
    .size_x		 rw 1
    .start_x	     rw 1

right_panel_y:
    .size_y		 rw 1
    .start_y	     rw 1
    
left_panel_y_correct_size_y	rw 1
right_panel_y_correct_size_y	rw 1

drawing_panel_folder rb 1 ; 1-left, 2 right
;---------------------------------------------------------------------
align 4
mouse_position:
    .y		 rw 1
    .x	     rw 1
mouse_keys rd 1
mouse_keys_tick rd 1
mouse_scroll_data:
    .vertical   rw 1
    .horizontal rw 1
left_scroll_compens rd 1
right_scroll_compens rd 1
mouse_keys_delta rb 1
mouse_keys_delta_1 rb 1
scroll_bar_event rb 1
scroll_pointer rb 1
scroll_pointer_1 rb 1
;---------------------------------------------------------------------
align 4
max_progress_value rd 1
current_progress_value rd 1
current_progress_value_1 rd 1
temp_progress_value:
     rd 1
amount_output_numbers rd 1
draw_all_progressbar rb 1
copy_many_files:
     rb 1
;---------------------------------------------------------------------
align 4
max_panel_line rd 1
temp_panel_end rd 1
;---------------------------------------------------------------------
align 4
left_start_draw_line rd 1
left_folder_block rd 1
left_start_draw_cursor_line rw 1
left_folder_block_1 rd 1
;---------------------------------------------------------------------
align 4
right_start_draw_line rd 1
right_folder_block rd 1
right_start_draw_cursor_line rw 1
right_folder_block_1 rd 1
;---------------------------------------------------------------------
align 4
sort_counter rd 1
sorting_low_limit rd 1
sorting_high_limit rd 1
dir_temp_counter rd 1
dir_temp_counter_1 rd 1
draw_scroll_character rb 1
nesting_direction rb 1
dir_or_file  rb 1
;---------------------------------------------------------------------
align 4
left_marked_counter rd 1
right_marked_counter rd 1
confirmation_type rd 1
recursive_pointer rd 1
marked_file rb 1
marked_action rb 1
mark_action_counter rb 1
work_confirmation rb 1
work_confirmation_yes rb 1
delete_marked_yes rb 1
inf_marked_yes rb 1
move_file_flag rb 1
;---------------------------------------------------------------------
align 4
start_draw_menu_cursor_line rd 1
search_pre_folder rb 1
not_clear_lines rb 1
select_panel_counter rb 1
extended_key rb 1
press_enter_menu rb 1
close_menu rb 1
;---------------------------------------------------------------------
align 4
extension_start rd 1
extension_size rd 1
file_name_length rd 1
temp_counter_dword rd 1
error_pointer rd 1
file_system_error rd 1
end_tag rd 1
icons_end_tag rd 1
exit_to_dir rb 1
exit_dir rb 1
use_patch rb 1
;---------------------------------------------------------------------
align 4
appl_memory rd 1
appl_memory_old rd 1
buttons_img_start rd 1
ini_file_start rd 1
left_folder_data rd 1
right_folder_data rd 1
;---------------------------------------------------------------------
shift_flag rb 1
ctrl_flag  rb 1
alt_flag   rb 1
;---------------------------------------------------------------------
align 4
left_sort_flag rd 1
right_sort_flag rd 1
;---------------------------------------------------------------------
align 4
retrieved_devices_table_counter rd 1
temp_counter_1 rd 1
temp_counter rb 1
;---------------------------------------------------------------------
align 4
; for EDITBOX
mouse_flag: rd 1
;---------------------------------------------------------------------
align 4
create_dir_name:
	rb 1
	rb 256
;---------------------------------------------------------------------
align 4
left_folder_data_1:
	rb 304*32+32
;---------------------------------------------------------------------
align 4
right_folder_data_1:
	rb 304*32+32
;---------------------------------------------------------------------
align 4
retrieved_devices_table:
	rb 10*100
;---------------------------------------------------------------------
align 4
procinfo:
	rb 1024
;---------------------------------------------------------------------
align 4
type_name:
	rb 512
;---------------------------------------------------------------------
align 4
type_files_string:
	rb 16
;---------------------------------------------------------------------
align 4
type_directories_string:
	rb 16
;---------------------------------------------------------------------
align 4
delete_dir_data:
	rb 32
;---------------------------------------------------------------------
align 4
extension_temp_storage:
	rb 256
;---------------------------------------------------------------------
;stack
align 4
	rb 4096
stacktop:
;---------------------------------------------------------------------
align 4
	rb 4096
thread_stack:
;---------------------------------------------------------------------
align 4
file_features_temp_area:
	rb 40
;---------------------------------------------------------------------
align 4
initial_data:
	rb 4096
;---------------------------------------------------------------------
align 4
path:
	rb 4096
;---------------------------------------------------------------------
align 4
file_name:
	rb 4096
;---------------------------------------------------------------------
align 4
start_parameter:
	rb 4096
;---------------------------------------------------------------------
align 4
read_folder_name:
	rb 4096
;---------------------------------------------------------------------
align 4
read_folder_1_name:
	rb 4096
;---------------------------------------------------------------------
align 4
read_folder_2_name:
	rb 64
;---------------------------------------------------------------------
align 4
read_folder_3_name:
	rb 64
;---------------------------------------------------------------------
