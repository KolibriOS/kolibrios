

struct BOX
  left	 dd ?
  top	 dd ?
  width  dd ?
  height dd ?
ends

struct process_information
  cpu_usage		  dd ?	; +0
  window_stack_position   dw ?	; +4
  window_stack_value	  dw ?	; +6
			  dw ?	; +8
  process_name		  TCHAR 12 dup(?)    ; +10
  memory_start		  dd ?	; +22
  used_memory		  dd ?	; +26
  PID			  dd ?	; +30
  box			  BOX	   ; +34
  slot_state		  dw ?	; +50
			  dw ?	; +52
  client_box		  BOX	   ; +54
  wnd_state		  db ?	 ; +70
  align 1024
ends

struct system_colors
  frame 	   dd ? ;nonset1
  grab		   dd ? ;nonset2
  work_3d_dark	   dd ?
  work_3d_light    dd ?
  grab_text	   dd ? ;window_title
  work		   dd ?
  work_button	   dd ?
  work_button_text dd ?
  work_text	   dd ?
  work_graph	   dd ?
ends

struct FILEIO
	cmd		dd ?
	offset		dd ?
	union
		flags	dd ?
		offshigh dd ?
	ends
	count		dd ?
	buffer		dd ?
ends
label FILEIO.filesize dword at -4

struct namedFILEIO FILEIO
	name	db MAX_PATH dup (?)
ends

struct fullFILEIO FILEIO
	name	db ?
	lpname	dd ?
ends

struct FILEDATE
  Second db ?
  Minute db ?
  Hour	 db ?
	 db ?
  Day	 db ?
  Month  db ?
  Year	 dw ?
ends

struct FILEINFO
  Attributes dd ?
  IsUnicode  db ?
	     db 3 dup(?)
  DateCreate FILEDATE
  DateAccess FILEDATE
  DateModify FILEDATE
  size	     dq ?
ends

