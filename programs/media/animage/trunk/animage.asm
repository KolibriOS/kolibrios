;*******************************************************
;**************GRAPHICS EDITOR ANIMAGE *****************
;*******************************************************
; version:	1.2
; last update:  30/09/2010
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      Program used function 68 instead 64 is now,
;               select path with OpenDialog
;--------------------------------------------------------
; version 1.1 year 9.12.2006
; AUTORS:
; programming by andrew_programmer
; design by golus

use32
org	0x0

	db 'MENUET01'
	dd 0x1
	dd START
	dd IM_END
	dd I_END	;0x19000;100	kb
	dd stacktop	;0x19000;
	dd file_path	;parameters
	dd cur_dir_path


include	'..\..\..\macros.inc'
include '../../../develop/libraries/box_lib/load_lib.mac'
	@use_library
COLOR_ORDER equ MENUETOS
include	'gif_lite.inc'
include	'bmplib.inc'
;include	'dialog.inc'
include	'dialog2.inc'
include	'design.inc'
include	'graphlib.inc'

include	'cursors.inc'

include	'load_from_parameters.inc'

START:
	mcall	68,11
	
load_libraries l_libs_start,end_l_libs

	cmp	eax,-1
	jz	close
;--------------------------------------------------------
;---------------set events mask--------------------------
;--------------------------------------------------------
	mcall	40,1100111b
;---------------------------------------------------------
;-----------------------init data-------------------------
;---------------------------------------------------------
include	'init_data.inc'

;----------------------------------------------------------
;--------get memory and draw window of program-------------
;----------------------------------------------------------
	call	GetMemory
	call	cleare_work_arrea
	call	load_icons

;load	cursors
	mov	eax,CursorsID
	call	load_cursors

	call	drawwin
;---------------------------------------------------------
;---------Check loading of file from psrsmeters-----------
;---------------------------------------------------------
;	mov	eax,parameters
;	mov	ebx,file_path
;	call	check_loading_from_parameters
	mov	eax,file_path
	cmp [eax],byte 0
	jz @f
	call load_picture
@@:

;---------------------------------------------------------------------
	mov	edi,filename_area
	mov	esi,path4+5
	call	copy_str_1

	mov	edi,file_path
	cmp	[edi],byte 0
	jne	@f
	mov	esi,path4
	call	copy_str_1
@@:
;OpenDialog	initialisation
	push    dword OpenDialog_data
	call    [OpenDialog_Init]
;---------------------------------------------------------------------
;----------------------------------------------------------
;---------------------MAIN LOOP----------------------------
;----------------------------------------------------------
still:
	call	event
	cmp	eax,1
	jne	no_redraw_window

	call	drawwin
	jmp	still

no_redraw_window:
	cmp	eax,2
	je	keys

	cmp	eax,3
	je	buttons

	cmp	eax,6
	je	mouse

	jmp	still
	
;---------------------------------------------------------------------
copy_str_1:
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
	ret
;---------------------------------------------------------------------
;---------------------------------------------------------
;---------------------------------------------------------
;---------------------------------------------------------
include	'events.inc'
include	'events_of_window.inc'
include	'events_of_keys.inc'
include	'events_of_buttons.inc'
include	'events_of_mouse.inc'
include	'panel_engen.inc'
include	'screen.inc'
include	'menu_instruments.inc'
include	'icons_instruments.inc'
include	'icons.inc'
include	'sprites.inc'
include	'string.inc'
include	'palette.inc'
include	'files.inc'
include	'time.inc'
include	'memory.inc'
;-----------------------------------------------------------
;------------variables and data of program------------------
;-----------------------------------------------------------
sound_havent_memory	db 150,64,0
include 'lib_data.inc'
include	'panel_data.inc'
include	'palitra256.inc'
include	'brushes.inc'
include	'spray.inc'
include	'width_lines.inc'
;----------------------------------------------------------
;-------------------ICON"S picture-------------------------
;----------------------------------------------------------
	dd	0
panel_picture:
file 'panel_buttons.gif'
;****************cursors******************
brush_cursor:
file 'brush.cur'
;----------------------------------------------------------
flood_fill_cursor:
file 'flood_fill.cur'
;----------------------------------------------------------
lastik_cursor:
file 'lastik.cur'
;----------------------------------------------------------
other_cursor:
file 'other.cur'
;----------------------------------------------------------
pencil_cursor:
file 'pencil.cur'
;----------------------------------------------------------
pipette_cursor:
file 'pipette.cur'
;----------------------------------------------------------
spray_cursor:
file 'spray.cur'
;----------------------------------------------------------
zoom_cursor:
file 'zoom.cur'
;----------------------------------------------------------
IM_END:
;-----------------------------------------------------------
;------------variables and data of program------------------
;-----------------------------------------------------------
time			rd 1
PosX			rd 1
PosY			rd 1
PointerToIcons		rd 1
ScreenPointer		rd 1
PointerToPicture	rd 1
PointerToCopyPicture	rd 1
PointerToCopyPicture2	rd 1
PointerToEditBufer	rd 1
PointerToSpriteBufer	rd 1
PointerToPalette	rd 1
Color			rd 1
Number_Brush		rd 1
Brush_SizeX		rd 1
Brush_SizeY		rd 1
Current_instrument	rd 1
Last_instrument		rd 1
Activate_instrument	rb 1
SColor			rd 1
OldX			rd 1
OldY			rd 1

MouseX			rd 1
MouseY			rd 1
Window_SizeX		rd 1
Window_SizeY		rd 1
Window_CordinatX	rd 1
Window_CordinatY	rd 1
Picture_SizeX		rd 1
Picture_SizeY		rd 1
ScreenX			rd 1
ScreenY			rd 1
WorkScreen_SizeX	rd 1
WorkScreen_SizeY	rd 1
MaxWorkScreen_SizeX	rd 1
MaxWorkScreen_SizeY	rd 1
k			rd 1

ReserveArray		rd 1
register		rd 1
CounterX		rd 1
CounterY		rd 1
OffsetYPicture		rd 1
OffsetYWorkScreen	rd 1
OffsetYBigPixel		rd 1

Icon_X			rd 1
Icon_Y			rd 1
counter			rd 1
counter2		rd 1
Icon_text_x		rd 1
Icon_text_y		rd 1
Panel_flag		rb 1
counter_menu		rd 1
menu_coordinat_x	rd 1
menu_size_x		rd 1
menu_counter		rd 1
counter_11		rd 1
number_panel		rd 1
number_menu		rd 1

Scroll1CoordinatX	rd 1
Scroll1CoordinatY	rd 1
Scroll1MaxSizeX		rd 1
Scroll1MaxSizeY		rd 1
Scroll1SizeX		rd 1
Scroll1FreeX		rd 1

Scroll2CoordinatX	rd 1
Scroll2CoordinatY	rd 1
Scroll2MaxSizeX		rd 1
Scroll2MaxSizeY		rd 1
Scroll2SizeY		rd 1
Scroll2FreeY		rd 1

;extended_memory		rd 1
type			rw 1
x			rd 1
y			rd 1
save_flag		rb 1
exit_from_work_arrea	rb 1

Radius			rd 1
Dx_			rd 1
Dy_			rd 1
line_width		rd 1
lastik_is_active	rb 1
a_ellips		rd 1
b_ellips		rd 1
instrument_used		rb 1
used_OldX		rd 1
used_OldY		rd 1
rectangular_shade_x	rd 1
rectangular_shade_y	rd 1
crossing_old_x		rd 1
crossing_old_y		rd 1
crossing		rd 1
finishing_crossing	rd 1
number_undo		rd 1
DrawSprite_flag		rb 1
Paste_flag		rb 1
SpriteSizeX		rd 1
SpriteSizeY		rd 1
SpriteCoordinatX	rd 1
SpriteCoordinatY	rd 1
SpriteOldCoordinatX	rd 1
SpriteOldCoordinatY	rd 1
;---------------------------------------------------------------------
IncludeUGlobals
;---------------------------------------------------------------------
align 4
CursorsID	rd 10
;---------------------------------------------------------------------
;align 4
;parameters
;	rb 257
;---------------------------------------------------------------------
align 4
file_path:
	rb 4096	;rb 1024+16
;---------------------------------------------------------------------
align 4
filename_area:
	rb 256
;---------------------------------------------------------------------
align 4
temp_dir_pach:
	rb 4096
;---------------------------------------------------------------------
align 4
library_path:
	rb 4096
;---------------------------------------------------------------------
align 4
cur_dir_path:
	rb 4096
;---------------------------------------------------------------------
align 4
procinfo:
	rb 1024
;---------------------------------------------------------------------
align 4
IPC_table	rd 256
;---------------------------------------------------------------------
align 4
	rb 4096
stacktop:
;---------------------------------------------------------------------
I_END: