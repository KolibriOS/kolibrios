
checkbox checkbox1 = { "Checkbox", true };
more_less_box spinbox1 = { 23, 0, 999, "SpinBox" };
edit_box edit_cmm = {180,NULL,NULL,0xffffff,0x94AECE,0xFFFfff,0xffffff,
	0x10000000,sizeof(param)-2,#param,0, 0b};
char st_str[16];
edit_box edit_st = {180,NULL,NULL,0xffffff,0x94AECE,0xFFFfff,0xffffff,
	0x10000000,sizeof(st_str)-2,#st_str,0, 0b};


void DrawUiElementsPreview(dword x,y,h)
{
	incn y2;
	spinbox1.draw(x, y2.set(y+30));
	WriteText(x, y2.inc(30), 0x90, sc.work_text, "C-- Edit");
	DrawEditBoxPos(x, y2.inc(20), #edit_cmm);
	WriteText(x, y2.inc(35), 0x90, sc.work_text, "Standard Edit");
	DrawStEditBoxPos(x, y2.inc(20), #edit_st);
	DrawStandartCaptButton(x, y+h-40, GetFreeButtonId(), "Button1");
	DrawStandartCaptButton(x+100, y+h-40, GetFreeButtonId(), "Button2");
}

:void DrawStEditBoxPos(dword x,y, edit_box_pointer)
{
	dword c_inactive = MixColors(sc.line, sc.work, 128);
	dword c_active = MixColors(sc.line, 0, 128);
	ESI = edit_box_pointer;
	ESI.edit_box.left = x;
	ESI.edit_box.top = y;
	ESI.edit_box.border_color = c_inactive;
	ESI.edit_box.focus_border_color = c_active;
	edit_box_draw  stdcall (edit_box_pointer);
}