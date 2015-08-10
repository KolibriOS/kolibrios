//Leency - 2012-2013

char *ITEMS_LIST[]={
"WIN                Ctrl+E",05,
"DOS                Ctrl+D",04,
"KOI                Ctrl+K",11,
"UTF                Ctrl+U",21,
#ifdef LANG_RUS
"Zoom 2x                 Z",122,
"Посмотреть исходник    F3",52,
"Редактировать исходник F4",53,
"Очистить кэш картинок"    ,02,
"История"                  ,03,
"Менеджер загрузок"        ,06,
#else
"Zoom 2x                 Z",122,
"View source            F3",52,
"Edit source            F4",53,
"Free image cache"         ,09,
"History"                  ,03,
"Download Manager"         ,06,
#endif
0}; 

llist menu;

void menu_rmb()
{
	proc_info MenuForm;
	int key;

	menu.first = menu.current = 0;
	while (ITEMS_LIST[menu.count*2]) menu.count++;
	menu.SetSizes(2,2,177,menu.count*19,0,19);
	SetEventMask(100111b); 

	_BEGIN_APPLICATION_MENU:
	switch(WaitEvent())
	{
	case evMouse:
				GetProcessInfo(#MenuForm, SelfInfo);
				if (!CheckActiveProcess(MenuForm.ID)) ExitProcess();

				mouse.get();
				if (menu.ProcessMouse(mouse.x, mouse.y)) DrawMenuList();
				if (mouse.lkm)&&(mouse.up) { action_buf = ITEMS_LIST[menu.current*2+1]; ExitProcess(); }
				break;
				
		case evKey:
				key = GetKey();
				if (key==27) ExitProcess();
				if (menu.ProcessKey(key)) DrawMenuList();
				if (key==13)
				{
					action_buf = ITEMS_LIST[menu.current*2+1];
					ExitProcess();
				}
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
		if (N==menu.current) 
			DrawBar(menu.x, N*menu.line_h+menu.y, menu.w-3, menu.line_h, 0x94AECE);
		else
		{
			DrawBar(menu.x, N*menu.line_h+menu.y, menu.w-3, menu.line_h, col_bg);
			WriteText(19,N*menu.line_h+9,0x80,0xf2f2f2,ITEMS_LIST[N*2]);
		}
		WriteText(18,N*menu.line_h+8,0x80,0x000000,ITEMS_LIST[N*2]);
	}
	if (cur_encoding!=CH_NULL)
		WriteText(5, cur_encoding*menu.line_h+7, 0x80, 0x777777, "\x10"); //show current encoding
	else 
		WriteText(5, CH_CP866*menu.line_h+7, 0x80, 0x777777, "\x10"); //show current encoding

	if (WB1.DrawBuf.zoom == 2) DrawBar(6, 4*menu.line_h+8, 6, 6, 0x777777);
}
