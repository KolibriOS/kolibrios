count_colors_text dd (text-color_wnd_text)/4
count_key_words dd (f1-text)/48
color_cursor dd 0xf1fcd0
color_wnd_capt dd 0x0000a0
color_wnd_work dd 0x000000
color_wnd_bord dd 0xc0c0c0
color_select dd 0x808080
color_cur_text dd 0x808080
color_wnd_text:
	dd 0xffff00
	dd 0xffffff
	dd 0x00ff00
	dd 0x00ffff
	dd 0x808080
	dd 0xff40ff
	dd 0x4080ff
	dd 0xff0000
	dd 0x8080ff
	dd 0x00ccff
include 'asm.inc'
