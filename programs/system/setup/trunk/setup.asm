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

;******************************************************************************
  use32
  org	  0x0
  db	  'MENUET01'   ; 8 byte identifier
  dd	  0x01		 ; header version
  dd	  START        ; pointer to program start
  dd	  I_END        ; size of image
  dd	  0x4000      ; reguired amount of memory
  dd	  0x4000      ; stack pointer (esp)
  dd	  I_PARAM,0    ; parameters, reserved
  include '..\..\..\macros.inc'
;******************************************************************************

LLL equ (56+3)
BBB equ 25

;******************************************************************************
apply_all:

    call _pci_acc    ;12
    call _syslang    ;5
    call _keyboard    ;2
    call _mouse_speed
    call _mouse_delay

ret
;-------------------------------------------------------------------------------
;******************************************************************************
apply_all_and_exit:

	mcall	70, read_fileinfo
	call	apply_all
	jmp	close

;******************************************************************************
set_language_and_exit:

	mcall	26, 2, 9

	cmp	eax, 1
	je	russian

	xor	eax,eax

set_lang_now:
	mov	[keyboard], eax
	call	_keyboard
	jmp	close

russian:
	mov	eax, 3
	jmp	set_lang_now

set_syslanguage_and_exit:

	mcall	26, 5
	cmp	eax,6
	jne	temp
	xor	eax,eax

temp:
	inc	eax
	mov	[syslang], eax
	call	_syslang
	jmp	close



get_setup_values:

	mcall	26, 2, 9
	dec	eax
	mov	[keyboard], eax

	mcall	26, 5
	mov	[syslang], eax

	mcall	26, 11
	mov	[lba_read], eax

	mcall	26, 12
	mov	[pci_acc], eax

	mcall	18, 19, 0
	mov	[mouse_speed], eax

	mcall	18, 19, 2
	mov	[mouse_delay], eax

	ret

;******************************************************************************

START:
	cmp	[I_PARAM], 'SLAN'
	je	set_syslanguage_and_exit

	cmp	[I_PARAM], 'LANG'
	je	set_language_and_exit

	cmp	[I_PARAM], 'BOOT'
	je	apply_all_and_exit

	call	get_setup_values
	call	loadtxt

red:
	call draw_window

still:

 sysevent:
	mov	eax, 23
	mov	ebx, 8	      ; wait here for event with timeout
	mcall

	cmp	eax, 1
	jz	red

	cmp	eax, 2
	jz	key

	cmp	eax, 3
	jz	button

	jmp	still


  key:
	;mov    eax,2
	mcall

	jmp	still

  button:

    mov  eax,17
    mcall

    cmp  ah,99
    jne  nosaveall
    mcall 70,save_fileinfo

    jmp  still
nosaveall:
    cmp  ah,100
    jne  no_apply_all
    call apply_all
    jmp  still
no_apply_all:

    cmp  ah,1		; CLOSE APPLICATION
    jne  no_close
close:
    or	       eax,-1
    mcall
  no_close:


    cmp  ah,4		; SET KEYBOARD
    jnz  nokm
    mov  eax,[keyboard]
    test eax,eax
    je	       downuplbl
    dec  eax
    jmp  nodownup
   downuplbl:
    mov  eax,5
   nodownup:
    mov  [keyboard],eax
    call draw_infotext
  nokm:
    cmp  ah,5
    jnz  nokp
    mov  eax,[keyboard]
    cmp  eax,5
    je	       updownlbl
    inc  eax
    jmp  noupdown
   updownlbl:
    xor  eax,eax
   noupdown:
    mov  [keyboard],eax
    call draw_infotext
  nokp:


    cmp  ah,92	       ; SET LBA READ
    jne  no_lba_d
  slbal:
    btc  [lba_read],0
    call draw_infotext
    jmp  still
   no_lba_d:
    cmp  ah,93
    jne  no_lba_i
    jmp  slbal
  no_lba_i:
    cmp  ah,91
    jne  no_set_lba_read
    call _lba_read
    jmp  still
   no_set_lba_read:


    cmp  ah,102       ; SET PCI ACCESS
    jne  no_pci_d
  pcip:
    btc  [pci_acc],0
    call draw_infotext
    jmp  still
  no_pci_d:
    cmp  ah,103
    jne  no_pci_i
    jmp  pcip
   no_pci_i:
    cmp  ah,101
    jne  no_set_pci_acc
    call _pci_acc
    jmp  still
  no_set_pci_acc:


    cmp  ah,42		; SET SYSTEM LANGUAGE BASE
    jnz  nosysm
    mov  eax,[syslang]
    dec  eax
    jz	       still
    mov  [syslang],eax
    call draw_infotext
  nosysm:
    cmp  ah,43
    jnz  nosysp
    mov  eax,[syslang]
    cmp  eax,6
    je	       nosysp
    inc  eax
    mov  [syslang],eax
    call draw_infotext
  nosysp:
    cmp  ah,41
    jnz  nosyss
    call _syslang
    call cleantxt
    call loadtxt
    call draw_window
  nosyss:
    cmp  ah,132        ; SET MOUSE SPEED
    jnz  .nominus
    mov  eax,[mouse_speed]
    sub  eax,2
    cmp  eax,9
    jb	      @f
    mov  eax,8
@@:
    inc  eax
    mov  [mouse_speed],eax
    call draw_infotext
  .nominus:
    cmp  ah,133
    jnz  .noplus
    mov  eax,[mouse_speed]
    cmp  eax,9
    jb	      @f
    mov  eax,0
@@:
    inc  eax
    mov  [mouse_speed],eax
    call draw_infotext
  .noplus:
    cmp  ah,131
    jnz  .noapply
    call _mouse_speed
  .noapply:
 mousedelay:
    cmp  ah,142        ; SET MOUSE DELAY
    jnz  .nominus
    mov  eax,[mouse_delay]
    sub  eax,2
    cmp  eax,0xfff
    jb	      @f
    mov  eax,0xffe
@@:
    inc  eax
    mov  [mouse_delay],eax
    call draw_infotext
  .nominus:
    cmp  ah,143
    jnz  .noplus
    mov  eax,[mouse_delay]
    cmp  eax,0xfff
    jb	      @f
    mov  eax,0
@@:
    inc  eax
    mov  [mouse_delay],eax
    call draw_infotext
  .noplus:
    cmp  ah,141
    jnz  .noapply
    call _mouse_delay
  .noapply:

    cmp  ah,3	      ; SET KEYMAP
    jne  still
    call _keyboard
    jmp  still

  _keyboard:
    cmp [keyboard],0
    jnz  nosetkeyle
    mov  eax,21       ; english
    mov  ebx,2
    mov  ecx,1
    mov  edx,en_keymap
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,en_keymap_shift
    mcall
    mov  eax,21
    mov  ecx,9
    mov  edx,1
    mcall
    call alt_gen
  nosetkeyle:
    cmp  [keyboard],1
    jnz  nosetkeylfi
    mov  eax,21       ; finnish
    mov  ebx,2
    mov  ecx,1
    mov  edx,fi_keymap
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,fi_keymap_shift
    mcall
    mov  eax,21
    mov  ecx,9
    mov  edx,2
    mcall
    call alt_gen
  nosetkeylfi:
    cmp  [keyboard],2
    jnz  nosetkeylge
    mov  eax,21       ; german
    mov  ebx,2
    mov  ecx,1
    mov  edx,ge_keymap
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,ge_keymap_shift
    mcall
    mov  eax,21
    mov  ecx,9
    mov  edx,3
    mcall
    call alt_gen
  nosetkeylge:
    cmp  [keyboard],3
    jnz  nosetkeylru
    mov  eax,21       ; russian
    mov  ebx,2
    mov  ecx,1
    mov  edx,ru_keymap
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,ru_keymap_shift
    mcall
    call alt_gen
    mov  eax,21
    mov  ecx,9
    mov  edx,4
    mcall
  nosetkeylru:
    cmp  [keyboard],4	     ;french
    jnz  nosetkeylfr
    mov  eax,21
    mov  ebx,2
    mov  ecx,1
    mov  edx,fr_keymap
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,fr_keymap_shift
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,fr_keymap_alt_gr
    mcall
    mov  eax,21
    mov  ecx,9
    mov  edx,5
    mcall
  nosetkeylfr:
    cmp  [keyboard],5
    jnz  nosetkeylet
    mov  eax,21       ; estonian
    mov  ebx,2
    mov  ecx,1
    mov  edx,et_keymap
    mcall
    mov  eax,21
    inc  ecx
    mov  edx,et_keymap_shift
    mcall
    mov  eax,21
    mov  ecx,9
    mov  edx,6
    mcall
    call alt_gen
  nosetkeylet:
    ret

 alt_gen:
   mov eax,21
   mov ecx,3
   mov edx,alt_general
   mcall
   ret



draw_buttons:

    pusha

    shl  ecx,16
    add  ecx,12
    mov  ebx,(350-50)*65536+46+BBB

    mov  eax,8
    mcall

    mov  ebx,(350-79)*65536+9
    inc  edx
    mcall

    mov  ebx,(350-67)*65536+9
    inc  edx
    mcall

    popa
    ret



; ********************************************
; ******* WINDOW DEFINITIONS AND DRAW  *******
; ********************************************


draw_window:

    pusha

    mov  eax,12
    mov  ebx,1
    mcall

    xor  eax,eax       ; DRAW WINDOW
    mov  ebx,40*65536+355+BBB
    mov  ecx,40*65536+320
    mov  edx,0x94111199
    mov  edi,title
    mcall

    mov  eax,8		   ; APPLY ALL
    mov  ebx,(350-79)*65536+100
    mov  ecx,282*65536+12
    mov  edx,100
    mov  esi,0x005588dd
    mcall
    add  ecx,16*65536	      ; SAVE ALL
    dec  edx
    mcall

    mov  esi,0x5580c0

    mov  edx,41
    mov  ecx,43+8*8
    call draw_buttons

    mov  edx,3
    mov  ecx,43+10*8
    call draw_buttons

    mov  edx,91
    mov  ecx,43+16*8
    call draw_buttons

    mov  edx,101
    mov  ecx,43+18*8
    call draw_buttons

    mov  edx,131
    mov  ecx,43+24*8 ; 26
    call draw_buttons

    mov  edx,141
    mov  ecx,43+26*8 ; 26
    call draw_buttons

    call draw_infotext

    mov  eax,12
    mov  ebx,2
    mcall

    popa
    ret



draw_infotext:

    pusha

    mov  eax,[keyboard]       ; KEYBOARD
    test eax,eax
    jnz  noen
    mov  [text00+LLL*5+28],dword 'ENGL'
    mov  [text00+LLL*5+32],dword 'ISH '
  noen:
    cmp  eax,1
    jnz  nofi
    mov  [text00+LLL*5+28],dword 'FINN'
    mov  [text00+LLL*5+32],dword 'ISH '
  nofi:
    cmp  eax,2
    jnz  noge
    mov  [text00+LLL*5+28],dword 'GERM'
    mov  [text00+LLL*5+32],dword 'AN  '
  noge:
    cmp  eax,3
    jnz  nogr
    mov  [text00+LLL*5+28],dword 'RUSS'
    mov  [text00+LLL*5+32],dword 'IAN '
  nogr:
    cmp  eax,4
    jnz  nofr
    mov  [text00+LLL*5+28],dword 'FREN'
    mov  [text00+LLL*5+32],dword 'CH  '
  nofr:
    cmp  eax,5
    jnz  noet
    mov  [text00+LLL*5+28],dword 'ESTO'
    mov  [text00+LLL*5+32],dword 'NIAN'
  noet:

    mov  eax,[syslang]		  ; SYSTEM LANGUAGE
    dec  eax
    test eax,eax
    jnz  noen5
    mov  [text00+LLL*4+28],dword 'ENGL'
    mov  [text00+LLL*4+32],dword 'ISH '
  noen5:
    cmp  eax,1
    jnz  nofi5
    mov  [text00+LLL*4+28],dword 'FINN'
    mov  [text00+LLL*4+32],dword 'ISH '
  nofi5:
    cmp  eax,2
    jnz  noge5
    mov  [text00+LLL*4+28],dword 'GERM'
    mov  [text00+LLL*4+32],dword 'AN  '
  noge5:
    cmp  eax,3
    jnz  nogr5
    mov  [text00+LLL*4+28],dword 'RUSS'
    mov  [text00+LLL*4+32],dword 'IAN '
  nogr5:
    cmp  eax,4
    jne  nofr5
    mov  [text00+LLL*4+28],dword 'FREN'
    mov  [text00+LLL*4+32],dword 'CH  '
  nofr5:
    cmp  eax,5
    jne  noet5
    mov  [text00+LLL*4+28],dword 'ESTO'
    mov  [text00+LLL*4+32],dword 'NIAN'
  noet5:

    mov  eax,[lba_read]
    call onoff				; LBA READ
    mov  [text00+LLL*8+28],ebx

    mov  eax,[pci_acc]
    call onoff				; PCI ACCESS
    mov  [text00+LLL*9+28],ebx

    mov  eax,[mouse_speed]		; MOUSE SPEED
    add  al,48
    mov  [text00+LLL*12+28],al

    mov  eax,[mouse_delay]
    mov  esi,text00+LLL*13+32
    call hexconvert			; MOUSE DELAY

    call text_out

    popa
    ret

text_out:
    mov  eax,13
    mov  ebx,175*65536+85
    mov  ecx,40*65536+225
    mov  edx,0x80111199-19
    mcall

    mov  edx,text00
    mov  ebx,10*65536+45
    mov  eax,4
    mov  ecx,0xffffff
    mov  esi,LLL
    mov  ebp,text1_strings
  newline:
    mcall
    add  ebx,8+8
    add  edx,esi
    dec  ebp
    jnz  newline
    mov  ebp,text2_strings
    add  ebx,8+8
  @@:
    mcall
    add  ebx,8+8
    add  edx,esi
    dec  ebp
    jnz  @b
    ret



  hexconvert:	     ;converting dec to hex in ascii
    xor  ebx,ebx
    mov  bl,al
    and  bl,15
    add  ebx,hex
    mov  cl,[ebx]
    mov  [esi],cl
    shr  eax,4
    xor  ebx,ebx
    mov  bl,al
    and  bl,15
    add  ebx,hex
    mov  cl,[ebx]
    dec  esi
    mov  [esi],cl
    shr  eax,4
    xor  ebx,ebx
    mov  bl,al
    and  bl,15
    add  ebx,hex
    mov  cl,[ebx]
    dec  esi
    mov  [esi],cl
    ret

onoff:
    cmp [syslang],4
    jne norus1
    mov ebx,'ДА  '
    cmp eax,1
    je	      exitsub
    mov ebx,'НЕТ '
    ret
 norus1:
    mov ebx,'ON  '
    cmp eax,1
    je	      exitsub
    mov ebx,'OFF '
 exitsub:
    ret


_lba_read:
    mov  eax,21
    mov  ebx,11
    mov  ecx,[lba_read]
    mcall
    ret

_pci_acc:
    mov  eax,21
    mov  ebx,12
    mov  ecx,[pci_acc]
    mcall
    ret

_syslang:
    mov  eax,21
    mov  ebx,5
    mov  ecx,[syslang]
    mcall
 ret

_mouse_speed:
    mov  eax,18
    mov  ebx,19
    mov  ecx,1
    mov  edx,[mouse_speed]
    mcall
 ret

_mouse_delay:
    mov  eax,18
    mov  ebx,19
    mov  ecx,3
    mov  edx,[mouse_delay]
    mcall
 ret

loadtxt:
    mov  edi,text00
    mov  ecx,488 ;28
    cmp  [syslang],4
    jne  norus
    mov  esi,textrus
    jmp  sload
  norus:
    mov  esi,texteng
  sload:
    rep  movsd
    ret

cleantxt:
    xor  eax,eax
    mov  ecx,428
    cld
    mov  edi,text00
    rep stosd
    mov  [text00+1711],byte 'x'
    ret

; DATA AREA
count:		db 0x0
blinkpar: dd 0x0
time:	     dw 0x0
date:	     dd 0x0

textrus:

    db '                                                           '
    db '                                                           '
    db '                                                           '
    db '                                                           '
    db '?зык системы              : ENGLISH         - +   Применить'
    db 'Р скл дк  кл ви туры      : ENGLISH         - +   Применить'
    db '                                                           '
    db '                                                           '
    db '?ключить LBA              : OFF             - +   Применить'
    db '?оступ к шине PCI         : OFF             - +   Применить'
    db '                                                           '
    db '                                                           '
    db '?корость курсор  мыши     : 1               - +   Применить'
    db '? держк  ускорения мыши   : 0x00a           - +   Применить'

    db 'ВНИМАНИЕ:                                    Применить все '
    db 'НЕ ЗАБУДЬТЕ СОХРАНИТЬ НАСТРОЙКИ              Сохранить все '

texteng:

    db '                                                           '
    db '                                                           '
    db '                                                           '
    db '                                                           '
    db 'SYSTEM LANGUAGE           : ENGLISH         - +     APPLY  '
    db 'KEYBOARD LAYOUT           : ENGLISH         - +     APPLY  '
    db '                                                           '
    db '                                                           '
    db 'LBA READ ENABLED          : OFF             - +     APPLY  '
    db 'PCI ACCESS FOR APPL.      : OFF             - +     APPLY  '
    db '                                                           '
    db '                                                           '
    db 'Mouse pointer speed       : 1               - +     APPLY  '
    db 'Mouse pointer delay       : 0x00a           - +     APPLY  '
text1_strings = 14

    db 'NOTE:                                           APPLY ALL  '
    db 'SAVE YOUR SETTINGS BEFORE QUITING KOLIBRI       SAVE ALL   '
text2_strings = 2

title  db 'SETUP',0

hex db	       '0123456789ABCDEF'




include 'keymaps.inc'




read_fileinfo:
       dd 0
       dd 0
       dd 0
       dd 48
       dd keyboard
       db 0
       dd file_name

save_fileinfo:
       dd 2
       dd 0
       dd 0
       dd 48
       dd keyboard
file_name:   db '/sys/setup.dat',0

I_PARAM   dd 0

keyboard     dd 0x0
syslang      dd 0x1
lba_read     dd 0x1
pci_acc      dd 0x1
mouse_speed  dd 0x3
mouse_delay  dd 0x10
text00:

I_END:
table_area:
