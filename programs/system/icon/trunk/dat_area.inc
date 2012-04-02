;------------------------------------------------------------------------------
align 4
warea:
 .by_x:
  .right	rw 1
  .left		rw 1
 .by_y:
  .bottom	rw 1
  .top		rw 1

icons	rd 1
addr	rd 1

cur_str		rd 1
cur_band	rd 1

icon_count	rd 1
strip_file	rd 1

icon_position:	rd 1	; [ebp-4] 'AA-F' or... 
adress_of_icon_data:	rd 1	; [ebp+8]
adress_of_icon_data_2:	rd 1	; [ebp+8]
current_X:	rd 1
current_Y:	rd 1
current_X_mouse_thread:	rd 1
current_Y_mouse_thread:	rd 1

draw_area:	rd 1

x_left:		rd 1
x_right:	rd 1
y_top:		rd 1
y_bottom:	rd 1
;------------------------------------------------------------------------------
curx1		rd 1
cury1		rd 1
curx2		rd 1
cury2		rd 1
  
click_pos	rd 1
;------------------------------------------------------------------------------
align 4
redraw_all_icon:	rb 1
;------------------------------------------------------------------------------
align 4
param_str	rb 31
;------------------------------------------------------------------------------
align 4
procinfo process_information
;------------------------------------------------------------------------------	
align 4
path:
icon_data:
	rb 0x1000
;------------------------------------------------------------------------------
align 4
	rb 0x400
stack_mouse_thread:
;------------------------------------------------------------------------------
align 4
	rb 0x400
stack_area:
;------------------------------------------------------------------------------
align 4
I_Param:
	rb 0x100	; max 256 
;------------------------------------------------------------------------------
align 4
library_path:
	rb 0x1000
;------------------------------------------------------------------------------