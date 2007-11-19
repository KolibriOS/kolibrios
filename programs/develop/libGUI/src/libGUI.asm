;**********************************************************************
; library of Graphics Universal Interface for Kolibri operation system
;
; version 071119
; 2007 year
;
;autors:
;
;andrew_programmer  polynki@mail.ru
;
;menegement of controls : destroy_control, send_message,move_component
;                         resize_component,activate_trap_for_specialized_message
;GUI components : button,scroller,Bookmark,text,image,number,progres_bar
;
;<Lrz> and Maxxxx32
;
;GUI components : CheckBox,EditBox
;
;**********************************************************************

format MS COFF

public EXPORTS

control_header_size = 44

border_width        = 5

activate_trap       = 01000000b
deactivate_trap     = 10111111b

section '.flat' code readable align 16

     include 'macros.inc'
     include 'debug.inc'

     ;GUI components

     include 'button.inc'

     include 'scroller.inc'

     include 'bookmark.inc'

     include 'image.inc'

     include 'text.inc'

     include 'number.inc'

     include 'check_box.inc'

     include 'edit_box.inc'

     include 'progress_bar.inc'

     ;engen of libGUI(menegment of controls)

     include 'menegment_of_controls.inc'

     ;functions which proved work of GUI components

     include 'draw.inc'

     include 'string.inc'

     include 'check_crossing_box.inc'

     include 'memory.inc'

     ;function for get version of library
     include 'version.inc'

     ;function for resize GUI component
     include 'resize_component.inc'

     ;function for move GUI component
     include 'move_component.inc'

     ;function for activate trap for specialized messages
     include 'activate_trap.inc'

align 16
EXPORTS:
                   dd szDestroyControl,destroy_control
                   dd szSendMessage,send_message
                   dd szVersion,get_version
                   dd szResizeComponent,resize_component
                   dd szMoveComponent,move_component
                   dd szActivateTrap,activate_trap_for_specialized_message
                   dd szcraeteButton,craete_button
                   dd szcraeteScroller,craete_scroller
                   dd szcraeteBookmark,craete_Bookmark
                   dd szcraeteImage,craete_image
                   dd szcraeteText,craete_text
                   dd szcraeteNumber,craete_number
                   dd szcraeteCheckBox,craete_check_box
                   dd szcraeteEditBox,craete_edit_box
                   dd szcraeteProgressBar,craete_progress_bar
                   dd 0,0

szDestroyControl    db 'DestroyControl',0
szSendMessage       db 'SendMessage',0
szVersion           db 'Version',0
szResizeComponent   db 'ResizeComponent',0
szMoveComponent     db 'MoveComponent',0
szActivateTrap      db 'ActivateTrapForSpecializedMessage',0
szcraeteButton      db 'CraeteButton',0
szcraeteScroller    db 'CraeteScroller',0
szcraeteBookmark    db 'CraeteBookmark',0
szcraeteImage       db 'CraeteImage',0
szcraeteText        db 'CraeteText',0
szcraeteNumber      db 'CraeteNumber',0
szcraeteCheckBox    db 'CraeteCheckbox',0
szcraeteEditBox     db 'CraeteEditbox',0
szcraeteProgressBar db 'CraeteProgressbar',0

section '.data' data readable writable align 16
;************************************************
;******************DLL DATA**********************
;************************************************

point       db '.',0
signum      db '-',0

BitsPerPixel              rd 1
BytesPerString            rd 1
WindowCoordinatX          rd 1
WindowCoordinatY          rd 1
WindowSizeX               rd 1
WindowSizeY               rd 1

PointerToMem              rd 1
offset                    rd 1

v                         rd 1
v2                        rd 1
v3                        rd 1

r_min                     rb 1
r_max                     rb 1
g_min                     rb 1
g_max                     rb 1
b_min                     rb 1
b_max                     rb 1
r                         rb 1
g                         rb 1
b                         rb 1
r_f                       rd 1
g_f                       rd 1
b_f                       rd 1
step_r                    rd 1
step_g                    rd 1
step_b                    rd 1
length                    rd 1
length2                   rd 1
Color                     rd 1
AveregeColor              rd 1

line_coordinat_x          rd 1
line_coordinat_y          rd 1
line_size_x               rd 1
line_size_y               rd 1
line_size_y_f             rd 1

x                         rd 1
y                         rd 1
xo                        rd 1
yo                        rd 1
x_ctl                     rd 1
y_ctl                     rd 1

;ReturnAddresButton        rd 1
;ReturnAddresScroller      rd 1
;ReturnAddresBookmark      rd 1
;ReturnAddresImage         rd 1
;ReturnAddresText          rd 1
;ReturnAddresNumber        rd 1
;ReturnAddresCheckBox      rd 1
;ReturnAddresEditBox       rd 1
;ReturnAddresProgressBar   rd 1

Button_Flag               rb 1

Scrollersize              rd 1
ControlID                 rd 1
Message                   rd 4
Parend                    rd 1
Control                   rd 1
ReturnControl             rd 1
PointerToControl          rd 1
ActiveControl             rd 1
ActiveControlForKeys      rd 1
ButtonsOfMouse            rd 1

PointerToStructureForBookmark    rd 1
ParendForBookmark                rd 1
PointerToStructureForButton      rd 1
ParendForButton                  rd 1
PointerToStructureForScroller    rd 1
ParendForScroller                rd 1
PointerToStructureForImage       rd 1
PointerToStructureForText        rd 1
PointerToStructureForNumber      rd 1
PointerToStructureForCheckBox    rd 1
PointerToStructureForEditBox     rd 1
PointerToStructureForProgressBar rd 1

PointerForButton                rd 1
PointerForScroller              rd 1
PointerForBookmark              rd 1
PointerForImage                 rd 1
PointerForText                  rd 1
PointerForNumber                rd 1
PointerForCheckBox              rd 1
PointerForEditBox               rd 1
PointerForProgressBar           rd 1

ChisloZakladok                  rd 1

integer_part                    rd 1
float_part                      rd 1
tochnost1                       rd 1
tochnost2                       rd 1
signum_float_number             rb 1

skin_height                     rd 1

;*********************************
;**********GUI structures*********
;*********************************

;struc CONTROL
;{
; .ctrl_proc        rd 0  ;0
; .ctrl_fd          rd 0  ;4
; .ctrl_bk          rd 0  ;8
; .child_fd         rd 0  ;12
; .child_bk         rd 0  ;16
; .parend           rd 0  ;20
; .x                rd 0  ;24
; .y                rd 0  ;28
; .sizex            rd 0  ;32
; .sizey            rd 0  ;36
; .ID               rd 0  ;40
;}

struc BUTTON
{
 .ctrl_proc        rd 1
 .ctrl_fd          rd 1
 .ctrl_bk          rd 1
 .child_fd         rd 1
 .child_bk         rd 1
 .parend           rd 1
 .ctrl_x           rd 1
 .ctrl_y           rd 1
 .ctrl_sizex       rd 1
 .ctrl_sizey       rd 1
 .ctrl_ID          rd 1

 .type             rb 1
 .flag             rb 1
 .x                rw 1
 .y                rw 1
 .width            rw 1
 .height           rw 1
 .image            rd 1
 .imageX           rw 1
 .imageY           rw 1
 .imageSizeX       rw 1
 .imageSizeY       rw 1
 .transparentColor rd 1
 .text             rd 1
 .textX            rw 1
 .textY            rw 1
 .textcolor        rd 1
 .color1           rd 1
 .color2           rd 1
 .mouseX           rw 1
 .mouseY           rw 1
}

struc SCROLLER
{
 .ctrl_proc        rd 1
 .ctrl_fd          rd 1
 .ctrl_bk          rd 1
 .child_fd         rd 1
 .child_bk         rd 1
 .parend           rd 1
 .ctrl_x           rd 1
 .ctrl_y           rd 1
 .ctrl_sizex       rd 1
 .ctrl_sizey       rd 1
 .ctrl_ID          rd 1

 .type             rb 1
 .x                rw 1
 .y                rw 1
 .length           rw 1
 .color1           rd 1
 .size             rd 1
 .pos              rd 1
 .buttons_flags    rw 1
 .ChildButton1     rd 1
 .ChildButton2     rd 1
 .mouseX           rw 1
 .mouseY           rw 1
}

;********************************
;*********Child Buttons**********
;********************************
struc CHILDBUTTON
{
 .type             rb 1
 .flag             rb 1
 .x                rw 1
 .y                rw 1
 .width            rw 1
 .height           rw 1
 .image            rd 1
 .imageX           rw 1
 .imageY           rw 1
 .imageSizeX       rw 1
 .imageSizeY       rw 1
 .transparentColor rd 1
 .text             rd 1
 .textX            rw 1
 .textY            rw 1
 .textcolor        rd 1
 .color1           rd 1
 .color2           rd 1
 .mouseX           rw 1
 .mouseY           rw 1
}

struc BOOKMARK
{
 .type                          rb 1
 .flag                          rb 1
 .x                             rd 1
 .y                             rd 1
 .sizex                         rd 1
 .sizey                         rd 1
 .color1                        rd 1
 .reserved                      rd 1
 .color2                        rd 1

 .FullBookmarkSizeX             rd 1
 .BookmarkSizeX                 rd 1
 .PointerToTextForBookmark      rd 1
 .BookmarkX                     rd 1
 .BookmarkY                     rd 1
 .ChildButtonsForBookmark       rd 1
 .CounterChildButtons           rd 1
 .ChisloZakladok                rd 1
 .ChisloStrokeZakladok          rd 1
 .MessageForChildButton         rd 5
 .ChildControlForBookmark       rd 1
 .NumberBookmarksInActiveString rd 1
 .NumberBookmarksInEndString    rd 1
 .PointerToActiveBookmarks      rd 1
 .PointerToEndBookmarks         rd 1
 .BookmarkFlag                  rb 1
 .y_end_stroke                  rd 1

 .CounterChildControls          rd 1
 .ActiveChildControl            rd 1
 .AddresOfActiveChildControl    rd 1
 .MouseX                        rd 1
 .MouseY                        rd 1

 .DefectSizeX                   rd 1
 .ControlAddres                 rd 1

 .counter1                      rd 1
 .counter2                      rd 1

 .NumberActiveControl           rd 1

 .WorkPlace_x                   rd 1
 .WorkPlace_y                   rd 1
 .WorkPlace_sizex               rd 1
 .WorkPlace_sizey               rd 1
 .WorkPlace_windowx             rd 1
 .WorkPlace_windowsizex         rd 1

}

struc IMAGE
{
 .type                          rb 1
 .flag                          rb 1
 .color                         rd 1
 .x                             rd 1
 .y                             rd 1
 .sizex                         rd 1
 .sizey                         rd 1
 .pointer                       rd 1
}

struc TEXT
{
 .type                          rb 1
 .flag                          rb 1
 .color                         rd 1
 .x                             rd 1
 .y                             rd 1
 .length                        rd 1
 .pointer                       rd 1
}

struc NUMBER
{
  .type                         rb 1
  .flag                         rb 1
  .color                        rd 1
  .x                            rd 1
  .y                            rd 1
  .number                       rd 1
  .parameters                   rd 1
}

struc CHECKBOX
{
 .ch_flags                      rw 1
 .ch_left                       rw 1
 .ch_top                        rw 1
 .ch_text_margin                rd 1
 .ch_size                       rd 1
 .ch_size_2                     rw 1
 .ch_size_3                     rw 1
 .ch_color                      rd 1
 .ch_border_color               rd 1
 .ch_text_color                 rd 1
 .ch_text_ptr                   rd 1
 .ch_text_length                rw 1
 .mouseX                        rd 1
 .mouseY                        rd 1
}

struc EDITBOX
{
 .ed_width                      rd 1
 .ed_left                       rd 1
 .ed_top                        rd 1
 .ed_color                      rd 1
 .shift_color                   rd 1
 .ed_focus_border_color         rd 1
 .ed_blur_border_color          rd 1
 .ed_text_color                 rd 1
 .ed_max                        rd 1
 .ed_text                       rd 1
 .ed_flags                      rw 1
 .ed_size                       rd 1
 .ed_pos                        rd 1
 .ed_offset                     rd 1
 .cl_curs_x                     rd 1
 .cl_curs_y                     rd 1
 .ed_shift_pos                  rd 1
 .ed_shift_pos_old              rd 1
 .ed_height                     rd 1
 .mouseX                        rd 1
 .mouseY                        rd 1
}

struc PROGRESSBAR
{
 .type                          rb 1
 .flag                          rb 1
 .color1                        rd 1
 .color2                        rd 1
 .x                             rd 1
 .y                             rd 1
 .sizex                         rd 1
 .sizey                         rd 1
 .progress                      rd 1
 .color3                        rd 1
}

;********************************
;*Graphics primitives structures*
;********************************
struc LINE
{
 .x1               rd 1
 .y1               rd 1
 .x2               rd 1
 .y2               rd 1
 .color            rd 1
}

struc PIXEL
{
 .x                rd 1
 .y                rd 1
 .color            rd 1
}

struc FONT1
{
 .sizeX            rd 6
 .sizeY            rd 9
 .x                rd 1
 .y                rd 1
}

struc RECTANGLE
{
 .x                rd 1
 .y                rd 1
 .width            rd 1
 .height           rd 1
 .color            rd 1
}

struc SCROLLBAR
{
 .x                rd 1
 .y                rd 1
}

Line               LINE
Pixel              PIXEL
Font               FONT1
Rectangle          RECTANGLE
ScrollBar          SCROLLBAR
ChildButton        CHILDBUTTON

Button             BUTTON
Scroller           SCROLLER
Bookmark           BOOKMARK
Image              IMAGE
Text               TEXT
Number             NUMBER
CheckBox           CHECKBOX
EditBox            EDITBOX
ProgressBar        PROGRESSBAR

pointer                   rd 50
pointer2                  rd 13
IPC_table  process_information;               rb 1024
colors_table1             rd 15
colors_table2             rd 15
