
format MS COFF

public _res_caption_left
public _res_caption_right
public _res_caption_body

public _res_panel_left
public _res_panel_right
public _res_panel_body

public _res_border_left
public _res_border_right

public _res_close_btn
public _res_close_btn_hl
public _res_close_btn_pressed
public _res_minimize_btn
public _res_minimize_btn_hl
public _res_minimize_btn_pressed

public _res_play_btn
public _res_play_btn_pressed

public _res_pause_btn
public _res_pause_btn_pressed

public _res_stop_btn
public _res_stop_btn_pressed


public _res_cursor_ns
public _res_cursor_we
public _res_cursor_nwse
public _res_cursor_nesw

;public _res_logo

public _res_level
public _res_slider
public _res_vol_slider

public _res_progress_bar
public _res_prg_level

public _res_def_font

section '.rdata' data readable align 16

_res_caption_left:  file 'cptleft.raw'
_res_caption_right: file 'cptright.raw'
_res_caption_body:  file 'cptbody.raw'

_res_panel_left:  file 'panelleft.raw'
_res_panel_right: file 'panelright.raw'
_res_panel_body:  file 'panel.raw'

_res_border_left:   file 'lborder.raw'
_res_border_right:  file 'rborder.raw'

_res_close_btn:     file 'clbn.raw'
_res_close_btn_hl:  file 'clbhl.raw'
_res_close_btn_pressed: file 'clbp.raw'

_res_play_btn:          file 'playbtn.raw'
_res_play_btn_pressed:  file 'playbp.raw'
_res_pause_btn:         file 'pausebtn.raw'
_res_pause_btn_pressed: file 'pausebp.raw'

_res_stop_btn:         file 'stopbtn.raw'
_res_stop_btn_pressed: file 'stopbtnp.raw'


_res_minimize_btn:         file 'minbn.raw'
_res_minimize_btn_hl:      file 'minbhl.raw'
_res_minimize_btn_pressed: file 'minbp.raw'

_res_cursor_ns:     file 'size_ns.cur'
_res_cursor_we:     file 'size_we.cur'
_res_cursor_nwse:   file 'size_nwse.cur'
_res_cursor_nesw:   file 'size_nesw.cur'

;_res_logo:          file 'logo.raw'

_res_level:         file 'vol_level.raw'
_res_vol_slider:    file 'vol_slider.raw'
_res_slider:        file 'slider.raw'

_res_progress_bar:  file 'pbar.raw'
_res_prg_level:     file 'prg_level.raw'

_res_def_font:      file 'IstokWeb.ttf'
