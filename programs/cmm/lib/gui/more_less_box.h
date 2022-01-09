:struct more_less_box
{
	unsigned value, min, max;
	dword text;
	int click_delta;
	int x,y;
	unsigned id_inc, id_dec;
	bool disabled;
	void check_values();
	bool click();
	bool inc();
	bool dec();
	void draw();
	void redraw();
};

:void more_less_box::check_values()
{
	if (!id_inc) id_inc = GetFreeButtonId();
	if (!id_dec) id_dec = GetFreeButtonId();
	if (!click_delta) click_delta = 1;
}

:bool more_less_box::click(unsigned id)
{
	if (disabled) return 0;
	if (id==id_dec) { value = math.max(value-click_delta, min); redraw(); return 1; }
	if (id==id_inc) { value = math.min(value+click_delta, max); redraw(); return 1; }
	return 0;
}

:bool more_less_box::inc()
{
	click(id_inc);
}

:bool more_less_box::dec()
{
	click(id_dec);
}

:void more_less_box::draw(dword _x,_y)
{
	#define VALUE_FIELD_W 34
	#define SIZE 18
	dword text_col;
	dword value_text = itoa(value);

	check_values();
	x=_x; y=_y;

	DrawRectangle(x, y, VALUE_FIELD_W+1, SIZE, sc.line);
	DrawRectangle3D(x+1, y+1, VALUE_FIELD_W-2, SIZE-2, 0xDDDddd, 0xffffff);

	if (disabled)
	{
		DrawRectangle(x+1, y+1, VALUE_FIELD_W-2, SIZE-2, 0xffffff);
		DrawBar(x+2, y+2, VALUE_FIELD_W-3, SIZE-3, 0xCCCccc);
		text_col = sc.line;
	}
	else 
	{
		DrawBar(x+2, y+2, VALUE_FIELD_W-3, SIZE-3, 0xffffff);
		text_col = sc.work_text;
	}

	WriteText( -strlen(value_text)+3*8 + x+6, SIZE / 2 + y -6, 0x90, 0x333333, value_text);

	DrawCaptButton(VALUE_FIELD_W + x + 1,    y, SIZE, SIZE, id_inc, sc.button, sc.button_text, "+");
	DrawCaptButton(VALUE_FIELD_W + x + SIZE, y, SIZE, SIZE, id_dec, sc.button, sc.button_text, "-");
	WriteTextWithBg(x+VALUE_FIELD_W+SIZE+SIZE+10, SIZE / 2 + y -7, 0xD0, text_col, text, sc.work);
	DrawRectangle3D(x-1,y-1,VALUE_FIELD_W+SIZE+SIZE+2,SIZE+2,sc.dark,sc.light);
}

:void more_less_box::redraw()
{
	draw(x,y);
}

