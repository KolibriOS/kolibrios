
void DrawTopPanelButton(dword _button_id, _x, _y, signed int _icon_n)
{
	#define TSZE 25
	static libimg_image top_icons;
	static dword semi_white=0, bg_col_light, bg_col_dark;
	if (!semi_white) {
		Libimg_LoadImage(#top_icons, "/sys/icons16.png");

		semi_white = MixColors(sc.work, 0xFFFfff, skin_is_dark()*90 + 96);
		bg_col_dark = MixColors(sc.work, sc.work_graph, 90);
		bg_col_light = MixColors(semi_white, 0xFFFfff, skin_is_dark()*90 + 10);

		Libimg_ReplaceColor(top_icons.image, top_icons.w, top_icons.h, 0xffFFFfff, semi_white);
		Libimg_ReplaceColor(top_icons.image, top_icons.w, top_icons.h, 0xffCACBD6, MixColors(semi_white, 0, 220));
	}

	DrawWideRectangle(_x+1, _y+1, TSZE, TSZE, 5, semi_white);
	DrawOvalBorder(_x, _y, TSZE, TSZE, bg_col_light, bg_col_dark, semi_white, sc.work);

	DefineHiddenButton(_x, _y, TSZE+1, TSZE+1, _button_id);
	if (_icon_n==-1) {
		DrawBar(_x+6, _y+5, 16, 16, semi_white);
	} else {
		img_draw stdcall(top_icons.image, _x+6, _y+5, 16, 16, 0, _icon_n*16);
	}
}