idbChange	equ 2
idbCreate	equ 3
idbLeft 	equ 4
idbRight	equ 5
idbCancel	equ 1

DlgAdd:
DlgProp:

	mov	eax,[SelIcon]
	mov	[DlgSelIcon],eax
	cmp	eax,-1
	jne	SetProp

    ;-----------
	mov	eax,[MouseX]
	mov	ebx,[MouseY]

	cmp	ax,[wsX]
	jae	@f
	mov	ax,[wsX]
      @@:

	cmp	bx,[wsY]
	jae	@f
	mov	bx,[wsY]
      @@:			    ;eax,ebx - реальная координата. Все координаты в проге относительно левого верхнего угла рабочей области

	xor	edx,edx
	mov	dx,[wsXe]
	sub	edx,ICON_SIZE
	cmp	eax,edx
	jbe	@f
	mov	eax,edx
      @@:

	mov	dx,[wsYe]
	sub	edx,ICON_SIZE
	cmp	ebx,edx
	jbe	@f
	mov	ebx,edx
      @@:

	xor	edx,edx 	     ;преобразовываем в относительные
	mov	dx,[wsW]
	shr	edx,1
	add	dx,[wsX]
	cmp	eax,edx
	jbe	@f
	sub	ax,[wsXe]
	inc	ax
	jmp	.dlg1
      @@:
	sub	ax,[wsX]
    .dlg1:

	xor	edx,edx
	mov	dx,[wsH]
	shr	edx,1
	add	dx,[wsY]
	cmp	ebx,edx
	jbe	@f
	sub	bx,[wsYe]
	inc	bx
	jmp	.dlg2
      @@:
	sub	bx,[wsY]
    .dlg2:

	mov	[AddX],eax
	mov	[AddY],ebx
    ;-----------

	mov	ecx,NAME_LENGTH
	mov	edi,DAreaName
	xor	al,al
	rep	stosb
	mov	dword[edtName.size],0
	mov	dword[edtName.pos],0

	mov	ecx,256
	mov	edi,DAreaPath
	rep	stosb
	mov	dword[edtExePath.size],0
	mov	dword[edtExePath.pos],0

	mov	ecx,256
	mov	edi,DAreaParams
	rep	stosb
	mov	dword[edtParams.size],0
	mov	dword[edtParams.pos],0

	mov	ecx,256
	mov	edi,DAreaIcon

	mov	[DAreaIcon],'0'
	mov	dword[edtIcon.size],1
	mov	dword[edtIcon.pos],1

	jmp	startDlg
    ;--------
SetProp:
	mov	esi,[IconsOffs+eax*4]
	mov	edi,DAreaName
	xor	ecx,ecx
     @@:
	lodsb
	stosb
	inc	ecx
	test	al,al
	jnz	@b
	dec	ecx
	mov	dword[edtName.size],ecx
	mov	dword[edtName.pos],ecx

	xor	ecx,ecx
	mov	edi,DAreaPath
     @@:
	lodsb
	stosb
	inc	ecx
	test	al,al
	jnz	@b
	dec	ecx
	mov	dword[edtExePath.size],ecx
	mov	dword[edtExePath.pos],ecx

	xor	ecx,ecx
	mov	edi,DAreaParams
     @@:
	lodsb
	stosb
	inc	ecx
	test	al,al
	jnz	@b
	dec	ecx
	mov	dword[edtParams.size],ecx
	mov	dword[edtParams.pos],ecx

	xor	ecx,ecx
	mov	edi,DAreaIcon
     @@:
	lodsb
	stosb
	inc	ecx
	test	al,al
	jnz	@b
	dec	ecx
	mov	dword[edtIcon.size],ecx
	mov	dword[edtIcon.pos],ecx


startDlg:
	mcall	9,RBProcInfo,-1
	mcall	18,21,dword[RBProcInfo+30]
	mov	[slotDlgAdd],eax

	or	[edtName.flags],ed_focus

	mcall	40,EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER
	xor	eax,eax
	mov	dword[MaxPage],0
      @@:
	add	eax,ICONS_DRAW_COUNTW
	inc	dword[MaxPage]
	cmp	eax,[icon_count]
	jb	@b
	dec	dword[MaxPage]
	cmp	dword[MaxPage],0
	jne	@f
	mov	dword[MaxPage],1
      @@:

DReDraw:
	mcall	48,3,sc,40

	mov	eax,[sc.work]
	rol	eax,16
	add	al,9
	jnc	@f
	mov	al,0FFh
      @@:
	rol	eax,16
	add	al,9
	jnc	@f
	mov	al,0FFh
      @@:
	add	ah,9
	jnc	@f
	mov	ah,0FFh
      @@:

	mov	[sbIcons.bckg_col],eax
	m2m	[sbIcons.frnt_col],[sc.work]
	m2m	[sbIcons.line_col],[sc.work_graph]

	;edit_boxes_set_sys_color edtIcon,endEdits,sc
	edit_boxes_set_sys_color edtName,endEdits,sc

	m2m	[edtIcon.color],[sc.work]
	m2m	[edtIcon.blur_border_color],[sc.work]

	call	DRedrawWin

DMessages:
	mcall	10		;WaitMessage

	dec	eax
	jz	DReDraw
	dec	eax
	jz	DKey
	dec	eax
	jz	DButton
	sub	eax,3
	jz	DMouse

	jmp  DMessages


;---------------------------------------------------------------------
DKey:
	mcall	2		;GetKeyPressed
	cmp	ah,1Bh
	je	DExit
	cmp	ah,9  ;Tab
	je	DNextEdit
	stdcall [edit_box_key],edtName
	stdcall [edit_box_key],edtExePath
	stdcall [edit_box_key],edtParams
	;stdcall [edit_box_key],edtIcon

	jmp	DMessages


DNextEdit:
	test	[edtName.flags],ed_focus
	jne	 .DNE1
	test	[edtExePath.flags],ed_focus
	jne	 .DNE2
	test	[edtParams.flags],ed_focus
	jne	 .DNE3
	;test    [edtIcon.flags],ed_focus
	;jne      .DNE4
	jmp	DMessages

    .DNE1:
	and	[edtName.flags],not ed_focus
	or	[edtExePath.flags],ed_focus
	stdcall [edit_box_draw],edtName
	stdcall [edit_box_draw],edtExePath
	jmp	DMessages

    .DNE2:
	and	[edtExePath.flags],not ed_focus
	or	[edtParams.flags],ed_focus
	stdcall [edit_box_draw],edtExePath
	stdcall [edit_box_draw],edtParams
	jmp	DMessages

    .DNE3:
	and	[edtParams.flags],not ed_focus
	or	[edtName.flags],ed_focus
	stdcall [edit_box_draw],edtParams
	stdcall [edit_box_draw],edtName
	jmp	DMessages

    ;.DNE4:
    ;    and     [edtIcon.flags],not ed_focus
    ;    or      [edtName.flags],ed_focus
    ;    stdcall [edit_box_draw],edtName
    ;    stdcall [edit_box_draw],edtIcon
    ;    jmp     DMessages
;---------------------------------------------------------------------
DButton:
	mcall	17		;GetButtonPressed
	cmp	ah, idbCancel
	je	DExit
	cmp	ah,idbChange
	je	DSetExePath
	cmp	ah,idbCreate
	je	DSaveIcon

	jmp	DMessages
;-------------------------------------------------------------------------------
DMouse:
	stdcall [edit_box_mouse],edtName
	stdcall [edit_box_mouse],edtExePath
	stdcall [edit_box_mouse],edtParams
;        stdcall [edit_box_mouse],edtIcon

	push	[sbIcons.position]
	stdcall [scrollbar_h_mouse],sbIcons
	pop	eax
	cmp	eax,[sbIcons.position]
	je	@f
	call	DrawStdIcons
      @@:

	mcall	37,2		;GetMouseKey
	test	eax,1
	jz	DMessages
	mcall	37,1		;GetMouseWinPos

	test	eax,10001000h
	jne	DMessages
	xor	edx,edx
	mov	dx,ax	  ;y
	shr	eax,16	  ;x

	sub	eax,ICONSX
	js	DMessages
	sub	edx,ICONSY
	js	DMessages

	cmp	eax,(IMG_SIZE+SPCW)*ICONS_DRAW_COUNTW-SPCW
	ja	DMessages
	cmp	edx,(IMG_SIZE+SPCH)*ICONS_DRAW_COUNTH-SPCH
	ja	DMessages

	xor	ebx,ebx
	mov	ecx,[sbIcons.position]
	test	ecx,ecx
	jz	.DM
     @@:
	add	ebx,ICONS_DRAW_COUNTH
	loop	@b
     .DM:

	sub	eax,IMG_SIZE+SPCW
	js     .DM1
     @@:
	add	ebx,ICONS_DRAW_COUNTH
	sub	eax,IMG_SIZE+SPCW
	jns	@b
     .DM1:


	sub	edx,IMG_SIZE+SPCH
	js     .DM2
     @@:
	inc	ebx
	sub	edx,IMG_SIZE+SPCH
	jns	@b
     .DM2:

	mov	eax,ebx

	cmp	eax,[icon_count]
	jae	DMessages

;       stdcall byteToHex,al,DAreaIcon  ;2
	stdcall intToStr,eax,DAreaIcon
	mov	dword[edtIcon.size],edx

    ; stdcall [edit_box_draw],edtIcon

	call DrawSelIcon
	jmp	 DMessages
;-------------------------------------------------------------------------------
DSetExePath:
	stdcall [OpenDialog_Start],OpenDialog_data
	mov	edi,DAreaPath
	xor	al,al
	or	ecx,-1
	repne	scasb
	sub	edi,DAreaPath
	dec	edi
	mov	dword[edtExePath+12*4],edi

	jmp	DMessages;DReDraw;

DSaveIcon:
	mov	edi,DAreaName
	mov	esi,secRButt
    @@: lodsb
	scasb
	jne	@f
	test	al,al
	jnz	@b

	mov	dword[fiRunProg+8],ErrName
	mov	dword[fiRunProg+21],pthNotify
	mcall	70,fiRunProg

	jmp	DMessages
    @@:
	mov	dword[fiRunProg+8],WarningSave
	mov	dword[fiRunProg+21],pthNotify
	mcall	70,fiRunProg


	cmp	[DlgSelIcon],-1
	je	@f

	stdcall EditIcon,[DlgSelIcon],DAreaIcon,DAreaName,DAreaPath,DAreaParams
	mov	ebx,[DlgSelIcon]
	mov	eax,[IconsID+ebx*4]
	jmp	DExitAndSave
     @@:

	call	GenerateID

	mov	ebx,[MaxNumIcon]
	mov	[IconsID+ebx*4],eax
	push	eax
	stdcall AddIcon,[AddX],[AddY],DAreaIcon,DAreaName,DAreaPath,DAreaParams
	pop	eax

;-------------------------------------------------------------------------------
DExitAndSave:

	mov	dword[nameSection],eax
	stdcall [ini_set_str],IconIni,nameSection,keyName,DAreaName,dword[edtName.size]
	stdcall [ini_set_str],IconIni,nameSection,keyPath,DAreaPath,dword[edtExePath.size]
	stdcall [ini_set_str],IconIni,nameSection,keyParams,DAreaParams,dword[edtParams.size]
	stdcall [ini_set_str],IconIni,nameSection,keyIco,DAreaIcon,dword[edtIcon.size]

	cmp	[DlgSelIcon],-1
	jne	@f
	stdcall [ini_set_int],IconIni,nameSection,keyX,[MouseX]
	stdcall [ini_set_int],IconIni,nameSection,keyY,[MouseY]
      @@:
DExit:
	mov	[DlgAddActiv],0
	mov	[slotDlgAdd],0
	mcall	-1		;ExitThread

;##################################################################
proc DRedrawWin
	mcall	12,1		;RedrawWinBeg
	mov	edx,[sc.work]
	or	edx,34000000h
	cmp	[DlgSelIcon],-1
	jne	 @f
	mov	edi,DTitleAdd
	jmp	.l1
      @@:
	mov	edi,DTitleProp
      .l1:
	mcall	0,<100,END_ICONS_AREAW+ICONSX+10>,<100,(IMG_SIZE+SPCH)*ICONS_DRAW_COUNTH+178>,,,

	stdcall [edit_box_draw],edtName
	stdcall [edit_box_draw],edtExePath
	stdcall [edit_box_draw],edtParams

	call	DrawStdIcons


	stdcall [scrollbar_h_draw],sbIcons
	mcall 38,<ICONSX+1,END_ICONS_AREAW-2>,<END_ICONS_AREAH+3,END_ICONS_AREAH+3>,[sc.work_graph]
	mcall	,<ICONSX+1,END_ICONS_AREAW-2>,<END_ICONS_AREAH+3+15,END_ICONS_AREAH+3+15>,[sc.work_graph]

	mcall	,<ICONSX,ICONSX>,<END_ICONS_AREAH+3+1,END_ICONS_AREAH+3+15-1>,[sc.work_graph]
	mcall	,<END_ICONS_AREAW-1,END_ICONS_AREAW-1>,<END_ICONS_AREAH+3+1,END_ICONS_AREAH+3+15-1>,[sc.work_graph]

	;BUTTONS
	mcall	8,<265,75>,<30+END_ICONS_AREAH,24>,idbCreate,[sc.work_button]
	mcall	 ,<360,77>,			  ,idbCancel
	mcall	 ,<END_ICONS_AREAW-30,30>,<30,20> ,idbChange

	;CAPTIONS
	mov	ecx,[sc.work_text]
	or	ecx,90000000h
	mcall	4,<30+IMG_SIZE,10>,,DCaptName
	mcall	 ,<30+IMG_SIZE,35>,,DCaptPath
	mcall	 ,<30+IMG_SIZE,59>,,DCaptParams

if lang eq ru
	mov	ecx,[sc.work_button_text]
	or	ecx,90000000h
	mcall	 ,<369,35+END_ICONS_AREAH>,,DCaptCancel

	cmp	[DlgSelIcon],-1
	jne	@f
	mpack	ebx,275,35+END_ICONS_AREAH
	mov	edx,DCaptCreate
	jmp	.DRD1
      @@:
	mpack	ebx,272,35+END_ICONS_AREAH
	mov	edx,DCaptProperties
     .DRD1:
else
	mov	ecx,[sc.work_button_text]
	or	ecx,90000000h
	mcall	 ,<372,35+END_ICONS_AREAH>,,DCaptCancel


	cmp	[DlgSelIcon],-1
	jne	@f
	mpack	ebx,277,35+END_ICONS_AREAH
	mov	edx,DCaptCreate
	jmp	.DRD1
      @@:
	mpack	ebx,277,35+END_ICONS_AREAH
	mov	edx,DCaptProperties
     .DRD1:
end if
	mcall

	pusha
	mov	ecx,[sc.work_button_text]
	or	ecx,90000000h
	mcall 4,<END_ICONS_AREAW-30+4,33>,,DCaptDots
	popa

	call DrawSelIcon

	mcall	12,2		;RedrawWinEnd
	ret
endp


proc DrawStdIcons
local IcoX:DWORD,\
      IcoY:DWORD,\
      iIcon:DWORD

	mov	ecx,[sbIcons.position]
	xor	eax,eax
	m2m	dword[iIcon],0

	test	ecx,ecx
	jz	.DST1
     @@:
	add	[iIcon],ICONS_DRAW_COUNTH
	add	eax,IMG_SIZE*IMG_SIZE*4*ICONS_DRAW_COUNTH
	loop	@b
   .DST1:

	lea	esi,[eax+44]
	add	esi,[raw_pointer]

	mov	[IcoX],ICONSX

	mov	ecx,ICONS_DRAW_COUNTW
  .DrawIcons:
	push	ecx

	mov	[IcoY],ICONSY

	mov	ecx,ICONS_DRAW_COUNTH
  .DrawIcoStolb:
	push	ecx

	mov	edi,DlgBufImg
	mov	edx,IMG_SIZE

    .DrawLine:


	mov	ecx,IMG_SIZE
     .DrawPix:

	lodsd
	test	eax,0FF000000h
	jnz	@f
	mov	eax,[sc.work]
     @@:

	mov	[edi],ax
	shr	eax,16
	mov	[edi+2],al
	add	edi,3

	loop	.DrawPix

	dec	edx
	jnz    .DrawLine

	mov	edx,[IcoX]
	shl	edx,16
	mov	dx,word[IcoY]
	mcall	7,DlgBufImg,<IMG_SIZE,IMG_SIZE>

	inc	[iIcon]
	mov	eax,[iIcon]
	cmp	eax,[icon_count]
	jae	.endDrawIcon

	add	dword[IcoY],IMG_SIZE+SPCH
	pop	ecx
	loop	.DrawIcoStolb

	add	dword[IcoX],IMG_SIZE+SPCW
	pop	ecx
	loop	.DrawIcons
	jmp	.endProc

    .endDrawIcon:

	mov	ecx,IMG_SIZE*IMG_SIZE
	mov	edi,DlgBufImg
	mov	eax,[sc.work]
	mov	ebx,eax
	shr	ebx,16
     @@:
	mov	word[edi],ax
	mov	[edi+2],bl
	add	edi,3
	loop	@b

				     ;затирание не использованых мест
	jmp	.Dalee

  .DrawIcons2:
	push	ecx

	mov	[IcoY],ICONSY
	mov	ecx,ICONS_DRAW_COUNTH
  .DrawIcoStolb2:
	push	ecx

	mov	edx,[IcoX]
	shl	edx,16
	mov	dx,word[IcoY]
	mcall	7,DlgBufImg,<IMG_SIZE,IMG_SIZE>

     .Dalee:
	add	dword[IcoY],IMG_SIZE+SPCH
	pop	ecx
	loop	.DrawIcoStolb2

	add	dword[IcoX],IMG_SIZE+SPCW
	pop	ecx
	loop	.DrawIcons2

   .endProc:

	ret
endp

proc DrawSelIcon
	mov	edx,[sc.work]
	mcall 13,<ICONSX+18,3*8>,<59,12>

	mov	ecx,[sc.work_text]
	or	ecx,90000000h
	mcall 4,<ICONSX+18,59>,,DAreaIcon

	mov	edi,DAreaIcon
	cmp	byte[edi],0
	jne	@f
	ret
      @@:

	;stdcall hexToInt, edi
	stdcall strToInt, edi
     @@:	     ;eax=num icon
	cmp	eax,[icon_count]
	jb	@f
	xor	eax,eax
     @@:
	test	eax,eax
	je	.DI1
	mov	ecx,eax
	xor	eax,eax
      @@:
	add	eax,IMG_SIZE*IMG_SIZE*4
	loop	@b
     .DI1:
	mov	esi,eax
	add	esi,[raw_pointer]
	add	esi,44

	mov	edi,DlgBufImg
	mov	edx,IMG_SIZE
    .DrawLine:

	mov	ecx,IMG_SIZE
     .DrawPix:

	lodsd
	test	eax,0FF000000h
	jnz	@f
	mov	eax,[sc.work]
     @@:

	mov	word[edi],ax
	shr	eax,16
	mov	[edi+2],al
	add	edi,3

	loop	.DrawPix

	dec	edx
	jnz	.DrawLine

	mcall	7,DlgBufImg,<IMG_SIZE,IMG_SIZE>,<30,15>

	ret
endp

;NOT USED
proc hexToInt strz:DWORD
	push	ebx
	mov	ebx,[strz]
	mov	al,[ebx]
	xor	edx,edx
.loop:
	xor	ecx,ecx
	inc	ebx
	shl	edx,4
	cmp	al,'0'
	jb	.err
	cmp	al,'9'
	jbe	.conv
	mov	ecx,10
	sub	al,11h
	cmp	al,'0'
	jb	.err
	cmp	al,'6'
	ja	.err
.conv:
	sub	al,30h
	add	dl,al
	mov	al,[ebx]
	add	dl,cl
	test	al,al
	jnz	.loop
	mov	eax,edx
	pop	ebx
	ret
.err:
	xor	eax,eax
	pop	ebx
	ret
endp


;NOT USED
proc byteToHex byti:BYTE,target:DWORD
	push	ebx
	mov	ebx,[target]
	mov	dl,[byti]
	rol	dl,4
	mov	ecx,2
.loop:
	mov	al,dl
	and	al,0xF
	cmp	al,10
	jae	@f
	add	al,0x30
	jmp	.n
@@:
	add	al,0x41-10
.n:
	mov	[ebx],al
	inc	ebx
	rol	dl,4
	loop	.loop

	mov	byte[ebx],0
@@:	pop	ebx
	ret
	jmp	@b
endp


proc strToInt strz:DWORD
	push	ebx edi esi
	stdcall strLen, [strz]
	xor	edx,edx
	xor	ebx,ebx
	xor	edi,edi
	std
	inc	ebx
	mov	esi,[strz]
	lea	esi,[esi+eax-2]
.loop:
	xor	eax,eax
	lodsb
	cmp	al,'0'
	jb	.err
	cmp	al,'9'
	ja	.err

	sub	al,0x30
	mul	ebx
	lea	ebx,[ebx*4+ebx] 	;ebx = ebx*10
	shl	ebx,1
	add	edi,eax
	cmp	esi,[strz]
	jae	.loop
	cld
	mov	eax,edi
	pop	esi edi ebx
	ret
.err:
	cld
	or	eax,-1
	pop	esi edi ebx
	ret
endp


;ret: target - string, edx - length string
proc intToStr inti:DWORD, target:DWORD
locals
   resl rb 16
endl
	lea	edi,[resl]
	mov	eax,[inti]
	xor	ecx,ecx
	mov	ebx,10
	jmp	.stlp
.loop:
	xor	edx,edx
	div	ebx
	mov	[edi],dl
	inc	edi
	inc	ecx
.stlp:	cmp	eax,10
	jae	.loop
	mov	[edi],al
	inc	ecx
	mov	edx,ecx
	mov	ebx,[target]
.conv:
	mov	al,[edi]
	add	al,0x30
	mov	[ebx],al
	inc	ebx
	dec	edi
	loop	.conv
	mov	byte[ebx],0
	ret
endp

;for 'ab',0  strLen = 3
proc strLen stringZ:DWORD
	push	edi
	xor	al,al
	mov	edi,[stringZ]
	or	ecx,-1
	repne	scasb
	sub	edi,[stringZ]
	mov	eax,edi
	pop	edi
	ret
endp


