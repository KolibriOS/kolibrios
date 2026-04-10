
IncludeUGlobals

read_info:
  .mode         dd ?            ; read
  .start_block  dd ?            ; first block
  .blocks       dd ?            ; 512 bytes
  .address      dd ?
  .workarea     dd ?
dtp_name rb 4096

virtual at read_info
 write_info:
  .mode         dd ?
  rd 1
  .bytes2write  dd ?
  .address      dd ?
  .workarea     dd ?
end virtual

read_info2:
  .mode         dd ?            ; read
  .start_block  dd ?            ; first block
  .blocks       dd ?            ; 512 bytes
  .address      dd ?
  .workarea     dd ?
skin_info rb 4096

app_param   rb      257

align 4
app_colours:

w_frame              dd ?
w_grab               dd ?
w_work_dark          dd ?
w_work_light         dd ?
w_grab_text          dd ?
w_work               dd ?
w_work_button        dd ?
w_work_button_text   dd ?
w_work_text          dd ?
w_work_graph         dd ?

color_table:
	times 10 dd ?
	
current_skin_high	rd 1
;---------------------------------------------------------------------
align 4
cur_dir_path:
	rb 4096
;---------------------------------------------------------------------
library_path:
	rb 4096
;---------------------------------------------------------------------
temp_dir_pach:
	rb 4096
;---------------------------------------------------------------------
temp_dir_pach2:
	rb 4096
;---------------------------------------------------------------------
text_work_area:
	rb 1024
;---------------------------------------------------------------------
text_work_area2:
	rb 1024
;---------------------------------------------------------------------
procinfo:
	rb 1024
;---------------------------------------------------------------------
filename_area:
	rb 256
;---------------------------------------------------------------------
filename_area2:
	rb 256
;---------------------------------------------------------------------
	rb 4096
stacktop:
;---------------------------------------------------------------------
file_load_area:	; old 0x10000
	rb 32*1024
;---------------------------------------------------------------------
not_packed_area:	; old 0x18000
	rb 32*1024
;---------------------------------------------------------------------
unpack_area:	; old 0x20000
	rb 32*1024
;---------------------------------------------------------------------