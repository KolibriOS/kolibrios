;Hello world example

use32
	db	'MENUET01'
	dd	1
	dd	start
	dd	i_end
	dd	0x10000
	dd	0x10000
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
	SetWindowSizeRequest [window],270,282
	;set callback function for button close window
	SetCallbackFunction [window],DELETE_EVENT,callback_func_delete_window,NULL

	;create control HorizontalScrollBar
	mov [swdata.x],5
	mov [swdata.y],5
	mov [swdata.wight],250
	mov [swdata.height],250
	CreateScrolledWindow swdata
	mov [scrolled_window],eax

	;create buttons
	mov ecx,10
	next_j:
		mov edx,10
		next_i:
			mov esi,edx ;i
			mov edi,ecx ;j
			dec esi     ;i-1
			dec edi     ;j-1
			imul esi,75 ;(i-1)*75
			imul edi,25 ;(j-1)*25
			add esi,10  ;10+(i-1)*75
			add edi,10  ;10+(j-1)*25

			mov [bdata.x],esi
			mov [bdata.y],edi
			mov [bdata.wight],70
			mov [bdata.height],20

			push ecx edx esi edi;save registers befor call

			CreateButtonWithText bdata,btxt
			mov [button],eax

			;set callback functions for scroll bars
			SetCallbackFunction [button],BUTTON_PRESSED_EVENT,callback_func,NULL
			;pack control Button in ScrolledWindow
			ScrolledWindowPackControls [scrolled_window],[button]

			pop edi esi edx ecx
		dec edx
		jnz next_i
	dec ecx
	jnz next_j

	;pack scrolled window in window
	PackControls [window],[scrolled_window]

	;start libGUI main loop
	LibGUImain [window]


;void callback_func(header_t *control,void *data)
callback_func:

	mov eax,[esp+4];pointer to control

	push esi;save befor use
	mov esi,message
	call gui_ksys_debug_out_str
	pop esi

	ret

;void callback_func_delete_window(header_t *control,void *data)
callback_func_delete_window:

	mov eax,[esp+4] ;control

	QuitLibGUI eax

	ret
align 4
;----------------data-----------------
btxt		db 'button',0
message 	db 13,10,'pressed button',0
bdata		gui_button_data_t
swdata		gui_scrolled_window_data_t

window		rd 1
scrolled_window rd 1
button		rd 1

i_end: