;********************************
;*                              *
;*     DESKTOP ICON MANAGER     *
;*                              *
;*  Compile with flat assembler *
;*                              *
;********************************
; version:	3.00
; last update:  02/04/2012
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      Program used only 2 threads: draw and mouse
;               Used new kernel functions: 25, 34, 15.8, 4 (redirect).
;               Used PNG icons with transparent.
;---------------------------------------------------------------------
; version:	2.11
; last update:  19/03/2012
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      The program uses only 140 Kb memory is now.
;               Some speedup.
;---------------------------------------------------------------------
; version:	2.1
; last update:  17/03/2012
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      Global optimization! The program uses
;               only 161 KB of memory instead of 603 kb is now.
;---------------------------------------------------------------------
; version:	2.02
; last update:  15/03/2012
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      some optimisations and code refactoring
;---------------------------------------------------------------------
; version:	2.01
; last update:  27/09/2011
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      deactivate the window after click
;              (working to kernel r.2244 and above)
;---------------------------------------------------------------------
; Many fix's and changes created by:
;               Halyavin, Diamond, Heavyiron, 
;               SPraid, Dunkaist
;---------------------------------------------------------------------
; version:	2.00
; last update:  22/02/2005
; changed by:   Willow and Mario79
; changes:      modified for work with new multi-thread ICON.
;******************************************************************************
ICON_SIZE equ 32*32*4
REC_SIZE equ 80
ICONS_DAT equ '/sys/icons.dat'
ICON_APP equ '/sys/icon'
ICON_STRIP equ '/rd/1/iconstrp.png'
;------------------------------------------------------------------------------
	use32
	org 0x0
	db 'MENUET01'	; 8 byte id
	dd 0x01		; header version
	dd START	; start of code
	dd IM_END	; size of image
	dd I_END	; memory for app
	dd stack_area	; esp
	dd I_Param	; boot parameters
	dd path		; path
;------------------------------------------------------------------------------
include 'lang.inc'
include '../../../macros.inc'
include '../../../develop/libraries/box_lib/load_lib.mac'
COLOR_ORDER equ MENUETOS
;include 'debug.inc'
;------------------------------------------------------------------------------
        @use_library    ;use load lib macros
;------------------------------------------------------------------------------
START:		; start of execution
	mcall	68,11
;---------------------------------------------------------------------	
load_libraries l_libs_start,end_l_libs
	test	eax,eax
	jnz	close
	
; unpack deflate
	mov	eax,[unpack_DeflateUnpack2]
	mov	[deflate_unpack],eax
;---------------------------------------------------------------------
; get size of file ICONSTRP.GIF
	mcall	70,finfo
	test	eax,eax
	jnz	close
; get memory for ICONSTRP.GIF
	mov	ecx,[procinfo+32]
	mov	[finfo.size],ecx
	mov	[img_size],ecx
	mcall	68,12
	mov	[finfo.point],eax
	mov	[image_file],eax
; load ICONSTRP.GIF
	mov	[finfo],dword 0
	mcall	70,finfo
	test	eax,eax
	jnz	close
; convert PNG to RAW
	xor	eax,eax
	mov	[return_code],eax

	push	image_file
	call	[cnv_png_import.Start]

	mov	eax,[raw_pointer]
	mov	eax,[eax+28]
	add	eax,[raw_pointer]
	mov	[strip_file],eax
; back memeory to system
	mcall	68,13,[finfo.point]
	
	mov	eax,[raw_pointer]
	mov	eax,[eax+8]
	shr	eax,5
	mov	[icon_count],eax
; load ICON.DAT
	call	load_ic
boot_str:
	cmp	[I_Param],dword 'BOOT'
	je	START2
;------------------------------------------------------------------------------	
; ICON EDITOR MODE - START1 entering label
;------------------------------------------------------------------------------	
START1:
	call	load_icon_list
;------------------------------------------------------------------------------
align 4	
red:
	call	draw_window	; at first, draw the window
	mov	esi,[current_icon]
	jmp	band
;------------------------------------------------------------------------------
align 4
still:
	mcall	10	; wait here for event
	dec	eax	; redraw request ?
	jz	red
	
	dec	eax	; key in buffer ?
	jz	key
;------------------------------------------------------------------------------
align 4
button:
	mcall	17	; get id
	shr	eax,8

	cmp	eax,1	; button id=1 ?
	je	close
	
	mov	esi,[current_icon]
	add	esi,12
	mov	ebx,[cur_band]
	
	cmp	eax,11
	jb	@f
	cmp	eax,13
	ja	@f
	call	read_string
	jmp	still
;--------------------------------------
align 4
@@:
	cmp	eax,21	; apply changes
	je	apply
	
	cmp	eax,22		; user pressed the 'add icon' button
	je	add_icon
	
	cmp	eax,23	; user pressed the remove icon button
	je	remove_icon

	
	cmp	eax,30	; left arrow to icons bar
	je	arrow.left
	
	cmp	eax,31	; right arrow to icons bar
	je	arrow.right
	
	cmp	eax,32
	je	ico

	cmp	eax,40	; user pressed button for icon position
	jae	press_button_for_icon_position
	
	jmp	still
;------------------------------------------------------------------------------
align 4
arrow:
.right:

	mov	eax,[icon_count]
	sub	eax,8
	cmp	ebx,eax
	jae	still	;.drwic2

	add	ebx,1
	jmp	.drwic2
;--------------------------------------
align 4
.left:
	test	ebx,ebx
	jz	still

	sub	ebx,1
;--------------------------------------
align 4
.drwic2:
	mov	[cur_band],ebx
;--------------------------------------
align 4
.drwic1:
	call	draw_icon
	jmp	still
;------------------------------------------------------------------------------
align 4
ico:
	push	ebx
	mcall	37,1
	pop	ebx
	shr	eax,16
	sub	eax,33-19
	mov	edi,34
	xor	edx,edx
	div	edi
	lea	ecx,[ebx+eax]
	cmp	ecx,[icon_count]
	jae	still

	mov	ecx,eax
	add	eax,ebx
	call	itoa
	jmp	arrow.drwic1
;------------------------------------------------------------------------------
align 4 
apply:
	; (1) save list
	mov	ebx,finfo
	mov	dword[ebx],2
	mov	edx,REC_SIZE
	imul	edx,dword [icons]
	mov	[ebx+12],edx
	mov	esi,iconlst
	call	lst_path
	mcall	70
	; (2) terminate all icons
	or	ecx,-1
	mcall	9,procinfo
	mov	edi,[ebx+30]
;--------------------------------------
align 4
newread2:
	mov	esi,1
;--------------------------------------
align 4
newread:
	inc	esi
	mov	ecx,esi
	mcall	9,procinfo

	cmp	edi,[ebx+30]
	je	newread

	cmp	esi,eax
	jg	.apply_changes

	mov	eax,[ebx+10]
	and	eax,not 20202020h
	cmp	eax,'@ICO'
	jz	@f
	cmp	eax,'ICON'
	jne	newread
;--------------------------------------
align 4
@@:
	xor	eax,eax
	cmp	eax,[ebx+42]
	jne	newread

	cmp	eax,[ebx+46]
	jne	newread

	mov	ecx,esi
	mcall	18,2

	jmp	newread2
;--------------------------------------
align 4
.apply_changes:
	mov	ebx, finfo_start
	mov	dword [ebx+8], boot_str+6
	mov	esi, iconname
	call	lst_path
	mcall	70
	mcall	68,1
	mcall	15,3
	jmp	still
;------------------------------------------------------------------------------
align 4
add_icon:
	mov	ebx,24*65536+250+8*14
	mcall	4,,0xc0ff0000,add_text,,0xffffff

	mcall	10
	cmp	eax,3
	jne	still

	mcall	17
	shr	eax,8
	cmp	eax,40
	jb	no_f
	mov	edi,eax
	sub	eax,40
	
	xor	edx,edx		; bcd -> 10
	mov	ebx,16
	div	ebx
; multiply x10
	shl	eax,1		; multiply x2
	lea	eax,[eax+eax*4] ; multiply x5
	add	eax,edx
	
	mov	ebx,eax
	add	ebx,icons_reserved
	cmp	[ebx],byte 'x'
	je	no_f
	mov	[ebx],byte 'x'
	
	mov	[cur_btn],edi
	xor	edx,edx
	mov	ebx,10
	div	ebx
	add	eax,65
	add	edx,65
	mov	[icon_default+0],dl
	mov	[icon_default+1],al
	
	inc	dword [icons]
	mov	edi,[icons]
	dec	edi
	imul	edi,REC_SIZE
	add	edi,icon_data
	
	mov	[current_icon],edi

	mov	esi,icon_default
	mov	ecx,REC_SIZE
	cld
	rep	movsb
	mov	esi,[current_icon]
	jmp	band
;--------------------------------------
align 4
no_f:
	call	draw_btns	;draw_window
	jmp	still
;------------------------------------------------------------------------------
align 4
remove_icon:
	mov	ebx,24*65536+250+8*14
	mcall	4,,0xc0ff0000,rem_text,,0xffffff
	
	mcall	10
	cmp	eax,3
	jne	no_f

	mcall	17
	shr	eax,8
	cmp	eax,40
	jb	red
	sub	eax,40
	
	xor	edx,edx
	mov	ebx,16
	div	ebx
; multiply x10
	shl	eax,1		; multiply x2
	lea	eax,[eax+eax*4] ; multiply x5
	add	eax,edx
	
	mov	ebx,eax
	add	ebx,icons_reserved
	cmp	[ebx],byte 'x'
	jne	red
	mov	[ebx],byte ' '
	
	xor	edx,edx
	mov	ebx,10
	div	ebx
	shl	eax,8
	mov	al,dl
	
	add	eax,65*256+65
	
	mov	esi,icon_data
	mov	edi,REC_SIZE
	imul	edi,[icons]
	add	edi,icon_data
;--------------------------------------
align 4 
news:
	cmp	word [esi],ax
	je	foundi
	add	esi,REC_SIZE
	cmp	esi,edi
	jb	news
	jmp	red
;--------------------------------------
align 4
foundi:
	mov	ecx,edi
	sub	ecx,esi
	
	mov	edi,esi
	add	esi,REC_SIZE
	
	cld
	rep	movsb
	
	dec	[icons]
	
	mov	eax,icon_data
	mov	[current_icon],eax
	movzx	ebx,word[eax]
	sub	bx,'AA'
	shl	bl,4
	shr	ebx,4
	add	ebx,40
	mov	[cur_btn],ebx
	jmp	red
;------------------------------------------------------------------------------
align 4
press_button_for_icon_position:
	mov	edi,eax
	sub	eax,40
	mov	edx,eax
	shl	eax,4
	and	edx,0xf
	mov	dh,ah
	add	edx,65*256+65
	mov	esi,icon_data
	mov	ecx,[icons]
	cld
;--------------------------------------
align 4
findl1:
	cmp	dx,[esi]
	je	foundl1
	add	esi,REC_SIZE
	loop	findl1
	jmp	still
;--------------------------------------
align 4
foundl1:
	mov	[current_icon],esi
	mov	[cur_btn],edi
;--------------------------------------
align 4
band:
	add	esi,12
	call	ASCII_to_icon_number
	and	eax,0xfffff8
	mov	[cur_band],eax
	call	draw_btns
	jmp	still
;------------------------------------------------------------------------------
align 4
print_strings:
	pusha
	mcall	13,<100,180>,<278+12,40>,0xffffff	; clear text area
	xor	edi,edi
	mov	eax,4		; icon text
	mov	ebx,100*65536+278+14
	mov	ecx,3
;--------------------------------------
align 4
.ll:
	push	ecx
	mov	ecx,0x000000
	mov	edx,[current_icon]
	add	edx,[positions+edi*4]
	movzx	esi,byte[str_lens+edi]
	inc	edi
	mcall
	add	ebx,14
	pop	ecx
	loop	.ll

	popa
	ret
;------------------------------------------------------------------------------
align 4
load_icon_list:
	mov	edi,icons_reserved	; clear reserved area
	mov	eax,32
	mov	ecx,10*9
	cld
	rep	stosb

	mov	ecx,[icons]	; set used icons to reserved area
	mov	esi,icon_data
;--------------------------------------
align 4
ldl1:
	movzx	ebx,byte [esi+1]
	sub	ebx,65
; multiply x10
	shl	ebx,1		; multiply x2
	lea	ebx,[ebx+ebx*4] ; multiply x5
	movzx	eax,byte [esi]
	add	ebx,eax
	sub	ebx,65
	add	ebx,icons_reserved
	mov	[ebx],byte 'x'
	add	esi,REC_SIZE
	loop	ldl1
	ret
;------------------------------------------------------------------------------
align 4
lst_path:
	mov	ecx,30
	mov	edi,finfo.path
	rep	movsb
	ret
;------------------------------------------------------------------------------
align 4
load_ic:
	mov	ebx,finfo
	mov	dword[ebx+12],48*REC_SIZE
	mov	dword[ebx+16],icon_data
	mov	esi,iconlst
	call	lst_path
	mcall	70
	lea	eax,[ebx+10]
	xor	edx,edx
	mov	ebx,REC_SIZE
	div	ebx
	mov	[icons],eax
	ret
;------------------------------------------------------------------------------
align 4
read_string:
	pusha
	sub	eax,11
	movzx	ecx,byte[str_lens+eax]
	mov	[cur_str],ecx
	mov	eax,[positions+eax*4]
	mov	edi,[current_icon]
	add	edi,eax
	mov	[addr],edi
	add	edi,ecx
;--------------------------------------
align 4
.l1:
	dec	edi
	cmp	byte[edi],' '
	jne	.found

	mov	byte[edi],'_'
	loop	.l1

	dec	edi
;--------------------------------------
align 4
.found:
	inc	edi
	push	edi
	call	print_strings
	pop	edi
;--------------------------------------
align 4
f11:
	mcall	10
	cmp	eax,2
	jz	fbu
	jmp	rs_done
;--------------------------------------
align 4
fbu:
	mcall	2
	shr	eax,8
	cmp	eax,13
	je	rs_done
	cmp	eax,8
	jnz	nobsl
	cmp	edi,[addr]
	jz	f11
	dec	edi
	mov	[edi],byte '_'
	call	print_strings
	jmp	f11
;--------------------------------------
align 4
nobsl:
	cmp	eax,31
	jbe	f11
	mov	[edi],al
	call	print_strings
	inc	edi
	mov	esi,[addr]
	add	esi,[cur_str]
	cmp	esi,edi
	jnz	f11
;--------------------------------------
align 4
rs_done:
	mov	ecx,[addr]
	add	ecx,[cur_str]
	sub	ecx,edi
	mov	eax,32
	cld
	rep	stosb
	call	print_strings
	popa
	ret
;------------------------------------------------------------------------------
align 4
key:
	mcall	2	; just read it and ignore
	jmp	still
;------------------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
align 4
draw_window:
	mcall	12,1
	; DRAW WINDOW
	xor	eax,eax
	xor	esi,esi
	mcall	,<210,300>,<30,(390-14)>,0x14ffffff,,title

	mcall	13,<20,260>,<35,200>,0x3366cc	; WINDOW AREA

	mcall	38,<150,150>,<35,235>,0xffffff	; VERTICAL LINE ON WINDOW AREA

	mcall	,<20,280>,<135,135>	; HOROZONTAL LINE ON WINDOW AREA

	mcall	8,<20,72>,<(275+1+14),(13-2)>,11,[bcolor]	;id 11 TEXT ENTER BUTTONS
	
	inc	edx
	add	ecx,14*65536
	mcall			; id 12
	
	inc	edx
	add	ecx,14*65536
	mcall			; id 13

	mcall	,<20,259>,<(329+2),(15-4)>,21	; id 21 APPLY AND SAVE CHANGES BUTTON

	add	ecx,14*65536
	inc	edx
	mcall	,<20,(129-2)>	; id 22 ADD ICON BUTTON

	add	ebx,(130+2)*65536
	inc	edx
	mcall			; id 23 REMOVE ICON BUTTON

	mcall	,<(20-14),8>,<(260-23),32>,30 + 1 shl 30	; id 30 IMAGE BUTTON

	inc	edx
	add	ebx,(36*7+26) shl 16
	mcall		; id 31

	add	edx,1 + 1 shl 29
	mcall	,<(33-19),(34*8)>	; id 32
	
	mcall	4,<(23-15),(273-24)>,0,arrows,1

	add	ebx,(36*7+27)shl 16
	add	edx,2
	mcall

	dec	edx
	mcall	,<120,250>

	lea	edx,[ebx+8 shl 16]
	mcall	47,0x30000,[icon_count],,0

	mov	eax,4
	mov	ebx,24 shl 16+(250+14+14+14)
	mov	ecx,0xffffff
	mov	edx,text
	mov	esi,47
;--------------------------------------
align 4
.newline:
	mov	ecx,[edx]
	add	edx,4
	mcall
	add	ebx,14
	add	edx,47
	cmp	[edx],byte 'x'
	jne	.newline
;--------------------------------------
align 4
draw_btns:
	mov	eax,0	; DRAW BUTTONS ON WINDOW AREA
	mov	ebx,20 shl 16+25
	mov	ecx,35 shl 16+19
	mov	edi,icon_table
	mov	edx,40
;--------------------------------------
align 4
newbline:
	cmp	[edi],byte 'x'
	jne	no_button

	mov	esi,0x5577cc
	cmp	[edi+90],byte 'x'
	jne	nores

	mov	esi,0xcc5555
	cmp	edx,[cur_btn]
	jne	nores

	mov	esi,0xe7e05a
;--------------------------------------
align 4
nores:
	push	eax
	mcall	8
	pop	eax
;--------------------------------------
align 4
no_button:
	add	ebx,26 shl 16

	inc	edi
	inc	edx
	inc	al
	cmp	al,9
	jbe	newbline
	
	mov	al,0
	add	edx,6
	ror	ebx,16
	mov	bx,20
	ror	ebx,16
	add	ecx,20 shl 16
	inc	ah
	cmp	ah,8
	jbe	newbline

	call	print_strings
	call	draw_icon
	mcall	12,2
	ret
;------------------------------------------------------------------------------
align 4
draw_icon:
	mcall	13,<(33-20),(34*8+2)>,<(260-24),(37+15-2)>,0xffffff
	mov	esi,[current_icon]
	add	esi,12
	call	ASCII_to_icon_number
	push	eax
	cmp	eax,[cur_band]
	jb	.nou
	sub	eax,[cur_band]
	cmp	eax,7
	ja	.nou
	imul	eax,34 shl 16
	lea	ebx,[eax+(33-19) shl 16]
	mov	bx,34
	mcall	13,,<(236+35),3>,0xff0000
	mov	eax,[esp]
;--------------------------------------
align 4
.nou:
	mov	eax,[cur_band]
	push	eax
	imul	eax,ICON_SIZE
	mov	ebx,[strip_file]
	add	ebx,eax
	mov	ecx,8
	mov	edx,(33-18) shl 16+238
;--------------------------------------
align 4
.nxt:
	push	ecx
	pusha
	mov	ebp,0
	mcall	65,,<32,32>,,32
	popa
	pop	ecx
	add	ebx,ICON_SIZE
	add	edx,34 shl 16
	loop	.nxt

	mcall	4,<45,280-2>,0,rep_text,rep_text_len-rep_text
	lea	edx,[ebx+(8*5)shl 16]
	pop	ecx
	mcall	47,0x30000,,,0xff
	
	add	ecx,7
	add	edx,(3*8+4)shl 16
	mcall
	
	add	edx,(5*8+4)shl 16
	mcall	,,[icon_count]
	
	pop	ecx
	add	edx,(10*8+4)shl 16
	mcall	,,,,0xff0000
	ret
;------------------------------------------------------------------------------
align 4
close:
	or	eax,-1
	mcall
;------------------------------------------------------------------------------
; ICON PROCESSING MODE - START2 entering label
;------------------------------------------------------------------------------
align 4
START2:
	mcall	40,10000b	; only Event 5 - draw background
	
	mcall	51,1,START_mouse_thread,stack_mouse_thread
;------------------------------------------------------------------------------
align 4
still2:
	mcall	10

	mcall	15,8

	mov	ecx,eax
	shr	ecx,16
	mov	[x_left],ecx
	and	eax,0xffff
	mov	[x_right],eax

	mov	ecx,ebx
	shr	ecx,16
	mov	[y_top],ecx
	and	ebx,0xffff
	mov	[y_bottom],ebx

	call	get_bg_info

	mov	ecx,[icons]
	mov	ebx,icon_data
;--------------------------------------
align 4
.start_new:
	push	ebx ecx
	mov	[adress_of_icon_data],ebx
	mov	eax,[ebx]
	call	calc_icon_pos
	mov	[current_X],ebx
	mov	[current_Y],eax

	call	draw_picture
	pop	ecx ebx
	add	ebx,REC_SIZE
	dec	ecx
	jnz	.start_new
	jmp	still2
;------------------------------------------------------------------------------	
align 4
draw_picture:
	mov	eax,[current_X]
	cmp	eax,[x_right]
	ja	.exit

	add	eax,52
	cmp	eax,[x_left]
	jb	.exit

	mov	eax,[current_Y]
	cmp	eax,[y_bottom]
	ja	.exit

	add	eax,52
	cmp	eax,[y_top]
	jb	.exit
;--------------------------------------
align 4
@@:
	mov	edi,[adress_of_icon_data]	;[ebp+8]
	lea	esi,[edi+12]
	call	ASCII_to_icon_number
; protect for icon area RAW size limit
	cmp	eax,[icon_count]
	jbe	@f
;--------------------------------------
align 4
.exit:
	ret
;--------------------------------------
align 4
@@:
	push	eax
	mcall	68,12,52*52*4+8
	mov	[draw_area],eax
	mov	ebx,52
	mov	[eax],ebx
	mov	[eax+4],ebx
	pop	eax

	shl	eax,12		; multiply x4096
	add	eax,[strip_file]
	mov	esi,eax

	mov	edi,[draw_area]
	add	edi,(52-32)/2*4+8
	mov	ebx,32
	cld
;--------------------------------------
align 4	
.y:
	mov	ecx,32
	rep	movsd
	
	add	edi,(52-32)*4
	dec	ebx
	jnz	.y

	call	draw_text
	
	mov	edx,[current_X]	;[ebp+0]
	shl	edx,16
	add	edx,[current_Y]	;[ebp+4]
	mov	ebx,[draw_area]
	add	ebx,8
	mcall	25,,<52,52>
	mcall	68,13,[draw_area]
	ret
;------------------------------------------------------------------------------
align 4
draw_text:
	mov	esi,[adress_of_icon_data]	;[ebp+8]
	add	esi,3
	push	edi
	mov	edi,title
	mov	ecx,8/4
	cld
	rep	movsd
	pop	edi
	mov	eax,title
;--------------------------------------
align 4
news2:
	cmp	[eax],byte 33
	jb	founde
	inc	eax
	cmp	eax,title+8	;11
	jb	news2
;--------------------------------------
align 4
founde:
	sub	eax,title
	mov	[tl],eax
	
	mov	eax,[tl]
	lea	eax,[eax+eax*2]		; eax *= char_width/2
	shl	eax,16
	
	mov	ebx,27 shl 16+40
	sub	ebx,eax
	
	xor	ecx,ecx		; black shade of text
	or	ecx,0x08000000	; redirect the output to the user area
	add	ebx,1 shl 16	;*65536+1
	
	mov	edi,[draw_area]
	mcall	4,,,title,[tl]

	inc	ebx
	mcall

	add	ebx,1 shl 16
	mcall

	inc	ebx
	mcall

	sub	ebx,1 shl 16
	mcall

	dec	ebx
	sub	ebx,1 shl 16
	mcall

	sub	ebx,1 shl 16
	dec	ebx
	mcall

	dec	ebx
	add	ebx,1 shl 16
	mcall

	inc	ebx
	mov	ecx,0xffffff		; white text
	or	ecx,0x08000000	; redirect the output to the user area
	mcall
	ret
;------------------------------------------------------------------------------
align 4
ASCII_to_icon_number:
;
; in:
; esi - adress of icon ASCII text number (line start + 12)
;
; out:
; eax - number of icon
;
	push	esi
	xor	eax,eax
	xor	ebx,ebx
;--------------------------------------
align 4
.next:
	lodsb
	cmp	al,'0'
	jb	.done
	cmp	al,'9'
	ja	.done
	sub	eax,'0'
; multiply x10
	shl	ebx,1		; multiply x2
	lea	ebx,[ebx+ebx*4] ; multiply x5
	add	ebx,eax
	jmp	.next
;--------------------------------------
align 4
.done:
	pop	esi
	mov	eax,ebx
	ret
;------------------------------------------------------------------------------
align 4
itoa:
	add	esi,2
	mov	ebx,10
	mov	ecx,3
;--------------------------------------
align 4
.l0:
	xor	edx,edx
	div	ebx
	add	dl,'0'
	mov	[esi],dl
	dec	esi
	loop	.l0
	ret
;------------------------------------------------------------------------------
align 4
get_bg_info:
	mcall	39,1	; get background size
	mov	[bgrxy],eax

	mov	ebx,eax
	shr	eax,16
	and	ebx,0xffff
	mov	[bgrx],eax
	mov	[bgry],ebx
	
	mcall	48,5
	mov	[warea.by_x],eax
	mov	[warea.by_y],ebx
	
	mcall	14
	add	eax,0x00010001
	mov	[scrxy],eax
	ret
;------------------------------------------------------------------------------
align 4
calc_icon_pos:
; in:
; eax - dword [icon_position]
; out:
; eax - current Y
; ebx - current X
	push	eax
	movzx	eax,al
	sub	eax,'A'		;eax - number of letter
	cmp	eax,4
	jg	no_left

	shl	eax,6	;imul eax,64
	add	eax,16
	movzx	ebx,[warea.left]
	add	eax,ebx
	jmp	x_done
;--------------------------------------
align 4
no_left:
	sub	eax,9
	sal	eax,6	;imul eax,64
	sub	eax,16+52-1
	movzx	ebx,[warea.right]
	add	eax,ebx
;--------------------------------------
align 4
x_done:
	mov	ebx,eax
	pop	eax
	push	ebx
	movzx	eax,ah
	sub	eax,'A'		; eax - number of letter
	cmp	eax,4
	jg	no_up

	shl	eax,6		;imul eax,80
	add	eax,16
	movzx	ebx,[warea.top]
	add	eax,ebx
	jmp	y_done
;--------------------------------------
align 4
no_up:
	sub	eax,9
	shl	eax,6		;imul eax,80
	sub	eax,16-1
	movzx	ebx,[warea.bottom]
	add	eax,ebx
;--------------------------------------
align 4
y_done:
	pop	ebx
	ret
;------------------------------------------------------------------------------
include 'mouse.inc'
;------------------------------------------------------------------------------
include 'data.inc'
;------------------------------------------------------------------------------
IM_END:
;------------------------------------------------------------------------------
include 'dat_area.inc'
;------------------------------------------------------------------------------
I_END:
;------------------------------------------------------------------------------
