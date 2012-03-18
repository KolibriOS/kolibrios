;********************************
;*                              *
;*     DESKTOP ICON MANAGER     *
;*                              *
;*  Compile with flat assembler *
;*                              *
;********************************
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
; last update:  22|02|2005
; changed by:   Willow and Mario79
; changes:      modified for work with new multi-thread ICON.
;******************************************************************************
ICON_SIZE equ 32*32*3
;RAW_SIZE equ ICON_SIZE*32	;350000
;GIF_SIZE equ 12*1024	;45000
REC_SIZE equ 80
ICONS_DAT equ '/sys/ICONS.DAT'
ICON_APP equ '/sys/ICON'
ICON_STRIP equ '/sys/ICONSTRP.GIF'
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
	dd 0x0		; path
;------------------------------------------------------------------------------
include '../../../macros.inc'
include 'lang.inc'
COLOR_ORDER equ MENUETOS
include 'gif_lite.inc'
;include 'debug.inc'
purge newline
;------------------------------------------------------------------------------
START:		; start of execution
	mcall	68,11
; get size of file ICONSTRP.GIF
	mcall	70,finfo
	test	eax,eax
	jnz	close
; get memory for ICONSTRP.GIF
	mov	ecx,[process_table+32]
	mov	[finfo.size],ecx
	mcall	68,12
	mov	[finfo.point],eax
; load ICONSTRP.GIF
	mov	[finfo],dword 0
	mcall	70,finfo
	test	eax,eax
	jnz	close
; calculate and get memory for RAW data	
	mov	esi,[finfo.point]
	movzx	eax,word [esi+6]	;x
	movzx	edi,word [esi+8]	;y
	imul	eax,edi		;x*y
	lea	edi,[eax+eax*2]	; multiply x3
	mov	ecx,edi
	mcall	68,12
	mov	[strip_file],eax
	mov	edi,eax
; convert GIF to RAW
	call	ReadGIF
; back memeory to system
	mcall	68,13,[finfo.point]

	mov	eax,dword[edi+4]
	shr	eax,5
	mov	[icon_count],eax
; load ICON.DAT
	call	load_ic
boot_str:
	cmp	[I_Param],dword 'BOOT'
	je	load_icon_list2
	call	load_icon_list
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

;	dps	"button id: "
;	dpd	eax
;	newline_1

	cmp	eax,1	; button id=1 ?
	je	close
;--------------------------------------
align 4
noclose:
	mov	esi,[current_icon]
	add	esi,12
	mov	ebx,[cur_band]	;eax
	cmp	eax,31	; right arrow to icons bar
	jne	.no_back

	add	ebx,8
	mov	eax,[icon_count]
	cmp	eax,ebx
	jae	.drwic2

	xor	ebx,ebx
	jmp	.drwic2
;--------------------------------------
align 4
.no_back:
	cmp	eax,30	; left arrow to icons bar
	jne	.no_side

	test	ebx,ebx
	jnz	.dec

	mov	ebx,[icon_count]
	and	ebx,0xfffffff8
	add	ebx,8
;--------------------------------------
align 4
.dec:
	sub	ebx,8
;--------------------------------------
align 4
.drwic2:
	mov	[cur_band],ebx
;--------------------------------------
align 4
.drwic1:
	call	draw_icon
	jmp	still
;--------------------------------------
align 4
.no_side:
	cmp	eax,32
	jne	.no_ico
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

	mov	[sel_icon1],eax
	mov	ecx,eax
	add	eax,ebx
	call	itoa
	jmp	.drwic1
;--------------------------------------
align 4
.no_ico:
	cmp	eax,11
	jb	no_str
	cmp	eax,13
	jg	no_str
	call	read_string
	jmp	still
;--------------------------------------
align 4 
no_str:
	cmp	eax,21	; apply changes
	jne	no_apply
	; (1) save list
	mov	ebx,finfo	; Change by spraid
	mov	dword[ebx],2
	mov	edx,REC_SIZE
	imul	edx,dword [icons]
	mov	[ebx+12],edx
	mov	esi,iconlst
	call	lst_path
	mcall	70
	; (2) terminate all icons
	or	ecx,-1
	mcall	9,process_table
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
	mcall	9,process_table
	cmp	edi,[ebx+30]
	je	newread

	cmp	esi,eax
	jg	apply_changes

	mov	eax,[ebx+10]
	and	eax,not 20202020h
	cmp	eax,'@ICO'
	jz	@f
	cmp	eax,'ICON'
	jne	newread
;--------------------------------------
align 4
@@:
	mov	eax,51
	cmp	eax,[ebx+42]
	jne	newread

	cmp	eax,[ebx+46]
	jne	newread

	mov	ecx,esi
	mcall	18,2
	jmp	newread2
;------------------------------------------------------------------------------
align 4
finfo_start:
	dd 7
	dd 0
.params	dd 0
	dd 0
	dd 0
	db 0
	dd finfo.path
;------------------------------------------------------------------------------
align 4
finfo:
	dd 5
	dd 0
	dd 0
.size	dd 0	;GIF_SIZE
.point	dd process_table	;gif_file
.path:
	db ICON_STRIP,0
	rb 31-($-.path)
;------------------------------------------------------------------------------
align 4
apply_changes:
	mov	ebx, finfo_start
	mov	dword [ebx+8], boot_str+6
	mov	esi, iconname
	call	lst_path
	mcall	70
	jmp	still
;------------------------------------------------------------------------------
align 4
no_apply:
	cmp	eax,22		; user pressed the 'add icon' button
	jne	no_add_icon

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
;--------------------------------------
align 4
no_add_icon:
	cmp	eax,23	; user pressed the remove icon button
	jne	no_remove_icon
	
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
;--------------------------------------
align 4
no_remove_icon:
	cmp	eax,40	; user pressed button for icon position
	jb	still
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
	call	atoi
	and	eax,0xfffff8
	mov	[cur_band],eax
	call	draw_btns
	jmp	still
;------------------------------------------------------------------------------
current_icon	dd icon_data
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
iconlst	db ICONS_DAT,0
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
positions	dd 3,16,47
str_lens	db 8,30,30
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
newline:
	mov	ecx,[edx]
	add	edx,4
	mcall
	add	ebx,14
	add	edx,47
	cmp	[edx],byte 'x'
	jne	newline
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
	cmp	ah,8	;9
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
	call	atoi
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
	and	eax,0xfffffff8
	push	eax
	imul	eax,ICON_SIZE
	mov	ebx,[strip_file]
	lea	ebx,[ebx+8+eax]
	mov	ecx,8
	mov	edx,(33-18) shl 16+238
;--------------------------------------
align 4
.nxt:
	push	ecx
	mcall	7,,<32,32>
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
; DATA AREA
bcolor dd 0x335599
;------------------------------------------------------------------------------
icon_table:
 times 4  db 'xxxx  xxxx'
 times 2  db '          '
 times 1  db '          '
 times 2  db 'xxxx  xxxx'
; times 1  db '          '
;------------------------------------------------------------------------------
icons_reserved:
	times 9  db '          '
;------------------------------------------------------------------------------
if lang eq ru
text:
	db 255,255,255,0,   '   íÖäëí                                       '
	db 255,255,255,0,   ' èêéÉêÄååÄ                                     '
	db 255,255,255,0,   ' èÄêÄåÖíêõ                                     '
	db 255,255,255,0,   '                 èêàåÖçàíú                     '
	db 255,255,255,0,   '      ÑéÅÄÇàíú              ìÑÄãàíú            '
	db 0,0,0,0,         'çÄÜåàíÖ çÄ èéáàñàû àäéçäà Ñãü êÖÑÄäíàêéÇÄçàü   '
	db                  'x' ; <- END MARKER, DONT DELETE

add_text	db 'çÄÜåàíÖ çÄ èéáàñàû çÖàëèéãúáìÖåéâ àäéçäà     ',0
rem_text	db 'çÄÜåàíÖ çÄ èéáàñàû àëèéãúáìÖåéâ àäéçäà       ',0
title		db 'å•≠•§¶•‡ ®™Æ≠Æ™',0

else if lang eq ge
text:
	db 255,255,255,0,   '   TITLE                                       '
	db 255,255,255,0,   '  APP NAME                                     '
	db 255,255,255,0,   ' PARAMETER                                     '
	db 255,255,255,0,   '                ANWENDEN                       '
	db 255,255,255,0,   '     HINZUFUEGEN              ENTFERNEN        '
	db 0,0,0,0,         'AUF BUTTON KLICKEN, UM ICON ZU EDITIEREN       '
	db                  'x' ; <- END MARKER, DONT DELETE

add_text	db 'AUF UNBENUTZTE ICONPOSITION KLICKEN          ',0
rem_text	db 'ICON ANKLICKEN; DAS GELOESCHT WERDEN SOLL    ',0
title		db 'Icon Manager',0

else
text:
	db 255,255,255,0,   '   TITLE                                       '
	db 255,255,255,0,   '  APP NAME                                     '
	db 255,255,255,0,   ' PARAMETERS                                    '
	db 255,255,255,0,   '                APPLY CHANGES                  '
	db 255,255,255,0,   '      ADD ICON              REMOVE ICON        '
	db 0,0,0,0,         'CLICK BUTTON ON ICON POSITION FOR EDIT         '
	db                  'x' ; <- END MARKER, DONT DELETE
add_text	db 'CLICK ON A NOT USED POSITION                 ',0
rem_text	db 'CLICK ICON POSITION; YOU WANT TO DELETE      ',0
title		db 'Icon Manager',0

end if
;------------------------------------------------------------------------------
arrows	db '</>'
;------------------------------------------------------------------------------
iconname:
	db ICON_APP,0
;------------------------------------------------------------------------------
icon_default:
	db 'AA-SYSXTREE-000-/RD/1/SYSXTREE                '
	db '-                              *'
	db 13,10
;------------------------------------------------------------------------------
rep_text:
if lang eq ru
	db 'áçÄóäà    -     àá    , ÇõÅêÄç #'
else
	db 'ICONS     -     OF    , SELECTED'
end if

rep_text_len:
;------------------------------------------------------------------------------
align 4
get_bg_info:
	mcall	39,4
	mov	[bgrdrawtype],eax

	mcall	39,1	; get background size
	mov	[bgrxy],eax

	mov	ebx,eax
	shr	eax,16
	and	ebx,0xffff
	mov	[bgrx],eax
	mov	[bgry],ebx
	ret
;------------------------------------------------------------------------------
align 4
calc_icon_pos:
	movzx	eax,byte [ebp-20]	; x position
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
	mov	[ebp-12],eax
	movzx	eax,byte [ebp-20+1]	; y position
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
	mov	[ebp-8],eax
	ret
;--------------------------------------
align 4
;START2:
load_icon_list2:
	call	get_bg_info

	mcall	48,5
	mov	[warea.by_x],eax
	mov	[warea.by_y],ebx

	mcall	14
	add	eax,0x00010001
	mov	[scrxy],eax
;--------------------------------------
align 4
apply_changes2:
	mov	edi,[icons]
	mov	esi,icon_data
	mov	ebp,thread_stack+0x100	;0x5000	; threads stack starting point
;--------------------------------------
align 4
start_new:
	mov	eax,[esi]
	mov	[ebp-20],eax
	call	calc_icon_pos

	mov	edx,ebp
	mov	dword[ebp-4],esi
	mcall	51,1,thread
	add	ebp,0x100
; change to next thread if mutex is blocked
	mov	eax,68
	mov	ebx,1
;--------------------------------------
align 4
wait_thread_start:		;wait until thread draw itself first time
	cmp	[create_thread_event],bl	;mutex
	jz	wait_thread_end
	mcall
	jmp	wait_thread_start
;--------------------------------------
align 4
wait_thread_end:
	dec	[create_thread_event]	;reset event
	add	esi,REC_SIZE
	dec	edi
	jnz	start_new
;--------------------------------------
align 4
close:
	or	eax,-1
	mcall
;------------------------------------------------------------------------------
; esp-28 = start of thread stack
; esp-24 = ???
; esp-20 = 'AA-F' or...
; esp-16 = ebp-4 - area of f. 15.6
; esp-12 = ebp+0 = X
; esp-8  = ebp+4 = Y
; esp-4  = ebp+8 = adress of icon_data
;------------------------------------------------------------------------------
align 4
thread:
	sub	esp,12
	mov	ebp,esp
	sub	esp,16
	call	draw_window2
	mov	[create_thread_event],1
	mcall	40,010101b
;------------------------------------------------------------------------------
align 4
still2:
	mcall	10
	cmp	eax,1
	je	red2

	cmp	eax,3
	je	button2
	
	call	get_bg_info
	call	draw_icon2_1
	
	jmp	still2
;------------------------------------------------------------------------------
align 4
red2:
	mcall	14
	add	eax,0x00010001
	mov	[scrxy],eax
	mcall	48,5
	mov	[warea.by_x],eax
	mov	[warea.by_y],ebx
	add	ebp,+12
	call	calc_icon_pos
	add	ebp,-12
	mcall	9,process_table,-1
	mov	eax,[process_table+process_information.box.left]
	cmp	eax,[ebp+0]
	jne	@f
	mov	eax,[process_table+process_information.box.top]
	cmp	eax,[ebp+4]
	je	.lp1
;--------------------------------------
align 4
@@:
	call	get_bg_info
	mcall	67,[ebp+0],[ebp+4],51,51
;--------------------------------------
align 4
.lp1:
	call	draw_window2
	jmp	still2
;------------------------------------------------------------------------------
align 4
button2:
	mcall	17
	cmp	ah, 2
	jnz	still2

	mcall	9,process_table,-1
	mov	ecx,[ebx+30]	; PID
	mcall	18,21
	mov	edx,eax		; SLOT
	mcall	18,7
	cmp	edx,eax	; compare with active SLOT
	jne	@f
	mov	ecx,edx
	mcall	18,1	; set to down
	call	draw_window2
;--------------------------------------
align 4
@@:
	mov	esi,[ebp+8]
	mov	ebx,1
	mov	edi,finfo.path
	call	fill_paths
	inc	ebx
	mov	edi,param_str
	mov	dword[finfo_start+8],edi
	call	fill_paths
	cmp	byte[edi],0
	jne	.no0
	and	dword[finfo_start+8],0
;--------------------------------------
align 4
.no0:
	mov	ebx,finfo_start
	mcall	70
	jmp	still2
;------------------------------------------------------------------------------
align 4
fill_paths:
	push	esi edi
	movzx	ecx,byte[str_lens+ebx]
	add	esi,[positions+ebx*4]
	push	esi
	add	esi,ecx
;--------------------------------------
align 4
.l1:
	dec	esi
	cmp	byte[esi],' '
	jnz	.found
	loop	.l1
	pop	esi
	jmp	.noms
;--------------------------------------
align 4
.found:
	lea	ecx,[esi+1]
	pop	esi
	sub	ecx,esi
	rep	movsb
;--------------------------------------
align 4
.noms:
	and	byte[edi],0
	pop	edi esi
	ret
;--------------------------------------
align 4
atoi:
	push	esi
	xor	eax,eax
	xor	ebx,ebx
;--------------------------------------
align 4
.nxt:
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
	jmp	.nxt
;--------------------------------------
align 4
.done:
	pop	esi
	mov	eax,ebx
	ret
;--------------------------------------
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
draw_picture:
	mcall	15,6
	test	eax,eax
	jnz	@f
	mcall	68,1
	jmp	draw_picture
@@:
	mov	[ebp-4],eax
	mov	[image],image_area

	mov	edi,[ebp+8]
	lea	esi,[edi+12]
	call	atoi
	cmp	eax,[icon_count]
	ja	toponly.ex
;	imul	eax,(32*3*32)
	lea	eax,[eax+eax*2]	; multiply x3
	shl	eax,10		; multiply x1024
	
	mov	edi,[strip_file]
	lea	edi,[eax+edi+8]
	xor	ecx,ecx
	mov	esi,edi

	mov	[pixpos],0
;--------------------------------------
; loop Y start
align 4
new_line:	
	push	ecx	; Y
	xor	ebx,ebx
;--------------------------------------
; loop X start
align 4
newb:
	mov	ecx,[esp]
	push	ebx	; X

	cmp	ebx,10
	jb	yesbpix
	cmp	ebx,42
	jge	yesbpix
	cmp	ecx,31	;2
	jg	yesbpix

	push	esi
	mov	esi,edi
	add	esi,[pixpos]
;--------------------------------------
	add	[pixpos],3
	mov	eax,[esi]
	and	eax,0xffffff

	pop	esi

	cmp	eax,0
	je	yesbpix
	cmp	eax,0xfffcff	;f5f5f5
	je	yesbpix
	jmp	nobpix
;--------------------------------------
align 4
yesbpix:
stretch:
	cmp	[bgrdrawtype],dword 2
	jne	nostretch
	mov	eax,[ebp+4]
	add	eax,ecx
	imul	eax,[bgry]
	cdq
	movzx	ebx,word [scrxy]
	div	ebx
	imul	eax,[bgrx]
	push	eax
	mov	eax,[ebp+0]
	add	eax,[esp+4]
	imul	eax,[bgrx]
	cdq
	movzx	ebx,word [scrxy+2]
	div	ebx
	add	eax,[esp]
	add	esp,4
	jmp	notiled
;--------------------------------------
align 4
nostretch:
	cmp	[bgrdrawtype],dword 1
	jne	notiled
	mov	eax,[ebp+4]
	add	eax,ecx
	cdq
	movzx	ebx,word [bgrxy]
	div	ebx
	mov	eax,edx
	imul	eax,[bgrx]
	push	eax
	mov	eax,[ebp+0]
	add	eax,[esp+4]
	movzx	ebx,word [bgrxy+2]
	cdq
	div	ebx
	mov	eax,edx
	add	eax,[esp]
	add	esp,4
;--------------------------------------
align 4
notiled:
	lea	ecx,[eax+eax*2]
	add	ecx,[ebp-4]
	mov	eax,[ecx]
;--------------------------------------
align 4
nobpix:
	mov	edx,eax
	mov	eax,[image]

	mov	[eax],dl
	inc	eax
	ror	edx,8
	
	mov	[eax],dl
	inc	eax
	ror	edx,8
	
	mov	[eax],dl
	
	inc	eax
	mov	[image],eax

	pop	ebx

	inc	ebx
	
	mov	eax,[yw]
	inc	eax
	cmp	ebx,eax
	jb	newb

	pop	ecx

	inc	ecx
	mov	eax,[ya]
	add	[pixpos],eax

	cmp	[top],1
	jne	notop

	cmp	ecx,38
	je	toponly
;--------------------------------------
align 4
notop:
	cmp	ecx,52
	jnz	new_line
;--------------------------------------
align 4
toponly:
	mcall	15,7,[ebp-4]
	test	eax,eax
	jnz	@f
	mcall	68,1
	jmp	toponly
@@:
	xor	edx,edx
	mcall	7,image_area,<52,52>
;--------------------------------------
align 4	
.ex:
	mov	[load_pic],0
	ret
;------------------------------------------------------------------------------
align 4
draw_text:
	mov	esi,[ebp+8]
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
	add	ebx,1 shl 16	;*65536+1
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
	mcall	,,0xffffff
	mov	[draw_pic],0
	ret
;------------------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
align 4
draw_window2:
	mcall	12,1
	; DRAW WINDOW
	xor	eax,eax		; function 0 : define and draw window
	mov	ebx,[ebp+0-2]
	mov	ecx,[ebp+4-2]
	mov	bx,[yw]		; [x start] *65536 + [x size]
	mov	cx,51		; [y start] *65536 + [y size]
	mov	edx,0x41000000	; color of work area RRGGBB,8->color gl
	mcall
	
	mcall	8,51,50,0x40000002 ; button
;--------------------------------------
align 4
draw_icon2_1:
; change to next thread if mutex is blocked
	mov	eax,68
	mov	ebx,1
;--------------------------------------
align 4
draw_icon2:
	xchg	[load_pic],bl	;mutex
	test	bl,bl
	je	draw_icon_end
	mcall
	jmp	draw_icon2
;--------------------------------------
align 4
draw_icon_end:
; change to next thread if mutex is blocked
	mov	eax,68
	mov	ebx,1
;--------------------------------------
align 4
draw_icon_2:
	xchg	[draw_pic],bl	;mutex
	test	bl,bl
	je	draw_icon_end_2
	mcall
	jmp	draw_icon_2
;--------------------------------------
align 4
draw_icon_end_2:
	call	draw_picture
	call	draw_text
	mcall	12,2
	ret
;------------------------------------------------------------------------------
tl	dd 8
yw:	dd 51
ya	dd 0
cur_btn	dd 40

draw_pic	db 0
load_pic	db 0
create_thread_event	db 0

image	dd image_area

IncludeUGlobals
;------------------------------------------------------------------------------
IM_END:
;------------------------------------------------------------------------------
align 4
bgrx	dd ?
bgry	dd ?

bgrxy	dd ?
warea:
 .by_x:
  .right	dw ?
  .left		dw ?
 .by_y:
  .bottom	dw ?
  .top		dw ?
scrxy		dd ?
bgrdrawtype	dd ?

pixpos	dd ?
top	dd ?
icons	dd ?
addr	dd ?
cur_str		dd ?
cur_band	dd ?
sel_icon1	rd 1
icon_count	rd 1
strip_file	rd 1
;------------------------------------------------------------------------------
align 4
param_str	rb 31
;------------------------------------------------------------------------------
align 4
process_table:
	rb 0x400
;------------------------------------------------------------------------------	
align 4
icon_data:
	rb 0x1000
;------------------------------------------------------------------------------
align 4
	rb 0x1000
stack_area:
;------------------------------------------------------------------------------
align 4
I_Param:
	rb 0x100	; max 256 
;------------------------------------------------------------------------------
align 4
thread_stack:
	rb 0x100*48	; max 48 icons
;------------------------------------------------------------------------------
align 4
image_area:
	rb 52*52*3
;------------------------------------------------------------------------------
I_END:
