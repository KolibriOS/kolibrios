macro wo txt,lf1,p1,p2,p3{
@@: db txt
rb @b+40-$
dd lf1
db p1
dw p2+0
db p3
}
count_colors_text dd (text-color_wnd_text)/4
count_key_words dd (f1-text)/48
color_cursor dd 0xf1fcd0
color_wnd_capt dd 0x080808 ;sidebar bg
color_wnd_work dd 0x1C1C1C ;main work area bg
color_wnd_bord dd 0x606060 ;sidebar text
color_select dd 0x3E3D32
color_cur_text dd 0x808080
color_wnd_text:
	dd 0xD0D0D0 ;F8F8F2
	dd 0xffff00
	dd 0x00ff00
	dd 0x00ffff
	dd 0x808080
	dd 0xff40ff
	dd 0x4080ff
	dd 0xff0000
	dd 0x8080ff
	dd 0x00ccff
text:
wo<'"'>,0,4,34,3
wo<';'>,0,4,13,4
wo<'#'>,0,4,13,4
wo<'0'>,0,24,,3
wo<'1'>,0,24,,3
wo<'2'>,0,24,,3
wo<'3'>,0,24,,3
wo<'4'>,0,24,,3
wo<'5'>,0,24,,3
wo<'6'>,0,24,,3
wo<'7'>,0,24,,3
wo<'8'>,0,24,,3
wo<'9'>,0,24,,3
wo<'='>,0,0,,1
wo<'['>,0,4,93,2
wo<'auto'>,0,3,,5
wo<'default'>,0,3,,5
wo<'disabled'>,0,3,,5
wo<'false'>,0,3,,5
wo<'none'>,0,3,,5
wo<'true'>,0,3,,5
f1: db 0
