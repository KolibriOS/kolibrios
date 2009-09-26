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
	SetWindowSizeRequest [window],220,142
	;set callback function for button close window
	SetCallbackFunction [window],DELETE_EVENT,callback_func_delete_window,NULL

	;create control Image
	mov [imgdata.x],5
	mov [imgdata.y],5
	mov [imgdata.wight],200
	mov [imgdata.height],100
	mov [imgdata.bits_per_pixel],32
	CreateImage imgdata
	mov [image],eax

	;copy data from pointer to local variable
	;for access to filds of structure
	mov ecx,[image]
	mov eax,[ecx+HEADER_SIZE+2] ;pointer to picture

	xor ecx,ecx
	for_size_y:
		xor edx,edx
		for_size_x:
			;generate color
			mov ebx,edx ;i
			mov esi,ecx ;j
			imul ebx,ebx ;i*i
			imul esi,esi ;j*j
			add ebx,esi  ;i*i+j*J
			mov esi,edx  ;i
			lea esi,[esi+2*esi] ;i*3
			sub ebx,esi  ;i*i+j*j-i*3
			mov esi,ecx  ;j
			shl esi,1    ;j*2
			add ebx,esi  ;i*i+j*j-i*3+2*j
			imul ebx,100 ;100*(i*i+j*j-i*3+2*j)

			mov [eax],ebx
			add eax,4
		inc edx
		cmp edx,200
		jl for_size_x
	inc ecx
	cmp ecx,100
	jl for_size_y

	;pack control Text in window
	PackControls [window],[image]

	;start libGUI main loop
	LibGUImain [window]


;void callback_func_delete_window(header_t *control,void *data)
callback_func_delete_window:

	mov eax,[esp+4] ;control

	QuitLibGUI eax

	ret
align 4
;----------------data-----------------
imgdata 	gui_image_data_t
window		rd 1;parent_t *window
image		rd 1

i_end: