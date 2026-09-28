struct FileBrowser
	type				dd ? ;+0
	union
		x dd ?
		struct
		size_x 			dw ? ;+4
		start_x			dw ? ;+6
		ends
	ends
	union
		y dd ?
		struct
		size_y 			dw ? ;+8
		start_y			dw ? ;+10
		ends
	ends
	icon_size_y			dw ? ; +12
	icon_size_x			dw ? ; +14
	line_size_x			dw ? ; +16
	line_size_y			dw ? ; +18
	type_size_x			dw ? ; +20
	size_size_x			dw ? ; +22
	date_size_x			dw ? ; +24
	attributes_size_x		dw ? ; +26
	icon_assoc_area		dd ? ; +28
	icon_raw_area			dd ? ; +32
	resolution_raw 		dd ? ; +36
	palette_raw			dd ? ; +40
	directory_path_area		dd ? ; +44
	file_name_area 		dd ? ; +48
	select_flag			dd ? ; +52
	background_color		dd ? ; +56
	select_color			dd ? ; +60
	seclect_text_color		dd ? ; +64
	text_color			dd ? ; +68
	reduct_text_color		dd ? ; +72
	marked_text_color		dd ? ; +76
	max_panel_line 		dd ? ; +80
	select_panel_counter		dd ? ; +84
	folder_block			dd ? ; +88
	start_draw_line		dd ? ; +92
	start_draw_cursor_line 	dw ? ; +96 ; pixels
	folder_data			dd ? ; +98
	temp_counter			dd ? ; +102
	file_name_length		dd ? ; +106
	marked_file			dd ? ; +110
	extension_size 		dd ? ; +114
	extension_start		dd ? ; +118
	type_table			dd ? ; +122
	ini_file_start 		dd ? ; +126
	ini_file_end			dd ? ; +130
	draw_scroll_bar		dd ? ; +134
	font_size_y			dw ? ; +138
	font_size_x			dw ? ; +140
	mouse_keys			dd ? ; +142
	mouse_keys_old 		dd ? ; +146
	mouse_pos			dd ? ; +150
	mouse_keys_delta		dd ? ; +154
	mouse_key_delay		dd ? ; +158
	mouse_keys_tick		dd ? ; +162
	start_draw_cursor_line_2	dw ? ;+166
	all_redraw			dd ? ;+168
	selected_BDVK_adress		dd ? ;+172
	key_action			dw ? ;+176
	key_action_num 		dw ? ;+178
	name_temp_area 		dd ? ;+180
	max_name_temp_size		dd ? ;+184
	display_name_max_length	dd ? ;+188
	draw_panel_selection_flag	dd ? ;+192
	mouse_pos_old			dd ? ;+196
	marked_counter 		dd ? ;+200
	keymap_pointer 		dd ?
ends