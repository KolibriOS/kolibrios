; SPDX-License-Identifier: NOASSERTION
;
; System settings.
;   SETUP BOOT - apply the settings from system.ini (run from AUTORUN.DAT)
;   SETUP      - window to change and save them
;
; Texts are UTF-8

include '../../macros.inc'
format meos 01
entry START
stack 4096

include '../../proc32.inc'
include '../../dll.inc'

;---------------------------------------------------------------
START:
	mcall	68,11
	stdcall dll.Load, @IMPORT
	test	eax,eax
	jnz	quit

	GetCommandLine eax
	cmp	dword[eax],'BOOT'
	jnz	@f

	call	settings_read_boot
	call	settings_read_ini
	call	settings_apply_boot
	call	style_apply
quit:
	mcall	-1

@@:
	call	settings_read_system
; the checkbox image is shared by @RESHARE, 0 if it is not running
	mcall	68,22,sz_checkbox,0,0
	mov	[checkboxImage],eax

;---------------------------------------------------------------
draw_window:
	mov	eax,[language]
	mov	eax,[texts+eax*4]
	mov	[text],eax

	mcall	12,1
	mcall	48,3,sc,sizeof.system_colors
	mov	byte[sc.work_text+3],FONT
	mov	byte[sc.work_button_text+3],FONT
	mcall	48,4
	lea	ecx,[eax+(50 shl 16)+CLIENT_H+4]
	mov	edx,[sc.work]
	or	edx,0x34000000
	mcall	0,<50,CLIENT_W+9>,,,0,title

; a row: label, value (drawn by draw_values), '-' and '+' buttons
	xor	ebp,ebp
.row:
	imul	edi,ebp,ROW_STEP
	add	edi,TOP		; y of the row
	mov	ecx,edi
	shl	ecx,16
	add	ecx,ROW_H-1	; the button is 1 pixel bigger than given
	lea	edx,[FIRST_ROW_BUTTON+ebp*2]
	mcall	8,<BUTTON1_X,ROW_H-1>,,,[sc.work_button]
	lea	edx,[FIRST_ROW_BUTTON+1+ebp*2]
	mcall	8,<BUTTON2_X,ROW_H-1>
; the kernel leaves the corners of a button out: one frame for both,
; its right corners stay out
	mov	edx,[sc.work_button]
	shr	edx,1
	and	edx,0x7F7F7F	; the border colour of a button
	mov	cx,1
	mcall	13,<BUTTON1_X,BUTTON2_X+ROW_H-1-BUTTON1_X>
	add	ecx,(ROW_H-1) shl 16
	mcall	13

	lea	ebx,[edi+TEXT_DY+(LABEL_X shl 16)]
	mov	ecx,ebp
	call	get_text
	mcall	4,,[sc.work_text]

	lea	ebx,[edi+TEXT_DY+((BUTTON1_X+8) shl 16)]
	lea	edx,[glyphs+ebp*4]
	mcall	4,,[sc.work_button_text]
	lea	ebx,[edi+TEXT_DY+((BUTTON2_X+8) shl 16)]
	lea	edx,[glyphs+2+ebp*4]
	mcall	4

	inc	ebp
	cmp	ebp,ROWS
	jb	.row

; group box of the checkboxes: a frame, its title over it on the work colour
	mcall	13,<GROUP_X,GROUP_W>,<GROUP_Y,GROUP_H>,[sc.work_graph]
	mcall	13,<GROUP_X+1,GROUP_W-2>,<GROUP_Y+1,GROUP_H-2>,[sc.work]
	mov	ecx,T_GROUP
	call	get_text
	mov	ecx,[sc.work_text]
	or	ecx,0x40000000
	mcall	4,<GROUP_TITLE_X,GROUP_TITLE_Y>,,,,[sc.work]

; The checkboxes are spread over the group, the last one ends at its end.
; LBA and PCI are fixed, so the step only depends on the speaker label.
	mov	ecx,T_SPEAKER
	call	get_text
	mov	eax,ITEM_END-ITEM_X-CHECK_SIZE-8
.length:
	mov	cl,[edx]
	inc	edx
	test	cl,cl
	jz	.step
	and	cl,0xC0
	cmp	cl,0x80		; UTF-8 continuation byte
	jz	.length
	sub	eax,8
	jmp	.length
.step:
	shr	eax,1
	mov	[checkStep],eax

; a checkbox: the box (drawn by draw_values), then the label, both under
; one button up to the end of the group. The next button covers the rest.
	xor	ebp,ebp
.check:
	mov	edi,[checkStep]
	imul	edi,ebp
	add	edi,ITEM_X	; x of the box
	lea	ebx,[edi+CHECK_SIZE+8]
	shl	ebx,16
	add	ebx,CHECK_Y+TEXT_DY
	call	check_label
	mcall	4,,[sc.work_text]
	mov	ebx,edi
	shl	ebx,16
	add	ebx,ITEM_END
	sub	ebx,edi
	lea	edx,[FIRST_CHECK_BUTTON+BT_HIDE+BT_NOFRAME+ebp]
	mcall	8,,<CHECK_Y,ROW_H-1>
	inc	ebp
	cmp	ebp,CHECKS
	jb	.check

	mcall	12,2

;---------------------------------------------------------------
draw_values:
	mov	edx,languageNames
	mov	ecx,[language]
	call	nth_string
	mov	[valueText],edx

	mov	edx,sz_subpixel
	mov	ecx,[fontSmoothing]
	cmp	ecx,2
	jae	@f
	add	ecx,T_OFF
	call	get_text
@@:
	mov	[valueText+4],edx

	mov	eax,[fontHeight]
	mov	bl,10
	div	bl
	add	ax,'00'
	mov	word[heightText],ax
	mov	[valueText+8],heightText

; each value in a light box framed on three sides, the button is the fourth,
; the left corners are cut off
	xor	ebp,ebp
.row:
	imul	edi,ebp,ROW_STEP
	add	edi,TOP
	mov	ecx,edi
	shl	ecx,16
	add	ecx,ROW_H
	mcall	13,<VALUE_X+1,VALUE_W-1>,,[sc.work_graph]
	add	ecx,(1 shl 16)-2
	mcall	13,<VALUE_X,VALUE_W>
; sunken, the inner lines of a button reversed: three boxes, each 1 pixel
; smaller, leave darker lines left and top and lighter right and bottom
	mov	edx,[sc.work_light]
	mov	eax,edx
	shr	eax,3
	and	eax,0x1F1F1F
	sub	edx,eax		; 1/8 darker
	mcall	13,<VALUE_X+1,VALUE_W-1>
	mov	edx,[sc.work_light]
	or	edx,0x1F1F1F	; lighter
	add	ecx,(1 shl 16)-1
	mcall	13,<VALUE_X+2,VALUE_W-2>
	dec	ecx
	mcall	13,<VALUE_X+2,VALUE_W-3>,,[sc.work_light]
	lea	ebx,[edi+TEXT_DY+((VALUE_X+VALUE_PAD) shl 16)]
	mov	edx,[valueText+ebp*4]
	mcall	4,,[sc.work_text]
	inc	ebp
	cmp	ebp,ROWS
	jb	.row

; checkboxes: a frame, light inside, the @RESHARE image when set
	xor	ebp,ebp
.check:
	mov	ebx,[checkStep]
	imul	ebx,ebp
	add	ebx,ITEM_X
	shl	ebx,16
	add	ebx,CHECK_SIZE
	mcall	13,,<CHECK_BOX_Y,CHECK_SIZE>,[sc.work_graph]
	add	ebx,(1 shl 16)-2
	mcall	13,,<CHECK_BOX_Y+1,CHECK_SIZE-2>,[sc.work_light]
	mov	eax,[checks+ebp*8]
	mov	eax,[eax]
	xor	eax,[checks+ebp*8+4]
	jz	.next
	mov	edx,ebx
	mov	dx,CHECK_BOX_Y+1
	mov	ebx,[checkboxImage]
	test	ebx,ebx		; no @RESHARE: the box stays empty
	jz	.next
	mcall	7,,<CHECK_SIZE-2,CHECK_SIZE-2>
.next:
	inc	ebp
	cmp	ebp,CHECKS
	jb	.check

;---------------------------------------------------------------
still:
	mcall	10
	cmp	eax,1
	jz	draw_window
	cmp	eax,2
	jz	.key
	cmp	eax,3
	jnz	still

	mcall	17
	shr	eax,8
	cmp	eax,1
	jz	quit
	sub	eax,FIRST_ROW_BUTTON
	jb	still		; 0: no button after all
	cmp	eax,FIRST_CHECK_BUTTON-FIRST_ROW_BUTTON
	jae	.check

; '-' or '+' of a row: step the value within the limits of the row
	shr	eax,1		; eax = row, CF = '+'
	lea	esi,[eax*3]
	lea	esi,[rows+esi*4]
	mov	edi,[esi]
	mov	ecx,[edi]
	jc	.plus
	cmp	ecx,[esi+4]
	jbe	still
	dec	ecx
	jmp	.set
.plus:
	cmp	ecx,[esi+8]
	jae	still
	inc	ecx
.set:
	mov	[edi],ecx
	push	eax
	call	apply_and_save
	pop	eax
	cmp	eax,1		; the language and the font smoothing
	jbe	draw_window	; change all the texts: repaint the window
	jmp	draw_values

.check:
	mov	eax,[checks-(FIRST_CHECK_BUTTON-FIRST_ROW_BUTTON)*8+eax*8]
	xor	dword[eax],1
	call	apply_and_save
	jmp	draw_values

.key:
	mcall	2
	jmp	still

;---------------------------------------------------------------
; every change takes effect at once and is kept in system.ini
apply_and_save:
	call	settings_apply
	jmp	settings_save_ini

; edx = label of the checkbox ebp
check_label:
	mov	edx,[checkLabels+ebp*4]
	test	edx,edx
	jnz	@f
	mov	ecx,T_SPEAKER
	call	get_text
@@:
	ret

; edx = string number ecx of the window language
get_text:
	mov	edx,[text]
; edx = string number ecx of the zero separated list at edx
nth_string:
	jecxz	.done
@@:
	inc	edx
	cmp	byte[edx-1],0
	jnz	@b
	loop	@b
.done:
	ret

;---------------------------------------------------------------
; DATA
@IMPORT:
library libini, 'libini.obj'
import	libini, \
	ini.get_str, 'ini_get_str',\
	ini.get_int, 'ini_get_int',\
	ini.set_str, 'ini_set_str',\
	ini.set_int, 'ini_set_int'

ROWS	= 3
CHECKS	= 3
FIRST_ROW_BUTTON = 10	; row N has '-' = 10+N*2 and '+' = 11+N*2
FIRST_CHECK_BUTTON = 20
FONT	= 0xB0		; text flags: zero terminated, 8x16 UTF-8

; layout of the client area: controls ROW_H high, GAP empty pixels apart,
; EDGE to the window border, GAP at the top
LABEL_CHARS = 20	; the longest label, Spanish
GAP	= 12
EDGE	= 8
TOP	= GAP
ROW_H	= 24
ROW_STEP = ROW_H + GAP
TEXT_DY	= 5		; the capitals, rows 2..11 of the font, centred in ROW_H
LABEL_X	= EDGE - 1	; column 0 of a letter is empty: aligned with the group frame
VALUE_X	= LABEL_X + LABEL_CHARS*8 + GAP
VALUE_PAD = GAP
VALUE_W	= 8*8 + VALUE_PAD*2
BUTTON1_X = VALUE_X + VALUE_W	; right after the value box
BUTTON2_X = BUTTON1_X + ROW_H - 1	; the buttons share a border
CLIENT_W = BUTTON2_X + ROW_H + EDGE
GROUP_X	= EDGE
GROUP_W	= CLIENT_W - EDGE*2
GROUP_Y	= TOP + ROWS*ROW_STEP + 5	; GAP to the capitals of the title
GROUP_TITLE_Y = GROUP_Y - 7		; the frame crosses the capitals
CHECK_Y	= GROUP_Y + 1 + GAP		; the row of the checkboxes
GROUP_H	= 1 + GAP + ROW_H + GAP + 1
CHECK_SIZE = 15
CHECK_BOX_Y = CHECK_Y + (ROW_H-CHECK_SIZE)/2
ITEM_X	= GROUP_X + 1 + GAP
ITEM_END = GROUP_X + GROUP_W - GAP	; the last column of a letter is empty
GROUP_TITLE_X = ITEM_X - 8 - 1	; a space, then a letter from its column 1
CLIENT_H = GROUP_Y + GROUP_H + EDGE

; variable, min, max of every row
align 4
rows:
	dd	language,      0, LANGUAGES-1
	dd	fontSmoothing, 0, 2
	dd	fontHeight,    FONT_H_MIN, FONT_H_MAX

; variable and the value of it that means 'not checked'
checks	dd lba,0, pci,0, speakerMute,1
checkLabels dd sz_lba, sz_pci, 0	; 0: the speaker label of the language

glyphs	db '<',0,'>',0,'<',0,'>',0,'-',0,'+',0
sz_subpixel db 'Subpixel',0
languageNames db 'English',0,'Finnish',0,'German',0,'Russian',0
	db 'French',0,'Estonian',0,'Spanish',0,'Italian',0

title	db "System settings",0
sz_checkbox db "CHECKBOX",0

; Texts of the window languages: zero terminated strings in this order
; (row labels, group title, speaker label, 'off', 'on')
T_GROUP	= ROWS
T_SPEAKER = ROWS + 1
T_OFF	= ROWS + 2

; the texts of every kernel language, English where there is no translation
texts	dd texteng, texteng, texteng, textrus, texteng, texteng, textspa, texteng

texteng	db 'System language',0, 'Font smoothing',0, 'Font height',0
	db ' Access settings ',0, 'Speaker',0
	db 'Off',0, 'On',0

textspa	db 'Idioma del sistema',0, 'Suavizado de fuentes',0, 'Altura de fuente',0
	db ' Ajustes de acceso ',0, 'Altavoz',0
	db 'No',0, 'Sí',0

textrus	db 'Язык системы',0, 'Сглаживание шрифтов',0, 'Высота шрифтов',0
	db ' Настройки доступа ',0, 'Динамик',0
	db 'Нет',0, 'Да',0

;---------------------------------------------------------------
include 'settings.inc'


text	dd ?		; texts of the window language
checkboxImage dd ?
checkStep dd ?		; x distance of two checkboxes
sc	system_colors
valueText rd ROWS	; the strings shown in the value boxes
heightText rb 4
