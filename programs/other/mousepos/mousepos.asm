include "../../macros.inc"
include "../../KOSfuncs.inc"

KOS_APP_START

CODE

mcall SF_SYSTEM, SSF_WINDOW_BEHAVIOR, SSSF_SET_WB, -1, 1
mcall SF_STYLE_SETTINGS, SSF_GET_COLORS, syscl, sizeof.system_colors
mcall SF_SET_EVENTS_MASK, EVM_REDRAW + EVM_MOUSE + EVM_KEY
mcall SF_KEYBOARD, SSF_SET_INPUT_MODE, 1

wait_event:
	mcall SF_WAIT_EVENT
	
	cmp eax, EV_REDRAW
	jz redraw_event
	
	cmp eax, EV_MOUSE
	jz draw_text
	
	cmp eax, EV_KEY
	jz key_event

	jmp wait_event

key_event:
	mcall SF_GET_KEY
	cmp eax, 256 ;Escape
	jnz wait_event
	mcall SF_TERMINATE_PROCESS

redraw_event:
	mcall SF_REDRAW, SSF_BEGIN_DRAW
	mcall SF_CREATE_WINDOW, <0, WIN_W>, <0, WIN_H>, 0x01000000, 0x1000000, 0
	mcall SF_DRAW_RECT, <0, WIN_W + 1>, <0, WIN_H + 1>, [syscl.work]
	mcall SF_REDRAW, SSF_END_DRAW

draw_text:
	; Draw labels
	mov ecx, 0xF0000000
	or ecx, [syscl.work_text]
	mcall SF_DRAW_TEXT, <10, 10>, , gxlabel, , [syscl.work]
	mcall , <10, 30>, , gylabel
	mcall , <10, 50>, , pidlabel
	mcall , <10, 70>, , lxlabel
	mcall , <10, 90>, , lylabel
	
	; Get mouse porition
	mcall SF_MOUSE_GET, SSF_SCREEN_POSITION
	push eax
	
	; Draw coords
	mov esi, 0x50000000
	or esi, [syscl.work_text]
	xor ecx, ecx
	mov cx, [esp + 2]
	mcall SF_DRAW_NUMBER, 0x00040000, , <102, 10>
	mov cx, [esp]
	mcall , , , <102, 30>
	
	; Get pixel owner
	xor ebx, ebx
	mov bx, [esp + 2]
	mov cx, [esp]
	mcall SF_GET_PIXEL_OWNER
	
	; Draw pixel owner
	xchg ecx, eax
	mcall SF_DRAW_NUMBER, 0x00040000, , <102, 50>
	
	mcall SF_THREAD_INFO, pinf
	
	xor ecx, ecx
	mov cx, [esp + 2]
	sub ecx, [pinf.box.left]
	mcall SF_DRAW_NUMBER, 0x00040000, , <102, 70>
	mov cx, [esp]
	sub ecx, [pinf.box.top]
	mcall , , , <102, 90>
	
	pop eax
	jmp wait_event

DATA

WIN_W = 200
WIN_H = 150
gxlabel db 'Global X:', 0
gylabel db 'Global Y:', 0
lxlabel db 'Local X:', 0
lylabel db 'Local Y:', 0
pidlabel db 'PID:', 0

syscl system_colors
pinf process_information

UDATA
KOS_APP_END
