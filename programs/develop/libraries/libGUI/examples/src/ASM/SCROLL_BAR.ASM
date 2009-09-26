;Hello world example

use32
	db	'MENUET01'
	dd	1
	dd	start
	dd	i_end
	dd	0x1000
	dd	0x1000
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
	SetWindowSizeRequest [window],270,207
	;set callback function for button close window
	SetCallbackFunction [window],DELETE_EVENT,callback_func_delete_window,NULL

	;create control HorizontalScrollBar
	mov [hsbdata.x],5
	mov [hsbdata.y],5
	mov [hsbdata.wight],250
	mov [hsbdata.height],16
	mov [hsbdata.ruller_size],dword 0.2 ;ruler size 0..1
	mov [hsbdata.ruller_pos],dword 0.5  ;ruler position 0..1
	mov [hsbdata.ruller_step],dword 0.1 ;ruler step 0..1
	CreateHorizontalScrollBar hsbdata
	mov [hscroll_bar],eax

	;create control HorizontalScrollBar
	mov [vsbdata.x],5
	mov [vsbdata.y],26
	mov [vsbdata.wight],16
	mov [vsbdata.height],150
	mov [vsbdata.ruller_size],dword 0.5 ;ruler size 0..1
	mov [vsbdata.ruller_pos],dword 0.05 ;ruler position 0..1
	mov [vsbdata.ruller_step],dword 0.1 ;ruler step 0..1
	CreateVerticalScrollBar vsbdata
	mov [vscroll_bar],eax

	;set callback functions for scroll bars
	SetCallbackFunction [hscroll_bar],SCROLLBAR_CHANGED_EVENT,hscroll_bar_callback,NULL
	SetCallbackFunction [vscroll_bar],SCROLLBAR_CHANGED_EVENT,vscroll_bar_callback,NULL

	;pack controls in window
	PackControls [window],[hscroll_bar]
	PackControls [window],[vscroll_bar]

	;start libGUI main loop
	LibGUImain [window]


;void hscroll_bar_callback(header_ *control,void *data)
hscroll_bar_callback:

	mov eax,[esp+4];pointer to control scroll bar

	push ebx
	mov ebx,[eax+HEADER_SIZE];size of ruler
	mov ecx,[eax+HEADER_SIZE+4];position of ruler
	mov edx,[eax+HEADER_SIZE+8];step change position of ruler by press
				   ;buttons of scroll bar
	pop ebx

	ret

;void hscroll_bar_callback(header_ *control,void *data)
vscroll_bar_callback:

	mov eax,[esp+4];pointer to control scroll bar

	push ebx ;save befor use

	mov ebx,[eax+HEADER_SIZE];size of ruler
	mov ecx,[eax+HEADER_SIZE+4];position of ruler
	mov edx,[eax+HEADER_SIZE+8];step change position of ruler by press
				   ;buttons of scroll bar
	pop ebx
	ret

;void callback_func_delete_window(header_t *control,void *data)
callback_func_delete_window:

	mov eax,[esp+4] ;control

	QuitLibGUI eax

	ret

align 4
;----------------data-----------------
hsbdata 	gui_scroll_bar_data_t
vsbdata 	gui_scroll_bar_data_t

window		rd 1
hscroll_bar	rd 1
vscroll_bar	rd 1

i_end: