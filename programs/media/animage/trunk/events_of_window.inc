;----------------------------------------------------------
;-------------draw window of program-----------------------
;----------------------------------------------------------
drawwin:
	or	ecx,-1
	mcall SF_THREAD_INFO,procinfo
	mov	esi,procinfo

	mov	eax,[esi+34]
	mov	ebx,[esi+38]
	mov	ecx,[esi+42]
	mov	edx,[esi+46]
	mov	[Window_CordinatX],eax
	mov	[Window_CordinatY],ebx
	
	cmp	ecx,585
	jae	@f
		mov	ecx,585 ;minimum size x
	@@:
	cmp	[Window_SizeY],400
	jae	@f
		mov	edx,400 ;minimum size y
	@@:
	mov	[Window_SizeX],ecx
	mov [Window_SizeY],edx

	mov	[MaxWorkScreen_SizeX],ecx
	mov	[MaxWorkScreen_SizeY],edx
	sub	[MaxWorkScreen_SizeX],ci_edit_wnd_x_pos+2*ci_edit_wnd_border+34
	sub	[MaxWorkScreen_SizeY],ci_edit_wnd_y_pos+2*ci_edit_wnd_border+34

	cmp	[MaxWorkScreen_SizeX],0
	jns	no_znak1

	mov	[MaxWorkScreen_SizeX],ecx

no_znak1:
	cmp	[MaxWorkScreen_SizeY],0
	jns	no_znak2

	mov	[MaxWorkScreen_SizeY],edx

no_znak2:

	mcall SF_REDRAW,SSF_BEGIN_DRAW
;------------------------------------------------
	mov	ebx,50 ;[Window_CordinatX]
	mov	ecx,50 ;[Window_CordinatY]
	shl	ebx,16
	shl	ecx,16
	add	ebx,[Window_SizeX]
	add	ecx,[Window_SizeY]
	mcall SF_CREATE_WINDOW,,,0x53aabbcc,,name_of_program
	
	mov	eax,[procinfo+70] ;staus of window
	test	eax,100b
	jne	.end
	
;------------------------------------------------
	mov	eax,5
	mov	ebx,20
	mov	ecx,[Window_SizeX]
	mov	edx,15
	mov	esi,6
	sub	ecx,10
	call	draw_volume_rectangle ; top menu - file and etc.

	mov	[counter],7
	mov	edi,panel_text
	call	print_panel_text
;------------------------------------------------
	mov	eax,5
	mov	ebx,20+15+1
	mov	ecx,[Window_SizeX]
	mov	edx,50
	mov	esi,1
	sub	ecx,10
	call	draw_volume_rectangle ; tools panel
	call	draw_icons
	call	PrintMousePos
;------------------------------------------------
	mov ecx,[k]
	call MovePictureToWorkScreen
;------------------------------------------------
	mov	ebx,[WorkScreen_SizeX]
	add	ebx,ci_edit_wnd_x_pos+ci_edit_wnd_border
	shl	ebx,16
	mov	bx,ci_edit_wnd_border ;ширина бордюра
	
	mov	ecx,ci_edit_wnd_y_pos+ci_edit_wnd_border
	mov	edx,[Window_SizeY]
	sub	edx,ci_edit_wnd_y_pos+2*ci_edit_wnd_border+34
	shl	ecx,16
	mov	cx,dx ;высота бордюра

	mcall SF_DRAW_RECT,,,0xc7cbcf ;бордюр справа от окна перед скроллингом
;------------------------------------------------
	mcall ,<ci_edit_wnd_x_pos,ci_edit_wnd_border> ;бордюр слева от окна
;------------------------------------------------
	mov	ebx,ci_edit_wnd_x_pos
	mov	edx,[Window_SizeX]
	sub	edx,25+10+ci_edit_wnd_x_pos
	shl	ebx,16
	mov	bx,dx ;ширина бордюра

	mov	ecx,[WorkScreen_SizeY]
	add	ecx,ci_edit_wnd_y_pos+ci_edit_wnd_border
	shl	ecx,16
	mov	cx,ci_edit_wnd_border ;высота бордюра

	mcall SF_DRAW_RECT,,,0xc7cbcf ;бордюр под окном и над скроллингом
;------------------------------------------------
	mcall ,,<ci_edit_wnd_y_pos,ci_edit_wnd_border> ;бордюр под панелью и над окном
;------------------------------------------------
	mov	eax,ci_edit_wnd_x_pos
	mov	ebx,[Window_SizeY]
	mov	ecx,[Window_SizeX]
	mov	edx,20+10
	mov	esi,3
	sub	ecx,35+ci_edit_wnd_x_pos
	sub	ebx,25+10
	call draw_volume_rectangle ; horizontal scrollbar border
;------------------------------------------------
	mov	eax,[Window_SizeX]
	mov	ebx,ci_edit_wnd_y_pos
	mov	ecx,20+10
	mov	edx,[Window_SizeY]
	mov	esi,3
	sub	eax,25+10
	sub	edx,35+ci_edit_wnd_y_pos
	call draw_volume_rectangle ; vertical scrollbar border
	call draw_scrollers
;------------------------------------------------
	mov	eax,[Window_SizeX]
	mov	ebx,[Window_SizeY]
	mov	ecx,20+10
	mov	edx,20+10
	mov	esi,6
	sub	eax,25+10
	sub	ebx,25+10
	call draw_volume_rectangle ; down and right small area
;------------------------------------------------
.end:
	mcall SF_REDRAW,SSF_END_DRAW
	ret
;-----------------------------------------------------------------------------