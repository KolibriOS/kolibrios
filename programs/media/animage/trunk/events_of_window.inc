;----------------------------------------------------------
;-------------draw window of program-----------------------
;----------------------------------------------------------
drawwin:
	or	ecx,-1
	mcall	9,procinfo
	mov	esi,procinfo

	mov	eax,[esi+34]
	mov	ebx,[esi+38]
	mov	ecx,[esi+42]
	mov	edx,[esi+46]
	mov	[Window_CordinatX],eax
	mov	[Window_CordinatY],ebx
	mov	[Window_SizeX],ecx
	mov	[Window_SizeY],edx

	cmp	[Window_SizeX],585
	jae	no_minimum_size_x

	mov	[Window_SizeX],585
	mov	ecx,[Window_SizeX]

no_minimum_size_x:
	cmp	[Window_SizeY],400
	jae	no_minimum_size_y

	mov	[Window_SizeY],400
	mov	edx,[Window_SizeY]

no_minimum_size_y:
	mov	[MaxWorkScreen_SizeX],ecx
	mov	[MaxWorkScreen_SizeY],edx
	sub	[MaxWorkScreen_SizeX],20+10+5+10
	sub	[MaxWorkScreen_SizeY],20+10+15+1+49+20+10

	cmp	[MaxWorkScreen_SizeX],0
	jns	no_znak1

	mov	[MaxWorkScreen_SizeX],ecx

no_znak1:
	cmp	[MaxWorkScreen_SizeY],0
	jns	no_znak2

	mov	[MaxWorkScreen_SizeY],edx

no_znak2:

	mcall	12,1
;------------------------------------------------
	mov	ebx,50 ;[Window_CordinatX]
	mov	ecx,50 ;[Window_CordinatY]
	shl	ebx,16
	shl	ecx,16
	add	ebx,[Window_SizeX]
	add	ecx,[Window_SizeY]
	;xor	esi,esi	;0x805080d0
;	xor	edi,edi	;0x005080d0
	mcall	0,,,0x53aabbcc,,name_of_program
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
	mov	ecx,[k]
	and	[PosX],0
	and	[PosY],0
	call	MovePictureToWorkScreen
;------------------------------------------------
	mov	ebx,[WorkScreen_SizeX]
	add	ebx,5+3
	mov	edx,[Window_SizeX]
	sub	edx,ebx
	sub	edx,25+10-3
	shl	ebx,16
	mov	bx,dx
	
	mov	ecx,20+15+1+49+2
	mov	edx,[Window_SizeY]
	mov	eax,[WorkScreen_SizeY]
	shr	eax,16
	sub	edx,eax
	sub	edx,20+15+1+49+2+25+10
	shl	ecx,16
	mov	cx,dx

	mcall	13,,,0xc7cbcf
;------------------------------------------------
	mcall	,<5,3>
;------------------------------------------------
	mov	ebx,5
	mov	edx,[Window_SizeX]
	mov	eax,[WorkScreen_SizeX]
	shr	eax,16
	sub	edx,eax
	sub	edx,25+10
	shl	ebx,16
	mov	bx,dx

	mov	ecx,[WorkScreen_SizeY]
	add	ecx,20+15+1+49+2+3
	mov	edx,[Window_SizeY]
	sub	edx,ecx
	sub	edx,25+10-3
	shl	ecx,16
	mov	cx,dx

	mcall	13,,,0xc7cbcf
;------------------------------------------------
	mcall	,,<20+15+1+49+2,3>
;------------------------------------------------
;	draw_volume_rectangle
;	eax - start X
;	ebx - start Y
;	ecx - size X
;	edx - size Y

	mov	eax,5
	mov	ebx,[Window_SizeY]
	mov	ecx,[Window_SizeX]
	mov	edx,20+10
	mov	esi,3
	sub	ecx,30
	sub	ebx,25+10
	call	draw_volume_rectangle	; horizontal scrollbar
;------------------------------------------------
	mov	eax,[Window_SizeX]
	mov	ebx,20+15+1+49+2
	mov	ecx,20+10
	mov	edx,[Window_SizeY]
	mov	esi,3
	sub	eax,25+10
	sub	edx,20+15+1+49+5+20
	call	draw_volume_rectangle	; vertical scrollbar
	call	draw_scrollers
;------------------------------------------------
	mov	eax,[Window_SizeX]
	mov	ebx,[Window_SizeY]
	mov	ecx,20+10
	mov	edx,20+10
	mov	esi,6
	sub	eax,25+10
	sub	ebx,25+10
	call	draw_volume_rectangle	; down and right small area
;------------------------------------------------
	mcall	12,2
	ret
;-----------------------------------------------------------------------------