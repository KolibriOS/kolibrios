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
	dd skin_info	; parameters
	dd cur_dir_path	; path to file
 
include 'lang.inc'
include '../../../config.inc'		;for nightbuild
include '../../../macros.inc'
include 'kglobals.inc'
include 'unpacker.inc'
include '../../../develop/libraries/box_lib/load_lib.mac'
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
    width  dw ?
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
  .y      = 220
  .width  = 420
  .height = 50
;--------------------------------------  
frame_2:
  .x      = 5
  .y      = 280
  .width  = 420
  .height = 50
;---------------------------------------------------------------------
START:		; start of execution
;---------------------------------------------------------------------
	mcall	68,11
	
	test	eax,eax
	jz	close	

load_libraries l_libs_start,end_l_libs

;if return code =-1 then exit, else nornary work
	inc	eax
	test	eax,eax
	jz	close
;---------------------------------------------------------------------
	mov	edi,filename_area
	mov	esi,start_temp_file_name
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b


	mov	edi,fname
	mov	esi,default_dtp
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b

;---------------------------------------------------------------------
	mov	edi,skin_info
	cmp	byte [edi], 0
	jne	skin_path_ready
	mov	esi,default_skin
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
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
	mcall	48,3,color_table,4*10	; get current colors
	call	load_skin_file.2
;---------------------------------------------------------------------	
red:
	call	draw_window		; at first, draw the window
;---------------------------------------------------------------------	
still:
	mcall	10	; wait here for event

	dec	eax	; redraw request ?
	jz	red

	dec	eax	; key in buffer ?
	jz	key

	dec	eax	; button in buffer ?
	jz	button

	jmp	still
;---------------------------------------------------------------------
key:		; key
	mcall	2	; just read it and ignore
	jmp	still
;---------------------------------------------------------------------
button:		; button
	mcall	17	; get id

 	cmp	ah,12	; load file
 	jne	no_load

	call	load_file
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

	mcall	48,1,1
 	jmp	doapply
;--------------------------------------
no_3d:
 	cmp	ah,15	; set flat buttons
 	jne	no_flat

	mcall	48, 1, 0
;--------------------------------------
doapply:
	mcall	48, 0, 0
 	jmp	still
;--------------------------------------
no_flat:
 	cmp	ah,16	; apply
 	jne	no_apply
;--------------------------------------
apply_direct:
	mcall	48,2,color_table,10*4
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

	mcall	48,8,skin_info
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
load_file:
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
	xor	eax, eax
	mov	ebx, read_info
	mov	dword [ebx], eax	; subfunction: read
	mov	dword [ebx+4], eax	; offset (low dword)
	mov	dword [ebx+8], eax	; offset (high dword)
	mov	dword [ebx+12], 40     ; read colors file: 4*10 bytes
	mov	dword [ebx+16], color_table ; address
	mcall	70
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
	mcall	70

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
	mcall	70
	ret
;---------------------------------------------------------------------
draw_button_row:
	mov	edx,0x60000000 + 31		; BUTTON ROW
	mov	ebx,220*65536+14
	mov	ecx,10*65536+14
	mov	eax,8
;-----------------------------------
.newb:
	mcall
	add	ecx,20*65536
	inc	edx
	cmp	edx,0x60000000 + 40
	jbe	.newb
	ret
;---------------------------------------------------------------------
draw_button_row_of_texts:
	mov	ebx,240*65536+13	; ROW OF TEXTS
	mov	ecx,[w_work_text]
	mov	edx,text
	mov	esi,32
	mov	eax,4
;-----------------------------------
.newline:
	mcall
	add	ebx,20
	add	edx,32
	cmp	[edx],byte 'x'
	jne	.newline
	ret
;---------------------------------------------------------------------
draw_colours:
	pusha
	mov	esi,color_table
	mov	ebx,220*65536+14
	mov	ecx,10*65536+14
	mov	eax,13
;--------------------------------------
newcol:
	mov	edx,[esi]
	mcall
	call	draw_rectangle
	add	ecx,20*65536
	add	esi,4
	cmp	esi,color_table+4*9
	jbe	newcol

	popa
	ret
;----------------------------------------------------------------------
draw_PathShow:
	pusha
	mcall	13,<frame_1.x+5,frame_1.width-15>,<frame_1.y+7,15>,0xffffff
	mcall	13,<frame_2.x+5,frame_2.width-15>,<frame_2.y+7,15>,0xffffff
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
	mcall	12,1
	mcall	48,3,app_colours,10*4
	mcall	14
	mcall	48,4
	mov	[current_skin_high],eax
; DRAW WINDOW
	xor	eax,eax		; function 0 : define and draw window
	xor	esi,esi
	mov	edx,[w_work]	; color of work area RRGGBB,8->color
	or	edx,0x34000000
	mov	ecx,50 shl 16 + 346
	add	ecx,[current_skin_high]
	mcall	,<110,440>,,,,title

	mcall	9,procinfo,-1
	
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
	mcall	8,<frame_1.x+10,load_w>,<frame_1.y+25,15>,12,[w_work_button]
; SAVE BUTTON
	add	ebx,(load_w+2)*65536-load_w+save_w
	inc	edx
	mcall		; button 13
; APPLY BUTTON
	mov	ebx,(frame_1.x + frame_1.width - apply_w - 15)*65536+apply_w
	mcall	8,,,16	; button 17
; select color DTP button text
	mcall	4,<frame_1.x+16,frame_1.y+29>,[w_work_button_text],t1,t1.size
;-----------------------------------	
; select skin frame	
; LOAD SKIN BUTTON	; button 17
	mcall	8,<frame_2.x+10,load_w>,<frame_2.y+25,15>,17,[w_work_button]
; 3D
	mov	ebx,(frame_2.x+155)*65536+34
	mcall	,,,14	; button 14
; FLAT
	add	ebx,36*65536-34+flat_w
	inc	edx
	mcall		; button 15
; APPLY SKIN BUTTON
	mov	ebx,(frame_2.x + frame_2.width - apply_w -15)*65536+apply_w
	mcall	,,,18		; button 18
; select skin button text
	mcall	4,<frame_2.x+16,frame_2.y+29>,[w_work_button_text],t2,t2.size
;-----------------------------------		
	call	draw_button_row
	call	draw_button_row_of_texts
	call	draw_colours
;-----------------------------------
	mov	ebx,frame_1.x shl 16+frame_1.width
	mov	ecx,frame_1.y shl 16+frame_1.height
	call	draw_rectangle

; select color DTP text
	mov	ecx,[w_work_text]
	and	ecx,0xffffff
	add	ecx,0x40000000
	mcall	4,<frame_1.x+10,frame_1.y-4>,,select_dtp_text,\
				select_dtp_text.size,[w_work]
;-----------------------------------
	mov	ebx,frame_2.x shl 16+frame_2.width
	mov	ecx,frame_2.y shl 16+frame_2.height
	call	draw_rectangle
	
; select skin text
	mov	ecx,[w_work_text]
	and	ecx,0xffffff
	add	ecx,0x40000000
	mcall	4,<frame_2.x+10,frame_2.y-4>,,select_skin_text,\
				select_skin_text.size,[w_work]
;-----------------------------------
	call	draw_PathShow
;-----------------------------------
	cmp	dword[not_packed_area+SKIN_HEADER.ident],'SKIN'
	jne	@f
	call	draw_skin
@@:
.end:
	mcall	12,2
	ret
;-----------------------------------------------------------------------------
include 'drawrect.inc'
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