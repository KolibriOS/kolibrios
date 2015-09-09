char stak[4096];

byte action_buf;

llist menu;

void menu_rmb()
{
	proc_info MenuForm;
	menu.ClearList();
	while (charsets[menu.count]) menu.count++;
	menu.SetSizes(2,2,140,menu.count*19,19);
	SetEventMask(100111b);
	_BEGIN_APPLICATION_MENU:
	switch(WaitEvent())
	{
	case evMouse:
		GetProcessInfo(#MenuForm, SelfInfo);
		if (!CheckActiveProcess(MenuForm.ID)) ExitProcess();
		mouse.get();
		if (menu.ProcessMouse(mouse.x, mouse.y)) DrawMenuList();
		if (mouse.lkm)&&(mouse.up) ItemClick();
		break;		
	case evKey:
		GetKeys();
		if (key_scancode==SCAN_CODE_ESC) ExitProcess();
		if (key_scancode==SCAN_CODE_ENTER) ItemClick();
		if (menu.ProcessKey(key_scancode)) DrawMenuList();
		break;
	case evReDraw:
		DefineAndDrawWindow(Form.left+104,Form.top+29+SKIN.height,menu.w+2,menu.h+4,0x01, 0, 0, 0x01fffFFF);
		DrawPopup(0,0,menu.w,menu.h+3,0, 0xE4DFE1,0x9098B0);
		DrawMenuList();				
	}
	goto _BEGIN_APPLICATION_MENU;
}

void DrawMenuList()
{
	int N;
	for (N=0; N<menu.count; N++;)
	{
		if (N==menu.cur_y) 
			DrawBar(menu.x, N*menu.item_h+menu.y, menu.w-3, menu.item_h, 0xFFFfff);
		else
		{
			DrawBar(menu.x, N*menu.item_h+menu.y, menu.w-3, menu.item_h, 0xE4DFE1);
			WriteText(19,N*menu.item_h+9,0x80,0xf2f2f2,charsets[N]);
		}
		WriteText(18,N*menu.item_h+8,0x80,0x000000,charsets[N]);
	}
	WriteText(5, encoding*menu.item_h+7, 0x80, 0x777777, "\x10");
}

void ItemClick()
{
	if (encoding!=menu.cur_y)
	{
		encoding = menu.cur_y;
		action_buf = true;
	}
	ExitProcess();
}