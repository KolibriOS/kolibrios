//Leency - 2012-2013

char *ITEMS_LIST[]={
#ifdef LANG_RUS
"Посмотреть исходник",
"Редактировать исходник",
"История",
"Очистить кэш картинок",
"Менеджер загрузок",
#else
"View source",
"Edit source",
"History",
"Free image cache",
"Download Manager",
#endif
0};

llist menu;

void menu_rmb()
{
	proc_info MenuForm;

	menu.ClearList();
	while (ITEMS_LIST[menu.count]) menu.count++;
	menu.SetSizes(2,2,177,menu.count*19,19);
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
		DefineAndDrawWindow(Form.left+mouse.x-6,Form.top+mouse.y+GetSkinHeight()+3,menu.w+2,menu.h+4,0x01, 0, 0, 0x01fffFFF);
		DrawPopup(0,0,menu.w,menu.h+3,0, col_bg,border_color);
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
			DrawBar(menu.x, N*menu.item_h+menu.y, menu.w-3, menu.item_h, 0x94AECE);
		else
		{
			DrawBar(menu.x, N*menu.item_h+menu.y, menu.w-3, menu.item_h, col_bg);
			WriteText(19,N*menu.item_h+9,0x80,0xf2f2f2,ITEMS_LIST[N]);
		}
		WriteText(18,N*menu.item_h+8,0x80,0x000000,ITEMS_LIST[N]);
	}
}

void ItemClick()
{
	action_buf = VIEW_SOURCE + menu.cur_y;
	ExitProcess();
}