;----------------------------------------------------------
;-------------draw window of program-----------------------
;----------------------------------------------------------
drawwin:
	or	ecx,-1
	mcall SF_THREAD_INFO,procinfo
	mov	esi,procinfo

	mov	eax,[esi+process_information.box.left]
	mov	ebx,[esi+process_information.box.top]
	mov	ecx,[esi+process_information.box.width]
	mov	edx,[esi+process_information.box.height]
	mov	[Window_CordinatX],eax
	mov	[Window_CordinatY],ebx
	
	cmp ecx,ci_wnd_min_siz_x
	jge @f
		mov ecx,ci_wnd_min_siz_x
	@@:
	cmp edx,ci_wnd_min_siz_y
	jge @f
		mov edx,ci_wnd_min_siz_y
	@@:
	mov	[Window_SizeX],ecx
	mov [Window_SizeY],edx
	mov	[MaxWorkScreen_SizeX],ecx
	mov	[MaxWorkScreen_SizeY],edx
	sub	[MaxWorkScreen_SizeX],ci_edit_wnd_x_pos+2*ci_edit_wnd_border\
		+ci_scroll_dim+ci_offs_skin_w+5
	sub	[MaxWorkScreen_SizeY],ci_edit_wnd_y_pos+2*ci_edit_wnd_border\
		+ci_scroll_dim+ci_offs_skin_h+5

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
	mov	ebx,50 shl 16 ;[Window_CordinatX]
	mov	ecx,50 shl 16 ;[Window_CordinatY]
	add	ebx, [Window_SizeX]
	add	ecx, [Window_SizeY]
	mcall SF_CREATE_WINDOW,,,0x73aabbcc,,name_of_program
	
	mov	eax,[procinfo+70] ;staus of window
	test eax,100b
	jne	.end
	
;------------------------------------------------
	mov	ebx,[Window_SizeX]
	sub	ebx,5+ci_panel_x_pos+ci_offs_skin_w-1
	mcall SF_DRAW_RECT,,ci_panel_y_pos,dword[syscolors+20] ; чистка полосы для меню

	call put_main_menu
;------------------------------------------------
	; draw toolbard background
	mov ebx, ci_panel_x_pos
	shl ebx, 16
	add ebx, [Window_SizeX]
	sub	ebx, 9
	mov ecx, (ci_panel_y_pos shl 16) + 51
	mov edx, 0xD5CDCB
	mcall SF_DRAW_RECT

	call	draw_icons
	call	PrintMousePos
;------------------------------------------------
	mov ecx,[k]
	call MovePictureToWorkScreen
;------------------------------------------------
	mov	ebx,[WorkScreen_SizeX]
	add	ebx,ci_edit_wnd_x_pos+ci_edit_wnd_border
	mov	edx,[Window_SizeX]
	sub	edx,ebx
	sub	edx,ci_scroll_dim+5-ci_edit_wnd_border
	shl	ebx,16
	mov	bx,dx ;ширина бордюра
	
	mov	ecx,ci_edit_wnd_y_pos+ci_edit_wnd_border
	mov	edx,[Window_SizeY]
	sub	edx,ci_edit_wnd_y_pos+2*ci_edit_wnd_border+ci_scroll_dim+5+ci_offs_skin_h
	shl	ecx,16
	mov	cx,dx ;высота бордюра

	mcall SF_DRAW_RECT,,,dword[syscolors+20] ;бордюр справа от окна перед скроллингом
;------------------------------------------------
	mcall ,<ci_edit_wnd_x_pos,ci_edit_wnd_border> ;бордюр слева от окна
;------------------------------------------------
	mov	ebx,ci_edit_wnd_x_pos
	mov	edx,[Window_SizeX]
	sub	edx,ci_scroll_dim+5+ci_edit_wnd_x_pos
	shl	ebx,16
	mov	bx,dx ;ширина бордюра

	mov	ecx,[WorkScreen_SizeY]
	add	ecx,ci_edit_wnd_y_pos+ci_edit_wnd_border
	mov	edx,[Window_SizeY]
	sub	edx,ecx
	sub	edx,ci_scroll_dim+5+ci_offs_skin_h;ci_edit_wnd_border
	shl	ecx,16
	mov	cx,dx ;высота бордюра

	mcall SF_DRAW_RECT,,,dword[syscolors+20] ;бордюр под окном и над скроллингом
;------------------------------------------------
	mcall ,,<ci_edit_wnd_y_pos,ci_edit_wnd_border> ;бордюр под панелью и над окном
;------------------------------------------------
	mov	eax,ci_edit_wnd_x_pos
	mov	ebx,[Window_SizeY]
	mov	ecx,[Window_SizeX]
	mov	edx,ci_scroll_dim
	mov	esi,3
	sub	ecx,ci_scroll_dim+5+ci_edit_wnd_x_pos+ci_offs_skin_w
	sub	ebx,ci_scroll_dim+5+ci_offs_skin_h
	call draw_volume_rectangle ; horizontal scrollbar border
;------------------------------------------------
	mov	eax,[Window_SizeX]
	mov	ebx,ci_edit_wnd_y_pos
	mov	ecx,ci_scroll_dim
	mov	edx,[Window_SizeY]
	mov	esi,3
	sub	eax,ci_scroll_dim+5+ci_offs_skin_w
	sub	edx,ci_scroll_dim+5+ci_edit_wnd_y_pos+ci_offs_skin_h
	call draw_volume_rectangle ; vertical scrollbar border
	call draw_scrollers
;------------------------------------------------
	mov	eax,[Window_SizeX]
	mov	ebx,[Window_SizeY]
	mov	ecx,ci_scroll_dim
	mov	edx,ci_scroll_dim
	mov	esi,6
	sub	eax,ci_scroll_dim+5+ci_offs_skin_w
	sub	ebx,ci_scroll_dim+5+ci_offs_skin_h
	call draw_volume_rectangle ; down and right small area
;------------------------------------------------
.end:
	mcall SF_REDRAW,SSF_END_DRAW
	ret
;-----------------------------------------------------------------------------