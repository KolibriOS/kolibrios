//Leency - 2012-2013

char *ITEMS_LIST[]={
"WIN                  F5",54,
"DOS              Ctrl+D",04,
"KOI              Ctrl+K",11,
"UTF              Ctrl+U",21,
#ifdef LANG_RUS
"Исходник страницы    F3",52,
"Очистить кэш картинок"  ,02,
#else
"View source          F3",52,
"Free image cache"       ,02,
#endif
0}; 


void menu_rmb()
{
	mouse mm;
	proc_info MenuForm;
	llist menu;
	int overid, key, N;
	dword col_work    = 0xE4DFE1;
	dword col_border  = 0x9098B0;

	menu.first = menu.current = 0;
	while (ITEMS_LIST[menu.count*2]) {menu.count++; menu.visible++;}
	menu.line_h = 19;
	menu.w = 165;
	menu.h = menu.count * menu.line_h;
	SetEventMask(100111b); 

	loop() switch(WaitEvent())
	{
	case evMouse:
				GetProcessInfo(#MenuForm, SelfInfo);
				N=GetProcessSlot(MenuForm.ID);
				if (N<>GetActiveProcess()) ExitProcess();

				mm.get();
				if (menu.ProcessMouse(mm.x, mm.y)) goto _ITEMS_DRAW;
				if (mm.lkm) || (mm.pkm) { action_buf = ITEMS_LIST[menu.current*2+1]; ExitProcess(); }
				break;
				
		case evKey:
				key = GetKey();
				if (key==27) ExitProcess();
				if (menu.ProcessKey(key)) goto _ITEMS_DRAW;
				if (key==13)
				{
					action_buf = ITEMS_LIST[menu.current*2+1];
					ExitProcess();
				}
				break;
				
		case evReDraw:
				DefineAndDrawWindow(Form.left+m.x,Form.top+m.y+GetSkinHeight()+3,menu.w+2,menu.count*menu.line_h+4,0x01, 0, 0, 0x01fffFFF);
				DrawPopup(0,0,menu.w,menu.count*menu.line_h+3,0, col_work,col_border);

				_ITEMS_DRAW:
				for (N=0; N<menu.count; N++;)
				{
					if (N==menu.current) 
						DrawBar(2, N*menu.line_h+2, menu.w-3, menu.line_h, 0x94AECE);
					else
					{
						DrawBar(2, N*menu.line_h+2, menu.w-3, menu.line_h, col_work);
						WriteText(19,N*menu.line_h+9,0x80,0xf2f2f2,ITEMS_LIST[N*2]);
					}
					WriteText(18,N*menu.line_h+8,0x80,0x000000,ITEMS_LIST[N*2]);
				}
				DrawBar(7, cur_encoding*menu.line_h+9, 4, 4, 0x444444); //show current encoding
	}
}





