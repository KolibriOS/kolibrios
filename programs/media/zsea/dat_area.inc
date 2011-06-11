string:
;	rb	string+257-$
	rb  4096
;string_1:
;	rb  4096
;----------------------
path:
	rb  4096
;----------------------
;plugin_pach_1:
;	rb  4096
;----------------------
plugin_pach:
	rb  4096
;----------------------
library_path:
	rb 4096
;----------------------
Filter:
	rb 4096
;----------------------
Help_path:
	rb 1024 ;4096
;----------------------
Editor_path:
	rb 1024 ;4096
;----------------------
OpenDialog_path:
	rb 1024 ;4096
;----------------------
key_help_name:
	rb 128
;----------------------
plugin_0_name:
	rb 128
plugin_1_name:
	rb 128
plugin_2_name:
	rb 128
plugin_3_name:
	rb 128
plugin_4_name:
	rb 128
plugin_5_name:
	rb 128
plugin_6_name:
	rb 128
plugin_7_name:
	rb 128
plugin_8_name:
	rb 128
plugin_9_name:
	rb 128
;----------------------
file_name:
	rb 1024
;----------------------
temp_area:
	rb  4096
;----------------------
procinfo:
process_info:
	rb 1024
;----------------------
procinfo_threads:
	rb 1024
;----------------------
; clear bacground thread
;	rb 512
;clear_thread_stack:
;----------------------
; window option
;	rb 1024 ;4096
;thread_stack_7:
;----------------------
; window error message
;	rb 1024 ;4096
;thread_stack_6:
;----------------------
; window set sort mode
;	rb 1024 ;4096
;thread_stack_5:
;----------------------
; window background
;	rb 1024 ;4096
;thread_stack_4:
;----------------------
; window about
;	rb 1024 ;4096
;thread_stack_3:
;----------------------
; file info
;	rb 1024 ;4096
;thread_stack_2:
;----------------------
; file open
;	rb 1024 ;4096
;thread_stack:
;----------------------
	rb 4096
stacktop:
;----------------------