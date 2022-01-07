;---------------------------------------------------------------------
; Kpack - Kolibri Packer
; Kolibri version
; Written by diamond in 2006, 2007 specially for KolibriOS
;
; Disassemled and corrected in 2010-2011 specially for FASM
;            by Marat Zakiyanov aka Mario79, aka Mario
;
; Uses LZMA compression library by Igor Pavlov
; (for more information on LZMA and 7-Zip visit http://www.7-zip.org)
; (plain-C packer and ASM unpacker are ported by diamond)
;---------------------------------------------------------------------
use32
	org	0

	db 'MENUET01'
	dd 1
	dd START
	dd IM_END
	dd I_END
	dd stacktop
	dd params
	dd cur_dir_path
;---------------------------------------------------------------------
include '../../../config.inc'		;for nightbuild
include '../../../KOSfuncs.inc'
include '../../../macros.inc'
include '../../../gui_patterns.inc'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../../load_lib.mac'
  @use_library

START:
	mcall	SF_SYS_MISC,SSF_HEAP_INIT
	mcall	SF_SET_EVENTS_MASK,0x80000027

	load_libraries l_libs_start,load_lib_end
	cmp eax,-1
	je exit

	init_checkboxes2 check1,check1_end
	call	clear_messages

; pack kernel ?
	cmp	[params], dword '-ker'
	jne @f

	mov	esi,kernel_name
	mov	edi,inname
	call	copy_1

	mov	esi,kernel_name
	mov	edi,outname
	call	copy_1
	
	mov	esi,defpath
	mov	edi,path
	call	copy_1

	call	pack
	jmp exit

@@:
; set default path = /SYS/
	mov	esi,defpath
	mov	edi,path
	mov	[edi-4],dword 6
	movsw
	movsd
; get system window info
	xor	eax,eax
	cmp	[params],al
	je	default
	
	mov	edi,path
	mov	esi,params
	call	copy_1

	sub	esi,2
	std
@@:
	lodsb
	dec	edi
	cmp	al,byte '/'
	jnz	@r
	
	mov	[edi-1],byte 0
	mov	edi,inname
	add	esi,2
	push	esi
	call	copy_1
	pop	esi
	mov	edi,outname
	call	copy_1
end_param:
;---------------------------------------------------------------------
	call set_editbox_position_all
;---------------------------------------------------------------------
	call	draw_window
	call	pack
	jmp	OD_initialization
;*********************************************************************
default:
	mov	esi,definoutname
	mov	edi,esi
	xor	ecx,ecx
	xor	eax,eax
	dec	ecx
	repnz	scasb
	not	ecx
	dec	ecx
	mov	[innamelen],ecx
	push	ecx
	push	esi
	mov	edi,inname
	rep	movsb
	pop	esi
	pop	ecx
	mov	[outnamelen],ecx
	mov	edi,outname
	rep	movsb
;---------------------------------------------------------------------
	call	set_editbox_position_all
;---------------------------------------------------------------------
OD_initialization:
;OpenDialog	initialisation
	push    dword OpenDialog_data
	call    [OpenDialog_Init]
;---------------------------------------------------------------------
red:
	call	draw_window
;--------------------------------------
still:
	mcall	SF_WAIT_EVENT
	dec	eax
	jz	red

	dec	eax
	jz	key

	dec	eax
	jz	button

	push	dword edit1
	call	[edit_box_mouse]

	push	dword edit2
	call	[edit_box_mouse]

	push	dword edit3
	call	[edit_box_mouse]

	push	dword check1
	call	[check_box_mouse]
	
	jmp	still
;*********************************************************************
tell_compress_mess:
	push	compressing_len
	pop	ecx
	mov	esi,compressing_str
	call	write_string
	ret
;*********************************************************************
clear_mess_and_displogo:
	call	refresh_editbox_data
; clear messages
	call	clear_messages
; display logo
	mov	esi,info_str
	push	info_len
	pop	ecx
	call	write_string
	ret
;*********************************************************************
clear_messages:
	xor	eax,eax
	mov	ecx,80*20/4+1
	mov	edi,message_mem
	rep	stosd
	ret
;*********************************************************************
exit:
	xor	eax,eax
	dec	eax
	mcall	;SF_TERMINATE_PROCESS
;*********************************************************************
button:
; button pressed
	mcall	SF_GET_BUTTON
	xchg	al,ah
	cmp	al,7
	jz	but7

	dec	eax
	jz	exit

	dec	eax
	jnz	nopack

	bt dword[check1.flags],1
	jc	@f

	call	pack
	jmp	still
;---------------------------------------------------------------------	
@@:
	call	kerpack
	jmp	still	
;---------------------------------------------------------------------
nopack:
	dec	eax
	jnz	nounpack
	
	call	unpack
	jmp	still
;---------------------------------------------------------------------	
but7:
	call	clear_messages
; display logo
	mov	esi,info_str
	push	info_len
	pop	ecx
	call	write_string
; display info
	mov	esi,usage_str
	mov	ecx,usage_len
	call	write_string
	jmp	still
;---------------------------------------------------------------------
nounpack:
	dec	eax
	jnz	still

	call	OpenDialog_start
	jmp	still
;*********************************************************************
OpenDialog_start:
	mov	esi,path
	mov	edi,temp_dir_pach
	call	copy_1	

	push    dword OpenDialog_data
	call    [OpenDialog_Start]
	cmp	[OpenDialog_data.status],1
	jne	@f

	mov	esi,filename_area
	mov	edi,inname
	call	copy_1

	mov	esi,filename_area
	mov	edi,outname
	call	copy_1
	
	mov	esi,temp_dir_pach
	mov	edi,path
	call	copy_1
	
	call	refresh_editbox_data

	call	set_editbox_position_all

	call	draw_editbox
@@:
	ret
;*********************************************************************
copy_1:
	xor	eax,eax
	cld	
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@r
	ret
;*********************************************************************
refresh_editbox_data:
	mov	esi,inname
	mov	edi,innamelen
	call	refresh_data

	mov	esi,outname
	mov	edi,outnamelen
	call	refresh_data

	mov	esi,path
	mov	edi,pathlen
	call	refresh_data

	ret
;*********************************************************************
refresh_data:
	push	esi
	xor	eax,eax
	cld	
@@:
	lodsb
	test	eax,eax
	jnz	@r
	pop	eax
	sub	esi,eax
	dec	esi
	mov	[edi],esi
	ret
;*********************************************************************
set_editbox_position_all:
	mov	ebx,inname
	mov	edi,edit1
	call	set_editbox_position

	mov	ebx,outname
	mov	edi,edit2
	call	set_editbox_position

	mov	ebx,path
	mov	edi,edit3
	call	set_editbox_position
	ret
;*********************************************************************
key:
	mcall	SF_GET_KEY

	push	dword edit1
	call	[edit_box_key]

	push	dword edit2
	call	[edit_box_key]

	push	dword edit3
	call	[edit_box_key]

	jmp	still
;*********************************************************************
get_full_name:
	push	esi
	mov	esi,path
	mov	ecx,[esi-4]
	mov	edi,fullname
	rep	movsb
	mov	al,'/'
	cmp	[edi-1],al
	jz	@f

	stosb
;--------------------------------------
@@:
	pop	esi
	cmp	[esi],al
	jnz	@f

	mov	edi,fullname
;--------------------------------------
@@:
	mov	ecx,[esi-4]
	rep	movsb
	xor	eax,eax
	stosb
	ret
;*********************************************************************
write_string:
; in: esi=pointer, ecx=length
	mov	edx,[message_cur_pos]
;--------------------------------------
x1:
	lea	edi,[message_mem+edx]
;--------------------------------------
do_write_char:
	lodsb
	cmp	al,10
	jz	newline

	stosb
	inc	edx
	loop	do_write_char
	jmp	x2
;---------------------------------------------------------------------
newline:
	xor	eax,eax
	stosb
	xchg	eax,edx
	push	ecx
	push	eax
	mov	ecx,80
	div	ecx
	pop	eax
	xchg	eax,edx
	sub	edx,eax
	add	edx,ecx
	pop	ecx
	loop	x1
;--------------------------------------
x2:
	mov	[message_cur_pos],edx
; update window
	call draw_log_area
;--------------------------------------
draw_messages:
	mov	ebx,12 shl 16 + LOG_Y + 7
	mov	edi,message_mem
;--------------------------------------
@@:
	push	edi
	xor	eax,eax
	push	80
	pop	ecx
	repnz	scasb
	sub	ecx,79
	neg	ecx
	mov	esi,ecx
	pop	edi
	mcall	SF_DRAW_TEXT,,0xB0000000,edi
	add	ebx,16
	add	edi,80
	cmp	edi,message_cur_pos
	jb	@b

	ret
;*********************************************************************
draw_log_area:
	DrawRectangle 5, LOG_Y, WIN_W-12, LOG_H, [sc.work_graph]
	mcall	SF_DRAW_RECT, <6,WIN_W-13>, <LOG_Y+1,LOG_H-1>, 0xFFFfff
	DrawRectangle3D 6, LOG_Y+1, WIN_W-13, LOG_H-1, 0xDEDEDE, [sc.work_graph] 
	ret
;*********************************************************************
draw_window:
; start redraw
	mcall	SF_REDRAW,SSF_BEGIN_DRAW	
	mcall	SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,40
	;--------------------------------------
	edit_boxes_set_sys_color edit1,editboxes_end,sc
	check_boxes_set_sys_color2 check1,check1_end,sc
	;--------------------------------------
; define window
	mcall	SF_STYLE_SETTINGS,SSF_GET_SKIN_HEIGHT

	mov	ecx,100 shl 16 + WIN_H
	add ecx, eax
	
	mov	edx,[sc.work]
	add	edx,34000000h
	xor	esi,esi
	xor	edi,edi
	mcall	SF_CREATE_WINDOW,<250,WIN_W+10>,,,,caption_str
	mcall	SF_THREAD_INFO,procinfo,-1
	
	mov	eax,[procinfo+70] ;status of window
	test	eax,100b
	jne	.end	
;--------------------------------------
; draw lines and frame
	call    draw_log_area
; draw buttons
	call	draw_buttons
; draw messages
	call	draw_messages
; draw editbox's
	call	draw_editbox
; end redraw
.end:
	mcall	SF_REDRAW,SSF_END_DRAW
	ret
;*********************************************************************
draw_editbox:
	push	dword edit1
	call	[edit_box_draw]
	
	push	dword edit2
	call	[edit_box_draw]
	
	push	dword edit3
	call	[edit_box_draw]
	
	mov eax,[sc.work_text]
	or eax, 0x90000000
	mov	[check1.text_color], eax
	push	dword check1
	call	[check_box_draw]
	ret
;*********************************************************************
set_editbox_position:
	mov	esi,ebx
	cld
@@:
	lodsb
	test	al,al
	jne	@r
	sub	esi,ebx
	mov	eax,esi
	dec	eax
	mov	[edi+48], eax  ;ed_size
	mov	[edi+52], eax  ;ed_pos
	ret
;*********************************************************************
draw_buttons:
; define compress button
	mov	cx,18
	mcall	SF_DEFINE_BUTTON,<WIN_W - RIGHT_BTN_W - 5, RIGHT_BTN_W>, <3, 20>,2,[sc.work_button]
; uncompress button
	inc	edx
	add	ecx,LINE_H shl 16
	mcall
; question button
	push	esi
	mov	dl,7
	mcall	,<WIN_W-25,18>,<LINE_H*2+3,18>
	mov ecx,[sc.work_button_text]
	or  ecx,0x90000000
	mov edx,aQuestion
	mcall	SF_DRAW_TEXT,<WIN_W-19, LINE_H*2+5>
	pop	esi
; define Path button
	mcall	SF_DEFINE_BUTTON,<6,64>,<LINE_H*2+3,20>,4
; text on Path button
	mov	ebx,8 shl 16+5
	mov	al,4
	mov	ecx,[sc.work_text]
	push	buttons1names
	pop	edx
	push	8
	pop	esi
;--------------------------------------
; text on settings buttons
	mov ecx, [sc.work_text]
	or ecx, 0x10000000
	mcall	, <8, 5>, , buttons1names, 8

	add	edx,esi
	add	ebx,LINE_H
	mcall
	add	edx,esi
	add	ebx,LINE_H
	mov	ecx,[sc.work_button_text]
	or ecx, 0x10000000
	sub ebx, 10 shl 16
	mcall
; text on compress and decompress buttons
	or	ecx,0x80000000
	mcall	,<WIN_W - RIGHT_BTN_W+7,6>,,aCompress
	mcall	,<WIN_W - RIGHT_BTN_W+7,LINE_H+6>,,aDecompress
	ret
;*********************************************************************
;Pack procedures
include 'packpoc.inc'
;---------------------------------------------------------------------
;UnPack procedures
include 'upacproc.inc'
;---------------------------------------------------------------------
;lzma_compress:
include 'lzma_compress.inc'
;---------------------------------------------------------------------
;lzma_set_dict_size:
include 'lzma_set_dict_size.inc'
;---------------------------------------------------------------------
;lzma_decompress:
include	'lzma_decompress.inc'
;---------------------------------------------------------------------
;kerpack code:
include	'kerpack.inc'
;---------------------------------------------------------------------
;initialized variables and constants
include 'const_var.inc'
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
;uninitialized data
include 'data.inc'
;---------------------------------------------------------------------
I_END:
;---------------------------------------------------------------------