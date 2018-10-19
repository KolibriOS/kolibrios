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
	"Свойства",             "F8",110,
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
	"Properties",     "F8",110,
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
	"Properties",    "F8",110,
	0, 0, 0};
#endif

llist rbmenu;
int cur_action_buf;

void FileMenu()
{
	proc_info MenuForm;
	int index;

	rbmenu.ClearList();
	rbmenu.SetFont(6, 9, 0x80);
	rbmenu.SetSizes(0,0,10,0,18);
	for (index=0; file_captions[index]!=0; index+=3)
	{
		if (selected_count > 0) {
			//if there are files selected then show only specific menu items
			if (file_captions[index+2]>=200) continue;
			if (file_captions[index+2]==100) continue;
		}
		else if (itdir) && (file_captions[index+2]>=200) continue;
		if (strlen(file_captions[index])>rbmenu.w) rbmenu.w = strlen(file_captions[index]);
		rbmenu.count++;
		rbmenu.visible++;
	}
	rbmenu.w = rbmenu.w + 3 * rbmenu.font_w + 50;
	rbmenu.h = rbmenu.count * rbmenu.item_h;
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE);
	goto _MENU_DRAW;
	
	loop() switch(WaitEvent())
	{
		case evMouse:
				mouse.get();
				if (!CheckActiveProcess(MenuForm.ID)){ cmd_free=1; ExitProcess();}
				else if (mouse.move)&&(rbmenu.ProcessMouse(mouse.x, mouse.y)) MenuListRedraw();
				else if (mouse.key&MOUSE_LEFT)&&(mouse.up) {action_buf = cur_action_buf; cmd_free=1; ExitProcess(); }
		break;
				
		case evKey:
				GetKeys();
				if (key_scancode == SCAN_CODE_ESC) {cmd_free=1;ExitProcess();}
				if (key_scancode == SCAN_CODE_ENTER) {action_buf = cur_action_buf; cmd_free=1; ExitProcess(); }
				if (rbmenu.ProcessKey(key_scancode)) MenuListRedraw();
				break;
				
		case evReDraw: _MENU_DRAW:
				if (menu_call_mouse) 
					DefineAndDrawWindow(mouse.x+Form.left+5, mouse.y+Form.top+skin_height,rbmenu.w+3,rbmenu.h+6,0x01, 0, 0, 0x01fffFFF);
				else 
					DefineAndDrawWindow(Form.left+files.x+15, files.item_h*files.cur_y+files.y+Form.top+30,rbmenu.w+3,rbmenu.h+6,0x01, 0, 0, 0x01fffFFF);
				GetProcessInfo(#MenuForm, SelfInfo);
				DrawRectangle(0,0,rbmenu.w+1,rbmenu.h+2,col_graph);
				DrawBar(1,1,rbmenu.w,1,0xFFFfff);
				DrawPopupShadow(1,1,rbmenu.w,rbmenu.h,0);
				MenuListRedraw();
	}
}

void MenuListRedraw()
{
	int start_y=0;
	int index;

	dword m_col_bg;
	dword m_col_text;
	dword m_col_sh_text;

	for (index=0; file_captions[index*3]!=0; index++)
	{
		if (selected_count > 0) {
			if (file_captions[index*3+2]==100) continue;
			if (file_captions[index*3+2]>=200) continue;
		}
		else if ((itdir) && (file_captions[index*3+2]>=200)) continue;
		DrawBar(1,start_y+2,1,rbmenu.item_h,0xFFFfff);
		if (start_y/rbmenu.item_h==rbmenu.cur_y)
		{
			cur_action_buf = file_captions[index*3+2];
			m_col_bg = 0xFFFfff;
			m_col_sh_text = 0xFAFAFA;
			m_col_text = 0;
		}
		else
		{
			m_col_bg = col_work;
			m_col_text = system.color.work_text;
			m_col_sh_text = system.color.work_light;
		}
		DrawBar(2, start_y+2, rbmenu.w-1, rbmenu.item_h, m_col_bg);
		WriteText(8, start_y + rbmenu.text_y + 4, rbmenu.font_type, m_col_sh_text, file_captions[index*3]);
		WriteText(7, start_y + rbmenu.text_y + 3, rbmenu.font_type, m_col_text, file_captions[index*3]);
		WriteText(-strlen(file_captions[index*3+1])-1*rbmenu.font_w + rbmenu.w, start_y + rbmenu.text_y + 3, rbmenu.font_type, 0x888888, file_captions[index*3+1]);
		start_y+=rbmenu.item_h;
	}	
}