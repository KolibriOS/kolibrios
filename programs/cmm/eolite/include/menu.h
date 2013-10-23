//Leency 2008-2013

//pay attension: >200 this is only file actions, not supported by folders
#ifdef LANG_RUS
char *file_captions[] = {
	"Открыть",               "Enter",100,
	"Открыть с помощью...",  "CrlEnt",201,
	"Открыть как текст",     "F3",202,
	"Открыть как HEX",       "F4",203,
	"Копировать",            "Crl+C",104,
	"Вырезать",              "Crl+X",105,
	"Вставить",              "Crl+V",106,
	"Переименовать",         "F2",207,
	"Удалить",               "Del",108,
	"Обновить папку",        "F5",109,
	0, 0, 0};
#elif LANG_EST
char *file_captions[] = {
	"Ava",            "Enter",100,
	"Ava ...",        "CrlEnt",201,
	"Vaata tekstina", "F3",202,
	"Vaata HEX",      "F4",203,
	"Kopeeri",        "Crl+C",104,
	"Lїika",          "Crl+X",105,
	"Aseta",          "Crl+V",106,
	"Nimeta №mber",   "F2",207,
	"Kustuta",        "Del",108,
	"Vфrskenda",      "F5",109,
	0, 0, 0};
#else
char *file_captions[] = {
	"Open",          "Enter",100,
	"Open with...",  "CrlEnt",201,
	"View as text",  "F3",202,
	"View as HEX",   "F4",203,
	"Copy",          "Crl+C",104,
	"Cut",           "Crl+X",105,
	"Paste",         "Crl+V",106,
	"Rename",        "F2",207,
	"Delete",        "Del",108,
	"Refresh",       "F5",109,
	0, 0, 0};
#endif

llist menu;
int cur_action_buf;

void FileMenu()
{
	mouse mm;
	word slot, key;
	proc_info MenuForm;
	int index;

	menu.ClearList();
	menu.SetSizes(m.x+Form.left+5,m.y+Form.top+GetSkinHeight(),10,0,0,18);
	for (index=0; file_captions[index]!=0; index+=3)
	{
		if (itdir) && (file_captions[index+2]>=200) continue;
		if (strlen(file_captions[index])>menu.w) menu.w = strlen(file_captions[index]);
		menu.count++;
		menu.visible++;
	}
	menu.w = menu.w + 3 * 6 + 50;
	menu.h = menu.count*menu.line_h;
	SetEventMask(100111b);
	goto _MENU_DRAW;
	
	loop() switch(WaitEvent())
	{
		case evMouse:
				slot = GetProcessSlot(MenuForm.ID);
				if (slot != GetActiveProcess()) ExitProcess();
				mm.get();
				menu.current_temp = mm.y - 1 / menu.line_h;
				if (mm.y<=0) || (mm.y>menu.h+5) || (mm.x<0) || (mm.x>menu.w) menu.current_temp = -1;
				if (menu.current<>menu.current_temp)
				{
					menu.current=menu.current_temp;
					MenuListRedraw();
				}
				if (mm.lkm) {action_buf = cur_action_buf; pause(5); ExitProcess(); }
				break;
				
		case evKey:
				key = GetKey();
				if (key==27) ExitProcess();
				if (key == 13) {action_buf = cur_action_buf; ExitProcess(); }
				if (menu.ProcessKey(key)) MenuListRedraw();
				break;
				
		case evReDraw: _MENU_DRAW:
				DefineAndDrawWindow(menu.x, menu.y,menu.w+3,menu.h+6,0x01, 0, 0, 0x01fffFFF);
				GetProcessInfo(#MenuForm, SelfInfo);
				DrawRectangle(0,0,menu.w+1,menu.h+2,sc.work_graph);
				DrawBar(1,1,menu.w,1,0xFFFfff);
				DrawPopupShadow(1,1,menu.w,menu.h,0);
				MenuListRedraw();
	}
}

void MenuListRedraw()
{
	int start_y=0;
	int index;
	int texty = menu.line_h/2-4;
	for (index=0; file_captions[index*3]!=0; index++)
	{
		if ((itdir) && (file_captions[index*3+2]>=200)) continue;
		DrawBar(1,start_y+2,1,menu.line_h,0xFFFfff);
		if (start_y/menu.line_h==menu.current)
		{
			cur_action_buf = file_captions[index*3+2];
			DrawBar(2,start_y+2,menu.w-1,menu.line_h,0xFFFfff);
		}
		else
		{
			DrawBar(2,start_y+2,menu.w-1,menu.line_h,sc.work);
			WriteText(8,start_y+texty+3,0x80,0xf2f2f2,file_captions[index*3]);
		}
		WriteText(7,start_y+texty+2,0x80,sc.work_text,file_captions[index*3]);
		WriteText(-strlen(file_captions[index*3+1])*6-6+menu.w,start_y+texty+2,0x80,0x888888,file_captions[index*3+1]);
		start_y+=menu.line_h;
	}	
}