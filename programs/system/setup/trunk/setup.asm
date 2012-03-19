;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                               ;;
;;          DEVICE SETUP         ;;
;;                               ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Authors: Ville       - original version
;          A. Ivushkin - autostart (w launcher)
;          M. Lisovin  - added many feauters (apply all, save all, set time...)
;          I. Poddubny - fixed russian keymap
;14/08/06  Mario79 - added regulation of mouse features
;-------------------------------------------------------------------------------
	use32
	org 0x0

	db 'MENUET01'	; 8 byte identifier
	dd 0x01		; header version
	dd START	; pointer to program start
	dd IM_END	; size of image
	dd I_END	;0x4000	; reguired amount of memory
	dd stack_area	; stack pointer (esp)
	dd I_PARAM	; boot parameters
	dd 0x0		; path
;-------------------------------------------------------------------------------
include '..\..\..\macros.inc'
;-------------------------------------------------------------------------------
LLL equ (56+3)
BBB equ 25
;-------------------------------------------------------------------------------
apply_all:
	call	_pci_acc    ;12
	call	_syslang    ;5
	call	_keyboard    ;2
	call	_mouse_speed
	call	_mouse_delay
	ret
;-------------------------------------------------------------------------------
apply_all_and_exit:
	mcall	70,read_fileinfo
	call	apply_all
	jmp	close
;-------------------------------------------------------------------------------
set_language_and_exit:
	mcall	26,2,9
	cmp	eax,1
	je	russian

	xor	eax,eax
;--------------------------------------
set_lang_now:
	mov	[keyboard],eax
	call	_keyboard
	jmp	close
;--------------------------------------
russian:
	mov	eax,3
	jmp	set_lang_now
;-------------------------------------------------------------------------------
set_syslanguage_and_exit:
	mcall	26,5
	cmp	eax,6
	jne	temp
	xor	eax,eax
;--------------------------------------
temp:
	inc	eax
	mov	[syslang],eax
	call	_syslang
	jmp	close
;-------------------------------------------------------------------------------
get_setup_values:
	mcall	26,2,9
	dec	eax
	mov	[keyboard],eax

	mcall	26,5
	mov	[syslang],eax

	mcall	26,11
	mov	[lba_read],eax

	mcall	26,12
	mov	[pci_acc],eax

	mcall	18,19,0
	mov	[mouse_speed],eax

	mcall	18,19,2
	mov	[mouse_delay],eax
	ret
;-------------------------------------------------------------------------------
START:
	cmp	[I_PARAM],'SLAN'
	je	set_syslanguage_and_exit

	cmp	[I_PARAM],'LANG'
	je	set_language_and_exit

	cmp	[I_PARAM],'BOOT'
	je	apply_all_and_exit

	call	get_setup_values
	call	loadtxt
;-------------------------------------------------------------------------------
red:
	call	draw_window
;-------------------------------------------------------------------------------
still:
sysevent:
	mcall	23,8	      ; wait here for event with timeout
	cmp	eax,1
	jz	red

	cmp	eax,2
	jz	key

	cmp	eax,3
	jz	button

	jmp	still
;-------------------------------------------------------------------------------
key:
	mcall	2
	jmp	still
;-------------------------------------------------------------------------------
button:
	mcall	17
	cmp	ah,99
	jne	nosaveall
	mcall	70,save_fileinfo
	jmp	still
;--------------------------------------    
nosaveall:
	cmp	ah,100
	jne	no_apply_all
	call	apply_all
	jmp	still
;--------------------------------------
no_apply_all:
	cmp	ah,1	; CLOSE APPLICATION
	jne	no_close
;--------------------------------------
close:
	or	eax,-1
	mcall
;--------------------------------------
no_close:
	cmp	ah,4	; SET KEYBOARD
	jnz	nokm
	mov	eax,[keyboard]
	test	eax,eax
	je	downuplbl
	dec	eax
	jmp	nodownup
;--------------------------------------
downuplbl:
	mov	eax,5
;--------------------------------------
nodownup:
	mov	[keyboard],eax
	call	draw_infotext
;--------------------------------------
nokm:
	cmp	ah,5
	jnz	nokp
	mov	eax,[keyboard]
	cmp	eax,5
	je	updownlbl
	inc	eax
	jmp	noupdown
;--------------------------------------
updownlbl:
	xor	eax,eax
;--------------------------------------
noupdown:
	mov	[keyboard],eax
	call	draw_infotext
;--------------------------------------
nokp:
	cmp	ah,92	; SET LBA READ
	jne	no_lba_d
;--------------------------------------
slbal:
	btc	[lba_read],0
	call	draw_infotext
	jmp	still
;--------------------------------------
no_lba_d:
	cmp	ah,93
	jne	no_lba_i
	jmp	slbal
;--------------------------------------
no_lba_i:
	cmp	ah,91
	jne	no_set_lba_read
	call	_lba_read
	jmp	still
;--------------------------------------
no_set_lba_read:
	cmp	ah,102	; SET PCI ACCESS
	jne	no_pci_d
;--------------------------------------
pcip:
	btc	[pci_acc],0
	call	draw_infotext
	jmp	still
;--------------------------------------
no_pci_d:
	cmp	ah,103
	jne	no_pci_i
	jmp	pcip
;--------------------------------------
no_pci_i:
	cmp	ah,101
	jne	no_set_pci_acc
	call	_pci_acc
	jmp	still
;--------------------------------------
no_set_pci_acc:
	cmp	ah,42	; SET SYSTEM LANGUAGE BASE
	jnz	nosysm
	mov	eax,[syslang]
	dec	eax
	jz	still
	mov	[syslang],eax
	call	draw_infotext
;--------------------------------------
nosysm:
	cmp	ah,43
	jnz	nosysp
	mov	eax,[syslang]
	cmp	eax,6
	je	nosysp
	inc	eax
	mov	[syslang],eax
	call	draw_infotext
;--------------------------------------
nosysp:
	cmp	ah,41
	jnz	nosyss
	call	_syslang
	call	cleantxt
	call	loadtxt
	call	draw_window
;--------------------------------------
nosyss:
	cmp	ah,132	; SET MOUSE SPEED
	jnz	.nominus
	mov	eax,[mouse_speed]
	sub	eax,2
	cmp	eax,9
	jb	@f
	mov	eax,8
;--------------------------------------
@@:
	inc	eax
	mov	[mouse_speed],eax
	call	draw_infotext
;--------------------------------------
.nominus:
	cmp	ah,133
	jnz	.noplus
	mov	eax,[mouse_speed]
	cmp	eax,9
	jb	@f
	mov	eax,0
;--------------------------------------
@@:
	inc	eax
	mov	[mouse_speed],eax
	call	draw_infotext
;--------------------------------------
.noplus:
	cmp	ah,131
	jnz	.noapply
	call	_mouse_speed
;--------------------------------------    
.noapply:
mousedelay:
	cmp	ah,142        ; SET MOUSE DELAY
	jnz	.nominus
	mov	eax,[mouse_delay]
	sub	eax,2
	cmp	eax,0xfff
	jb	@f
	mov	eax,0xffe
;--------------------------------------
@@:
	inc	eax
	mov	[mouse_delay],eax
	call	draw_infotext
;--------------------------------------
.nominus:
	cmp	ah,143
	jnz	.noplus
	mov	eax,[mouse_delay]
	cmp	eax,0xfff
	jb	@f
	mov	eax,0
;--------------------------------------
@@:
	inc	eax
	mov	[mouse_delay],eax
	call	draw_infotext
;--------------------------------------
.noplus:
	cmp	ah,141
	jnz	.noapply
	call	_mouse_delay
;--------------------------------------
.noapply:
	cmp	ah,3	      ; SET KEYMAP
	jne	still
	call	_keyboard
	jmp	still
;-------------------------------------------------------------------------------
_keyboard:
	cmp	[keyboard],0	; english
	jnz	nosetkeyle
	
	mcall	21,2,1,en_keymap
	
	inc	ecx
	mcall	21,,,en_keymap_shift

	mcall	21,,9,1
	call	alt_gen
;--------------------------------------
nosetkeyle:
	cmp	[keyboard],1	; finnish
	jnz	nosetkeylfi

	mcall	21,2,1,fi_keymap
	
	inc	ecx
	mcall	21,,,fi_keymap_shift

	mcall	21,,9,2
	call	alt_gen
;--------------------------------------
nosetkeylfi:
	cmp  [keyboard],2	; german
	jnz  nosetkeylge

	mcall	21,2,1,ge_keymap
	
	inc	ecx
	mcall	21,,,ge_keymap_shift

	mcall	21,,9,3
	call	alt_gen
;--------------------------------------
nosetkeylge:
	cmp	[keyboard],3	; russian
	jnz	nosetkeylru
    
	mcall	21,2,1,ru_keymap
	
	inc	ecx
	mcall	21,,,ru_keymap_shift

	mcall	21,,9,4
	call	alt_gen
;--------------------------------------
nosetkeylru:
	cmp	[keyboard],4 	;french
	jnz	nosetkeylfr

	mcall	21,2,1,fr_keymap
	
	inc	ecx
	mcall	21,,,fr_keymap_shift

	inc  ecx
	mcall	21,,,fr_keymap_alt_gr

	mcall	21,,9,5
;--------------------------------------
nosetkeylfr:
	cmp	[keyboard],5	; estonian
	jnz	nosetkeylet

	mcall	21,2,1,et_keymap
	
	inc	ecx
	mcall	21,,,et_keymap_shift

	mcall	21,,9,6
	call	alt_gen
;--------------------------------------
nosetkeylet:
	ret
;-------------------------------------------------------------------------------
alt_gen:
	mcall	21,,3,alt_general
	ret
;-------------------------------------------------------------------------------
draw_buttons:
	pusha
	shl  ecx,16
	add  ecx,12
	mcall	8,<(350-57),(46+BBB)>

	inc	edx
	mcall	,<(350-85),9>

	inc	edx
	mcall	,<(350-73),9>

	popa
	ret
;-------------------------------------------------------------------------------
; ********************************************
; ******* WINDOW DEFINITIONS AND DRAW  *******
; ********************************************
draw_window:
	pusha
	mcall	12,1

	xor	eax,eax       ; DRAW WINDOW
	xor	esi,esi
	mcall	,<40,(355+BBB)>,<40,(12*15)>,0xB4111199,,title

	mcall	8,<(350-85),100>,<(5+14*8),12>,100,0x005588dd	; APPLY ALL

	add	ecx,16*65536	      ; SAVE ALL
	dec	edx
	mcall

	mov	esi,0x5580c0

	mov	edx,41
	mov	ecx,5+0*8
	call	draw_buttons

	mov	edx,3
	mov	ecx,5+2*8
	call	draw_buttons

	mov	edx,91
	mov	ecx,5+4*8
	call	draw_buttons

	mov	edx,101
	mov	ecx,5+6*8
	call	draw_buttons

	mov	edx,131
	mov	ecx,5+8*8
	call	draw_buttons

	mov	edx,141
	mov	ecx,5+10*8
	call	draw_buttons

	call	draw_infotext

	mcall	12,2
	popa
	ret
;-------------------------------------------------------------------------------
draw_infotext:
	pusha
	mov	eax,[keyboard]       ; KEYBOARD
	test	eax,eax
	jnz	noen
	mov	[text00+LLL*1+28],dword 'ENGL'
	mov	[text00+LLL*1+32],dword 'ISH '
;--------------------------------------
noen:
	cmp	eax,1
	jnz	nofi
	mov	[text00+LLL*1+28],dword 'FINN'
	mov	[text00+LLL*1+32],dword 'ISH '
;--------------------------------------
nofi:
	cmp	eax,2
	jnz	noge
	mov	[text00+LLL*1+28],dword 'GERM'
	mov	[text00+LLL*1+32],dword 'AN  '
;--------------------------------------
noge:
	cmp	eax,3
	jnz	nogr
	mov	[text00+LLL*1+28],dword 'RUSS'
	mov	[text00+LLL*1+32],dword 'IAN '
;--------------------------------------
nogr:
	cmp	eax,4
	jnz	nofr
	mov	[text00+LLL*1+28],dword 'FREN'
	mov	[text00+LLL*1+32],dword 'CH  '
;--------------------------------------
nofr:
	cmp	eax,5
	jnz	noet
	mov	[text00+LLL*1+28],dword 'ESTO'
	mov	[text00+LLL*1+32],dword 'NIAN'
;--------------------------------------
noet:
	mov	eax,[syslang]		  ; SYSTEM LANGUAGE
	dec	eax
	test	eax,eax
	jnz	noen5
	mov	[text00+LLL*0+28],dword 'ENGL'
	mov	[text00+LLL*0+32],dword 'ISH '
;--------------------------------------
noen5:
	cmp	eax,1
	jnz	nofi5
	mov	[text00+LLL*0+28],dword 'FINN'
	mov	[text00+LLL*0+32],dword 'ISH '
;--------------------------------------
nofi5:
	cmp	eax,2
	jnz	noge5
	mov	[text00+LLL*0+28],dword 'GERM'
	mov	[text00+LLL*0+32],dword 'AN  '
;--------------------------------------
noge5:
	cmp	eax,3
	jnz	nogr5
	mov	[text00+LLL*0+28],dword 'RUSS'
	mov	[text00+LLL*0+32],dword 'IAN '
;--------------------------------------
nogr5:
	cmp	eax,4
	jne	nofr5
	mov	[text00+LLL*0+28],dword 'FREN'
	mov	[text00+LLL*0+32],dword 'CH  '
;--------------------------------------
nofr5:
	cmp	eax,5
	jne	noet5
	mov	[text00+LLL*0+28],dword 'ESTO'
	mov	[text00+LLL*0+32],dword 'NIAN'
;--------------------------------------
noet5:
	mov	eax,[lba_read]
	call	onoff				; LBA READ
	mov	[text00+LLL*2+28],ebx

	mov	eax,[pci_acc]
	call	onoff				; PCI ACCESS
	mov	[text00+LLL*3+28],ebx

	mov	eax,[mouse_speed]		; MOUSE SPEED
	add	al,48
	mov	[text00+LLL*4+28],al

	mov	eax,[mouse_delay]
	mov	esi,text00+LLL*5+32
	call	hexconvert			; MOUSE DELAY
	call	text_out
	popa
	ret
;-------------------------------------------------------------------------------
text_out:
	mcall	13,<165,85>,<0,(12*8)>,0x80111199	;0x80111199-19

	mov	edx,text00
	mov	ebx,3*65536+7
	mov	eax,4
	mov	ecx,0xffffff
	mov	esi,LLL
	mov	ebp,text1_strings
;--------------------------------------
newline:
	mcall
	add	ebx,8+8
	add	edx,esi
	dec	ebp
	jnz	newline

	mov	ebp,text2_strings
	add	ebx,8+8
;--------------------------------------
@@:
	mcall
	add	ebx,8+8
	add	edx,esi
	dec	ebp
	jnz	@b
	ret
;-------------------------------------------------------------------------------
hexconvert:	     ;converting dec to hex in ascii
	xor	ebx,ebx
	mov	bl,al
	and	bl,15
	add	ebx,hex
	mov	cl,[ebx]
	mov	[esi],cl
	shr	eax,4
	xor	ebx,ebx
	mov	bl,al
	and	bl,15
	add	ebx,hex
	mov	cl,[ebx]
	dec	esi
	mov	[esi],cl
	shr	eax,4
	xor	ebx,ebx
	mov	bl,al
	and	bl,15
	add	ebx,hex
	mov	cl,[ebx]
	dec	esi
	mov	[esi],cl
	ret
;-------------------------------------------------------------------------------
onoff:
	cmp	[syslang],4
	jne	norus1
	mov	ebx,'ДА  '
	cmp	eax,1
	je	exitsub
	mov	ebx,'НЕТ '
	ret
;--------------------------------------
norus1:
	mov	ebx,'ON  '
	cmp	eax,1
	je	exitsub
	mov	ebx,'OFF '
;--------------------------------------
exitsub:
	ret
;-------------------------------------------------------------------------------
_lba_read:
	mcall	21,11,[lba_read]
	ret
;-------------------------------------------------------------------------------
_pci_acc:
	mcall	21,12,[pci_acc]
	ret
;-------------------------------------------------------------------------------
_syslang:
	mcall	21,5,[syslang]
	ret
;-------------------------------------------------------------------------------
_mouse_speed:
	mcall	18,19,1,[mouse_speed]
	ret
;-------------------------------------------------------------------------------
_mouse_delay:
	mcall	18,19,3,[mouse_delay]
	ret
;-------------------------------------------------------------------------------
loadtxt:
	mov	edi,text00
	mov	ecx,LLL*(text1_strings + text2_strings)/4
	cmp	[syslang],4
	jne	norus

	mov	esi,textrus
	jmp	sload
;--------------------------------------
norus:
	mov	esi,texteng
;--------------------------------------
sload:
	rep	movsd
	ret
;-------------------------------------------------------------------------------
cleantxt:
	xor	eax,eax
	mov	ecx,LLL*(text1_strings + text2_strings)/4
	cld
	mov	edi,text00
	rep	stosd
	mov	[text00+1711],byte 'x'
	ret
;-------------------------------------------------------------------------------
; DATA AREA
count:		db 0x0
blinkpar:	dd 0x0
time:		dw 0x0
date:		dd 0x0
;-------------------------------------------------------------------------------
textrus:
	db 'Язык системы              : ENGLISH         - +   Применить'
	db 'Раскладка клавиатуры      : ENGLISH         - +   Применить'
	db 'Включить LBA              : OFF             - +   Применить'
	db 'Доступ к шине PCI         : OFF             - +   Применить'
	db 'Скорость курсора мыши     : 1               - +   Применить'
	db 'Задержка ускорения мыши   : 0x00a           - +   Применить'
	
	db 'ВНИМАНИЕ:                                    Применить все '
	db 'НЕ ЗАБУДЬТЕ СОХРАНИТЬ НАСТРОЙКИ              Сохранить все '
;-------------------------------------------------------------------------------
texteng:
	db 'SYSTEM LANGUAGE           : ENGLISH         - +     APPLY  '
	db 'KEYBOARD LAYOUT           : ENGLISH         - +     APPLY  '
	db 'LBA READ ENABLED          : OFF             - +     APPLY  '
	db 'PCI ACCESS FOR APPL.      : OFF             - +     APPLY  '
	db 'Mouse pointer speed       : 1               - +     APPLY  '
	db 'Mouse pointer delay       : 0x00a           - +     APPLY  '
text1_strings = 6

	db 'NOTE:                                           APPLY ALL  '
	db 'SAVE YOUR SETTINGS BEFORE QUITING KOLIBRI       SAVE ALL   '
text2_strings = 2
;-------------------------------------------------------------------------------
title	db 'SETUP',0

hex	db '0123456789ABCDEF'
;-------------------------------------------------------------------------------
include 'keymaps.inc'
;-------------------------------------------------------------------------------
read_fileinfo:
	dd 0
	dd 0
	dd 0
	dd 48
	dd keyboard
	db 0
	dd file_name
;-------------------------------------------------------------------------------
save_fileinfo:
	dd 2
	dd 0
	dd 0
	dd 48
	dd keyboard
file_name:	db '/sys/setup.dat',0
;-------------------------------------------------------------------------------
I_PARAM	dd 0
;-----------------------------------------------------------------------------
; Note to SVN revision 2299 - some parameters has not used,
; but keep the order of the parameter has always needed!
keyboard	dd 0x0
		dd 0	;midibase  - not use, but retained for backward compat.
		dd 0	;cdbase - not use, but retained for backward compat.
		dd 0	;sb16 - not use, but retained for backward compat.
syslang		dd 0x1
		dd 0	;hdbase - not use, but retained for backward compat.
		dd 0	;f32p - not use, but retained for backward compat.
		dd 0	;sound_dma - not use, but retained for backward compat.
lba_read	dd 0x1
pci_acc		dd 0x1
mouse_speed	dd 0x3
mouse_delay	dd 0x10
;-----------------------------------------------------------------------------
IM_END:
;-----------------------------------------------------------------------------
align 4
text00:
	rb	LLL*(text1_strings + text2_strings)
;-----------------------------------------------------------------------------
align 4
	rb	0x1000
stack_area:
;-----------------------------------------------------------------------------
;table_area:
I_END:
;-------------------------------------------------------------------------------