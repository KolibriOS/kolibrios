ICON_STRIP	equ '/rd/1/iconstrp.png'
ICON_INI	equ '/rd/1/settings/icon.ini'
ICON_SIZE	equ 68	  ;размер области для иконки с надписью
IMG_SIZE	equ 32	  ;размер иконок
TEXT_BOTTOM_Y	equ 15	  ;отступ по Y текста от низа иконки
IMAGE_TOP_Y	equ 10	  ;>=1 Координата Y иконки в области для данной иконки
ALIGN_SIZE	equ 68	  ;размер сетки выравнивания
NAME_LENGTH	equ 11	 ;длина имени иконки
MIN_NO_MOVING	equ 8	 ;через столько пикселей сдвига мыши начинается таскание иконки

		     ;--------для диалога создания/редактирования
ICONSX		equ 20
ICONSY		equ 80
ICONS_DRAW_COUNTW equ 10  ;количество иконок в ширину
ICONS_DRAW_COUNTH equ 2   ;количество иконок в высоту
SPCW		equ 3	  ;пробел между иконками по горизонтали
SPCH		equ 3
END_ICONS_AREAW equ ICONSX+(IMG_SIZE+SPCW)*ICONS_DRAW_COUNTW-SPCW
END_ICONS_AREAH equ ICONSY+(IMG_SIZE+SPCH)*ICONS_DRAW_COUNTH-SPCH



SizeData	equ bufStdIco+32
BegData 	equ fiStdIco.point
;------------------------------------------------------------------------------
	use32
	org 0x0
	db 'MENUET01'	; 8 byte id
	dd 0x01 	; header version
	dd START	; start of code
	dd I_END	; size of image
	dd ENDMEM	; memory for app
	dd stack_main	; esp
	dd 0		; boot parameters
	dd 0		; path
;------------------------------------------------------------------------------
include 'lang.inc'
include '../../macros.inc'
include '../../proc32.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../dll.inc'
;include '../../debug.inc'
;------------------------------------------------------------------------------
START:		; start of execution
	mcall	68,11
	stdcall dll.Load,IMPORTS
	test	eax,eax
	jnz	ErrLoadLibs

; unpack deflate
	mov	eax,[unpack_DeflateUnpack2]
	mov	[deflate_unpack],eax
;---------------------------------------------------------------------
; get size of file ICONSTRP.PNG
	mcall	70,fiStdIco
	test	eax,eax
	jnz	ErrorStrp
; get memory for ICONSTRP.PNG
	mov	ecx,dword[bufStdIco+32]
	mov	[fiStdIco.size],ecx
	mov	[img_size],ecx
	mcall	68,12
	mov	[fiStdIco.point],eax
	mov	[image_file],eax
; load ICONSTRP.PNG
	mov	dword[fiStdIco],0
	mcall	70,fiStdIco
	test	eax,eax
	jnz	close
; convert PNG to RAW
	xor	eax,eax
	mov	[return_code],eax
;int3

	push	image_file
	call	[cnv_png_import.Start]

	mov	eax,[raw_pointer]
	mov	ebx,[eax+32]
	mov	[strip_file_size],ebx
	mov	eax,[eax+28]
	add	eax,[raw_pointer]
	mov	[strip_file],eax
; back memory to system
	mcall	68,13,[fiStdIco.point]

	mov	eax,[raw_pointer]
	mov	eax,[eax+8]
	shr	eax,5
	mov	[icon_count],eax

	and	eax,0x7
	mov	[cur_band_compensation],eax

;########## загружаем данные иконок в память ##########################

	mcall	70,fiIni		     ;выделяем память, достаточной для хранения ini файла.
	test	eax,eax 		;Её точно хватит для хранения данных об иконках
	jnz	ErrorIni

	cmp	dword[bufIni+32],0
	je	ErrorIni
	mcall	68,12,dword[bufIni+32]
	mov	dword[BegData],eax
	jmp	NoErrIni

  ErrorIni:
	mcall	70,fiRunProg
	mcall	-1

  NoErrIni:
	m2m	[PIcoDB],[BegData]



	mov	edi,IconsID
	xor	eax,eax
	mov	ecx,100h/4
	rep stosd
	mov	[nLoadIcon],0
	stdcall [ini_enum_sections],IconIni,LoadIconsData
;int3
	mov	eax,dword[PIcoDB]
	sub	eax,[BegData]
	mov	dword[SizeData],eax
	mov	eax,[BegData]
	cmp	eax,[PIcoDB]
	jne	@f
	mov	dword[eax],0
	mov	dword[SizeData],0
   @@:
;######################################################################
	call	FillIconsOffs		       ;заполняет MaxNumIcon,IconsOffs

	mcall	40,0100000b		       ;нужны только события мыши, перерисовка иконок будет в другом потоке

	mov	eax,[icon_count]
	mov	bl,ICONS_DRAW_COUNTH
	div	bl
	test	ah,ah
	jz	@f
	inc	al
     @@:
	and	eax,0FFh
	mov	[sbIcons.max_area],eax

	mcall	51,1,BGRedrawThread,stack_bredraw	;запускаем поток перерисовки иконок
	stdcall [OpenDialog_Init],OpenDialog_data

;dph [MaxNumIcon]

messages:
	mcall	10
	sub	eax,6
	jz	MSGMouse

	jmp	messages


MSGMouse:
	mcall	37,0	;GetMousePos
	xor	ebx,ebx
	mov	bx,ax
	shr	eax,16
	mov	ecx,ebx
	mov	ebx,eax

	mcall	34
	cmp	eax,1
	jne	messages

	cmp	[RButtonActiv],1
	je	messages

	mov	[MouseY],ecx
	mov	[MouseX],ebx


	mcall	37,2
	test	al,001b
	jnz	LButtonPress
	test	al,010b
	jnz	RButtonPress
	jmp	messages

ErrLoadLibs:
	;dps     'Не удалось загрузить необходимые библиотеки'
	;debug_newline
	jmp	close
ErrorStrp:
	;dps     'Ошибка открытия iconstrp.png'
	;debug_newline
close:
	mcall	-1

LButtonPress:

	stdcall GetNumIcon,[MouseX],[MouseY],-1
;int3
	cmp	eax,-1
	jnz	@f

    WaitLB1:
	mcall	37,2
	test	al,001b
	jz	messages
	;Yield
	mcall	5,1
	jmp	WaitLB1


     @@:
	push	eax
	stdcall DrawIcon,eax,1
   WaitLB:
	mcall	37,2
	test	al,001b
	jz	endWaitLB

	mcall	37,0
	xor	ebx,ebx
	mov	bx,ax
	shr	eax,16
	sub	eax,[MouseX]
	jns	@f
	neg	eax
      @@:
	sub	ebx,[MouseY]
	jns	@f
	neg	ebx
      @@:
	cmp	[bFixIcons],0
	jne	@f
	cmp	eax,MIN_NO_MOVING
	ja	MovingIcon
	cmp	ebx,MIN_NO_MOVING
	ja	MovingIcon
      @@:
	;Yield
	mcall	5,1	;Sleep   1

	jmp	WaitLB
   endWaitLB:

	mcall	37,0
	xor	ebx,ebx
	mov	bx,ax
	shr	eax,16
	mov	ecx,ebx
	mov	ebx,eax
	mov	[MouseX],ebx
	mov	[MouseY],ecx

	stdcall GetNumIcon,[MouseX],[MouseY],-1
	cmp	eax,[esp]	;[esp] = номер иконки
	jne	@f

	mov	edi,[IconsOffs+eax*4]
	or	ecx,-1
	xor	al,al
	repne	scasb
	mov	ebx,edi
	repne	scasb
			     ;run program
	mov	dword[fiRunProg+8],edi
	mov	dword[fiRunProg+21],ebx
	mcall	70,fiRunProg
	test	eax,80000000h
	jz	@f

	mov	dword[fiRunProg+8],ErrRunProg
	mov	dword[fiRunProg+21],pthNotify
	mcall	70,fiRunProg

     @@:
	pop	eax
	stdcall RestoreBackgrnd,eax
	jmp	messages

;-------------------------------------------------------------------------------
MovingIcon:
	stdcall GetNumIcon,[MouseX],[MouseY],-1
	mov	[SelIcon],eax
	mov	[IconNoDraw],eax
	stdcall RestoreBackgrnd,[SelIcon]

;        mov     ecx,[MaxNumIcon]
;        xor     ebx,ebx
;   .MI: push    ecx
;        cmp     ebx,[SelIcon]
;        je      @f
;        stdcall DrawIcon,ebx,0
;      @@:
;        inc     ebx
;        pop     ecx
;        loop    .MI



;dps 'Moving'
;debug_newline
;        mov     edi,[SelIcon]
;        mov     edi,[IconsOffs+edi*4]
;        or      ecx,-1
;        xor     al,al
;        repne scasb
;        repne scasb
;        repne scasb
;        repne scasb
;        xor     ebx,ebx
;        xor     esi,esi
;        mov     bx,word[edi+2]
;        mov     si,word[edi]
;        stdcall GetNumIcon,ebx,esi,[SelIcon]
;        cmp     eax,-1
;        je      @f
;  dps 'Q'
;        stdcall DrawIcon,eax,0
;     @@:
;
;        add     ebx,ICON_SIZE-1
;        stdcall GetNumIcon,ebx,esi,[SelIcon]
;        cmp     eax,-1
;        je      @f
;  dps 'Q'
;        stdcall DrawIcon,eax,0
;
;     @@:
;        add     esi,ICON_SIZE-1
;        stdcall GetNumIcon,ebx,esi,[SelIcon]
;        cmp     eax,-1
;        je      @f
;  dps 'Q'
;        stdcall DrawIcon,eax,0
;
;     @@:
;        sub     ebx,ICON_SIZE-1
;        stdcall GetNumIcon,ebx,esi,[SelIcon]
;        cmp     eax,-1
;        je      @f
;   dps 'Q'
;        stdcall DrawIcon,eax,0
;     @@:


; Sleep 40

;qweqwe:

	mov    [MovingActiv],1
	mcall	51,1,MovingWnd,stack_mov	;CreateThread MovingWnd,stack_dlg
   .WaitLB:
	mcall	37,2	;GetMouseKey
	test	al,001b
	jz	.endWaitLB

	;Yield
	mcall	5,1	;Sleep 1
	jmp	.WaitLB
   .endWaitLB:


	mcall	37,0		;GetMousePos
	xor	ebx,ebx
	mov	bx,ax
	shr	eax,16

	sub	eax,1+ICON_SIZE/2
	jnc	@f
	xor	eax,eax
      @@:

	sub	ebx,ICON_SIZE/2-7
	jnc	@f
	xor	ebx,ebx
      @@:

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
	jmp	.mov1
      @@:
	sub	ax,[wsX]
    .mov1:

	xor	edx,edx
	mov	dx,[wsH]
	shr	edx,1
	add	dx,[wsY]
	cmp	ebx,edx
	jbe	@f
	sub	bx,[wsYe]
	inc	bx
	jmp	.mov2
      @@:
	sub	bx,[wsY]
    .mov2:

	stdcall SetPosition,[SelIcon],eax,ebx

	m2m	[PIcoDB],[BegData]
	stdcall [ini_enum_sections],IconIni,Ini_SavePos  ;in RButton.inc

	mov	[bNotSave],1
	mov	[IconNoDraw],-1

	mov    [MovingActiv],0		;только теперь отключаем окно с мышью

	jmp	messages

;-------------------------------------------------------------------------------

RButtonPress:
	mov	[RButtonActiv],1

     @@:
	mcall	37,2	;GetMouseKey
	test	al,010b
	jz	@f
	mcall	5,1	;Yield
	jmp	@b
     @@:

	mcall	51,1,RButtonWin,stack_rmenu	  ;CreateThread RButtonWin,stack_rmenu

	jmp	messages


;###############################################################################
;ret eax = 1/0 = удача/ошибка
proc DrawIcon NumIcon:DWORD,Activ:DWORD ;NumIcon = 0..n
local	IconData:DWORD
	push	ebx edi esi

	mov	ecx,ICON_SIZE*ICON_SIZE
	xor	eax,eax
	mov	edi,IconArea
	rep	stosd

	mov	eax,[NumIcon]
	cmp	eax,[MaxNumIcon]
	jb	@f
	xor	eax,eax
	pop	esi edi ebx
	ret
      @@:

	mov	esi,[IconsOffs+eax*4]
	mov	[IconData],esi

			      ;рисуем текст
	mov	edi,esi
	xor	al,al
	or	ecx,-1
	repne scasb
	mov	edx,esi
	mov	eax,edi
	sub	eax,esi
	dec	eax
	shl	eax,1		 ;*6
	lea	eax,[eax*2+eax]
	mov	ebx,ICON_SIZE
	sub	ebx,eax
	shr	ebx,1	     ;ebx = x текста
	shl	ebx,16
	mov	bx,ICON_SIZE
	sub	bx,TEXT_BOTTOM_Y
	mov	ecx,88000000h
	mov	edi,IconAreaH
	add	ebx,-1*10000h+0
	mcall	4
	add	ebx,2*10000h+0
	mcall	;4
	add	ebx,-1*10000h-1
	mcall	;4
	add	ebx,0*10000h+2
	mcall	;4
	add	ebx,1*10000h+0
	mcall	;4
	inc	ebx;add     ebx,0*10000h+1
	mcall	;4
	add	ebx,-1*10000h+0
	mcall	;4
	add	ebx,0*10000h-2
	mov	ecx,88FFFFFFh
	mcall	;4
				    ;рисуем картинку

	mov	edi,esi
	xor	al,al
	or	ecx,-1
	repne	scasb
	repne	scasb
	repne	scasb
	mov	al,[edi]
	cmp	al,'9'
	ja	PathToIcon
	cmp	al,'/'
	jne	GetIconInd

   PathToIcon:
	mov	al,30h		 ;заглушка!!!!!!!!!!!!!
	mov	byte[edi+1],0

	jmp	CopyToMem
   GetIconInd:

	sub	al,30h
	cmp	byte[edi+1],0
	je	@f
	shl	eax,1
	lea	eax,[eax*4+eax]
	xor	edx,edx
	mov	dl,[edi+1]
	sub	dl,30h
	add	eax,edx
     @@:	     ;eax=num icon
	cmp	eax,[icon_count]
	jb	@f
	xor	eax,eax
     @@:
	test	eax,eax
	je	DI1
	mov	ecx,eax
	xor	eax,eax
      @@:
	add	eax,IMG_SIZE*IMG_SIZE*4
	loop	@b
     DI1:
	add	eax,[raw_pointer]
	add	eax,0+4*11
	mov	esi,eax

  CopyToMem:
	mov	edi,IconArea+((IMAGE_TOP_Y*ICON_SIZE)+((ICON_SIZE-IMG_SIZE)/2))*4

	mov	eax,IMG_SIZE
	mov	edx,eax
      @@:
	mov	ecx,eax
	rep	movsd
	add	edi,(ICON_SIZE-IMG_SIZE)*4
	dec	edx
	jnz	@b

;-----------------
				;если надо, то рисуем обводку
	cmp	[Activ],0
	je	.NoSelect

	mov	edi,IconArea
	mov	eax,0FF000000h
	mov	ecx,ICON_SIZE
	rep	stosd
	;mov     edi,IconArea+ICON_SIZE*1

	mov	ecx,ICON_SIZE-1
    @@: mov	dword[edi],eax
	add	edi,(ICON_SIZE)*4
	loop	@b

	mov	edi,IconArea+ICON_SIZE*2*4-4
	mov	ecx,ICON_SIZE-1
    @@: mov	dword[edi],eax
	add	edi,(ICON_SIZE)*4
	loop	@b

	mov	edi,IconArea+ICON_SIZE*(ICON_SIZE-1)*4+4
	mov	ecx,ICON_SIZE-2
	rep	stosd


	mov	edi,IconArea+ICON_SIZE*4+4
	mov	eax,0FFFFFFFFh
	mov	ecx,ICON_SIZE-2
	rep	stosd

	mov	edi,IconArea+ICON_SIZE*4+4
	mov	ecx,ICON_SIZE-2
    @@: mov	dword[edi],eax
	add	edi,(ICON_SIZE)*4
	loop	@b

	mov	edi,IconArea+ICON_SIZE*2*4-4*2
	mov	ecx,ICON_SIZE-3
    @@: mov	dword[edi],eax
	add	edi,(ICON_SIZE)*4
	loop	@b

	mov	edi,IconArea+ICON_SIZE*(ICON_SIZE-2)*4+4*2
	mov	ecx,ICON_SIZE-3
	rep	stosd
;--------------

   .NoSelect:
	mov	edi,[IconData]
	xor	al,al
	or	ecx,-1
	repne	scasb
	repne	scasb
	repne	scasb
	repne	scasb
	mov	edx,[edi]

	test	edx,00008000h
	jz	@f
	add	dx,[wsYe]
	jmp	.DI11
    @@:
	add	dx,[wsY]
   .DI11:

	test	edx,80000000h
	jz	@f
	mov	ax,[wsXe]
	shl	eax,16
	add	edx,eax
	jmp	.DI12
    @@:
	mov	ax,[wsX]
	shl	eax,16
	add	edx,eax
   .DI12:

	mov	ebx,IconArea
	mov	ecx,ICON_SIZE*10000h+ICON_SIZE
	mcall	25

;        mov     eax,1
	pop	esi edi ebx
	ret
endp

proc RestoreBackgrnd,NumIcon:DWORD
	push	ebx edi
	mov	eax,[NumIcon]
	cmp	eax,[MaxNumIcon]
	jb	@f
	xor	eax,eax
	pop	edi ebx
	ret
      @@:

	mov	edi,[IconsOffs+eax*4]
	xor	al,al
	or	ecx,-1
	repne	scasb
	repne	scasb
	repne	scasb
	repne	scasb

	mov	ax,[edi+2]
	test	ax,8000h
	jz	@f
	add	ax,[wsXe]
	jmp	.rbg1
      @@:
	add	ax,[wsX]
     .rbg1:

	mov	bx,[edi]
	test	bx,8000h
	jz	@f
	add	bx,[wsYe]
	jmp	.rbg2
      @@:
	add	bx,[wsY]
     .rbg2:

	mov	cx,ax
	shl	ecx,16
	add	ax,ICON_SIZE
	mov	cx,ax
	mov	dx,bx
	shl	edx,16
	add	bx,ICON_SIZE
	mov	dx,bx
	dec	ecx
	dec	edx
	mcall	15,9
	pop	edi ebx
	ret
endp

				 ;заполняет MaxNumIcon,IconsOffs
proc FillIconsOffs
	push	ebx edi
	mov	edi,[BegData]
	mov	dword[MaxNumIcon],0
	cmp	dword[edi],0
	jne	@f
	mov	dword[IconsOffs],0
	pop	edi ebx
	ret
    @@:

	mov	[IconsOffs],edi
	xor	al,al
	xor	edx,edx
	mov	ebx,dword[SizeData]
	add	ebx,dword[BegData]
	or	ecx,-1
 .CalcNumIc:

	repne	scasb
	repne	scasb
	repne	scasb
	repne	scasb
	add	edi,4
	mov	dword[IconsOffs+edx+4],edi
	inc	dword[MaxNumIcon]
	add	edx,4

	cmp	edi,ebx
	jae	@f
	jmp	.CalcNumIc
   @@:

	mov	dword[IconsOffs+edx],0

	pop	edi ebx
	ret
endp

proc LoadIconsData stdcall,f_name,sec_name
	push	ebx esi edi

	mov	edi,secRButt
	mov	esi,[sec_name]
    @@: lodsb
	scasb
	jnz	.lid1
	test	al,al
	jnz	@b

	mov	eax,1
	pop	edi esi ebx
	ret
     .lid1:


	mov	ebx,[sec_name]		;копируем ID
	mov	ax,[ebx]
	mov	edi,[nLoadIcon]
	mov	word[edi*4+IconsID],ax
	mov	word[edi*4+IconsID+2],0

	mov	edi,[PIcoDB]
	stdcall [ini_get_str],[f_name],[sec_name],keyName,edi,4096,0
	test	eax,eax
	jz	@f
	xor	eax,eax
	pop	edi esi ebx
	ret
     @@:
	xor	al,al
	or	ecx,-1
	repne	scasb



	stdcall [ini_get_str],[f_name],[sec_name],keyPath,edi,4096,0
	test	eax,eax
	jz	@f
	xor	eax,eax
	pop	edi esi ebx
	ret
     @@:
	xor	al,al
	or	ecx,-1
	repne	scasb

	stdcall [ini_get_str],[f_name],[sec_name],keyParams,edi,4096,0
	test	eax,eax
	jz	@f
	xor	eax,eax
	pop	edi esi ebx
	ret
     @@:
	xor	al,al
	or	ecx,-1
	repne	scasb

	stdcall [ini_get_str],[f_name],[sec_name],keyIco,edi,4096,0
	test	eax,eax
	jz	@f
	xor	eax,eax
	pop	edi esi ebx
	ret
     @@:
	xor	al,al
	or	ecx,-1
	repne	scasb

	stdcall [ini_get_int],[f_name],[sec_name],keyX,80000000h
	cmp	eax,80000000h
	jne	@f
	xor	eax,eax
	pop	edi esi ebx
	ret
     @@:
	mov	word[edi+2],ax

	stdcall [ini_get_int],[f_name],[sec_name],keyY,80000000h
	cmp	eax,80000000h
	jne	@f
	xor	eax,eax
	pop	edi esi ebx
	ret
     @@:
	mov	word[edi],ax
	add	edi,4
	mov	[PIcoDB],edi

	inc	[nLoadIcon]

	mov	eax,1
	pop	edi esi ebx
	ret
endp

proc GenerateID ;ax = ID
	push	ebx edi
	mov	ebx,[MaxNumIcon]
	test	ebx,ebx
	jnz	@f
	mov	eax,'00'
	pop	edi ebx
	ret
     @@:

	mov	eax,dword[IconsID+ebx*4-4]
  .inc:
	inc	ah
	cmp	ah,'9'+1
	jne	@f
	mov	ah,'A'
       @@:
	cmp	ah,'F'+1
	jne	@f
	mov	ah,'0'
	inc	al
       @@:
	cmp	al,'9'+1
	jne	@f
	mov	al,'A'
       @@:
	cmp	al,'F'+1
	jne	@f
	mov	al,'0'
       @@:


	mov	edi,IconsID
	;cmp     dword[edi],0
	;je      @f
	mov	ecx,100h
    @@: scasd
	je	.inc
	cmp	dword[edi],0
	je	@f
	loop	@b
     @@:

	pop	edi ebx
	ret
endp

include 'iconman.inc'
include 'bgredraw.inc'
include 'RButton.inc'
include 'DlgAdd.inc'
include 'Moving.inc'

;'Eolite',0,'/sys/File managers/eolite',0,'/hd0/3/Muzik',0,'1',0,00010001h
;-------------------------------------------------------------------------------
;##### DATA ####################################################################
;-------------------------------------------------------------------------------
; not change this section!!!
; start section
;------------------------------------------------------------------------------
align 4
image_file     dd 0 ;+0
raw_pointer    dd 0 ;+4
return_code    dd 0 ;+8
img_size       dd 0 ;+12
deflate_unpack dd 0 ;+16        ; not use for scaling
raw_pointer_2  dd 0 ;+20        ; not use for scaling
;------------------------------------------------------------------------------
; end section
;------------------------------------------------------------------------------


align 4
fiStdIco:
	dd 5
	dd 0
	dd 0
.size	dd 0
.point	dd bufStdIco
	db ICON_STRIP,0


align 4
fiRunProg:	      ;для запуска программ
	dd 7
	dd 0
	dd 0
	dd 0
	dd ErrNotFoundIni
	db 0
	dd pthNotify

fiIni	dd 5	       ;для ini файла
	dd 0
	dd 0
	dd 0
	dd bufIni
	db 0
	dd IconIni


IconsFile	db ICON_STRIP,0

align 4
MaxNumIcon	dd 0		;количество иконок
IconNoDraw	dd -1		;-1 либо номер иконки, которую не надо рисовать( когда её таскают )

bFixIcons	dd 1
bNotSave	dd 0

LButtonActiv	dd 0
RButtonActiv	dd 0
MovingActiv	dd 0
DlgAddActiv	dd 0

IconIni 	db ICON_INI,0

pthNotify	db '/rd/1/@notify',0

keyName 	db 'name',0
keyPath 	db 'path',0
keyParams	db 'param',0
keyIco		db 'ico',0
keyX		db 'x',0
keyY		db 'y',0

;-------------------------------------------------------------------------------
IMPORTS:
library cnv_png ,'cnv_png.obj',\
	archiver,'archiver.obj',\
	box_lib ,'box_lib.obj',\
	proc_lib,'proc_lib.obj',\
	libini	,'libini.obj'

import	cnv_png,\
	cnv_png_import.Start	,'START',\
	cnv_png_import.Version	,'version',\
	cnv_png_import.Check	,'Check_Header',\
	cnv_png_import.Assoc	,'Associations'

import	archiver,\
	unpack_DeflateUnpack2	,'deflate_unpack2'

import	box_lib,\
	edit_box_draw		,'edit_box',\
	edit_box_key		,'edit_box_key',\
	edit_box_mouse		,'edit_box_mouse',\
	scrollbar_h_draw	,'scrollbar_h_draw',\
	scrollbar_h_mouse	,'scrollbar_h_mouse'

import	proc_lib,\
	OpenDialog_Init 	,'OpenDialog_init',\
	OpenDialog_Start	,'OpenDialog_start'

import	libini,\
	ini_enum_sections	,'ini_enum_sections',\
	ini_enum_keys		,'ini_enum_keys',\
	ini_get_str		,'ini_get_str',\
	ini_set_str		,'ini_set_str',\
	ini_get_color		,'ini_get_color',\
	ini_get_int		,'ini_get_int',\
	ini_set_int		,'ini_set_int',\
	ini_del_section 	,'ini_del_section',\
	ini_exist_sect		,'ini_exist_sect'


;ini.get_str (f_name, sec_name, key_name, buffer, buf_len, def_val)
;ini.set_str (f_name, sec_name, key_name, buffer, buf_len)


;-------------------------------------------------------------------------------
;----- RButton.inc -------------------------------------------------------------
;-------------------------------------------------------------------------------

if lang eq ru
 MinRMenuW	 dd 18*6+10
else
 MinRMenuW	 dd 15*6+10
end if

secRButt	db 'rbmenu',0

PredItem	dd -1

if lang eq ru
 RMenuRedrawFon db 'Перерисовать',0
 RMenuAlign	db 'Выровнять по сетке',0
 RMenuOffMoving db 'Закрепить иконки',0
 RMenuOnMoving	db 'Открепить иконки',0
 RMenuAdd	db 'Добавить',0
 RMenuDel	db 'Удалить',0
 RMenuProp	db 'Свойства',0
else
 RMenuRedrawFon db 'Redraw',0
 RMenuAlign	db 'Snap to Grid',0
 RMenuOffMoving db 'Fix the icons',0
 RMenuOnMoving	db 'Unfix the icons',0
 RMenuAdd	db 'Add',0
 RMenuDel	db 'Delete',0
 RMenuProp	db 'Properties',0
end if

if lang eq ru
 ErrRunProg	db '"Icon\nОшибка запуска программы" -tE"',0
 WarningSave	db '"Icon\nНе забудьте сохранить изменения, запустить RDSave" -tI',0
 ErrNotFoundIni db '"Icon\nНе найден icon.ini" -tE',0
 ErrName	db '"Icon\nИмя \"rbmenu\" зарезервировано" -tE',0
else
 ErrRunProg	db '"Icon\nError running program" -tE',0
 WarningSave	db '"Icon\nDo not forget to save the changes, run RDSave" -tI',0
 ErrNotFoundIni db '"Icon\nicon.ini not found" -tE',0
 ErrName	db '"Icon\nThe name \"rbmenu\" is reserved" -tE',0
end if

;-------------------------------------------------------------------------------
;------- AddDlg.inc ---------------------------------------------------------------
;-------------------------------------------------------------------------------
if lang eq ru
DTitleAdd	db 'Добавить иконку',0
DTitleProp	db 'Изменить иконку',0

DCaptName	db 'Имя',0
DCaptPath	db 'Путь',0
DCaptParams	db 'Параметры',0
DCaptIcon	db 'Иконка',0
;DCaptChange     db '.',0
DCaptCreate	db 'Создать',0
DCaptProperties db 'Изменить',0
DCaptCancel	db 'Отменить',0

else
DTitleAdd	db 'Add icon',0
DTitleProp	db 'Change icon',0

DCaptName	db 'Name',0
DCaptPath	db 'Path',0
DCaptParams	db 'Parameters',0
DCaptIcon	db 'Icon',0
DCaptCreate	db 'Create',0
DCaptProperties db 'Change',0
DCaptCancel	db 'Cancel',0
end if

;/не менять положение
edtName    edit_box NAME_LENGTH*6+4,70+20+IMG_SIZE,6,0FFFFFFh,06F9480h,0FFh,0h,0,NAME_LENGTH,\
		DAreaName,mouse_dd,0,0,0
edtExePath edit_box 281-3-20-IMG_SIZE,70+20+IMG_SIZE,26,0FFFFFFh,06F9480h,0FFh,0h,0,255,\
		DAreaPath,mouse_dd,0,0,0
edtParams  edit_box 295-20-IMG_SIZE,70+20+IMG_SIZE,46,0FFFFFFh,06F9480h,0FFh,0h,0,255,\
		DAreaParams,mouse_dd,0,0,0
edtIcon    edit_box 295-20-IMG_SIZE,70+20+IMG_SIZE,66,0FFFFFFh,06F9480h,0FFh,0h,0,255,\
		DAreaIcon,mouse_dd,0,0,0
endEdits:
;\

sbIcons:
	     dw END_ICONS_AREAW-ICONSX
	     dw ICONSX
	     dw 15
	     dw END_ICONS_AREAH+3
	     dd 0
	     dd 1
 .max_area   dd 0
 .cur_area   dd ICONS_DRAW_COUNTW
 .position   dd 0
 .bckg_col   dd 0
 .frnt_col   dd 0
 .line_col   dd 0
 .redraw     dd 0
 .delta      dd 0
 .delta2     dd 0
 .r_size_x   dw 0
 .r_start_x  dw 0
 .r_size_y   dw 0

 .r_start_y  dw 0
 .m_pos      dd 0
 .m_pos2     dd 0
 .m_keys     dd 0
 .run_size   dd 0
 .position2  dd 0
 .work_size  dd 0
 .all_redraw dd 0
 .ar_offset  dd 0

;-------------------------------------------------------------------------------
OpenDialog_data:
.type			dd 0
.procinfo		dd RBProcInfo	    ;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_pach		dd temp_dir_pach	;+16
.dir_default_pach	dd communication_area_default_pach	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd DRedrawWin;draw_window_for_OD   ;+28
.status 		dd 0	;+32
.openfile_pach		dd DAreaPath;fname_Info   ;+36
.filename_area		dd 0;DAreaPath        ;+40
.filter_area		dd Filter
.x:
.x_size 		dw 420 ;+48 ; Window X size
.x_start		dw 100 ;+50 ; Window X position
.y:
.y_size 		dw 320 ;+52 ; Window y size
.y_start		dw 100 ;+54 ; Window Y position

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_path:
	db '/sys/File managers/opendial',0

communication_area_default_pach:
	db '/sys',0

Filter	dd 0

;open_dialog     db 0
;-------------------------------------------------------------------------------

;/не разделять
align 4
IconAreaH	dd ICON_SIZE,ICON_SIZE
I_END:
;##### UDATA ###################################################################
IconArea	rb 4*ICON_SIZE*ICON_SIZE
;\

sc		system_colors
sc.workL	rd 1
sc.workH	rd 1

align 4

ScreenW 	rw 1
ScreenH 	rw 1
wsX		rw 1
wsY		rw 1
wsXe		rw 1
wsYe		rw 1
wsW		rw 1
wsH		rw 1


MouseX		rd 1
MouseY		rd 1

RBSlot		rd 1
AddX		rd 1
AddY		rd 1

SelIcon 	rd 1
DlgSelIcon	rd 1
slotDlgAdd	rd 1
DlgBufImg	rb IMG_SIZE*IMG_SIZE*3

align 4
bufStdIco	rb 40
IconsOffs	rd 100		;таблица с указателями на конкретные иконки(для ускорения)
PIcoDB		rd 1
nLoadIcon	rd 1		;номер читаемой из ini иконки
IconsID 	rd 100		;ID иконок - 2 байтa + байт 0 + выравнивающий байт - строка с 2мя шеснадцетиричными цифрами

nameSection	rb 4

align 4
icon_count	rd 1
strip_file	rd 1
strip_file_size rd 1

cur_band_compensation rd 1

;---- RButton -----------------------------------------------------------------------

bufIni		rb 40
NumMenuButt	rd 1
RBMenuCP	rd MAX_USER_BUTTONS*2	      ;указатели на подпись и путь с прогой (Caption(dd), Path(dd)) и запасной (dd) для завершающего нуля
RMenuW		rw 1
RMenuH		rw 1

MaxPage 	rd 1
mouse_dd	rd 1

DAreaName	rb NAME_LENGTH+1
DAreaPath	rb 255+1
DAreaParams	rb 255+1
DAreaIcon	rb 255+1

align 4
RBProcInfo	rb 1024
align 4


;------ OpenDialog -------------------------------
temp_dir_pach	rb 1024
fname_Info	rb 1024

;-------------------------------------------------------------------------------
		rb 512
stack_mov:			;одновременно таскать и держать открытым менюшку невозможно
stack_rmenu:
		rb 512
stack_dlg:
		rb 512
stack_bredraw:
		rb 512
stack_main:
;------------------------------------------------------------------------------
ENDMEM: