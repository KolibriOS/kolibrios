;---------------------------------------------------------
;--------initializate data and params of program----------
;---------------------------------------------------------
	mov	[Window_SizeX],640
	mov	[Window_SizeY],220
	mov	[Window_CordinatX],50
	mov	[Window_CordinatY],50
	mov	[WorkScreen_SizeX],100
	mov	[WorkScreen_SizeY],100
	mov	[MaxWorkScreen_SizeX],100
	mov	[MaxWorkScreen_SizeY],100
	and	[PosX],0
	and	[PosY],0
	mov	[Picture_SizeX],640
	mov	[Picture_SizeY],400
	mov	[k],1

	mcall SF_SYS_MISC,SSF_MEM_ALLOC,4096
	mov	[ScreenPointer],eax

	and	[save_flag],0
	mov	[line_width],1
	mov	[lastik_is_active],0
	and	[crossing],0
	and	[number_undo],0
	and	[instrument_used],0
	and	[DrawSprite_flag],0
finit
