;
;    UNIFORM WINDOW COLOURS & SKIN
;
;    Compile with FASM for Menuet
;
;    < russian edition by Ivan Poddubny >
;    < skin selection by Mike Semenyako >
;******************************************************************************
; last update:  01/04/2013
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      select colors with ColorDialog
;               some redesign of the look of the program
;******************************************************************************
; last update:  10/09/2010
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      select path with OpenDialog
;******************************************************************************
	use32
	org 0
	db 'MENUET01'	; identifier
	dd 1		; header version
	dd START	; start address
	dd IM_END	; file size
	dd I_END	; memory
	dd stacktop	; stack pointer
	dd app_param	; parameters
	dd cur_dir_path	; path to file
 
include 'lang.inc'
include '../../../proc32.inc'
include '../../../config.inc'		;for nightbuild
include '../../../macros.inc'
include '../../../string.inc'
include '../../../dll.inc'
include 'kglobals.inc'
include 'unpacker.inc'
include '../../../KOSfuncs.inc'
include '../../../load_lib.mac'
	@use_library
;******************************************************************************
;--------------------------------------
struct SKIN_HEADER
  ident   dd ?
  version dd ?
  params  dd ?
  buttons dd ?
  bitmaps dd ?
ends
;--------------------------------------
struct SKIN_PARAMS
  skin_height    dd ?
  margin.right   dw ?
  margin.left    dw ?
  margin.bottom  dw ?
  margin.top     dw ?
  colors.inner   dd ?
  colors.outer   dd ?
  colors.frame   dd ?
  colors_1.inner dd ?
  colors_1.outer dd ?
  colors_1.frame dd ?
  dtp.size       dd ?
  dtp.data       db 40 dup (?)
ends
;--------------------------------------
struct SKIN_BUTTONS
  type     dd ?
  pos:
    left   dw ?
    top    dw ?
  size:
    w  dw ?
    height dw ?
ends
;--------------------------------------
struct SKIN_BITMAPS
  kind  dw ?
  type  dw ?
  _data  dd ?
ends
;--------------------------------------
frame_1:
  .x      = 5
  .y      = area.y + area.height + 20
  .w  = area.w + 217
  .height = 65
;--------------------------------------  
frame_2:
  .x      = frame_1.x
  .y      = frame_1.y + frame_1.height + 20
  .w  = frame_1.w
  .height = frame_1.height
;---------------------------------------------------------------------
win:
  .w = frame_2.w + frame_2.x + frame_2.x + 9
  .h = frame_2.y + frame_2.height + 10
;---------------------------------------------------------------------
START:		; start of execution
;---------------------------------------------------------------------
	mcall	SF_SYS_MISC,SSF_HEAP_INIT
	
	test	eax,eax
	jz	close	

stdcall dll.Load,LibIniImportTable 
load_libraries l_libs_start,end_l_libs

;if return code =-1 then exit, else nornary work
	inc	eax
	test	eax,eax
	jz	close
;---------------------------------------------------------------------
; set default pathes
	stdcall string.copy, default_skin, skin_info
	stdcall string.copy, default_dtp, dtp_name
;---------------------------------------------------------------------
; check app param
	stdcall string.length, app_param
	add eax, app_param
	mov ecx, [eax-4]
	or ecx, 0x20202000 ;letters to lowercase
	cmp ecx, '.skn'
	je  load_skin_from_param
	cmp ecx, '.dtp'
	je load_dtp_from_param
	jmp no_param
	
load_dtp_from_param:
	stdcall string.copy, app_param, dtp_name
	call   load_dtp_file.1
	jmp    skin_path_ready

load_skin_from_param:
	stdcall string.copy, app_param, skin_info
	call    load_skin_file.2
	jmp     skin_path_ready

no_param:
	mcall	SF_STYLE_SETTINGS,SSF_GET_COLORS,color_table,4*10	; get current colors
	call	load_skin_file.2
	
skin_path_ready:	
;---------------------------------------------------------------------
;OpenDialog	initialisation
	push	dword OpenDialog_data
	call	[OpenDialog_Init]

	push	dword OpenDialog_data2
	call	[OpenDialog_Init]
;--------------------------------------------------------------------
;init_ColorDialog	ColorDialog_data
	push    dword ColorDialog_data
	call    [ColorDialog_Init]
;--------------------------------------------------------------------
; prepare for PathShow
	push	dword PathShow_data_1
	call	[PathShow_prepare]
	
	push	dword PathShow_data_2
	call	[PathShow_prepare]
;---------------------------------------------------------------------	
red:
	call	draw_window		; at first, draw the window
;---------------------------------------------------------------------	
still:
	mcall	SF_WAIT_EVENT

	dec	eax	; redraw request ?
	jz	red

	dec	eax	; key in buffer ?
	jz	key

	dec	eax	; button in buffer ?
	jz	button

	jmp	still
;---------------------------------------------------------------------
key:		; key
	mcall	SF_GET_KEY
	jmp	still
;---------------------------------------------------------------------
button:		; button
	mcall	SF_GET_BUTTON

 	cmp	ah,12	; load file
 	jne	no_load

	call	load_dtp_file
	call	draw_window
 	jmp	still
;--------------------------------------
no_load:
 	cmp	ah,13	; save file
 	jne	no_save

	call	save_file
 	jmp	still
;--------------------------------------
no_save:
 	cmp	ah,14	; set 3d buttons
 	jne	no_3d

	mcall	SF_STYLE_SETTINGS,SSF_SET_BUTTON_STYLE,1
	invoke  ini_set_int, aIni, aSectionSkn, aButtonStyle, 1
 	jmp	doapply
;--------------------------------------
no_3d:
 	cmp	ah,15	; set flat buttons
 	jne	no_flat

	invoke  ini_set_int, aIni, aSectionSkn, aButtonStyle, 0
	mcall	SF_STYLE_SETTINGS,SSF_SET_BUTTON_STYLE, 0
;--------------------------------------
doapply:
	mcall	SF_STYLE_SETTINGS,SSF_APPLY, 0
 	jmp	still
;--------------------------------------
no_flat:
 	cmp	ah,16	; apply
 	jne	no_apply
;--------------------------------------
apply_direct:
	mcall	SF_STYLE_SETTINGS,SSF_SET_COLORS,color_table,10*4
 	jmp	doapply
;--------------------------------------
 no_apply:
 	cmp	ah,17	; load skin file
 	jne	no_load_skin

	call	load_skin_file
	call	draw_window
 	jmp	still
;--------------------------------------
no_load_skin:
 	cmp	ah,18	; apply skin
 	jne	no_apply_skin

 	cmp	[skin_info],0
 	je	no_apply_skin

	mcall	SF_STYLE_SETTINGS,SSF_SET_SKIN,skin_info
	call	draw_window
 	jmp	still
;--------------------------------------
no_apply_skin:
	cmp	ah,31
	jb	no_new_colour

	cmp	ah,41
	jg	no_new_colour
	
;---------------------------------------------------------------------	
.start_ColorDialog:
	push    dword ColorDialog_data
	call    [ColorDialog_Start]
; 2 - use another method/not found program
	cmp	[ColorDialog_data.status],2
	je	still
; 1 - OK, color selected	
	cmp	[ColorDialog_data.status],1
	jne	still
;---------------------------------------------------------------------	
	
	shr	eax,8
	sub	eax,31
	shl	eax,2
	mov	ebx,[ColorDialog_data.color]
	and	ebx,0xffffff	; temporary for ColorDialog!!!!!!!!!!
	mov	[eax+color_table],ebx
 	cmp	dword[not_packed_area+SKIN_HEADER.ident],'SKIN'
 	jne	@f

	mov	edi,[not_packed_area+SKIN_HEADER.params]
	mov	dword[edi+not_packed_area+SKIN_PARAMS.dtp.data+eax],ebx
	call	draw_skin
;--------------------------------------
@@:
	call	draw_colours
 	jmp	still
;--------------------------------------
no_new_colour:
	cmp	ah,1	; terminate
	jnz	noid1
;--------------------------------------
close:
	or	eax,-1
	mcall
;--------------------------------------
noid1:
 	jmp	still
;---------------------------------------------------------------------
load_dtp_file:
;---------------------------------------------------------------------
; invoke OpenDialog
	mov	[OpenDialog_data.type],dword 0
	push	dword OpenDialog_data
	call	[OpenDialog_Start]
	cmp	[OpenDialog_data.status],1
	je	.1
	ret
.1:
; prepare for PathShow
	push	dword PathShow_data_1
	call	[PathShow_prepare]

	call	draw_PathShow
;---------------------------------------------------------------------
.2:
	xor	eax, eax
	mov	ebx, read_info
	mov	dword [ebx], eax	; subfunction: read
	mov	dword [ebx+4], eax	; offset (low dword)
	mov	dword [ebx+8], eax	; offset (high dword)
	mov	dword [ebx+12], 40     ; read colors file: 4*10 bytes
	mov	dword [ebx+16], color_table ; address
	mcall	SF_FILE
	ret
;---------------------------------------------------------------------
load_skin_file:
;---------------------------------------------------------------------
; invoke OpenDialog
	push	dword OpenDialog_data2
	call	[OpenDialog_Start]
	cmp	[OpenDialog_data2.status],1
	je	.1
	ret
.1:
; prepare for PathShow
	push	dword PathShow_data_2
	call	[PathShow_prepare]

	call	draw_PathShow
;---------------------------------------------------------------------
.2:
	xor	eax,eax
	mov	ebx,read_info2
	mov	dword [ebx], eax	; subfunction: read
	mov	dword [ebx+4], eax	; offset (low dword)
	mov	dword [ebx+8], eax	; offset (high dword)
	mov	dword [ebx+12], 32*1024 ; read: max 32 KBytes
	mov	dword [ebx+16], file_load_area ; address
	mcall	SF_FILE

	mov	esi, file_load_area

	cmp	dword [esi], 'KPCK'
	jnz	notpacked

	cmp	dword [esi+4], 32*1024 ; max 32 KBytes
	ja	doret

	push	unpack_area
	push	esi
	call	unpack
	mov	esi,unpack_area
;--------------------------------------
notpacked:
 	cmp	[esi+SKIN_HEADER.ident],dword 'SKIN'
 	jne	doret

	mov	edi,not_packed_area
	mov	ecx,0x8000/4
	rep	movsd

	mov	ebp,not_packed_area
	mov	esi,[ebp+SKIN_HEADER.params]
	add	esi,ebp
	lea	esi,[esi+SKIN_PARAMS.dtp.data]
	mov	edi,color_table
	mov	ecx,10
	rep	movsd
;--------------------------------------
doret:
	ret
;---------------------------------------------------------------------
save_file:
;---------------------------------------------------------------------
; invoke OpenDialog
	mov	[OpenDialog_data.type],dword 1
	push	dword OpenDialog_data
	call	[OpenDialog_Start]
	cmp	[OpenDialog_data.status],1
	je	.1
	ret
.1:
; prepare for PathShow
	push	dword PathShow_data_1
	call	[PathShow_prepare]

	call	draw_PathShow
;---------------------------------------------------------------------
	xor	eax,eax
	mov	ebx,write_info
	mov	[ebx],dword 2			; subfunction: write
	and	[ebx+4],eax			; (reserved)
	and	[ebx+8],eax			; (reserved)
	mov	[ebx+12],dword 10*4		; bytes to write
	mov	[ebx+16],dword color_table	; address
	mcall	SF_FILE
	ret
;---------------------------------------------------------------------
draw_button_row:
	mov	edx,0x40000000 + 31		; BUTTON ROW
	mov	ebx,(area.w+18)*65536+29
	mov	ecx,9*65536+15
	mov	eax,8
;-----------------------------------
.newb:
	mcall
	add	ecx,22*65536
	inc	edx
	cmp	edx,0x40000000 + 40
	jbe	.newb
	ret
;---------------------------------------------------------------------
draw_button_row_of_texts:
	mov	ebx,(area.w+49)*65536+9	; ROW OF TEXTS
	mov	ecx,[w_work_text]
	add ecx,0x10000000
	mov	edx,text
	mov	esi,32
	mov	eax,4
;-----------------------------------
.newline:
	mcall
	add	ebx,22
	add	edx,32
	cmp	[edx],byte 'x'
	jne	.newline
	ret
;---------------------------------------------------------------------
draw_colours:
	pusha
	mov	esi,color_table
	mov	ebx,(area.w+19)*65536+28
	mov	ecx,10*65536+14
	mov	eax,13
	mov	[frame_data.draw_text_flag],dword 0
;--------------------------------------
newcol:
	mov	edx,[esi]
	mcall

	push	ebx ecx

	sub	ebx,2 shl 16
	add	bx,4
	sub	ecx,2 shl 16
	add	cx,4
	
	mov	[frame_data.x],ebx
	mov	[frame_data.y],ecx	

	push	dword frame_data
	call	[Frame_draw]

	pop	ecx ebx

	add	ecx,22*65536
	add	esi,4
	cmp	esi,color_table+4*9
	jbe	newcol

	popa
	ret
;----------------------------------------------------------------------
draw_PathShow:
	pusha
	mcall	SF_DRAW_RECT,<frame_1.x+10,frame_1.w-25>,<frame_1.y+16,15>,0xffffff
	mcall	SF_DRAW_RECT,<frame_2.x+10,frame_2.w-25>,<frame_2.y+16,15>,0xffffff
; draw for PathShow
	push	dword PathShow_data_1
	call	[PathShow_draw]
	
	push	dword PathShow_data_2
	call	[PathShow_draw]
	popa
	ret
;---------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:
	mcall	SF_REDRAW,SSF_BEGIN_DRAW
	mcall	SF_STYLE_SETTINGS,SSF_GET_COLORS,app_colours,10*4
	mcall	SF_GET_SCREEN_SIZE
	mcall	SF_STYLE_SETTINGS,SSF_GET_SKIN_HEIGHT
	mov	[current_skin_high],eax
; DRAW WINDOW
	xor	eax,eax		; function 0 : define and draw window
	xor	esi,esi
	mov	edx,[w_work]	; color of work area RRGGBB,8->color
	or	edx,0x34000000
	mov	ecx,50 shl 16 + win.h
	add	ecx,[current_skin_high]
	mcall	,<110, win.w>,,,,title

	mcall	SF_THREAD_INFO,procinfo,-1
	
	mov	eax,[procinfo+70] ;status of window
	test	eax,100b
	jne	.end

;if lang eq ru
  load_w  = (5*2+6*9)
  save_w  = (5*2+6*9)
  flat_w  = (5*2+6*7)
  apply_w = (5*2+6*9)
;else
;  load_w  = (5*2+6*6)
;  save_w  = (5*2+6*8)
;  flat_w  = (5*2+6*4)
;  apply_w = (5*2+6*7)
;end if
;-----------------------------------
; select color DTP frame
; LOAD BUTTON	; button 12
	mcall	SF_DEFINE_BUTTON,<frame_1.x+10,load_w>,<frame_1.y+38,18>,12,[w_work_button]
; SAVE BUTTON
	add	ebx,(load_w+2)*65536-load_w+save_w
	inc	edx
	mcall		; button 13
; APPLY BUTTON
	mov	ebx,(frame_1.x + frame_1.w - apply_w - 15)*65536+apply_w
	mcall	SF_DEFINE_BUTTON,,,16	; button 17
; select color DTP button text
	mcall	SF_DRAW_TEXT,<frame_1.x+16,frame_1.y+44>,[w_work_button_text],t1,t1.size
;-----------------------------------	
; select skin frame	
; LOAD SKIN BUTTON	; button 17
	mcall	SF_DEFINE_BUTTON,<frame_2.x+10,load_w>,<frame_2.y+38,18>,17,[w_work_button]
; 3D
	mov	ebx,(frame_2.x+155)*65536+34
	mcall	,,,14	; button 14
; FLAT
	add	ebx,36*65536-34+flat_w
	inc	edx
	mcall		; button 15
; APPLY SKIN BUTTON
	mov	ebx,(frame_2.x + frame_2.w - apply_w -15)*65536+apply_w
	mcall	,,,18		; button 18
; select skin button text
	mcall	SF_DRAW_TEXT,<frame_2.x+16,frame_2.y+44>,[w_work_button_text],t2,t2.size
;-----------------------------------		
	call	draw_button_row
	call	draw_button_row_of_texts
	call	draw_colours
;-----------------------------------
	mov	[frame_data.x],dword frame_1.x shl 16+frame_1.w
	mov	[frame_data.y],dword frame_1.y shl 16+frame_1.height
	mov	[frame_data.text_pointer],dword select_dtp_text
	mov	eax,[w_work]
	mov	[frame_data.font_backgr_color],eax
	mov	eax,[w_work_text]
	mov	[frame_data.font_color],eax
	mov	[frame_data.draw_text_flag],dword 1
	
	push	dword frame_data
	call	[Frame_draw]
;-----------------------------------
	mov	[frame_data.x],dword frame_2.x shl 16+frame_2.w
	mov	[frame_data.y],dword frame_2.y shl 16+frame_2.height
	mov	[frame_data.text_pointer],dword select_skin_text

	push	dword frame_data
	call	[Frame_draw]
;-----------------------------------
	call	draw_PathShow
;-----------------------------------
	cmp	dword[not_packed_area+SKIN_HEADER.ident],'SKIN'
	jne	@f
	call	draw_skin
@@:
.end:
	mcall	SF_REDRAW,SSF_END_DRAW
	ret
;-----------------------------------------------------------------------------
include 'drawskin.inc'
;-----------------------------------------------------------------------------
; DATA AREA
;-----------------------------------------------------------------------------
include 'idata.inc'
;-----------------------------------------------------------------------------
IM_END:
;-----------------------------------------------------------------------------
include 'udata.inc'
;-----------------------------------------------------------------------------
I_END:
;-----------------------------------------------------------------------------