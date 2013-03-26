;---------------------------------------------------------------------
align 4
palette_area	rd 1
tone_area	rd 1
part_of_size_y	rd 1
active_process	rd 1
;color_dialog_type	rd 1
communication_area	rd 1
;--------------------------------------
offset_y:
.red	rd 1
.green	rd 1
.blue	rd 1
;--------------------------------------
offset_x:
.red	rd 1
.green	rd 1
.blue	rd 1
;--------------------------------------
tone_color:
.blue	rb 1
.green	rb 1
.red	rb 1
.align	rb 1
;--------------------------------------
selected_color:
.blue	rb 1
.green	rb 1
.red	rb 1
.align	rb 1
;--------------------------------------
palette_SIZE_Y	rd 1
palette_SIZE_X	rd 1

tone_SIZE_Y	rd 1
tone_SIZE_X	rd 1
;---------------------------------------------------------------------
align 4
procinfo:
	rb 1024
;---------------------------------------------------------------------
align 4
param:
	rb 256
;---------------------------------------------------------------------
align 4
	rb 4096
stacktop:
;---------------------------------------------------------------------