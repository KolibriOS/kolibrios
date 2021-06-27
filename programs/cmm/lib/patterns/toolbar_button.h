
:unsigned int DrawTopPanelButton(dword _button_id, _x, _y, signed int _icon_n, bool pressed)
{
	#define TSZE 25
	static libimg_image top_icons;
	static dword semi_white=0, bg_col_light, bg_col_dark;
	int i;
	if (!semi_white) {
		top_icons.load("/sys/icons16.png");

		semi_white = MixColors(sc.work, 0xFFFfff, skin_is_dark()*90 + 96);
		bg_col_dark = MixColors(sc.work, sc.work_graph, 90);
		bg_col_light = MixColors(semi_white, 0xFFFfff, skin_is_dark()*90 + 10);

		top_icons.replace_color(0xffFFFfff, semi_white);
		top_icons.replace_color(0xffCACBD6, MixColors(semi_white, 0, 220));
	}

	DrawWideRectangle(_x+1, _y+1, TSZE, TSZE, 5, semi_white);

	DefineHiddenButton(_x, _y, TSZE+1, TSZE+1, _button_id);
	if (_icon_n==-1) {
		DrawBar(_x+6, _y+5, 16, 16, semi_white);
		for (i=0; i<=2; i++) DrawBar(_x+6, i*5+_y+7, 15, 3, sc.work_graph);
	} else {
		i = TSZE - top_icons.w / 2; //icon pos
		img_draw stdcall(top_icons.image, _x+i+2, _y+i+1+pressed, top_icons.w, top_icons.w, 0, _icon_n*top_icons.w);
	}

	if (!pressed) {
		DrawOvalBorder(_x, _y, TSZE, TSZE, bg_col_light, bg_col_dark, semi_white, sc.work);
	} else {
		DrawOvalBorder(_x, _y, TSZE, TSZE, sc.work_graph, bg_col_light, semi_white, sc.work);
		PutShadow(_x+1, _y+1, TSZE, TSZE, true, 2);
	}

	return _x;
}