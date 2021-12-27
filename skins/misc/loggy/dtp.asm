; [ DTP ] Common Colors
include 'loggy.inc'
; set table
struc system_colors
{
__menu_body    dd menu_body
__3d_face      dd 3d_face
__3d_dark      dd 3d_dark
__3d_light     dd 3d_light
__win_title    dd win_title
__win_body     dd win_body
__btn_face     dd btn_face
__btn_text     dd btn_text

__win_text     dd win_text
__panel_frame  dd panel_frame
__win_face     dd win_face
__win_inface   dd win_inface
__win_frame    dd win_frame
__win_inframe  dd win_inframe
__win_border   dd win_border
__win_inborder dd win_inborder

__win_graytext dd win_graytext
__menu_frame   dd menu_frame
__menu_text    dd menu_text
__panel_body   dd panel_body
__panel_text   dd panel_text
__hint_frame   dd hint_frame
__hint_body    dd hint_body
__hint_text    dd hint_text

__btn_inface   dd btn_inface
__btn_fcface   dd btn_fcface
__btn_frame    dd btn_frame
__btn_inframe  dd btn_inframe
__btn_fcframe  dd btn_fcframe
__btn_intext   dd btn_intext
__btn_fctext   dd btn_fctext
__gui_shadow   dd gui_shadow

__gui_face     dd gui_face
__gui_inface   dd gui_inface
__gui_fcface   dd gui_fcface
__gui_frame    dd gui_frame
__gui_inframe  dd gui_inframe
__gui_fcframe  dd gui_fcframe
__gui_text     dd gui_text
__gui_intext   dd gui_intext

__gui_fctext   dd gui_fctext
__gui_select   dd gui_select
__res_var_a    dd reserved
__res_var_b    dd reserved
__res_var_c    dd reserved
__res_var_d    dd reserved
__res_var_e    dd reserved
__res_var_f    dd reserved
}
struct system_colors
