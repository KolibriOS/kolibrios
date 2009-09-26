;Hello world example

use32
	db	'MENUET01'
	dd	1
	dd	start
	dd	i_end
	dd	0x1500
	dd	0x1500
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
	SetWindowSizeRequest [window],90,60

	;set callback function for button close window
	SetCallbackFunction [window],DELETE_EVENT,callback_func_delete_window,NULL

	;create control Button with text
	mov [bdata.x],5
	mov [bdata.y],5
	mov [bdata.wight],70
	mov [bdata.height],20
	CreateButtonWithText bdata,btext
	mov [button],eax

	;set callback functions for button
	SetCallbackFunction [button],BUTTON_ENTER_EVENT,callback_func1,NULL
	SetCallbackFunction [button],BUTTON_PRESSED_EVENT,callback_func2,NULL
	SetCallbackFunction [button],BUTTON_RELEASED_EVENT,callback_func3,NULL
	SetCallbackFunction [button],BUTTON_LEAVE_EVENT,callback_func4,NULL

	;pack control Text in window
	PackControls [window],[button]

	;start libGUI main loop
	LibGUImain [window]


;void callback_func_delete_window(header_t *control,void *data)
callback_func_delete_window:

	mov eax,[esp+4] ;control

	QuitLibGUI eax

	ret

;void callback_func1(header_t *control,void *data)
callback_func1:
	;save esi befor use
	push esi
	mov esi,btext1
	call gui_ksys_debug_out_str
	pop esi

	ret

;void callback_func2(header_t *control,void *data)
callback_func2:
	;save esi befor use
	push esi
	mov esi,btext2
	call gui_ksys_debug_out_str
	pop esi

	ret

;void callback_func3(header_t *control,void *data)
callback_func3:
	;save esi befor use
	push esi
	mov esi,btext3
	call gui_ksys_debug_out_str
	pop esi

	ret

;void callback_func4(header_t *control,void *data)
callback_func4:
	;save esi befor use
	push esi
	mov esi,btext4
	call gui_ksys_debug_out_str
	pop esi

	ret

align 4
;----------------------data--------------------
btext		db 'Click Me!',0
btext1		db 13,10,'entry in button',0
btext2		db 13,10,'button pressed',0
btext3		db 13,10,'button released',0
btext4		db 13,10,'leave button',0

bdata		gui_button_data_t
window		rd 1;parent_t *window
button		rd 1;gui_button_t *button

i_end: