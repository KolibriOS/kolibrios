; kpack = Kolibri Packer
;---------------------------------------------------------------------
; version:	0.20
; last update:  08/18/2011
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      Checking for "rolled up" window
;---------------------------------------------------------------------
; version:	0.20
; last update:  07/12/2010
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      Added code for packing the kernel.mnt
;---------------------------------------------------------------------
; version:	0.15
; last update:  06/11/2010
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      1) Window Y=4, B=1
;               2) Refresh lenght of data after Editbox editing
;               3) Changed format of start parameter -
;                    longer path (total length 255 + zero).
;---------------------------------------------------------------------
; version:	0.14
; last update:  03/11/2010
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      select path with OpenDialog,
;               using Box_Lib and Proc_Lib
;---------------------------------------------------------------------
; Kpack - Kolibri Packer
; Kolibri version
; Written by diamond in 2006, 2007 specially for KolibriOS
;
; Disassemled and corrected in 2010 specially for FASM
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
include '../../../macros.inc'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../../develop/libraries/box_lib/load_lib.mac'
  @use_library

START:
	mcall	68,11
	mcall	40,100111b

load_libraries l_libs_start,load_lib_end
	cmp eax,-1
	je exit

	init_checkboxes2 check1,check1_end
	;push	check1
	;call	[init_checkbox]

	call	clear_messages
; set default path = /RD/1/
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
;---------------------------------------------------------------------
	call	set_editbox_position_all
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
	mcall	10
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
	mcall
;*********************************************************************
button:
; button pressed
	mcall	17
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
	mcall	2

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
	mcall	13,<6,414>,<54,222>,[color_table+20]
;--------------------------------------
draw_messages:
	mov	ebx,12 shl 16+60
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
	mcall	4,,[color_table+32],edi
	add	ebx,10
	add	edi,80
	cmp	edi,message_cur_pos
	jb	@b

	ret
;*********************************************************************
draw_window:
; start redraw
	mcall	12,1
	
	mcall	48,3,color_table,40
;--------------------------------------
edit_boxes_set_sys_color edit1,editboxes_end,color_table
check_boxes_set_sys_color2 check1,check1_end,color_table
;--------------------------------------
; define window
	xor	eax,eax
	mov	ecx,100 shl 16+306
	mov	edx,[color_table+20]
	add	edx,34000000h
	xor	esi,esi
	xor	edi,edi
	mcall	,<100,436>,,,,caption_str
	mcall	9,procinfo,-1
	
	mov	eax,[procinfo+70] ;status of window
	test	eax,100b
	jne	.end	
;--------------------------------------
; draw lines and frame
	call	draw_lines
; draw buttons
	call	draw_buttons
; draw messages
	call	draw_messages
; draw editbox's
	mov	eax,[procinfo+42]
	sub	eax,65+72+10
	mov	[edit1.width],eax
	mov	[edit2.width],eax
	mov	[edit3.width],eax

	call	draw_editbox
; end redraw
.end:
	mcall	12,2
	ret
;*********************************************************************
draw_editbox:
	push	dword edit1
	call	[edit_box_draw]
	
	push	dword edit2
	call	[edit_box_draw]
	
	push	dword edit3
	call	[edit_box_draw]
	
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
draw_lines:
	mov	ecx,2 shl 16+12*3
; draw frame for messages data
	push	ecx
	add	ecx,50 shl 16+16
	mcall	38,<3,423>,,[color_table+36]
	add	ecx,224*(1 shl 16+1)
	mcall
	sub	cx,224
	mcall	,<3,3>
	mcall	,<423,423>
	pop	ecx
	ret
;*********************************************************************
draw_buttons:
; define compress button
	mov	cx,18
	mcall	8,<351,72>,<1, 17>,2,[color_table+24]
; uncompress button
	add	ecx,18 shl 16
	inc	edx
	mcall
	add	ecx,-12h+0Ah+140000h
; question button
	push	esi
	mov	dl,7
	mcall	,<414,9>
	shr	ecx,16
	lea	ebx,[ecx+1A10002h]
	mcall	4,,[color_table+28],aQuestion,1
	pop	esi
; define settings buttons
	mov	ecx,16*2+2
	shl	ecx,16
	mov	cx,13
	mcall	8,<6,50>,,4
; text on settings buttons
	mov	ebx,9 shl 16+5
	mov	al,4
	mov	ecx,[color_table+32]
	push	buttons1names
	pop	edx
	push	8
	pop	esi
;--------------------------------------
	mcall
	add	edx,esi
	add	ebx,16
	mcall
	add	edx,esi
	add	ebx,16
	mov	ecx,[color_table+28]
	mcall
; text on compress and decompress buttons
	or	ecx,80000000h
	mcall	,<364,6>,,aCompress
	mcall	,<359,24>,,aDecompress
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