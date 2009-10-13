;*****************************************************************************
; Box_Lib - library of graphical components
;
; Authors:
; Alexey Teplov aka <Lrz>
; Marat Zakiyanov aka Mario79, aka Mario
; Evtikhov Maxim aka Maxxxx32
; Eugene Grechnikov aka Diamond
; hidnplayr
;*****************************************************************************

format MS COFF

public EXPORTS

section '.flat' code readable align 16
include '../../../../macros.inc'
include 'editbox.mac'   ;macro which should make life easier :) specially for editbox
include 'checkbox.mac'	;macro implements checkbox
include 'optionbox.mac' ;macro implements optionbox
include 'scrollbar.mac' ;macro implements scrollbar
include 'd_button.mac' ;macro implements dinamic_button
include 'menubar.mac' ;macro implements menubar
include 'filebrowser.mac' ;macro implements filebrowser
;----------------------------------------------------
;EditBox
;----------------------------------------------------
align 16
use_editbox_draw        ;macro reveals the function of the display.
align 16
use_editbox_key         ;macro reveals processing function of the keypad.
align 16
use_editbox_mouse       ;macro reveals processing function of the mouse.

;----------------------------------------------------
;CheckBox
;----------------------------------------------------
align 16
use_checkbox_draw       ;macro reveals the function of the display.
align 16
use_checkbox_mouse      ;macro reveals processing function of the mouse.


;--------------------------------------------------
;radiobutton Group
;--------------------------------------------------
align 16
use_optionbox_driver    ;macro that control the operating modes
align 16
use_optionbox_draw      ;macro reveals the function of the display.
align 16
use_optionbox_mouse     ;macro reveals processing function of the mouse.

;--------------------------------------------------
;scrollbar Group
;--------------------------------------------------
align 16
use_scroll_bar
align 16
use_scroll_bar_vertical
align 16
use_scroll_bar_horizontal

;--------------------------------------------------
;dinamic button Group
;--------------------------------------------------
align 16
use_dinamic_button

;--------------------------------------------------
;menubar Group
;--------------------------------------------------
align 16
use_menu_bar

;--------------------------------------------------
;filebrowser Group
;--------------------------------------------------
align 16
use_file_browser
;--------------------------------------------------
;align 16
init:
ret

;;;;;;;;;;;
;;Data
;;;;;;;;;;;
;align 16
;mouse_flag dd 0x0


align 16
EXPORTS:


        dd      sz_init,                init
        dd      sz_version,             0x00000001

        dd      sz_edit_box,            edit_box
        dd      sz_edit_box_key,        edit_box_key
        dd      sz_edit_box_mouse,      edit_box_mouse
        dd      szVersion_ed,           0x00000001

        dd      sz_check_box_draw,      check_box_draw
        dd      sz_check_box_mouse,     check_box_mouse
        dd      szVersion_ch,           0x00000001

        dd      sz_option_box_draw,     option_box_draw
        dd      sz_option_box_mouse,    option_box_mouse
        dd      szVersion_op,           0x00000001

        dd      sz_Scrollbar_ver_draw,  scroll_bar_vertical.draw
        dd      sz_Scrollbar_ver_mouse, scroll_bar_vertical.mouse
        dd      sz_Scrollbar_hor_draw,  scroll_bar_horizontal.draw
        dd      sz_Scrollbar_hor_mouse, scroll_bar_horizontal.mouse
        dd      szVersion_scrollbar,    0x00010001

        dd      sz_Dbutton_draw,        dinamic_button.draw
        dd      sz_Dbutton_mouse,       dinamic_button.mouse
        dd      szVersion_dbutton,      0x00010001

        dd      sz_Menu_bar_draw,       menu_bar.draw
        dd      sz_Menu_bar_mouse,      menu_bar.mouse
        dd      sz_Menu_bar_activate,   menu_bar.activate
        dd      szVersion_menu_bar,     0x00010002

        dd      sz_FileBrowser_draw,    fb_draw_panel
        dd      sz_FileBrowser_mouse,   fb_mouse
        dd      sz_FileBrowser_key,     fb_key
        dd      szVersion_FileBrowser,  0x00010001
        dd      0,0


sz_init                db 'lib_init',0
sz_version             db 'version',0

sz_edit_box            db 'edit_box',0
sz_edit_box_key        db 'edit_box_key',0
sz_edit_box_mouse      db 'edit_box_mouse',0
szVersion_ed           db 'version_ed',0

sz_check_box_draw      db 'check_box_draw',0
sz_check_box_mouse     db 'check_box_mouse',0
szVersion_ch           db 'version_ch',0

sz_option_box_draw     db 'option_box_draw',0
sz_option_box_mouse    db 'option_box_mouse',0
szVersion_op           db 'version_op',0

sz_Scrollbar_ver_draw   db 'scrollbar_v_draw',0
sz_Scrollbar_ver_mouse  db 'scrollbar_v_mouse',0
sz_Scrollbar_hor_draw   db 'scrollbar_h_draw',0
sz_Scrollbar_hor_mouse  db 'scrollbar_h_mouse',0
szVersion_scrollbar     db 'version_scrollbar',0

sz_Dbutton_draw                 db 'dbutton_draw',0
sz_Dbutton_mouse                db 'dbutton_mouse',0
szVersion_dbutton               db 'version_dbutton',0

sz_Menu_bar_draw                db 'menu_bar_draw',0
sz_Menu_bar_mouse               db 'menu_bar_mouse',0
sz_Menu_bar_activate            db 'menu_bar_activate',0
szVersion_menu_bar              db 'version_menu_bar',0

sz_FileBrowser_draw             db 'FileBrowser_draw',0
sz_FileBrowser_mouse            db 'FileBrowser_mouse',0
sz_FileBrowser_key              db 'FileBrowser_key',0
szVersion_FileBrowser           db 'version_FileBrowser',0