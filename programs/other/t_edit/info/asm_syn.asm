count_colors_text dd (text-color_wnd_text)/4
count_key_words dd (f1-text)/48
color_cursor dd 0xffd000
color_wnd_capt dd 0x008080
color_wnd_work dd 0x000000
color_wnd_bord dd 0x00ff00
color_select dd 0x808080
color_cur_text dd 0x808080
color_wnd_text:
	dd 0xd0d080
	dd 0xffffff
	dd 0x00ff00
	dd 0xb0b0ff
	dd 0x808080
	dd 0x0099ff
	dd 0xff0099
	dd 0xff8000
	dd 0x00ccff
	dd 0xffcc00
include 'asm.inc'
