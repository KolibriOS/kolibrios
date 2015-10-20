;;;;;;;;;;;;;;;;;;;;;;;
;;  SYSTEM SETTINGS  ;;
;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""
use32
org 0

db 'MENUET01'
dd 1
dd START
dd IM_END
dd I_END
dd stack_area
dd param
dd 0

include '../../../proc32.inc'
include '../../../macros.inc'
include '../../../dll.inc'
;---------------------------------------------------------------
BootSettings:
; Set system language
	mov	word[param],0
	invoke	ini.get_str, sz_ini, sz_system, sz_language, param, 2, 0
	mov	ax, [param]
	or	ax, 0x2020	; convert to lowercase
	mov	ecx,8
	mov	edi,langMarks
	repnz scasw
	jnz	@f
	neg	ecx
	add	ecx,8
	mcall	21,5
@@:

; Set keyboard layout
	mov	word[param],0
	invoke	ini.get_str, sz_ini, sz_system, sz_keyboard, param, 2, 0
	mov	ax, [param]
	or	ax, 0x2020
	mov	ecx,8
	mov	edi,langMarks
	repnz scasw
	jnz	@f
	neg	ecx
	add	ecx,7
	mov	[keyboard],ecx
	call	_keyboard
@@:

; Set font smoothing
	mov	dword[param],0
	invoke	ini.get_str, sz_ini, sz_system, sz_fontSmooth, param, 4, 0
	xor	ecx,ecx
	mov	eax,[param]
	or	eax,0x20202020
	cmp	eax,'off '
	jz	@f
	inc	ecx
	cmp	eax,'on  '
	jz	@f
	cmp	eax,'sbp '
	jnz	.skipFont
	inc	ecx
@@:
	mcall	48,10
.skipFont:

; Enable/disable system speaker
	mov	dword[param],0
	invoke	ini.get_str, sz_ini, sz_system, sz_speaker, param, 4, 0
	mov	eax,[param]
	or	eax,0x20202020
	cmp	eax,'off '
	jz	@f
	cmp	eax,'on  '
	jnz	.skipSpeaker
	inc	[speaker_mute]
@@:
	call	_speaker_mute
.skipSpeaker:

; Set mouse speed
	invoke	ini.get_int, sz_ini, sz_mouse, sz_speed, 1
	mov	edx,eax
	mcall	18,19,1

; Set mouse delay
	invoke	ini.get_int, sz_ini, sz_mouse, sz_delay, 1
	mov	edx,eax
	mcall	18,19,3

; Enable/disable LBA access for applications
	mov	dword[param],0
	invoke	ini.get_str, sz_ini, sz_low_level, sz_lba, param, 4, 0
	xor	ecx,ecx
	mov	eax,[param]
	or	eax,0x20202020
	cmp	eax,'off '
	jz	@f
	cmp	eax,'on  '
	jnz	.skipLBA
	inc	ecx
@@:
	mcall	21,11
.skipLBA:

; Enable/disable PCI access for applications
	mov	dword[param],0
	invoke	ini.get_str, sz_ini, sz_low_level, sz_pci, param, 4, 0
	xor	ecx,ecx
	mov	eax,[param]
	or	eax,0x20202020
	cmp	eax,'off '
	jz	@f
	cmp	eax,'on  '
	jnz	close
	inc	ecx
@@:
	mcall	21,12
	jmp	close
;---------------------------------------------------------------
START:
	mcall	68,11
	stdcall dll.Load, @IMPORT
	push	eax
	test	eax,eax
	jnz	close

	cmp	[param],dword 'BOOT'
	jz	BootSettings
	pop	eax
; get current settings
	mcall	26,2,9
	dec	eax
	mov	[keyboard],eax

	mcall	26,5
	dec	eax
	mov	[syslang],eax

	mcall	26,11
	mov	[lba_read],eax

	mcall	26,12
	mov	[pci_acc],eax

	mcall	18,8,1
	mov	[speaker_mute],eax

	mcall	48,9
	mov	[fontSmoothing],eax

	call	loadtxt

draw_infotext:
	mov	eax,[syslang]
	mov	edi,[text]
	lea	esi,[eax*8+langs]
	add	edi,28
	movsd
	movsd

	mov	eax,[keyboard]
	add	edi,LLL-8
	lea	esi,[eax*8+langs]
	movsd
	movsd
	add	edi,LLL-8

	mov	eax,[lba_read]
	call	onoff
	mov	[edi],ebx

	mov	eax,[pci_acc]
	call	onoff
	mov	[edi+LLL],ebx

	mov	eax,[speaker_mute]
	call	onoff
	mov	[edi+LLL*2],ebx

	mov	ebx,'SUBP'
	mov	ecx,'IXEL'
	cmp	[fontSmoothing],2
	jz	@f
	mov	eax,[fontSmoothing]
	call	onoff
	mov	ecx,'    '
@@:
	mov	[edi+LLL*3],ebx
	mov	[edi+LLL*3+4],ecx

draw_window:
	mcall	12,1
	mov	ecx,50*65536+32*(4+stringsAmount)
	mcall	0,<50,700>,,0xB4111199,0,title
; Main buttons
	mov	eax,8
	mov	ecx,6*65536+26
	mov	edx,4
	mov	esi,0x5580c0
	mov	ebp,stringsAmount
@@:
	mcall	,<490,24>
	inc	edx
	mcall	,<526,24>
	inc	edx
	mcall	,<562,120>
	inc	edx
	add	ecx,32*65536
	dec	ebp
	jnz	@b
; APPLY ALL
	add	ecx,32*65536
	mcall	,<514,168>,,3,0x005588dd
; SAVE ALL
	add	ecx,32*65536
	dec	edx
	mcall
; text
	mov	eax,4
	mov	ebx,6*65536+11
	mov	ecx,1ffffffh
	mov	edx,[text]
	mov	esi,LLL
	mov	ebp,stringsAmount
newline:
	mcall
	add	ebx,32
	add	edx,esi
	dec	ebp
	jnz	newline

	mov	ebp,2
	add	ebx,32
@@:
	mcall
	add	ebx,32
	add	edx,esi
	dec	ebp
	jnz	@b

	mcall	12,2

still:
	mcall	10
	cmp	eax,1
	jz	draw_window

	cmp	eax,2
	jz	key

	cmp	eax,3
	jz	button

	jmp	still
;---------------------------------------------------------------
key:
	mcall	2
	jmp	still
;---------------------------------------------------------------
button:
	mcall	17
	shr	eax,8
	call	dword[eax*4+buttonTab-4]
	jmp	draw_infotext
close:
	pop	eax
	mcall	-1
language1:
	dec	[syslang]
	jns	@f
	mov	[syslang],7
	jmp	@f
language2:
	inc	[syslang]
	cmp	[syslang],8
	jnz	@f
	mov	[syslang],0
@@:
	jmp	loadtxt
layout1:
	dec	[keyboard]
	jns	@f
	mov	[keyboard],7
	ret
layout2:
	inc	[keyboard]
	cmp	[keyboard],8
	jnz	@f
	mov	[keyboard],0
	ret
LBA1:
	btr	[lba_read],0
	ret
LBA2:
	bts	[lba_read],0
	ret
PCI1:
	btr	[pci_acc],0
	ret
PCI2:
	bts	[pci_acc],0
	ret
SPEAKER1:
	btr	[speaker_mute],0
	ret
SPEAKER2:
	bts	[speaker_mute],0
	ret
font1:
	cmp	[fontSmoothing],0
	jz	@f
	dec	[fontSmoothing]
	ret
font2:
	cmp	[fontSmoothing],2
	jz	@f
	inc	[fontSmoothing]
@@:
	ret
apply_all:
	call	fontApply
	call	_syslang
	call	_lba_read
	call	_pci_acc
	call	_speaker_mute
	call	_keyboard
	ret
_syslang:
	mov	ecx,[syslang]
	inc	ecx
	mcall	21,5
	ret
_lba_read:
	mcall	21,11,[lba_read]
	ret
_pci_acc:
	mcall	21,12,[pci_acc]
	ret
fontApply:
	mcall	48,10,[fontSmoothing]
	ret
_speaker_mute:
	mcall	18,8,1
	cmp	[speaker_mute],eax
	je	@b
	inc	ecx
	mcall	18
_keyboard:
	mov	ebp,[keyboard]
	mov	edx,[ebp*4+keymapTab]
	mcall	21,2,1
	inc	ecx
	mov	edx,[ebp*4+shiftKeymapTab]
	mcall	21
	inc	ecx
	mov	edx,[ebp*4+altKeymapTab]
	mcall	21
	mov	edx,ebp
	inc	edx
	mov	cl, 9
	mcall	21
	ret
;---------------------------------------------------------------
loadtxt:
	cmp	[syslang],3
	jz	.ru
	cmp	[syslang],5
	jz	.et
	mov	[text],texteng
	ret
.ru:
	mov	[text],textrus
	ret
.et:
	mov	[text],textet
	ret
;---------------------------------------------------------------
onoff:
	cmp	[syslang],3
	jz	.ru
	cmp	[syslang],5
	jz	.et
	mov	ebx,'OFF '
	test	eax,eax
	jz	@f
	mov	ebx,'ON  '
	ret
.ru:
	mov	ebx,'НЕТ '
	test	eax,eax
	jz	@f
	mov	ebx,'ДА  '
	ret
.et:
	mov	ebx,'V─L.'
	test	eax,eax
	jz	@f
	mov	ebx,'SEES'
@@:
	ret
;---------------------------------------------------------------
saveAll:
; system language
	mov	eax,[syslang]
	mov	ax, [eax*2+langMarks]
	mov	[param],eax
	invoke	ini.set_str, sz_ini, sz_system, sz_language, param, 2

; keyboard layout
	mov	eax,[keyboard]
	mov	ax, [eax*2+langMarks]
	mov	[param],eax
	invoke	ini.set_str, sz_ini, sz_system, sz_keyboard, param, 2

; font smoothing
	mov	dword[param],'off'
	cmp	[fontSmoothing],0
	jz	@f
	mov	dword[param],'on '
	cmp	[fontSmoothing],1
	jz	@f
	mov	dword[param],'sbp'
@@:
	invoke	ini.set_str, sz_ini, sz_system, sz_fontSmooth, param, 3

; system speaker
	mov	dword[param],'off'
	cmp	[speaker_mute],0
	jz	@f
	mov	dword[param],'on '
@@:
	invoke	ini.set_str, sz_ini, sz_system, sz_speaker, param, 3

; LBA access for applications
	mov	dword[param],'off'
	cmp	[lba_read],0
	jz	@f
	mov	dword[param],'on '
@@:
	invoke	ini.set_str, sz_ini, sz_low_level, sz_lba, param, 3

; PCI access for applications
	mov	dword[param],'off'
	cmp	[pci_acc],0
	jz	@f
	mov	dword[param],'on '
@@:
	invoke	ini.set_str, sz_ini, sz_low_level, sz_pci, param, 3
	ret
;---------------------------------------------------------------
align 4
buttonTab:	; button handler pointers: -,+,apply
	dd close
	dd saveAll
	dd apply_all
	dd language1
	dd language2
	dd _syslang
	dd layout1
	dd layout2
	dd _keyboard
	dd LBA1
	dd LBA2
	dd _lba_read
	dd PCI1
	dd PCI2
	dd _pci_acc
	dd SPEAKER1
	dd SPEAKER2
	dd _speaker_mute
	dd font1
	dd font2
	dd fontApply
keymapTab:
	dd en_keymap
	dd fi_keymap
	dd ge_keymap
	dd ru_keymap
	dd fr_keymap
	dd et_keymap
	dd be_keymap
	dd it_keymap
shiftKeymapTab:
	dd en_keymap_shift
	dd fi_keymap_shift
	dd ge_keymap_shift
	dd ru_keymap_shift
	dd fr_keymap_shift
	dd et_keymap_shift
	dd be_keymap_shift
	dd it_keymap_shift
altKeymapTab:
	dd alt_general
	dd alt_general
	dd alt_general
	dd alt_general
	dd fr_keymap_alt_gr
	dd alt_general
	dd be_keymap_alt_gr
	dd it_keymap_alt_gr

syslang 	dd 0
keyboard	dd 0
lba_read	dd 0
pci_acc 	dd 0
speaker_mute	dd 0
fontSmoothing	dd 0

@IMPORT:
library libini, 'libini.obj'
import	libini, \
	ini.get_str, 'ini_get_str',\
	ini.get_int, 'ini_get_int',\
	ini.set_str, 'ini_set_str',\
	ini.set_int, 'ini_set_int'

title	db "System settings",0
sz_ini	db "/sys/settings/system.ini",0

sz_system	db "system",0
sz_language	db "language",0
sz_keyboard	db "keyboard",0
sz_fontSmooth	db "font smoothing",0
sz_speaker	db "speaker mute",0

sz_mouse	db "mouse",0
sz_speed	db "speed",0
sz_delay	db "delay",0

sz_low_level	db "low-level",0
sz_lba		db "LBA",0
sz_pci		db "PCI",0

LLL = 56
stringsAmount = 6

align 4
text	dd 0
langs:
db 'ENGLISH FINNISH GERMAN  RUSSIAN FRENCH  ESTONIANBELGIAN ITALIAN '
langMarks:
db	'enfiderufretesit'

textrus:
db 'Язык системы              :              <  >  Применить'
db 'Раскладка клавиатуры      :              <  >  Применить'
db 'Включить LBA              :              -  +  Применить'
db 'Доступ к шине PCI         :              -  +  Применить'
db 'Выключить SPEAKER         :              -  +  Применить'
db 'Сглаживание шрифтов       :              -  +  Применить'

db 'ВНИМАНИЕ:                                  Применить все'
db 'НЕ ЗАБУДЬТЕ СОХРАНИТЬ НАСТРОЙКИ            Сохранить все'

texteng:
db 'System language           :              <  >    Apply  '
db 'Keyboard layout           :              <  >    Apply  '
db 'Allow LBA access          :              -  +    Apply  '
db 'Allow PCI access          :              -  +    Apply  '
db 'Disable SPEAKER           :              -  +    Apply  '
db 'Font smoothing            :              -  +    Apply  '

db 'NOTE:                                        Apply all  '
db 'SAVE YOUR SETTINGS BEFORE QUITING KOLIBRI    Save all   '

textet:
db 'S№steemi keel             :              <  >   Kinnita '
db 'Klaviatuuri paigutus      :              <  >   Kinnita '
db 'LBA lugemine lubatud      :              -  +   Kinnita '
db 'PCI juurdepффs programm.  :              -  +   Kinnita '
db 'Disable SPEAKER           :              -  +   Kinnita '
db 'Font smoothing            :              -  +   Kinnita '

db 'M─RKUS:                                    Kinnita kїik '
db 'SALVESTA SEADED ENNE KOLIBRIST V─LJUMIST   Salvesta kїik'

include 'keymaps.inc'
IM_END:
param:
	rb 1024
stack_area:
I_END: