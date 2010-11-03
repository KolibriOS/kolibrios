; kpack = Kolibri Packer
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
	dd I_END		;memf
	dd stacktop
	dd params
	dd cur_dir_path
;---------------------------------------------------------------------
include '..\..\..\macros.inc'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../../develop/libraries/box_lib/load_lib.mac'
  @use_library

START:
	mcall	68,11
	mcall	40,100111b

	call	clear_messages
; set default path = /RD/1/
	mov	esi,defpath
	mov	edi,path
	mov	[edi-4],dword 6
	movsw
	movsd
; get system window info
	mcall	48,3,color_table,40
	inc	ebx
	mcall
	mov	[skinheight],eax
	jmp	default
; check command line
	mov	esi,params
	mov	[esi+100h],byte 0
;--------------------------------------
parse_opt:
	call	skip_spaces
	test	al,al
	jz	default

	mov	edi,inname
	call	copy_name
	test	al,al
	jz	outeqin

	mov	edi,outname
	call	copy_name
	test	al,al
	jnz	default
;--------------------------------------
doit:
	call	draw_window
	call	pack
	jmp	still
;---------------------------------------------------------------------
clear_messages:
	xor	eax,eax
	mov	ecx,80*20/4+1
	mov	edi,message_mem
	rep	stosd
	ret
;---------------------------------------------------------------------
exit:
	xor	eax,eax
	dec	eax
	mcall
;--------------------------------------
outeqin:
	mov	ecx,256/4+1
	mov	esi,inname-4
	mov	edi,outname-4
	rep	movsd
	jmp	doit
;---------------------------------------------------------------------
default:

load_libraries l_libs_start,load_lib_end
	cmp eax,-1
	je exit

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
;	mov	edi,fname_buf
;	mov	esi,path4
;	call	copy_1
	
;OpenDialog	initialisation
	push    dword OpenDialog_data
	call    [OpenDialog_Init]
;---------------------------------------------------------------------
	call	set_editbox_position_all
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

	jmp	still
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

	call	pack
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
	push    dword OpenDialog_data
	call    [OpenDialog_Start]
	cmp	[OpenDialog_data.status],1
	jne	@f

	mov	esi,filename_area
	mov	edi,inname
	call	copy_1
	sub	edi,inname
	mov	[innamelen],edi

	mov	esi,filename_area
	mov	edi,outname
	call	copy_1
	sub	edi,outname
	mov	[outnamelen],edi
	
	mov	esi,temp_dir_pach
	mov	edi,path
	call	copy_1
	sub	edi,path
	dec	edi
	mov	[pathlen],edi
	
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
onenter:
;	cmp	[curedit],inname
;	jnz	@f

	push	2
	pop	eax
	jmp	nounpack
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
	mov	ecx,[skinheight]
	shl	ecx,16
	add	ecx,3700DEh
	mcall	13,<9,417>,,[color_table+20]
;--------------------------------------
draw_messages:
	mov	ebx,[skinheight]
	add	ebx,3Ch+12*10000h
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
	mov	edi,[skinheight]
;--------------------------------------
; define window
	xor	eax,eax
	mov	ecx,100 shl 16+286
	add	ecx,edi
	mov	edx,[color_table+20]
	add	edx,13000000h
;	push	edi
	xor	esi,esi
	mcall	,<100,435>,,,,fullname	;temp_dir_pach	;caption_str
;	pop	edi
	mcall	9,procinfo,-1
;--------------------------------------
; draw lines and frame
	call	draw_lines
; draw buttons
	call	draw_bittons
; infile, outfile, path strings
;	call	draw_strings
; draw messages
	call	draw_messages
; draw editbox's
	mov	eax,[procinfo+42]
	sub	eax,65+72+10
	mov	[edit1.width],eax ; устанавливаем ширину текстовых полей
	mov	[edit2.width],eax
	mov	[edit3.width],eax

	call	draw_editbox
; end redraw
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
;	mov	edi,[skinheight]
; lines - horizontal
;	mov	ebx,8 shl 16+352
;	mov	ecx,edi
;	shl	ecx,16
;	or	ecx,edi
;	add	ecx,2 shl 16+2
;	mcall	38,,,[color_table+36]
;	mov	esi,3
;@@:
;	add	ecx,12 shl 16+12
;	mcall
;	dec	esi
;	jnz	@r
;--------------------------------------
; lines - vertical
;	sub	ecx,36 shl 16
;	mcall	,<8,8>
;	add	ebx,52 shl 16+52
;	mcall
;	add	ebx,292 shl 16+292
;	mcall
;--------------------------------------
	mov	edi,[skinheight]
	mov	ecx,edi
	shl	ecx,16
	or	ecx,edi
	add	ecx,2 shl 16+2+12*3
;	add	ecx,12*3
; draw frame for messages data
	push	ecx
	add	ecx,52 shl 16+16
	mcall	38,<8,425>,,[color_table+36]
	add	ecx,224*(1 shl 16+1)
	mcall
	sub	cx,224
	mcall	,<8,8>
	mcall	,<426,426>
	pop	ecx
	ret
;*********************************************************************
draw_bittons:
; define compress button
	mov	cx,18
	mcall	8,<354,72>,,2,[color_table+36]
; uncompress button
	add	ecx,18 shl 16
	inc	edx
	mcall
	add	ecx,-12h+0Ah+140000h
; question button
	push	esi
	mov	dl,7
	mcall	,<417,9>
	shr	ecx,16
	lea	ebx,[ecx+1A40002h]
	mcall	4,,[color_table+28],aQuestion,1
;	mov	al,8
	pop	esi
; define settings buttons
;	mov	ebx,9 shl 16+50
	lea	ecx,[edi+2]
	add	ecx,16*2
	shl	ecx,16
	mov	cx,13
;	push	4
;	pop	edx
;--------------------------------------
;@@:
;	mcall
;	add	ecx,12 shl 16
;	inc	edx
;	cmp	edx,6
;	jbe	@b
	mcall	8,<9,50>,,4
; text on settings buttons
	lea	ebx,[edi+5+0C0000h]
	mov	al,4
	mov	ecx,[color_table+28]
	push	buttons1names
	pop	edx
	push	8
	pop	esi
;--------------------------------------
@@:
	mcall
	add	edx,esi
	add	ebx,16
	cmp	[edx-6],byte ' '
	jnz	@b
; text on compress and decompress buttons
	lea	ebx,[edi+8+1720000h]
	or	ecx,80000000h
	mcall	,,,aCompress
	lea	ebx,[edi+1Ah+16A0000h]
	mcall	,,,aDecompress
	ret
;*********************************************************************
copy_name:
	lea	edx,[edi+256]
;--------------------------------------
@@:
	lodsb
	cmp	al,' '
	jbe	copy_name_done

	stosb
	cmp	edi,edx
	jb	@b
;--------------------------------------
@@:
	lodsb
	cmp	al,' '
	ja	@b
;--------------------------------------
copy_name_done:
	dec	esi
	sub	edx,256
	sub	edi,edx
	mov	[edx-4],edi
;--------------------------------------
skip_spaces:
	lodsb
	cmp	al,0
	jz	@f

	cmp	al,' '
	jbe	skip_spaces
;--------------------------------------
@@:
	dec	esi
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