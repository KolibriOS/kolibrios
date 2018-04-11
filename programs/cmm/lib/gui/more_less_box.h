:struct more_less_box
{
	signed x,y;
	unsigned value, min, max;
	unsigned bt_id_more, bt_id_less;
	dword text;
	bool click();
	void draw();
};

:bool more_less_box::click(unsigned id)
{
	if (id==bt_id_less) { value = math.max(value-1, min); draw(); return 1; }
	if (id==bt_id_more) { value = math.min(value+1, max); draw(); return 1; }
	return 0;
}

:void more_less_box::draw()
{
	#define VALUE_FIELD_W 34
	#define SIZE 18
	dword value_text = itoa(value);

	DrawRectangle(x, y, VALUE_FIELD_W+1, SIZE, system.color.work_graph);
	DrawRectangle3D(x+1, y+1, VALUE_FIELD_W-2, SIZE-2, 0xDDDddd, 0xffffff);
	DrawBar(x+2, y+2, VALUE_FIELD_W-3, SIZE-3, 0xffffff);
	WriteText( -strlen(value_text)+3*8 + x+6, SIZE / 2 + y -6, 0x90, 0x333333, value_text);

	DrawCaptButton(VALUE_FIELD_W + x + 1,    y, SIZE, SIZE, bt_id_more, system.color.work_button, system.color.work_button_text, "+");
	DrawCaptButton(VALUE_FIELD_W + x + SIZE, y, SIZE, SIZE, bt_id_less, system.color.work_button, system.color.work_button_text, "-");
	EDI = system.color.work;
	WriteText(x+VALUE_FIELD_W+SIZE+SIZE+10, SIZE / 2 + y -7, 0xD0, system.color.work_text, text);
	DrawRectangle3D(x-1,y-1,VALUE_FIELD_W+SIZE+SIZE+2,SIZE+2,system.color.work_dark,system.color.work_light);
}

//OUTDATED: to be removed
:void MoreLessBox(dword x,y, bt_id_more, bt_id_less, value, text)
{
	#define VALUE_FIELD_W 34
	#define SIZE 18
	dword value_text = itoa(value);

	DrawRectangle(x, y, VALUE_FIELD_W+1, SIZE, system.color.work_graph);
	DrawRectangle3D(x+1, y+1, VALUE_FIELD_W-2, SIZE-2, 0xDDDddd, 0xffffff);
	DrawBar(x+2, y+2, VALUE_FIELD_W-3, SIZE-3, 0xffffff);
	WriteText( -strlen(value_text)+3*8 + x+6, SIZE / 2 + y -6, 0x90, 0x333333, value_text);

	DrawCaptButton(VALUE_FIELD_W + x + 1,    y, SIZE, SIZE, bt_id_more, system.color.work_button, system.color.work_button_text, "+");
	DrawCaptButton(VALUE_FIELD_W + x + SIZE, y, SIZE, SIZE, bt_id_less, system.color.work_button, system.color.work_button_text, "-");
	EDI = system.color.work;
	WriteText(x+VALUE_FIELD_W+SIZE+SIZE+10, SIZE / 2 + y -7, 0xD0, system.color.work_text, text);
	DrawRectangle3D(x-1,y-1,VALUE_FIELD_W+SIZE+SIZE+2,SIZE+2,system.color.work_dark,system.color.work_light);
}
