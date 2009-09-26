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
	SetWindowSizeRequest [window],320,57
	;set callback function for button close window
	SetCallbackFunction [window],DELETE_EVENT,callback_func_delete_window,NULL

	;create control Text
	mov [pbdata.x],5
	mov [pbdata.y],5
	mov [pbdata.wight],300;use default system libGUI font
	mov [pbdata.height],25;use background for text
	mov [pbdata.progress],dword 0.0
	CreateProgressBar pbdata
	mov [progress_bar],eax

	;create timer for update progress level each 50 milliseconds
	SetTimerCallbackForFunction [window],5,progress_bar_callback,[progress_bar]

	;pack control Text in window
	PackControls [window],[progress_bar]

	;redraw progress bar automatically each 50 milleseconds
	SetProgressBarPulse [progress_bar],5

	;start libGUI main loop
	LibGUImain [window]


;void progress_bar_callback(void *data)
progress_bar_callback:

	mov eax,[esp+4];pointer to control progress bar

	mov ecx,[eax+HEADER_SIZE];progress
	mov [progress],ecx
	mov [increment],dword 0.01
	mov [digit],100

	fld dword[increment]
	fld dword[progress]
	fadd st0,st1
	fst dword [progress]
	fild dword[digit]
	fmul st0,st1
	fistp dword[digit]
	fstp st0  ;free progress
	fstp st0  ;free increment

	cmp [progress],dword 1.0
	jl no_max_progress
		mov [progress],dword 0.0
	no_max_progress:

	mov ecx,[progress]
	mov [eax+HEADER_SIZE],ecx ;save new value of progress

	;print progressvalue in string
	push ebx ;save befor use

	mov eax,txt
	add eax,9
	mov ebx,eax ;save eax

	push eax
	push [digit]
	call dec_to_str

	add ebx,eax
	mov [ebx],byte '%'
	mov [ebx+1],byte 0

	pop ebx

	;set text for progress bar
	ProgressBarSetText [progress_bar],txt

	ret

;void callback_func_delete_window(header_t *control,void *data)
callback_func_delete_window:

	mov eax,[esp+4] ;control

	QuitLibGUI eax

	ret

;int dectostr(int digit,char *str)
dec_to_str:
	mov eax,[esp+4];digit
	mov ecx,[esp+8];string

	push ebx esi edi ;save befor use

	cmp eax,10
	jl print_digit

	mov ebx,eax ;save digit
	mov edi,1
	mov esi,10
	next_i_div:
		cdq	   ;extend to edx:eax
		idiv esi   ;eax=eax/esi
		test eax,eax
		jz no_mul_ten
		       imul edi,esi ;edi=edi*10
		no_mul_ten:
		mov eax,edx ;eax=eax % 10
	cmp eax,esi
	jae next_i_div

	test eax,eax
	jnz no_multyple_10
		;print digit in string
		mov [ecx],byte '1'
		inc ecx
		mov eax,ebx ;eax=digit
		next_i_str_:
			cdq	    ;extend to edx:eax
			idiv edi    ;eax=eax/edi

			mov [ecx],byte '0'

			inc ecx
			mov eax,edx ;eax=eax % edi
		cmp eax,10
		jae next_i_str_
		dec ecx
	jmp exit_multyple
	no_multyple_10:
		;print digit in string
		mov eax,ebx ;eax=digit
		next_i_str:
			cdq	    ;extend to edx:eax
			idiv edi    ;eax=eax/edi

			add al,byte '0' ;al='0'+digit
			mov [ecx],byte al

			inc ecx
			mov eax,edx ;eax=eax % edi
		cmp eax,10
		jae next_i_str
	print_digit:

	add al,byte '0'
	mov [ecx],byte al

	exit_multyple:
	mov eax,ecx

	pop edi esi ebx

	sub eax,[esp+8] ;number of printed simbols
	inc eax

	ret 8

align 4
;----------------data-----------------
txt		db 'PROGRESS ',0,0,0,0,0,0
pbdata		gui_progress_bar_data_t
window		rd 1
progress_bar	rd 1
progress	rd 1
increment	rd 1
digit		rd 1
i_end: