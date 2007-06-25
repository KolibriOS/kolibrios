;*******************************************************
;**************GRAPHICS EDITOR ANIMAGE *****************
;*******************************************************


; version 1.1  year      9.12.2006

; AUTORS:
; programming by andrew_programmer
; design      by golus

use32
org 0x0

      db  'MENUET01'
      dd  0x1
      dd  START
      dd  I_END
      dd  0x19000;100 kb
      dd  0x19000;
      dd  parameters,0x0


  include '..\..\..\macros.inc'
  COLOR_ORDER equ MENUETOS
  include 'gif_lite.inc'
  include 'bmplib.inc'
  include 'dialog.inc'
  include 'dialog2.inc'
  include 'design.inc'
  include 'graphlib.inc'

  include 'cursors.inc'

  include 'load_from_parameters.inc'

START:

;--------------------------------------------------------
;---------------set events mask--------------------------
;--------------------------------------------------------
   mov eax,40
   mov ebx,1100111b
   mcall
;---------------------------------------------------------
;-----------------------init data-------------------------
;---------------------------------------------------------
   include 'init_data.inc'
;----------------------------------------------------------
;--------get memory and draw window of program-------------
;----------------------------------------------------------
   call GetMemory
   call cleare_work_arrea
   call load_icons

   ;load cursors
   mov eax,CursorsID
   call load_cursors

   call drawwin
;---------------------------------------------------------
;---------Check loading of file from psrsmeters-----------
;---------------------------------------------------------
  
   mov eax,parameters
   mov ebx,file_path
   call check_loading_from_parameters

;----------------------------------------------------------
;---------------------MAIN LOOP----------------------------
;----------------------------------------------------------

   still:

   call event

   cmp eax,1
   jne no_redraw_window

   call drawwin
   jmp still

   no_redraw_window:

   cmp eax,2
   je keys

   cmp eax,3
   je buttons

   cmp eax,6
   je mouse

   jmp still
;---------------------------------------------------------
;---------------------------------------------------------
;---------------------------------------------------------

   include 'events.inc'
   include 'events_of_window.inc'
   include 'events_of_keys.inc'
   include 'events_of_buttons.inc'
   include 'events_of_mouse.inc'
   include 'panel_engen.inc'
   include 'screen.inc'
   include 'menu_instruments.inc'
   include 'icons_instruments.inc'
   include 'icons.inc'
   include 'sprites.inc'
   include 'string.inc'
   include 'palette.inc'
   include 'files.inc'
   include 'time.inc'
   include 'memory.inc'

;-----------------------------------------------------------
;------------variables and data of program------------------
;-----------------------------------------------------------

CursorsID	     rd 10

parameters           rb 257

file_path:
times 1024+16	     db 0

time		     dd 0
sound_havent_memory  db 150,64,0
PosX		     dd 0
PosY		     dd 0
PointerToIcons	     dd 0
ScreenPointer	     dd 0
PointerToPicture     dd 0
PointerToCopyPicture dd 0
PointerToCopyPicture2 dd 0
PointerToEditBufer   dd 0
PointerToSpriteBufer dd 0
PointerToPalette     dd 0
Color		     dd 0
Number_Brush	     dd 0
Brush_SizeX	     dd 0
Brush_SizeY	     dd 0
Current_instrument   dd 0
Last_instrument      dd 0
Activate_instrument  db 0
SColor		     dd 0
OldX		     dd 0
OldY		     dd 0

MouseX		     dd 0
MouseY		     dd 0
Window_SizeX	     dd 0
Window_SizeY	     dd 0
Window_CordinatX     dd 0
Window_CordinatY     dd 0
Picture_SizeX	     dd 0
Picture_SizeY	     dd 0
ScreenX 	     dd 0
ScreenY 	     dd 0
WorkScreen_SizeX     dd 0
WorkScreen_SizeY     dd 0
MaxWorkScreen_SizeX  dd 0
MaxWorkScreen_SizeY  dd 0
k		     dd 0
IPC_table	     rd 256
ReserveArray	     dd 0
register	     dd 0
CounterX	     dd 0
CounterY	     dd 0
OffsetYPicture	     dd 0
OffsetYWorkScreen    dd 0
OffsetYBigPixel      dd 0

Icon_X		     dd 0
Icon_Y		     dd 0
counter 	     dd 0
counter2	     dd 0
Icon_text_x	     dd 0
Icon_text_y	     dd 0
Panel_flag	     db 0
counter_menu	     dd 0
menu_coordinat_x     dd 0
menu_size_x	     dd 0
menu_counter	     dd 0
counter_11	     dd 0
number_panel	     dd 0
number_menu	     dd 0

Scroll1CoordinatX    dd 0
Scroll1CoordinatY    dd 0
Scroll1MaxSizeX      dd 0
Scroll1MaxSizeY      dd 0
Scroll1SizeX	     dd 0
Scroll1FreeX	     dd 0

Scroll2CoordinatX    dd 0
Scroll2CoordinatY    dd 0
Scroll2MaxSizeX      dd 0
Scroll2MaxSizeY      dd 0
Scroll2SizeY	     dd 0
Scroll2FreeY	     dd 0

extended_memory      dd 0
type		     dw 0
x		     dd 0
y		     dd 0
save_flag	     db 0
exit_from_work_arrea db 0

Radius		     dd 0
Dx_		     dd 0
Dy_		     dd 0
line_width	     dd 0
lastik_is_active     db 0
a_ellips	     dd 0
b_ellips	     dd 0
instrument_used      db 0
used_OldX	     dd 0
used_OldY	     dd 0
rectangular_shade_x  dd 0
rectangular_shade_y  dd 0
crossing_old_x	     dd 0
crossing_old_y	     dd 0
crossing	     dd 0
finishing_crossing   dd 0
number_undo	     dd 0
DrawSprite_flag      db 0
Paste_flag	     db 0
SpriteSizeX	     dd 0
SpriteSizeY	     dd 0
SpriteCoordinatX     dd 0
SpriteCoordinatY     dd 0
SpriteOldCoordinatX  dd 0
SpriteOldCoordinatY  dd 0

   include 'panel_data.inc'
   include 'palitra256.inc'
   include 'brushes.inc'
   include 'spray.inc'
   include 'width_lines.inc'
;----------------------------------------------------------
;-------------------ICON"S picture-------------------------
;----------------------------------------------------------
dd 0

panel_picture:
file 'panel_buttons.gif'

;****************cursors******************
brush_cursor:
file 'brush.cur'

flood_fill_cursor:
file 'flood_fill.cur'

lastik_cursor:
file 'lastik.cur'

other_cursor:
file 'other.cur'

pencil_cursor:
file 'pencil.cur'

pipette_cursor:
file 'pipette.cur'

spray_cursor:
file 'spray.cur'

zoom_cursor:
file 'zoom.cur'

;----------------------------------------------------------

I_END:

IncludeUGlobals
