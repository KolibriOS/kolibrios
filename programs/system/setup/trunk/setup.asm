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
dd I_END
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
	mov	ecx,langMarks.size/2
	mov	edi,langMarks
	repnz scasw
	jnz	@f
	neg	ecx
	add	ecx,langMarks.size/2
	mcall	21,5
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

; Set font size
	invoke	ini.get_int, sz_ini, sz_system, sz_fontSize, 9
	mov	ecx,eax
	mcall	48,12

; Set mouse speed
	invoke	ini.get_int, sz_ini, sz_mouse, sz_speed, 1
	mov	edx,eax
	mcall	18,19,1

; Set mouse acceleration
	invoke	ini.get_int, sz_ini, sz_mouse, sz_acceleration, 1
	mov	edx,eax
	mcall	18,19,3
	
; Set mouse double click delay
	invoke	ini.get_int, sz_ini, sz_mouse, sz_double_click_delay, 1
	mov	edx,eax
	mcall	18,19,7

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

	mcall	48,11
	mov	[fontSize],eax

loadtxt:
	cmp	[syslang],3
	jz	.ru
	cmp	[syslang],5
	jz	.et
	mov	[text],texteng
	jmp	draw_window
.et:
	mov	[text],textet
	jmp	draw_window
.ru:
	mov	[text],textrus

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
	mcall	12,2

draw_infotext:
	mov	eax,[syslang]
	mov	edi,[text]
	lea	esi,[eax*8+langs]
	add	edi,28
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

	mov	eax,[fontSize]
	mov	bl, 10
	div	bl
	add	ax, '00'
	mov	[edi+LLL*4],ax
; draw text
	mcall	13,<342,96>,32*stringsAmount,80111199h
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
	jc	@f
	mov	[syslang],0
@@:
	pop	eax
	jmp	loadtxt
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
	ret
fontSize1:
	cmp	[fontSize],10
	jc	@f
	dec	[fontSize]
@@:
	ret
fontSize2:
	inc	[fontSize]
	ret
apply_all:
	call	_lba_read
	call	_pci_acc
	call	_speaker_mute
	call	fontApply
	call	fontSizeApply
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
fontSizeApply:
	mcall	48,12,[fontSize]
	ret
_speaker_mute:
	mcall	18,8,1
	cmp	[speaker_mute],eax
	jz	@f
	inc	ecx
	mcall	18
@@:
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
	mov	ebx,'çÖí '
	test	eax,eax
	jz	@f
	mov	ebx,'ÑÄ  '
	ret
.et:
	mov	ebx,'VƒL.'
	test	eax,eax
	jz	@f
	mov	ebx,'SEES'
@@:
	ret
;---------------------------------------------------------------
saveAll:
; system language
	mov	eax,[syslang]
	mov	ax, word[eax*2+langMarks]
	mov	[param],eax
	invoke	ini.set_str, sz_ini, sz_system, sz_language, param, 2

; font size
	invoke	ini.set_int, sz_ini, sz_system, sz_fontSize, [fontSize]

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
; DATA
align 4
buttonTab:	; button handler pointers: -,+,apply
	dd close
	dd saveAll
	dd apply_all
	dd language1
	dd language2
	dd _syslang
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
	dd fontSize1
	dd fontSize2
	dd fontSizeApply

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
sz_fontSize	db "font height",0
sz_fontSmooth	db "font smoothing",0
sz_speaker	db "speaker mute",0

sz_mouse	db "mouse",0
sz_speed	db "speed",0
sz_acceleration	db "acceleration",0
sz_double_click_delay	db "double_click_delay",0

sz_low_level	db "low-level",0
sz_lba		db "LBA",0
sz_pci		db "PCI",0

LLL = 56
stringsAmount = 6

align 4
langs:
db 'ENGLISH FINNISH GERMAN  RUSSIAN FRENCH  ESTONIANSPANISH ITALIAN '
sz langMarks, 'en','fi','de','ru','fr','et','sp','it'

textrus:
db 'üßÎ™ ·®·‚•¨Î              :              <  >  è‡®¨•≠®‚Ï'
db 'Ç™´ÓÁ®‚Ï LBA              :              -  +  è‡®¨•≠®‚Ï'
db 'ÑÆ·‚„Ø ™ Ë®≠• PCI         :              -  +  è‡®¨•≠®‚Ï'
db 'ÇÎ™´ÓÁ®‚Ï SPEAKER         :              -  +  è‡®¨•≠®‚Ï'
db 'ë£´†¶®¢†≠®• Ë‡®‰‚Æ¢       :              -  +  è‡®¨•≠®‚Ï'
db 'ÇÎ·Æ‚† Ë‡®‰‚Æ¢            :              -  +  è‡®¨•≠®‚Ï'

db 'ÇçàåÄçàÖ:                                  è‡®¨•≠®‚Ï ¢·•'
db 'çÖ áÄÅìÑúíÖ ëéïêÄçàíú çÄëíêéâäà            ëÆÂ‡†≠®‚Ï ¢·•'

texteng:
db 'System language           :              <  >    Apply  '
db 'Allow LBA access          :              -  +    Apply  '
db 'Allow PCI access          :              -  +    Apply  '
db 'Disable SPEAKER           :              -  +    Apply  '
db 'Font smoothing            :              -  +    Apply  '
db 'Font height               :              -  +    Apply  '

db 'NOTE:                                        Apply all  '
db 'SAVE YOUR SETTINGS BEFORE QUITING KOLIBRI    Save all   '

textet:
db 'S¸steemi keel             :              <  >   Kinnita '
db 'LBA lugemine lubatud      :              -  +   Kinnita '
db 'PCI juurdep‰‰s programm.  :              -  +   Kinnita '
db 'Disable SPEAKER           :              -  +   Kinnita '
db 'Font smoothing            :              -  +   Kinnita '
db 'Font height               :              -  +   Kinnita '

db 'MƒRKUS:                                    Kinnita kıik '
db 'SALVESTA SEADED ENNE KOLIBRIST VƒLJUMIST   Salvesta kıik'

IM_END:

text	dd  ?

syslang 	dd  ?
lba_read	dd  ?
pci_acc 	dd  ?
speaker_mute	dd  ?
fontSmoothing	dd  ?
fontSize	dd  ?

param:
	rb 1024
I_END:
