;
;   MyKey. Version 0.2.
;
;   Author:         Asper
;   Date of issue:  29.12.2009
;   Compiler:       FASM
;   Target:         KolibriOS
;

use32
	org	0x0

	db	'MENUET01'	; 8 byte id
	dd	38		; required os
	dd	STARTAPP	; program start
	dd	I_END		; program image size
	dd	0x1000000	; required amount of memory
	dd	0x1000000	; stack heap
	dd	0x0
	dd	app_path

include 'lang.inc'	;language support

include 'string.inc'
;include 'macros.inc'
include '../../../macros.inc'
include 'ASPAPI.INC'
;include 'editbox_ex.mac'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
;include 'load_lib.mac'
include '../../../develop/libraries/box_lib/load_lib.mac'
include '../../../dll.inc'

include 'debug.inc'
DEBUG	 equ 0;1

N_KEYCOLOR    equ 0x00EEEEEE ; Normal button color
C_KEYCOLOR    equ 0x00CBE1E1 ; Control button color
A_KEYCOLOR    equ 0x00FF6400;258778 ; Active button color
C_TEXTCOLOR   equ 0x80000000 ; Button caption color
CA_TEXTCOLOR  equ 0x80FFFFFF ; Active button caption color
A_TEXTCOLOR   equ 0x00FFFFFF ; Active text color

WIN_X	      equ 265
WIN_Y	      equ 50;175
WIN_W	      equ 595
WIN_H	      equ 415 ;570
WIN_COLOR     equ 0x040099BB;0x04EEEEEE

ITEM_BUTTON_W	      equ 192;100
ITEM_BUTTON_H	      equ 23
ITEM_BUTTON_SPACE     equ 0
FIRST_ITEM_BUTTON_ID  equ 7

BUT_W	      equ 80
BUT_H	      equ 20

MAX_HOTKEYS_NUM equ 15 ;  Bad bounding :/. Until we have normal listbox control.
PATH_MAX_CHARS equ 255

@use_library

STARTAPP:
	; Initialize memory
	mcall	68, 11
	or	eax,eax
	jz	close_app
	; Import libraries
	sys_load_library  boxlib_name, sys_path, boxlib_name, system_dir0, err_message_found_lib, head_f_l, myimport,err_message_import, head_f_i
	cmp	eax,-1
	jz	close_app
	stdcall dll.Load,importTable
	test	eax, eax
	jnz	close_app

	mcall	68, 12, MAX_HOTKEYS_NUM*PATH_MAX_CHARS	 ; Get memory for editboxes text
	mov	dword [buf_cmd_line], eax
	mov	dword [edit1.text],   eax
	mcall	68, 12, MAX_HOTKEYS_NUM*PATH_MAX_CHARS
	mov	dword [buf_cmd_params], eax
	mov	dword [edit2.text],	eax
	mcall	68, 12, MAX_HOTKEYS_NUM*32
	mov	dword [it_buf_cmd_line], eax
	mov	dword [it_edit.text],	 eax

	call	Load_HotkeyList

	mcall	66, 1, 1  ; Set keyboard mode to get scancodes.
	mcall	26, 2, 1, ascii_keymap

get_mykey_window_slot_number:
	call	draw_window
	mcall	18, 7
	mov	[mykey_window], eax

set_event_mask:
	mcall	 40, 0xC0000027

red:
      .test_slot:
	mcall	18, 7
	mov	ebx, [mykey_window]
	cmp	eax, ebx
	jne	@f

	mov	ecx, [it_window]
	cmp	ebx, ecx
	je	@f
      .activate_it_window:
	mov	al,  byte [it_alive]
	test	al,  al
	jz	@f
	mov	byte [it_alive], 0

	mcall	18, 3			    ; Activate input thread window
      @@:
	call	draw_window

still:
	call	reset_modifiers

	mcall	10		 ; Wait for an event in the queue.

	cmp	al,1		      ; redraw request ?
	jz	red
	cmp	al,2		      ; key in buffer ?
	jz	key
	cmp	al,3		      ; button in buffer ?
	jz	button
	cmp	al,6
	jz	mouse

	jmp	still

key:
	mcall	2

	push	eax
	mcall	66, 3
	;mov     edx, eax
	;and     edx, 0x00000FF;F
	mov	dword [modifiers], eax;edx
	pop	eax

	test	word [edit1.flags], 10b
	jnz	.editbox_input
	test	word [edit2.flags], 10b
	jz	@f
     .editbox_input:
	cmp	ah, 0x80 ;if key up
	ja	still
	cmp	ah, 42 ;LShift
	je	still
	cmp	ah, 54 ;RShift
	je	still
	cmp	ah, 56 ;Alt
	je	still
	cmp	ah, 29 ;Ctrl
	je	still
	cmp	ah, 69 ;Pause/Break
	je	still

       mov     esi, ascii_keymap
       call    Scan2ASCII

       push dword edit1
       call [edit_box_key]

       push dword edit2
       call [edit_box_key]
       jmp still
     @@:

      ;------------------------
	mov	cl, byte [hotkeys_num]
     .test_next_hotkey:
	dec	cl
	mov	bl, cl
	and	ebx, 0xFF
	shl	ebx, 2;5
;        mov     esi, ebx
	add	ebx, dword Hotkeys.codes

	mov	edx, dword [ebx]
	cmp	ah, dl
	jne	@f

	shr	edx, 8
	cmp	edx, dword [modifiers]
	jne	@f

	push	eax
	mov	eax, PATH_MAX_CHARS
	mul	cl
	mov	edx, eax
	add	edx, dword [buf_cmd_params]
	add	eax, dword [buf_cmd_line]
	mov	esi, eax
	pop	eax
	call	RunProgram
	jmp	.end_test
     @@:
	or	cl, cl
	jnz	.test_next_hotkey
     .end_test:
      ;------------------------

	jmp	still

button:
	mcall	17	       ; Get pressed button code
	cmp	ah, 1		    ; Test x button
	je	close_app

	cmp	ah, 2
	jne	@f
	call	AddHotKey
	jmp	red
       @@:
	cmp	ah, 5
	jne	@f
	call	Load_HotkeyList
	jmp	red
       @@:
	cmp	ah, 6
	jne	@f
	call	WriteIni
	xor	edx, edx
	mov	esi, aRamSaver
	call	RunProgram
       @@:

	cmp	ah, FIRST_ITEM_BUTTON_ID     ; Test if pressed buttons
	jb	still			     ; is a HotKey button...
	mov	al, ah
	sub	al, FIRST_ITEM_BUTTON_ID
	cmp	al, byte [hotkeys_num]
	jnb	still			     ; ...so, if not then still,


	mov	byte [butt], ah 	  ; if yes then save pressed button ID
	and	eax, 0xFF
	mov	cl, byte PATH_MAX_CHARS
	mul	cl
	mov	ebx, eax
	add	ebx, dword [buf_cmd_params]
	add	eax, dword [buf_cmd_line]

	mov	dword [edit1.text], eax
	mov	dword [edit2.text], ebx

	mov	esi, eax
	call	strlen
	mov	dword [edit1.size], ecx
	mov	dword [edit1.pos], ecx

	mov	esi, ebx
	call	strlen
	mov	dword [edit2.size], ecx
	mov	dword [edit2.pos], ecx

	jmp	red

mouse:
	push	dword edit1
	call	[edit_box_mouse]
	push	dword edit2
	call	[edit_box_mouse]

	jmp	still


close_app:
	mov	eax,-1			; close this program
	int	0x40


draw_window:
	start_draw_window WIN_X,WIN_Y,WIN_W,WIN_H,WIN_COLOR,labelt, 11;labellen-labelt

	;bar         5, 24, 585, 385, 0x800000 or 0x90D2
	;rectangle2  6, 25, 585, 385, 0xFFFFFF, 0

	;bar         5, 24, BUT_W+4, 350, 0x008C00D2;0x800000 or A_KEYCOLOR
	;rectangle2  6, 25, BUT_W+4, 350, 0xFFFFFF, 0


	push	dword edit1
	call	[edit_box_draw]
	push	dword edit2
	call	[edit_box_draw]

	stdcall draw_button,   7,WIN_H-BUT_H-10,BUT_W,BUT_H,2,0x0050D250,AddKeyText,   0,C_TEXTCOLOR	; Add Hotkey.
    if 0
	stdcall draw_button,  90,WIN_H-BUT_H-10,BUT_W,BUT_H,3,C_KEYCOLOR,DeleteKeyText,0,C_TEXTCOLOR	; Delete Hotkey.
	stdcall draw_button, 173,WIN_H-BUT_H-10,BUT_W,BUT_H,4,C_KEYCOLOR,ManageKeyText,0,C_TEXTCOLOR	; Manage Hotkey.
    end if
	stdcall draw_button,   WIN_W-BUT_W*2-14,WIN_H-BUT_H-10,BUT_W,BUT_H,5,0x0050D250,ReloadKeyText,	 0,C_TEXTCOLOR	  ; Save Hotkeys list.
	stdcall draw_button,   WIN_W-BUT_W-7,WIN_H-BUT_H-10,BUT_W,BUT_H,6,0x0050D250,SaveKeyText,   0,C_TEXTCOLOR    ; Save Hotkeys list.

	movzx	ecx, byte [hotkeys_num]
	cmp	ecx, MAX_HOTKEYS_NUM
	jng	@f
	mov	ecx, MAX_HOTKEYS_NUM
     @@:
	mov	eax, 30
	mov	ebx, FIRST_ITEM_BUTTON_ID
     @@:
	or	cl, cl
	jz	@f

	mov	edx, ebx
	sub	edx, FIRST_ITEM_BUTTON_ID
	shl	edx, 5; edx=edx*32
	add	edx, dword Hotkeys

	cmp	bl, byte [butt]
	jne	.l1
	stdcall draw_button, 7,eax,ITEM_BUTTON_W,ITEM_BUTTON_H,ebx,A_KEYCOLOR  ,edx,0,CA_TEXTCOLOR
	bar	    220, 70, 350, 30, 0x00C8E1F0 ;0x800000 or A_KEYCOLOR
	rectangle2  221, 71, 350, 30, 0xFFFFFF, 0
	mov	esi, Hotkeys.code_names
	sub	edx, dword Hotkeys
	shl	edx, 1
	add	esi, edx
	stdcall outtextxy, 225, 80, esi, 64, C_TEXTCOLOR
	jmp	.l2
     .l1:
	stdcall draw_button, 7,eax,ITEM_BUTTON_W,ITEM_BUTTON_H,ebx,N_KEYCOLOR,edx,0,C_TEXTCOLOR
     .l2:

	add	eax, ITEM_BUTTON_H+ITEM_BUTTON_SPACE
	inc	ebx
	dec	cl
	jmp	@b
      @@:
	end_draw_window
ret


AddHotKey:
	mov	al, byte [hotkeys_num]
	cmp	al, MAX_HOTKEYS_NUM
	jge	.end
	inc	al
	mov	byte [hotkeys_num], al

	mcall	51, 1, dword start_input_thread, dword input_thread_stack_top
    .end:
ret


Load_HotkeyList:
	call	ReadIni

	mov	al, byte [butt]
	and	eax, 0xFF
	sub	al, FIRST_ITEM_BUTTON_ID
	mov	cl, byte PATH_MAX_CHARS
	mul	cl
	mov	ebx, eax
	add	eax, dword [buf_cmd_line]
	add	ebx, dword [buf_cmd_params]


	;mov  [butt], FIRST_ITEM_BUTTON_ID
	mov	esi, eax
	call	strlen
	mov	dword [edit1.size], ecx
	mov	dword [edit1.pos], ecx

	mov	esi, ebx
	call	strlen
	mov	dword [edit2.size], ecx
	mov	dword [edit2.pos], ecx
ret


reset_modifiers:
	pusha
	mov	esi, dword [it_hotkey_addr]
	test	esi, esi
	jz	.end_set_mods

	lodsd

	; Set new hotkey for the main thread
	mov	cl, al
	shr	eax, 8

	xor    edx, edx
	push	cx
	mov    cl, 3
     .next_pair:
	shl    edx, 4
	mov    bl, al
	and    bl, 3

	or     bl, bl
	jz     .l1

	cmp    bl, 3 ; both?
	jne    @f
	or     dl, 2
	jmp    .l1
      @@:
	add    bl, 2
	or     dl, bl
      .l1:
	shr    eax, 2
	dec    cl
	test   cl, cl
	jnz    .next_pair

	mov    bx, dx
	and    bx, 0xF0F
	xchg   bl, bh
	and    dx, 0x0F0
	or     dx, bx
	pop    cx

	mcall	66, 4
	mov	dword [it_hotkey_addr], 0
     .end_set_mods:
	popa
ret


;######################## Input Thread code start  ##########################

start_input_thread:
	mov	ecx, 1	   ; to get scancodes.
	mcall	26, 2, 1, it_ascii_keymap
	mcall	66, 1	   ; Set keyboard mode
	mov	dword [it_hotkey_addr], 0
it_set_editbox:
	mov	al, byte [hotkeys_num]
	sub	al, 1
	and	eax, 0xFF
	shl	eax, 5
	add	eax, dword Hotkeys.names
	mov	dword [it_edit.text], eax

	mov	esi, eax
	call	strlen
	mov	dword [it_edit.size], ecx
	mov	dword [it_edit.pos], ecx
get_it_window_slot_number:
	call	it_draw_window
	mcall	18, 7
	mov	[it_window], eax

it_set_event_mask:
	mcall	40, 39
it_red:
	call	it_draw_window

it_still:
	mcall	10		 ; Wait for an event in the queue.

	cmp	al,1		      ; redraw request ?
	jz	it_red
	cmp	al,2		      ; key in buffer ?
	jz	it_key
	cmp	al,3		      ; button in buffer ?
	jz	it_button
	cmp	al,6
	jz	it_mouse

	jmp	it_still

it_key:
	mcall	2

	mov	byte [it_keycode], 0
	stdcall outtextxy, 10, 100, ctrl_key_names, 35, 0

	cmp	ah, 1 ;Esc
	jne	@f
	dec	byte [hotkeys_num]
	jmp	close_app
      @@:

	cmp	ah, 0x80 ;if key up
	ja	.end
	cmp	ah, 42 ;[Shift] (left)
	je	.end
	cmp	ah, 54 ;[Shift] (right)
	je	.end
	cmp	ah, 56 ;[Alt]
	je	.end
	cmp	ah, 29 ;[Ctrl]
	je	.end
	cmp	ah, 69 ;[Pause Break]
	je	.end

	mov	byte [it_keycode], ah
	mov	esi, it_ascii_keymap
	call	Scan2ASCII

	test	word [it_edit.flags], 10b
	jz	.end
	push	dword it_edit
	call	[edit_box_key]
	jmp	it_still
      .end:

	mcall	26, 2, 1, it_ascii_keymap
	call	it_test_key_modifiers
	test	dl, 3
	jz	@f
	push	edx
	mcall	26, 2, 2, it_ascii_keymap
	pop	edx
      @@:

	mov	al, byte [it_keycode]
	test	al, al
	jz	@f
	shl	edx, 8
	mov	dl, al

	mov	eax, dword [it_hotkey_addr]
	test	eax, eax
	jnz	@f

	call	it_set_keycode_name

	mov	al, byte [hotkeys_num]
	dec	al
	and	eax, 0xFF
	shl	eax, 2;5
	add	eax, dword Hotkeys.codes
	mov	dword [eax], edx
	mov	dword [it_hotkey_addr], eax

	mov	cl, dl ; finally set hotkey
	shr	edx, 8
	mcall	66, 4
      @@:

	jmp	it_still


it_test_key_modifiers:
	push	eax
	mcall	66, 3 ;get control keys state
	mov	edx,  eax
	;and     edx,  0x00000FFF
      .lshift:
	test	al, 1  ; LShift ?
	jz	.rshift
	stdcall outtextxy, 10, 100, ctrl_key_names, 6, A_TEXTCOLOR
      .rshift:
	test	al, 2  ; RShift ?
	jz	.lctrl
	stdcall outtextxy, 184, 100, ctrl_key_names+29, 6, A_TEXTCOLOR
      .lctrl:
	test	al, 4  ; LCtrl ?
	jz	.rctrl
	stdcall outtextxy, 52, 100, ctrl_key_names+7, 5, A_TEXTCOLOR
      .rctrl:
	test	al, 8  ; RCtrl ?
	jz	.lalt
	stdcall outtextxy, 148, 100, ctrl_key_names+23, 5, A_TEXTCOLOR
      .lalt:
	test	al, 0x10  ; LAlt ?
	jz	.ralt
	stdcall outtextxy, 88, 100, ctrl_key_names+13, 4, A_TEXTCOLOR
      .ralt:
	test	al, 0x20  ; RAlt ?
	jz	@f
	stdcall outtextxy, 118, 100, ctrl_key_names+18, 4, A_TEXTCOLOR
      @@:
	pop	eax
ret


it_set_keycode_name:
	pusha
	mov	al, byte [hotkeys_num]
	dec	al
	and	eax, 0xFF
	shl	eax, 6
	mov	edi, Hotkeys.code_names
	add	edi, eax

	mov	ecx, 64
	xor	ax, ax
	call	strnset
	mcall	66, 3 ;get control keys state
      .lshift:
	test	al, 1  ; LShift ?
	jz	.rshift
	mov	esi, ctrl_key_names
	mov	ecx, 6
	call	strncat

	mov	esi, aPlus
	mov	ecx, 3
	call	strncat
	;stdcall outtextxy, 10, 100, ctrl_key_names, 6, 0x00FF0000
      .rshift:
	test	al, 2  ; RShift ?
	jz	.lctrl
	mov	esi, ctrl_key_names+29
	mov	ecx, 6
	call	strncat

	mov	esi, aPlus
	mov	ecx, 3
	call	strncat
	;stdcall outtextxy, 184, 100, ctrl_key_names+29, 6, 0x00FF0000
      .lctrl:
	test	al, 4  ; LCtrl ?
	jz	.rctrl
	mov	esi, ctrl_key_names+7
	mov	ecx, 5
	call	strncat

	mov	esi, aPlus
	mov	ecx, 3
	call	strncat
	;stdcall outtextxy, 52, 100, ctrl_key_names+7, 5, 0x00FF0000
      .rctrl:
	test	al, 8  ; RCtrl ?
	jz	.lalt
	mov	esi, ctrl_key_names+23
	mov	ecx, 5
	call	strncat

	mov	esi, aPlus
	mov	ecx, 3
	call	strncat
	;stdcall outtextxy, 148, 100, ctrl_key_names+23, 5, 0x00FF0000
      .lalt:
	test	al, 0x10  ; LAlt ?
	jz	.ralt
	mov	esi, ctrl_key_names+13
	mov	ecx, 4
	call	strncat

	mov	esi, aPlus
	mov	ecx, 3
	call	strncat
	;stdcall outtextxy, 88, 100, ctrl_key_names+13, 4, 0x00FF0000
      .ralt:
	test	al, 0x20  ; RAlt ?
	jz	@f
	mov	esi, ctrl_key_names+18
	mov	ecx, 4
	call	strncat

	mov	esi, aPlus
	mov	ecx, 3
	call	strncat
	;stdcall outtextxy, 118, 100, ctrl_key_names+18, 4, 0x00FF0000
      @@:
	mov	esi, it_ascii_keymap
	and	edx, 0xFF
	add	esi, edx
	mov	ecx, 1
	call	strncat

	if 1;DEBUG
	  mov	  esi, edi;Hotkeys.code_names
	  call	  SysMsgBoardStr
	  newline
	end if

	popa
ret


it_button:
	mcall	17	       ; Get pressed button code
	cmp	ah, 1		    ; Test x button
	jne	@f
	jmp	close_app
      @@:
	jmp	it_still

it_mouse:

	push	dword it_edit
	call	[edit_box_mouse]

	jmp	it_still

it_draw_window:
	start_draw_window 450,WIN_Y+250,225,70,WIN_COLOR,it_labelt, 26;labellen-labelt

	push	dword it_edit
	call	[edit_box_draw]

	stdcall outtextxy, 43, 50, it_hint, 0, 0x323232
	stdcall outtextxy, 10, 100, ctrl_key_names, 0, 0
	;stdcall draw_button,   7,WIN_H-30,80,20,2,C_KEYCOLOR,AddKeyText,   0,C_TEXTCOLOR    ; Add Hot key.
	end_draw_window
	mov	byte [it_alive], 1
ret

;######################## Input Thread code end ##########################


; Read configuration file
ReadIni:
	; Get path
	mov	edi, ini_path
	mov	esi, app_path
	call	strlen

      .get_path:
	cmp	byte [app_path+ecx-1], '/'
	je	@f
	loop	.get_path
      @@:
	call	strncpy
	mov	byte [ini_path+ecx], 0
	mov	esi, aIni
	call	strlen
	call	strncat

	; Get hotkey number
	invoke	ini_get_int, ini_path, aMain, aKeynum, 0

	and	eax, 0xFF
	test	al, al
	jz	.end
	cmp	al, MAX_HOTKEYS_NUM
	jle	@f
	mov	al, MAX_HOTKEYS_NUM
      @@:
	mov	byte [hotkeys_num], al

	mov	ecx, eax
	xor	eax, eax
      .get_next_hotkey_values:
	call	set_next_hotkey_section_name
	; Get hotkey name
	mov	edi, eax
	shl	edi, 5 ; edi=eax*32
	add	edi, dword Hotkeys
	push	eax ecx
	invoke	ini_get_str, ini_path, aHotkey, aName, edi, 32, 0
	pop	ecx eax
	; Get hotkey code
	mov	edi, eax
	shl	edi, 2 ; edi=eax*4
	add	edi, dword Hotkeys.codes
	push	eax ecx edx
	invoke	ini_get_int, ini_path, aHotkey, aKeycode, 0
	mov	dword [it_hotkey_addr], edi
	stosd
	; set hotkey
	call	reset_modifiers
	pop	edx ecx eax
	; Get hotkey code_name
	mov	edi, eax
	shl	edi, 6 ; edi=eax*64
	add	edi, dword Hotkeys.code_names
	push	eax ecx
	invoke	ini_get_str, ini_path, aHotkey, aKeycodeName, edi, 64, 0
	pop	ecx eax
	; Get hotkey path and param
	push	eax ecx
	mov	cl, byte PATH_MAX_CHARS
	mul	cl
	mov	edi, eax
	push	edi
	add	edi, dword [buf_cmd_line]
	invoke	ini_get_str, ini_path, aHotkey, aPath, edi, 32, 0
	pop	edi
	add	edi, dword [buf_cmd_params]
	invoke	ini_get_str, ini_path, aHotkey, aParam, edi, 32, 0
	pop	ecx eax

	inc	al
	dec	ecx
	test	ecx, ecx
	jnz    .get_next_hotkey_values
    .end:
ret


; Write configuration file
WriteIni:
	mov	edi, ini_path
	; Set hotkey number
	movzx	ecx, byte [hotkeys_num]
	invoke	ini_set_int, ini_path, aMain, aKeynum, ecx

	xor	eax, eax
      .get_next_hotkey_values:
	call	set_next_hotkey_section_name
	; Set hotkey name
	push	eax ecx
	mov	esi, eax
	shl	esi, 5 ; edi=eax*32
	add	esi, dword Hotkeys
	call	strlen
	invoke	ini_set_str, ini_path, aHotkey, aName, esi, ecx
	pop	ecx eax
	; Set hotkey code
	mov	esi, eax
	shl	esi, 2 ; edi=eax*4
	add	esi, dword Hotkeys.codes
	push	eax ecx edx
	invoke	ini_set_int, ini_path, aHotkey, aKeycode, dword [esi]
	pop	edx ecx eax
	; Set hotkey code_name
	mov	esi, eax
	shl	esi, 6 ; edi=eax*64
	add	esi, dword Hotkeys.code_names
	push	eax ecx
	call	strlen
	invoke	ini_set_str, ini_path, aHotkey, aKeycodeName, esi, ecx
	pop	ecx eax
	; Set hotkey path and param
	push	eax ecx
	;inc     al
	mov	cl, byte PATH_MAX_CHARS
	mul	cl
	mov	esi, eax
	push	esi
	add	esi, dword [buf_cmd_line]
	call	strlen
	invoke	ini_set_str, ini_path, aHotkey, aPath, esi, ecx
	pop	esi
	add	esi, dword [buf_cmd_params]
	call	strlen
	invoke	ini_set_str, ini_path, aHotkey, aParam, esi, ecx
	pop	ecx eax

	inc	al
	dec	ecx
	test	ecx, ecx
	jnz    .get_next_hotkey_values
    .end:
ret


set_next_hotkey_section_name:		;(eax - num)
; this code mainly from debug.inc
	push	eax ecx edi
	mov	edi, aHotkey
	add	edi, 6 ; + strlen("hotkey")
	mov	ecx, 10
	push	-'0'
    .l0:
	xor	edx, edx
	div	ecx
	push	edx
	test	eax, eax
	jnz	.l0
    .l1:
	pop	eax
	add	al, '0'
	;call   debug_outchar
	stosb
	jnz	.l1
	pop	edi ecx eax
ret


;****************************************
;*  input:  esi = pointer to keymap     *
;*           ah  = scan code            *
;*  output:  ah  = ascii code           *
;****************************************
Scan2ASCII:
	push	esi
	shr	eax, 8
	add	esi, eax
	lodsb
	shl	eax, 8
	pop	esi
ret



;********************************************
;*  input:  esi = pointer to the file name  *
;*          edx = pointer to the parametrs  *
;********************************************

RunProgram:
    pusha
    mov      dword [InfoStructure],    7   ; run program
    mov      dword [InfoStructure+4],  0   ; flags
    mov      dword [InfoStructure+8],  edx ; pointer to the parametrs
    mov      dword [InfoStructure+12], 0   ; reserved
    mov      dword [InfoStructure+16], 0   ; reserved
    mov      dword [InfoStructure+20], 0   ; reserved
    mov      dword [InfoStructure+21], esi ; pointer to the file name
    mcall    70, InfoStructure
    cmp      eax, 0
    jl	     .err_out
  .out:
    popa
    clc
    ret
  .err_out:
    if lang eq it
		print	 "Impossibile caricare il programma"
;--------------------------------------
    else
		print	 "Can't load program"

    end if
    popa
    stc
    ret


; DATA AREA

; Application Title
labelt		db	'MyKey v.0.2'
mykey_window	dd	0	   ; Slot number of MyKey


;########### Input Thread data start ############

; Input Thread Title
if lang eq it
	it_labelt	db	"Inserisci hotkey e nome   "
;--------------------------------------
else
	it_labelt	db	"Input hotkey and it's name"

end if
;labellen:
it_edit edit_box 180, 20, 30, 0xffffff, 0xAA80, 0x0000ff, 0x0, 0x0, 31, it_buf_cmd_line, 0, 0
it_buf_cmd_line   dd	  0 ;db MAX_HOTKEYS_NUM*32 dup(0)  ; !Make it dynamic!!!
it_window	  dd	  0	     ; Slot number of the input thread
it_alive	  db	  0	     ; Flag of the input thread existance
it_keycode	  db	  0
it_hotkey_addr	  dd	  0
if lang eq it
	it_hint 	  db	  'o premi Esc per cancellare',0
;--------------------------------------
else
	it_hint 	  db	  'or press Esc to cancel',0

end if
;########### Input Thread data end   ############

;Button names
if lang eq it
	AddKeyText	db 'Aggiungi',0
	ReloadKeyText	db 'Ricarica',0
	SaveKeyText	db 'Salva',0
	;DeleteKeyText   db 'Delete',0
	;ManageKeyText   db 'Manage',0
;--------------------------------------
else
	AddKeyText	db 'Add',0
	ReloadKeyText	db 'Reload',0
	SaveKeyText	db 'Save',0
	;DeleteKeyText   db 'Delete',0
	;ManageKeyText   db 'Manage',0

end if

hotkeys_num   db 0;15
;keyboard_mode db 0       ; Scan or ASCII keys to send ?  0 - ASCII , 1 - Scan
butt	      db FIRST_ITEM_BUTTON_ID	    ; Pressed button ID
modifiers     dd 0

;Data structures for loadlib.mac and editbox_ex.mac [
edit1 edit_box 350, 220, 30, 0xffffff, 0xAA80, 0x0000ff, 0x0, 0x0, PATH_MAX_CHARS+1, buf_cmd_line, 0, 0
edit2 edit_box 350, 220, 50, 0xffffff, 0xAA80, 0x0000ff, 0x0, 0x0, PATH_MAX_CHARS+1, buf_cmd_params, 0, 0

buf_cmd_line   dd 0 ;db MAX_HOTKEYS_NUM*PATH_MAX_CHARS dup(0)  ; !Make it dynamic!!!
buf_cmd_params dd 0 ;db MAX_HOTKEYS_NUM*PATH_MAX_CHARS dup(0)  ; !Make it dynamic!!!

sys_path:
system_dir0 db '/sys/lib/'
boxlib_name db 'box_lib.obj',0

if lang eq it
	err_message_found_lib	db "Non trovo box_lib.obj",0
	head_f_i:
	head_f_l		db 'Errore di sistema',0
	err_message_import	db 'Error di importazione di box_lib.obj',0
;--------------------------------------
else
	err_message_found_lib	db "Can't find box_lib.obj",0
	head_f_i:
	head_f_l		db 'System error',0
	err_message_import	db 'Error on import box_lib.obj',0

end if
align 4
myimport:
edit_box_draw	dd  aEdit_box_draw
edit_box_key	dd  aEdit_box_key
edit_box_mouse	dd  aEdit_box_mouse
version_ed	dd  aVersion_ed
		dd  0,0

aEdit_box_draw	db 'edit_box_draw',0
aEdit_box_key	db 'edit_box_key',0
aEdit_box_mouse db 'edit_box_mouse',0
aVersion_ed	db 'version_ed',0

align 16
importTable:
library 						\
	libini, 'libini.obj';,                           \
;        boxlib, 'boxlib.obj',                           \
;        libio, 'libio.obj',                            \

;import  boxlib, \
;edit_box_draw  , 'edit_box_draw', \
;edit_box_key   , 'edit_box_key', \
;edit_box_mouse , 'edit_box_mouse', \
;version_ed     , 'version_ed'


import	libini, \
	ini_get_str  ,'ini_get_str', \
	ini_set_str  ,'ini_set_str', \
	ini_get_int  ,'ini_get_int', \
	ini_set_int  ,'ini_set_int';, \
;        ini_get_color,'ini_get_color', \
;        ini_set_color,'ini_set_color'


;] Data structures for loadlib.mac and editbox_ex.mac

InfoStructure:
		     dd      0x0     ; subfunction number
		     dd      0x0     ; position in the file in bytes
		     dd      0x0     ; upper part of the position address
		     dd      0x0     ; number of     bytes to read
		     dd      0x0     ; pointer to the buffer to write data
		     db      0
		     dd      0	     ; pointer to the filename


I_END:			  ; End of application code and data marker

   rb 300 ;input thread stack size
input_thread_stack_top:

ascii_keymap:
	     db 128 dup(?)
ctrl_key_names db  'LShift LCtrl LAlt RAlt RCtrl RShift',0
aPlus	       db  ' + ',0
aIni	       db  'settings/mykey.ini',0
aMain	       db  'main',0
aKeynum        db  'keynum',0
aHotkey        db  'hotkey',0,0,0
aName	       db  'name',0
aKeycode       db  'keycode',0
aKeycodeName   db  'keycode_name',0
aPath	       db  'path',0
aParam	       db  'param',0
aRamSaver      db  '/sys/rdsave',0

app_path       rb  255
ini_path       rb  255

Hotkeys:  ;(name = 32 b) + (modifiers = 3 b) + (keycode = 1 b) = 36 byte for 1 hotkey
    .names:
	     db 'My1',0
	     rb 28
	     db 'My2',0
	     rb 28
	     db 'My3',0
	     rb 28
	     rb MAX_HOTKEYS_NUM*32-3
    .codes:
	     dd MAX_HOTKEYS_NUM dup (0)
    .code_names:
	     rb MAX_HOTKEYS_NUM*64

it_ascii_keymap:
