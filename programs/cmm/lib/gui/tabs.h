
#define TAB_PADDING 15
#define TAB_HEIGHT 28

:struct _tabs
{
	int x,y,w,h;
	int active_tab;
	int c;
	void draw_button();
	int click();
	void draw_wrapper();
};

:void _tabs::draw_wrapper()
{
	dword color_light = MixColors(system.color.work, 0xFFFfff, 40);
	dword color_content = MixColors(system.color.work, 0xFFFfff, 120);
	dword color_light_border = MixColors(system.color.work, system.color.work_graph, 120);

	DrawRectangle(x-1, y-1, w+1, h+1, system.color.work_graph);
	DrawBar(x, y, w, h, color_content); //0xF3F3F3
	DrawRectangle3D(x, y, w-1, h-1, color_light, color_content); //0xF3F3F3

	DrawBar(x+1, y+h+1, w-2, 2, system.color.work_dark); //"shadow"

	DrawBar(x, y+TAB_HEIGHT-1, w, 1, color_light_border);
	DrawBar(x, y+TAB_HEIGHT, w, 1, color_light);

	c = y + TAB_HEIGHT;
}

:void _tabs::draw_button(dword xx, but_id, text)
{
	dword col_bg, col_text;
	dword ww=strlen(text)*8, hh=TAB_HEIGHT;

	if (but_id==active_tab)
	{
		col_bg=0xE44C9C;
		col_text=0x000000;
	}
	else
	{
		col_bg=0xC3A1B7;
		col_text=0x333333;
	} 
	DefineHiddenButton(xx,y, ww-1,hh-1, but_id);
	WriteText(xx, y+6, 0x90, col_text, text);
	DrawBar(xx, y+hh-3, ww, 3, col_bg);
}

:int _tabs::click(int N)
{
	if (N==active_tab) return false;
	active_tab = N;
	return true;
}