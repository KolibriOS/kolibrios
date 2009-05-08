;Libray from Editbox
; SEE YOU File FAQ.txt and HISTORY. Good Like!
;Last Change 13/02/2009
;;;;;;;;;;;;;;;;;;

format MS COFF

public EXPORTS

section '.flat' code readable align 16
include 'macros.inc'
include 'editbox.mac'   ;макрос который должен облегчить жизнь :) специально для editbox
include 'checkbox.mac'  ;макрос содержит реализацию checkbox
include 'optionbox.mac' ;макрос содержит реализацию optionbox
include 'scrollbar.mac' ;макрос содержит реализацию scrollbar
include 'd_button.mac' ;макрос содержит реализацию dinamic button
include 'menubar.mac' ;макрос содержит реализацию menubar
;----------------------------------------------------
;EditBox
;----------------------------------------------------
align 4
use_editbox_draw        ;макрос расскрывает функцию отображения бокса.
align 4
use_editbox_key         ;макрос расскрывает функцию обработки клавишь.
align 4
use_editbox_mouse       ;макрос расскрывает функцию обработки мыши.

;----------------------------------------------------
;CheckBox
;----------------------------------------------------
align 4
use_checkbox_draw       ;макрос расскрывает функцию отображения бокса.
align 4
use_checkbox_mouse      ;макрос расскрывает функцию обработки мыши.


;--------------------------------------------------
;radiobutton Group
;--------------------------------------------------
align 4
use_optionbox_driver    ;макросы которые управляют работой бокса )
align 4
use_optionbox_draw      ;макрос расскрывает функцию отображения бокса.
align 4
use_optionbox_mouse     ;макрос расскрывает функцию обработки мыши.

;--------------------------------------------------
;scrollbar Group
;--------------------------------------------------
align 4
use_scroll_bar
align 4
use_scroll_bar_vertical
align 4
use_scroll_bar_horizontal

;--------------------------------------------------
;dinamic button Group
;--------------------------------------------------
align 4
use_dinamic_button

;--------------------------------------------------
;menubar Group
;--------------------------------------------------
align 4
use_menu_bar


;--------------------------------------------------
init:
ret

;;;;;;;;;;;
;;Data
;;;;;;;;;;;
align 4
mouse_flag dd 0x0


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
        dd      szVersion_menu_bar,     0x00010001
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
szVersion_menu_bar              db 'version_menu_bar',0

