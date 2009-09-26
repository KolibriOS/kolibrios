;Hello world example

use32
	db	'MENUET01'
	dd	1
	dd	start
	dd	i_end
	dd	0x800
	dd	0x800
	dd	0
	dd	0

TRUE	=     1
FALSE	=     0

include 'libGUI.inc'

start:
	;load libGUI labrari
	push NULL  ;use default system path to library
	call LoadLibGUI
	;create main window
	CreateWindow
	mov [window],eax
	;change size of main window
	SetWindowSizeRequest [window],92,46
	;set callback function for button close window
	SetCallbackFunction [window],DELETE_EVENT,callback_func_delete_window,NULL

	;create control Text
	mov [txtdata.x],5
	mov [txtdata.y],5
	mov [txtdata.font],NULL;use default system libGUI font
	mov [txtdata.background],TRUE;use background for text
	mov [txtdata.color],0xffffff;text color
	mov [txtdata.background_color],0xff8000
	mov [txtdata.text],txt
	CreateText txtdata
	mov [text],eax

	;pack control Text in window
	PackControls [window],[text]

	;start libGUI main loop
	LibGUImain [window]


;void callback_func_delete_window(header_t *control,void *data)
callback_func_delete_window:

	mov eax,[esp+4] ;control

	QuitLibGUI eax

	ret
align 4
;----------------data-----------------
txt		db 'Hello world!',0
txtdata 	gui_text_data_t
window		rd 1;parent_t *window
text		rd 1;gui_text_data_t *text

i_end: