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

	call	settings_read_system
	GetCommandLine eax
	cmp	dword[eax],'BOOT'
	jnz	@f

	call	settings_read_ini
	call	settings_apply
	call	style_apply
quit:
	mcall	-1

@@:
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

; group box of the checkboxes: a frame, cut under its title
	mcall	13,<GROUP_X,GROUP_W>,<GROUP_Y,GROUP_H>,[sc.work_graph]
	mcall	13,<GROUP_X+1,GROUP_W-2>,<GROUP_Y+1,GROUP_H-2>,[sc.work]
	mov	ecx,T_GROUP
	call	get_text
	call	utf8_length
	lea	ebx,[(GROUP_TITLE_X-4) shl 16+eax*8+8]
	push	edx
	mcall	13,,<GROUP_Y,1>,[sc.work]
	pop	edx
	mcall	4,<GROUP_TITLE_X,GROUP_Y-8>,[sc.work_text]

; a checkbox: label, then the box (drawn by draw_values), both under
; one button. The checkboxes are spread over the width of the group.
	mov	esi,GROUP_W-GROUP_PAD*2
	xor	ebp,ebp
.width:
	call	check_label
	call	utf8_length
	lea	eax,[eax*8+8+CHECK_SIZE]
	mov	[itemW+ebp*4],eax
	sub	esi,eax
	inc	ebp
	cmp	ebp,CHECKS
	jb	.width
	mov	eax,esi
	xor	edx,edx
	mov	ecx,CHECKS-1
	div	ecx
	mov	esi,eax		; the gap between two checkboxes

	mov	edi,ITEM_X	; x of the checkbox
	xor	ebp,ebp
.check:
	call	check_label
	mov	ebx,edi
	shl	ebx,16
	add	ebx,CHECK_ROW_Y+1
	mcall	4,,[sc.work_text]
	mov	eax,[itemW+ebp*4]
	lea	ecx,[edi+eax-CHECK_SIZE]
	mov	[boxX+ebp*4],ecx
	mov	ebx,edi
	shl	ebx,16
	add	ebx,eax
	lea	edx,[FIRST_CHECK_BUTTON+BT_HIDE+ebp]
	mcall	8,,<CHECK_ROW_Y-4,24>
	add	edi,[itemW+ebp*4]
	add	edi,esi
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

; each value on a light box with the corners cut off: two bars crosswise
	xor	ebp,ebp
.row:
	imul	edi,ebp,ROW_STEP
	add	edi,TOP
	mov	ecx,edi
	shl	ecx,16
	add	ecx,ROW_H
	mcall	13,<VALUE_X+1,VALUE_W-2>,,[sc.work_light]
	add	ecx,(1 shl 16)-2
	mcall	13,<VALUE_X,VALUE_W>
	lea	ebx,[edi+TEXT_DY+((VALUE_X+VALUE_PAD) shl 16)]
	mov	edx,[valueText+ebp*4]
	mcall	4,,[sc.work_text]
	inc	ebp
	cmp	ebp,ROWS
	jb	.row

; checkboxes: a frame, white inside, the @RESHARE image when set
	xor	ebp,ebp
.check:
	mov	ebx,[boxX+ebp*4]
	shl	ebx,16
	add	ebx,CHECK_SIZE
	mcall	13,,<CHECK_BOX_Y,CHECK_SIZE>,[sc.work_graph]
	add	ebx,(1 shl 16)-2
	mcall	13,,<CHECK_BOX_Y+1,CHECK_SIZE-2>,0xFFFFFF
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
	cmp	eax,FIRST_CHECK_BUTTON
	jae	.check

; '-' or '+' of a row: step the value within the limits of the row
	sub	eax,FIRST_ROW_BUTTON
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
	mov	eax,[checks-FIRST_CHECK_BUTTON*8+eax*8]
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

; eax = number of characters of the UTF-8 string at edx
utf8_length:
	xor	eax,eax
	push	edx
.next:
	mov	cl,[edx]
	inc	edx
	test	cl,cl
	jz	.done
	and	cl,0xC0
	cmp	cl,0x80		; continuation byte
	jz	.next
	inc	eax
	jmp	.next
.done:
	pop	edx
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

; layout of the client area
LABEL_CHARS = 20	; the longest label, Spanish
TOP	= 10
ROW_H	= 24
ROW_STEP = 30
TEXT_DY	= (ROW_H-16)/2
LABEL_X	= 10
VALUE_X	= LABEL_X + LABEL_CHARS*8 + 8
VALUE_PAD = 14
VALUE_W	= 8*8 + VALUE_PAD*2
BUTTON1_X = VALUE_X + VALUE_W + 10
BUTTON2_X = BUTTON1_X + 30
CLIENT_W = BUTTON2_X + ROW_H + 10
GROUP_X	= LABEL_X
GROUP_W	= CLIENT_W - LABEL_X*2
GROUP_Y	= TOP + ROWS*ROW_STEP + 14
GROUP_H	= 50
GROUP_TITLE_X = GROUP_X + 12
GROUP_PAD = 14
CHECK_ROW_Y = GROUP_Y + (GROUP_H-16)/2	; y of the checkbox labels
CHECK_SIZE = 15
CHECK_BOX_Y = CHECK_ROW_Y + (16-CHECK_SIZE)/2
ITEM_X	= GROUP_X + GROUP_PAD
CLIENT_H = GROUP_Y + GROUP_H + 10

; variable, min, max of every row
align 4
rows:
	dd	language,      0, LANGUAGES-1
	dd	fontSmoothing, 0, 2
	dd	fontHeight,    9, 99

; variable and the value of it that means 'not checked'
checks	dd lba,0, pci,0, speakerMute,1
checkLabels dd labelLba, labelPci, 0	; 0: the speaker label of the language
labelLba db 'LBA',0
labelPci db 'PCI',0

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
	db 'Access settings',0, 'Speaker',0
	db 'Off',0, 'On',0

textspa	db 'Idioma del sistema',0, 'Suavizado de fuentes',0, 'Altura de fuente',0
	db 'Ajustes de acceso',0, 'Altavoz',0
	db 'No',0, 'Sí',0

textrus	db 'Язык системы',0, 'Сглаживание шрифтов',0, 'Высота шрифтов',0
	db 'Настройки доступа',0, 'Динамик',0
	db 'Нет',0, 'Да',0

;---------------------------------------------------------------
include 'settings.inc'


text	dd ?		; texts of the window language
checkboxImage dd ?
boxX	rd CHECKS	; x of every checkbox box, set by draw_window
itemW	rd CHECKS	; width of every checkbox with its label
sc	system_colors
valueText rd ROWS	; the strings shown in the value boxes
heightText rb 4
