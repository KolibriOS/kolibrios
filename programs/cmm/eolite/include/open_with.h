//Leency 2013

llist app_list;
struct app_list_string { char item[1024]; char ext[5]; };
app_list_string app_paths[100];

int GetListOfPrograms()
{
	byte section[32], parametr[32], option[256], InfType=0;
	char bukva[2];
	int tj, ti;
	static dword buff, fsize;

	free(buff);
	if (!GetFile(#buff, #fsize, abspath("Eolite.ini")))
	{
		notify("Eolite.ini not found. Don't know any programm.");
		return -1;
	}

	for (tj=0; tj<fsize; tj++;) 
	{   
		bukva = ESBYTE[buff+tj];
		switch (bukva)
		{
			case ';': InfType=COMMENT; break;				
			case '[': InfType=SECTION; section=NULL; break;
			case ']': InfType=PARAM; break;
			case '=': InfType=OPTION; break;
			case 0x0a:
			case 0x0d:
				if (!strcmp(#section,"Associations")) && (option) && (InfType!=COMMENT)
				{
					for (ti=0; ti<app_list.count; ti++) //do not add duplications
					{
						if (strcmp(#app_paths[ti].item, #option)==0) goto _OUT;
					}
					if (kolibrios_drive==false) && (strstr(#option,"kolibrios/")!=0) goto _OUT;
					strcpy(#app_paths[app_list.count].item, #option);
					if (strlen(#parametr)<=5) && (parametr[0]) strcpy(#app_paths[app_list.count].ext, #parametr);
					else strcpy(#app_paths[app_list.count].ext, "kex");
					app_list.count++;
				}
				_OUT:
				InfType=PARAM;
				parametr=option=NULL;
				break;
			default:
				IF (InfType==SECTION) chrcat(#section, bukva);
				IF (InfType==PARAM) chrcat(#parametr, bukva);
				IF (InfType==OPTION) chrcat(#option, bukva);
		}
	}
}



void OpenWith()
{
	#define WIN_W 290
	#define OPEN_LIST_VISIBLE_N 12
	#define OPEN_LIST_LINE_H 20
	#define PANEL_H 40
	#define PADDING 8
	int WIN_H;
	mouse mm;
	word key, slot;
	proc_info MenuForm;

	app_list.ClearList();
	app_list.SetSizes(PADDING,PANEL_H+1,WIN_W-PADDING-PADDING,OPEN_LIST_VISIBLE_N*OPEN_LIST_LINE_H,150,OPEN_LIST_LINE_H);
	if (!app_list.count) if (GetListOfPrograms()==-1) return;
	SetEventMask(100111b);
	goto _APP_LIST_DRAW;

	loop() switch(WaitEvent())
	{
		case evMouse:
				slot = GetProcessSlot(MenuForm.ID);
				if (slot != GetActiveProcess()) ExitProcess();
				mm.get();
				if (mm.vert) && (app_list.MouseScroll(mm.vert)) DrawAppList();
				if (app_list.ProcessMouse(mm.x, mm.y)) DrawAppList();
				if (app_list.MouseOver(mm.x, mm.y)) && (mm.lkm)
				{
					RunProgram(#app_paths[app_list.current].item, #file_path);
					ExitProcess();
				}

				break;
				
		case evKey:
				key = GetKey();
				if (key==27) ExitProcess();
				if (key==13) { RunProgram(#app_paths[app_list.current].item, #file_path); ExitProcess(); }
				if (app_list.ProcessKey(key)) DrawAppList();
				break;
				
		case evReDraw: _APP_LIST_DRAW:
				WIN_H = app_list.h+PANEL_H+PADDING+2;
				DefineAndDrawWindow(files.w-WIN_W/2+files.x+Form.left+6, 
				files.h-WIN_H/2+files.y+Form.top+GetSkinHeight(),WIN_W+1,WIN_H,0x01, 0, 0, 0x01fffFFF);
				GetProcessInfo(#MenuForm, SelfInfo);
				DrawPopup(0,0,MenuForm.width-2,MenuForm.height-2,0, sc.work, sc.work_graph);
				DrawRectangle(app_list.x-1, app_list.y-2, app_list.w+1, app_list.h+2, sc.work_graph);

				Put_icon(#file_name+_strrchr(#file_name,'.'), 10, 13, sc.work, 0);
				WriteText(35,10, 0x80, sc.work_text, T_SELECT_APP_TO_OPEN_WITH);
				WriteText(35,23, 0x80, sc.work_text, #file_name);

				DrawAppList();
	}
}

void DrawAppList()
{
	#define SCROLL_WIDTH 5
	llist tiny_scroll;
	dword index, col_bg;
	for (index = 0; (index<app_list.visible) && (index+app_list.first<app_list.count); index++)
	{
		if (index+app_list.first==app_list.current) col_bg = col_selec; else col_bg = 0xFFFfff;
		DrawBar(app_list.x, index*app_list.line_h+app_list.y, app_list.w, app_list.line_h, col_bg);
		Put_icon(#app_paths[index+app_list.first].ext, app_list.x+4, index*app_list.line_h+app_list.y+2, col_bg, 6);
		WriteText(app_list.x+25, index*app_list.line_h+app_list.y+7, 0x80, 0, #app_paths[index+app_list.first].item);
	}
	tiny_scroll.x = app_list.x+app_list.w-SCROLL_WIDTH-1;
	tiny_scroll.h = app_list.h * app_list.visible / app_list.count;
	tiny_scroll.y = app_list.h * app_list.first / app_list.count + app_list.y;
	debugi(tiny_scroll.y + tiny_scroll.h - app_list.y - app_list.h);
	if (tiny_scroll.y + tiny_scroll.h - app_list.y - app_list.h >= 0) tiny_scroll.y = app_list.y + app_list.h - tiny_scroll.h-1;
	DrawBar(tiny_scroll.x, tiny_scroll.y, SCROLL_WIDTH, tiny_scroll.h, 0x555555); //scroll
}