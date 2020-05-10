#ifndef TAB_PADDING
#define TAB_PADDING 15
#endif

#define TAB_HEIGHT 28

:struct _tabs
{
	int active_tab;
	int x,y,w,h;
	dword draw_button();
	int click();
	void draw_wrapper();
};

:void _tabs::draw_wrapper()
{
	DrawRectangle(x,y+TAB_HEIGHT,w-1,h-TAB_HEIGHT, sc.work_graph);
	DrawBar(x+1,y+1+TAB_HEIGHT,w-3,1, sc.work_light);
}

:dword _tabs::draw_button(dword xx, but_id, text)
{
	dword col_bg, col_text;
	dword ww=strlen(text)*8, hh=TAB_HEIGHT;

	if (but_id==active_tab)
	{
		col_bg=0xE44C9C;
		col_text=sc.work_text;
	}
	else
	{
		col_bg=0xC3A1B7;
		col_text= MixColors(sc.work, sc.work_text, 120);
	} 
	DefineHiddenButton(xx-2,y, ww-1+4,hh-1, but_id);
	WriteText(xx, y+6, 0x90, col_text, text);
	DrawBar(xx, y+hh-3, ww, 3, col_bg);
	//DrawStandartCaptButton(xx, y, but_id, text); //GetFreeButtonId()
	return xx;
}

:int _tabs::click(int N)
{
	if (N==active_tab) return false;
	active_tab = N;
	return true;
}