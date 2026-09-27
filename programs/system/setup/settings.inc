; SPDX-License-Identifier: NOASSERTION
;
; System settings: read them from the kernel or from system.ini,
; apply them to the kernel and save them to system.ini.

;---------------------------------------------------------------
; Take the current values from the kernel
settings_read_system:
	mcall	26,5
	mov	ecx,LANGUAGES-1	; a language not in the list becomes English
@@:
	cmp	al,[kernelLanguages+ecx]
	jz	@f
	loop	@b
@@:
	mov	[language],ecx
	mcall	26,11
	mov	[lba],eax
	mcall	26,12
	mov	[pci],eax
	mcall	18,8,1
	mov	[speakerMute],eax
	mcall	48,9
	mov	[fontSmoothing],eax
	mcall	48,11
	mov	[fontHeight],eax
	mcall	18,19,0
	mov	[mouseSpeed],eax
	mcall	18,19,2
	mov	[mouseAccel],eax
	mcall	18,19,6
	mov	[mouseDelay],eax
	ret

;---------------------------------------------------------------
; Override the values with the ones stored in system.ini.
; A missing or unknown value keeps the current one.
settings_read_ini:
	mov	esi,iniSettings
.next:
	mov	edi,[esi+12]
	test	edi,edi
	jnz	.word
	mov	eax,[esi+8]
	invoke	ini.get_int, sz_ini, [esi], [esi+4], [eax]
	jmp	.store
.word:
	and	[iniWord],0
	invoke	ini.get_str, sz_ini, [esi], [esi+4], iniWord, 4, 0
	mov	eax,[iniWord]
	or	eax,0x20202020	; lowercase, zeroes become spaces
	xor	edx,edx
@@:
	cmp	eax,[edi+4+edx*4]
	jz	.found
	inc	edx
	cmp	edx,[edi]
	jb	@b
	jmp	.skip
.found:
	mov	eax,edx
.store:
	mov	edx,[esi+8]
	mov	[edx],eax
.skip:
	add	esi,16
	cmp	esi,iniSettings.end
	jb	.next
	ret

;---------------------------------------------------------------
; Store the values to system.ini (all but the mouse ones).
; Not ebx for the loop: ini_set_str returns with ebx = 13.
settings_save_ini:
	mov	esi,iniSettings
.next:
	mov	eax,[esi+8]
	mov	eax,[eax]
	mov	edi,[esi+12]
	test	edi,edi
	jnz	.word
	invoke	ini.set_int, sz_ini, [esi], [esi+4], eax
	jmp	.done
.word:
	mov	eax,[edi+4+eax*4]
	mov	[iniWord],eax
	xor	ecx,ecx		; length of the word
@@:
	cmp	byte[iniWord+ecx],' '
	jz	@f
	inc	ecx
	cmp	ecx,4
	jb	@b
@@:
	invoke	ini.set_str, sz_ini, [esi], [esi+4], iniWord, ecx
.done:
	add	esi,16
	cmp	esi,iniSettings.saved
	jb	.next
	ret

;---------------------------------------------------------------
; Pass the values to the kernel
settings_apply:
	mov	eax,[language]
	movzx	ecx,byte[kernelLanguages+eax]
	mcall	21,5
	mcall	21,11,[lba]
	mcall	21,12,[pci]
	mcall	48,10,[fontSmoothing]
	mcall	48,12,[fontHeight]
	mcall	18,19,1,[mouseSpeed]
	mcall	18,19,3,[mouseAccel]
	mcall	18,19,7,[mouseDelay]
; the speaker can only be toggled
	mcall	18,8,1
	cmp	eax,[speakerMute]
	jz	@f
	mcall	18,8,2
@@:
	ret

;---------------------------------------------------------------
; Button style, desktop background and skin from the [style] section.
; Used at system start only.
style_apply:
	invoke	ini.get_int, sz_ini, sz_style, sz_buttonStyle, 0
	mcall	48,1,eax

; the background is set by a program: bg_program with bg_param
	invoke	ini.get_str, sz_ini, sz_style, sz_bgProgram, bgProgram, PATH_MAX, 0
	invoke	ini.get_str, sz_ini, sz_style, sz_bgParam, bgParam, PATH_MAX, 0
	cmp	byte[bgParam],'\'	; KIV param like '\S__/path/to/img'
	jnz	@f
	mov	[fileInfo.path],bgParam+4
	call	wait_for_file
@@:
	mcall	70,runBgProgram

	invoke	ini.get_str, sz_ini, sz_style, sz_skin, skinPath, PATH_MAX, 0
	cmp	byte[skinPath],0
	jz	@f
	mov	[fileInfo.path],skinPath
	call	wait_for_file
	mcall	48,8,skinPath
@@:
	ret

; Files on /kolibrios and /usbhd may appear a few seconds after boot:
; try to read the file for up to 5 seconds
wait_for_file:
	mov	edi,5
@@:
	mcall	70,fileInfo
	test	eax,eax
	jz	@f
	mcall	5,100
	dec	edi
	jnz	@b
@@:
	ret

;---------------------------------------------------------------
; DATA
; The languages of the window, their numbers in the kernel
LANGUAGES = 3
kernelLanguages db 1, 7, 4	; English, Spanish, Russian
PATH_MAX = 256

; Every value stored in system.ini: section, key, variable, words.
; Words = 0 for a number, otherwise the list of words for 0, 1, 2...
align 4
iniSettings:
	dd	sz_system,   sz_language,   language,      wordsLanguage
	dd	sz_system,   sz_fontHeight, fontHeight,    0
	dd	sz_system,   sz_fontSmooth, fontSmoothing, wordsSmoothing
	dd	sz_system,   sz_speaker,    speakerMute,   wordsOnOff
	dd	sz_lowLevel, sz_lba,        lba,           wordsOnOff
	dd	sz_lowLevel, sz_pci,        pci,           wordsOnOff
.saved:	; the mouse is configured by its own application
	dd	sz_mouse,    sz_speed,      mouseSpeed,    0
	dd	sz_mouse,    sz_accel,      mouseAccel,    0
	dd	sz_mouse,    sz_dblClick,   mouseDelay,    0
.end:

wordsOnOff	dd 2, 'off ','on  '
wordsSmoothing	dd 3, 'off ','on  ','sbp '
wordsLanguage	dd LANGUAGES, 'en  ','es  ','ru  '

runBgProgram:
	dd	7, 0, bgParam, 0, 0
	db	0
	dd	bgProgram

fileInfo:
	dd	0, 0, 0, 4, fileBuf
	db	0
.path	dd	?

sz_ini		db "/sys/settings/system.ini",0

sz_system	db "system",0
sz_language	db "language",0
sz_fontHeight	db "font height",0
sz_fontSmooth	db "font smoothing",0
sz_speaker	db "speaker mute",0

sz_lowLevel	db "low-level",0
sz_lba		db "LBA",0
sz_pci		db "PCI",0

sz_mouse	db "mouse",0
sz_speed	db "speed",0
sz_accel	db "acceleration",0
sz_dblClick	db "double_click_delay",0

sz_style	db "style",0
sz_buttonStyle	db "buttons_gradient",0
sz_bgProgram	db "bg_program",0
sz_bgParam	db "bg_param",0
sz_skin		db "skin",0

; not stored in the program file
language	dd ?
lba		dd ?
pci		dd ?
speakerMute	dd ?
fontSmoothing	dd ?
fontHeight	dd ?
mouseSpeed	dd ?
mouseAccel	dd ?
mouseDelay	dd ?

iniWord		dd ?	; a word of system.ini
fileBuf		dd ?
bgProgram	rb PATH_MAX
bgParam		rb PATH_MAX
skinPath	rb PATH_MAX
