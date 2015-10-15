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
dd boot_param
dd 0

include '../../../macros.inc'
;---------------------------------------------------------------
set_language_and_exit:
	mcall	26,2,9
	cmp	eax,1
	je	russian
	xor	eax,eax
@@:
	mov	[keyboard],eax
	call	_keyboard
	jmp	close
russian:
	mov	eax,3
	jmp	@b
;---------------------------------------------------------------
set_syslanguage_and_exit:
	mcall	26,5
	cmp	eax,6
	jne	@f
	xor	eax,eax
@@:
	inc	eax
	mov	[syslang],eax
	call	_syslang
	jmp	close
;---------------------------------------------------------------
apply_all_and_exit:
	mov	byte[fileinfo],0
	mcall	70,fileinfo
	call	apply_all
	jmp	close
;---------------------------------------------------------------
apply_all:
	call	fontApply
	call	_syslang
	call	_lba_read
	call	_pci_acc
	call	_speaker_mute
	call	_keyboard
	ret
;---------------------------------------------------------------
_syslang:
	mcall	21,5,[syslang]
	jmp	loadtxt
;---------------------------------------------------------------
_lba_read:
	mcall	21,11,[lba_read]
	ret
;---------------------------------------------------------------
_pci_acc:
	mcall	21,12,[pci_acc]
	ret
;---------------------------------------------------------------
fontApply:
	mcall	48,10,[fontSmoothing]
	ret
;---------------------------------------------------------------
_speaker_mute:
	mcall	18,8,1
	cmp	[speaker_mute],eax
	je	@f
	inc	ecx
	mcall	18
@@:
	ret
;---------------------------------------------------------------
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
START:
	mov	eax,boot_param
	cmp	[eax],dword 'SLAN'
	je	set_syslanguage_and_exit

	cmp	[eax],dword 'LANG'
	je	set_language_and_exit

	cmp	[eax],dword 'BOOT'
	je	apply_all_and_exit
; get current settings
	mcall	26,2,9
	dec	eax
	mov	[keyboard],eax

	mcall	26,5
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
	lea	esi,[eax*8+langs-8]
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
saveAll:
	mov	byte[fileinfo],2
	mcall	70,fileinfo
	ret
language1:
	dec	[syslang]
	jnz	@f
	mov	[syslang],6
	ret
language2:
	inc	[syslang]
	cmp	[syslang],7
	jnz	@f
	mov	[syslang],1
	ret
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
;---------------------------------------------------------------
loadtxt:
	cmp	[syslang],4
	jne	@f
	mov	[text],textrus
	ret
@@:
	cmp	[syslang],6
	jne	@f
	mov	[text],textet
	ret
@@:
	mov	[text],texteng
	ret
;---------------------------------------------------------------
onoff:
	cmp	[syslang],4
	jne	norus1
	mov	ebx,'ДА  '
	cmp	eax,1
	je	exitsub
	mov	ebx,'НЕТ '
	ret
norus1:
	cmp	[syslang],6
	jne	noet1
	mov	ebx,'SEES'
	cmp	eax,1
	je	exitsub
	mov	ebx,'V─L.'
	ret
noet1:
	mov	ebx,'ON  '
	cmp	eax,1
	je	exitsub
	mov	ebx,'OFF '
exitsub:
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

setup.dat:	; file structure
syslang 	dd 0
keyboard	dd 0
lba_read	dd 0
pci_acc 	dd 0
speaker_mute	dd 0
fontSmoothing	dd 0

fileinfo:
	dd 0
	dd 0
	dd 0
	dd 4*6
	dd setup.dat
	db '/SYS/SETTINGS/SETUP.DAT',0

title	db 'System settings',0
hex	db '0123456789ABCDEF'

LLL = 56
stringsAmount = 6

align 4
text	dd 0
langs:
db 'ENGLISH FINNISH GERMAN  RUSSIAN FRENCH  ESTONIANBELGIAN ITALIAN '

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
db 'LBA read enabled          :              -  +    Apply  '
db 'PCI access for appl.      :              -  +    Apply  '
db 'SPEAKER disabled          :              -  +    Apply  '
db 'Font smoothing            :              -  +    Apply  '

db 'NOTE:                                        Apply all  '
db 'SAVE YOUR SETTINGS BEFORE QUITING KOLIBRI    Save all   '

textet:
db 'S№steemi keel             :              <  >   Kinnita '
db 'Klaviatuuri paigutus      :              <  >   Kinnita '
db 'LBA lugemine lubatud      :              -  +   Kinnita '
db 'PCI juurdepффs programm.  :              -  +   Kinnita '
db 'SPEAKER disabled          :              -  +   Kinnita '
db 'Font smoothing            :              -  +   Kinnita '

db 'M─RKUS:                                    Kinnita kїik '
db 'SALVESTA SEADED ENNE KOLIBRIST V─LJUMIST   Salvesta kїik'

include 'keymaps.inc'
IM_END:
boot_param:
	rb 1024
stack_area:
I_END: