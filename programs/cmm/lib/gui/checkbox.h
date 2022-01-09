#ifndef INCLUDE_CHECKBOX
#define INCLUDE_CHECKBOX

struct checkbox
{
	dword text;
	bool checked;
	bool disabled;
	dword x,y, id;
	bool click();
	void draw();
	void redraw();
};

:bool checkbox::click(dword _id)
{
	if (disabled) return 0;
	if (_id == id) {
		checked^=1; 
		redraw(); 
		return 1;		
	}
	return 0;
}

:void checkbox::draw(dword _x,_y)
{
	#define SIZE 14
	static dword checkbox_flag;
	dword text_col = sc.work_text;
	if (!id) id = GetFreeButtonId();
	x=_x; y=_y;

	DefineHiddenButton(x-1, y-1, strlen(text)*8 + SIZE + 17, SIZE+2, id+BT_NOFRAME);
	UnsafeDefineButton(x, y, SIZE, SIZE, id, 0);
	DrawRectangle(x, y, SIZE, SIZE, sc.line);
	if (disabled)
	{
		DrawRectangle(x+1, y+1, SIZE-2, SIZE-2, 0xffffff);
		DrawBar(x+2, y+2, SIZE-3, SIZE-3, 0xCCCccc);
		text_col = MixColors(sc.work, sc.work_text, 128);
	}
	else if (checked == false)
	{
		DrawRectangle3D(x+1, y+1, SIZE-2, SIZE-2, 0xDDDddd, 0xffffff);
		DrawBar(x+2, y+2, SIZE-3, SIZE-3, 0xffffff);
	} 
	else if (checked == true)
	{
		if (!checkbox_flag) checkbox_flag = memopen("CHECKBOX", NULL, SHM_READ);
		if (checkbox_flag) PutImage(x+1, y+1, 13, 13, checkbox_flag);
		else DrawBar(x+2, y+2, SIZE-3, SIZE-3, 0x58C33C);
	}
	if (text) WriteTextWithBg(x+SIZE+8, SIZE / 2 + y -7, 0xD0, text_col, text, sc.work);
	DrawRectangle3D(x-1,y-1,SIZE+2,SIZE+2,sc.dark,sc.light);
}

:void checkbox::redraw()
{
	draw(x,y);
}

#endif