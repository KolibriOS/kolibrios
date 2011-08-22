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
color_cursor dd 0xffd000
color_wnd_capt dd 0x0000a0
color_wnd_work dd 0x000000
color_wnd_bord dd 0xc0c0c0
color_select dd 0x000080
color_cur_text dd 0x8080ff
color_wnd_text:
	dd 0x808080
	dd 0xffffff
	dd 0xffff00
	dd 0x008080
	dd 0x0000ff
	dd 0x0080ff
text:
wo<'"'>,0,4,34,3
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
