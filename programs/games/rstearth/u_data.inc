;---------------------------------------------------------------------
; uninitialized data
;---------------------------------------------------------------------
extended_key		rb 1
draw_all_level		rb 1
current_cell_NPC	rb 1
death_of_protagonist	rb 1
target_detected		rb 1
sound_flag		rb 1
snd_kick_flag		rb 1
;---------------------------------------------------------------------
align 4
N_error		rd 1
error_type	rd 1
error_path	rd 1
;---------------------------------------------------------------------
base_icons		rd 1
red_brick_icons		rd 1
white_brick_icons	rd 1
npc_miku_icons		rd 1
npc_death_icons		rd 1
npc_skeleton_icons	rd 1
npc_ifrit_icons		rd 1
npc_barret_icons	rd 1
;---------------------------------------------------------------------
background_music	rd 1
stone_kick_sound	rd 1
;---------------------------------------------------------------------
show_tiles_iterations	rd 1
characters_offset_x	rd 1
characters_offset_y	rd 1
characters_size_x	rd 1
characters_size_y	rd 1
buffer_offset_x		rd 1
buffer_offset_y		rd 1
;---------------------------------------------------------------------
action_cell_x		rd 1
action_cell_y		rd 1
target_action_cell_x	rd 1
target_action_cell_y	rd 1
;------------------------------------------------------------------------------
align 4 
version		rd 1	; версия интерфейса, текущая 0x0101
; snd_format одна из констант вида PCM_X_Y_Z где
; X- число каналов
; Y- разрядность в битах на канал
; Z- частота в КГц из фикс. набора 11025Гц, 12000Гц, 16000Гц, 22050Гц, 24000Гц, 32000Гц, 44100Гц, 48000Гц
; Если формат не известен заранее используется константа PCM_ALL.
; Окончательно формат устанавливается вызовом SetFormat.
snd_format	rd 1	; формат данных 
; хэндл буфера
;hBuff		rd 1
hBuff1		rd 1
hBuff2		rd 1

data_offset	rd 1
;---------------------------------------------------------------------
voice_samples_start	rd 1	; start of *.SMP
wav_for_test		rd 1
wav_for_test_end	rd 1
;---------------------------------------------------------------------
align 4
buffer_area:
	rb 64*64*4 ; 64p * 64p * 32bpp
;---------------------------------------------------------------------
align 4
plan_level:
	rb LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4
;---------------------------------------------------------------------
align 4
objects_id:
	rb LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4
;---------------------------------------------------------------------
align 4
plan_level_old:
	rb LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4
;---------------------------------------------------------------------
align 4
objects_id_old:
	rb LEVEL_MAP_SIZE_X*LEVEL_MAP_SIZE_Y*4
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
file_info:
	rb 40
;---------------------------------------------------------------------
;align 4
;procinfo:
;process_info:
;	rb 1024
;---------------------------------------------------------------------
align 4
; sound thread
	rb 1024
snd_kick_action_thread_stack:
;---------------------------------------------------------------------
align 4
; sound thread
	rb 1024
snd_background_music_thread_stack:
;---------------------------------------------------------------------
align 4
; window error message
	rb 1024
thread_stack:
;---------------------------------------------------------------------
align 4
	rb 1024
stacktop:
;---------------------------------------------------------------------