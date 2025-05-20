; SPDX-License-Identifier: NOASSERTION
;

;*******************************************************
;**************GRAPHICS EDITOR ANIMAGE *****************
;*******************************************************
; version: 1.52
; last update:  23.11.2016
; changes:      Can save *.png files
; author:       IgorA
;--------------------------------------------------------
; version: 1.51
; last update:  23.03.2016
; changes:      Use library 'kmenu.obj', update GUI
; authors:      IgorA, Veliant, Leency
;--------------------------------------------------------
; version: 1.4
; last update:  12.03.2016
; changes:      Use library 'libimg.obj'
; author:       IgorA
;--------------------------------------------------------
; version: 1.3
; last update:  05.10.2010
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      Fixed window flicker when redrawing,
;               Fixed memory leak for stack
;--------------------------------------------------------
; version: 1.2
; last update:  30.09.2010
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      Program used function 68 instead 64 is now,
;               select path with OpenDialog
;--------------------------------------------------------
; version: 1.1
; last update:  09.12.2006
; authors:
; programming by andrew_programmer
; design by golus

use32
org     0
        db 'MENUET01'
        dd 1, START, IM_END, I_END
        dd stacktop, file_path, cur_dir_path

include '../../config.inc' ;for nightbuild
include '../../macros.inc'
include '../../proc32.inc'
include '../../KOSfuncs.inc'
include '../../load_lib.mac'
include '../../dll.inc'
include '../../develop/libraries/libs-dev/libio/libio.inc'
include '../../develop/libraries/libs-dev/libimg/libimg.inc'
;include '../../debug.inc'

@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load

;---------------------------------------------------------
; *** constants for interface  ***

; skin adjustments
ci_offs_skin_w equ  5 ;adjustment to the width of the skin frame
ci_offs_skin_h equ 24 ;skin height adjustment

; main window
ci_wnd_min_siz_x equ 585 ;minimum size x
ci_wnd_min_siz_y equ 400 ;minimum size y

; toolbar
ci_panel_x_pos equ  0 ;coord. x for panel
ci_panel_y_pos equ 20 ;coord. y for panel
ci_panel_but_y1 equ ci_panel_y_pos +5 ;y coordinate for 1st row of buttons
ci_panel_but_y2 equ ci_panel_y_pos+30 ;y coordinate for 2nd row of buttons
ci_palete_y_pos equ ci_panel_y_pos+51 ;y coordinate for color palette
ci_panel_zoom_x equ 178 ;x coordinate for zoom bar buttons

; editor window
ci_edit_wnd_x_pos  equ  0 ;x coordinate for editor window
ci_edit_wnd_y_pos  equ 71 ;y coordinate for editor window
ci_edit_wnd_border equ  3 ;frame around the editor window

; scrolling
ci_scroll_dim equ 22 ;scroll sizes
ci_scrollh_coord_x_min equ (ci_edit_wnd_x_pos+3) ;x minimum slider position
        ;horizontal scrolling
ci_scrollv_coord_y_min equ (ci_edit_wnd_y_pos+3) ;y minimum slider position
        ;vertical scrolling
;---------------------------------------------------------

include 'bmplib.inc'
include 'dialog2.inc'
include 'design.inc'
include 'graphlib.inc'

include 'cursors.inc'
include 'memory.inc'
include 'load_from_parameters.inc'

START:
        mcall SF_SYS_MISC,SSF_HEAP_INIT
        mcall SF_STYLE_SETTINGS, SSF_GET_COLORS, syscolors, syscolors_end-syscolors

load_libraries l_libs_start,end_l_libs

        cmp     eax,-1
        jz      close

        mcall SF_SET_EVENTS_MASK,0x80000067 ; 1100111b
;---------------------------------------------------------
;-----------------------init data-------------------------
;---------------------------------------------------------
include 'init_data.inc'

;----------------------------------------------------------
;--------get memory and draw window of program-------------
;----------------------------------------------------------
        call GetMemory
        mov     [Current_instrument],10 ;pencil
        call TakeButtonInstruments  ;set startup instrument
        call cleare_work_arrea
        call load_icons
        call init_main_menu

;load cursors
        mov     eax,CursorsID
        call load_cursors

;---------------------------------------------------------
;---------check loading of file from parameters-----------
;---------------------------------------------------------
        mov     eax,file_path
        cmp [eax],byte 0
        jz @f
        call load_picture
        call MovePictureToWorkScreen
@@:

;---------------------------------------------------------------------
        mov     edi,filename_area
        mov     esi,path4+5
        call    copy_str_1

        mov     edi,file_path
        cmp     [edi],byte 0
        jne     @f
        mov     esi,path4
        call    copy_str_1
@@:
;OpenDialog     initialisation
        stdcall [OpenDialog_Init], OpenDialog_data
        stdcall [ColorDialog_Init], ColorDialog_data
;---------------------------------------------------------------------

align 4
red:
        call drawwin
;----------------------------------------------------------
;---------------------main loop----------------------------
;----------------------------------------------------------
align 4
still:
        mcall SF_WAIT_EVENT

        cmp     eax,1
        je      red

        cmp     eax,2
        je      keys

        cmp     eax,3
        je      buttons

        cmp     eax,6
        je      mouse

        jmp     still

;---------------------------------------------------------------------
copy_str_1:
        xor     eax,eax
        cld
@@:
        lodsb
        stosb
        test    eax,eax
        jnz     @b
        ret
;---------------------------------------------------------------------
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
include 'menu.inc'
;-----------------------------------------------------------
;------------variables and data of program------------------
;-----------------------------------------------------------
;sound_havent_memory db 150,64,0

include 'lib_data.inc'
include 'panel_data.inc'
include 'brushes.inc'
include 'spray.inc'
include 'width_lines.inc'
;----------------------------------------------------------
;-------------------icon's picture-------------------------
;----------------------------------------------------------
align 4
panel_picture:
file 'panel_buttons.png'
.end:
align 4
panel_zoom:
file 'panel_zoom.png'
.end:
;****************cursors******************
brush_cursor:
file 'cursors/brush.cur'
flood_fill_cursor:
file 'cursors/flood_fill.cur'
lastik_cursor:
file 'cursors/lastik.cur'
other_cursor:
file 'cursors/other.cur'
pencil_cursor:
file 'cursors/pencil.cur'
pipette_cursor:
file 'cursors/pipette.cur'
spray_cursor:
file 'cursors/spray.cur'
zoom_cursor:
file 'cursors/zoom.cur'
;----------------------------------------------------------
align 4
IM_END:
;-----------------------------------------------------------
;------------variables and data of program------------------
;-----------------------------------------------------------
time                    rd 1
PosX                    rd 1 ;scroll x file position
PosY                    rd 1 ;scroll y file position
PointerToIcons          rd 1
ScreenPointer           rd 1
PointerToPicture        rd 1
PointerToCopyPicture    rd 1
PointerToCopyPicture2   rd 1
PointerToEditBufer      rd 1
PointerToSpriteBufer    rd 1
PointerToPalette        rd 1 ;pointer to the file (needed for saving in *.bmp)
Color                   rd 1
SColor                  rd 1
Number_Brush            rd 1
Brush_SizeX             rd 1
Brush_SizeY             rd 1
Current_instrument      rd 1
Last_instrument         rd 1
OldX                    rd 1
OldY                    rd 1

MouseX                  rd 1
MouseY                  rd 1
MouseBut                rd 1 ;mouse button events
Window_SizeX            rd 1
Window_SizeY            rd 1
Window_CordinatX        rd 1
Window_CordinatY        rd 1
Picture_SizeX           rd 1
Picture_SizeY           rd 1
ScreenX                 rd 1 ;x coordinate of the cursor accounting for the scale
ScreenY                 rd 1 ;y coordinate of the cursor accounting for the scale
WorkScreen_SizeX        rd 1 ;width of the working screen
WorkScreen_SizeY        rd 1 ;height of the working screen
MaxWorkScreen_SizeX     rd 1
MaxWorkScreen_SizeY     rd 1
k                       rd 1 ;scale

ReserveArray            rd 1 ;pointer to memory used when drawing figures
        ;to save background areas under figures, as well as for filling
CounterX                rd 1 ;number of image pixels by width that fall
        ;within the workspace screen, given the current zoom level
CounterY                rd 1
OffsetYPicture          rd 1 ;y-shift in image buffer when drawing
        ;next line of image
OffsetYWorkScreen       rd 1 ;y-shift in the workspace buffer when
        ;drawing the next line of pixels. The larger the scale, the lower
        ;you need to go down the workspace.
OffsetYBigPixel         rd 1 ;shift along the y-axis to draw pixels on a large
        ;scale. The 1st strip of the pixel is drawn, and then you need to move down and
        ;left to draw the next strip.

Icon_X                  rd 1
Icon_Y                  rd 1
counter                 rd 1
counter2                rd 1
number_panel            rd 1
number_menu             rd 1

Scroll1CoordinatX       rd 1 ;scroll x screen position
Scroll1CoordinatY       rd 1 ;scroll y screen position
Scroll1MaxSizeX         rd 1
Scroll1MaxSizeY         rd 1
Scroll1SizeX            rd 1 ;scroll polzunok size
Scroll1FreeX            rd 1

Scroll2CoordinatX       rd 1
Scroll2CoordinatY       rd 1
Scroll2MaxSizeX         rd 1
Scroll2MaxSizeY         rd 1
Scroll2SizeY            rd 1
Scroll2FreeY            rd 1

x                       rd 1
y                       rd 1

Radius                  rd 1
Dx_                     rd 1
Dy_                     rd 1
line_width              rd 1
a_ellips                rd 1
b_ellips                rd 1

used_OldX               rd 1 ;for draw hard contour
used_OldY               rd 1
paste_img_w rd 1 ;width of the inserted image
paste_img_h rd 1 ;height of the inserted image
crossing_old_x          rd 1 ;initial x-coord of copy area
crossing_old_y          rd 1 ;initial y-coord of copy area
rectangular_shade_x     rd 1 ;end x-coord of copy area
rectangular_shade_y     rd 1 ;end y-coord of copy area
crossing                rd 1 ;0 - no selection, 1 - selection in progress,
        ;2 - selection complete, 3 - selection area moving
number_undo             rd 1
SpriteSizeX             rd 1 ;???
SpriteSizeY             rd 1 ;???
SpriteCoordinatX        rd 1
SpriteCoordinatY        rd 1
SpriteOldCoordinatX     rd 1
SpriteOldCoordinatY     rd 1

CursorsID       rd 10

Activate_instrument     rb 1 ;if equal to 0 - then copy the current buffer
        ;for editing it, if 1 - the current buffer is not copied
save_flag               rb 1
exit_from_work_arrea    rb 1
lastik_is_active        rb 1
instrument_used         rb 1
DrawSprite_flag         rb 1
Paste_flag              rb 1
;---------------------------------------------------------------------
IncludeUGlobals
;---------------------------------------------------------------------
align 4
file_path rb 4096
filename_area rb 256
temp_dir_pach rb 4096
library_path rb 4096
cur_dir_path rb 4096
procinfo: rb 1024
align 4
syscolors rb 192
syscolors_end:
;---------------------------------------------------------------------
align 4
        rb 4096
stacktop:
;---------------------------------------------------------------------
I_END:
