;
;   MyKey. Version 0.1.
;
;   Author:         Asper
;   Date of issue:  04.12.2009
;   Compiler:       FASM
;   Target:         KolibriOS
;

use32
	org	0x0

	db	'MENUET00'	; 8 byte id
	dd	38		; required os
	dd	STARTAPP	; program start
	dd	I_END		; program image size
	dd	0x1000000	 ; required amount of memory
	dd	0x00000000	; reserved=no extended header

include "aspAPI.inc"
include 'macros.inc'
include 'editbox_ex.mac'
include 'load_lib.mac'

include 'debug.inc'
DEBUG	 equ 0;1

N_KEYCOLOR    equ 0x00EEEEEE ; Normal button color
C_KEYCOLOR    equ 0x00CBE1E1 ; Control button color
A_KEYCOLOR    equ 0x000099BB;258778 ; Active button color
C_TEXTCOLOR   equ 0x80000000 ; Button caption color
CA_TEXTCOLOR  equ 0x80FFFFFF ; Active button caption color

WIN_X	   equ 265
WIN_Y	   equ 50;175
WIN_W	   equ 595
WIN_H	   equ 415 ;570
WIN_COLOR  equ 0x04EEEEEE

BUT_W	   equ 192;100
BUT_H	   equ 23
BUT_SPACE  equ 0

MAX_HOTKEYS_NUM equ 15 ;  Bad bounding :/. Until we have normal listbox control.
PATH_MAX_CHARS equ 255

@use_library

STARTAPP:
	sys_load_library  boxlib_name, sys_path, boxlib_name, system_dir0, err_message_found_lib, head_f_l, myimport,err_message_import, head_f_i
	cmp	eax,-1
	jz	close_app
	;mcall 68,11
	;or eax,eax
	;jz close_app

	mcall	66, 1, 1  ; Set keyboard mode to get scancodes.
	mcall	26, 2, 1, ascii_keymap

;get_mykey_window_slot_number:
;        mcall   5,  10 ;wait
;        mcall   18, 7
;        mov     [mykey_window], eax

set_event_mask:
	mcall	 40, 39

red:
;      .test_slot:
;        mov     eax, [mykey_window]              ; Test is receiver MyKey window
;        mov     ecx, [it_window]
;        cmp     eax, ecx
;        je      @f;still                           ; if yes still.
;      .activate_it_window:
;        mov     eax, 18
;        mov     ebx, 3
;        int     0x40
;      @@:
	call	draw_window

still:
	call	reset_modifiers

	mov	eax, 10 	      ; Wait for an event in the queue.
	int	0x40

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
	mov	eax, 2
	int	0x40

	push	eax
	mcall	66, 3
	mov	edx, eax
	and	edx, 0x00000FFF
	mov	dword [modifiers], edx
	pop	eax

	test	word [edit1.flags], 10b;ed_focus ; если не в фокусе, выходим
	jnz	.editbox_input
	test	word [edit2.flags], 10b;ed_focus ; если не в фокусе, выходим
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

;    cmp [keyUpr],0
;    jne still

    call Scan2ASCII

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
	shl	ebx, 5
	mov	esi, ebx
	add	ebx, dword Hotkeys.codes

	cmp	ah, byte [ebx]
	jne	@f

	mov	edx, dword [ebx]
	shr	edx, 8
	cmp	edx, dword [modifiers]
	jne	@f

	push	eax
	mov	eax, PATH_MAX_CHARS
	mul	cl
	mov	edx, eax
	add	edx, dword buf_cmd_params
	add	eax, dword buf_cmd_line
	mov	esi, eax
	pop	eax
	call	RunProgram
	jmp	.end_test
      @@:
	or	cl, cl ;cmp     cl, 0
	jnz	.test_next_hotkey ;jge     .test_next_hotkey
      .end_test:
      ;------------------------

	jmp	still

button:
	mov	eax, 17 	    ; Get pressed button code
	int	0x40
	cmp	ah, 1		    ; Test x button
	je	close_app

	cmp	ah, 2
	jne	@f
	call	AddHotKey
	jmp	red
       @@:

	cmp	ah, 5		    ; Test if pressed buttons
	jb	still		     ; is a HotKey button...
	mov	al, ah
	sub	al, 5
	cmp	al, byte [hotkeys_num]
	jnb	still		     ; ...so, if not then still,


	mov	byte [butt], ah 	  ; if yes then save pressed button ID
	and	eax, 0xFF;shr     ax, 8
	if DEBUG
	   dps	"Button = "
	   dph	eax
	end if
	mov	cl, byte PATH_MAX_CHARS
	mul	cl
	if DEBUG
	   dps	"  offset = "
	   dph	eax
	end if
	mov	ebx, eax
	add	ebx, dword buf_cmd_params
	add	eax, dword buf_cmd_line

	mov	dword [edit1.text], eax
	mov	dword [edit2.text], ebx

	mov	esi, eax
	call	strlen
	if DEBUG
	   dps	"  len = "
	   dph	ecx
	   newline
	end if
	mov	dword [edit1.size], ecx
	mov	dword [edit1.pos], ecx

	mov	esi, ebx
	call	strlen
	mov	dword [edit2.size], ecx
	mov	dword [edit2.pos], ecx

	jmp	red ;still

mouse:
	push	dword edit1
	call	[edit_box_mouse]

	push	dword edit2
	call	[edit_box_mouse]

	;test    word [edit1.flags],10b;ed_focus ; если не в фокусе, выходим
	;jne     still
	jmp	still


close_app:
    mov  eax,-1 		 ; close this program
    int  0x40


draw_window:
  start_draw_window WIN_X,WIN_Y,WIN_W,WIN_H,WIN_COLOR,labelt, 11;labellen-labelt

  push dword edit1
  call [edit_box_draw]

  push dword edit2
  call [edit_box_draw]

  stdcall draw_button,	 7,WIN_H-30,80,20,2,C_KEYCOLOR,AddKeyText,   0,C_TEXTCOLOR    ; Add Hot key.
 if 0
  stdcall draw_button,	90,WIN_H-30,80,20,3,C_KEYCOLOR,DeleteKeyText,0,C_TEXTCOLOR    ; Delete Hot key.
  stdcall draw_button, 173,WIN_H-30,80,20,4,C_KEYCOLOR,ManageKeyText,0,C_TEXTCOLOR    ; Manage Hot key.
 end if

  movzx ecx, byte [hotkeys_num]
  cmp	ecx, MAX_HOTKEYS_NUM
  jng	@f
  mov	ecx, MAX_HOTKEYS_NUM
 @@:
  mov	eax, 30
  mov	ebx, 5
 @@:
  or	cl, cl
  jz	@f

  mov	edx, ebx
  sub	edx, 5
  shl	edx, 5; edx=edx*32
  add	edx, dword Hotkeys

  cmp	bl, byte [butt]
  jne	.l1
  stdcall draw_button, 7,eax,BUT_W,BUT_H,ebx,A_KEYCOLOR,edx,0,CA_TEXTCOLOR ; F5
  jmp	.l2
 .l1:
  stdcall draw_button, 7,eax,BUT_W,BUT_H,ebx,N_KEYCOLOR,edx,0,C_TEXTCOLOR ; F5
 .l2:

  add	eax, BUT_H+BUT_SPACE
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

	mov  eax, 51
	mov  ebx, 1
	mov  ecx, start_input_thread
	mov  edx, dword input_thread_stack_top
	mcall

    .end:
  ret


reset_modifiers:
	pusha
	mov	esi, dword [it_hotkey_addr]
	test	esi, esi
	jz	.end_set_mods

	lodsd

	mov	cl, al ; set new hotkey
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
     .end_set_mods:
     popa
ret


;######################## Input Thread code start  ##########################

start_input_thread:

;get_it_window_slot_number:
;        mcall   5,  10 ;wait
;        mcall   18, 7
;        mov     [it_window], eax
	mov	ecx, 1	   ; to get scancodes.
	  mov	  eax, 66
	  mov	  ebx, 1     ; Set keyboard mode
	  int	  0x40
	  mcall   26, 2, 1, ascii_keymap
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


it_set_event_mask:
	mcall	40, 39
it_red:
	call	it_draw_window

it_still:
	mov	eax, 10 	      ; Wait for an event in the queue.
	int	0x40

	cmp	al,1		      ; redraw request ?
	jz	it_red
	cmp	al,2		      ; key in buffer ?
	jz	it_key
	cmp	al,3		      ; button in buffer ?
	jz	button
	cmp	al,6
	jz	it_mouse

	jmp	it_still

it_key:
	mov	eax, 2
	int	0x40

	mov	byte [it_keycode], 0
	stdcall outtextxy, 10, 100, ctrl_key_names, 35, 0
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


      ;------------------------
	mov	cl, byte [hotkeys_num]
      .test_next_hotkey:
	mov	bl, cl
	and	ebx, 0xFF
	shl	ebx, 5
	mov	esi, ebx
	add	ebx, dword Hotkeys.codes
	cmp	ah, byte [ebx]
	jne	@f

	push	eax
	mov	eax, PATH_MAX_CHARS
	mul	cl
	add	eax, dword buf_cmd_line
	mov	esi, eax
	pop	eax
	mov	edx, 0 ; no parametrs yet <- change it!
	call	RunProgram
	jmp	.end_test
      @@:
	dec	cl
	cmp	cl, 0
	jge	.test_next_hotkey
      .end_test:
      ;------------------------

	mov	byte [it_keycode], ah
	call	Scan2ASCII

	test	word [it_edit.flags], 10b;ed_focus ; если не в фокусе, выходим
	jz	.end
	push	dword it_edit
	call	[edit_box_key]
	jmp	it_still
      .end:

	call	it_test_key_modifiers
	mov	al, byte [it_keycode]
	test	al, al
	jz	@f
	shl	edx, 8
	mov	dl, al

	mov	eax, dword [it_hotkey_addr]
	test	eax, eax
	jnz	@f
	mov	al, byte [hotkeys_num]
	sub	al, 1
	and	eax, 0xFF
	shl	eax, 5
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
	and	edx,  0x00000FFF
      .lshift:
	test	al, 1  ; LShift ?
	jz	.rshift
	stdcall outtextxy, 10, 100, ctrl_key_names, 6, 0x00FF0000
      .rshift:
	test	al, 2  ; RShift ?
	jz	.lctrl
	stdcall outtextxy, 184, 100, ctrl_key_names+29, 6, 0x00FF0000
      .lctrl:
	test	al, 4  ; LCtrl ?
	jz	.rctrl
	stdcall outtextxy, 52, 100, ctrl_key_names+7, 5, 0x00FF0000
      .rctrl:
	test	al, 8  ; RCtrl ?
	jz	.lalt
	stdcall outtextxy, 148, 100, ctrl_key_names+23, 5, 0x00FF0000
      .lalt:
	test	al, 0x10  ; LAlt ?
	jz	.ralt
	stdcall outtextxy, 88, 100, ctrl_key_names+13, 4, 0x00FF0000
      .ralt:
	test	al, 0x20  ; RAlt ?
	jz	@f
	stdcall outtextxy, 118, 100, ctrl_key_names+18, 4, 0x00FF0000
      @@:
	pop	eax
ret

it_mouse:

	push	dword it_edit
	call	[edit_box_mouse]

	jmp	it_still

it_draw_window:
  start_draw_window WIN_X,WIN_Y+250,225,70,WIN_COLOR,it_labelt, 26;labellen-labelt

  push dword it_edit
  call [edit_box_draw]

  stdcall outtextxy, 10, 100, ctrl_key_names, 35, 0
  ;stdcall draw_button,   7,WIN_H-30,80,20,2,C_KEYCOLOR,AddKeyText,   0,C_TEXTCOLOR    ; Add Hot key.
  end_draw_window
ret


;######################## Input Thread code end ##########################


Scan2ASCII:
	push	esi
	mov	esi, ascii_keymap
	shr	eax, 8
	add	esi, eax
	lodsb
	shl	eax, 8
	pop	esi
ret


;****************************************
;*  input:  esi = pointer to string     *
;*  output: ecx = length of the string  *
;****************************************
strlen:
      push	eax
      xor	ecx, ecx
      @@:
	lodsb
	or	al, al
	jz	@f
	inc	ecx
	jmp	@b
      @@:
      pop      eax
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
    mov      eax, 70
    mov      ebx, InfoStructure
    int      0x40
    cmp      eax, 0
    jl	     .err_out
  .out:
    popa
    clc
    ret
  .err_out:
    print    "Can't load program"
    popa
    stc
    ret


; DATA AREA

; Application Title
labelt		db	'MyKey v.0.1'
;mykey_window    dd      0          ; Slot number of MyKey


;########### Input Thread data start ############

; Input Thread Title
it_labelt	db	"Input hotkey and it's name"
;labellen:
it_edit edit_box 180, 20, 30, 0xffffff, 0xAA80, 0x0000ff, 0x0, 0x0, 31, it_buf_cmd_line, 0, 0
it_buf_cmd_line   db MAX_HOTKEYS_NUM*32 dup(0)	; !Make it dinamyc!!!
;it_window        dd      0          ; Slot number of Input thread
it_keycode	  db	  0
it_hotkey_addr	  dd	  0
;########### Input Thread data end   ############

;Button names
AddKeyText     db 'Add',0
DeleteKeyText  db 'Delete',0
ManageKeyText  db 'Manage',0


hotkeys_num   db 0;15
;keyboard_mode db 0       ; Scan or ASCII keys to send ?  0 - ASCII , 1 - Scan
butt	      db 5	  ; Pressed button ID
modifiers     dd 0

;Data structures for loadlib.mac and editbox_ex.mac [
edit1 edit_box 350, 220, 30, 0xffffff, 0xAA80, 0x0000ff, 0x0, 0x0, PATH_MAX_CHARS+1, buf_cmd_line, 0, 0
edit2 edit_box 350, 220, 50, 0xffffff, 0xAA80, 0x0000ff, 0x0, 0x0, PATH_MAX_CHARS+1, buf_cmd_params, 0, 0

buf_cmd_line   db MAX_HOTKEYS_NUM*PATH_MAX_CHARS dup(0)  ; !Make it dinamyc!!!
buf_cmd_params db MAX_HOTKEYS_NUM*PATH_MAX_CHARS dup(0)  ; !Make it dinamyc!!!

sys_path:
system_dir0 db '/sys/lib/'
boxlib_name db 'box_lib.obj',0

err_message_found_lib	db "Can't find box_lib.obj",0
head_f_i:
head_f_l		db 'System error',0
err_message_import	db 'Error on import box_lib.obj',0

align 4
myimport:
edit_box_draw	dd  aEdit_box_draw
edit_box_key	dd  aEdit_box_key
edit_box_mouse	dd  aEdit_box_mouse
version_ed	dd  aVersion_ed
		dd  0,0

aEdit_box_draw	db 'edit_box',0
aEdit_box_key	db 'edit_box_key',0
aEdit_box_mouse db 'edit_box_mouse',0
aVersion_ed	db 'version_ed',0

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

Hotkeys:  ;(name = 32 b) + (modifiers = 3 b) + (keycode = 1 b) = 36 byte for 1 hotkey
    .names:
	     db 'My Favorite 1',0
	     rb 18
	     db 'My Favorite 2',0
	     rb 18
	     db 'My Favorite 3',0
	     rb 18
	     rb MAX_HOTKEYS_NUM*32-3
    .codes:
	     dd MAX_HOTKEYS_NUM dup (0)
